#define INT3	_asm{int 3}

/* return values for UtilLocate() */
#define LOC_NOPATH	0
#define LOC_DIRENTRY	1
#define LOC_NOENTRY	2
#define LOC_FILEENTRY	3

/* size of the "." and ".." entries that are at the beginning of a dir */
#define DIR_DOTSSIZE	(FIELDOFFSET(DIRENTRY,achName)+1 + FIELDOFFSET(DIRENTRY,achName)+2)

/* round up to nearest 4K multiple */
#define ROUNDUP(a)  (((a) + 0x00000FFFlu) & 0xFFFFF000lu)

extern USHORT   _pascal DOS32FLATDS;
extern PFN near DevHlp;
extern PGINFOSEG near pGiseg;
extern unsigned long alloc_flags;


#ifdef DEBUG

  extern char debugging;

  int _cdecl debug_printf (const char _ds *format, ...);
  unsigned int hextoi (const char _ds *str);

  #define DEBUG_PRINTF0(fmt)              debug_printf (fmt)
  #define DEBUG_PRINTF1(fmt,a)            debug_printf (fmt,a)
  #define DEBUG_PRINTF2(fmt,a,b)          debug_printf (fmt,a,b)
  #define DEBUG_PRINTF3(fmt,a,b,c)        debug_printf (fmt,a,b,c)
  #define DEBUG_PRINTF4(fmt,a,b,c,d)      debug_printf (fmt,a,b,c,d)
  #define DEBUG_PRINTF5(fmt,a,b,c,d,e)    debug_printf (fmt,a,b,c,d,e)
  #define DEBUG_PRINTF6(fmt,a,b,c,d,e,f)  debug_printf (fmt,a,b,c,d,e,f)

#else

  #define DEBUG_PRINTF0(fmt)
  #define DEBUG_PRINTF1(fmt,a)
  #define DEBUG_PRINTF2(fmt,a,b)
  #define DEBUG_PRINTF3(fmt,a,b,c)
  #define DEBUG_PRINTF4(fmt,a,b,c,d)
  #define DEBUG_PRINTF5(fmt,a,b,c,d,e)
  #define DEBUG_PRINTF6(fmt,a,b,c,d,e,f)

#endif


ULONG UtilGetDateTime (void);

int  UtilLocate (FLAT _ss *pflatBlkDir, FLAT _ss *pflatEntry, PDIRENTRY pEntry,
		 PSZ pszPath);

int  UtilAttrMatch (USHORT usPattern, USHORT usAttr);

int  UtilInsertEntry (PVOLUME pVolume, FLAT flatBlkDir, PDIRENTRY pEntry,
		      FLAT _ss *pflatEntry);

void UtilDeleteEntry (PVOLUME pVolume, FLAT flatBlkDir, PDIRENTRY pEntry,
		      FLAT flatEntry);

void UtilEnterRamfs (void);
void UtilExitRamfs  (void);
