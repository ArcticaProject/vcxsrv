ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\symbols\nec_vndr
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"


dist_symbols_DATA = \
jp

DATA_FILES=$(dist_symbols_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
