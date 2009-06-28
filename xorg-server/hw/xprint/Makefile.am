SUBDIRS = doc pcl pcl-mono raster ps etc config

bin_PROGRAMS = Xprt

Xprt_CFLAGS = @DIX_CFLAGS@ @XPRINT_CFLAGS@ \
	-DPRINT_ONLY_SERVER -D_XP_PRINT_SERVER_  \
	-DXPRINTDIR=\"$(libdir)/X11/xserver\"    \
	-DXPRASTERDDX -DXPPCLDDX -DXPMONOPCLDDX -DXPPSDDX \
	-DXFree86Server

Xprt_LDFLAGS = -L$(top_srcdir)
XPRINT_LIBS = \
	@XPRINT_LIBS@ \
	ps/libps.la \
	raster/libraster.la \
	pcl/libpcl.la \
	pcl-mono/libpcl.la \
	$(top_builddir)/fb/libfb.la \
	$(top_builddir)/render/librender.la \
	$(top_builddir)/dix/libXpdix.la \
	$(XSERVER_LIBS) \
	$(top_builddir)/Xext/libXext.la \
	$(top_builddir)/xkb/libxkb.la \
	$(top_builddir)/xkb/libxkbstubs.la \
	$(top_builddir)/os/libos.la # for Xprintf

Xprt_DEPENDENCIES = $(XPRINT_LIBS)
Xprt_LDADD = $(XPRINT_LIBS) $(XPRINT_SYS_LIBS) $(XSERVER_SYS_LIBS)

miinitext-wrapper.c:
	echo "#include \"$(top_srcdir)/mi/miinitext.c\"" >> $@

dpmsstubs-wrapper.c:
	echo "#include \"$(top_srcdir)/Xext/dpmsstubs.c\"" >> $@

Xprt_SOURCES =			\
	attributes.c		\
	attributes.h		\
	AttrValid.c		\
	AttrValid.h		\
	ddxInit.c		\
	DiPrint.h		\
	Init.c			\
	mediaSizes.c		\
	Oid.c			\
	OidDefs.h		\
	Oid.h			\
	OidStrs.h		\
	spooler.c		\
	spooler.h		\
	Util.c			\
	miinitext-wrapper.c	\
	dpmsstubs-wrapper.c     \
        $(top_srcdir)/fb/fbcmap_mi.c

EXTRA_DIST = ValTree.c

CLEANFILES = miinitext-wrapper.c dpmsstubs-wrapper.c
