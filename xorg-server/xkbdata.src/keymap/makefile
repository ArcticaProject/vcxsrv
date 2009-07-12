ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=keymap

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

SUBDIRS = sun digital sgi

dist_xkbdata_keymap_DATA = \
	amiga \
	ataritt \
	macintosh \
	README \
	sony \
	xfree86 \
	xfree98

DATA_FILES=$(dist_xkbdata_keymap_DATA:%=$(DESTDIR)\%)

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

.PHONY: extrastuff

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
