#include "includes.h"





APIRET EXPENTRY FS_IOCTL (
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	cat,
    USHORT	func,
    PCHAR	pParm,
    USHORT	lenMaxParm,
    PUSHORT	plenInOutParm,
    PCHAR	pData,
    USHORT	lenMaxData,
    PUSHORT	plenInOutData )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_IOCTL  sfn=%d cat=%04X func=%04X",
		 psffsi->sfi_selfsfn, cat, func);

  rc = NO_ERROR;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
