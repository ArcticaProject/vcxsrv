gldir = $(includedir)/GL
gl_HEADERS = \
	glxint.h \
	glxmd.h \
	glxproto.h \
	glxtokens.h

glinternaldir = $(includedir)/GL/internal
glinternal_HEADERS = \
	glcore.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = glproto.pc

EXTRA_DIST = glproto.pc.in

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
