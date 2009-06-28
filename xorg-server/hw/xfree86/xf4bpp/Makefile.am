module_LTLIBRARIES = libxf4bpp.la

sdk_HEADERS = xf4bpp.h

libxf4bpp_la_LDFLAGS = -avoid-version

libxf4bpp_la_SOURCES = \
        ppcArea.c \
        ppcClip.c \
        ppcCpArea.c \
        ppcCReduce.c \
        ppcDepth.c \
        ppcFillRct.c \
        ppcWinFS.c \
        ppcPixFS.c \
        ppcGC.c \
        ppcGetSp.c \
        ppcImg.c \
        ppcPixmap.c \
        ppcPolyPnt.c \
        ppcQuery.c \
        ppcRslvC.c \
        ppcSetSp.c \
        ppcWindow.c \
        ppcIO.c \
        emulOpStip.c \
        emulRepAre.c \
        emulTile.c \
        vgaGC.c \
        vgaBitBlt.c \
        vgaImages.c \
        vgaStipple.c \
        vgaSolid.c \
        offscreen.c \
        wm3.c \
        mfbimggblt.c \
        mfbline.c \
        mfbseg.c \
        mfbhrzvert.c \
        mfbbres.c \
        mfbbresd.c \
        mfbfillarc.c \
        mfbzerarc.c \
        vgamodule.c

EXTRA_DIST = OScompiler.h ibmTrace.h ppcGCstr.h ppcSpMcro.h vgaReg.h \
             vgaVideo.h wm3.h NOTES

DISTCLEANFILES = mfbseg.c

mfbseg.c:
	echo "#define POLYSEGMENT" > $@
	echo "#include \"$(srcdir)/mfbline.c\"" >> $@

INCLUDES = $(XORG_INCS) -I$(srcdir)/../xf1bpp -I$(top_srcdir)/mfb

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)
