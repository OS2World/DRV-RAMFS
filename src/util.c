#include "includes.h"

#include <stdarg.h>
#include <stdio.h>
#include <dos.h>

#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#ifndef MK_FP
 #define MK_FP(seg,ofs)  ((void FAR *)(((unsigned long)seg<<16)+(unsigned long)ofs))
#endif

PFN near DevHlp;
PGINFOSEG near pGiseg;
unsigned long alloc_flags=0x00000004;   /* Swappable by default */

/* static */
volatile char InRamfs = FALSE;
#ifdef DEBUG
unsigned short debug_port = 0x2F8;
#endif


#ifdef DEBUG

char debugging = TRUE;

/*
 * These three routines are just stubs to prevent the linker from
 * plunging into the full-blown MS C v 6.0 RTL
 */

/* An imitation of _flsbuf that assumes we work on strings only */

int _FAR_ _cdecl _flsbuf(int a, FILE _FAR_ *b)
{
 return(-1);
}

/* 80x87 trap? Unused */

void _cdecl _fptrap() {}

/* Reads a character from a port */

static unsigned char inportb(unsigned int d)
{
 _asm{
  mov dx, d
  in al, dx
 }
}

/* Writes a character to a port */

static void outportb(unsigned int d, unsigned char a)
{
 _asm{
  mov dx, d
  mov al, a
  out dx, al
 }
}

/* A simplified hexadecimal translator to avoid pulling strtol() */

#define ishex(s) ((s>='0'&&s<='9')||(s>='A'&&s<='F')||(s>='a'&&s<='f'))
unsigned int hextoi (const char _ds *str)
{
 unsigned int rc=0;

 while(ishex(*str))
 {
  rc<<=4;
  if(*str>='0'&&*str<='9')
   rc+=(*str-48);
  else if(*str>='A'&&*str<='F')
   rc+=(*str-55);
  else
   rc+=(*str-87);
  str++;
 }
 return(rc);
}

/* The main debug printf() routine */

int _cdecl _loadds debug_printf (const char _ds *format, ...)
{
  va_list args;
  char buf [256];
  int i;
  int len = 0;

  if (!debugging)
    return 0;

  va_start (args, format);
  vsprintf (buf, format, args);
  for (i=0; buf[i]!='\0'; i++)
  {
    while (!(inportb (debug_port + 5) & 0x20))
      ;
    outportb (debug_port, buf[i]);
    len++;
  }
  va_end (args);
  return len;
}

#endif




ULONG UtilGetDateTime (void)
{
  USHORT dateNow;
  USHORT timeNow;

  do
  {
    dateNow = ((pGiseg->year-1980) <<  9) |
	       (pGiseg->month      <<  5) |
	       (pGiseg->day        <<  0);

    timeNow =  (pGiseg->hour    << 11) |
	       (pGiseg->minutes <<  5) |
	       (pGiseg->seconds >>  1);
  }
  while
  (
    dateNow != (((pGiseg->year-1980) <<  9) |
	       (pGiseg->month      <<  5) |
	       (pGiseg->day        <<  0))
    ||
    timeNow != ((pGiseg->hour    << 11) |
	       (pGiseg->minutes <<  5) |
	       (pGiseg->seconds >>  1))
  );
  return dateNow + ((ULONG)timeNow << 16);
}




