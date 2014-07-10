/* $Id: maxheap.c,v 1.2 2003/11/07 07:46:20 root Exp $ */

#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ramfsctl.h"

#include "ramfsutl.h"

int ramfs_set_max_heap(unsigned long cbHeapMax)
{
    APIRET              rc;
    struct set_max_heap max;
    ULONG               cbParam = sizeof(max);

    max.cbHeapMax = cbHeapMax;
    rc = FSCtl(NULL, 0, NULL,
               &max, cbParam, &cbParam,
               RAMFS_FSCTL_SET_MAX_HEAP,
               RAMFS_NAME,
               (HFILE)-1,
               FSCTL_FSDNAME);
    if (!rc)
        printf("DosFSCtl successfully set max heap size to %lu (0x%08lx)\n",
               cbHeapMax, cbHeapMax);
    else
        printf("DosFSCtl failed with rc=%d\n", rc);
    return rc;
}


int GENMAIN(maxheap) (int argc, char **argv)
{
    char               *pszEnd = NULL;
    int                 cShift;
    APIRET              rc;
    int                 i;
    unsigned long       cbHeapMax;

    if (    argc != 2
        ||  argv[1][0] > '9'
        ||  argv[1][0] < '0')
    {
        printf("Syntax error - invalid heap size parameter\n");
        return(BADSYNTAX);
    }

    cbHeapMax = strtoul(argv[1], &pszEnd, 10);
    if (cbHeapMax == 0)
    {
        printf("Syntax error - heap cannot be restrained to zero\n");
        return(BADSYNTAX);
    }

    cShift = 0;
    if (pszEnd && *pszEnd)
    {
        switch (*pszEnd)
        {
            case 'k':
            case 'K':
                cShift = 10;
                break;

            case 'm':
            case 'M':
                cShift = 20;
                break;

            case 'g':
            case 'G':
                cShift = 30;
                break;

            default:
                printf("Syntax error - invalid size postfix\n");
                return BADSYNTAX;
        }
    }

    if (    !(cbHeapMax << cShift)
        ||  !(cbHeapMax << cShift) > 0xc0000000)
    {
        printf("Error: size is out of range, max size is 3G.\n");
        return 1;
    }
    cbHeapMax <<= cShift;

    rc = ramfs_set_max_heap(cbHeapMax);

    return rc != NO_ERROR;
}

