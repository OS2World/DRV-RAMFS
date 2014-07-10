#include "includes.h"



APIRET EXPENTRY FS_FINDFROMNAME (
    struct fsfsi *pfsfsi,	/* not used */
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    ULONG	position,	/* not used */
    PSZ		pName,
    USHORT	flags )
{
  DIRENTRY Entry;
  int      rc;
  BLOCK    blkDir;
  PSZ      pRefEntry = NULL;

  UtilEnterRamfs();
  DEBUG_PRINTF4 ("FS_FINDFROMNAME  level=%d, flags=%d, position=0x%lx, name='%s'", level, flags, position, pName);

  rc = ERROR_NO_MORE_FILES;
  if (pfsfsd->pSearch != 0 && pfsfsd->pSearch->flatEntry != SEARCH_END)
  {
    /* This code enables the findnext resumption hack for NETWKSTA.200.
       FIXME: scanning the list from the beginning every time results
       in a complexity of O(n^2) when retrieving directory contents. */
    if (pName != NULL && findnext_resume)
    {
      VMReadBlk (&blkDir, pfsfsd->pSearch->flatBlkDir);
      pfsfsd->pSearch->flatFallbackEntry = pfsfsd->pSearch->flatEntry;
      pfsfsd->pSearch->flatEntry = blkDir.flatAddr;
      pRefEntry = pName;
    }
    rc = FindEntries (pfsfsd->pSearch, pRefEntry, &Entry, pData, cbData, pcMatch, level, flags);
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
