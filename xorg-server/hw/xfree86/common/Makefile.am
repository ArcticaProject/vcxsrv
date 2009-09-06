noinst_LTLIBRARIES = libcommon.la

if XORG_BUS_SPARC
SBUS_SOURCES = xf86sbusBus.c
endif

if XV
XVSOURCES = xf86xv.c xf86xvmc.c
XVSDKINCS = xf86xv.h xf86xvmc.h xf86xvpriv.h
endif

if XF86VIDMODE
XF86VMODE_SDK = vidmodeproc.h
endif

if DGA
DGASOURCES = xf86DGA.c
endif

XISOURCES = xf86Xinput.c xisb.c
XISDKINCS = xf86Xinput.h xisb.h
RANDRSOURCES = xf86RandR.c

BUSSOURCES = xf86pciBus.c xf86fbBus.c xf86noBus.c $(SBUS_SOURCES)

MODEDEFSOURCES = $(srcdir)/vesamodes $(srcdir)/extramodes

xf86DefModeSet.c: $(srcdir)/modeline2c.awk $(MODEDEFSOURCES)
	cat $(MODEDEFSOURCES) | LC_ALL=C $(AWK) -f $(srcdir)/modeline2c.awk > $@
	echo >> $@

BUILT_SOURCES = xf86DefModeSet.c

AM_LDFLAGS = -r
libcommon_la_SOURCES = xf86Configure.c xf86ShowOpts.c xf86Bus.c xf86Config.c \
                      xf86Cursor.c $(DGASOURCES) xf86DPMS.c \
                      xf86Events.c xf86Globals.c xf86AutoConfig.c \
                      xf86Option.c xf86Init.c xf86VGAarbiter.c \
                      xf86VidMode.c xf86fbman.c xf86cmap.c \
                      xf86Helper.c xf86PM.c xf86Xinput.c xisb.c \
                      xf86Mode.c xorgHelper.c \
                      $(XVSOURCES) $(BUSSOURCES) $(RANDRSOURCES)
nodist_libcommon_la_SOURCES = xf86DefModeSet.c xf86Build.h

INCLUDES = $(XORG_INCS) -I$(srcdir)/../ddc -I$(srcdir)/../i2c \
	   -I$(srcdir)/../loader -I$(srcdir)/../parser \
           -I$(srcdir)/../vbe -I$(srcdir)/../int10 \
	   -I$(srcdir)/../vgahw -I$(srcdir)/../dixmods/extmod \
	   -I$(srcdir)/../modes -I$(srcdir)/../ramdac

sdk_HEADERS = compiler.h fourcc.h xf86.h xf86Module.h xf86Opt.h \
              xf86PciInfo.h xf86Priv.h xf86Privstr.h \
              xf86cmap.h xf86fbman.h xf86str.h xf86Xinput.h xisb.h \
              $(XVSDKINCS) $(XF86VMODE_SDK) xorgVersion.h \
              xf86sbusBus.h xf86VGAarbiter.h

DISTCLEANFILES = xf86Build.h
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	compiler.h \
	fourcc.h \
	scoasm.h \
	vidmodeproc.h \
	xf86.h \
	xf86Bus.h \
	xf86Config.h \
	xf86InPriv.h \
	xf86Module.h \
	xf86Opt.h \
	xf86PciInfo.h \
	xf86Priv.h \
	xf86Privstr.h \
	xf86Xinput.h \
	xf86cmap.h \
	xf86fbman.h \
	xf86pciBus.h \
	xf86str.h \
	xf86xv.h \
	xf86xvmc.h \
	xf86xvpriv.h \
	xisb.h \
	xorgVersion.h \
	$(MODEDEFSOURCES) \
	modeline2c.awk \
	xf86VGAarbiter.h \
	xf86VGAarbiterPriv.h \
        $(DISTKBDSOURCES)

if LNXACPI
XORG_CFLAGS += -DHAVE_ACPI
endif

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)
