#include "includes.h"



int EaGetList (PFEALIST pFEAList, PEAOP pEAOP, PBLOCK pblkEA)
{
  PUCHAR  pCurGEA;
  PUCHAR  pCurFEA;
  USHORT  cbRestFEA;
  USHORT  cbRestGEA;

  pCurFEA   = (PUCHAR) &pFEAList->list[0];
  cbRestFEA = (USHORT) pFEAList->cbList - sizeof(ULONG);

  pCurGEA = (PUCHAR) &pEAOP->fpGEAList->list[0];
  if (pEAOP->fpGEAList->cbList < sizeof(ULONG))
  {
    pEAOP->oError = 0;
    return ERROR_EA_LIST_INCONSISTENT;
  }
  cbRestGEA = (USHORT) pEAOP->fpGEAList->cbList - sizeof(ULONG);

  /* loop through all the EAs that we are asked to find */
  while (cbRestGEA)
  {
    USHORT cbName;
    USHORT cbGEA;
    USHORT cbFEA;
    FLAT   flatCurFEA;
    FLAT   flatEnd;

    cbName = pCurGEA[0];
    cbGEA  = cbName + 2;

    if (cbRestGEA < cbGEA)
    {
      pEAOP->oError = 0;
      return ERROR_EA_LIST_INCONSISTENT;
    }

    if (FSH_CHECKEANAME (1, cbName, &pCurGEA[1]) != NO_ERROR)
    {
      pEAOP->oError = (USHORT) pCurGEA - (USHORT) pEAOP->fpGEAList;
      return ERROR_INVALID_EA_NAME;
    }

    /* let's try to find this name in blkEA */
    /* tentatively build an FEA with zero-length value */
    cbFEA = cbName + 5;
    if (cbRestFEA < cbFEA)
    {
      pFEAList->cbList = 0;
      if (pblkEA->cbSize)
	pFEAList->cbList = pblkEA->cbSize + 4;
      return ERROR_BUFFER_OVERFLOW;
    }
    pCurFEA[0] = 0;		/* flags	*/
    pCurFEA[1] = cbName;	/* cbName	*/
    pCurFEA[2] = 0;		/* cbValue LSB	*/
    pCurFEA[3] = 0;		/* cbValue MSB	*/
    FSH_UPPERCASE (&pCurGEA[1], cbName+1, &pCurFEA[4]);

    /* scan through all FEAs in blkEA for the EA */
    flatCurFEA = pblkEA->flatAddr;
    flatEnd = flatCurFEA + pblkEA->cbSize;
    while (flatCurFEA < flatEnd)
    {
      USHORT cbCurName;
      USHORT cbCurValue;
      USHORT cbCurFEA;

      cbCurName  = VMReadUChar  (flatCurFEA+1);
      cbCurValue = VMReadUShort (flatCurFEA+2);
      cbCurFEA   = 5 + cbCurName + cbCurValue;
      if (cbCurName == cbName)
      {
	char szCurName[256];

	VMRead (szCurName, flatCurFEA+4, cbCurName);
	if (!memcmp (szCurName, &pCurFEA[4], cbCurName))
	{
	  /* this is the EA we're looking for, overwrite the tentative FEA
	     with this one */
	  if (cbRestFEA < cbCurFEA)
	  {
	    pFEAList->cbList = 0;
	    if (pblkEA->cbSize)
	      pFEAList->cbList = pblkEA->cbSize + 4;
	    return ERROR_BUFFER_OVERFLOW;
	  }
	  VMRead (pCurFEA, flatCurFEA, cbCurFEA);
	  cbFEA = cbCurFEA;
	  break;
	}
      }

      flatCurFEA += cbCurFEA;
    }

#ifdef DEBUG
    if (flatCurFEA > flatEnd)
    {
      debugging = TRUE;
      DEBUG_PRINTF3 ("\r\n!!! EaGetList  blkEA.flatAddr = 0x%08lX  flatCurFEA = 0x%08lX  flatEnd = 0x%08lX\r\n",
		     pblkEA->flatAddr, flatCurFEA, flatEnd);
      INT3;
    }
#endif

    pCurFEA   += cbFEA;
    cbRestFEA -= cbFEA;
    pCurGEA   += cbGEA;
    cbRestGEA -= cbGEA;
  }

  pFEAList->cbList -= cbRestFEA;

  return NO_ERROR;
}




