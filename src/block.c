#include "includes.h"


#define MIN(a,b)    ((a) < (b) ? (a) : (b))

#define GRANULARITY    4096     /* Smallest allocation unit */


struct vlclu blk_size[MAX_CLUSTER_RANGES+1]=
{
  {1, 2},                       /* First 8K in 4K blocks */
  {4, 4},                       /* Then 16K up to 72K */
  {8, 0}                        /* Switch to 32K till the end */
};




void BlockDirCopy (PVOLUME pVolume, FLAT flatDest, FLAT flatSrc, ULONG cbLen)
{
  LONG      lDelta;
  FLAT      flatSrcEnd;
  POPENFILE pCurOpenfile;
  PCURDIR   pCurCurdir;
  PSEARCH   pCurSearch;

  if (cbLen == 0)
    return;

  VMCopy (flatDest, flatSrc, cbLen);

  flatSrcEnd = flatSrc + cbLen;
  lDelta = flatDest - flatSrc;

  pCurOpenfile = pVolume->pFirstOpenfile;
  while (pCurOpenfile != 0)
  {
    if (pCurOpenfile->flatEntry >= flatSrc  &&  pCurOpenfile->flatEntry < flatSrcEnd)
      pCurOpenfile->flatEntry += lDelta;
    pCurOpenfile = pCurOpenfile->pNextOpenfile;
  }

  pCurCurdir = pVolume->pFirstCurdir;
  while (pCurCurdir != 0)
  {
    if (pCurCurdir->flatBlkDir >= flatSrc  &&  pCurCurdir->flatBlkDir < flatSrcEnd)
      pCurCurdir->flatBlkDir += lDelta;
    pCurCurdir = pCurCurdir->pNextCurdir;
  }

  pCurSearch = pVolume->pFirstSearch;
  while (pCurSearch != 0)
  {
    if (pCurSearch->flatBlkDir >= flatSrc  &&  pCurSearch->flatBlkDir < flatSrcEnd)
      pCurSearch->flatBlkDir += lDelta;
    if (pCurSearch->flatEntry >= flatSrc  &&  pCurSearch->flatEntry < flatSrcEnd)
      pCurSearch->flatEntry += lDelta;
    pCurSearch = pCurSearch->pNextSearch;
  }
}




int BlockAlloc (PBLOCK pBlk, ULONG cbSize)
{
  FLAT flat;

  flat = 0;
  if (cbSize != 0)
  {
    flat = VMAlloc (FALSE, ROUNDUP (cbSize));
    if (flat == 0)
      return ERROR_DISK_FULL;
  }
  pBlk->flatAddr = flat;
  pBlk->cbSize = cbSize;
  return NO_ERROR;
}




int BlockFree (PBLOCK pBlk)
{
  if (pBlk->cbSize != 0)
    VMFree (pBlk->flatAddr, ROUNDUP(pBlk->cbSize));
  pBlk->flatAddr = 0;
  pBlk->cbSize = 0;
  return NO_ERROR;
}




int BlockRealloc (PBLOCK pBlk, ULONG cbNewSize)
{
  if (pBlk->cbSize == 0)
    return BlockAlloc (pBlk, cbNewSize);

  if (cbNewSize == 0)
    return BlockFree (pBlk);

  if (ROUNDUP(cbNewSize) != ROUNDUP(pBlk->cbSize))
  {
    FLAT flatNew;

    flatNew = VMAlloc (pBlk->cbSize > cbNewSize && cbNewSize < 0x00400000ul,
                       ROUNDUP (cbNewSize));
    if (flatNew == 0)
      return ERROR_DISK_FULL;
    VMCopy (flatNew, pBlk->flatAddr, MIN (pBlk->cbSize, cbNewSize));
    VMFree (pBlk->flatAddr, ROUNDUP(pBlk->cbSize));
    pBlk->flatAddr = flatNew;
  }

  pBlk->cbSize = cbNewSize;
  return NO_ERROR;
}




