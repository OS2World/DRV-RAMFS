#include "includes.h"





APIRET EXPENTRY FS_CLOSE (
    USHORT	type,
    USHORT	IOflag,		/* not used */
    struct sffsi *psffsi,
    struct sffsd *psffsd )
{
  int rc;
  POPENFILE pOpenfile;
  POPENFILE pCur;
  POPENFILE pPrev;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  DIRENTRY  Entry;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_CLOSE  sfn=%d, type=%d", psffsi->sfi_selfsfn, type);

  FSH_GETVOLPARM (psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  pOpenfile = psffsd->pOpenfile;

  if (type != 2)
  {
    /* not final close - do nothing */
    rc = NO_ERROR;
    goto end;
  }

  pPrev = 0;
  pCur = pvpfsd->pVolume->pFirstOpenfile;

  /* No open files at all, volume might have been already detached */
  if(pCur == 0)
  {
    rc = NO_ERROR;
    goto end;
  }

  while (pCur != pOpenfile)
  {
#ifdef DEBUG
    if (pCur == 0)
    {
      debugging = TRUE;
      DEBUG_PRINTF0 ("\r\n!!! Openfile instance not found\r\n");
      INT3;
      rc = ERROR_NOT_SUPPORTED;
      goto end;
    }
#endif

    pPrev = pCur;
    pCur = pCur->pNextOpenfile;
  }

  if (pOpenfile->flatEntry != 0)
  {
    VMRead (&Entry, pOpenfile->flatEntry, sizeof(Entry)-CCHMAXPATHCOMP);
    Entry.datiCreate = psffsi->sfi_cdate + ((ULONG)psffsi->sfi_ctime << 16);
    Entry.datiAccess = psffsi->sfi_adate + ((ULONG)psffsi->sfi_atime << 16);
    Entry.datiWrite  = psffsi->sfi_mdate + ((ULONG)psffsi->sfi_mtime << 16);
    VMWrite (pOpenfile->flatEntry, &Entry, sizeof(Entry)-CCHMAXPATHCOMP);
  }

  if (pPrev)
    pPrev->pNextOpenfile = pOpenfile->pNextOpenfile;
  else
    pvpfsd->pVolume->pFirstOpenfile = pOpenfile->pNextOpenfile;
  NearFree (pOpenfile);
  pvpfsd->pVolume = scavenge_volume(pvpfsd->pVolume);
  rc = NO_ERROR;

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