/* static */
int add_ea (PBLOCK pblkEA, PFEA pFEA)
{
  FLAT   flatCurFEA;
  FLAT   flatEnd;
  USHORT cbFEA;
  char   szUpName[256];
  int    rc;

  FSH_UPPERCASE ((char *)pFEA + 4, sizeof(szUpName), szUpName);
  flatCurFEA = pblkEA->flatAddr;
  flatEnd    = flatCurFEA + pblkEA->cbSize;

  /* scan through all existing EAs for the file */
  while (flatCurFEA < flatEnd)
  {
    USHORT cbCurName;
    USHORT cbCurValue;
    USHORT cbCurFEA;

    cbCurName  = VMReadUChar (flatCurFEA+1);
    cbCurValue = VMReadUShort (flatCurFEA+2);
    cbCurFEA   = 5 + cbCurName + cbCurValue;
    if (cbCurName == pFEA->cbName)
    {
      char szCurName[256];

      VMRead (szCurName, flatCurFEA+4, cbCurName);
      if (!memcmp (szCurName, szUpName, cbCurName))
      {
	/* the EA already exists, delete it so that it can be replaced */
	VMCopy (flatCurFEA, flatCurFEA+cbCurFEA, flatEnd-flatCurFEA-cbCurFEA);
	rc = BlockRealloc (pblkEA, pblkEA->cbSize - cbCurFEA);
	if (rc)
	  return rc;
	break;
      }
    }
    flatCurFEA += cbCurFEA;
  }

  if (pFEA->cbValue != 0)
  {
    /* the new EA has a value, so add it to the list */
    cbFEA = 5 + pFEA->cbName + pFEA->cbValue;
    rc = BlockRealloc (pblkEA, pblkEA->cbSize + cbFEA);
    if (rc)
    {
      /* @@@ if the first BlockRealloc succeeded and this one failed, we are
	     returning an error code without having restored the state */
      return rc;
    }

    /* write the whole FEA with mixed-case name */
    VMWrite (pblkEA->flatAddr + pblkEA->cbSize - cbFEA, pFEA, cbFEA);
    /* replace name with the correct uppercased version */
    VMWrite (pblkEA->flatAddr + pblkEA->cbSize - cbFEA + 4, szUpName, pFEA->cbName);
  }
  return NO_ERROR;
}




int EaAddList (PBLOCK pblkEA, PEAOP pEAOP)
{
  UCHAR *pCurFEA;
  USHORT oError;
  USHORT cbRest;
  int    rc;

  if (FP_SEG(pEAOP) < 4)
    return NO_ERROR;

  pCurFEA = (UCHAR *) pEAOP->fpFEAList;
  cbRest = *(PUSHORT) pCurFEA;
  if (cbRest < 4)
  {
    pEAOP->oError = 0;
    return ERROR_EA_LIST_INCONSISTENT;
  }

  pCurFEA += sizeof(ULONG);
  cbRest  -= sizeof(ULONG);
  oError   = sizeof(ULONG);
  while (cbRest > 0)
  {
    USHORT cbName;
    USHORT cbValue;
    USHORT cbFEA;

    /* preliminary check of FEA size */
    if (cbRest < 6)
    {
      pEAOP->oError = oError;
      return ERROR_EA_LIST_INCONSISTENT;
    }

    /* check for invalid EA flag bits */
    if (pCurFEA[0] & 0x7F)
    {
      pEAOP->oError = oError;
      return ERROR_EA_LIST_INCONSISTENT;
    }

    /* check for invalid EA name */
    cbName = pCurFEA[1];
    rc = FSH_CHECKEANAME (1, cbName, &pCurFEA[4]);
    if (rc)
    {
      pEAOP->oError = oError;
      return ERROR_INVALID_EA_NAME;
    }

    /* final check of FEA size */
    cbValue = *(USHORT *) &pCurFEA[2];
    cbFEA = 5 + cbName + cbValue;
    if (cbRest < cbFEA)
    {
      pEAOP->oError = oError;
      return ERROR_EA_LIST_INCONSISTENT;
    }

    /* this FEA seems OK, try to add it */
    rc = add_ea (pblkEA, (PFEA) pCurFEA);
    if (rc)
    {
      pEAOP->oError = oError;
      return rc;
    }

    pCurFEA += cbFEA;
    cbRest  -= cbFEA;
    oError  += cbFEA;
  }

  return NO_ERROR;
}
