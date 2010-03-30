noinst_LTLIBRARIES = libos.la liblog.la

AM_CFLAGS = $(DIX_CFLAGS) $(SHA1_CFLAGS)

SECURERPC_SRCS = rpcauth.c
XDMCP_SRCS = xdmcp.c
STRLCAT_SRCS = strlcat.c strlcpy.c

# Build a convenience library liblog.la that will be added into
# libos.la. The split is done so that log.c can be built with
# different compiler options.
liblog_la_SOURCES = log.c
# Add flags needed for proper backtraces of functions marked with GCC
# __attribute__((noreturn)). Currently those flags are needed for
# FatalError and AbortServer in log.c.
liblog_la_CFLAGS = $(AM_CFLAGS) $(ARM_BACKTRACE_CFLAGS)

libos_la_SOURCES = 	\
	WaitFor.c	\
	access.c	\
	auth.c		\
	backtrace.c	\
	connection.c	\
	io.c		\
	mitauth.c	\
	oscolor.c	\
	osdep.h		\
	osinit.c	\
	utils.c		\
	strcasecmp.c	\
	strcasestr.c	\
	xdmauth.c	\
	xsha1.c		\
	xstrans.c	\
	xprintf.c
libos_la_LIBADD = @SHA1_LIBS@ $(DLOPEN_LIBS) liblog.la

if SECURE_RPC
libos_la_SOURCES += $(SECURERPC_SRCS)
endif

if XDMCP
libos_la_SOURCES += $(XDMCP_SRCS)
endif

if NEED_STRLCAT
libos_la_SOURCES += $(STRLCAT_SRCS)
endif

EXTRA_DIST = $(SECURERPC_SRCS) $(INTERNALMALLOC_SRCS) \
     $(XDMCP_SRCS) $(STRLCAT_SRCS)

if SPECIAL_DTRACE_OBJECTS
# Generate dtrace object code for probes in libos & libdix
dtrace.o: $(top_srcdir)/dix/Xserver.d $(am_libos_la_OBJECTS)
	$(AM_V_GEN)$(DTRACE) -G -C -o $@ -s $(top_srcdir)/dix/Xserver.d .libs/*.o ../dix/.libs/*.o

noinst_PROGRAMS = os.O

os.O: dtrace.o $(am_libos_la_OBJECTS)
	$(AM_V_GEN)ld -r -o $@ dtrace.o .libs/*.o
endif

os.c:
	touch $@

CLEANFILES = os.c
