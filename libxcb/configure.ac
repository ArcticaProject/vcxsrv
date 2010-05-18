#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([libxcb],
        1.6,
        [xcb@lists.freedesktop.org])
AC_CONFIG_SRCDIR([xcb.pc.in])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_PATH_PYTHON([2.5])

PKG_CHECK_MODULES(CHECK, [check >= 0.9.4], [HAVE_CHECK=yes], [HAVE_CHECK=no])
AM_CONDITIONAL(HAVE_CHECK, test x$HAVE_CHECK = xyes)

AC_CONFIG_HEADERS([src/config.h])

AC_PROG_LIBTOOL
AC_PROG_CC

AC_PATH_PROG(XSLTPROC, xsltproc, no)
if test "$XSLTPROC" = "no"; then
   AC_MSG_ERROR([XCB requires xsltproc.])
fi

HTML_CHECK_RESULT=false
if test x"$HAVE_CHECK" = xyes; then
	if test x"$XSLTPROC" != xno; then
		HTML_CHECK_RESULT=true
	fi
fi
AC_SUBST(HTML_CHECK_RESULT)

# Checks for pkg-config packages
PKG_CHECK_MODULES(XCBPROTO, xcb-proto >= 1.6)
NEEDED="pthread-stubs xau >= 0.99.2"
PKG_CHECK_MODULES(NEEDED, $NEEDED)

have_xdmcp="no"
PKG_CHECK_MODULES(XDMCP, xdmcp,
	AC_CHECK_LIB(Xdmcp, XdmcpWrap,
		[
		AC_DEFINE(HASXDMAUTH,1,[Has Wraphelp.c needed for XDM AUTH protocols])
		NEEDED="$NEEDED xdmcp"
		have_xdmcp="yes"
		],
		[
		XDMCP_CFLAGS=
		XDMCP_LIBS=
		], [$XDMCP_LIBS]),
	[AC_MSG_RESULT(no)])

AC_SUBST(NEEDED)

# Find the xcb-proto protocol descriptions
AC_MSG_CHECKING(XCBPROTO_XCBINCLUDEDIR)
XCBPROTO_XCBINCLUDEDIR=`$PKG_CONFIG --variable=xcbincludedir xcb-proto`
AC_MSG_RESULT($XCBPROTO_XCBINCLUDEDIR)
AC_SUBST(XCBPROTO_XCBINCLUDEDIR)

# Find the xcb-proto version
XCBPROTO_VERSION=`$PKG_CONFIG --modversion xcb-proto`
AC_SUBST(XCBPROTO_VERSION)

# Find the xcbgen Python package
AC_MSG_CHECKING(XCBPROTO_XCBPYTHONDIR)
XCBPROTO_XCBPYTHONDIR=`$PKG_CONFIG --variable=pythondir xcb-proto`
AC_MSG_RESULT($XCBPROTO_XCBPYTHONDIR)
AC_SUBST(XCBPROTO_XCBPYTHONDIR)

AC_HEADER_STDC
AC_SEARCH_LIBS(getaddrinfo, socket)
AC_SEARCH_LIBS(connect, socket)

case $host_os in
linux*)
	AC_DEFINE([HAVE_ABSTRACT_SOCKETS], 1, [Define if your platform supports abstract sockets])
	;;
esac

dnl define buffer queue size
AC_ARG_WITH([queue-size],
            AC_HELP_STRING([--with-queue-size=SIZE],
            [Set the XCB buffer queue size (default is 16384)]),
            [xcb_queue_buffer_size="$withval"],
            [xcb_queue_buffer_size=16384])
AC_DEFINE_UNQUOTED(XCB_QUEUE_BUFFER_SIZE, [$xcb_queue_buffer_size],
                   [XCB buffer queue size])

dnl check for the sockaddr_un.sun_len member
AC_CHECK_MEMBER([struct sockaddr_un.sun_len],
		[AC_DEFINE(HAVE_SOCKADDR_SUN_LEN,1,[Have the sockaddr_un.sun_len member.])],
		[],
		[ #include <sys/types.h>
		  #include <sys/un.h>
		])

xcbincludedir='${includedir}/xcb'
AC_SUBST(xcbincludedir)

if  test "x$GCC" = xyes ; then
    CWARNFLAGS="-Wall -pedantic -Wpointer-arith \
		-Wstrict-prototypes -Wmissing-declarations -Wnested-externs"
