include $(top_srcdir)/cpprules.in

if DRI
DRI_SUBDIR = dri
endif

if DRI2
DRI2_SUBDIR = dri2
endif

if XF86UTILS
XF86UTILS_SUBDIR = utils
endif

if XAA
XAA_SUBDIR = xaa
endif

if VGAHW
VGAHW_SUBDIR = vgahw
endif

if VBE
VBE_SUBDIR = vbe
endif

if INT10MODULE
INT10_SUBDIR = int10
endif

DOC_SUBDIR = doc

SUBDIRS = common ddc i2c x86emu $(INT10_SUBDIR) fbdevhw os-support parser \
	  ramdac shadowfb $(VBE_SUBDIR) $(VGAHW_SUBDIR) $(XAA_SUBDIR) \
	  xf8_16bpp loader dixmods exa modes \
	  $(DRI_SUBDIR) $(DRI2_SUBDIR) $(XF86UTILS_SUBDIR) $(DOC_SUBDIR)

DIST_SUBDIRS = common ddc i2c x86emu int10 fbdevhw os-support \
               parser ramdac shadowfb vbe vgahw xaa \
               xf8_16bpp loader dixmods dri dri2 exa modes \
	       utils doc

bin_PROGRAMS = Xorg
Xorg_SOURCES = xorg.c

AM_CFLAGS = $(DIX_CFLAGS) @XORG_CFLAGS@
INCLUDES = @XORG_INCS@ 

noinst_LTLIBRARIES = libxorg.la
libxorg_la_SOURCES = libxorg.c
libxorg_la_LIBADD = \
            $(XSERVER_LIBS) \
            loader/libloader.la \
            os-support/libxorgos.la \
            common/libcommon.la \
            parser/libxf86config_internal.la \
            dixmods/libdixmods.la \
            modes/libxf86modes.la \
            ramdac/libramdac.la \
            ddc/libddc.la \
            i2c/libi2c.la \
            dixmods/libxorgxkb.la \
            $(top_builddir)/mi/libmi.la \
            $(top_builddir)/os/libos.la \
            @XORG_LIBS@

libxorg_la_DEPENDENCIES = $(libxorg_la_LIBADD)

libxorg.c xorg.c:
	touch $@

DISTCLEANFILES = libxorg.c xorg.c

Xorg_DEPENDENCIES = libxorg.la
Xorg_LDADD = $(MAIN_LIB) libxorg.la $(XORG_SYS_LIBS) $(XSERVER_SYS_LIBS)

Xorg_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

BUILT_SOURCES = xorg.conf.example
DISTCLEANFILES += xorg.conf.example xorg.conf.example.pre
EXTRA_DIST = xorgconf.cpp

if SPECIAL_DTRACE_OBJECTS
# Re-add dtrace object code that gets lost when building static libraries
Xorg_LDADD += $(XSERVER_LIBS)
endif

if SOLARIS_ASM_INLINE
# Needs to be built before any files are compiled when using Sun compilers
# so in*/out* inline definitions are properly processed.

BUILT_SOURCES += os-support/solaris/solaris-@SOLARIS_INOUT_ARCH@.il

os-support/solaris/solaris-@SOLARIS_INOUT_ARCH@.il:
	cd os-support/solaris ; \
	 $(MAKE) $(AM_MAKEFLAGS) solaris-@SOLARIS_INOUT_ARCH@.il
endif

# do not use $(mkdir_p) if you want automake 1.7 to work
install-data-local:
	mkdir -p $(DESTDIR)$(logdir)


install-exec-local: install-binPROGRAMS
	(cd $(DESTDIR)$(bindir) && rm -f X && ln -s Xorg X)
if INSTALL_SETUID
	chown root $(DESTDIR)$(bindir)/Xorg
	chmod u+s $(DESTDIR)$(bindir)/Xorg
endif

CPP_FILES_FLAGS = \
	$(MANDEFS) \
	-DDEFAULTFONTPATH="\"$(COMPILEDDEFAULTFONTPATH)\"" \
	-DMODULEPATH=\"$(DEFAULT_MODULE_PATH)\"

relink:
	rm -f Xorg && $(MAKE) Xorg

xorg.conf.example.pre: xorgconf.cpp
	cp $(srcdir)/xorgconf.cpp $@
