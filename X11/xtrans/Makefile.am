Xtransincludedir = $(includedir)/X11/Xtrans
Xtransinclude_HEADERS = \
	Xtrans.h \
	Xtrans.c \
	Xtransint.h \
	Xtranslcl.c \
	Xtranssock.c \
	Xtranstli.c \
	Xtransutil.c \
	transport.c

aclocaldir = $(datadir)/aclocal
aclocal_DATA = xtrans.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xtrans.pc

EXTRA_DIST = xtrans.pc.in ${aclocal_DATA}

CLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
