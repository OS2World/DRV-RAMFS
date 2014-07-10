#include "includes.h"





APIRET EXPENTRY FS_WRITE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pData,
    PUSHORT	pLen,
    USHORT	IOflag )	/* not used */
{
  int       rc;
  POPENFILE pOpenfile;
  ULONG     cbNewPos;
  FBLOCK    fblkFile;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_WRITE  sfn=%d *pLen=%u", psffsi->sfi_selfsfn, *pLen);

  pOpenfile = psffsd->pOpenfile;

  if (pOpenfile == NULL || pOpenfile->flatEntry == NULL)
  {
    /* Volume already detached */
    rc = ERROR_HANDLE_EOF;
    *pLen = 0;
    goto end;
  }

  VMRead (&fblkFile, pOpenfile->flatEntry + FIELDOFFSET(DIRENTRY,fblkFile), sizeof(FBLOCK));

#ifdef DEBUG
  if (fblkFile.fSize != psffsi->sfi_size)
  {
    debugging = TRUE;
    DEBUG_PRINTF2 ("\r\n!!! fblkFile.fSize = %lu, sfi_size = %lu\r\n",
		   fblkFile.fSize, psffsi->sfi_size);
    INT3;
  }
#endif

  cbNewPos = psffsi->sfi_position + *pLen;
  if (cbNewPos > psffsi->sfi_size)
  {
    rc = chsize_helper (&fblkFile, cbNewPos);
    if (rc)
      goto end;
    VMWrite (pOpenfile->flatEntry + FIELDOFFSET(DIRENTRY,fblkFile), &fblkFile, sizeof(FBLOCK));
    psffsi->sfi_size = cbNewPos;
  }
  /* Defer the actual operation until we proclaim reentrancy */
  rc = NO_ERROR;

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();

  /* Now do it */
  if (rc == NO_ERROR)
  {
    write_fblocks(pData, &fblkFile, psffsi->sfi_position, *pLen);
    psffsi->sfi_position = cbNewPos;
    psffsi->sfi_tstamp |= (ST_SWRITE | ST_PWRITE);
  }
  
  return rc;
}
