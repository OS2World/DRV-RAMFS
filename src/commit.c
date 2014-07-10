#include "includes.h"





APIRET EXPENTRY FS_COMMIT (
    USHORT	type,
    USHORT	IOflag,
    struct sffsi *psffsi,
    struct sffsd *psffsd )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_COMMIT?  sfn=%d, type=%d", psffsi->sfi_selfsfn, type);

  rc = NO_ERROR;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
