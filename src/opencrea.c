#include "includes.h"



/* static */
void ShareAccess (USHORT usOpenMode, USHORT _ss *pfShare, USHORT _ss *pfAccess)
{
  switch (usOpenMode & 0x0070)
  {
    case OPEN_SHARE_DENYREADWRITE:
	   *pfShare = SHARE_DENYREAD | SHARE_DENYWRITE;
	   break;

    case OPEN_SHARE_DENYWRITE:
	   *pfShare = SHARE_DENYWRITE;
	   break;

    case OPEN_SHARE_DENYREAD:
	   *pfShare = SHARE_DENYREAD;
	   break;

    case OPEN_SHARE_DENYNONE:
	   *pfShare = 0;
	   break;
  }

  switch (usOpenMode & 0x0007)
  {
    case OPEN_ACCESS_READONLY:
	   *pfAccess = ACCESS_READ;
	   break;

    case OPEN_ACCESS_WRITEONLY:
	   *pfAccess = ACCESS_WRITE;
	   break;

    case OPEN_ACCESS_READWRITE:
	   *pfAccess = ACCESS_READ | ACCESS_WRITE;
	   break;
  }
}




/* static */
int CreateFile (
    PDIRENTRY     pEntry,
    FLAT          flatBlkDir,
    FLAT          flatEntry,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PVOLUME       pVolume,
    USHORT	  usAttr,
    PEAOP	  pEABuf )
{
  int       rc;
  USHORT    fShare;
  USHORT    fAccess;
  POPENFILE pOpenfile;

  ShareAccess ((USHORT) psffsi->sfi_mode, &fShare, &fAccess);

  psffsi->sfi_DOSattr = (UCHAR) usAttr;
  pEntry->fDOSattr    = (UCHAR) usAttr;
  pEntry->datiCreate = pEntry->datiAccess = pEntry->datiWrite =
    psffsi->sfi_mdate + ((ULONG)psffsi->sfi_mtime << 16);

  rc = fblock_init(&pEntry->fblkFile);
  if (rc)  goto backout1;

  rc = chsize_helper(&pEntry->fblkFile, psffsi->sfi_size);
  if (rc)  goto backout2;

  pEntry->blkEA.flatAddr = 0;
  pEntry->blkEA.cbSize   = 0;
  rc = EaAddList (&pEntry->blkEA, pEABuf);
  if (rc)  goto backout2;

  rc = NearAlloc ((PNEARBLOCK *) &pOpenfile, sizeof(OPENFILE) + pEntry->cbName);
  if (rc)  goto backout3;

  rc = UtilInsertEntry (pVolume, flatBlkDir, pEntry, &flatEntry);
  if (rc)  goto backout4;

  pOpenfile->pNextOpenfile = pVolume->pFirstOpenfile;
  pVolume->pFirstOpenfile = pOpenfile;
  pOpenfile->flatEntry = flatEntry;
  pOpenfile->fShare    = fShare;
  memcpy (pOpenfile->szName, pEntry->achName, pEntry->cbName+1);
  psffsd->pOpenfile = pOpenfile;

  return NO_ERROR;


backout4:
  NearFree (pOpenfile);
backout3:
  BlockFree (&pEntry->blkEA);
backout2:
  fblock_shutdown (&pEntry->fblkFile);
backout1:
  return rc;
}




/* static */
int ReplaceFile (
    PDIRENTRY     pEntry,
    FLAT          flatEntry,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PVOLUME       pVolume,
    USHORT        usAttr,
    PEAOP	  pEABuf )
{
  int       rc;
  USHORT    fShare;
  USHORT    fAccess;
  POPENFILE pOpenfile;
  FBLOCK    fblkFile;
  BLOCK     blkEA;

  ShareAccess ((USHORT)psffsi->sfi_mode, &fShare, &fAccess);

  /* attempting to overwrite a read-only file? */
  if (pEntry->fDOSattr & DOSATTR_READONLY)
    return ERROR_ACCESS_DENIED;

  /* prevent replacing a hidden/system file with a regular one */
  if ((pEntry->fDOSattr & ~usAttr) & (DOSATTR_HIDDEN | DOSATTR_SYSTEM))
    return ERROR_ACCESS_DENIED;

  /* any open instances of this file? */
  pOpenfile = pVolume->pFirstOpenfile;
  while (pOpenfile != 0)
  {
    if (pOpenfile->flatEntry == flatEntry && (pOpenfile->fShare & SHARE_DENYWRITE))
      return ERROR_SHARING_VIOLATION;
    pOpenfile = pOpenfile->pNextOpenfile;
  }

  rc = NearAlloc ((PNEARBLOCK *) &pOpenfile, sizeof(OPENFILE) + pEntry->cbName);
  if (rc)  goto backout1;

  blkEA.flatAddr = 0;
  blkEA.cbSize   = 0;
  rc = EaAddList (&blkEA, pEABuf);
  if (rc)  goto backout2;

  rc = fblock_init (&fblkFile);
  if (rc)  goto backout3;

  rc = chsize_helper(&fblkFile, psffsi->sfi_size);
  if (rc)
  {
   fblock_shutdown(&fblkFile);
   goto backout3;
  }
  
  /* everything that could go wrong has now been done. commit the changes. */
  fblock_shutdown (&pEntry->fblkFile);
  BlockFree (&pEntry->blkEA);

  pEntry->fblkFile       = fblkFile;
  pEntry->blkEA.flatAddr = blkEA.flatAddr;
  pEntry->blkEA.cbSize   = blkEA.cbSize;

  pEntry->fDOSattr    = (UCHAR) usAttr;
  psffsi->sfi_DOSattr = (UCHAR) usAttr;
  pEntry->datiCreate = pEntry->datiAccess = pEntry->datiWrite =
    psffsi->sfi_mdate + ((ULONG)psffsi->sfi_mtime << 16);
  VMWrite (flatEntry, pEntry, sizeof(DIRENTRY)-CCHMAXPATHCOMP + pEntry->cbName);

  pOpenfile->pNextOpenfile = pVolume->pFirstOpenfile;
  pVolume->pFirstOpenfile = pOpenfile;
  pOpenfile->flatEntry = flatEntry;
  pOpenfile->fShare    = fShare;
  memcpy (pOpenfile->szName, pEntry->achName, pEntry->cbName+1);
  psffsd->pOpenfile = pOpenfile;

  return NO_ERROR;

backout3:
  BlockFree (&blkEA);
backout2:
  NearFree (pOpenfile);
backout1:
  return rc;
}




