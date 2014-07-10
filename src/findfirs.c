#include "includes.h"





APIRET EXPENTRY FS_FINDFIRST (
    struct cdfsi *pcdfsi,
    struct cdfsd *pcdfsd,
    PSZ		pName,
    USHORT	iCurDirEnd,
    USHORT	attr,
    struct fsfsi *pfsfsi,	/* not used */
    struct fsfsd *pfsfsd,
    PCHAR	pData,
    USHORT	cbData,
    PUSHORT	pcMatch,
    USHORT	level,
    USHORT	flags )
{
  int      rc;
  PVOLUME  pVolume;
  PSEARCH  pSearch;
  DIRENTRY Entry;
  FLAT     flatEntry;
  FLAT     flatBlkDir;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  BLOCK    BlkDir;
  PEAOP    pEAOP;

  UtilEnterRamfs();
  DEBUG_PRINTF4 ("FS_FINDFIRST  pName='%s' attr=%04X level=%d flags=%d",
		 pName, attr, level, flags);

  FSH_GETVOLPARM (pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  pVolume = pvpfsd->pVolume;
  flatBlkDir = pVolume->flatBlkRootDir;

  if(flatBlkDir == 0)
  {
    rc = ERROR_PATH_NOT_FOUND;
    goto end;
  }

  pName += 3;
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


    case LOC_FILEENTRY:
    case LOC_DIRENTRY:
	   /* exact name match */
	   pfsfsd->pSearch = 0;		/* no need to allocate a SEARCH */
	   if (UtilAttrMatch (attr, Entry.fDOSattr) != NO_ERROR)
	   {
	     /* attribute didn't match, don't return the entry */
	     rc = ERROR_NO_MORE_FILES;
	     break;
	   }

	   /* attribute is ok, try to return the entry */
	   pEAOP = NULL;

           if (*pcMatch == 0)
           {
             /* we aren't allowed to return even a single entry */
             rc = ERROR_BUFFER_OVERFLOW;
             break;
           }

	   if (level == FIL_QUERYEASFROMLIST)
	   {
	     /* Data starts with an EAOP - skip it, remembering pEAOP */
	     if (cbData < sizeof(EAOP))
	     {
	       rc = ERROR_BUFFER_OVERFLOW;
	       break;
	     }
	     pEAOP   = (PEAOP)pData;
	     pData  += sizeof(EAOP);
	     cbData -= sizeof(EAOP);
	   }
	   rc = FindStoreEntry (&pData, &cbData, level, flags, attr, pEAOP, &Entry);
           if (rc == NO_ERROR)
           {
             /* ok, stored one entry */
             *pcMatch = 1;
           }
	   break;


    case LOC_NOENTRY:
	   if (memchr (Entry.achName, '*', Entry.cbName) == 0  &&
	       memchr (Entry.achName, '?', Entry.cbName) == 0)
	   {
	     /* no exact match and no wildcards */
	     rc = ERROR_NO_MORE_FILES;
	     break;
	   }

	   /* wildcards present, allocate and build a SEARCH */
	   rc = NearAlloc ((PNEARBLOCK *) &pSearch, sizeof(SEARCH)+Entry.cbName);
	   if (rc)  break;
	   pSearch->flatBlkDir = flatBlkDir;
	   VMReadBlk (&BlkDir, flatBlkDir);
	   pSearch->flatEntry = BlkDir.flatAddr;
           pSearch->flatFallbackEntry = 0;
	   pSearch->usAttr = attr;
	   FSH_UPPERCASE (Entry.achName, Entry.cbName+1, pSearch->szPattern);
	   rc = FindEntries (pSearch, NULL, &Entry, pData, cbData, pcMatch, level, flags);
	   if (rc == NO_ERROR)
	   {
	     /* some files were found, link the SEARCH into the list of SEARCHes */
	     pfsfsd->pSearch = pSearch;
	     pSearch->pNextSearch = pVolume->pFirstSearch;
	     pVolume->pFirstSearch = pSearch;
	   }
	   else
	   {
	     /* no files were found, forget everything */
	     NearFree (pSearch);
	   }
	   break;
  }

end:

  DEBUG_PRINTF1 (" => %d\r\n", rc);
  UtilExitRamfs();
  return rc;
}
