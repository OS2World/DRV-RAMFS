/* $Id: kernfix.c,v 1.3 2003/11/26 21:06:04 root Exp $ */

/*
 * Misc. kernel hotfixes required to make RAMFS going smoothly. Of course, all
 * of them can be abandoned if desired, see the documentation for relevant
 * options.
 *
 * [1] Allow using NPX registers in FSDs. -- AAB 15/10/2002
 *
 *     Background: the kernel traps with MMX-based drivers if a user-mode program:
 *     
 *     1. Executes FP code (including the LIBCS.DLL setup routines),
 *        AND
 *     2. Operates with files on RAMFS.
 *     
 *     The trap occurs during program termination in SaveNPX (14.091b_W4):
 *     
 *     (ebx is zero)
 *     0178:fff19dbe 8b9b38020000   mov       ebx,dword ptr [ebx+00000238]
 *     0178:fff19dc4 66833dccf6dbff02 cmp       word ptr [ffdbf6cc],+02
 *     0178:fff19dcc 7505           jnz       fff19dd3
 *     0178:fff19dce 0fae03         fxsave    dword ptr [ebx]
 *     0178:fff19dd1 eb02           jmp       fff19dd5
 *     0178:fff19dd3 dd33           fsave     byte ptr [ebx]
 *     0178:fff19dd5 32c0           xor       al,al
 *     0178:fff19dd7 e6f0           out       f0,al
 *     0178:fff19dd9 66c70562addbff0000 mov       word ptr [NPX_Owner (ffdbad62)],0000
 *     0178:fff19de2 07             pop     es
 *     0178:fff19de3 5e             pop       esi
 *     0178:fff19de4 59             pop       ecx
 *     
 *     Seems like the kernel is trying to save the NPX context for a thread that is
 *     being cleaned up. And yes, the FSD _does_ perform the necessary fsave/frstor
 *     around the memory transfers.
 *     
 *     Mangling with CR0 TS/EM flags would slow down the system to a freeze, so not
 *     letting the kernel to know about NPX transactions in the FSD is a wrong way.
 *     
 *     The correct workaround was to cut in the kernel code above the failing
 *     instruction:
 *     
 *     0178:fff19dbb 8B1C98         mov       ebx,[eax][ebx]*4
 *     0178:fff19dbe 8B9B38020000   mov       ebx,[ebx][000000238]
 *     
 *     And replace it with a far call to our routine (to fit in 9 bytes):
 *     
 *     0178:fff19dbb 9Axxxxxxxxxxxx call      far32 ptr _savenpx_override
 *     0178:fff19dbe 7215           jc        fff19dd9
 *     
 *     See the ASM file for the routine body.
 *     
 * [2] Partially revert to 14.088d DBCS logic. -- AAB 09/06/2003
 *
 *     Background: due to some issues with Far Eastern language support, the
 *     late Aurora kernels only tolerate high-ASCII characters in pairs when
 *     checking UNC/remote filenames. This affects both RAMFS and NETWKSTA.200.
 *     The problem only occurs with kernel revisions from 14.089 to 14.095,
 *     inclusively.
 *
 *     In particular, the show-stoppers are:
 *     1. ScanPathChar <- UNC_Canonicalize
 *     2. PathPref
 *     3. CopyComponent
 *     x. ValidateCDS (?) - no degradation seen yet
 *
 *     The fixes brought in by this driver basically cancel the new validation
 *     logic. All known DBCS checks are bypassed, so the filenames may be
 *     composed of an odd number of high-ASCII characters, as used to.
 *
 */

#include <stddef.h>

#include "std32.h"

#include "ldrtypes.h"
#include "ldrmte.h"

#include "patchram.h"

/* Imported stuff */

struct ldrmte_s * locate_krnl_mte();
extern unsigned long npx_tcb_base;
extern short han_zi;
void savenpx_override();
unsigned short get_cs();

/*
 * NPX hooks
 */

/* SaveNPX */

struct locator os2krnl_savenpx[]=
{
 {0, 1, 0x50},                          /* Entry */
 {1, 1, 0x51},
 {1, 1, 0x56},
 {1, 1, 0x06},
 {1, 1, 0x0F},
 {1, 1, 0x06},
 {4, 12, 0x0F},                         /* movzx ebx, bx */
 {1, 1, 0xB7},
 {1, 1, 0xDB},
 {1, 16, 0xA1},                         /* mov eax, ... _papTCBSlots */
 {3, 16, 0x8B},                         /* mov ebx, dword ptr [eax+ebx*4] */
 {1, 1, 0x1C},
 {1, 1, 0x98},
 {1, 1, 0x8B},                          /* mov ebx,                     */
 {1, 1, 0x9B},                          /*          dword ptr [ebx+...] */
 /* 6 bytes (incl. previous 2 bytes) to cut in here! */
 {3, 1, 0x00},                          /* The structures are unlikely */
 {1, 1, 0x00},                          /* to exceed 65536 bytes! */
 /* Cut-in area ends! Next goes a Katmai branch which is kernel specific
    (older kernels don't care about SSE), and some gap should be accounted
    for. */
 {3, 127, 0x66},                        /* Landing area */
 {1, 1, 0xC7},                          /* Store immediate value: */
 {6, 1, 0x00},                          /* 00 */
 {1, 1, 0x00},                          /* 00 */
 {1, 32, 0x07},                         /* Exit sequence */
 {1, 1, 0x5E},
 {1, 1, 0x59},
 {1, 1, 0x58},
 {-1, -1, 0} 
};

