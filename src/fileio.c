#include "includes.h"





APIRET EXPENTRY FS_FILEIO (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    PCHAR	pCmdList,	/* not actually PCHAR */
    USHORT	cbCmdList,
    PUCHAR	poError,
    USHORT	IOflag )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_FILEIO?  sfn=%d", psffsi->sfi_selfsfn);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
