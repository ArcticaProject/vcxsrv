AUTOMAKE_OPTIONS=dist-bzip2 foreign nostdinc
DISTCHECK_CONFIGURE_FLAGS=--disable-xorgcfg

if COMPOSITE
COMPOSITE_DIR=composite
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
	$(RECORD_DIR) \
	xfixes \
	damageext \
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

DISTCLEANFILES = doltlibtool doltcompile

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || \
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
	record \
	xfixes \
	damageext \
	composite \
	glx \
	exa \
	config \
	hw

# gross hack
relink: all
	$(MAKE) -C hw relink
