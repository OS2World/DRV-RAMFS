#include "includes.h"





APIRET EXPENTRY FS_READ (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pData,
    PUSHORT	pLen,
    USHORT	IOflag )	/* not used */
{
  int       rc;
  POPENFILE pOpenfile;
  FBLOCK    fblkFile;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_READ  sfn=%d *pLen=%u", psffsi->sfi_selfsfn, *pLen);

  pOpenfile = psffsd->pOpenfile;

  if (pOpenfile == NULL || pOpenfile->flatEntry == NULL)
  {
    /* Volume already detached */
    rc = ERROR_HANDLE_EOF;
    *pLen = 0;
    goto end;
  }

  if (psffsi->sfi_position > psffsi->sfi_size)
    *pLen = 0;
  else if (*pLen > psffsi->sfi_size - psffsi->sfi_position)
    *pLen = psffsi->sfi_size - psffsi->sfi_position;

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
  rc = NO_ERROR;

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);

  /* Do it here so the context-safe read operation can be reentrant */
  UtilExitRamfs();

  read_fblocks(pData, &fblkFile, psffsi->sfi_position, *pLen);
  psffsi->sfi_position += *pLen;
  psffsi->sfi_tstamp |= (ST_SREAD | ST_PREAD);

  return rc;
}