/* static */
int UtilScanDir (FLAT flatBlkDir, FLAT _ss *pflatEntry, PDIRENTRY pEntry,
		 char *pachName, int cbName)
{
  BLOCK BlkDir;
  FLAT  flatEnd;
  char  szUpName[256];
  char  szUpCurName[256];
  int   cbCurName;
  int   rc;

  memcpy (szUpName, pachName, cbName);
  szUpName[cbName] = '\0';
  FSH_UPPERCASE (szUpName, 256, szUpName);

  VMReadBlk (&BlkDir, flatBlkDir);

  *pflatEntry = BlkDir.flatAddr + DIR_DOTSSIZE;
  flatEnd     = BlkDir.flatAddr + BlkDir.cbSize;
  while (*pflatEntry < flatEnd)
  {
    cbCurName = VMReadUChar (*pflatEntry);
    VMRead (pEntry, *pflatEntry, sizeof(DIRENTRY)-CCHMAXPATHCOMP + cbCurName);
    pEntry->achName[cbCurName] = '\0';
    FSH_UPPERCASE (pEntry->achName, 256, szUpCurName);

    rc = memcmp (szUpName, szUpCurName, MIN(cbName,cbCurName));
    if (!rc  &&  cbName == cbCurName)
      return NO_ERROR;
    if (rc < 0)
      return ERROR_FILE_NOT_FOUND;
    *pflatEntry += sizeof(DIRENTRY)-CCHMAXPATHCOMP + cbCurName;
  }

#ifdef DEBUG
  if (*pflatEntry != flatEnd)
  {
    debugging = TRUE;
    DEBUG_PRINTF2 ("\r\n!!! UtilScanDir  flatAddr = 0x%08lX   cbSize = 0x%08lX\r\n",
		   BlkDir.flatAddr, BlkDir.cbSize);
    INT3;
  }
#endif

  return ERROR_FILE_NOT_FOUND;
}




int UtilLocate (FLAT _ss *pflatBlkDir, FLAT _ss *pflatEntry, PDIRENTRY pEntry,
		PSZ pszPath)
{
  int cbPath;
  int cbComp;
  unsigned char *pchMatch;
  static char achSeparator[2] = { '\\', '/' };

  cbPath = strlen (pszPath);
  while (1)
  {
    pchMatch = pszPath;
    if (FSH_FINDCHAR (2, achSeparator, &pchMatch) == ERROR_CHAR_NOT_FOUND)
      break;

    /* not reached the end of the path yet, this path component is a subdir */
    cbComp = (int)pchMatch - (int)pszPath;
    if (UtilScanDir (*pflatBlkDir, pflatEntry, pEntry, pszPath, cbComp))
      return LOC_NOPATH;
    if ((pEntry->fDOSattr & DOSATTR_DIRECTORY) == 0)
      return LOC_NOPATH;
    *pflatBlkDir = *pflatEntry + FIELDOFFSET(DIRENTRY,fblkFile);
    pszPath += cbComp+1;
    cbPath  -= cbComp+1;
  }

  /* no more backslashes in pszPath, we have reached the name component */
  cbComp = cbPath;
  if (UtilScanDir (*pflatBlkDir, pflatEntry, pEntry, pszPath, cbComp))
  {
    /* name doesn't exist, let's initialize cbName and achName in case this
       file is about to be created */
    pEntry->cbName = cbComp;
    memcpy (pEntry->achName, pszPath, cbComp+1);
    return LOC_NOENTRY;
  }

  if (pEntry->fDOSattr & DOSATTR_DIRECTORY)
    return LOC_DIRENTRY;
  else
    return LOC_FILEENTRY;
}




int UtilAttrMatch (USHORT usPattern, USHORT usAttr)
{
  USHORT usMusthave;
  USHORT usMayhave;

  usMusthave = usPattern >> 8;
  usMayhave  = usPattern & 0x00FF;

  if (usMusthave == 0)
    usMayhave |= (DOSATTR_READONLY | DOSATTR_ARCHIVED);

  if (usAttr & ~usMayhave)
    return ERROR_FILE_NOT_FOUND;

  if (usMusthave & ~usAttr)
    return ERROR_FILE_NOT_FOUND;

  return NO_ERROR;
}




