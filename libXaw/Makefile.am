SUBDIRS = include src man spec

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = 

if BUILD_XAW6

pkgconfig_DATA += xaw6.pc
if PLATFORM_DARWIN
DEFAULT_LIB = libXaw.6.@LIBEXT@
else
DEFAULT_LIB = libXaw6.@LIBEXT@
endif

endif

if BUILD_XAW7

pkgconfig_DATA += xaw7.pc
if PLATFORM_DARWIN
DEFAULT_LIB = libXaw.7.@LIBEXT@
else
DEFAULT_LIB = libXaw7.@LIBEXT@
endif

endif

EXTRA_DIST = xaw6.pc.in \
	xaw7.pc.in \
	autogen.sh \
	old-doc/Changelog \
	old-doc/CHANGES \
	ChangeLog

if !PLATFORM_WIN32
install-exec-hook:
	cd $(DESTDIR)$(libdir) && rm -f libXaw.@LIBEXT@ && ln -s $(DEFAULT_LIB) libXaw.@LIBEXT@
endif

MAINTAINERCLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
