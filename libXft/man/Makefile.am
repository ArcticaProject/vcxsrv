libman_PRE = Xft.man
libmandir = $(LIB_MAN_DIR)
libman_DATA = $(libman_PRE:man=$(LIB_MAN_SUFFIX))

EXTRA_DIST = $(libman_PRE)
CLEANFILES = $(libman_DATA)
SUFFIXES = .$(LIB_MAN_SUFFIX) .man

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
.man.$(LIB_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@
