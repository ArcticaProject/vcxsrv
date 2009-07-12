ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\rules
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

dist_xkbdata_rules_DATA = \
	README \
	sgi \
	sgi.lst \
	sun \
	sun.lst \
	xfree98 \
	xfree98.lst \
	xkb.dtd \
	xml2lst.pl \
	xorg \
	xorg-it.lst \
	xorg.lst \
	xorg.xml 

DATA_FILES=$(dist_xkbdata_rules_DATA:%=$(DESTDIR)\%)

include ..\xkbrules.mak
