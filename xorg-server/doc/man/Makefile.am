# Xserver.man covers options generic to all X servers built in this tree
# (i.e. those handled in the os/utils.c options processing instead of in
#  the DDX-level options processing)

appmandir = $(APP_MAN_DIR)
appman_PRE = Xserver.man
appman_DATA = $(appman_PRE:man=$(APP_MAN_SUFFIX))

EXTRA_DIST = $(appman_PRE)
CLEANFILES = $(appman_DATA)
SUFFIXES = .$(APP_MAN_SUFFIX) .man

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
# 's|/,|/, |g' will add a space to help font path formatting
MAN_SUBSTS += 	-e 's|__datadir__|$(datadir)|g' \
		-e 's|__default_font_path__|$(COMPILEDDEFAULTFONTPATH)|g' \
		-e '\|$(COMPILEDDEFAULTFONTPATH)| s|/,|/, |g'

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@
