#include "includes.h"

/* Suspends all active filesystem operations for a volume */

static void disconnect_operations(PVOLUME pVolume)
{
   POPENFILE pOpenfile;
   PSEARCH pSearch;

   DEBUG_PRINTF0 (" [");
   pSearch = pVolume->pFirstSearch;
   while (pSearch != NULL)
   {
      DEBUG_PRINTF0 ("S");
      pSearch->flatEntry = SEARCH_END;
      pSearch = pSearch->pNextSearch;
   }

   pOpenfile = pVolume->pFirstOpenfile;
   while (pOpenfile != NULL)
   {
      DEBUG_PRINTF0 ("O");
      pOpenfile->flatEntry = 0;
      pOpenfile = pOpenfile->pNextOpenfile;
   }

   DEBUG_PRINTF0 ("] ");
}

/* Drops the remaining volume structure after all operation structures are
   reclaimed */

PVOLUME scavenge_volume(PVOLUME pVolume)
{
   if(!pVolume->sAttached &&
      pVolume->pFirstSearch == NULL &&
      pVolume->pFirstCurdir == NULL &&
      pVolume->pFirstOpenfile == NULL)
   {
      DEBUG_PRINTF1 (" | scavenge_volume (%08lx) |", pVolume);
      NearFree (pVolume);
      pVolume = NULL;
   }
   return pVolume;
}

/* Performs a recursive deletion upon directories */

static void rm_rf(PBLOCK pBlkDir)
{
  DIRENTRY_HDR dh;
  FLAT flatEntry, flatEnd;

  DEBUG_PRINTF0 ("D{");
  flatEntry = pBlkDir->flatAddr + DIR_DOTSSIZE;
  flatEnd   = pBlkDir->flatAddr + pBlkDir->cbSize;

  while (flatEntry < flatEnd)
  {
    VMRead (&dh, flatEntry, sizeof(DIRENTRY_HDR));
    if (dh.fDOSattr & DOSATTR_DIRECTORY)
    {
      rm_rf (&dh.fblkFile.clusters);
      BlockFree (&dh.fblkFile.clusters);
    }
    else
    {
#ifdef DEBUG
      DEBUG_PRINTF0 ("f");
#endif
      fblock_shutdown (&dh.fblkFile);
    }
    BlockFree (&dh.blkEA);
    flatEntry += sizeof(DIRENTRY) - CCHMAXPATHCOMP + dh.cbName;
  }
  DEBUG_PRINTF0 ("}");
}

/* FS_ATTACH entry point */

APIRET EXPENTRY FS_ATTACH (
    USHORT	flag,
    PSZ		pDev,
    struct vpfsd *pvpfsd,
    struct cdfsd *pcdfsd,	/* not used */
    PCHAR	pParm,
    PUSHORT	pLen )
{
  int rc;
  BLOCK   blkRootDir;
  PVOLUME pVolume;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_ATTACH  pDev='%s' flag=%d", pDev, flag);

  switch (flag)
  {
    case 0: /* Attach */
	    if (pDev[1] != ':')
	    {
	      /* only drives, please */
	      rc = ERROR_NOT_SUPPORTED;
	      break;
	    }

	    rc = NearAlloc ((PNEARBLOCK *) &pVolume, sizeof(VOLUME));
	    if (rc)
	      break;

	    rc = BlockMakeEmptyDir (&blkRootDir);
	    if (rc)
	    {
	      NearFree (pVolume);
	      break;
	    }

	    pvpfsd->pVolume = pVolume;
	    pVolume->blkRootDir.flatAddr = blkRootDir.flatAddr;
	    pVolume->blkRootDir.cbSize   = blkRootDir.cbSize;
	    pVolume->flatBlkRootDir = VMVirtToFlat (&pVolume->blkRootDir);
	    pVolume->pFirstOpenfile = 0;
	    pVolume->pFirstCurdir = 0;
	    pVolume->pFirstSearch = 0;
            pVolume->sAttached = 1;
	    memset (pVolume->szLabel, 0, sizeof(pVolume->szLabel));
	    if (FP_SEG(pParm) > 3)
	      strncpy (pVolume->szLabel, pParm, 11);	/* volume label */
            pVolume->datiCreate = UtilGetDateTime();

	    rc = NO_ERROR;
	    break;


    case 1: /* Detach */
	    if (pDev[1] != ':')
	    {
	      /* only drives, please */
	      rc = ERROR_NOT_SUPPORTED;
	      break;
	    }
            pVolume = pvpfsd->pVolume;
            disconnect_operations (pVolume);
            rm_rf (&pVolume->blkRootDir);
            BlockFree (&pVolume->blkRootDir);
            pVolume->flatBlkRootDir = 0;
            pVolume->sAttached = 0;
            pvpfsd->pVolume = pVolume = scavenge_volume(pVolume);

	    rc = NO_ERROR;
	    break;


    case 2: /* Query */
	    if (*pLen >= 8)
	    {
	      /* set cbFSAData to 0 => we return 0 bytes in rgFSAData area */
	      *((USHORT *) pParm) = 6;
	      pParm[2] = 'H';
	      pParm[3] = 'e';
	      pParm[4] = 'l';
	      pParm[5] = 'l';
	      pParm[6] = 'o';
	      pParm[7] = '\0';
	      rc = NO_ERROR;
	    }
	    else
	    {
	      /* not enough room to tell that we wanted to return 0 bytes */
	      rc = ERROR_BUFFER_OVERFLOW;
	    }
	    *pLen = 8;
	    break;
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
