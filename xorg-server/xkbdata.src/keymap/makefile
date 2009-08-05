ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=keymap

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)

SUBDIRS = digital_vndr sgi_vndr sun_vndr

dist_xkbdata_keymap_DATA = \
amiga ataritt macintosh \
sony xfree86 xfree98 \
README

DATA_FILES=$(dist_xkbdata_keymap_DATA:%=$(DESTDIR)\%)

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

.PHONY: extrastuff

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
