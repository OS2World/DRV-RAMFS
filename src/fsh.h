/*static char *SCCSID = "@(#)fsh.h	6.5 91/10/20";*/
/*  fsh.h   - FSH_ = fshelper interface declarations */

/*
 *  FSH_DOVOLIO2 omits flag definition
 */

APIRET APIENTRY FSH_ADDSHARE (
    PSZ		pName,
    USHORT	mode,
    USHORT	hVPB,
    PULONG	phShare );

APIRET APIENTRY FSH_CALLDRIVER (
    PVOID	pPkt,
    USHORT	hVPB,
    USHORT	fControl );

APIRET APIENTRY FSH_CANONICALIZE (
    PSZ		pPathName,
    USHORT	cbPathBuf,
    PSZ		pPathBuf,
    PUSHORT 	pFlags );

APIRET APIENTRY FSH_CHECKEANAME (
    USHORT	iLevel,
    ULONG	cbEAName,
    PSZ		szEAName );

APIRET APIENTRY FSH_CRITERROR (
    USHORT	cbMessage,
    PSZ		pMessage,
    USHORT	nSubs,
    PSZ		pSubs,
    USHORT	fAllowed );

/*  Flags for fAllowed
 */
#define CE_ALLFAIL	0x0001		/*  FAIL allowed		      */
#define CE_ALLABORT	0x0002		/*  ABORT allowed		      */
#define CE_ALLRETRY	0x0004		/*  RETRY allowed		      */
#define CE_ALLIGNORE	0x0008		/*  IGNORE allowed		      */
#define CE_ALLACK	0x0010		/*  ACK allowed			      */

/*  Return values from FSH_CRITERR
 */
#define CE_RETIGNORE	0x0000		/*  User said IGNORE		      */
#define CE_RETRETRY	0x0001		/*  User said RETRY		      */
#define CE_RETFAIL	0x0003		/*  User said FAIL/ABORT	      */
#define CE_RETACK	0x0004		/*  User said continue		      */

APIRET APIENTRY FSH_DEVIOCTL (
    USHORT	flag,
    ULONG	hDev,
    USHORT	sfn,
    USHORT	cat,
    USHORT	func,
    PVOID	pParm,
    USHORT	cbParm,
    PVOID	pData,
    USHORT	cbData );

APIRET APIENTRY FSH_DOVOLIO (
    USHORT	operation,
    USHORT	fAllowed,
    USHORT	hVPB,
    PVOID	pData,
    PUSHORT	pcSec,
    ULONG	iSec );

/*  Flags for operation
 */
#define DVIO_OPREAD	0x0000		/*  no bit on => readi		      */
#define DVIO_OPWRITE	0x0001		/*  ON => write else read	      */
#define DVIO_OPBYPASS	0x0002		/*  ON => cache bypass else no bypass */
#define DVIO_OPVERIFY	0x0004		/*  ON => verify after write	      */
#define DVIO_OPHARDERR	0x0008		/*  ON => return hard errors directly */
#define DVIO_OPWRTHRU	0x0010		/*  ON => write thru		      */
#define DVIO_OPNCACHE	0x0020		/*  ON => don't cache data            */
#define DVIO_OPRESMEM	0x0040		/*  ON => don't lock this memory      */

/*  Flags for fAllowed
 */
#define DVIO_ALLFAIL	0x0001		/*  FAIL allowed		      */
#define DVIO_ALLABORT	0x0002		/*  ABORT allowed		      */
#define DVIO_ALLRETRY	0x0004		/*  RETRY allowed		      */
#define DVIO_ALLIGNORE	0x0008		/*  IGNORE allowed		      */
#define DVIO_ALLACK	0x0010		/*  ACK allowed			      */

APIRET APIENTRY FSH_DOVOLIO2 (
    ULONG	hDev,
    USHORT	sfn,
    USHORT	cat,
    USHORT	func,
    PVOID	pParm,
    USHORT	cbParm,
    PVOID	pData,
    USHORT	cbData );

APIRET APIENTRY FSH_FINDCHAR (
    USHORT	nChars,
    PCHAR	pChars,
    PSZ	FAR *   ppStr );

APIRET APIENTRY FSH_FINDDUPHVPB (
    USHORT	hVPB,
    PUSHORT	phVPB );

APIRET APIENTRY FSH_FORCENOSWAP (
    USHORT	sel );

APIRET APIENTRY FSH_GETPRIORITY (
    VOID );

