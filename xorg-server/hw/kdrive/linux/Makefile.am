INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

AM_CFLAGS = -DHAVE_DIX_CONFIG_H

noinst_LTLIBRARIES = liblinux.la

if TSLIB
TSLIB_C = tslib.c
endif

KDRIVE_HW_SOURCES =	\
	evdev.c		\
	keyboard.c	\
	linux.c

liblinux_la_SOURCES = 	\
	bus.c		\
	klinux.h	\
	mouse.c		\
	ms.c		\
	ps2.c		\
	$(KDRIVE_HW_SOURCES) \
	$(TSLIB_C)
