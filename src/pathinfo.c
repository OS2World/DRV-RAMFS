#include "includes.h"





APIRET EXPENTRY FS_PATHINFO (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	level,
    PCHAR	pData,
    USHORT	cbData )
{
  int      rc;
  PVOLUME  pVolume;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_PATHINFO  flag=%d pName='%s' level=%d", flag, pName, level);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  if (pVolume == NULL)
  {
    /* Already detached */
    rc = ERROR_FILE_NOT_FOUND;
    goto end;
  }

  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;

  if (pName[0] == '\0')
  {
    /* get/set info about root */
    if (flag == 0)
    {
      /* get info about root - build some fake root information */
      Entry.cbName = 0;
      Entry.fDOSattr = DOSATTR_DIRECTORY;
      memset(&Entry.fblkFile, 0, sizeof(FBLOCK));
      Entry.blkEA.flatAddr   = 0;
      Entry.blkEA.cbSize     = 0;
      Entry.datiCreate = Entry.datiWrite = pVolume->datiCreate;
      Entry.datiAccess = UtilGetDateTime();
      Entry.achName[0] = '\0';
      rc = InfoQuery (pData, cbData, level, &Entry);
    }
    else
    {
      /* set info about root - ignore it */
      Entry.blkEA.flatAddr = 0;
      Entry.blkEA.cbSize   = 0;
      rc = InfoSet (pData, cbData, level, &Entry, NULL);
      BlockFree (&Entry.blkEA);
    }
    goto end;
  }

  if (iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;


    case LOC_NOENTRY:
	   rc = ERROR_FILE_NOT_FOUND;
	   break;


    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   if (flag == 0)
	   {
	     /* retrieve information */
	     rc = InfoQuery (pData, cbData, level, &Entry);
	   }
	   else
	   {
	     /* set information */
	     rc = InfoSet (pData, cbData, level, &Entry, NULL);
	     if (rc == NO_ERROR)
	       VMWrite (flatEntry, &Entry, sizeof(Entry)-CCHMAXPATHCOMP);
	   }
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
