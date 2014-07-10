/* 'external' header for FSCTLS */

#define RAMFS_NAME      "RAMFS"

/* Note! 0x8000-0x8fff are reserved */

#define RAMFS_FSCTL_HEAP_STATS  0xa042
struct heap_stats
{
    unsigned long cbHeapMax;
    unsigned long cbHeapUsed;
    unsigned long cVMBlocks;
    unsigned long cVMAllocs;
    unsigned long cVMFrees;
};

#define RAMFS_FSCTL_SET_MAX_HEAP  0xa043
struct set_max_heap
{
    unsigned long cbHeapMax;
};
