if XLIB_LOADABLE_I18N
ORDER=src modules
else
ORDER=modules src
endif
SUBDIRS=include $(ORDER) nls man specs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = x11.pc
if XCB
pkgconfig_DATA += x11-xcb.pc
endif

MAINTAINERCLEANFILES = ChangeLog INSTALL
EXTRA_DIST=x11.pc.in x11-xcb.pc.in 

DISTCLEANFILES = doltcompile doltlibtool

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL

if LINT
# Check source code with tools like lint & sparse
LINT_SUBDIRS=src # modules

lint:
	@for subdir in $(LINT_SUBDIRS) ; do \
		echo "Making $@ in $$subdir"; \
		(cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) lint) ; \
	done
endif LINT
