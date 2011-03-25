bin_PROGRAMS = cxpm

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include/X11
AM_CFLAGS = $(CWARNFLAGS) $(XPM_CFLAGS)

cxpm_SOURCES = cxpm.c

# Man page
appmandir = $(APP_MAN_DIR)

appman_PRE = cxpm.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRA_DIST = $(appman_PRE)
CLEANFILES = $(appman_DATA)

SUFFIXES = .$(APP_MAN_SUFFIX) .man

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

if USE_GETTEXT
noinst_DATA = cxpm.po

cxpm.po: $(cxpm_SOURCES:%=$(srcdir)/%)
	$(AM_V_GEN)xgettext -c"L10N_Comments" -d cxpm -n $(cxpm_SOURCES:%=$(srcdir)/%)

CLEANFILES += cxpm.po
endif
