#FONTENCDIR=@ENCODINGSDIR@
#FONTENCDEFS = -DFONT_ENCODINGS_DIRECTORY=\"$(FONTENCDIR)/encodings.dir\"

LIBRARY = libfontfile

INCLUDES += $(MHMAKECONF)\zlib\src\zlib-1.2.3

#DEFINES += XFONT_TYPE1 XFONT_SPEEDO XFONT_BITMAP XFONT_FREETYPE BUILD_SPEEDO BUILD_TYPE1 BUILD_FREETYPE X_GZIP_FONT_COMPRESSION
DEFINES += X_GZIP_FONT_COMPRESSION

CSRCS = bitsource.c		\
	bufio.c			\
	decompress.c		\
	defaults.c		\
	dirfile.c		\
	ffcheck.c		\
	fileio.c		\
	filewr.c		\
	fontdir.c		\
	fontencc.c		\
	fontfile.c		\
	fontscale.c		\
	gunzip.c		\
	printerfont.c		\
	register.c		\
	renderers.c		\
	catalogue.c

#if X_BZIP2_FONT_COMPRESSION
#libfontfile_la_SOURCES += bunzip2.c
#endif
INCLUDES := ../../../../build $(INCLUDES)

