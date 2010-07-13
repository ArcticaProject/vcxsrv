ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\geometry\sgi_vndr


dist_geom_DATA = \
indigo  indy  O2

DATA_FILES=$(dist_geom_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
