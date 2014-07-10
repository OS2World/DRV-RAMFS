#include "includes.h"





APIRET EXPENTRY FS_FINDNOTIFYFIRST (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	attr,
    PUSHORT	pHandle,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    ULONG	timeout )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF4 ("FS_FINDNOTIFYFIRST?  pName='%s' attr=0x%04X level=%d timeout=%lu",
		 pName, attr, level, timeout);

  rc = ERROR_NOT_SUPPORTED;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
