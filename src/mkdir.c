#include "includes.h"





APIRET EXPENTRY FS_MKDIR (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    PEAOP	pEABuf,
    USHORT	flags )
{
  int      rc;
  PVOLUME  pVolume;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  ULONG    datiNow;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_MKDIR  pName='%s' flags=%d", pName, flags);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;
  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;

  if (pName[0] == '\0')
  {
    /* tried to create root directory */
    rc = ERROR_ACCESS_DENIED;
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


    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   rc = ERROR_ACCESS_DENIED;
	   break;


    case LOC_NOENTRY:
	   Entry.fDOSattr = flags | DOSATTR_DIRECTORY;
	   datiNow = UtilGetDateTime();
	   Entry.datiCreate = datiNow;
	   Entry.datiAccess = datiNow;
	   Entry.datiWrite  = datiNow;

	   rc = BlockMakeEmptyDir (&Entry.fblkFile.clusters);
	   if (rc)  goto backout1;

	   Entry.blkEA.flatAddr = 0;
	   Entry.blkEA.cbSize   = 0;
	   rc = EaAddList (&Entry.blkEA, pEABuf);
	   if (rc)  goto backout2;

	   rc = UtilInsertEntry (pVolume, flatBlkDir, &Entry, &flatEntry);
	   if (rc)  goto backout3;

	   break;


	 backout3:
	   BlockFree (&Entry.blkEA);
	 backout2:
	   BlockFree (&Entry.fblkFile.clusters);
	 backout1:
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
