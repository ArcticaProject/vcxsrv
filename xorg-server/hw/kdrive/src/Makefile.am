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

libkdrive_a_SOURCES =	\
	fourcc.h	\
	kaa.c		\
	kaa.h		\
	kaapict.c	\
	kasync.c	\
	kcmap.c		\
	kcurscol.c	\
	kdrive.c	\
	kdrive.h	\
	kinfo.c		\
	kinput.c	\
	kkeymap.c	\
	kmap.c		\
	kmode.c		\
	knoop.c		\
	koffscreen.c	\
	kshadow.c	\
	ktest.c		\
	kxv.c		\
	kxv.h		\
	$(KDRIVE_HW_SOURCES) \
        $(top_srcdir)/mi/miinitext.c

libkdrivestubs_a_SOURCES = \
        $(top_srcdir)/fb/fbcmap_mi.c
