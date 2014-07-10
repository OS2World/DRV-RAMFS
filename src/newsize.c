#include "includes.h"





APIRET EXPENTRY FS_NEWSIZE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    ULONG	len,
    USHORT	IOflag )	/* not used */
{
  int       rc;
  POPENFILE pOpenfile;
  FBLOCK    fblkFile;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_NEWSIZE  sfn=%d, len=%lu", psffsi->sfi_selfsfn, len);

  pOpenfile = psffsd->pOpenfile;

  if (pOpenfile == NULL || pOpenfile->flatEntry == NULL)
  {
    /* Volume already detached */
    rc = ERROR_HANDLE_EOF;
    goto end;
  }

  VMRead (&fblkFile, pOpenfile->flatEntry + FIELDOFFSET(DIRENTRY,fblkFile), sizeof(FBLOCK));

#ifdef DEBUG
  if (fblkFile.fSize != psffsi->sfi_size)
  {
    debugging = TRUE;
    DEBUG_PRINTF2 ("\r\n!!! fblkFile.cbSize = %lu, sfi_size = %lu\r\n",
		  fblkFile.fSize, psffsi->sfi_size);
    INT3;
  }
#endif

  rc = chsize_helper (&fblkFile, len);
  if (rc == NO_ERROR)
  {
    VMWrite (pOpenfile->flatEntry + FIELDOFFSET(DIRENTRY,fblkFile), &fblkFile, sizeof(FBLOCK));
    psffsi->sfi_size = len;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
