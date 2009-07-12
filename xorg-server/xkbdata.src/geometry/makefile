ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=geometry

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

dist_xkbdata_geometry_DATA = \
	amiga \
	ataritt \
	chicony \
	dell \
	everex \
	fujitsu \
	hp \
	keytronic \
	kinesis \
	macintosh \
	microsoft \
	nec \
	northgate \
	pc \
	README \
	sony \
	sun \
	winbook

DATA_FILES=$(dist_xkbdata_geometry_DATA:%=$(DESTDIR)\%)

SUBDIRS = digital ibm sgi

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

.PHONY: extrastuff

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
