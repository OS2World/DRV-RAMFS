#include "includes.h"



APIRET EXPENTRY FS_DELETE (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		 pFile,
    USHORT	 iCurDirEnd )
{
  int       rc;
  PVOLUME   pVolume;
  DIRENTRY  Entry;
  FLAT      flatEntry;
  FLAT      flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
#ifdef DEBUG
  POPENFILE pCurOpenfile;
#endif

  UtilEnterRamfs();
  DEBUG_PRINTF1 ("FS_DELETE  pFile='%s'", pFile);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;

  if (pVolume == NULL || pVolume->flatBlkRootDir == 0)
  {
    /* Already detached */
    rc = NO_ERROR;
    goto end;
  }

  flatBlkDir = pVolume->flatBlkRootDir;
  pFile += 3;
  if (iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pFile += iCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pFile))
  {
    case LOC_NOPATH:	   /* bad path */
	   rc = ERROR_PATH_NOT_FOUND;
	   break;

    case LOC_DIRENTRY:	   /* attempted to delete a directory */
	   rc = ERROR_ACCESS_DENIED;
	   break;

    case LOC_NOENTRY:	   /* file not found */
	   rc = ERROR_FILE_NOT_FOUND;
	   break;

    case LOC_FILEENTRY:	   /* file exists */
#ifdef DEBUG
	   /* check for any open instances of this file.
	      This isn't necessary because OS/2 only calls FS_DELETE if there
	      is no open instances. */
	   pCurOpenfile = pVolume->pFirstOpenfile;
	   while (pCurOpenfile != 0)
	   {
	     if (pCurOpenfile->flatEntry == flatEntry)
	     {
	       /* found an open instance of the file */
	       debugging = TRUE;
	       DEBUG_PRINTF1 ("\r\n!!! FS_DELETE Attempt to delete open file %s\r\n",
			      pCurOpenfile->szName);
	       INT3;
	       rc = ERROR_SHARING_VIOLATION;
	       goto end;
	     }
	     pCurOpenfile = pCurOpenfile->pNextOpenfile;
	   }
#endif

	   if (Entry.fDOSattr & DOSATTR_READONLY)
	   {
	     /* attempted to delete a read-only file */
	     rc = ERROR_ACCESS_DENIED;
	     break;
	   }

	   UtilDeleteEntry (pVolume, flatBlkDir, &Entry, flatEntry);
	   fblock_shutdown (&Entry.fblkFile);
	   BlockFree (&Entry.blkEA);
	   rc = NO_ERROR;
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
