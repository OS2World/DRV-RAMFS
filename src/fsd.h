/*static char *SCCSID = "@(#)fsd.h	6.5 91/12/16";*/
/*  fsd.h   - File system driver entry point declarations */

/*  FS_ATTRIBUTE bit field values */

#define FSA_REMOTE  0x00000001	/* Set if REMOTE FSD		      */
#define FSA_UNC	    0x00000002	/* Set if FSD implements UNC support  */
#define FSA_LOCK    0x00000004	/* Set if FSD needs lock notification */
#define FSA_LVL7    0x00000008	/* Set if FSD will accept Level 7     */
     /* DosQueryPathInfo requests - These are requests for the case-  */
     /* perserved "path" of the input file/path name.		      */
#define FSA_PSVR    0x00000010	/* Set if FSD manages Remote NmPipes  */

#define CDDWORKAREASIZE 8
#define SFDWORKAREASIZE 30
#define VPDWORKAREASIZE 36


/* Volume Parameter structures */

#define VPBTEXTLEN 12

struct	vpfsi {
    ULONG	vpi_vid;		/* 32 bit volume ID */
    ULONG	vpi_hDEV;		/* handle to device driver */
    USHORT	vpi_bsize;		/* sector size in bytes */
    ULONG	vpi_totsec;		/* total number of sectors */
    USHORT	vpi_trksec;		/* sectors / track */
    USHORT	vpi_nhead;		/* number of heads */
    CHAR	vpi_text[VPBTEXTLEN];	/* volume name */
    PVOID	vpi_pDCS;		/* device capability struc */
    PVOID	vpi_pVCS;		/* volume characteristics struc */
    UCHAR	vpi_drive;		/* drive (0=A) */
    UCHAR	vpi_unit;		/* unit code */
};  /* vpfsi */


/*
 * Predefined volume IDs: [note - keep this list in sync with list in
 * dos/dosinc/vpb.inc!]
 */
/* Unique ID for vpb_ID field for unreadable media. */
#define UNREAD_ID  0x534E4A52L		/* Stored as (bytes) 0x52,4A,4E,53. */

/*
 * Unique ID for vpb_ID field for damaged volume (recognized by IFS but cannot
 * be normally mounted).
 */
#define DAMAGED_ID 0x0L			/* Stored as (bytes) 0,0,0,0. */

#define CCHMAXPATH	260
#define CCHMAXPATHCOMP	256

/* Current Directory structures */

struct cdfsi {
    USHORT	cdi_hVPB;		/* VPB handle for associated device */
    USHORT	cdi_end;		/* end of root portion of curdir */
    CHAR	cdi_flags;		/* flags indicating state of cdfsd */
    CHAR	cdi_curdir[CCHMAXPATH];	/* text of current directory */
};  /* cdfsi */


/* bit values for cdi_flags (state of cdfsd structure */

#define CDI_ISVALID	0x80		/* format is known */
#define CDI_ISROOT	0x40		/* cur dir == root */
#define CDI_ISCURRENT	0x20


/* Per open-instance (System File) structures */

struct	sffsi {
    ULONG	sfi_mode;		/* access/sharing mode */
    USHORT	sfi_hVPB;		/* volume info. */
    USHORT	sfi_ctime;		/* file creation time */
    USHORT	sfi_cdate;		/* file creation date */
    USHORT	sfi_atime;		/* file access time */
    USHORT	sfi_adate;		/* file access date */
    USHORT	sfi_mtime;		/* file modification time */
    USHORT	sfi_mdate;		/* file modification date */
    ULONG	sfi_size;		/* size of file */
    ULONG	sfi_position;		/* read/write pointer */
    /* the following may be of use in sharing checks */
    USHORT	sfi_UID;		/* user ID of initial opener */
    USHORT	sfi_PID;		/* process ID of initial opener */
    USHORT	sfi_PDB;		/* PDB (in 3.x box) of initial opener */
    USHORT	sfi_selfsfn;		/* system file number of file instance */
    UCHAR	sfi_tstamp;		/* update/propagate time stamp flags */
					/* use with ST_Sxxx and ST_Pxxx */
    USHORT	sfi_type;		/* use with STYPE_ */
    ULONG	sfi_pPVDBFil;		/* performance counter data block pointer */
    UCHAR	sfi_DOSattr;		/* DOS file attributes  D/S/A/H/R  */
};  /* sffsi */


