noinst_LTLIBRARIES = liblinux.la

if LINUX_IA64
PLATFORM_PCI_SUPPORT = $(srcdir)/../shared/ia64Pci.c
PLATFORM_INCLUDES = -I$(srcdir)/../shared
endif
if LINUX_ALPHA
noinst_LTLIBRARIES += liblinuxev56.la
PLATFORM_PCI_SUPPORT = \
       $(srcdir)/lnx_axp.c \
       $(srcdir)/../shared/xf86Axp.c

liblinuxev56_la_CFLAGS = $(AM_CFLAGS) -mcpu=ev56

liblinuxev56_la_SOURCES = lnx_ev56.c
endif

if LNXACPI
ACPI_SRCS = lnx_acpi.c lnx_apm.c
XORG_CFLAGS += -DHAVE_ACPI
endif

if LNXAPM
APM_SRCS = lnx_apm.c
XORG_CFLAGS += -DHAVE_APM
endif

liblinux_la_SOURCES = lnx_init.c lnx_video.c \
                     lnx_pci.c lnx_agp.c lnx_kmod.c lnx_bell.c \
                     $(srcdir)/../shared/bios_mmap.c \
		     $(srcdir)/../shared/VTsw_usl.c \
		     $(srcdir)/../shared/posix_tty.c \
                     $(srcdir)/../shared/vidmem.c \
		     $(srcdir)/../shared/sigio.c \
                     $(srcdir)/../shared/stdResource.c \
                     $(ACPI_SRCS) \
                     $(APM_SRCS) \
                     $(PLATFORM_PCI_SUPPORT)

AM_CFLAGS = -DUSESTDRES -DHAVE_SYSV_IPC $(DIX_CFLAGS) $(XORG_CFLAGS) $(PLATFORM_DEFINES)

INCLUDES = $(XORG_INCS) $(PLATFORM_INCLUDES) $(LIBDRM_CFLAGS)

# FIXME: These need to be added to the build
LNX_EXTRA_SRCS = \
	lnx_font.c

EXTRA_DIST = \
	$(LNX_EXTRA_SRCS) \
	lnx.h \
	$(srcdir)/../shared/xf86Axp.h

if LINUX_ALPHA
liblinux_la_LIBADD = liblinuxev56.la
endif
