/* $Id: ramdisk.c,v 1.2 2003/11/07 07:46:20 root Exp $ */

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <string.h>

#include "ramfsutl.h"

int GENMAIN(ramdisk) (int argc, char **argv)
{
  int rc;

  if (argc < 2)
  {
#ifdef MERGE
    return(0);                          /* Suppress the error blurb */
#else
    return(BADSYNTAX);
#endif
  }

#ifdef MERGE
  printf ("Creating RAM disk %s\n", argv[1]);
#else
  puts ("RAMFS volume created");
#endif

#ifdef __32BIT__
  if (argc == 2)
    rc = DosFSAttach (argv[1], "RAMFS", NULL, 0, FS_ATTACH);
  else
    rc = DosFSAttach (argv[1], "RAMFS", argv[2], strlen(argv[2]+1), FS_ATTACH);
#else
  if (argc == 2)
    rc = DosFSAttach (argv[1], "RAMFS", NULL, 0, FSATTACH, 0);
  else
    rc = DosFSAttach (argv[1], "RAMFS", argv[2], strlen(argv[2]+1), FSATTACH, 0);
#endif

  switch (rc)
  {
    case ERROR_INVALID_FSD_NAME:
           puts ("Error: IFS=RAMFS.IFS not loaded in CONFIG.SYS");
           return 1;

    case ERROR_INVALID_PATH:
           puts ("Error: Invalid drive letter");
           return 1;

    case ERROR_ALREADY_ASSIGNED:
           puts ("Error: This drive letter is already in use");
           return 1;
 
    case NO_ERROR:
           puts ("OK");
           return 0;

    default:
           puts ("DosFSAttach failed");
           return 1;
  }
}
