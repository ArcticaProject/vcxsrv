INCLUDES = -I$(top_srcdir)/hw/xprint
 
AM_CFLAGS = @DIX_CFLAGS@ @XPRINT_CFLAGS@ -D_XP_PRINT_SERVER_ $(PCL_DRIVER)
 
libpcl_la_SOURCES =		\
	$(srcdir)/../pcl/PclArc.c	\
	$(srcdir)/../pcl/PclArea.c	\
	$(srcdir)/../pcl/PclAttr.c	\
	$(srcdir)/../pcl/PclAttVal.c	\
	$(srcdir)/../pcl/PclColor.c	\
	$(srcdir)/../pcl/PclCursor.c	\
	$(srcdir)/../pcl/PclDef.h	\
	$(srcdir)/../pcl/PclFonts.c	\
	$(srcdir)/../pcl/PclGC.c	\
	$(srcdir)/../pcl/Pcl.h		\
	$(srcdir)/../pcl/PclInit.c	\
	$(srcdir)/../pcl/PclLine.c	\
	$(srcdir)/../pcl/Pclmap.h	\
	$(srcdir)/../pcl/PclMisc.c	\
	$(srcdir)/../pcl/PclPixel.c	\
	$(srcdir)/../pcl/PclPolygon.c	\
	$(srcdir)/../pcl/PclPrint.c	\
	$(srcdir)/../pcl/PclSFonts.c	\
	$(srcdir)/../pcl/PclSFonts.h	\
	$(srcdir)/../pcl/PclSpans.c	\
	$(srcdir)/../pcl/PclText.c	\
	$(srcdir)/../pcl/PclWindow.c
