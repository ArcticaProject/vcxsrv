ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\symbols\sun_vndr
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"


dist_symbols_DATA = \
 cs cz de dk es \
 fi fr gb gr hu \
 it jp ko lt lv \
 nl no pl pt ru \
 se solaris sw tr tuv \
 tw us usb

DATA_FILES=$(dist_symbols_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
