void BlockDirCopy (PVOLUME pVolume, FLAT flatDest, FLAT flatSrc, ULONG cbLen);
int  BlockAlloc (PBLOCK pBlk, ULONG cbSize);
int  BlockFree (PBLOCK pBlk);
int  BlockRealloc (PBLOCK pBlk, ULONG cbNewSize);
int  BlockReallocDir (PVOLUME pVolume, PBLOCK pBlk, ULONG cbNewSize);
int  BlockMakeEmptyDir (PBLOCK pBlk);

void read_fblocks (char far *dest, PFBLOCK fblk, unsigned long first, unsigned long len);
void write_fblocks (char far *src, PFBLOCK fblk, unsigned long first, unsigned long len);
int  chsize_helper (PFBLOCK fblk, unsigned long newsize);
int  fblock_init (PFBLOCK fblk);
void fblock_shutdown (PFBLOCK fblk);

/* The data pertinent to a position within a block */
struct bop
{
  unsigned long block_num;      /* Block number */
  unsigned long offset;         /* Offset from beginning of block */
  unsigned long block_size;     /* Size of block */
};


/* Variable-length clusters */
struct vlclu
{
  unsigned char multiplier;
  unsigned long count;          /* 0 = final block size */
};


#define MAX_CLUSTER_RANGES     16
#define MIN_MULTIPLIER          1
#define MAX_MULTIPLIER          8

/* Cluster size ranges */

extern struct vlclu blk_size[MAX_CLUSTER_RANGES+1];
