#include "includes.h"





APIRET EXPENTRY FS_COPY (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pSrc,
    USHORT	iSrcCurDirEnd,
    PSZ		pDst,
    USHORT	iDstCurDirEnd,
    USHORT	nameType )
{
  int rc;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_COPY?  nametype=%d pSrc='%s' pDst='%s'",
		 nameType, pSrc, pDst);

  rc = ERROR_CANNOT_COPY;

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
