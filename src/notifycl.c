#include "includes.h"





APIRET EXPENTRY FS_FINDNOTIFYCLOSE (
    USHORT	handle )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_FINDNOTIFYCLOSE?  handle=%d", handle);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
