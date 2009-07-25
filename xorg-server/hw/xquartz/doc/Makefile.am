appmandir = $(APP_MAN_DIR)
appman_PRE = Xquartz.man.pre
appman_PROCESSED = $(appman_PRE:man.pre=man)
appman_DATA = $(appman_PRE:man.pre=@APP_MAN_SUFFIX@)

CLEANFILES = $(appman_PROCESSED) $(appman_DATA)

include $(top_srcdir)/cpprules.in

.man.$(APP_MAN_SUFFIX):
	cp $< $@

EXTRA_DIST = \
	Xquartz.man.pre
