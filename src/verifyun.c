#include "includes.h"





APIRET EXPENTRY FS_VERIFYUNCNAME (
    USHORT	flag,
    PSZ		pName )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_VERIFYUNCNAME?  flag=%d  pName='%s'", flag, pName);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
