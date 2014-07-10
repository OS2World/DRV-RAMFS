/* $Id: ldrmte.h,v 1.2 2003/11/07 07:46:20 root Exp $ */

/* OS/2 loader structures - derived from various SDF files */

#ifndef LDRMTE_INCLUDED
#define LDRMTE_INCLUDED

/* Swappable Module Table Entry */

struct ldrsmte_s
{
 ulong_t smte_mpages;                   /* 0x00 */
 ulong_t smte_startobj;                 /* 0x04 */
 ulong_t smte_eip;                      /* 0x08 */
 ulong_t smte_stackobj;                 /* 0x0c */
 ulong_t smte_esp;                      /* 0x10 */
 ulong_t smte_pageshift;                /* 0x14 */
 ulong_t smte_fixupsize;                /* 0x18 */
 ulong_t smte_objtab;                   /* 0x1c */
 ulong_t smte_objcnt;                   /* 0x20 */
 ulong_t smte_objmap;                   /* 0x24 */
 ulong_t smte_itermap;                  /* 0x28 */
 ulong_t smte_rsrctab;                  /* 0x2c */
 ulong_t smte_rsrccnt;                  /* 0x30 */
 ulong_t smte_restab;                   /* 0x34 */
 ulong_t smte_enttab;                   /* 0x38 */
 ulong_t smte_fpagetab;                 /* 0x3c */
 ulong_t smte_frectab;                  /* 0x40 */
 ulong_t smte_impmod;                   /* 0x44 */
 ulong_t smte_impproc;                  /* 0x48 */
 ulong_t smte_datapage;                 /* 0x4c */
 ulong_t smte_nrestab;                  /* 0x50 */
 ulong_t smte_cbnrestab;                /* 0x54 */
 ulong_t smte_autods;                   /* 0x58 */
 ulong_t smte_debuginfo;                /* 0x5c */
 ulong_t smte_debuglen;                 /* 0x60 */
 ulong_t smte_heapsize;                 /* 0x64 */
 ulong_t smte_path;                     /* 0x68 */
 ushort_t smte_semcount;                /* 0x6c */
 ushort_t smte_semowner;                /* 0x6e */
 ulong_t smte_pfilecache;               /* 0x70 */
 ulong_t smte_stacksize;                /* 0x74 */
 ushort_t smte_alignshift;              /* 0x78 */
 ushort_t smte_NEexpver;                /* 0x7a */
 ushort_t smte_pathlen;                 /* 0x7c */
 ushort_t smte_NEexetype;               /* 0x7e */
 ushort_t smte_csegpack;                /* 0x80 */
#ifdef AURORA
 uchar_t smte_major_os;                 /* 0x82 */
 uchar_t smte_minor_os;                 /* 0x83 */
#endif
};

#ifdef XR_M006

/* RAS MTE entry */

struct ldrRAS_s
{
 ulong_t RASmte_objtab;                 /* 0x00 */
 ulong_t RASmte_objcnt;                 /* 0x04 */
 ulong_t RASmte_rsrccnt;                /* 0x08 */
};

#endif

/* Object Table Entry */

struct ldrote_s
{
 ulong_t ote_size;                      /* 0x00 */
 ulong_t ote_base;                      /* 0x04 */
 ulong_t ote_flags;                     /* 0x08 */
 ulong_t ote_pagemap;                   /* 0x0c */
 ulong_t ote_mapsize;                   /* 0x10 */
 unsigned long reserved;                /* 0x14 - seems to be a union */
};


/* Module Table Entry */

struct ldrmte_s
{
 ushort_t mte_flags2;                   /* 0x00 */
 ushort_t mte_handle;                   /* 0x02 */
 struct ldrsmte_s *mte_swapmte;         /* 0x04 */
 struct ldrmte_s *mte_link;             /* 0x08 */
 ulong_t mte_flags1;                    /* 0x0c */
 ulong_t mte_impmodcnt;                 /* 0x10 */
 ushort_t mte_sfn;                      /* 0x14 */
 ushort_t mte_usecnt;                   /* 0x16 */
 char mte_modname[8];                   /* 0x18 */
#ifdef XR_M006
 struct ldrRAS_s *mte_RAS;              /* 0x20 */
 ulong_t mte_modver;                    /* 0x24 */
#endif
};

#endif
