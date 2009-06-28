INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

AM_CFLAGS = -DHAVE_DIX_CONFIG_H

noinst_LIBRARIES = liblinux.a

if TSLIB
TSLIB_C = tslib.c
endif

if H3600_TS
TS_C = ts.c
endif

if KDRIVE_HW
KDRIVE_HW_SOURCES =	\
	agp.c		\
	agp.h		\
	evdev.c		\
	keyboard.c	\
	linux.c
endif

liblinux_a_SOURCES = 	\
	bus.c		\
	klinux.h	\
	mouse.c		\
	ms.c		\
	ps2.c		\
	$(KDRIVE_HW_SOURCES) \
	$(TSLIB_C)	\
	$(TS_C)
