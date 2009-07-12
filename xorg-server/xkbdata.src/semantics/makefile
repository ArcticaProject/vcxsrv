ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=semantics

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

dist_xkbdata_semantics_DATA = \
	basic \
	complete \
	default \
	xtest

DATA_FILES=$(dist_xkbdata_semantics_DATA:%=$(DESTDIR)\%)

include ..\xkbrules.mak
