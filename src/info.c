#include "includes.h"


struct level1
{
  ULONG   datiCreate;
  ULONG   datiAccess;
  ULONG   datiWrite;
  ULONG   cbFile;
  ULONG   cbFileAlloc;
  USHORT  usAttr;
};


struct level2
{
  ULONG   datiCreate;
  ULONG   datiAccess;
  ULONG   datiWrite;
  ULONG   cbFile;
  ULONG   cbFileAlloc;
  USHORT  usAttr;
  ULONG   cbList;
};




int InfoQuery (PCHAR pData, USHORT cbData, int level, PDIRENTRY pEntry)
{
  struct level1 *pInfo;
  PEAOP    pEAOP;
  PFEALIST pFEAList;
  ULONG    cbEA;

  switch (level)
  {
    case FIL_QUERYEASIZE:	/* 2 */
	    /* special stuff for level 2 */
	    if (cbData < sizeof(struct level2))
	      return ERROR_BUFFER_OVERFLOW;
	    cbEA = pEntry->blkEA.cbSize;
	    if (cbEA)
	      cbEA += 4;
	    ((struct level2 *) pData)->cbList = cbEA;
	    /* fall through */

    case FIL_STANDARD:		/* 1 */
	    /* common stuff for level 1 and 2 */
	    if (cbData < sizeof(struct level1))
	      return ERROR_BUFFER_OVERFLOW;
	    pInfo = (struct level1 *) pData;
	    pInfo->datiCreate  = pEntry->datiCreate;
	    pInfo->datiAccess  = pEntry->datiAccess;
	    pInfo->datiWrite   = pEntry->datiWrite;
	    pInfo->cbFile      = pEntry->fblkFile.fSize;
	    pInfo->cbFileAlloc = ROUNDUP (pEntry->fblkFile.fSize);
	    if (pEntry->fDOSattr & DOSATTR_DIRECTORY)
	    {
	      pInfo->cbFile      = 0;
	      pInfo->cbFileAlloc = 0;
	    }
	    pInfo->usAttr      = pEntry->fDOSattr & ~(DOSATTR_NON83 | DOSATTR_NEEDEA);
	    break;


    case FIL_QUERYEASFROMLIST:	/* 3 */
	    if (cbData < sizeof(EAOP))
	      return ERROR_BUFFER_OVERFLOW;
	    pEAOP = (PEAOP) pData;
	    if (pEAOP->fpFEAList->cbList < 4)
	      return ERROR_BUFFER_OVERFLOW;

	    return EaGetList (pEAOP->fpFEAList, pEAOP, &pEntry->blkEA);


    case 4: /* return full EA set (undocumented, used by EAUTIL and COPY) */
	    if (cbData < sizeof(EAOP))
	      return ERROR_BUFFER_OVERFLOW;

	    pEAOP = (PEAOP) pData;
	    pFEAList = pEAOP->fpFEAList;
	    if (pEntry->blkEA.cbSize+4 > pFEAList->cbList)
	      return ERROR_BUFFER_OVERFLOW;

	    pFEAList->cbList = pEntry->blkEA.cbSize + 4;
	    VMRead (&pFEAList->list, pEntry->blkEA.flatAddr, (unsigned short)pEntry->blkEA.cbSize);
	    break;


    case FIL_QUERYFULLNAME:	/* 5 */
	    /* never happens, the kernel handles this itself */
	    return ERROR_NOT_SUPPORTED;


    case FIL_NAMEISVALID:	/* 6 */
	    /* verify pathname */
	    return ERROR_NOT_SUPPORTED;


    case 7: /* get case-preserved version of path+filename */
	    /* never happens since we haven't set LV7 in FS_ATTRIBUTE */
	    return ERROR_NOT_SUPPORTED;
  }

  return NO_ERROR;
}




int InfoSet (PCHAR pData, USHORT cbData, int level, PDIRENTRY pEntry, struct sffsi *psffsi)
{
  struct level1 *pData1;

  switch (level)
  {
    case 1: /* FIL_STANDARD */
	    if (cbData < sizeof(struct level1))
	      return ERROR_INSUFFICIENT_BUFFER;
	    pData1 = (struct level1 *) pData;
	    if (pData1->datiCreate)
	      pEntry->datiCreate = pData1->datiCreate;
	    if (pData1->datiAccess)
	      pEntry->datiAccess = pData1->datiAccess;
	    if (pData1->datiWrite)
	      pEntry->datiWrite  = pData1->datiWrite;
	    pEntry->fDOSattr = (pEntry->fDOSattr & (DOSATTR_NON83 | DOSATTR_DIRECTORY)) |
			       (pData1->usAttr  & ~(DOSATTR_NON83 | DOSATTR_DIRECTORY));
	    if (psffsi != NULL)
	    {
	      if (pData1->datiCreate)
	      {
		psffsi->sfi_ctime = (USHORT) (pData1->datiCreate >> 16);
		psffsi->sfi_cdate = (USHORT) (pData1->datiCreate);
		psffsi->sfi_tstamp = (psffsi->sfi_tstamp | ST_PCREAT) & ~ST_SCREAT;
	      }
	      if (pData1->datiAccess)
	      {
		psffsi->sfi_atime = (USHORT) (pData1->datiAccess >> 16);
		psffsi->sfi_adate = (USHORT) (pData1->datiAccess);
		psffsi->sfi_tstamp = (psffsi->sfi_tstamp | ST_PREAD) & ~ST_SREAD;
	      }
	      if (pData1->datiWrite)
	      {
		psffsi->sfi_mtime = (USHORT) (pData1->datiWrite >> 16);
		psffsi->sfi_mdate = (USHORT) (pData1->datiWrite);
		psffsi->sfi_tstamp = (psffsi->sfi_tstamp | ST_PWRITE) & ~ST_SWRITE;
	      }
	      psffsi->sfi_DOSattr = pEntry->fDOSattr;
	    }
	    return NO_ERROR;


    case 2: /* set EAs */
	    return EaAddList (&pEntry->blkEA, (PEAOP)pData);


    default:
	    return ERROR_NOT_SUPPORTED;
  }
}
