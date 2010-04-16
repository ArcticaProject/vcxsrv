appmandir = $(APP_MAN_DIR)
appman_PRE = Xquartz.man.pre
appman_PROCESSED = $(appman_PRE:man.pre=man)
appman_DATA = $(appman_PRE:man.pre=@APP_MAN_SUFFIX@)

CLEANFILES = $(appman_PROCESSED) $(appman_DATA)

include $(top_srcdir)/cpprules.in

MANDEFS += -D__laucnd_id_prefix__=$(LAUNCHD_ID_PREFIX)

.man.$(APP_MAN_SUFFIX):
	$(AM_V_at)cp $< $@

EXTRA_DIST = \
	Xquartz.man.pre
