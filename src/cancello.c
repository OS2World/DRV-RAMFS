#include "includes.h"





APIRET EXPENTRY FS_CANCELLOCKREQUEST (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct filelock pLockRange )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_CANCELLOCKREQUEST?  sfn=%d", psffsi->sfi_selfsfn);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
