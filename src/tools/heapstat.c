/* $Id: heapstat.c,v 1.2 2003/11/07 07:46:20 root Exp $ */

#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include "../ramfsctl.h"

#include "ramfsutl.h"

int GENMAIN(heapstat) (int argc, char **argv)
{
    APIRET              rc;
    struct heap_stats   stats;
    ULONG               cbData = sizeof(stats);

    rc = FSCtl(&stats, cbData, &cbData,
               NULL, 0, NULL,
               RAMFS_FSCTL_HEAP_STATS,
               RAMFS_NAME,
               (HFILE)-1,
               FSCTL_FSDNAME);
    if (!rc)
    {
        printf("RAMFS heap stats:\n"
               "  Currently used    %10lu (0x%08lx)\n"
               "  Maximum heap size %10lu (0x%08lx)\n"
               "  Number of blocks  %10lu (0x%08lx)\n"
               "  VMAlloc Calls     %10lu (0x%08lx)\n"
               "  VMFree Calls      %10lu (0x%08lx)\n",
               stats.cbHeapUsed, stats.cbHeapUsed,
               stats.cbHeapMax,  stats.cbHeapMax,
               stats.cVMBlocks,  stats.cVMBlocks,
               stats.cVMAllocs,  stats.cVMAllocs,
               stats.cVMFrees,   stats.cVMFrees);
    }
    else
    {
        printf("DosFSCtl failed with rc=%d\n", rc);
    }
    return rc != NO_ERROR;
}