APIRET APIENTRY FSH_GETVOLPARM (
    USHORT	hVPB,
    struct vpfsi FAR * FAR *ppVPBfsi,
    struct vpfsd FAR * FAR *ppVPBfsd );

APIRET APIENTRY FSH_INTERR (
    PCHAR	pMsg,
    USHORT	cbMsg );

APIRET APIENTRY FSH_IOBOOST (
    VOID );

APIRET APIENTRY FSH_IOSEMCLEAR (
    PVOID	pSem );

APIRET APIENTRY FSH_ISCURDIRPREFIX (
    PSZ		pName );

APIRET APIENTRY FSH_LOADCHAR (
    PSZ FAR *	ppStr,
    PUSHORT	pChar );

APIRET APIENTRY FSH_NAMEFROMSFN (
    USHORT	sfn,
    PSZ		pName,
    PUSHORT	pcbName );

APIRET APIENTRY FSH_PREVCHAR (
    PSZ		pBeg,
    PSZ FAR *	ppStr );

APIRET APIENTRY FSH_PROBEBUF (
    USHORT	operation,
    PVOID	pData,
    USHORT	cbData );

/*  Values for operation
 */
#define PB_OPREAD   0x0000		/*  Check for read access	      */
#define PB_OPWRITE  0x0001		/*  Check for write access	      */

APIRET APIENTRY FSH_QSYSINFO (
    USHORT	index,
    PVOID	pData,
    USHORT	cbData );

/* Values for index
 */
#define QSI_SECSIZE	1	/* index to query max sector size */
#define QSI_PROCID	2	/* index to query PID,UserID and Currentpdb */
#define QSI_THREADNO	3	/* index to query abs.thread no */
#define QSI_VERIFY	4	/* index to query per-process verify */

APIRET APIENTRY FSH_REGISTERPERFCTRS (
    PVOID	pDataBlk,
    PVOID	pTextBlk,
    USHORT	fsFlags );

/*  Flags for fsFlags
 */
#define RPC_16BIT	0x0000		/*  16-bit FSH interface	      */
#define RPC_32BIT	0x0001		/*  32-bit FSH interface	      */


APIRET APIENTRY FSH_REMOVESHARE (
    ULONG	hShare );

APIRET APIENTRY FSH_SEGALLOC (
    USHORT	flags,
    ULONG	cbSeg,
    PUSHORT	pSel );

/*  Fields for flags
 */
#define SA_FLDT		0x0001		/*  ON => alloc LDT else GDT	      */
#define SA_FSWAP	0x0002		/*  ON => swappable memory	      */

#define SA_FRINGMASK	0x6000		/*  mask for isolating ring	      */
#define SA_FRING0	0x0000		/*  ring 0			      */
#define SA_FRING1	0x2000		/*  ring 1			      */
#define SA_FRING2	0x4000		/*  ring 2			      */
#define SA_FRING3	0x6000		/*  ring 3			      */


APIRET APIENTRY FSH_SEGFREE (
    USHORT	sel );

APIRET APIENTRY FSH_SEGREALLOC (
    USHORT	sel,
    ULONG	cbSeg );


/*  Timeout equates for all semaphore operations
 */
#define TO_INFINITE	0xFFFFFFFFL
#define TO_NOWAIT	0x00000000L

APIRET APIENTRY FSH_SEMCLEAR (
    PVOID	pSem );

APIRET APIENTRY FSH_SEMREQUEST (
    PVOID	pSem,
    ULONG	cmsTimeout );

APIRET APIENTRY FSH_SEMSET (
    PVOID	pSem );

APIRET APIENTRY FSH_SEMSETWAIT (
    PVOID	pSem,
    ULONG	cmsTimeout );

APIRET APIENTRY FSH_SEMWAIT (
    PVOID	pSem,
    ULONG	cmsTimeout );

APIRET APIENTRY FSH_SETVOLUME (
    USHORT	hVPB,
    USHORT	fControl );

APIRET APIENTRY FSH_STORECHAR (
    USHORT	chDBCS,
    PSZ FAR *	ppStr );

APIRET APIENTRY FSH_UPPERCASE (
    PSZ		pName,
    USHORT	cbPathBuf,
    PSZ		pPathBuf );

APIRET APIENTRY FSH_WILDMATCH (
    PSZ		pPat,
    PSZ		pStr );

APIRET APIENTRY FSH_YIELD (
    VOID );
