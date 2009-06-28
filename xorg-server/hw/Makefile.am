if DMX
DMX_SUBDIRS = dmx
endif

if XORG
XORG_SUBDIRS = xfree86
endif

if XVFB
XVFB_SUBDIRS = vfb
endif

if XNEST
XNEST_SUBDIRS = xnest
endif

if XWIN
XWIN_SUBDIRS = xwin
endif

if XGL
XGL_SUBDIRS = xgl
endif

if KDRIVE
KDRIVE_SUBDIRS = kdrive
endif

if XPRINT
XPRINT_SUBDIRS = xprint
endif

if XQUARTZ
XQUARTZ_SUBDIRS = xquartz
endif

SUBDIRS =			\
	$(XORG_SUBDIRS)		\
	$(XGL_SUBDIRS)		\
	$(XWIN_SUBDIRS)		\
	$(XVFB_SUBDIRS)		\
	$(XNEST_SUBDIRS)	\
	$(DMX_SUBDIRS)		\
	$(KDRIVE_SUBDIRS)	\
	$(XQUARTZ_SUBDIRS)	\
	$(XPRINT_SUBDIRS)

DIST_SUBDIRS = dmx xfree86 vfb xnest xwin xquartz kdrive xgl xprint

relink:
	for i in $(SUBDIRS) ; do $(MAKE) -C $$i relink ; done
