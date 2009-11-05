if INSTALL_MAKESTRS
bin_PROGRAMS = makestrs
else
noinst_PROGRAMS = makestrs
endif

CC = @CC_FOR_BUILD@
CFLAGS = @CFLAGS_FOR_BUILD@
LDFLAGS = @LDFLAGS_FOR_BUILD@

EXTRA_DIST = \
	Shell.ht \
	StrDefs.ct \
	StrDefs.ht \
	string.list

makestrs_CFLAGS = $(XT_CFLAGS)
makestrs_SOURCES = makestrs.c


# Man page
appmandir = $(APP_MAN_DIR)

appman_PRE = makestrs.man

if INSTALL_MAKESTRS
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)
else
noinst_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)
endif

EXTRA_DIST += $(appman_PRE)
CLEANFILES = $(appman_DATA)

SED = sed

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = \
	-e 's|__xorgversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@
