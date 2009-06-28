appmandir = $(APP_MAN_DIR)
filemandir = $(FILE_MAN_DIR)

# Xserver.man covers options generic to all X servers built in this tree
# (i.e. those handled in the os/utils.c options processing instead of in
#  the DDX-level options processing)
appman_PRE = Xserver.man.pre
fileman_PRE =

appman_PROCESSED = $(appman_PRE:man.pre=man)
fileman_PROCESSED = $(fileman_PRE:man.pre=man)

appman_DATA = $(appman_PRE:man.pre=@APP_MAN_SUFFIX@)
fileman_DATA = $(fileman_PRE:man.pre=@FILE_MAN_SUFFIX@)

BUILT_SOURCES = $(appman_PROCESSED) $(fileman_PROCESSED)

CLEANFILES = $(appman_PROCESSED) $(appman_DATA) \
		$(fileman_PROCESSED) $(fileman_DATA)

include $(top_srcdir)/cpprules.in

.man.$(APP_MAN_SUFFIX):
	cp $< $@

.man.$(FILE_MAN_SUFFIX):
	cp $< $@

EXTRAMANDEFS = -D__default_font_path__="`echo $(COMPILEDDEFAULTFONTPATH) | sed -e 's/,/, /g'`"

# Docs about X server internals that we ship with source but don't install
DEVEL_DOCS = smartsched


EXTRA_DIST = $(DEVEL_DOCS) $(appman_PRE) $(fileman_PRE)
