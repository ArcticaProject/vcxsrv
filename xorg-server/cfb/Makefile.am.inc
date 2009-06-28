libcfb_gen_sources = cfbseg.c cfbfillarcC.c cfbfillarcG.c cfbzerarcC.c cfbzerarcX.c cfbzerarcG.c \
	cfbbltC.c cfbbltX.c cfbbltO.c cfbbltG.c cfbsolidC.c cfbsolidX.c cfbsolidG.c cfbtileoddC.c \
	cfbtileoddG.c cfbtile32C.c cfbtile32G.c cfb8lineCO.c cfb8lineCP.c cfb8lineX.c cfb8lineG.c \
	cfb8segCS.c cfb8segC.c cfb8segX.c cfb8setG.c cfbply1rctC.c cfbply1rctG.c

DISTCLEANFILES = $(libcfb_gen_sources)

CFB_INCLUDES = -I$(top_srcdir)/mfb -I$(top_srcdir)/cfb

libcfb_common_sources =  $(top_srcdir)/cfb/cfbgc.c $(top_srcdir)/cfb/cfbrrop.c \
	$(top_srcdir)/cfb/cfbwindow.c \
	$(top_srcdir)/cfb/cfbmskbits.c $(top_srcdir)/cfb/cfbpixmap.c \
	$(top_srcdir)/cfb/cfbbitblt.c $(top_srcdir)/cfb/cfbfillsp.c \
	$(top_srcdir)/cfb/cfbsetsp.c $(top_srcdir)/cfb/cfbscrinit.c \
	$(top_srcdir)/cfb/cfballpriv.c $(top_srcdir)/cfb/cfbgetsp.c \
	$(top_srcdir)/cfb/cfbfillrct.c $(top_srcdir)/cfb/cfbigblt8.c \
	$(top_srcdir)/cfb/cfbglblt8.c $(top_srcdir)/cfb/cfbtegblt.c \
	$(top_srcdir)/cfb/cfbpolypnt.c \
	$(top_srcdir)/cfb/cfbbres.c $(top_srcdir)/cfb/cfbline.c \
	$(top_srcdir)/cfb/cfbhrzvert.c $(top_srcdir)/cfb/cfbbresd.c \
	$(top_srcdir)/cfb/cfbimage.c $(top_srcdir)/cfb/cfbcppl.c \
	$(top_srcdir)/cfb/cfbcmap.c

cfbseg.c:
	echo "#define POLYSEGMENT" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbline.c\"" >> $@

cfbfillarcC.c:
	echo "#define RROP GXcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbfillarc.c\"" >> $@

cfbfillarcG.c:
	echo "#define RROP GXset" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbfillarc.c\"" >> $@

cfbzerarcC.c:
	echo "#define RROP GXcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbzerarc.c\"" >> $@

cfbzerarcX.c:
	echo "#define RROP GXxor" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbzerarc.c\"" >> $@

cfbzerarcG.c:
	echo "#define RROP GXset" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbzerarc.c\"" >> $@

cfbbltC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbblt.c\"" >> $@

cfbbltX.c:
	echo "#define MROP Mxor" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbblt.c\"" >> $@

cfbbltO.c:
	echo "#define MROP Mor" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbblt.c\"" >> $@

cfbbltG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbblt.c\"" >> $@

cfbsolidC.c:
	echo "#define RROP GXcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbsolid.c\"" >> $@

cfbsolidX.c:
	echo "#define RROP GXxor" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbsolid.c\"" >> $@

cfbsolidG.c:
	echo "#define RROP GXset" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbsolid.c\"" >> $@

cfbtileoddC.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbtileodd.c\"" >> $@

cfbtileoddG.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbtileodd.c\"" >> $@

cfbtile32C.c:
	echo "#define MROP Mcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbtile32.c\"" >> $@

cfbtile32G.c:
	echo "#define MROP 0" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbtile32.c\"" >> $@

cfb8lineCO.c:
	echo "#define RROP GXcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8lineCP.c:
	echo "#define RROP GXcopy" > $@
	echo "#define PREVIOUS" >> $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8lineX.c:
	echo "#define RROP GXxor" > $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8lineG.c:
	echo "#define RROP GXset" > $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8segCS.c:
	echo "#define RROP GXcopy" > $@
	echo "#define POLYSEGMENT" >> $@
	echo "#define WIDTH_SHIFT" >> $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8segC.c:
	echo "#define RROP GXcopy" > $@
	echo "#define POLYSEGMENT" >> $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8segX.c:
	echo "#define RROP GXxor" > $@
	echo "#define POLYSEGMENT" >> $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfb8setG.c:
	echo "#define RROP GXset" > $@
	echo "#define POLYSEGMENT" >> $@
	echo "#include \"$(top_srcdir)/cfb/cfb8line.c\"" >> $@

cfbply1rctC.c:
	echo "#define RROP GXcopy" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbply1rct.c\"" >> $@

cfbply1rctG.c:
	echo "#define RROP GXset" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbply1rct.c\"" >> $@

cfbglrop8.c:
	echo "#define GLYPHROP" > $@
	echo "#include \"$(top_srcdir)/cfb/cfbglblt8.c\"" >> $@


if XPRINT

PLATFORMDEFS = -DXFREE86

cfb8bit.o:   compiler.h

compiler.h:
	echo "#include \"$(top_srcdir)/hw/xfree86/common/compiler.h\"" >> $@

endif

