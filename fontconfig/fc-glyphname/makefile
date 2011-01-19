ifneq ($(NORELDBG),1)
$(error NORELDBG should have been set to 1)
endif

INCLUDES+=..\src

TMPL=fcglyphname.tmpl.h
STMPL=${TMPL}
TARG=fcglyphname.h

TTYAPP=fc-glyphname

CSRCS= \
	fc-glyphname.c

GLYPHNAME=zapfdingbats.txt
SGLYPHNAME=zapfdingbats.txt

DEFINES += HAVE_CONFIG_H
INCLUDES := .. $(INCLUDES)

$(TARG): $(STMPL) $(OBJDIR)\fc-glyphname.exe $(SGLYPHNAME)
	del -e $(TARG)
	$(OBJDIR)\fc-glyphname.exe $(SGLYPHNAME) < $(STMPL) > $(TARG).tmp
	move $(TARG).tmp $(TARG)

