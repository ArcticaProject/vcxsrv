INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

noinst_LIBRARIES = libfake.a

bin_PROGRAMS = Xfake

libfake_a_SOURCES =	\
	fake.c		\
	kbd.c		\
	os.c		\
	mouse.c		\
	fake.h

Xfake_SOURCES = \
	fakeinit.c

Xfake_LDADD = 						\
	libfake.a					\
	@KDRIVE_LIBS@					\
        @XSERVER_LIBS@

Xfake_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

Xfake_DEPENDENCIES =	\
	libfake.a					\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
