xextdir = $(includedir)/X11/extensions
xext_HEADERS = \
	dpms.h \
	dpmsstr.h \
	extutil.h \
        ge.h \
        geproto.h \
        Xge.h \
	lbxbuf.h \
	lbxbufstr.h \
	lbxdeltastr.h \
	lbximage.h \
	lbxopts.h \
	lbxstr.h \
	lbxzlib.h \
	MITMisc.h \
	mitmiscstr.h \
	multibuf.h \
	multibufst.h \
	security.h \
	securstr.h \
	shape.h \
	shapestr.h \
	shmstr.h \
	sync.h \
	syncstr.h \
	Xag.h \
	Xagstr.h \
	Xcup.h \
	Xcupstr.h \
	Xdbe.h \
	Xdbeproto.h \
	XEVI.h \
	XEVIstr.h \
	Xext.h \
	XLbx.h \
	XShm.h \
	xtestext1.h \
	XTest.h \
	xteststr.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xextproto.pc

EXTRA_DIST = autogen.sh xextproto.pc.in

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
