# $Id: makefile,v 1.3 2006/04/01 09:51:35 andrew_belov Exp $
#
# Makefile for the RAMFS tool(s).
#
# Tools used:
#   Microsoft C v 6.00
#   MS LINK v 5.xx or IBM Linker/2 v 1.xx/2.0
#   NELITE by Veit Kannegieser (optional)
#
# User-configurable definitions:
#   NOMERGE=1    	will build the tools separately
#   PACK=1              will compress the output files w/NELITE
#   DEBUG=1         	will yield debug executables
#   DDKPATH=<d:\ddk\base\h>	points to the DDK headers
#

!ifndef NOMERGE
MERGE = 1
!endif

!ifdef DEBUG
OBJEXT = dbj
DBG_COPT = -DDEBUG
!else
OBJEXT = obj
DBG_COPT = -Os -Gs
!endif

!ifndef DDKPATH
DDKPATH = \DDK\BASE\H
!endif

# Compiler options

!ifdef MERGE
MRG_COPT = -DMERGE
!endif

!ifdef DEBUG
DBG_COPT = -DDEBUG -Od -Zd
DBG_LNKOPT = /CODEVIEW
!else
DBG_COPT = -Os -G2s
DBG_LNKOPT =
!endif

LRF = echo > NUL

COPT = -c $(MRG_COPT) $(DBG_COPT) -AS -G2 -Lp -I$(DDKPATH) -Fo$@
LINKOPT = /map:full $(DBG_LNKOPT) /noe /nod /stack:32768 /pmtype:vio

SUBTOOLS = ramdisk. deldisk. maxheap. heapstat.

#
# Rules
#

!ifdef MERGE

# Merged RAMDISK.EXE

!ifdef DEBUG
SUBTOOLSOBJS = $(SUBTOOLS:.=.obj )
!else
SUBTOOLSOBJS = $(SUBTOOLS:.=.dbj )
!endif

ramdisk.exe: main.$(OBJEXT) $(SUBTOOLSOBJS)
 $(LRF) @<<link.lrf
$(LINKOPT) $(**: = +^
)
$@
$(@R).map
slibcep+os2286
ramdisk.def;
<<
 link @link.lrf;
 if exist link.lrf del link.lrf
 mapsym $(@R).map
!ifdef PACK
 nelite $@ $@ /P:255
!endif

!else

all:
 for %1 in ($(SUBTOOLS)) do $(MAKE) -$(MAKEFLAGS) TARGET=%1 mktarg

mktarg: $(TARGET)$(OBJEXT)
 link $(LINKOPT) $(TARGET)$(OBJEXT),$(TARGET)exe,nul,slibcep+os2286;
!ifdef PACK
 nelite $(TARGET)exe $(TARGET)exe /P:255
!endif

!endif

.c.obj:
  cl $(COPT) $<

.c.dbj:
  cl -DDEBUG $(COPT) $<

.asm.obj:
!ifdef DEBUG
  alp -D:MAX_HEAP -D:DEBUG $<
!else
  alp -D:MAX_HEAP  $<
!endif

distclean:
 del *.?bj
 del *.map

clean: distclean
 del *.exe
 del *.sym
