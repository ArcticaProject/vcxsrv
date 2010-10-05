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


MAINTAINERCLEANFILES = ChangeLog INSTALL

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
