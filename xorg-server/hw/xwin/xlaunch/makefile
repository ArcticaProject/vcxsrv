INCLUDELIBFILES = window\$(OBJDIR)\window.lib \
                  $(MHMAKECONF)\libX11\src\$(OBJDIR)\libx11.lib \
                  $(MHMAKECONF)\libxcb\src\$(OBJDIR)\libxcb.lib \
                  $(MHMAKECONF)\libXau\$(OBJDIR)\libXau.lib \
                  $(MHMAKECONF)\libX11\src\xlibi18n\$(OBJDIR)\libi18n.lib \
                  $(MHMAKECONF)\libX11\modules\lc\xlocale\$(OBJDIR)\libxlocale.lib \
                  $(MHMAKECONF)\libX11\modules\lc\utf8\$(OBJDIR)\libxlcUTF8Load.lib \
                  $(MHMAKECONF)\libX11\modules\lc\gen\$(OBJDIR)\liblcGenConvLoad.lib \
                  $(MHMAKECONF)\libX11\modules\lc\def\$(OBJDIR)\libxlcDef.lib \
                  $(MHMAKECONF)\libX11\src\xkb\$(OBJDIR)\libxkb.lib \
                  $(MHMAKECONF)\libX11\modules\im\ximcp\$(OBJDIR)\libximcp.lib \
                  $(MHMAKECONF)\libX11\modules\om\generic\$(OBJDIR)\libxomGeneric.lib

CSRCS=config.cc main.cc

INCLUDES += $(MHMAKECONF)\libxml2\include

WINAPP = xlaunch

RESOURCES = resources.rc

$(OBJDIR)\%.res : resources\%.rc $(wildcard resources\*.rc) 
	$(RC) $(RCFLAGS) $(RCDEFINES:%=-d "%") $(RCINCLUDES:%=-i %) -Fo$(relpath $@) $<

LIBDIRS=$(dir $(INCLUDELIBFILES))

load_makefile $(LIBDIRS:%$(OBJDIR)\=%makefile MAKESERVER=$(MAKESERVER) DEBUG=$(DEBUG);)

LINKLIBS += $(MHMAKECONF)\libxml2\lib\libxml2.lib

ifeq ($(DEBUG),1)
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2d.lib
else
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2.lib
endif

