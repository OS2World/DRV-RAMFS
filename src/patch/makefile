# $Id: makefile,v 1.2 2003/11/07 07:46:20 root Exp $
#
# Kernel patch for using MMX instructions in FSD

all: patch.lib

patch.lib: kernfix.obj patchram.obj 32util.obj
 -@del $@
 lib /NOLOGO $@ $**,nul

# CHGNAM courtesy of osFree TPE team, donated anonymously
.c.obj:
 cl386 /c /Zlfeip /Asnw /I$(H) /G3 /Gs /NT_TEXT32 $<
 chgnam data32.fix $@

.asm.obj:
 alp $<
