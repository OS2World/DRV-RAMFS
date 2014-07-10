#include "includes.h"





APIRET EXPENTRY FS_MOVE (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pSrc,
    USHORT	iSrcCurDirEnd,
    PSZ		pDst,
    USHORT	iDstCurDirEnd,
    USHORT	flags )
{
  int       rc;
  PSZ	    pName;
  PVOLUME   pVolume;
  DIRENTRY  SrcEntry;
  DIRENTRY  DstEntry;
  FLAT      flatEntry;
  FLAT      flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_MOVE  pSrc='%s' pDst='%s' flags=%d",
		 pSrc, pDst, flags);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  /* examine source entry */
  flatBlkDir = pVolume->flatBlkRootDir;

  if (flatBlkDir == 0)
  {
     rc = ERROR_PATH_NOT_FOUND;
     goto end;
  }

  pName = pSrc + 3;
  if (iSrcCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iSrcCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &SrcEntry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   goto end;

    case LOC_NOENTRY:
	   rc = ERROR_FILE_NOT_FOUND;
	   goto end;

    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   /* source name seems OK */
	   break;
  }

  /* preserve original case in source entry */
  VMRead (SrcEntry.achName, flatEntry + FIELDOFFSET(DIRENTRY,achName),
	  SrcEntry.cbName);
  /* remove source entry */
  UtilDeleteEntry (pVolume, flatBlkDir, &SrcEntry, flatEntry);

  /* examine destination name */
  flatBlkDir = pVolume->flatBlkRootDir;
  pName = pDst + 3;
  if (iDstCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iDstCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &DstEntry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;

    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   /* destination entry already exists */
	   rc = ERROR_ACCESS_DENIED;
	   break;

    case LOC_NOENTRY:
	   /* ok, insert source entry here */
	   DstEntry.fDOSattr = (SrcEntry.fDOSattr & ~DOSATTR_NON83) | flags;
	   DstEntry.fblkFile         = SrcEntry.fblkFile;
	   DstEntry.blkEA.flatAddr   = SrcEntry.blkEA.flatAddr;
	   DstEntry.blkEA.cbSize     = SrcEntry.blkEA.cbSize;
	   DstEntry.datiCreate       = SrcEntry.datiCreate;
	   DstEntry.datiAccess       = SrcEntry.datiAccess;
	   DstEntry.datiWrite        = SrcEntry.datiWrite;
	   rc = UtilInsertEntry (pVolume, flatBlkDir, &DstEntry, &flatEntry);
	   break;
  }

  if (rc != NO_ERROR)
  {
    /* error inserting new entry, try to reinsert source entry */
    /* find out where to reinsert SrcEntry */
    flatBlkDir = pVolume->flatBlkRootDir;
    pName = pSrc + 3;
    if (iSrcCurDirEnd != 0xFFFF)
    {
      flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
      pName += iSrcCurDirEnd-3;
    }
    if (UtilLocate (&flatBlkDir, &flatEntry, &DstEntry, pName) != LOC_NOENTRY)
    {
      /* this never happens */
#ifdef DEBUG
      debugging = TRUE;
      DEBUG_PRINTF0 ("\r\n!!! FS_MOVE  reinserting src not LOC_NOENTRY\r\n");
#endif
      INT3;
    }

    if (UtilInsertEntry (pVolume, flatBlkDir, &SrcEntry, &flatEntry) != NO_ERROR)
    {
      /* let's hope this never happens */
#ifdef DEBUG
      debugging = TRUE;
      DEBUG_PRINTF2 ("\r\n!!! FS_MOVE  reinserting src error too difficult to handle src='%s' dst='%s'\r\n",
		     pSrc, pDst);
#endif
      INT3;
    }
  }


end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
