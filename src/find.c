#include "includes.h"



/* all info levels start with these fields */
struct common1
{
  ULONG  datiCreate;
  ULONG  datiAccess;
  ULONG  datiWrite;
  ULONG  cbFile;
  ULONG  cbFileAlloc;
  USHORT fDOSattr;
};


/* all info levels end with these fields */
struct common2
{
  UCHAR  cbName;
  UCHAR	 szName[1];
};


/* in FIL_QUERYEASIZE, these fields are between common1 and common2 */
struct easize
{
  ULONG cbList;
};




int FindStoreEntry (
    PCHAR   _ss *ppData,
    USHORT  _ss *pcbData,
    int          level,
    int          flag,
    USHORT       usAttrPattern,
    PEAOP        pEAOP,
    PDIRENTRY    pEntry )
{
  struct common1 *pCommon1;
  struct common2 *pCommon2;
  struct easize  *pEasize;
  PFEALIST pFEAList;
  int      cbRequired, cbNameAligned;
  int      rc;

  /* reserve space for all information except level-specific stuff - this
     is a preliminary buffer size check */
  if (flag == 0)
  {
    /* we shouldn't store ulPosition in front of returned data */
    cbRequired = sizeof(struct common1) +
		 sizeof(struct common2) + pEntry->cbName;

    if (*pcbData < cbRequired)
      return ERROR_BUFFER_OVERFLOW;
    *pcbData -= cbRequired;
  }
  else
  {
    /* HACK: this will account for DWORD alignment inside OS2KRNL (1 byte for
       attributes and 3 bytes for filename). Due to alignment, OS2KRNL accepts
       only part of a return buffer, and normally the FSD is expected to
       resume the search at the place where OS2KRNL tells it to, not where
       it has arrived previously. We ignore this hint from OS2KRNL, so a hack
       has been implemented -- AAB 16/06/2003 */
    cbNameAligned = (pEntry->cbName < 250) ? 5 : 0;
    /* we should store ulPosition in front of returned data */
    cbRequired = sizeof(ULONG) + cbNameAligned +
		 sizeof(struct common1) +
		 sizeof(struct common2) + pEntry->cbName;
    if (*pcbData < cbRequired)
      return ERROR_BUFFER_OVERFLOW;
    *pcbData -= cbRequired;

    /* we don't actually store ulPosition since we won't use it later anyway */
    *ppData += sizeof(ULONG);
  }

  /* store common file info */
  pCommon1 = (struct common1 *) *ppData;
  pCommon1->datiCreate  = pEntry->datiCreate;
  pCommon1->datiAccess  = pEntry->datiAccess;
  pCommon1->datiWrite   = pEntry->datiWrite;
  pCommon1->cbFile      = pEntry->fblkFile.fSize;
  /* BUGBUG: this isn't true anymore because the clusters != 4K and for large
     clusters, it's questionable whether the whole memory has been committed
     */
  pCommon1->cbFileAlloc = ROUNDUP (pEntry->fblkFile.fSize);
  if (pEntry->fDOSattr & DOSATTR_DIRECTORY)
  {
    pCommon1->cbFile = 0;
    pCommon1->cbFileAlloc = 0;
  }
  pCommon1->fDOSattr = pEntry->fDOSattr & ~(DOSATTR_NON83 | DOSATTR_NEEDEA);
  *ppData += sizeof(struct common1);

  switch (level)
  {
    case FIL_STANDARD:
	    /* level 1 only consists of common1 + common2 */
	    break;

    case FIL_QUERYEASIZE:
	    /* insert special level 2 stuff */
	    if (*pcbData < sizeof(struct easize))
	      return ERROR_BUFFER_OVERFLOW;
	    pEasize = (struct easize *) *ppData;
	    pEasize->cbList = 0;
	    if (pEntry->blkEA.cbSize)
	      pEasize->cbList = pEntry->blkEA.cbSize + 4;
	    *ppData  += sizeof(struct easize);
	    *pcbData -= sizeof(struct easize);
	    break;

    case FIL_QUERYEASFROMLIST:
	    /* insert special level 3 stuff using pEAOP */
	    if (*pcbData < 4)
	      return ERROR_BUFFER_OVERFLOW;
	    pFEAList = (PFEALIST) *ppData;
	    pFEAList->cbList = *pcbData;
	    rc = EaGetList (pFEAList, pEAOP, &pEntry->blkEA);
	    if (rc)
	      return rc;
	    *ppData  += (USHORT) pFEAList->cbList;
	    *pcbData -= (USHORT) pFEAList->cbList;
	    break;
  }

  /* store file name */
  pCommon2 = (struct common2 *) *ppData;
  pCommon2->cbName = pEntry->cbName;
  if (usAttrPattern & DOSATTR_NON83)
  {
    /* caller can cope with mixed-case names */
    memcpy (pCommon2->szName, pEntry->achName, pEntry->cbName+1);
  }
  else
  {
    /* MAYHAVE_NON83 wasn't set. DOS requires uppercased names. */
    FSH_UPPERCASE (pEntry->achName, pEntry->cbName+1, pCommon2->szName);
  }
  *ppData += sizeof(struct common2) + pEntry->cbName;

  return NO_ERROR;
}




