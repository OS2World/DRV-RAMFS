#include "includes.h"



APIRET EXPENTRY FS_ALLOCATEPAGESPACE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    ULONG	lSize,
    ULONG	lWantContig )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_ALLOCATEPAGESPACE?  sfn=%d, lSize = %lu",
		 psffsi->sfi_selfsfn, lSize);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
