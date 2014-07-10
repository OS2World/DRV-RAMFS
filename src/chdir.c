#include "includes.h"



APIRET EXPENTRY FS_CHDIR (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pDir,
    USHORT	iCurDirEnd )
{
  int      rc;
  PVOLUME  pVolume;
  PCURDIR  pCurdir;
  PCURDIR  pPrevCurdir;
  PCURDIR  pCurCurdir;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  PSZ      pFullDir;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_CHDIR  flag=%d  pDir='%s'", flag, pDir);

  switch (flag)
  {
    case 0: /* allocate new working directory */
	    FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
	    pVolume = pvpfsd->pVolume;
	    flatBlkDir = pVolume->flatBlkRootDir;
	    pFullDir = pDir;
	    pDir += 3;
	    if (iCurDirEnd != 0xFFFF)
	    {
	      flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
	      pDir += iCurDirEnd-3;
	    }

	    switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pDir))
	    {
	      case LOC_NOPATH:
		     rc = ERROR_PATH_NOT_FOUND;
		     break;

	      case LOC_FILEENTRY:
		     rc = ERROR_ACCESS_DENIED;
		     break;

	      case LOC_NOENTRY:
		     rc = ERROR_PATH_NOT_FOUND;
		     break;

	      case LOC_DIRENTRY:
		     rc = NearAlloc ((PNEARBLOCK *) &pCurdir, sizeof(CURDIR) + strlen(pFullDir));
		     if (rc)
		       break;
		     pcdfsd->pCurdir = pCurdir;
		     pCurdir->pNextCurdir = pVolume->pFirstCurdir;
		     pVolume->pFirstCurdir = pCurdir;
		     pCurdir->pVolume = pVolume;
		     pCurdir->flatBlkDir = flatEntry + FIELDOFFSET(DIRENTRY,fblkFile.clusters);
		     strcpy (pCurdir->szDir, pFullDir);
		     rc = NO_ERROR;
		     break;
	    }
	    break;


    case 1: /* verify working directory - only for removable media? */
	    rc = ERROR_NOT_SUPPORTED;
	    break;


    case 2: /* deallocate working directory */
	    if (pcdfsi != NULL)
	      FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
	    else
	      pvpfsd = NULL;
	    pCurdir = pcdfsd->pCurdir;
	    pVolume = pCurdir->pVolume;
	    pPrevCurdir = 0;
	    pCurCurdir = pVolume->pFirstCurdir;

	    if (pCurCurdir == 0)
            {
               /* All directories already freed, nothing to do */
               rc = NO_ERROR;
	       break;
	    }

	    while (pCurCurdir != pCurdir)
	    {
#ifdef DEBUG
	      if (pCurCurdir == 0)
	      {
		debugging = TRUE;
		DEBUG_PRINTF0 ("\r\n!!! CURDIR not found\r\n");
		INT3;
	      }
#endif
	      pPrevCurdir = pCurCurdir;
	      pCurCurdir = pCurCurdir->pNextCurdir;
	    }

	    if (pPrevCurdir != 0)
	      pPrevCurdir->pNextCurdir = pCurCurdir->pNextCurdir;
	    else
	      pVolume->pFirstCurdir = pCurCurdir->pNextCurdir;
	    NearFree (pCurCurdir);
	    if (pvpfsd != NULL)
	      pvpfsd->pVolume = scavenge_volume (pVolume);
	    rc = NO_ERROR;
	    break;
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
