INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xi810

noinst_LIBRARIES = libi810.a


libi810_a_SOURCES = 		\
	i810_cursor.c	\
	i810_video.c	\
	i810draw.c	\
	i810draw.h	\
	i810_reg.h	\
	i810.c		\
	i810.h

Xi810_SOURCES = \
	i810stub.c

I810_LIBS =						\
	libi810.a 					\
	@KDRIVE_LIBS@

if GLX
Xi810_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xi810_LDADD = \
	$(I810_LIBS)					\
	@KDRIVE_LIBS@

Xi810_DEPENDENCIES =	\
	libi810.a 					\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