int UtilInsertEntry (PVOLUME pVolume, FLAT flatBlkDir, PDIRENTRY pEntry,
		     FLAT _ss *pflatEntry)
{
  ULONG cbEntry;
  ULONG ofsEntry;
  ULONG ofsOldBlkEnd;
  BLOCK BlkDir;

  VMReadBlk (&BlkDir, flatBlkDir);

#ifdef DEBUG
  if (*pflatEntry < BlkDir.flatAddr + DIR_DOTSSIZE  ||
      *pflatEntry > BlkDir.flatAddr + BlkDir.cbSize)
  {
    debugging = TRUE;
    DEBUG_PRINTF3 ("\r\n!!! UtilInsertEntry  0x%08lX <= 0x%08lX <= 0x%08lX\r\n",
		   BlkDir.flatAddr + DIR_DOTSSIZE,
		   *pflatEntry,
		   BlkDir.flatAddr + BlkDir.cbSize);
    INT3;
  }
#endif

  ofsEntry = *pflatEntry - BlkDir.flatAddr;
  cbEntry = sizeof(DIRENTRY)-CCHMAXPATHCOMP + pEntry->cbName;
  ofsOldBlkEnd = BlkDir.cbSize;

  /* grow the dir block */
  if (BlockReallocDir (pVolume, &BlkDir, BlkDir.cbSize + cbEntry))
    return ERROR_DISK_FULL;

  *pflatEntry = BlkDir.flatAddr + ofsEntry;

  /* insert the new entry */
  BlockDirCopy (pVolume, BlkDir.flatAddr + ofsEntry + cbEntry,
		BlkDir.flatAddr + ofsEntry, ofsOldBlkEnd - ofsEntry);
  VMWrite (*pflatEntry, pEntry, (unsigned short)cbEntry);
  VMWriteBlk (flatBlkDir, &BlkDir);

  return NO_ERROR;
}




void UtilDeleteEntry (PVOLUME pVolume, FLAT flatBlkDir, PDIRENTRY pEntry,
		      FLAT flatEntry)
{
  ULONG cbEntry;
  ULONG ofsEntry;
  ULONG ofsOldBlkEnd;
  FLAT  flatOldBlk;
  BLOCK BlkDir;

  VMReadBlk (&BlkDir, flatBlkDir);

#ifdef DEBUG
  if (flatEntry <  BlkDir.flatAddr + DIR_DOTSSIZE  ||
      flatEntry >= BlkDir.flatAddr + BlkDir.cbSize)
  {
    debugging = TRUE;
    DEBUG_PRINTF3 ("\r\n!!! UtilDeleteEntry  0x%08lX <= 0x%08lX < 0x%08lX\r\n",
		   BlkDir.flatAddr + DIR_DOTSSIZE,
		   flatEntry,
		   BlkDir.flatAddr + BlkDir.cbSize);
    INT3;
  }
#endif

  ofsEntry = flatEntry - BlkDir.flatAddr;
  cbEntry  = sizeof(DIRENTRY)-CCHMAXPATHCOMP + pEntry->cbName;
  flatOldBlk = BlkDir.flatAddr;
  ofsOldBlkEnd = BlkDir.cbSize;

  BlockDirCopy (pVolume, flatOldBlk+ofsEntry, flatOldBlk+ofsEntry+cbEntry,
		ofsOldBlkEnd-ofsEntry-cbEntry);
  if (BlockReallocDir (pVolume, &BlkDir, ofsOldBlkEnd-cbEntry))
  {
#ifdef DEBUG
    debugging = TRUE;
    DEBUG_PRINTF0 ("\r\n!!! UtilDeleteEntry  Realloc error too difficult to handle\r\n");
    INT3; /* don't do this in release, it's better to get garbage than traps!!!! */
#endif
    return;
  }
  VMWriteBlk (flatBlkDir, &BlkDir);
}





void UtilEnterRamfs (void)
{
  while (InRamfs)
  {
    DEBUG_PRINTF0 ("\r\n\r\n>>> UtilEnterRamfs Conflict\r\n");

    /* Sleep for one tick */
    _asm
    {
      mov ax, 0xFEDE
      mov bx, 0xEBBE
      sub di, di
      mov cx, 1
      mov dx, 0x104
    }
    (*DevHlp)();
  }
  InRamfs = TRUE;
}



void UtilExitRamfs (void)
{
#ifdef DEBUG
  if (!InRamfs)
  {
    debugging = TRUE;
    DEBUG_PRINTF0 ("\r\n\a!!! Bad UtilExitRamfs\r\n");
    INT3;
  }
#endif

  InRamfs = FALSE;
}
