#include "includes.h"





APIRET EXPENTRY FS_NMPIPE (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	OpType,
    union npoper *pOpRec,
    PCHAR	pData,
    PSZ		pName )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_NMPIPE?  sfn=%d, OpType=%d pName='%s'",
		 psffsi->sfi_selfsfn, OpType, pName);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
