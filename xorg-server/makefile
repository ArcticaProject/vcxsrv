ifneq ($(MAKESERVER),1)
$(error Please specify MAKESERVER=1 on the command line or as environment variable)
endif

INCLUDELIBFILES =  \
 composite\$(OBJDIR)\libcomposite.lib \
 config\$(OBJDIR)\libconfig.lib \
 damageext\$(OBJDIR)\libdamageext.lib \
 dbe\$(OBJDIR)\libdbe.lib \
 dix\$(OBJDIR)\libdix.lib \
 exa\$(OBJDIR)\libexa.lib \
 fb\$(OBJDIR)\libfb.lib \
 glx\$(OBJDIR)\libglx.lib \
 hw\kdrive\ephyr\$(OBJDIR)\libxephyr.lib \
 hw\kdrive\src\$(OBJDIR)\libkdrive.lib \
 hw\xwin\$(OBJDIR)\libxwin.lib \
 mi\$(OBJDIR)\libmi.lib \
 miext\damage\$(OBJDIR)\libdamage.lib \
 miext\shadow\$(OBJDIR)\libshadow.lib \
 os\$(OBJDIR)\libos.lib \
 randr\$(OBJDIR)\librandr.lib \
 record\$(OBJDIR)\librecord.lib \
 Xext\$(OBJDIR)\libxext.lib \
 xfixes\$(OBJDIR)\libxfixes.lib \
 Xi\$(OBJDIR)\libxi.lib \
 xkb\$(OBJDIR)\libxkb.lib \
 render\$(OBJDIR)\librender.lib \
 $(MHMAKECONF)\libxau\$(OBJDIR)\libxau.lib \
 $(MHMAKECONF)\libxdmcp\$(OBJDIR)\libxdmcp.lib \
 $(MHMAKECONF)\libXfont\src\util\$(OBJDIR)\libutil.lib \
 $(MHMAKECONF)\libXfont\src\fc\$(OBJDIR)\libfc.lib \
 $(MHMAKECONF)\libXfont\src\fontfile\$(OBJDIR)\libfontfile.lib \
 $(MHMAKECONF)\libXfont\src\builtins\$(OBJDIR)\libbuiltins.lib \
 $(MHMAKECONF)\libXfont\src\bitmap\$(OBJDIR)\libbitmap.lib \
 $(MHMAKECONF)\pixman\pixman\$(OBJDIR)\libpixman-1.lib \
 $(MHMAKECONF)\libx11\modules\im\ximcp\$(OBJDIR)\libximcp.lib \
 $(MHMAKECONF)\libx11\src\xlibi18n\$(OBJDIR)\libi18n.lib \
 $(MHMAKECONF)\libx11\src\$(OBJDIR)\libx11.lib \
 $(MHMAKECONF)\libx11\src\xcms\$(OBJDIR)\libxcms.lib \
 $(MHMAKECONF)\libxcb\src\$(OBJDIR)\libxcb.lib \
 $(MHMAKECONF)\libx11\src\xkb\$(OBJDIR)\libxkb.lib \
 $(MHMAKECONF)\libx11\modules\om\generic\$(OBJDIR)\libxomGeneric.lib \
 $(MHMAKECONF)\libx11\modules\lc\utf8\$(OBJDIR)\libxlcUTF8Load.lib \
 $(MHMAKECONF)\libx11\modules\lc\def\$(OBJDIR)\libxlcDef.lib \
 $(MHMAKECONF)\libx11\modules\lc\gen\$(OBJDIR)\liblcGenConvLoad.lib \
 $(MHMAKECONF)\zlib\$(OBJDIR)\libz.lib \
 $(MHMAKECONF)\libx11\modules\lc\xlocale\$(OBJDIR)\libxlocale.lib \
 $(MHMAKECONF)\libfontenc\src\$(OBJDIR)\libfontenc.lib \
 $(MHMAKECONF)\libxfont\src\freetype\$(OBJDIR)\libft.lib \
 $(MHMAKECONF)\libxfont\src\stubs\$(OBJDIR)\libstubs.lib

LIBDIRS=$(dir $(INCLUDELIBFILES))

load_makefile $(LIBDIRS:%$(OBJDIR)\=%makefile MAKESERVER=$(MAKESERVER) DEBUG=$(DEBUG);)

OBJS = dix\$(OBJDIR)\main.obj

WINAPP=vcxsrv

ifeq ($(DEBUG),1)
LINKLIBS += $(MHMAKECONF)\openssl\out32_d\libeay32.lib \
            $(MHMAKECONF)\freetype\lib\freetype200b8MT_D.lib \
            $(MHMAKECONF)\pthreads\pthreadVC2d.lib
else
LINKLIBS += $(MHMAKECONF)\openssl\out32\libeay32.lib \
            $(MHMAKECONF)\freetype\lib\freetype200b8MT.lib \
            $(MHMAKECONF)\pthreads\pthreadVC2.lib
endif

RCINCLUDES += hw\xwin $(MHMAKECONF)\include $(MHMAKECONF)

XWin.rc: hw\xwin\XWin.rc
	copy $< $@

RESOURCES = XWin.rc

$(OBJDIR)\$(WINAPP).exe: $(LINKLIBS)

XKeysymDB: ..\libX11\src\XKeysymDB
	copy $< $@

$(WINAPP).exe: $(OBJDIR)\$(WINAPP).exe
	copy $< $@

load_makefile hw\xwin\xlaunch\makefile MAKESERVER=0 DEBUG=$(DEBUG)

xlaunch.exe: hw\xwin\xlaunch\$(OBJDIR:obj\$(OBJDIRPREFIX)%=obj\%)\xlaunch.exe
	copy $< $@

load_makefile ..\xkbcomp\makefile MAKESERVER=0 DEBUG=$(DEBUG)

xkbcomp.exe: ..\xkbcomp\$(OBJDIR:obj\$(OBJDIRPREFIX)%=obj\%)\xkbcomp.exe
	copy $< $@

all: $(WINAPP).exe xlaunch.exe xkbcomp.exe protocol.txt XKeysymDB

protocol.txt: dix\protocol.txt
	copy $< $@


