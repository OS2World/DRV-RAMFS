/* $Id: ramfsutl.h,v 1.2 2003/11/07 07:46:20 root Exp $ */

#ifndef RAMFSUTL_INCLUDED
#define RAMFSUTL_INCLUDED

#ifdef MERGE
  #define GENMAIN(x) x##_main
#else
  #define GENMAIN(X) main
#endif

/* DosFSCtl macro for 16/32-bit */

#ifdef __32BIT__
  #define FSCtl(a, b, c, d, e, f, g, h, i, j) DosFSCtl(a, b, c, d, e, f, g, h, i, j)
#else
  #define FSCtl(a, b, c, d, e, f, g, h, i, j) DosFSCtl(a, (USHORT)b, (PUSHORT)c, d, (USHORT)e, (PUSHORT)f, g, h, i, j, 0)
#endif

/* Special RC */

#define BADSYNTAX   125         /* Suggests to display help after the error message */

#endif
