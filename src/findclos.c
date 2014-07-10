#include "includes.h"





APIRET EXPENTRY FS_FINDCLOSE (
    struct fsfsi *pfsfsi,
    struct fsfsd *pfsfsd )
{
  int     rc;
  PVOLUME pVolume;
  PSEARCH pSearch;
  PSEARCH pCurSearch;
  PSEARCH pPrevSearch;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_FINDCLOSE  %04X:%04X", FP_SEG(pfsfsd), FP_OFF(pfsfsd));

  pSearch = pfsfsd->pSearch;

  if (pSearch == 0)
  {
    /* no SEARCH has been allocated - e.g. a search without wildcards */
    rc = NO_ERROR;
    goto end;
  }


  FSH_GETVOLPARM (pfsfsi->fsi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  DEBUG_PRINTF1 (" '%s'", (char *) pSearch->szPattern);

  if(pVolume == 0)
  {
    /* Already detached */
    goto end;
  }

  pPrevSearch = 0;
  pCurSearch = pVolume->pFirstSearch;

  if (pCurSearch == 0)
  {
    /* No active searches - this volume might have been detached */
    rc = NO_ERROR;
    goto end;
  }

  while (pCurSearch != pSearch)
  {
#ifdef DEBUG
    if (pCurSearch == 0)
    {
      debugging = TRUE;
      DEBUG_PRINTF0 ("\r\n!!! SEARCH not found\r\n");
      INT3;
    }
#endif
    pPrevSearch = pCurSearch;
    pCurSearch = pCurSearch->pNextSearch;
  }

  if (pPrevSearch != 0)
    pPrevSearch->pNextSearch = pCurSearch->pNextSearch;
  else
    pVolume->pFirstSearch = pCurSearch->pNextSearch;
  NearFree (pSearch);
  pvpfsd->pVolume = scavenge_volume (pVolume);
  rc = NO_ERROR;

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
