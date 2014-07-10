#include "includes.h"





APIRET EXPENTRY FS_FILELOCKS (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct filelock *pUnLockRange,
    struct filelock *pLockRange,
    ULONG	timeout,
    ULONG	flags )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_FILELOCKS?  sfn=%d, timeout=%lu, flags=%lu",
		 psffsi->sfi_selfsfn, timeout, flags);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