/* Patch area */

struct locator os2krnl_npx_takeoff[]=
{
 {0, 1, 0x8B},                          /* mov ebx, dword ptr [eax+ebx*4] */
 {1, 1, 0x1C},
 {1, 1, 0x98},
 {1, 1, 0x8B},                          /* mov ebx,                     */
 {1, 1, 0x9B},                          /*          dword ptr [ebx+...] */
 /* 6 bytes (incl. previous 2 bytes) to cut in here! */
 {3, 1, 0x00},                          /* The structures is unlikely */
 {1, 1, 0x00},                          /* to exceed 65536 bytes! */
 {-1, -1, 0} 
};

/* Landing area */

struct locator os2krnl_npx_landing[]=
{
 {0, 1, 0x66},                          /* 386 */
 {1, 1, 0xC7},                          /* Store immediate value: */
 {6, 1, 0x00},                          /* 00 */
 {1, 1, 0x00},                          /* 00 */
 {1, 32, 0x07},                         /* Exit sequence */
 {1, 1, 0x5E},
 {1, 1, 0x59},
 {1, 1, 0x58},
 {-1, -1, 0} 
};

/*
 * The DBCS kludges
 */

/* Revision tag - we check the version first before searching for the
   DBCS code, as the bug pertains to a very specific range of kernels */

struct locator os2krnl_revtag[]=
{
 {0, 1, 'I'},
 {1, 1, 'n'},
 {1, 1, 't'},
 {1, 1, 'e'},
 {1, 1, 'r'},
 {1, 1, 'n'},
 {1, 1, 'a'},
 {1, 1, 'l'},
 {1, 1, ' '},
 {2, 1, 'e'},
 {1, 1, 'v'},
 {1, 1, 'i'},
 {1, 1, 's'},
 {1, 1, 'i'},
 {1, 1, 'o'},
 {1, 1, 'n'},
 {-1, -1, 0}
};

/* DBCS validation at ScanPathChar */

struct locator os2krnl_dbcs1[]=
{
 {0, 1, 0x3C},                          /* cmp al, '\' */
 {1, 1, 0x5C},
 {1, 1, 0x74},                          /* je ... */
 {1, 1, 0x0D},                          /* Not so far below */
 {1, 1, 0x0A},                          /* or al, al*/
 {1, 1, 0xC0},
 {1, 1, 0x74},                          /* Chg. to 75 F7 */
 {1, 1, 0x09},
 /* Replace all these for 90 in order to clear the way for returning */
 {1, 1, 0x3C},
 {1, 1, 0x80},
 {1, 1, 0x72},
 {1, 1, 0xF3},
 {1, 1, 0xAC},
 {1, 1, 0x0A},
 {1, 1, 0xC0},
 {1, 1, 0x75},
 {1, 1, 0xEE},
 {-1, -1, 0} 
};

/* DBCS validation at CopyComponent */

struct locator os2krnl_dbcs2[]=
{
 {0, 1, 0x3C},
 {1, 1, 0x80},
 {1, 1, 0x72},                          /* -> EB */
 {1, 1, 0x11},
 {1, 1, 0xAA},
 {1, 1, 0x49},
 {1, 1, 0x0B},
 {1, 1, 0xC9},
 {1, 1, 0x75},
 {1, 1, 0x03},
 {-1, -1, 0} 
};

/* DBCS validation at PathPref */

struct locator os2krnl_dbcs3[]=
{
 {0, 1, 0x3C},
 {1, 1, 0x80},
 {1, 1, 0x72},                          /* -> EB */
 {1, 1, 0xD0},
 {1, 1, 0xEB},
 {1, 1, 0x14},
 {1, 1, 0x36},
 {1, 1, 0xF6},
 {1, 1, 0x06},
 {3, 1, 0x01},
 {1, 1, 0x75},
 {-1, -1, 0} 
};

/* Parses the kernel revision tag */

#define isdigit(c) ((c)>='0'&&(c)<='9')
#define mkdig(v, c) ((v)*10+(c)-'0')

