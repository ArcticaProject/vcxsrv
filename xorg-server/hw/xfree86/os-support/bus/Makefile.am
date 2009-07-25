noinst_LTLIBRARIES = libbus.la
sdk_HEADERS = xf86Pci.h

PCI_SOURCES =

if XORG_BUS_LINUXPCI
PCI_SOURCES += linuxPci.c
endif

if XORG_BUS_BSDPCI
PCI_SOURCES += bsd_pci.c
endif

if XORG_BUS_SPARC
PLATFORM_SOURCES = Sbus.c
sdk_HEADERS += xf86Sbus.h
endif

libbus_la_SOURCES = Pci.c Pci.h $(PCI_SOURCES) $(PLATFORM_PCI_SOURCES) \
                    $(PLATFORM_SOURCES)

INCLUDES = $(XORG_INCS)

AM_CFLAGS = $(XORG_CFLAGS) $(DIX_CFLAGS)

EXTRA_DIST = $(sdk_HEADERS)
