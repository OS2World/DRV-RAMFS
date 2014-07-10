#
# Main makefile for RAMFS
#
# Tools used:
#   Microsoft C v 6.00
#   IBM Assembly Language Processor
#   IBM Link386
#

LRF = echo > NUL

!ifdef DEBUG
OBJEXT = .dbj
EXENAME = ramfsd
!else
OBJEXT = .obj
EXENAME = ramfs
!endif

!ifndef DDKPATH
DDKPATH = \DDK\BASE\H
!endif

COPT = -c -Zi -Zl -DMAX_HEAP -D__COMPACT__ -D_ss=far -D_ds=far -Alfu -NT_TEXT -ND_DATA -G2s -Ot -Zp -J -I$(DDKPATH) -Fo$@

OBJS1 = allocate$(OBJEXT) attach$(OBJEXT) cancello$(OBJEXT) chdir$(OBJEXT) chgfilep$(OBJEXT) \
	close$(OBJEXT) commit$(OBJEXT) copy$(OBJEXT) delete$(OBJEXT) dopageio$(OBJEXT) \
	exit$(OBJEXT) fileattr$(OBJEXT) fileinfo$(OBJEXT) fileio$(OBJEXT) filelock$(OBJEXT) \
	findclos$(OBJEXT)
OBJS2 =	findfirs$(OBJEXT) findfrom$(OBJEXT) findnext$(OBJEXT) flushbuf$(OBJEXT) fsctl$(OBJEXT) \
	fsinfo$(OBJEXT) init$(OBJEXT) ioctl$(OBJEXT) mkdir$(OBJEXT) mount$(OBJEXT) \
	move$(OBJEXT) newsize$(OBJEXT) nmpipe$(OBJEXT) notifycl$(OBJEXT) notifyfi$(OBJEXT) \
	notifyne$(OBJEXT)
OBJS3 =	opencrea$(OBJEXT) openpage$(OBJEXT) pathinfo$(OBJEXT) processn$(OBJEXT) read$(OBJEXT) \
	rmdir$(OBJEXT) setswap$(OBJEXT) shutdown$(OBJEXT) verifyun$(OBJEXT) write$(OBJEXT) \
	block$(OBJEXT) ea$(OBJEXT) find$(OBJEXT) info$(OBJEXT) nearheap$(OBJEXT) \
	util$(OBJEXT) vmheap.obj

all: $(EXENAME).ifs
 cd tools
 $(MAKE) -$(MAKEFLAGS)

$(EXENAME).ifs: $(OBJS1) $(OBJS2) $(OBJS3) patch\patch.lib
 $(LRF) @<<link.lrf
/map:full /linenumbers /noe /nod $(**: = +^
)
$@
$(@R).map
fshelper.lib+os2286.lib+dhcalls.lib+vdh.lib+llibcep
ramfs.def
<<
 link386 @link.lrf;
 if exist link.lrf del link.lrf
 mapsym $(EXENAME).map

patch\patch.lib: mkpatch

mkpatch:
 cd patch
 $(MAKE) -$(MAKEFLAGS)
 cd ..

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

dist:
 cvs diff -u -r HEAD -r ramfs-1_01|gzip -acf9 >rollback.gz

distclean:
 del *$(OBJEXT)
 del patch\*$(OBJEXT)
 del patch\*.lib
 del *.map
