ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\keymap\sun_vndr
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"


dist_keymap_DATA = \
all de es fi \
fr no pl \
ru se uk \
us

DATA_FILES=$(dist_keymap_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
