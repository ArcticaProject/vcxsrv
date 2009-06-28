if KDRIVEFBDEV
FBDEV_INCLUDES =-I$(top_srcdir)/hw/kdrive/fbdev
FBDEV_LIBS = $(top_builddir)/hw/kdrive/fbdev/libfbdev.a
endif

if KDRIVEVESA
VESA_INCLUDES = -I$(top_srcdir)/hw/kdrive/vesa
VESA_LIBS = $(top_builddir)/hw/kdrive/vesa/libvesa.a
endif

INCLUDES =                      \
	@KDRIVE_INCS@               \
	$(FBDEV_INCLUDES)           \
	$(VESA_INCLUDES)            \
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xneomagic

noinst_LIBRARIES = libneomagic.a

libneomagic_a_SOURCES =         \
	neomagic.c                  \
	neomagic.h                  \
	neo_draw.c

Xneomagic_SOURCES = \
	neomagicstub.c

NEOMAGIC_LIBS =                 \
	libneomagic.a               \
	${FBDEV_LIBS}               \
	${VESA_LIBS}                \
	@KDRIVE_LIBS@

if GLX
Xneomagic_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xneomagic_LDADD = \
	$(NEOMAGIC_LIBS)            \
	@KDRIVE_LIBS@               \
	@XSERVER_LIBS@		  

Xneomagic_DEPENDENCIES =	\
	libneomagic.a               \
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