/* sfi_tstamps flags */
#define ST_SCREAT	 1		/* stamp creation time */
#define ST_PCREAT	 2		/* propagate creation time */
#define ST_SWRITE	 4		/* stamp last write time */
#define ST_PWRITE	 8		/* propagate last write time */
#define ST_SREAD	16		/* stamp last read time */
#define ST_PREAD	32		/* propagate last read time */

/* sfi_type flags */
#define STYPE_FILE	 0		/* file */
#define STYPE_DEVICE	 1		/* device */
#define STYPE_NMPIPE	 2		/* named pipe */
#define STYPE_FCB	 4		/* fcb sft */


/* file system independent - file search parameters */
struct fsfsi {
    USHORT	fsi_hVPB;	/* volume info. */
};  /* fsfsi */



/* file system dependent - device information */
struct devfsd {
    ULONG	FSDRef;	/* Reference obtained from FSD during ATTACH */
};  /* devfsd */

struct filelock {
    ULONG	FileOffset;		/* offset where the lock/unlock begins */
    ULONG	RangeLength;		/* length of region locked/unlocked */
};


/*****
 *
 * union and structures for FS_FSCTL
 */
/* pArgType == 1, FileHandle directed case */
struct SF {
    struct sffsi FAR *psffsi;
    struct sffsd FAR *psffsd;
};  /* SF */

/* pArgType == 2, PathName directed case */
struct CD {
    struct cdfsi FAR *pcdfsi;
    struct cdfsd FAR *pcdfsd;
    PCHAR	pPath;
    USHORT	iCurDirEnd;
};  /* CD */

union argdat {
    /* pArgType == 1, FileHandle directed case */
    struct SF sf;

    /* pArgType == 2, PathName directed case */
    struct CD cd;

    /* pArgType == 3, FSD Name directed case */
    /* nothing */
};  /* argdat */


/*****
 *
 * Union and structures for FS_NMPIPE
 *
 */

/* Get/SetPHandState parameter block */
struct	phs_param {
    SHORT	phs_len;
    SHORT	phs_dlen;
    SHORT	phs_pmode;	/* pipe mode set or returned */
};


/* DosQNmPipeInfo parameter block,
 * data is info. buffer addr */
struct	npi_param {
    SHORT	npi_len;
    SHORT	npi_dlen;
    SHORT	npi_level;	/* information level desired */
};


/* DosRawReadNmPipe parameters,
 * data is buffer addr */
struct	npr_param {
    SHORT	npr_len;
    SHORT	npr_dlen;
    SHORT	npr_nbyt;	/* number of bytes read */
};

/* DosRawWriteNmPipe parameters,
 * data is buffer addr */
struct	npw_param {
    SHORT	npw_len;
    SHORT	npw_dlen;
    SHORT	npw_nbyt;	/* number of bytes written */
};

/* NPipeWait parameters */
struct	npq_param {
    SHORT	npq_len;
    SHORT	npq_dlen;
    LONG	npq_timeo;	/* timeout in milliseconds */
    SHORT	npq_prio;	/* priority of caller */
};

/* DosCallNmPipe parameters,
 * data is in-buffer addr */
struct	npx_param {
    SHORT	npx_len;
    USHORT	npx_ilen;		/* length of in-buffer */
    PCHAR	npx_obuf;		/* pointer to out-buffer */
    USHORT	npx_olen;		/* length of out-buffer */
    USHORT	npx_nbyt;		/* number of bytes read */
    LONG	npx_timeo;		/* timeout in milliseconds */
};

/* PeekPipe parameters, data is buffer addr */
struct	npp_param {
    SHORT	npp_len;
    USHORT	npp_dlen;
    USHORT	npp_nbyt;		/* number of bytes read */
    USHORT	npp_avl0;		/* bytes left in pipe */
    USHORT	npp_avl1;		/* bytes left in current msg */
    USHORT	npp_state;		/* pipe state */
};

/* DosTransactNmPipe parameters,
 * data is in-buffer addr */
struct	npt_param {
    SHORT	npt_len;
    USHORT	npt_ilen;		/* length of in-buffer */
    PCHAR	npt_obuf;		/* pointer to out-buffer */
    USHORT	npt_olen;		/* length of out-buffer */
    USHORT	npt_nbyt;		/* number of bytes read */
};

/* QNmpipeSemState parameter block,
 * data is user data buffer */
struct	qnps_param {
    USHORT	qnps_len;		/* length of parameter block */
    USHORT	qnps_dlen;		/* length of supplied data block */
    LONG	qnps_semh;		/* system semaphore handle */
    USHORT	qnps_nbyt;		/* number of bytes returned */
};

