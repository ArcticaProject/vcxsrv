ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\geometry\digital_vndr
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

dist_geom_DATA = \
lk  pc  unix

DATA_FILES=$(dist_geom_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
