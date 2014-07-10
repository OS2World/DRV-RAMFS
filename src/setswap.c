#include "includes.h"





APIRET EXPENTRY FS_SETSWAP (
    struct sffsi *psffsi,
    struct sffsd *psffsd )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_SETSWAP?  sfn=%d", psffsi->sfi_selfsfn);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