int FindEntries (
    PSEARCH   pSearch,
    PSZ       pRefEntry,
    PDIRENTRY pEntry,
    PCHAR     pData,
    USHORT    cbData,
    PUSHORT   pcMatch,
    USHORT    level,
    USHORT    flags )
{
  USHORT cMatch;
  BLOCK	 blkDir;
  PEAOP  pEAOP;
  int    cbCurName;
  int    cbCurEntry;
  int    rc;

  if (pSearch->flatEntry == SEARCH_END)
    return ERROR_NO_MORE_FILES;

  pEAOP = NULL;
  if (level == FIL_QUERYEASFROMLIST)
  {
    /* buffer starts with an EAOP specifying EA names to return - skip it,
       remembering pEAOP */
    if (cbData < sizeof(EAOP))
      return ERROR_BUFFER_OVERFLOW;
    pEAOP   = (PEAOP)pData;
    pData  += sizeof(EAOP);
    cbData -= sizeof(EAOP);
  }

  VMReadBlk (&blkDir, pSearch->flatBlkDir);

  cMatch = 0;

#ifdef DEBUG
  if (pRefEntry != NULL && !findnext_resume)
  {
    debugging = TRUE;
    DEBUG_PRINTF1 ("\r\n!!! Find pRefEntry=%s, findnext_resume was not enabled!\r\n",
		   pRefEntry);
    INT3;
  }
#endif

fallback:                              /* For findnext_resume */
  while (pSearch->flatEntry < blkDir.flatAddr + blkDir.cbSize)
  {
    cbCurName = VMReadUChar (pSearch->flatEntry);
    cbCurEntry = sizeof(DIRENTRY)-CCHMAXPATHCOMP + cbCurName;
    VMRead (pEntry, pSearch->flatEntry, cbCurEntry);
    pEntry->achName [cbCurName] = '\0';

    if (UtilAttrMatch (pSearch->usAttr, pEntry->fDOSattr) == NO_ERROR)
    {
      /* attribute is ok, check if name also matches */
      char szCurUpName[256];

      FSH_UPPERCASE (pEntry->achName, sizeof(szCurUpName), szCurUpName);
      if (pRefEntry == NULL && FSH_WILDMATCH (pSearch->szPattern, szCurUpName) == NO_ERROR)
      {
	/* name also matches */
	if (cMatch == *pcMatch)
	  return NO_ERROR;	/* we aren't allowed to store more entries */

	rc = FindStoreEntry (&pData, &cbData, level, flags, pSearch->usAttr,
			     pEAOP, pEntry);
	if (rc == NO_ERROR)
	  cMatch++;
	else if (rc == ERROR_BUFFER_OVERFLOW  &&  cMatch != 0)
	{
	  /* found some entries, but not room to store this one */
	  *pcMatch = cMatch;
	  return NO_ERROR;
	}
	else
	  /* not room to store first entry, or other error */
	  return rc;
      }
    }
    if (pRefEntry != NULL && !_fstrcmp (pRefEntry, pEntry->achName))
      pRefEntry = NULL;
    pSearch->flatEntry += cbCurEntry;
  }

  /* Fallback hack -- 29/3/2006, 22:27. This is expected to provide a kind of
     workaround so that the search does not interrupt when requested to restart
     from a deleted file. */
  if (cMatch == 0 && pRefEntry != NULL && pSearch->flatFallbackEntry != SEARCH_END)
  {
#ifdef DEBUG
    if (pSearch->flatFallbackEntry == NULL)
    {
      debugging = TRUE;
      DEBUG_PRINTF4 ("\r\n!!! Find flatFallbackEntry=0x%08lX, flatEntry=0x%08lX, blkDir.flatAddr=0x%08lX, cbSize=0x%08lX\r\n",
		     pSearch->flatFallbackEntry, pSearch->flatEntry, blkDir.flatAddr, blkDir.cbSize);
      INT3;
    }
#endif
    pSearch->flatEntry = pSearch->flatFallbackEntry;
    pSearch->flatFallbackEntry = 0;
    pRefEntry = NULL;
    goto fallback;
  }

#ifdef DEBUG
  if (pSearch->flatEntry != blkDir.flatAddr + blkDir.cbSize)
  {
    debugging = TRUE;
    DEBUG_PRINTF3 ("\r\n!!! Find flatEntry=0x%08lX, blkDir.flatAddr=0x%08lX, cbSize=0x%08lX\r\n",
		   pSearch->flatEntry, blkDir.flatAddr, blkDir.cbSize);
    INT3;
  }
#endif

  pSearch->flatEntry = SEARCH_END;
  if (cMatch == 0)
    return ERROR_NO_MORE_FILES;

  *pcMatch = cMatch;
  return NO_ERROR;
}
