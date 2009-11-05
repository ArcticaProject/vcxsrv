SUBDIRS = man src

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xext.pc

EXTRA_DIST=xext.pc.in ChangeLog
MAINTAINERCLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog

if LINT
# Check source code with tools like lint & sparse
lint:
	(cd src && $(MAKE) $(MFLAGS) lint)
endif LINT