int BlockReallocDir (PVOLUME pVolume, PBLOCK pBlk, ULONG cbNewSize)
{

#ifdef DEBUG
  if (pBlk->cbSize < DIR_DOTSSIZE  ||  cbNewSize < DIR_DOTSSIZE)
  {
    debugging = TRUE;
    DEBUG_PRINTF2 ("\r\n!!! BlockReallocDir oldsize=%lu newsize=%lu\r\n",
		   pBlk->cbSize, cbNewSize);
    INT3;
  }
#endif

  if (ROUNDUP(cbNewSize) != ROUNDUP(pBlk->cbSize))
  {
    FLAT flatNew;

    flatNew = VMAlloc (pBlk->cbSize > cbNewSize, ROUNDUP (cbNewSize));
    if (flatNew == 0)
      return ERROR_DISK_FULL;
    BlockDirCopy (pVolume, flatNew, pBlk->flatAddr, MIN (pBlk->cbSize, cbNewSize));
    VMFree (pBlk->flatAddr, ROUNDUP(pBlk->cbSize));
    pBlk->flatAddr = flatNew;
  }

  pBlk->cbSize = cbNewSize;
  return NO_ERROR;
}




int BlockMakeEmptyDir (PBLOCK pBlk)
{
  DIRENTRY Entry;
  ULONG    datiNow;

  if (BlockAlloc (pBlk, DIR_DOTSSIZE))
    return ERROR_DISK_FULL;

  datiNow = UtilGetDateTime();

  Entry.fDOSattr = DOSATTR_DIRECTORY;
  fblock_init(&Entry.fblkFile);
  Entry.blkEA.flatAddr = 0;
  Entry.blkEA.cbSize = 0;
  Entry.datiCreate = datiNow;
  Entry.datiAccess = datiNow;
  Entry.datiWrite  = datiNow;

  /* make "." entry */
  Entry.cbName = 1;
  Entry.achName[0] = '.';
  VMWrite (pBlk->flatAddr, &Entry, sizeof(Entry)-CCHMAXPATHCOMP+1);

  /* make ".." entry */
  Entry.cbName = 2;
  Entry.achName[1] = '.';
  VMWrite (pBlk->flatAddr + sizeof(Entry)-CCHMAXPATHCOMP+1, &Entry,
	   sizeof(Entry)-CCHMAXPATHCOMP+2);

  return NO_ERROR;
}


static void byte_to_block (long b, struct bop *bop)
{
  unsigned long res;
  unsigned int  i = 0;
  unsigned long k, j = 0;

  res = b%GRANULARITY;          /* residue */
  b /= GRANULARITY;
  while (1)
  {
    k = b / blk_size[i].multiplier;
    if (k < blk_size[i].count || blk_size[i].count == 0)
    {
      bop->block_size = (unsigned long) blk_size[i].multiplier * GRANULARITY;
      bop->offset=res + (unsigned long) GRANULARITY * (b % blk_size[i].multiplier);
      bop->block_num  = k+j;
      break;
    }
    else
    {
      j += blk_size[i].count;
      b -= (long) blk_size[i].multiplier * blk_size[i].count;
    }
    i++;
  }
  /* bop should be filled at this point */
  return;
}




static unsigned long query_block_size (unsigned long num)
{
  unsigned int  i=0;

  while (num >= blk_size[i].count && blk_size[i].count != 0)
  {
    num -= blk_size[i].count;
    i++;
  }
  return ((unsigned long) GRANULARITY * blk_size[i].multiplier);
}




static FLAT get_blkaddr (FLAT blks, unsigned long blk_num)
{
  FLAT  rc;

  VMRead (&rc, blks + (blk_num << 2), 4);
  return rc;
}




static void set_blkaddr (FLAT blks, unsigned long blk_num, FLAT value)
{
   VMWrite (blks + (blk_num << 2), &value, 4);
}




void read_fblocks (char far *dest, PFBLOCK fblk, unsigned long first, unsigned long len)
{
  struct bop    bop;
  FLAT          addr;
  unsigned long fetch;

  while (len > 0)
  {
    byte_to_block(first, &bop);
    /* If the block is not present, panic! */
#ifdef DEBUG
    if (fblk->clusters.cbSize <= (bop.block_num << 2))
    {
      debugging = TRUE;
      DEBUG_PRINTF2 ("\r\n!!! read_fblocks() accesses block %lu outside of %lu bound\r\n",
                     bop.block_num, fblk->clusters.cbSize >> 2);
      INT3;
    }
#endif
    fetch = bop.block_size - bop.offset;
    if(fetch > len)
      fetch=len;
    addr=get_blkaddr (fblk->clusters.flatAddr, bop.block_num);
    DEBUG_PRINTF4 ("\r\nVMRead %lu bytes at blk#%lu (0x%08lx+%lu)", fetch, bop.block_num, addr, bop.offset);
    VMRead(dest, addr + bop.offset, (unsigned short)fetch);
    dest  += (unsigned short) fetch;
    first += fetch;
    len   -= fetch;
 }
}




