ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\keymap\sun_vndr

dist_keymap_DATA = \
all de es fi \
fr no pl \
ru se uk \
us

DATA_FILES=$(dist_keymap_DATA:%=$(DESTDIR)\%)

# We need to make the rule for all explicit since searching it in the implicit rules doesn't work for all
$(DESTDIR)\all: all.in
	copy $< $@


include ..\..\xkbrules.mak
