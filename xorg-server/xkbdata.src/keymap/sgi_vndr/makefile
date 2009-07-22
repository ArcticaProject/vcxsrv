ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\keymap\sgi_vndr
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"


dist_keymap_DATA = \
be bg ca \
ch cz de \
dk dvorak \
es fi \
fr gb \
hu it jp \
no pl pt \
ru se sk \
th us

DATA_FILES=$(dist_keymap_DATA:%=$(DESTDIR)\%)

include ..\..\xkbrules.mak
