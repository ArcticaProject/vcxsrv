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
	$(AM_V_GEN)sed $(MAN_SUBSTS) < $< > $@

if USE_GETTEXT
noinst_DATA = cxpm.po

cxpm.po: $(cxpm_SOURCES:%=$(srcdir)/%)
	$(AM_V_GEN)xgettext -c"L10N_Comments" -d cxpm -n $(cxpm_SOURCES:%=$(srcdir)/%)

CLEANFILES += cxpm.po
endif