/* ConnectPipe parameter block, no data block */
struct	npc_param {
    USHORT	npc_len;		/* length of parameter block */
    USHORT	npc_dlen;		/* length of data block */
};

/* DisconnectPipe parameter block, no data block */
struct	npd_param {
    USHORT	npd_len;		/* length of parameter block */
    USHORT	npd_dlen;		/* length of data block */
};

union npoper {
    struct phs_param    phs;
    struct npi_param    npi;
    struct npr_param    npr;
    struct npw_param    npw;
    struct npq_param    npq;
    struct npx_param    npx;
    struct npp_param    npp;
    struct npt_param    npt;
    struct qnps_param   qnps;
    struct npc_param    npc;
    struct npd_param    npd;
};	/* npoper */


/*****
*
* Declarations for FS_DOPAGEIO entry point.
*
*/

struct PageCmd {
    UCHAR	Cmd;		/* Cmd code for Read, Write, WriteVerify */
    UCHAR	Priority;	/* Same values as for request packets */
    UCHAR	Status;		/* Status byte */
    UCHAR	Error;		/* I24 error code */
    ULONG	Addr;		/* Physical (32 bit) or Virtual (16:16) */
    ULONG	FileOffset;	/* Byte offset in page file */
				/* (on page boundary) */
};

struct PageCmdHeader {
    UCHAR	InFlags;		/* Input flags */
    UCHAR	OutFlags;		/* Output flags - must be 0 on entry */
    UCHAR	OpCount;		/* Number of operations */
    UCHAR	Pad;			/* Preserve DWORD alignment */
    ULONG	Reserved1;		/* unused */
    ULONG	Reserved2;		/* unused */
    ULONG	Reserved3;		/* unused */
    struct PageCmd PageCmdList [1];	/* The actual commands */
};

/* FSD_DoPageIO InFlags values */
#define PGIO_FI_ORDER   0x01    /* Force Order of operations */

/* FSD_DoPageIO OutFlags values */
#define PGIO_FO_DONE    0x01    /* Operation done */
#define PGIO_FO_ERROR   0x02    /* Operation failed */

/* FSD_DoPageIO Status values */
#define PGIO_ATTEMPTED  0x0f    /* Operation attempted */
#define PGIO_FAILED	    0xf0    /* Operation failed */

/*****
 *
 * Declarations for the FSD entry points.
 *
 */

/* bit values for the IOflag parameter in various FS_ entry points */
#define IOFL_WRITETHRU	    0x10	/* Write through bit	*/
#define IOFL_NOCACHE	    0x20	/* No Cache bit		*/

/* values for flag in FS_ATTACH */
#define FSA_ATTACH		0x00
#define FSA_DETACH		0x01
#define FSA_ATTACH_INFO		0x02

/* values for flag in FS_CHDIR */
#define CD_EXPLICIT		0x00
#define CD_VERIFY		0x01
#define CD_FREE			0x02

/* values for type in FS_CHGFILEPTR */
#define CFP_RELBEGIN		0x00
#define CFP_RELCUR		0x01
#define CFP_RELEND		0x02

/* values for type in FS_CLOSE */
#define FS_CL_ORDINARY	0 /* ordinary close */
#define FS_CL_FORPROC	1 /* final close for the process */
#define FS_CL_FORSYS	2 /* final close for the system (for all processes) */

/* values for commit type in FS_COMMIT */
#define FS_COMMIT_ONE	1	/* commit for a single file */
#define FS_COMMIT_ALL	2	/* commit due to buf reset - for all files */

/* values for flag in FS_FILEATTRIBUTE */
#define FA_RETRIEVE		0x00
#define FA_SET			0x01

/* values for flag in FS_FILEINFO */
#define FI_RETRIEVE		0x00
#define FI_SET			0x01

/* values for flag in FS_FINDFIRST, FS_FINDFROMNAME, FS_FINDNEXT */
#define FF_NOPOS		0x00
#define FF_GETPOS		0X01

/* values for flag in FS_FLUSH */
#define FLUSH_RETAIN		0x00
#define FLUSH_DISCARD		0x01

/* values for iArgType in FS_FSCTL */
#define FSCTL_ARG_FILEINSTANCE	0x01
#define FSCTL_ARG_CURDIR	0x02
#define FSCTL_ARG_NULL		0x03

