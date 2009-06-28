# FIXME: Add the *.S files to build when applicable
I386_SRCS = BUSmemcpy.S IODelay.S SlowBcopy.S
OTHER_SRCS = BUSmemcpy.c IODelay.c SlowBcopy.c

ARCH_SRCS = $(OTHER_SRCS)

# FIXME: Add to the build (NeedPortIO)
PORTIO_SRCS = PortIO.S

# FIXME: Add to the build (if HasGcc || HasGcc2)
ILHACK_SRCS = xf86_IlHack.c

noinst_LTLIBRARIES = libmisc.la

libmisc_la_SOURCES = xf86_Util.c Delay.c $(ARCH_SRCS)

#AM_LDFLAGS = -r

INCLUDES = $(XORG_INCS)

AM_CFLAGS = $(XORG_CFLAGS) $(DIX_CFLAGS)

EXTRA_DIST = $(I386_SRCS) $(PORTIO_SRCS) $(ILHACK_SRCS)
