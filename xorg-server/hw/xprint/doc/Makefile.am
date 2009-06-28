MAN_SRCS = Xprt.man.pre

appmandir = $(APP_MAN_DIR)

appman_PRE = Xprt.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

include $(top_srcdir)/cpprules.in

BUILT_SOURCES = $(appman_PRE)
CLEANFILES = $(appman_PRE) $(appman_DATA)

SUFFIXES += .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	-rm -f $@
	$(LN_S) $< $@

EXTRA_DIST = $(MAN_SRCS) Xprt.html Xprt.sgml
