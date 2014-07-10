#include "includes.h"



unsigned long ramfs_units = 16384UL;



APIRET EXPENTRY FS_FSINFO (
    USHORT	flag,
    USHORT	hVPB,
    PCHAR	pData,
    USHORT	cbData,
    USHORT	level )
{
  int rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  PVOLUME pVolume;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_FSINFO  flag=%d, level=%d", flag, level);

  FSH_GETVOLPARM (hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  switch (level)
  {
    case 1: /* query/set sector and cluster information */
	    if (flag == 0)
	    {
	      /* query sector and cluster information */
	      PFSALLOCATE pFsAllocate;

	      if (cbData < sizeof(FSALLOCATE))
	      {
		rc = ERROR_BUFFER_OVERFLOW;
		break;
	      }
	      pFsAllocate = (PFSALLOCATE) pData;
	      pFsAllocate->idFileSystem = 0;
	      pFsAllocate->cSectorUnit = 8;
	      pFsAllocate->cUnit = ramfs_units;
	      pFsAllocate->cUnitAvail = ramfs_units;
	      pFsAllocate->cbSector = 512;
	      rc = NO_ERROR;
	    }
	    else
	      rc = ERROR_NOT_SUPPORTED;
	    break;


    case 2: /* query/set volume label */
	    if (flag == 0)
	    {
	      /* query volume label */
	      PFSINFO pFsInfo;

	      if (cbData < sizeof(FSINFO))
	      {
		rc = ERROR_BUFFER_OVERFLOW;
		break;
	      }

	      pFsInfo = (PFSINFO) pData;
	      memset (pFsInfo, 0, sizeof(FSINFO));
	      pFsInfo->vol.cch = strlen (pVolume->szLabel);
	      strcpy (pFsInfo->vol.szVolLabel, pVolume->szLabel);
	      rc = NO_ERROR;
	    }

	    else if (flag == 1)
	    {
	      /* set volume label */
	      PVOLUMELABEL pLabel;

	      pLabel = (PVOLUMELABEL) pData;
	      memcpy (pVolume->szLabel, pLabel->szVolLabel, sizeof(pVolume->szLabel));
	      pVolume->szLabel [sizeof(pVolume->szLabel)-1] = '\0';
	      rc = NO_ERROR;
	    }
	    break;
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
