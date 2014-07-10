#include "includes.h"





APIRET EXPENTRY FS_RMDIR (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd )
{
  int      rc;
  PVOLUME  pVolume;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  PSEARCH  pSearch;

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_RMDIR  pName='%s'", pName);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  if (pVolume == NULL)
  {
    /* Already detached */
    rc = NO_ERROR;
    goto end;
  }

  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;
  if (iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pName))
  {
    case LOC_NOPATH:
    case LOC_NOENTRY:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;


    case LOC_FILEENTRY:
	   rc = ERROR_ACCESS_DENIED;    /* Hack #1 -- AAB 21/10/2005 */ 
	   break;


    case LOC_DIRENTRY:
#ifdef DEBUG
	   if (Entry.fblkFile.clusters.cbSize < DIR_DOTSSIZE)
	   {
	     debugging = TRUE;
	     DEBUG_PRINTF1 ("\r\n!!! FS_RMDIR cbSize=%lu\r\n", Entry.fblkFile.clusters.cbSize);
	     INT3;
	   }
#endif

	   /* the dir should only contain . and .. */
	   if (Entry.fblkFile.clusters.cbSize != DIR_DOTSSIZE)
	   {
             /* Hack #2 -- AAB 21/10/2005. Both hacks are required to mimic the
                behavior of NETWKSTA.200. Local file systems return a different
                code for Hack #1. */
	     rc = ERROR_FILE_EXISTS;
	     break;
	   }

	   /* terminate any searches in progress in the dir */
	   pSearch = pVolume->pFirstSearch;
	   while (pSearch != 0)
	   {
	     if (pSearch->flatBlkDir == flatEntry+FIELDOFFSET(DIRENTRY,fblkFile))
	       pSearch->flatEntry = SEARCH_END;
	     pSearch = pSearch->pNextSearch;
	   }

	   BlockFree (&Entry.fblkFile.clusters);  /* AAB 29/09/2002 - perf regression */
	   BlockFree (&Entry.blkEA);
	   UtilDeleteEntry (pVolume, flatBlkDir, &Entry, flatEntry);
	   rc = NO_ERROR;
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
