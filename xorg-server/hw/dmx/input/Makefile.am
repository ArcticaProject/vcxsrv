noinst_LIBRARIES = libdmxinput.a

if DMX_BUILD_LNX
RAWSRCS = lnx-keyboard.c \
          lnx-keyboard.h \
          lnx-ms.c \
          lnx-ms.h \
          lnx-ps2.c \
          lnx-ps2.h
endif

if DMX_BUILD_USB
USBSRCS = usb-keyboard.c \
          usb-keyboard.h \
          usb-mouse.c \
          usb-mouse.h \
          usb-other.c \
          usb-other.h \
          usb-common.c \
          usb-common.h \
          usb-private.h
endif

DRVSRCS = dmxdummy.c \
          dmxdummy.h \
          dmxbackend.c \
          dmxbackend.h \
          dmxconsole.c \
          dmxconsole.h \
          dmxcommon.c \
          dmxcommon.h

DMXSRCS = dmxinputinit.c \
          dmxinputinit.h \
          dmxarg.c \
          dmxarg.h \
          dmxsigio.c \
          dmxsigio.h \
          dmxevents.c \
          dmxevents.h \
	  dmxxinput.c \
          dmxmotion.c \
          dmxmotion.h \
          dmxeq.c \
          dmxeq.h \
          dmxmap.c \
          dmxmap.h


libdmxinput_a_SOURCES = ChkNotMaskEv.c \
                        ChkNotMaskEv.h \
                        $(RAWSRCS) \
                        $(USBSRCS) \
                        $(DRVSRCS) \
                        $(DMXSRCS) \
                        $(LIBSRCS)

if GLX
GLX_DEFS = @GL_CFLAGS@
GLX_INCS = -I@MESA_SOURCE@/include
endif	    

AM_CFLAGS = $(DIX_CFLAGS) \
            -I$(top_srcdir)/hw/dmx \
            -I$(top_srcdir)/hw/xfree86/common \
            $(GLX_INCS) \
            -DHAVE_DMX_CONFIG_H \
            $(GLX_DEFS) \
            @DMXMODULES_CFLAGS@

EXTRA_DIST = dmxdetach.c