static unsigned int parse_tag(char *t)
{
 unsigned int rch=0, rcl=0;

 while(*t==' ')
  t++;
 while(isdigit(*t))
 {
  rch=mkdig(rch, *t);
  t++;
 }
 if(*t=='.')
  t++;
 while(isdigit(*t))
 {
  rcl=mkdig(rcl, *t);
  t++;
 }
 return(rch*1000+rcl);
}

/* Installs the NPX hook */

int _far fix_kernel_npx()
{
 struct ldrote_s *objtab;
 struct ldrmte_s *pmte;
 unsigned int i;
 struct area a;
 long o, ot, ol;
 int rc=1;
 int npx_hook_installed=0;

 pmte=locate_krnl_mte();
 if(pmte==NULL)
  return(-1);
 objtab=(struct ldrote_s *)pmte->mte_swapmte->smte_objtab;
 if(objtab==NULL)
  return(-1);
 i=pmte->mte_swapmte->smte_objcnt-1;
 a.first=(char *)objtab[i].ote_base;
 a.len=objtab[i].ote_size;
 /* NPX hook */
 if(!npx_hook_installed)
 {
  if((o=locate(os2krnl_savenpx, (struct area *)SSToDS(&a), 0))!=-1&&
     (ot=locate(os2krnl_npx_takeoff, (struct area *)SSToDS(&a), o))!=-1&&
     (ol=locate(os2krnl_npx_landing, (struct area *)SSToDS(&a), ot))!=-1&&
     (ol-ot)<137)
  {
   npx_tcb_base=*(unsigned long *)&a.first[ot+5];
   /* Compose the code to drive away from IBM's procedure */
   a.pos=ot;
   ac(0x9A);                          /* call far ... */
   ad((unsigned long)savenpx_override);
   aw(get_cs());
   ac(0x72);                          /* jc */
   ac((unsigned char)((ol-ot)-9));
   npx_hook_installed=1;
   a.pos=o;
   rc=0;
  }
 }
 /* Report if the NPX patch found its way into the kernel so we may use
    3DNow */
 return(rc);
}

/* Fixes any outstanding issues. Returns:
   < 0  on fatal errors
     0  on success
   > 0  (number of errors) if any patch failed */

int _far fix_kernel()
{
 struct ldrote_s *objtab;
 struct ldrmte_s *pmte;
 unsigned int i, j, k;
 struct area a;
 long o, ot, ol;
 int dbcs_hits=3;
 unsigned int krev;
 /* Segments to look within for the DBCS patch */
 static unsigned char segments[]={0x09, 0x0A, 0x0B, 0x08, 0x0C, 0x0D, 0x0E, 0x07, 0x0F, 0x10, 0xFF};
 int rc=0;

 pmte=locate_krnl_mte();
 if(pmte==NULL)
  return(-1);
 objtab=(struct ldrote_s *)pmte->mte_swapmte->smte_objtab;
 if(objtab==NULL)
  return(-1);
 i=pmte->mte_swapmte->smte_objcnt-1;
 a.first=(char *)objtab[i].ote_base;
 a.len=objtab[i].ote_size;
 /* Now try the DBCS hook. */
 if(!han_zi)
 {
  /* Check the kernel revision */
  a.first=(char *)objtab[0].ote_base;
  a.len=objtab[0].ote_size;
  if((o=locate(os2krnl_revtag, (struct area *)SSToDS(&a), 0))!=-1&&
     (krev=parse_tag(a.first+o+18))>=14089&&krev<=14095)
  {
   for(j=0; segments[j]!=0xFF&&dbcs_hits>0; j++)
   {
    /* Is this segment worth searching? */
    i=segments[j];
    if(i<pmte->mte_swapmte->smte_objcnt&&objtab[i].ote_size>16&&objtab[i].ote_base>=0xFF000000)
    {
     a.first=(char *)objtab[i].ote_base;
     a.len=objtab[i].ote_size;
     if((o=locate(os2krnl_dbcs1, (struct area *)SSToDS(&a), 0))!=-1)
     {
      a.pos=o+6;
      ac(0x75); ac(0xF7);
      for(k=0; k<9; k++)
       ac(0x90);
      dbcs_hits--;
     }
     if((o=locate(os2krnl_dbcs2, (struct area *)SSToDS(&a), 0))!=-1)
     {
      a.pos=o+2;
      ac(0xEB);
      dbcs_hits--;
     }
     if((o=locate(os2krnl_dbcs3, (struct area *)SSToDS(&a), 0))!=-1)
     {
      a.pos=o+2;
      ac(0xEB);
      dbcs_hits--;
     }
    }
   }
   if(dbcs_hits>0)
    rc++;
  }
 } /* !han_zi */
 return(rc);
}
