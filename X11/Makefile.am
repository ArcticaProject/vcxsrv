xprotodir = $(includedir)/X11
xproto_HEADERS = \
	ap_keysym.h \
	DECkeysym.h \
	HPkeysym.h \
	keysymdef.h \
	keysym.h \
	Sunkeysym.h \
	Xalloca.h \
	Xarch.h \
	Xatom.h \
	Xdefs.h \
	XF86keysym.h \
	Xfuncs.h \
	X.h \
	Xmd.h \
	Xosdefs.h \
	Xos.h \
	Xos_r.h \
	Xproto.h \
	Xprotostr.h \
	Xthreads.h \
	Xw32defs.h \
	XWDFile.h \
	Xwindows.h \
	Xwinsock.h

nodist_xproto_HEADERS = \
	Xfuncproto.h \
	Xpoll.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xproto.pc

EXTRA_DIST = autogen.sh xproto.pc.in ChangeLog

CLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

