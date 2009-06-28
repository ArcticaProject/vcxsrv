noinst_LTLIBRARIES = libafb.la

AM_CFLAGS = $(DIX_CFLAGS)

libafb_gen_sources = afbbltC.c afbbltX.c afbbltCI.c afbbltO.c afbbltG.c afbtileC.c afbtileG.c afbseg.c

DISTCLEANFILES = $(libafb_gen_sources)

libafb_la_SOURCES = afbgc.c afbwindow.c afbfont.c afbfillrct.c afbpixmap.c \
	 afbimage.c afbline.c afbbres.c afbhrzvert.c afbbresd.c afbpushpxl.c afbply1rct.c \
	 afbzerarc.c afbfillarc.c afbfillsp.c afbsetsp.c afbscrinit.c afbplygblt.c \
	 afbclip.c afbgetsp.c afbpolypnt.c afbbitblt.c afbcmap.c afbimggblt.c afbpntarea.c \
	 afbmisc.c afbtegblt.c $(libafb_gen_sources)

INCLUDES = -I$(top_srcdir)/mfb -I$(top_srcdir)/hw/xfree86/os-support

EXTRA_DIST = afbline.c afbblt.c afbtile.c README Xdaniver.doc

afbseg.c:
	echo "#define POLYSEGMENT" > $@
	echo "#include \"$(srcdir)/afbline.c\"" >> $@

afbbltC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(srcdir)/afbblt.c\"" >> $@

afbbltX.c:
	echo "#define MROP Mxor" > $@
	echo "#include \"$(srcdir)/afbblt.c\"" >> $@

afbbltCI.c:
	echo "#define MROP McopyInverted" > $@
	echo "#include \"$(srcdir)/afbblt.c\"" >> $@

afbbltO.c:
	echo "#define MROP Mor" > $@
	echo "#include \"$(srcdir)/afbblt.c\"" >> $@

afbbltG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(srcdir)/afbblt.c\"" >> $@

afbtileC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(srcdir)/afbtile.c\"" >> $@

afbtileG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(srcdir)/afbtile.c\"" >> $@

sdk_HEADERS = afb.h
