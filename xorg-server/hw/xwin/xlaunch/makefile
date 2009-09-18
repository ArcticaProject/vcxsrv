INCLUDELIBFILES = window\$(OBJDIR)\window.lib \
                  $(MHMAKECONF)\libx11\src\$(OBJDIR)\libx11.lib \
                  $(MHMAKECONF)\libxcb\src\$(OBJDIR)\libxcb.lib \
                  $(MHMAKECONF)\libxau\$(OBJDIR)\libxau.lib \
                  $(MHMAKECONF)\libx11\src\xlibi18n\$(OBJDIR)\libi18n.lib \
                  $(MHMAKECONF)\libx11\modules\lc\xlocale\$(OBJDIR)\libxlocale.lib \
                  $(MHMAKECONF)\libx11\modules\lc\utf8\$(OBJDIR)\libxlcUTF8Load.lib \
                  $(MHMAKECONF)\libx11\modules\lc\gen\$(OBJDIR)\liblcGenConvLoad.lib \
                  $(MHMAKECONF)\libx11\modules\lc\def\$(OBJDIR)\libxlcDef.lib \
                  $(MHMAKECONF)\libx11\src\xkb\$(OBJDIR)\libxkb.lib \
                  $(MHMAKECONF)\libx11\modules\im\ximcp\$(OBJDIR)\libximcp.lib \
                  $(MHMAKECONF)\libx11\modules\om\generic\$(OBJDIR)\libxomGeneric.lib

CSRCS=config.cc main.cc

WINAPP = xlaunch

RESOURCES = resources.rc

$(OBJDIR)\%.res : resources\%.rc
	$(RC) $(RCFLAGS) $(RCDEFINES:%=-d "%") $(RCINCLUDES:%=-i %) -Fo$(relpath $@) $<

LIBDIRS=$(dir $(INCLUDELIBFILES))

load_makefile $(LIBDIRS:%$(OBJDIR)\=%makefile MAKESERVER=$(MAKESERVER) DEBUG=$(DEBUG);)

ifeq ($(DEBUG),1)
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2d.lib
else
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2.lib
endif