/* values for func in FS_FSCTL */
#define FSCTL_FUNC_NONE		0x00
#define FSCTL_FUNC_NEW_INFO	0x01
#define FSCTL_FUNC_EASIZE	0x02

/* values for flag in FS_FSINFO */
#define INFO_RETREIVE		0x00
#define INFO_SET		0x01

/* values for flag in FS_MOUNT */
#define MOUNT_MOUNT		0x00
#define MOUNT_VOL_REMOVED	0x01
#define MOUNT_RELEASE		0x02
#define MOUNT_ACCEPT		0x03

/* Values for OpType in FS_NMPIPE   */
#define	    NMP_GetPHandState	    0x21
#define	    NMP_SetPHandState	    0x01
#define	    NMP_PipeQInfo	    0x22
#define	    NMP_PeekPipe	    0x23
#define	    NMP_ConnectPipe	    0x24
#define	    NMP_DisconnectPipe	    0x25
#define	    NMP_TransactPipe	    0x26
#define	    NMP_READRAW		    0x11
#define	    NMP_WRITERAW	    0x31
#define	    NMP_WAITPIPE	    0x53
#define	    NMP_CALLPIPE	    0x54
#define	    NMP_QNmPipeSemState	    0x58

/* value for *pfgenflag in FS_OPENCREATE*/
#define FOC_NEEDEAS	0x01	/* there are need eas for this file */

/* values for *pFlags in FS_OPENPAGEFILE */
#define PGIO_FIRSTOPEN 0x00000001   /* first open of page file	      */
#define PGIO_PADDR     0x00004000   /* physical addresses required    */
#define PGIO_VADDR     0x00008000   /* 16:16 virtual address required */

/* values for flag in FS_PATHINFO */
#define PI_RETRIEVE		0x00
#define PI_SET			0x01

/* values for usType in FS_SHUTDOWN */
#define SD_BEGIN		0x00
#define SD_COMPLETE		0x01

/* values for flag in FS_PATHINFO */
/* These values depend on code in GetUNCFSDFromPath */
/* Don't change values without also changing code */
#define VUN_PASS1		 0x0000
#define VUN_PASS2		 0x0001
#define ERROR_UNC_PATH_NOT_FOUND 0x0003	  /* equals ERROR_PATH_NOT_FOUND */



/* prototypes for all FS_* functions */

APIRET EXPENTRY FS_ALLOCATEPAGESPACE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    ULONG	lSize,
    ULONG	lWantContig );

APIRET EXPENTRY FS_ATTACH (
    USHORT	flag,
    PSZ		pDev,
    struct vpfsd *pvpfsd,
    struct cdfsd *pcdfsd,
    PCHAR	pParm,
    PUSHORT	pLen );

APIRET EXPENTRY FS_CANCELLOCKREQUEST (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct filelock pLockRange );

APIRET EXPENTRY FS_CHDIR (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pDir,
    USHORT	iCurDirEnd );

APIRET EXPENTRY FS_CHGFILEPTR (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    LONG	offset,
    USHORT	type,
    USHORT	IOflag );

APIRET EXPENTRY FS_CLOSE (
    USHORT	type,
    USHORT	IOflag,
    struct sffsi *psffsi,
    struct sffsd *psffsd );

APIRET EXPENTRY FS_COMMIT (
    USHORT	type,
    USHORT	IOflag,
    struct sffsi *psffsi,
    struct sffsd *psffsd );

APIRET EXPENTRY FS_COPY (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pSrc,
    USHORT	iSrcCurDirEnd,
    PSZ		pDst,
    USHORT	iDstCurDirEnd,
    USHORT	nameType );

APIRET EXPENTRY FS_DELETE (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		 pFile,
    USHORT	 iCurDirEnd );

APIRET EXPENTRY FS_DOPAGEIO (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct PageCmdHeader *pPageCmdList );

VOID EXPENTRY FS_EXIT (
    USHORT	uid, 
    USHORT	pid, 
    USHORT	pdb );

APIRET EXPENTRY FS_FILEATTRIBUTE (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    PUSHORT	pAttr );

APIRET EXPENTRY FS_FILEINFO (
    USHORT	flag,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	level,
    PCHAR	pData,
    USHORT	cbData,
    USHORT	IOflag );

APIRET EXPENTRY FS_FILEIO (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pCmdList,	/* not actually PCHAR */
    USHORT	cbCmdList,
    PUCHAR	poError,
    USHORT	IOflag );

