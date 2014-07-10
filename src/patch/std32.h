/* $Id: std32.h,v 1.2 2003/11/07 07:46:20 root Exp $ */

#ifndef STD32_INCLUDED
#define STD32_INCLUDED

extern unsigned long TKSSBase;
#define SSToDS(p) ((void *)(TKSSBase+(unsigned long)(p)))

#endif
