ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=keycodes

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

SUBDIRS = digital sgi

dist_xkbdata_keycodes_DATA = \
	aliases \
	amiga \
	ataritt \
	fujitsu \
	hp \
	ibm \
	macintosh \
	powerpcps2 \
	README \
	sony \
	sun \
	xfree86 \
	xfree98

DATA_FILES=$(dist_xkbdata_keycodes_DATA:%=$(DESTDIR)\%)

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

.PHONY: extrastuff

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
