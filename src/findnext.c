#include "includes.h"





APIRET EXPENTRY FS_FINDNEXT (
    struct fsfsi *pfsfsi,	/* not used */
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    USHORT	flags )
{
  DIRENTRY Entry;
  int      rc;

  UtilEnterRamfs();
  DEBUG_PRINTF2 ("FS_FINDNEXT  level=%d, flags=%d", level, flags);

  rc = ERROR_NO_MORE_FILES;
  if (pfsfsd->pSearch != 0 && pfsfsd->pSearch->flatEntry != SEARCH_END)
  {
    rc = FindEntries (pfsfsd->pSearch, NULL, &Entry, pData, cbData, pcMatch, level, flags);
  }

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
