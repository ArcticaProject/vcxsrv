if BUILD_SXPM

bin_PROGRAMS = sxpm

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = $(CWARNFLAGS) $(SXPM_CFLAGS)

sxpm_SOURCES = sxpm.c

LDADD = $(SXPM_LIBS) $(top_builddir)/src/libXpm.la

# Man page
appmandir = $(APP_MAN_DIR)

appman_PRE = sxpm.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

CLEANFILES = $(appman_DATA)

SUFFIXES = .$(APP_MAN_SUFFIX) .man

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

if USE_GETTEXT
noinst_DATA = sxpm.po

sxpm.po: $(sxpm_SOURCES:%=$(srcdir)/%)
	$(AM_V_GEN)xgettext -c"L10N_Comments" -d sxpm -n $(sxpm_SOURCES:%=$(srcdir)/%)

CLEANFILES += sxpm.po
endif
endif

EXTRA_DIST = \
	plaid_ext.xpm \
	plaid_mask.xpm \
	plaid.xpm \
	sxpm.man
