noinst_LIBRARIES = libinit.a
noinst_LTLIBRARIES = libcommon.la

if XORG_BUS_SPARC
SBUS_SOURCES = xf86sbusBus.c
endif

if XV
XVSOURCES = xf86xv.c xf86xvmc.c
XVSDKINCS = xf86xv.h xf86xvmc.h
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
                      xf86Cursor.c xf86DGA.c xf86DPMS.c \
                      xf86Events.c xf86Globals.c xf86AutoConfig.c \
                      xf86Option.c \
                      xf86VidMode.c xf86fbman.c xf86cmap.c \
                      xf86Helper.c xf86PM.c xf86RAC.c xf86Xinput.c xisb.c \
                      xf86Mode.c xorgHelper.c \
                      $(XVSOURCES) $(BUSSOURCES) $(RANDRSOURCES)
nodist_libcommon_la_SOURCES = xf86DefModeSet.c
libinit_a_SOURCES = xf86Init.c
nodist_libinit_a_SOURCES = xf86Build.h

INCLUDES = $(XORG_INCS) -I$(srcdir)/../ddc -I$(srcdir)/../i2c \
	   -I$(srcdir)/../loader -I$(srcdir)/../rac -I$(srcdir)/../parser \
           -I$(srcdir)/../vbe -I$(srcdir)/../int10 \
	   -I$(srcdir)/../vgahw -I$(srcdir)/../dixmods/extmod \
	   -I$(srcdir)/../modes

sdk_HEADERS = compiler.h fourcc.h xf86.h xf86Module.h xf86Opt.h \
              xf86PciInfo.h xf86Priv.h xf86Privstr.h xf86Resources.h \
              xf86cmap.h xf86fbman.h xf86str.h xf86RAC.h xf86Xinput.h xisb.h \
              $(XVSDKINCS) xorgVersion.h \
              xf86sbusBus.h xf86xvpriv.h

DISTCLEANFILES = xf86Build.h
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	compiler.h \
	fourcc.h \
	scoasm.h \
	xf86.h \
	xf86Bus.h \
	xf86Config.h \
	xf86InPriv.h \
	xf86Module.h \
	xf86Opt.h \
	xf86PciInfo.h \
	xf86Priv.h \
	xf86Privstr.h \
	xf86Resources.h \
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
	xf86Date.h \
	$(MODEDEFSOURCES) \
	modeline2c.awk \
        $(DISTKBDSOURCES)

if LNXACPI
XORG_CFLAGS += -DHAVE_ACPI
endif

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)
