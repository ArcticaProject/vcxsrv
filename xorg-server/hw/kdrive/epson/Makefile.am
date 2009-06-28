INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xepson

noinst_LIBRARIES = libepson.a

libepson_a_SOURCES = 	\
	epson13806.c	\
	epson13806.h	\
	epson13806draw.c\
	epson13806draw.h\
	epson13806reg.h

Xepson_SOURCES = \
	epson13806stub.c

EPSON_LIBS =					\
	libepson.a 				\
	@KDRIVE_LIBS@

if GLX
Xepson_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xepson_LDADD = \
	$(EPSON_LIBS)			       \
	@KDRIVE_LIBS@

Xepson_DEPENDENCIES =	\
	libepson.a 				\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
