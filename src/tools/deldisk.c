/* $Id: deldisk.c,v 1.1 2006/04/01 09:51:35 andrew_belov Exp $ */

#define INCL_BASE
#include <os2.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ramfsutl.h"


int GENMAIN(deldisk) (int argc, char **argv)
{
  int rc;

  if (argc != 2  ||  !isalpha(argv[1][0])  ||  argv[1][1] != ':')
  {
    printf("Syntax error - drive letter required\n");
    return(BADSYNTAX);
  }

#ifdef __32BIT__
  if (argc == 2)
    rc = DosFSAttach (argv[1], "RAMFS", NULL, 0, FS_DETACH);
  else
    rc = DosFSAttach (argv[1], "RAMFS", argv[2], strlen(argv[2]+1), FS_DETACH);
#else
  if (argc == 2)
    rc = DosFSAttach (argv[1], "RAMFS", NULL, 0, FSDETACH, 0);
  else
    rc = DosFSAttach (argv[1], "RAMFS", argv[2], strlen(argv[2]+1), FSDETACH, 0);
#endif

  switch (rc)
  {
    case ERROR_INVALID_FSD_NAME:
           puts ("Error: IFS=RAMFS.IFS not loaded in CONFIG.SYS");
           return 1;

    case ERROR_INVALID_DRIVE:
    case ERROR_INVALID_PATH:
           puts ("Error: Invalid drive letter");
           return 1;

    case NO_ERROR:
           puts ("OK");
           return 0;

    default:
           puts ("DosFSAttach failed");
           return 1;
  }

  /* NOTREACHED */
}

