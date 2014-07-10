/* #define DEBUG */

#define INCL_DOSINFOSEG
#define INCL_ERRORS
#include <os2.h>

#define	 APIRET		USHORT

#include "fsd.h"
#include "fsh.h"
#include "ramfs.h"
#include "util.h"
#include "block.h"
#include "ea.h"
#include "nearheap.h"
#include "vmheap.h"
#include "find.h"
#include "info.h"

#include <string.h>
#include <dos.h>
