noinst_LTLIBRARIES = libps.la

INCLUDES = -I$(top_srcdir)/hw/xprint

AM_CFLAGS = @DIX_CFLAGS@ @XPRINT_CFLAGS@ @FREETYPE_CFLAGS@ \
	    -D_XP_PRINT_SERVER_ -DXP_PSTEXT

libps_la_SOURCES =		\
	PsArc.c			\
	PsArea.c		\
	PsAttr.c		\
	PsAttVal.c		\
	PsCache.c		\
	PsColor.c		\
	PsDef.h			\
	PsFonts.c		\
	PsGC.c			\
	Ps.h			\
	PsImageUtil.c		\
	PsInit.c		\
	PsLine.c		\
	PsMisc.c		\
	psout.c			\
	psout.h			\
	PsPixel.c		\
	PsPixmap.c		\
	PsPolygon.c		\
	PsPrint.c		\
	PsSpans.c		\
	PsText.c		\
	PsWindow.c

EXTRA_DIST = 	PsFTFonts.c	\
	psout_ft.c		\
	psout_ftpstype1.c	\
	psout_ftpstype3.c	\
	ttf2pt1wrap.c

if XP_USE_FREETYPE
    AM_CFLAGS += -DXP_USE_FREETYPE
    libps_la_SOURCES += $(EXTRA_DIST)
endif
