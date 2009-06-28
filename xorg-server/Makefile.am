AUTOMAKE_OPTIONS=dist-bzip2 foreign nostdinc
DISTCHECK_CONFIGURE_FLAGS=--disable-xorgcfg

if COMPOSITE
COMPOSITE_DIR=composite
endif

if XTRAP
XTRAP_DIR=XTrap
endif

if CFB
CFB_DIR=cfb
CFB32_DIR=cfb32
endif

if AFB
AFB_DIR=afb
endif

if MFB
MFB_DIR=mfb
endif

if GLX
GLX_DIR=glx
endif

if DBE
DBE_DIR=dbe
endif

if RECORD
RECORD_DIR=record
endif

SUBDIRS = \
	doc \
	include \
	dix  \
	fb \
	mi \
	Xext \
	miext \
	os \
	randr \
	render  \
	Xi \
	xkb \
	$(DBE_DIR) \
	$(MFB_DIR) \
	$(AFB_DIR) \
	$(CFB_DIR) \
	$(CFB32_DIR) \
	$(RECORD_DIR) \
	xfixes \
	damageext \
	$(XTRAP_DIR) \
	$(COMPOSITE_DIR) \
	$(GLX_DIR) \
	exa \
	config \
	hw

aclocaldir = $(datadir)/aclocal
aclocal_DATA = xorg-server.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xorg-server.pc

EXTRA_DIST = xorg-server.pc.in xorg-server.m4 ChangeLog autogen.sh

MAINTAINERCLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || \
	(touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

DIST_SUBDIRS = \
	doc \
	include \
	dix  \
	fb \
	mi \
	Xext \
	miext \
	os \
	randr \
	render  \
	Xi \
	xkb \
	dbe \
	mfb \
	afb \
	cfb \
	cfb32 \
	record \
	xfixes \
	damageext \
	XTrap \
	composite \
	glx \
	exa \
	config \
	hw

# gross hack
relink: all
	$(MAKE) -C hw relink
