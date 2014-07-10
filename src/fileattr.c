#include "includes.h"





APIRET EXPENTRY FS_FILEATTRIBUTE (
    USHORT	flag,
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    PUSHORT	pAttr )
{
  int      rc;
  PVOLUME  pVolume;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  UtilEnterRamfs();
  DEBUG_PRINTF3 ("FS_FILEATTRIBUTE  flag=%d, *pAttr=0x%02X, pName='%s'",
		 flag, *pAttr, pName);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;
  flatBlkDir = pVolume->flatBlkRootDir;
  pName += 3;

  if (pName[0] == '\0'  &&  flag == 0)
  {
    /* get attribute of root */
    *pAttr = DOSATTR_DIRECTORY;
    rc = NO_ERROR;
    goto end;
  }

  if (iCurDirEnd != 0xFFFF)
  {
    flatBlkDir = pcdfsd->pCurdir->flatBlkDir;
    pName += iCurDirEnd-3;
  }

  switch (UtilLocate (&flatBlkDir, &flatEntry, &Entry, pName))
  {
    case LOC_NOPATH:
	   rc = ERROR_PATH_NOT_FOUND;
	   break;

    case LOC_NOENTRY:
	   rc = ERROR_FILE_NOT_FOUND;
	   break;

    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   if (flag == 0)
	   {
	     /* get attribute */
	     *pAttr = Entry.fDOSattr & ~DOSATTR_NON83;
	   }
	   else
	   {
	     /* set attribute */
	     Entry.fDOSattr = (Entry.fDOSattr & (DOSATTR_DIRECTORY | DOSATTR_NON83 | DOSATTR_NEEDEA))
			    | (*pAttr & ~(DOSATTR_DIRECTORY | DOSATTR_NON83 | DOSATTR_NEEDEA));
	     VMWrite (flatEntry + FIELDOFFSET(DIRENTRY,fDOSattr),
		      &Entry.fDOSattr, sizeof(Entry.fDOSattr));
	   }
	   rc = NO_ERROR;
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
