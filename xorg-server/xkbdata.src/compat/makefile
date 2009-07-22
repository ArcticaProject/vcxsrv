ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=compat

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

dist_xkbdata_compat_DATA = \
accessx basic complete \
default iso9995 \
japan keypad ledcaps \
lednum ledscroll level5 \
misc mousekeys norepeat \
olpc pc pc98 xfree86 \
xtest README

DATA_FILES=$(dist_xkbdata_compat_DATA:%=$(DESTDIR)\%)

include ..\xkbrules.mak
