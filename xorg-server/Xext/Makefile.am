# libXext.la:        includes all extensions and should be linked into Xvfb,
#                    Xnest, Xdmx and Xprt
# libXextbuiltin.la: includes those extensions that are built directly into
#                    Xorg by default
# libXextmodule.la:  includes those extensions that are built into a module
#                    that Xorg loads
if XORG
noinst_LTLIBRARIES = libXext.la libXextbuiltin.la libXextmodule.la
else
noinst_LTLIBRARIES = libXext.la
endif

INCLUDES = -I$(top_srcdir)/hw/xfree86/dixmods/extmod

AM_CFLAGS = $(DIX_CFLAGS)

if XORG
sdk_HEADERS = xvdix.h xvmcext.h
endif

# Sources always included in libXextbuiltin.la & libXext.la
BUILTIN_SRCS =			\
	shape.c			\
	sleepuntil.c		\
	sleepuntil.h		\
	xtest.c

# Sources always included in libXextmodule.la & libXext.la
MODULE_SRCS =			\
	bigreq.c		\
	mitmisc.c		\
	shape.c			\
	sync.c			\
	xcmisc.c

# Optional sources included if extension enabled by configure.ac rules

# MIT Shared Memory extension
MITSHM_SRCS = shm.c shmint.h
if MITSHM
BUILTIN_SRCS += $(MITSHM_SRCS)
endif

# XVideo extension
XV_SRCS = xvmain.c xvdisp.c xvmc.c xvdix.h xvmcext.h xvdisp.h
if XV
MODULE_SRCS  += $(XV_SRCS)
endif

# XResource extension: lets clients get data about per-client resource usage
RES_SRCS = xres.c
if RES
MODULE_SRCS  += $(RES_SRCS)
endif

# MIT ScreenSaver extension
SCREENSAVER_SRCS = saver.c
if SCREENSAVER
MODULE_SRCS  += $(SCREENSAVER_SRCS)
endif

# Xinerama extension: making multiple video devices act as one virtual screen
XINERAMA_SRCS = panoramiX.c panoramiX.h panoramiXh.h panoramiXsrv.h panoramiXprocs.c panoramiXSwap.c 
if XINERAMA
BUILTIN_SRCS += $(XINERAMA_SRCS)
endif

# X-ACE extension: provides hooks for building security policy extensions
# like XC-Security, X-SELinux & XTSol
XACE_SRCS = xace.c xace.h xacestr.h
if XACE
BUILTIN_SRCS += $(XACE_SRCS)
endif

# SELinux extension: provides SELinux policy support for X objects
# requires X-ACE extension
XSELINUX_SRCS = xselinux.c xselinux.h
if XSELINUX
MODULE_SRCS += $(XSELINUX_SRCS)
endif

# Security extension: multi-level security to protect clients from each other
XCSECURITY_SRCS = security.c securitysrv.h
if XCSECURITY   
BUILTIN_SRCS += $(XCSECURITY_SRCS)
endif

XCALIBRATE_SRCS = xcalibrate.c
if XCALIBRATE
BUILTIN_SRCS += $(XCALIBRATE_SRCS)
# XCalibrate needs tslib
endif

# X EVent Interception Extension: allows accessibility helpers & composite
# managers to intercept events from input devices and transform as needed
# before the clients see them.
XEVIE_SRCS = xevie.c
if XEVIE
BUILTIN_SRCS += $(XEVIE_SRCS)
endif

# XPrint: Printing via X Protocol
XPRINT_SRCS = xprint.c
if XPRINT
BUILTIN_SRCS += $(XPRINT_SRCS)
endif

# AppGroup
APPGROUP_SRCS = appgroup.c appgroup.h
if APPGROUP
BUILTIN_SRCS += $(APPGROUP_SRCS)
endif

# Colormap Utilization Protocol: Less flashing when switching between
# PsuedoColor apps and better sharing of limited colormap slots
CUP_SRCS = cup.c
if CUP
MODULE_SRCS  += $(CUP_SRCS)
endif

# Extended Visual Information
EVI_SRCS = EVI.c sampleEVI.c EVIstruct.h
if EVI
MODULE_SRCS  += $(EVI_SRCS)
endif

# Multi-buffering extension
MULTIBUFFER_SRCS = mbuf.c
EXTRA_MULTIBUFFER_SRCS = mbufbf.c mbufpx.c
if MULTIBUFFER
MODULE_SRCS  += $(MULTIBUFFER_SRCS)
endif

# Font Cache extension
FONTCACHE_SRCS = fontcache.c
if FONTCACHE
MODULE_SRCS  += $(FONTCACHE_SRCS)
endif

# XF86 Big Font extension
BIGFONT_SRCS = xf86bigfont.c
if XF86BIGFONT
BUILTIN_SRCS += $(BIGFONT_SRCS)
endif

# DPMS extension
DPMS_SRCS = dpms.c dpmsproc.h
if DPMSExtension
MODULE_SRCS += $(DPMS_SRCS)
endif

# Now take all of the above, mix well, bake for 10 minutes and get libXext*.la

libXext_la_SOURCES =		$(BUILTIN_SRCS) $(MODULE_SRCS)

if XORG
libXextbuiltin_la_SOURCES =	$(BUILTIN_SRCS)

libXextmodule_la_SOURCES =	$(MODULE_SRCS)
endif

EXTRA_DIST = \
	$(MITSHM_SRCS) \
	$(XV_SRCS) \
	$(RES_SRCS) \
	$(SCREENSAVER_SRCS) \
	$(XACE_SRCS) \
	$(XCSECURITY_SRCS) \
	$(XCALIBRATE_SRCS) \
	$(XINERAMA_SRCS) \
	$(XEVIE_SRCS) \
	$(XPRINT_SRCS) \
	$(APPGROUP_SRCS) \
	$(CUP_SRCS) \
	$(EVI_SRCS) \
	$(MULTIBUFFER_SRCS) \
	$(EXTRA_MULTIBUFFER_SRCS) \
	$(FONTCACHE_SRCS) \
	$(BIGFONT_SRCS) \
	$(DPMS_SRCS)

