#include "includes.h"
#include "ramfsctl.h"



struct easizebuf
{
  USHORT cbMaxEASize;
  ULONG  cbMaxEAListSize;
};




/* static */
int dump_lists (PVOLUME pVolume)
{
  POPENFILE pCurOpenfile;
  PCURDIR   pCurCurdir;
  PSEARCH   pCurSearch;
#ifdef DEBUG
  char      old_debugging;

  old_debugging = debugging;
  debugging = TRUE;
#endif

  pCurOpenfile = pVolume->pFirstOpenfile;
  if (pCurOpenfile != 0)
  {
    DEBUG_PRINTF0 ("\r\n  Open files:\r\n");
    while (pCurOpenfile != 0)
    {
      DEBUG_PRINTF4 ("    0x%08lX  %c%c  %s\r\n",
		     pCurOpenfile->flatEntry,
		     pCurOpenfile->fShare & ACCESS_READ  ? 'R' : '-',
		     pCurOpenfile->fShare & ACCESS_WRITE ? 'W' : ' ',
		     (char *) pCurOpenfile->szName);
      pCurOpenfile = pCurOpenfile->pNextOpenfile;
    }
  }
  else
  {
    DEBUG_PRINTF0 ("\r\n  No open files\r\n");
  }

  pCurCurdir = pVolume->pFirstCurdir;
  if (pCurCurdir != 0)
  {
    DEBUG_PRINTF0 ("  Current directories:\r\n");
    while (pCurCurdir != 0)
    {
      DEBUG_PRINTF2 ("    0x%08lX  %s\r\n",
		     pCurCurdir->flatBlkDir,
		     (char *) pCurCurdir->szDir);
      pCurCurdir = pCurCurdir->pNextCurdir;
    }
  }
  else
  {
    DEBUG_PRINTF0 ("  No current directories\r\n");
  }

  pCurSearch = pVolume->pFirstSearch;
  if (pCurSearch != 0)
  {
    DEBUG_PRINTF0 ("  Open searches:\r\n");
    while (pCurSearch != 0)
    {
      DEBUG_PRINTF3 ("    0x%08lX  0x%08lX  %s\r\n",
		     pCurSearch->flatBlkDir,
		     pCurSearch->flatEntry,
		     (char *) pCurSearch->szPattern);
      pCurSearch = pCurSearch->pNextSearch;
    }
  }
  else
  {
    DEBUG_PRINTF0 ("  No open searches\r\n");
  }

#ifdef DEBUG
  debugging = old_debugging;
#endif

  return NO_ERROR;
}




/* static */
int dump_entry (PVOLUME pVolume, struct CD *pCD)
{
  int      rc;
  PSZ      pName;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
#ifdef DEBUG
  char     old_debugging;

  old_debugging = debugging;
  debugging = TRUE;
#endif

  pName = pCD->pPath;
  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;
  if (pCD->iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pCD->pcdfsd->pCurdir->flatBlkDir;
    pName += pCD->iCurDirEnd - 3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;

    case LOC_NOENTRY:
	   rc = ERROR_FILE_NOT_FOUND;
	   break;

    case LOC_DIRENTRY:
    case LOC_FILEENTRY:
	   DEBUG_PRINTF1 ("\r\nflatBlkDir = 0x%08lX\r\n", flatBlkDir);
	   DEBUG_PRINTF1 ("flatEntry  = 0x%08lX\r\n", flatEntry);
	   DEBUG_PRINTF1 ("  name = '%s'\r\n", (char *) Entry.achName);
	   DEBUG_PRINTF1 ("  dosattr = 0x%04X\r\n", Entry.fDOSattr);
	   DEBUG_PRINTF3 ("  fblkFile flatAddr = 0x%08lX  cbSize = %lu  clusters=%lu\r\n",
			  Entry.fblkFile.clusters.flatAddr, Entry.fblkFile.fSize, Entry.fblkFile.clusters.cbSize>>2);
	   DEBUG_PRINTF2 ("  blkEA    flatAddr = 0x%08lX  cbSize = %lu\r\n",
			  Entry.blkEA.flatAddr, Entry.blkEA.cbSize);
	   rc = NO_ERROR;
	   break;
  }

#ifdef DEBUG
  debugging = old_debugging;
#endif
  return rc;
}






APIRET EXPENTRY FS_FSCTL (
    union argdat *pArgDat,
    USHORT	iArgType,
    USHORT	func,
    PCHAR	pParm,
    USHORT	lenParm,
    PUSHORT	plenParmIO,
    PCHAR	pData,
    USHORT	lenData,
    PUSHORT	plenDataIO )
{
  int     rc;
  PVOLUME pVolume;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  struct easizebuf *pEASizeBuf;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_FSCTL  iArgType=%d func=0x%04X", iArgType, func);

  switch (iArgType)
  {
    case FSCTL_HANDLE:
	   rc = ERROR_NOT_SUPPORTED;
	   break;


    case FSCTL_PATHNAME:
	   FSH_GETVOLPARM (pArgDat->cd.pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
	   pVolume = pvpfsd->pVolume;

	   switch (func)
	   {
	     case 0x0002: /* return max supported EA and EA list sizes */
			  if (lenData < sizeof(struct easizebuf))
			  {
			    rc = ERROR_BUFFER_OVERFLOW;
			    break;
			  }
			  pEASizeBuf = (struct easizebuf *) pData;
			  pEASizeBuf->cbMaxEASize     = 65531L;
			  pEASizeBuf->cbMaxEAListSize = 65535L;
			  *plenDataIO = sizeof(struct easizebuf);
			  rc = NO_ERROR;
			  break;


	     case 0x8100: /* dump internal lists */
			  rc = dump_lists (pVolume);
			  break;


	     case 0x8101: /* dump Entry */
			  rc = dump_entry (pVolume, &pArgDat->cd);
			  break;


	     default:     /* */
			  rc = ERROR_NOT_SUPPORTED;
			  break;
	   }
	   break;


      case FSCTL_FSDNAME:
          switch (func)
          {
#ifdef MAX_HEAP
                case RAMFS_FSCTL_HEAP_STATS:
                {
                    struct heap_stats *pStats;
                    if (    lenData != sizeof(struct heap_stats)
                        ||  pData == NULL)
                    {
                         rc = ERROR_INVALID_PARAMETER;
                         break;
                    }
                    pStats = (struct heap_stats*)pData;
                    pStats->cbHeapMax  = gcbHeapMax;
                    pStats->cbHeapUsed = gcbHeapUsed;
                    pStats->cVMBlocks  = gcVMBlocks;
                    pStats->cVMAllocs  = gcVMAllocs;
                    pStats->cVMFrees   = gcVMFrees;
                    if (plenDataIO)
                        *plenDataIO = sizeof(struct heap_stats);
                    rc = NO_ERROR;
                    break;
                }

                case RAMFS_FSCTL_SET_MAX_HEAP:
                {
                    struct set_max_heap *   p;
                    unsigned long           cb;
                    if (    lenParm != sizeof(struct set_max_heap)
                        ||  pParm == NULL)
                    {
                         rc = ERROR_INVALID_PARAMETER;
                         break;
                    }

                    cb = ((struct set_max_heap *)pParm)->cbHeapMax;
                    if (   cb <= 0xc0000000
                        && cb >= 1024*1024)
                    {
                        gcbHeapMax = cb;
                        rc = NO_ERROR;
                    }
                    else
                        rc = ERROR_BUFFER_OVERFLOW;
                    break;
                }
#endif

              default:
                  rc = ERROR_NOT_SUPPORTED;
                  break;
          }
	  break;
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
