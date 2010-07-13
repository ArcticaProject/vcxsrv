ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\keymap\digital_vndr

dist_keymap_DATA = \
us

DATA_FILES=$(dist_keymap_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
