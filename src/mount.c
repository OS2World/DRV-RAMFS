#include "includes.h"





APIRET EXPENTRY FS_MOUNT (
    USHORT	flag,
    struct vpfsi *pvpfsi,
    struct vpfsd *pvpfsd,
    USHORT	hVPB,
    PCHAR	pBoot )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_MOUNT?  flag=%d", flag);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
