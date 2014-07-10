#include "includes.h"





APIRET EXPENTRY FS_DOPAGEIO (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    struct PageCmdHeader *pPageCmdList )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_DOPAGEIO?  sfn=%d", psffsi->sfi_selfsfn);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
