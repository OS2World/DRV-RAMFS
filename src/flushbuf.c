#include "includes.h"





APIRET EXPENTRY FS_FLUSHBUF (
    USHORT	hVPB,
    USHORT	flag )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_FLUSHBUF?  flag=%d", flag);

  rc = NO_ERROR;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
