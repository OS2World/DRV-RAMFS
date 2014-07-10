#ifndef __COMPACT__
  #error Please use Compact memory model!
#endif

/* DIRENTRY.fDOSattr flags */
#define DOSATTR_READONLY	0x01
#define DOSATTR_HIDDEN		0x02
#define DOSATTR_SYSTEM		0x04
#define DOSATTR_DIRECTORY	0x10
#define DOSATTR_ARCHIVED	0x20
#define DOSATTR_NON83		0x40
#define DOSATTR_NEEDEA		0x80

#define SHARE_DENYREAD		0x01
#define SHARE_DENYWRITE		0x02
#define ACCESS_READ		0x01
#define ACCESS_WRITE		0x02

/* SEARCH.flatEntry if no more entries */
#define SEARCH_END		0xFFFFFFFFul

typedef unsigned long FLAT;


/* forwards */
typedef struct _BLOCK    BLOCK;
typedef struct _FBLOCK   FBLOCK;
typedef struct _OPENFILE OPENFILE;
typedef struct _CURDIR   CURDIR;
typedef struct _SEARCH   SEARCH;
typedef struct _VOLUME   VOLUME;
typedef struct _DIRENTRY_HDR DIRENTRY_HDR;
typedef struct _DIRENTRY DIRENTRY;

typedef BLOCK    _ss *PBLOCK;
typedef FBLOCK   _ss *PFBLOCK;
typedef OPENFILE far *POPENFILE;
typedef CURDIR   far *PCURDIR;
typedef SEARCH   far *PSEARCH;
typedef VOLUME   far *PVOLUME;
typedef DIRENTRY far *PDIRENTRY;


struct _BLOCK
{
  FLAT		flatAddr;
  ULONG		cbSize;
};

/* File data block - incorporates an array of FLAT pointers to individual
   clusters */

struct _FBLOCK
{
  struct _BLOCK clusters;               /* Cluster array = block of FLATs */
  ULONG		fSize;                  /* Size of the entire file */
};

/* we allocate a OPENFILE on the near heap for each struct sffsd */
/* size of a OPENFILE is sizeof(OPENFILE) + strlen(szName) */
struct _OPENFILE
{
  POPENFILE pNextOpenfile;
  FLAT      flatEntry;
  UCHAR     fShare;
  CHAR      szName[1];		/* asciiz name - for FS_FSCTL */
};


/* we allocate a CURDIR on the near heap for each struct cdfsd */
/* size of a CURDIR is sizeof(CURDIR) + strlen(szDir) */
struct _CURDIR
{
  PCURDIR   pNextCurdir;
  PVOLUME   pVolume;		/* only used by FS_CHDIR, flag=2 */
  FLAT      flatBlkDir;
  CHAR      szDir[1];		/* full asciiz dir name - for FS_FSCTL */
};


/* we allocate a SEARCH on the near heap for each struct fsfsd */
/* size of a SEARCH is sizeof(SEARCH) + strlen(szPattern) */
struct _SEARCH
{
  PSEARCH   pNextSearch;
  FLAT      flatBlkDir;
  FLAT      flatEntry;
  FLAT      flatFallbackEntry;  /* HACK for handling deleted files with
                                   findnext_resume -- 29/3/2006, 22:23 */
  USHORT    usAttr;
  CHAR      szPattern[1];	/* uppercased asciiz */
};


/* OS/2 allocates struct vpfsd's */
struct	vpfsd {
  PVOLUME   pVolume;
  char      vpd_unused[34];
};  /* vpfsd   36 bytes total */


/* we allocate a VOLUME on the near heap for each struct vpfsd */
struct _VOLUME
{
  BLOCK     blkRootDir;
  FLAT      flatBlkRootDir;
  POPENFILE pFirstOpenfile;
  PCURDIR   pFirstCurdir;
  PSEARCH   pFirstSearch;
  SHORT     sAttached;
  CHAR      szLabel [VPBTEXTLEN];
  ULONG     datiCreate;         /* for QueryPathInfo requests */
};


/* OS/2 allocates struct sffsd's */
struct  sffsd {
  POPENFILE pOpenfile;
  char      sfd_unused[28];
};  /* sffsd   30 bytes total */


/* OS/2 allocates struct cdfsd's */
struct cdfsd {
  PCURDIR   pCurdir;
  char      cdd_unused[6];
};  /* cdfsd   8 bytes total */


/* OS/2 allocates struct fsfsd's */
struct fsfsd {
  PSEARCH   pSearch;
  char      fsd_unused[22];
};  /* fsfsd   24 bytes total */


/* size of a DIRENTRY is sizeof(DIRENTRY)-CCHMAXPATHCOMP + DIRENTRY.cbName */
#define _direntry_hdr_members \
  UCHAR		cbName; \
  UCHAR		fDOSattr; \
  FBLOCK        fblkFile; \
  BLOCK		blkEA

struct _DIRENTRY_HDR
{
  _direntry_hdr_members;
};

struct _DIRENTRY
{
  _direntry_hdr_members;
  ULONG		datiCreate;     /* date=low word, time=high word */
  ULONG		datiAccess;	/* date=low word, time=high word */
  ULONG		datiWrite;	/* date=low word, time=high word */
  CHAR		achName[CCHMAXPATHCOMP];
				/* not zero terminated while on disk */
				/* zero terminated while in RAM */
};

PVOLUME scavenge_volume(PVOLUME pVolume);
