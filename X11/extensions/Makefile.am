xextdir = $(includedir)/X11/extensions
xext_HEADERS = \
	dpmsconst.h \
	dpmsproto.h \
        ge.h \
        geproto.h \
	lbx.h \
	lbxproto.h \
	mitmiscconst.h \
	mitmiscproto.h \
	multibufconst.h \
	multibufproto.h \
	secur.h \
	securproto.h \
	shapeconst.h \
	shapeproto.h \
	shm.h \
	shmproto.h \
	syncconst.h \
	syncproto.h \
	ag.h \
	agproto.h \
	cup.h \
	cupproto.h \
	dbe.h \
	dbeproto.h \
	EVI.h \
	EVIproto.h \
	xtestext1proto.h \
	xtestext1const.h \
	xtestconst.h \
	xtestproto.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xextproto.pc

EXTRA_DIST = xextproto.pc.in geproto.txt

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
