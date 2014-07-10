#include "includes.h"



#define HEAPSIZE	49152u		/* must be a multiple of 32 */


struct block
{
  USHORT cbSize;
  char fFree;
  char data[1];
};

char near nearheap [HEAPSIZE];




void NearInit (void)
{
  struct block near *blk;

  blk = (struct block near *) nearheap;
  blk->cbSize = HEAPSIZE;
  blk->fFree = TRUE;
}




int NearAlloc (PNEARBLOCK _ss *ppBlock, USHORT cbSize)
{
  struct block near *cur;
  struct block near *best_block;
  struct block near *new_free;
  USHORT best_size;

  cbSize = (cbSize+0x0022) & ~0x001F;
  best_size = 0xFFFF;
  cur = (struct block near *) &nearheap[0];

  while ((USHORT)cur < (USHORT) &nearheap[HEAPSIZE])
  {
    if (cur->fFree)
    {
      if (cur->cbSize == cbSize)
      {
	/* found a free block of the right size, take it */
	cur->fFree = FALSE;
	*ppBlock = &cur->data[0];
	return NO_ERROR;
      }
      else if (cur->cbSize > cbSize  &&  cur->cbSize < best_size)
      {
	/* remember the smallest free block that is large enough */
	best_block = cur;
	best_size = cur->cbSize;
      }
    }

    cur = (struct block near *) ((USHORT)cur + cur->cbSize);
  }

  if (best_size == 0xFFFF)
    return ERROR_OUT_OF_STRUCTURES;	/* no free block large enough */

  /* take a part of the smallest free block that was large enough */
  new_free = (struct block near *) ((USHORT)best_block + cbSize);
  new_free->cbSize = best_size - cbSize;
  new_free->fFree = TRUE;
  best_block->cbSize = cbSize;
  best_block->fFree = FALSE;
  *ppBlock = &best_block->data[0];
  return NO_ERROR;
}




void NearFree (PNEARBLOCK pBlock)
{
  struct block near *blk;
  struct block near *prev;
  struct block near *cur;
  struct block near *next;

  blk = (struct block near *) ((USHORT)pBlock - 3);
  prev = 0;

  cur = (struct block near *) &nearheap[0];
  while ((USHORT)cur < (USHORT) &nearheap[HEAPSIZE])
  {
    if (blk == cur)
    {
#ifdef DEBUG
      if (cur->fFree)
      {
	debugging = TRUE;
	DEBUG_PRINTF1 ("\r\n!!! NearFree (0x%04X) -- block already free\r\n", pBlock);
	INT3;
	return;
      }
#endif
      cur->fFree = TRUE;
      next = (struct block near *) ((USHORT)cur + cur->cbSize);
      if ((USHORT)next < (USHORT) &nearheap[HEAPSIZE]  &&  next->fFree)
	/* next block is also free, join us with it */
	cur->cbSize += next->cbSize;

      if (prev  &&  prev->fFree)
	/* previous block is also free, join us with it */
	prev->cbSize += cur->cbSize;
      return;
    }

    prev = cur;
    cur = (struct block near *) ((USHORT)cur + cur->cbSize);
  }

#ifdef DEBUG
  debugging = TRUE;
  DEBUG_PRINTF1 ("\r\n!!! NearFree (0x%04X) -- invalid block\r\n", pBlock);
  INT3;
#endif
}