void write_fblocks (char far *src, PFBLOCK fblk, unsigned long first, unsigned long len)
{
  struct bop    bop;
  FLAT          addr;
  unsigned long fetch;

  while (len > 0)
  {
    byte_to_block (first, &bop);
    /* If the block is not present, panic! */
#ifdef DEBUG
    if(fblk->clusters.cbSize<=(bop.block_num<<2))
    {
      debugging = TRUE;
      DEBUG_PRINTF2 ("\r\n!!! write_fblocks() accesses block %lu outside of %lu bound\r\n",
                     bop.block_num, fblk->clusters.cbSize>>2);
      INT3;
    }
#endif
    fetch = bop.block_size - bop.offset;
    if(fetch > len)
      fetch = len;
    addr = get_blkaddr (fblk->clusters.flatAddr, bop.block_num);
    DEBUG_PRINTF4 ("\r\nVMWrite %lu bytes at blk#%lu (0x%08lx+%lu)", fetch, bop.block_num, addr, bop.offset);
    VMWrite (addr + bop.offset, src, (unsigned short)fetch);
    src   += (unsigned short) fetch;
    first += fetch;
    len   -= fetch;
  }
}




int chsize_helper (PFBLOCK fblk, unsigned long newsize)
{
  struct bop    bop_old, bop_new;
  long oldcnt, newcnt; /* block count */
  long i;
  FLAT vma;

  byte_to_block (fblk->fSize, &bop_old);
  byte_to_block (newsize, &bop_new);
#ifdef DEBUG
  if(fblk->clusters.cbSize & 3)
  {
    debugging = TRUE;
    DEBUG_PRINTF1 ("\r\n!!! chsize_helper() detected a misaligned cluster map (size=0x%08lx)\r\n",
                   fblk->clusters);
    INT3;
  }
#endif
  oldcnt = fblk->clusters.cbSize >> 2;
  /* If the last block has 0 bytes, consider it to be empty */
  newcnt = bop_new.block_num + ((bop_new.offset == 0) ? 0 : 1);
  DEBUG_PRINTF3 ("\r\nchsize_helper()  newsize=%lu, oldcnt=%lu, newcnt=%lu",
	         newsize, oldcnt, newcnt);
  if(newcnt < oldcnt)
  {
    /* Drop unneeded clusters in case when we don't need more */
    for(i = newcnt; i < oldcnt; i++)
    {
      DEBUG_PRINTF2 ("\r\nVMFree blk#%lu (0x%08lx)", i, get_blkaddr (fblk->clusters.flatAddr, i));
      if(fblk->clusters.flatAddr)
      {
        VMFree (get_blkaddr(fblk->clusters.flatAddr, i), query_block_size (i));
        set_blkaddr(fblk->clusters.flatAddr, i, NULL);
      }
    }
    if (BlockRealloc (&fblk->clusters, newcnt << 2))
      return ERROR_DISK_FULL;
  }
  else if (newcnt > oldcnt)
  {
    if (BlockRealloc (&fblk->clusters, newcnt << 2))
      return ERROR_DISK_FULL;
    /* Allocate new clusters */
    for (i = oldcnt; i < newcnt; i++)
    {
      if ((vma = VMAlloc (FALSE, query_block_size (i))) == NULL)
      {
         /* Rollback to prevent memory leaks */
         while (--i >= oldcnt)
         {
           VMFree (get_blkaddr(fblk->clusters.flatAddr, i), query_block_size (i));
           set_blkaddr(fblk->clusters.flatAddr, i, NULL);
         }
         BlockRealloc (&fblk->clusters, oldcnt << 2);
         return ERROR_DISK_FULL;
      }
      set_blkaddr(fblk->clusters.flatAddr, i, vma);
      DEBUG_PRINTF3 ("\r\nVMAlloc %lu bytes for blk#%lu = 0x%08lx", query_block_size (i), i, get_blkaddr (fblk->clusters.flatAddr, i));
    }
  }
  /* Set the new file size */
  fblk->fSize = newsize;
  return NO_ERROR;
}




int fblock_init (PFBLOCK fblk)
{
  fblk->fSize = 0;
  DEBUG_PRINTF1 ("\r\nfblock_init(), new block at 0x%08lx", fblk);
  return (BlockAlloc (&fblk->clusters, 0));
}




void fblock_shutdown (PFBLOCK fblk)
{
  DEBUG_PRINTF1 ("\r\nfblock_shutdown(), launching chsize_helper(0x%08lx, 0) for cleanup", fblk);
  chsize_helper (fblk, 0);
  BlockFree (&fblk->clusters);
  fblk->fSize=0;
}