APIRET EXPENTRY FS_FILELOCKS (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct filelock *pUnLockRange,
    struct filelock *pLockRange,
    ULONG	timeout,
    ULONG	flags );

APIRET EXPENTRY FS_FINDCLOSE (
    struct fsfsi *pfsfsi, 
    struct fsfsd *pfsfsd );

APIRET EXPENTRY FS_FINDFIRST (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	attr,
    struct fsfsi *pfsfsi,
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    USHORT	flags );

APIRET EXPENTRY FS_FINDFROMNAME (
    struct fsfsi *pfsfsi,
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    ULONG	position,
    PSZ		pName,
    USHORT	flags );

APIRET EXPENTRY FS_FINDNEXT (
    struct fsfsi *pfsfsi,
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    USHORT	flags );

APIRET EXPENTRY FS_FLUSHBUF (
    USHORT hVPB, 
    USHORT flag );

APIRET EXPENTRY FS_FSCTL (
    union argdat *pArgDat,
    USHORT	iArgType,
    USHORT	func,
    PCHAR	pParm,
    USHORT	lenParm,
    PUSHORT	plenParmIO,
    PCHAR	pData,
    USHORT	lenData,
    PUSHORT	plenDataIO );

APIRET EXPENTRY FS_FSINFO (
    USHORT	flag,
    USHORT	hVPB,
    PCHAR	pData,
    USHORT	cbData,
    USHORT	level );

APIRET EXPENTRY FS_INIT (
    PSZ		szParm, 
    ULONG	pDevHlp, 
    PULONG	pMiniFSD );

APIRET EXPENTRY FS_IOCTL (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	cat,
    USHORT	func,
    PCHAR	pParm,
    USHORT	lenMaxParm,
    PUSHORT	plenInOutParm,
    PCHAR	pData,
    USHORT	lenMaxData,
    PUSHORT	plenInOutData );

APIRET EXPENTRY FS_MKDIR (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    PEAOP	pEABuf,
    USHORT	flags );

APIRET EXPENTRY FS_MOUNT (
    USHORT	flag,
    struct vpfsi *pvpfsi,
    struct vpfsd *pvpfsd,
    USHORT	hVPB,
    PCHAR	pBoot );

APIRET EXPENTRY FS_MOVE (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pSrc,
    USHORT	iSrcCurDirEnd,
    PSZ		pDst,
    USHORT	iDatCurDirEnd,
    USHORT	flags );

APIRET EXPENTRY FS_NEWSIZE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    ULONG	len,
    USHORT	IOflag );

APIRET EXPENTRY FS_NMPIPE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	OpType,
    union npoper *pOpRec,
    PCHAR	pData,
    PSZ		pName );

APIRET EXPENTRY FS_FINDNOTIFYCLOSE (
    USHORT	handle );

APIRET EXPENTRY FS_FINDNOTIFYFIRST (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	attr,
    PUSHORT	pHandle,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    ULONG	timeout );

APIRET EXPENTRY FS_FINDNOTIFYNEXT (
    USHORT	handle,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	infolevel,
    ULONG	timeout );

APIRET EXPENTRY FS_OPENCREATE (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    ULONG	ulOpenMode,
    USHORT	usOpenFlag,
    PUSHORT	pusAction,
    USHORT	usAttr,
    PEAOP	pEABuf,
    PUSHORT	pfgenflag );

APIRET EXPENTRY FS_OPENPAGEFILE (
    PULONG	pFlags,
    PULONG	pcMaxReq,
    PSZ		pName,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	usOpenMode,
    USHORT	usOpenFlag,
    USHORT	usAttr,
    ULONG	Reserved );

APIRET EXPENTRY FS_PATHINFO (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	level,
    PCHAR	pData,
    USHORT	cbData );

APIRET EXPENTRY FS_PROCESSNAME (
    PSZ		pNameBuf );

APIRET EXPENTRY FS_READ (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pData,
    PUSHORT	pLen,
    USHORT	IOflag );

APIRET EXPENTRY FS_RMDIR (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd );

APIRET EXPENTRY FS_SETSWAP (
    struct sffsi *psffsi, 
    struct sffsd *psffsd );

APIRET EXPENTRY FS_SHUTDOWN (
    USHORT	usType,
    ULONG	ulReserved );

APIRET EXPENTRY FS_VERIFYUNCNAME (
    USHORT	flag,
    PSZ		pName );

APIRET EXPENTRY FS_WRITE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pData,
    PUSHORT	pLen,
    USHORT	IOflag );
