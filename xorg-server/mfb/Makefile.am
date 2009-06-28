noinst_LTLIBRARIES = libmfb.la

AM_CFLAGS = $(DIX_CFLAGS)

INCLUDES = -I$(top_srcdir)/hw/xfree86/os-support

libmfb_gen_sources = mfbseg.c mfbpgbwht.c mfbpgbblak.c mfbpgbinv.c mfbigbwht.c \
	 mfbigbblak.c mfbpawhite.c mfbpablack.c mfbpainv.c mfbtewhite.c \
	 mfbteblack.c mfbbltC.c mfbbltX.c mfbbltCI.c mfbbltO.c mfbbltG.c \
	 mfbtileC.c mfbtileG.c mfbplywhite.c mfbplyblack.c mfbplyinv.c

DISTCLEANFILES = $(libmfb_gen_sources)

libmfb_la_SOURCES = mfbgc.c mfbwindow.c mfbfont.c \
	 mfbfillrct.c maskbits.c mfbpixmap.c \
	 mfbimage.c mfbline.c mfbbres.c mfbhrzvert.c mfbbresd.c \
	 mfbpushpxl.c mfbzerarc.c mfbfillarc.c \
         mfbfillsp.c mfbsetsp.c mfbscrinit.c mfbscrclse.c mfbclip.c \
         mfbbitblt.c mfbgetsp.c mfbpolypnt.c \
	 mfbcmap.c mfbmisc.c $(libmfb_gen_sources)

EXTRA_DIST = maskbits.h mergerop.h fastblt.h mfbline.c mfbblt.c mfbtile.c \
             mfbplygblt.c mfbimggblt.c mfbpntarea.c mfbtegblt.c mfbply1rct.c

sdk_HEADERS = mfb.h

mfbseg.c:
	echo "#define POLYSEGMENT" > $@
	echo "#include \"$(srcdir)/mfbline.c\"" >> $@

mfbbltC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(srcdir)/mfbblt.c\"" >> $@

mfbbltX.c:
	echo "#define MROP Mxor" > $@
	echo "#include \"$(srcdir)/mfbblt.c\"" >> $@

mfbbltCI.c:
	echo "#define MROP McopyInverted" > $@
	echo "#include \"$(srcdir)/mfbblt.c\"" >> $@

mfbbltO.c:
	echo "#define MROP Mor" > $@
	echo "#include \"$(srcdir)/mfbblt.c\"" >> $@

mfbbltG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(srcdir)/mfbblt.c\"" >> $@

mfbtileC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(srcdir)/mfbtile.c\"" >> $@

mfbtileG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(srcdir)/mfbtile.c\"" >> $@

mfbpgbwht.c:
	echo "#define OPEQ MFB_OPEQ_WHITE" > $@
	echo "#define MFBPOLYGLYPHBLT mfbPolyGlyphBltWhite" >> $@
	echo "#include \"$(srcdir)/mfbplygblt.c\"" >> $@

mfbpgbblak.c:
	echo "#define OPEQ MFB_OPEQ_BLACK" > $@
	echo "#define MFBPOLYGLYPHBLT mfbPolyGlyphBltBlack" >> $@
	echo "#include \"$(srcdir)/mfbplygblt.c\"" >> $@

mfbpgbinv.c:
	echo "#define OPEQ MFB_OPEQ_INVERT" > $@
	echo "#define MFBPOLYGLYPHBLT mfbPolyGlyphBltInvert" >> $@
	echo "#include \"$(srcdir)/mfbplygblt.c\"" >> $@

mfbigbwht.c:
	echo "#define OPEQ MFB_OPEQ_WHITE" > $@
	echo "#define MFBIMAGEGLYPHBLT mfbImageGlyphBltWhite" >> $@
	echo "#include \"$(srcdir)/mfbimggblt.c\"" >> $@

mfbigbblak.c:
	echo "#define OPEQ MFB_OPEQ_BLACK" > $@
	echo "#define MFBIMAGEGLYPHBLT mfbImageGlyphBltBlack" >> $@
	echo "#include \"$(srcdir)/mfbimggblt.c\"" >> $@

mfbpawhite.c:
	echo "#define OPEQ MFB_OPEQ_WHITE" > $@
	echo "#define MFBSOLIDFILLAREA mfbSolidWhiteArea" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_WHITE" >> $@
	echo "#define MFBSTIPPLEFILLAREA mfbStippleWhiteArea" >> $@
	echo "#include \"$(srcdir)/mfbpntarea.c\"" >> $@

mfbpablack.c:
	echo "#define OPEQ MFB_OPEQ_BLACK" > $@
	echo "#define MFBSOLIDFILLAREA mfbSolidBlackArea" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_BLACK" >> $@
	echo "#define MFBSTIPPLEFILLAREA mfbStippleBlackArea" >> $@
	echo "#include \"$(srcdir)/mfbpntarea.c\"" >> $@

mfbpainv.c:
	echo "#define OPEQ MFB_OPEQ_INVERT" > $@
	echo "#define MFBSOLIDFILLAREA mfbSolidInvertArea" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_INVERT" >> $@
	echo "#define MFBSTIPPLEFILLAREA mfbStippleInvertArea" >> $@
	echo "#include \"$(srcdir)/mfbpntarea.c\"" >> $@

mfbtewhite.c:
	echo "#define OP MFB_OP_WHITE" > $@
	echo "#define MFBTEGLYPHBLT mfbTEGlyphBltWhite" >> $@
	echo "#define CLIPTETEXT mfbImageGlyphBltWhite" >> $@
	echo "#include \"$(srcdir)/mfbtegblt.c\"" >> $@

mfbteblack.c:
	echo "#define OP MFB_OP_BLACK" > $@
	echo "#define MFBTEGLYPHBLT mfbTEGlyphBltBlack" >> $@
	echo "#define CLIPTETEXT mfbImageGlyphBltBlack" >> $@
	echo "#include \"$(srcdir)/mfbtegblt.c\"" >> $@

mfbplywhite.c:
	echo "#define OPEQ MFB_OPEQ_WHITE" > $@
	echo "#define MFBFILLPOLY1RECT mfbFillPolyWhite" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_WHITE" >> $@
	echo "#include \"$(srcdir)/mfbply1rct.c\"" >> $@

mfbplyblack.c:
	echo "#define OPEQ MFB_OPEQ_BLACK" > $@
	echo "#define MFBFILLPOLY1RECT mfbFillPolyBlack" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_BLACK" >> $@
	echo "#include \"$(srcdir)/mfbply1rct.c\"" >> $@

mfbplyinv.c:
	echo "#define OPEQ MFB_OPEQ_INVERT" > $@
	echo "#define MFBFILLPOLY1RECT mfbFillPolyInvert" >> $@
	echo "#define EQWHOLEWORD MFB_EQWHOLEWORD_INVERT" >> $@
	echo "#include \"$(srcdir)/mfbply1rct.c\"" >> $@
