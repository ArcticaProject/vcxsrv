# 
#  Copyright 2005  Red Hat, Inc.
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Red Hat not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Red Hat makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

bin_PROGRAMS = xkbcomp

AM_CFLAGS = $(XKBCOMP_CFLAGS) -DDFLT_XKB_CONFIG_ROOT='"$(datadir)/X11/xkb"'
xkbcomp_LDADD = $(XKBCOMP_LIBS)

xkbcomp_SOURCES = \
        action.c \
        action.h \
        alias.c \
        alias.h \
        compat.c \
        compat.h \
        expr.c \
        expr.h \
        geometry.c \
        indicators.c \
        indicators.h \
        keycodes.c \
        keycodes.h \
        keymap.c \
        keytypes.c \
        listing.c \
        misc.c \
        misc.h \
        parseutils.c \
        parseutils.h \
        symbols.c \
        tokens.h \
        utils.c \
        utils.h \
        vmod.c \
        vmod.h \
        xkbcomp.c \
        xkbcomp.h \
        xkbparse.y \
        xkbpath.c \
        xkbpath.h \
        xkbscan.c

appman_PRE = \
        xkbcomp.man

BUILT_SOURCES = xkbparse.c
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	README.config \
	README.enhancing

appmandir = $(APP_MAN_DIR)

appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRA_DIST += $(appman_PRE)
CLEANFILES = $(appman_DATA)

SED = sed

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = \
	-e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__xorgversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__xservername__|Xorg|g' \
	-e 's|__xconfigfile__|xorg.conf|g' \
	-e 's|__projectroot__|$(prefix)|g' \
	-e 's|__apploaddir__|$(appdefaultdir)|' \
	-e 's|__appmansuffix__|$(APP_MAN_SUFFIX)|g' \
	-e 's|__libmansuffix__|$(LIB_MAN_SUFFIX)|g' \
	-e 's|__adminmansuffix__|$(ADMIN_MAN_SUFFIX)|g' \
	-e 's|__miscmansuffix__|$(MISC_MAN_SUFFIX)|g' \
	-e 's|__filemansuffix__|$(FILE_MAN_SUFFIX)|g'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	sed $(MAN_SUBSTS) < $< > $@

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES += ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
