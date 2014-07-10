#include "includes.h"





APIRET EXPENTRY FS_OPENPAGEFILE (
    PULONG	pFlags,
    PULONG	pcMaxReq,
    PSZ		pName,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	usOpenMode,
    USHORT	usOpenFlag,
    USHORT	usAttr,
    ULONG	Reserved )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_OPENPAGEFILE?  sfn=%d pName='%s'",
		 psffsi->sfi_selfsfn, pName);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
