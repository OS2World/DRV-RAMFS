#include "includes.h"





APIRET EXPENTRY FS_FILEINFO (
    USHORT	flag,
    struct sffsi *psffsi,
    struct sffsd *psffsd,
    USHORT	level,
    PCHAR	pData,
    USHORT	cbData,
    USHORT	IOflag )	/* not used */
{
  int       rc;
  POPENFILE pOpenfile;
  DIRENTRY  Entry;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_FILEINFO  sfn=%d, flag=%d, level=%d",
		 psffsi->sfi_selfsfn, flag, level);

  pOpenfile = psffsd->pOpenfile;
  if (pOpenfile == NULL || pOpenfile->flatEntry == NULL)
  {
    /* Volume already detached */
    rc = ERROR_FILE_NOT_FOUND;
    goto end;
  }
  VMRead (&Entry, pOpenfile->flatEntry, sizeof(Entry)-CCHMAXPATHCOMP);

  if (flag == 0)
  {
    /* retrieve information */
    rc = InfoQuery (pData, cbData, level, &Entry);
  }
  else
  {
    /* set information */
    rc = InfoSet (pData, cbData, level, &Entry, psffsi);
    if (rc == NO_ERROR)
      VMWrite (pOpenfile->flatEntry, &Entry, sizeof(Entry)-CCHMAXPATHCOMP);
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
