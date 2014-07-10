#include "includes.h"





APIRET EXPENTRY FS_SHUTDOWN (
    USHORT	usType,
    ULONG	ulReserved )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_SHUTDOWN  usType=%d", usType);

  rc = NO_ERROR;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
