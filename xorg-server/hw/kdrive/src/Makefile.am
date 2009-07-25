INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

AM_CFLAGS =	-DHAVE_DIX_CONFIG_H

noinst_LIBRARIES = libkdrive.a libkdrivestubs.a

if KDRIVE_HW
KDRIVE_HW_SOURCES =	\
	vga.c		\
	vga.h
endif

if XV
KDRIVE_XV_SOURCES =	\
	kxv.c		\
	kxv.h
endif

libkdrive_a_SOURCES =	\
	fourcc.h	\
	kcmap.c		\
	kcurscol.c	\
	kdrive.c	\
	kdrive.h	\
	kinfo.c		\
	kinput.c	\
	kkeymap.c	\
	kmap.c		\
	kmode.c		\
	kshadow.c	\
	$(KDRIVE_XV_SOURCES) \
	$(KDRIVE_HW_SOURCES) \
        $(top_srcdir)/mi/miinitext.c

libkdrivestubs_a_SOURCES = \
        $(top_srcdir)/fb/fbcmap_mi.c
