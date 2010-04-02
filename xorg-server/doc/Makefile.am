
# Xserver.man covers options generic to all X servers built in this tree
# (i.e. those handled in the os/utils.c options processing instead of in
#  the DDX-level options processing)
appmandir = $(APP_MAN_DIR)
appman_PRE = Xserver.man.pre

appman_PROCESSED = $(appman_PRE:man.pre=man)
appman_DATA = $(appman_PRE:man.pre=@APP_MAN_SUFFIX@)

BUILT_SOURCES = $(appman_PROCESSED)
CLEANFILES = $(appman_PROCESSED) $(appman_DATA)

include $(top_srcdir)/cpprules.in

.man.$(APP_MAN_SUFFIX):
	$(AM_V_at)cp $< $@

EXTRAMANDEFS = -D__default_font_path__="`echo $(COMPILEDDEFAULTFONTPATH) | $(SED) -e 's/,/, /g'`"

# Docs about X server internals that we ship with source but don't install
DEVEL_DOCS = smartsched

EXTRA_DIST = $(DEVEL_DOCS) $(appman_PRE) $(fileman_PRE)
