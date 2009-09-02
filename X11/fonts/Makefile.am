fontsdir = $(includedir)/X11/fonts
fonts_HEADERS = \
	font.h \
	fontproto.h \
	fontstruct.h \
	FS.h \
	fsmasks.h \
	FSproto.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fontsproto.pc

EXTRA_DIST = fontsproto.pc.in

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