/* static */
int OpenFile (
    PDIRENTRY     pEntry,
    FLAT          flatEntry,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PVOLUME       pVolume )
{
  USHORT    fShare;
  USHORT    fAccess;
  POPENFILE pOpenfile;
  POPENFILE pCurOpenfile;
  int       rc;

  ShareAccess ((USHORT)psffsi->sfi_mode, &fShare, &fAccess);

  /* writing to a read-only file? */
  if ((fAccess & ACCESS_WRITE)  &&  (pEntry->fDOSattr & DOSATTR_READONLY))
    return ERROR_ACCESS_DENIED;

  /* conflicting sharing with another open instance of this file? */
  pCurOpenfile = pVolume->pFirstOpenfile;
  while (pCurOpenfile != NULL)
  {
    if (pCurOpenfile->flatEntry == flatEntry  &&  (pCurOpenfile->fShare & fAccess))
      return ERROR_SHARING_VIOLATION;
    pCurOpenfile = pCurOpenfile->pNextOpenfile;
  }

  rc = NearAlloc ((PNEARBLOCK *) &pOpenfile, sizeof(OPENFILE) + pEntry->cbName);
  if (rc)
    return rc;

  /* all ok, open the file */
  psffsi->sfi_ctime = (USHORT) (pEntry->datiCreate >> 16);
  psffsi->sfi_cdate = (USHORT) (pEntry->datiCreate);
  psffsi->sfi_atime = (USHORT) (pEntry->datiAccess >> 16);
  psffsi->sfi_adate = (USHORT) (pEntry->datiAccess);
  psffsi->sfi_mtime = (USHORT) (pEntry->datiWrite >> 16);
  psffsi->sfi_mdate = (USHORT) (pEntry->datiWrite);
  psffsi->sfi_size  = pEntry->fblkFile.fSize;
  psffsi->sfi_DOSattr = pEntry->fDOSattr;
  psffsi->sfi_type = (psffsi->sfi_type & STYPE_FCB) | STYPE_FILE;

  pOpenfile->pNextOpenfile = pVolume->pFirstOpenfile;
  pVolume->pFirstOpenfile = pOpenfile;
  pOpenfile->flatEntry = flatEntry;
  pOpenfile->fShare    = fShare;
  memcpy (pOpenfile->szName, pEntry->achName, pEntry->cbName+1);
  psffsd->pOpenfile = pOpenfile;

  return NO_ERROR;
}






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
    PUSHORT	pfgenflag )	/* not used */
{
  int      rc;
  PVOLUME  pVolume;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_OPENCREATE  sfn=%d pName='%s' usAttr=0x%04X",
		 psffsi->sfi_selfsfn, pName, usAttr);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;
  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;
  if (iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iCurDirEnd-3;
  }

  if (ulOpenMode & OPEN_FLAGS_DASD)
  {
    rc = ERROR_NOT_SUPPORTED;
    goto end;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;


    case LOC_DIRENTRY:
	   rc = ERROR_ACCESS_DENIED;
	   break;


    case LOC_NOENTRY:
	   switch (usOpenFlag & 0x00F0)
	   {
	     case OPEN_ACTION_CREATE_IF_NEW:
                    /* Ensure we don't end up with a zero-length filename */
                    if (*pName == '\0' && iCurDirEnd == 0xFFFF)
                    {
                      rc = ERROR_ACCESS_DENIED;
                      break;
                    }
		    /* create a new file */
		    *pusAction = FILE_CREATED;
		    rc = CreateFile (&Entry, flatBlkDir, flatEntry, psffsi,
				     psffsd, pVolume, usAttr, pEABuf);
		    break;

	     case OPEN_ACTION_FAIL_IF_NEW:
		    rc = ERROR_OPEN_FAILED;
		    break;
	   }
	   break;


    case LOC_FILEENTRY:
	   switch (usOpenFlag & 0x000F)
	   {
	     case OPEN_ACTION_OPEN_IF_EXISTS:
		    /* open an existing file */
		    *pusAction = FILE_EXISTED;
		    rc = OpenFile (&Entry, flatEntry, psffsi, psffsd, pVolume);
		    break;

	     case OPEN_ACTION_REPLACE_IF_EXISTS:
		    /* replace an existing file */
		    *pusAction = FILE_TRUNCATED;
		    rc = ReplaceFile (&Entry, flatEntry, psffsi, psffsd,
				      pVolume, usAttr, pEABuf);
		    break;

	     case OPEN_ACTION_FAIL_IF_EXISTS:
		    rc = ERROR_OPEN_FAILED;
		    break;
	   }
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
