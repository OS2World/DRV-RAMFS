#include "includes.h"

#include <stdlib.h>
#include <string.h>

ULONG _pascal FS_ATTRIBUTE = FSA_REMOTE;
CHAR  _pascal FS_NAME[] = "RAMFS";

extern unsigned short threednow;        /* 3DNow! trigger */
#ifdef DEBUG
 extern unsigned short debug_port;      /* COM port used for debugging */
#endif
extern unsigned long ramfs_units;       /* # of allocation units to report
                                           as free space */
extern unsigned int verify_memory;      /* Verify 16-bit segments prior
                                           to hitting them */
int han_zi = 0;                         /* We would require BIG-5 or Latin-2
                                           to transcribe the comment, sorry */
int findnext_resume = 0;                /* Experimental search resumption
                                           hack for DosFindNext */

static char hello_msg[] = "RAMFS.IFS version 1.21\r\n";

/* Helper routine to parse the cluster ranges from command line */

#define isdigit(c) ((c) >= '0' && (c) <= '9')
static void parse_cluster_ranges (char *p)
{
  unsigned int i = 0;

  while (!isdigit(*p))
    p++;
  while (1)
  {
    /* Parse the block size */
    blk_size[i].multiplier=atol(p);
    while (isdigit(*p))
      p++;
    if (blk_size[i].multiplier < MIN_MULTIPLIER)
      blk_size[i].multiplier = MIN_MULTIPLIER;
    else if (blk_size[i].multiplier > MAX_MULTIPLIER)
      blk_size[i].multiplier = MAX_MULTIPLIER;
    /* Has a count been specified? */
    if (*p != 'x')
      break;
    while (!isdigit(*p))
      p++;
    blk_size[i].count = atol(p);
    while (isdigit(*p))
      p++;
    if (blk_size[i].count < 1)
      blk_size[i].count = 1;
    /* Is there another valid entry? */
    if (i >= MAX_CLUSTER_RANGES - 1 || *p!=':' || !isdigit(*(p + 1)))
      break;
    i++;
    p++;
  }
  /* Mark up the terminating entry */
  blk_size[i].count = 0;
}


/* Main entry point */


APIRET EXPENTRY FS_INIT (
    PSZ		szParm,
    ULONG	pDevHlp,
    PULONG	pMiniFSD )	/* not used */
{
  int rc;
  USHORT wlen;
  int   quiet_init = 0;
  char  *p;
  unsigned long vs;
  int vs_mul = -12;
  char params[512];

  UtilEnterRamfs();

  if (szParm != NULL)
  {
    strncpy(params, szParm, sizeof(params)-1);
    params[sizeof(params)-1]='\0';
    strlwr(params);

    /* Force/disable 3DNow! */
    if (strstr(params, "/3"))
      threednow = 1;
    else if (strstr(params, "/!3"))
      threednow = 0;

    /* Quiet initialization */
    if (strstr(params, "/q"))
       quiet_init = 1;

    /* Do not check the memory access */
    if (strstr(params, "/!vm"))
       verify_memory = 0;

    /* Resident mode */
    if (strstr(params, "/r"))
       alloc_flags &= 0x00000004;

    /* Chinese mode (cancel the kernel fix for DBCS UNC path separation) */
    if (strstr(params, "/¤¤¤å")  ||  strstr(params, "/dbcs"))
       han_zi = 1;

    /* DosFindNext resumption hack */
    if (strstr(params, "/findnext_resume"))
       findnext_resume = 1;

    /* Specify cluster ranges */
    if ((p = strstr(params, "/clu:")) != NULL)
       parse_cluster_ranges(p + 5);

    /* Readjust size */
    if ((p = strstr(params, "/s:")) != NULL)
    {
       vs = atol(p+3);
       if (vs != 0)
       {
         while (*++p != '\0' && *p != ' ' && *p != '/')
         {
           if (*p == 'k')
             vs_mul = -2;
           else if (*p == 'm')
             vs_mul = 8;
           else if (*p == 'g')
             vs_mul = 18;
           else if (*p == 't')
             vs_mul = 28;
         }
       }
       ramfs_units = (vs_mul > 0) ? (vs << vs_mul) : (vs >> (-vs_mul));
    }

#ifdef MAX_HEAP
    if ((p = strstr(params, "/m:")) != NULL)
    {
       unsigned long vs2;
       vs = atol(p+3);
       if (vs != 0)
       {
         vs_mul = 0;
         while (*++p != '\0' && *p != ' ' && *p != '/')
         {
           if (*p == 'k')
             vs_mul = 10;
           else if (*p == 'm')
             vs_mul = 20;
           else if (*p == 'g')
             vs_mul = 30;
         }
         vs2 = vs << vs_mul;
         if (   vs2 >= vs
             && vs2 <= 0xc0000000
             && vs2 >= 1024*1024)
           gcbHeapMax = (vs << vs_mul);
       }
    }
#endif

    /* Debug output port */
#ifdef DEBUG
    if ((p = strstr(params, "/com:")) != NULL)
    {
       debug_port = (unsigned short)hextoi(p + 5);
       if (debug_port == 0)
         debug_port = 0x2F8;
    }
#endif

  }

  /* AAB 08/06/2003 - moved here so we know which port to use for debugging */
  DEBUG_PRINTF1 ("FS_INIT  szParm='%s'", szParm);

  /* show hello message */
  if (!quiet_init)
    DosWrite (1, hello_msg, sizeof(hello_msg)-1, &wlen);

  DevHlp = (PFN) pDevHlp;
  _asm{
   mov al, 1	        /* get global info seg */
   mov dl, 0x24;	/* DevHlp_GetDOSVar */
  }
  (*DevHlp)();
  _asm{
   push es
   mov es, ax
   mov ax, es:[bx]
   mov word ptr pGiseg, 0
   mov word ptr pGiseg+2, ax
   pop es
  }

  NearInit();
  VMInit();

  rc = NO_ERROR;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
