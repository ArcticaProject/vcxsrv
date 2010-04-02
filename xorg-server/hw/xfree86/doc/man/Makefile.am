# Xserver.man covers options generic to all X servers built in this tree
MAN_SRCS = Xorg.man.pre xorg.conf.man.pre

appmandir = $(APP_MAN_DIR)
appman_DATA = Xorg.$(APP_MAN_SUFFIX)

filemandir = $(FILE_MAN_DIR)
fileman_DATA = xorg.conf.$(FILE_MAN_SUFFIX)

Xorg.$(APP_MAN_SUFFIX): Xorg.man
	-$(AM_V_at)rm -f Xorg.$(APP_MAN_SUFFIX)
	$(AM_V_at)$(LN_S) Xorg.man Xorg.$(APP_MAN_SUFFIX)

xorg.conf.$(FILE_MAN_SUFFIX): xorg.conf.man
	-$(AM_V_at)rm -f xorg.conf.$(FILE_MAN_SUFFIX)
	$(AM_V_at)$(LN_S) xorg.conf.man xorg.conf.$(FILE_MAN_SUFFIX)

include $(top_srcdir)/cpprules.in

EXTRAMANDEFS = -D__logdir__=$(logdir)

CLEANFILES = $(appman_DATA) $(fileman_DATA) xorg.conf.man Xorg.man

EXTRA_DIST = $(MAN_SRCS)
