ifneq ($(NORELDBG),1)
$(error NORELDBG should have been set to 1)
endif

INCLUDES+=..\src

TMPL=fcarch.tmpl.h
STMPL=fcarch.tmpl.h
TARG=fcarch.h

ARCHITECTURE=x86-vc

TTYAPP=fc-arch
CSRCS=fc-arch.c

DEFINES += HAVE_CONFIG_H
INCLUDES := .. $(INCLUDES)

$(TARG): $(OBJDIR)\fc-arch.exe $(STMPL) ..\src\fcint.h ..\config.h
	del -e $(TARG)
	$(OBJDIR)\fc-arch.exe $(ARCHITECTURE) < $(STMPL) > $(TARG).tmp
	move $(TARG).tmp $(TARG)