else
    AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
    if test "x$SUNCC" = "xyes"; then
	CWARNFLAGS="-v"
    fi
fi
AC_SUBST(CWARNFLAGS)

XCB_CHECK_VISIBILITY()

# htmldir is not defined prior to autoconf 2.59c, so on earlier versions
# set an equivalent value.
AC_PREREQ([2.59c], [], [AC_SUBST([htmldir], [m4_ifset([AC_PACKAGE_TARNAME],
	  ['${datadir}/doc/${PACKAGE_TARNAME}'],
	  ['${datadir}/doc/${PACKAGE}'])
])])

XCB_CHECK_DOXYGEN()

case $host_os in
	# darwin has poll() but can't be used to poll character devices (atleast through SnowLeopard)
	darwin*) ;;
	*)
		AC_CHECK_FUNC(poll, [AC_DEFINE(USE_POLL, 1, [poll() function is available])], )
	;;
esac

XCB_EXTENSION(Composite, "yes")
XCB_EXTENSION(Damage, "yes")
XCB_EXTENSION(DPMS, "yes")
XCB_EXTENSION(DRI2, "yes")
XCB_EXTENSION(GLX, "yes")
XCB_EXTENSION(RandR, "yes")
XCB_EXTENSION(Record, "yes")
XCB_EXTENSION(Render, "yes")
XCB_EXTENSION(Resource, "yes")
XCB_EXTENSION(Screensaver, "yes")
XCB_EXTENSION(Shape, "yes")
XCB_EXTENSION(Shm, "yes")
XCB_EXTENSION(Sync, "yes")
XCB_EXTENSION(Xevie, "yes")
XCB_EXTENSION(XFixes, "yes")
XCB_EXTENSION(XFree86-DRI, "yes")
XCB_EXTENSION(Xinerama, "yes")
XCB_EXTENSION(XInput, "no")
XCB_EXTENSION(Xprint, "yes")
XCB_EXTENSION(SELinux, "no")
XCB_EXTENSION(XTest, "yes")
XCB_EXTENSION(Xv, "yes")
XCB_EXTENSION(XvMC, "yes")

AC_ARG_WITH(launchd, AS_HELP_STRING([--with-launchd], [Build with support for Apple's launchd (default: auto)]), [LAUNCHD=$withval], [LAUNCHD=auto])
if test "x$LAUNCHD" = xauto; then
	unset LAUNCHD
	AC_CHECK_PROG(LAUNCHD, [launchd], [yes], [no])
fi

if test "x$LAUNCHD" = xyes ; then
	AC_DEFINE(HAVE_LAUNCHD, 1, [launchd support available])
fi

AC_CONFIG_FILES([
Makefile
doc/Makefile
src/Makefile
tests/Makefile
])

AC_CONFIG_FILES([
xcb.pc
xcb-composite.pc
xcb-damage.pc
xcb-dpms.pc
xcb-dri2.pc
xcb-glx.pc
xcb-randr.pc
xcb-record.pc
xcb-render.pc
xcb-res.pc
xcb-screensaver.pc
xcb-shape.pc
xcb-shm.pc
xcb-sync.pc
xcb-xevie.pc
xcb-xf86dri.pc
xcb-xfixes.pc
xcb-xinerama.pc
xcb-xinput.pc
xcb-xprint.pc
xcb-xselinux.pc
xcb-xtest.pc
xcb-xv.pc
xcb-xvmc.pc
])

AC_CONFIG_FILES([
doc/xcb.doxygen
])

AC_OUTPUT

dnl Configuration output

echo ""
echo "  Package: ${PACKAGE_NAME} ${PACKAGE_VERSION}"
echo ""
echo "  Configuration"
echo "    XDM support.........: ${have_xdmcp}"
echo "    Build unit tests....: ${HAVE_CHECK}"
echo "    XCB buffer size.....: ${xcb_queue_buffer_size}"
echo ""
echo "  Used CFLAGS:"
echo "    CPPFLAGS............: ${CPPFLAGS}"
echo "    CFLAGS..............: ${CFLAGS}"
echo "    Warning CFLAGS......: ${CWARNFLAGS}"
echo ""
echo "  Installation:"
echo "    Prefix..............: ${prefix}"
echo ""
