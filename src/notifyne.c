#include "includes.h"





APIRET EXPENTRY FS_FINDNOTIFYNEXT (
    USHORT	handle,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	infolevel,
    ULONG	timeout )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_FINDNOTIFYNEXT?  handle=%d infolevel=%d timeout=%lu",
		 handle, infolevel, timeout);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
