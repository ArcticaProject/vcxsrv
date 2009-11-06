TTYAPP = xwininfo

INCLUDELIBFILES = \
 $(MHMAKECONF)\libX11\src\xkb\$(OBJDIR)\libxkb.lib \
 $(MHMAKECONF)\libX11\src\xcms\$(OBJDIR)\libxcms.lib \
 $(MHMAKECONF)\libx11\src\xlibi18n\$(OBJDIR)\libi18n.lib \
 $(MHMAKECONF)\libx11\modules\lc\xlocale\$(OBJDIR)\libxlocale.lib \
 $(MHMAKECONF)\libx11\modules\lc\utf8\$(OBJDIR)\libxlcUTF8Load.lib \
 $(MHMAKECONF)\libx11\modules\lc\def\$(OBJDIR)\libxlcDef.lib \
 $(MHMAKECONF)\libx11\modules\om\generic\$(OBJDIR)\libxomGeneric.lib \
 $(MHMAKECONF)\libx11\modules\lc\gen\$(OBJDIR)\liblcGenConvLoad.lib \
 $(MHMAKECONF)\libx11\modules\im\ximcp\$(OBJDIR)\libximcp.lib \
 $(MHMAKECONF)\libxcb\src\$(OBJDIR)\libxcb.lib \
 $(MHMAKECONF)\libxau\$(OBJDIR)\libxau.lib \
 $(MHMAKECONF)\libXext\src\$(OBJDIR)\libXext.lib \
 $(MHMAKECONF)\libX11\src\$(OBJDIR)\libX11.lib

ifeq ($(DEBUG),1)
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2d.lib
else
LINKLIBS += $(MHMAKECONF)\pthreads\pthreadVC2.lib
endif

CSRCS =	\
        clientwin.c \
        dsimple.c \
        xwininfo.c

