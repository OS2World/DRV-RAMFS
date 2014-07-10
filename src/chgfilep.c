#include "includes.h"





APIRET EXPENTRY FS_CHGFILEPTR (
    struct sffsi *psffsi,
    struct sffsd *psffsd,	/* not used in retail */
    LONG	offset,
    USHORT	type,
    USHORT	IOflag )	/* not used */
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_CHGFILEPTR  sfn=%d, offset=%ld, type=%d",
		 psffsi->sfi_selfsfn, offset, type);

  if (psffsd->pOpenfile == NULL || psffsd->pOpenfile->flatEntry == 0)
  {
    /* Volume already detached */
    rc = ERROR_HANDLE_EOF;
    goto end;
  }

#ifdef DEBUG
  {
    FBLOCK fblkFile;

    VMRead (&fblkFile, psffsd->pOpenfile->flatEntry + FIELDOFFSET(DIRENTRY,fblkFile), sizeof(FBLOCK));
    if (fblkFile.fSize != psffsi->sfi_size)
    {
      debugging = TRUE;
      DEBUG_PRINTF2 ("\r\n!!! fblkFile.cbSize = %lu, sfi_size = %lu\r\n",
		     fblkFile.fSize, psffsi->sfi_size);
      INT3;
    }
  }
#endif

  switch (type)
  {
    case 0: /* relative to beginning of file */
	    psffsi->sfi_position = offset;
	    break;

    case 1: /* relative to current position */
	    psffsi->sfi_position += offset;
	    break;

    case 2: /* relative to end of file */
	    psffsi->sfi_position = psffsi->sfi_size + offset;
	    break;
  }

  rc = NO_ERROR;

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
