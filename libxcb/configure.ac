#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([libxcb],
        1.1,
        [xcb@lists.freedesktop.org])
AC_CONFIG_SRCDIR([xcb.pc.in])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

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
PKG_CHECK_MODULES(XCBPROTO, xcb-proto >= 1.1)
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

AC_CHECK_HEADER([execinfo.h], [AC_DEFINE(HAVE_BACKTRACE,1,[Has backtrace*() needed for retrieving stack traces])])

AC_SUBST(NEEDED)

# Find the xcb-proto protocol descriptions
AC_MSG_CHECKING(XCBPROTO_XCBINCLUDEDIR)
XCBPROTO_XCBINCLUDEDIR=`$PKG_CONFIG --variable=xcbincludedir xcb-proto`
AC_MSG_RESULT($XCBPROTO_XCBINCLUDEDIR)
AC_SUBST(XCBPROTO_XCBINCLUDEDIR)

AC_HEADER_STDC
AC_SEARCH_LIBS(getaddrinfo, socket)
AC_SEARCH_LIBS(connect, socket)

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

GCC_CHECK_VISIBILITY()

# htmldir is not defined prior to autoconf 2.59c, so on earlier versions
# set an equivalent value.
AC_PREREQ([2.59c], [], [AC_SUBST([htmldir], [m4_ifset([AC_PACKAGE_TARNAME],
	  ['${datadir}/doc/${PACKAGE_TARNAME}'],
	  ['${datadir}/doc/${PACKAGE}'])
])])
AM_CHECK_DOXYGEN()

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile doc/Makefile])
AC_CONFIG_FILES([xcb.pc xcb-xlib.pc xcb-composite.pc xcb-damage.pc xcb-dpms.pc xcb-glx.pc xcb-randr.pc xcb-record.pc xcb-render.pc xcb-res.pc xcb-screensaver.pc xcb-shape.pc xcb-shm.pc xcb-sync.pc xcb-xevie.pc xcb-xf86dri.pc xcb-xfixes.pc xcb-xinerama.pc xcb-xprint.pc xcb-xtest.pc xcb-xv.pc xcb-xvmc.pc])
AC_CONFIG_FILES([doc/xcb.doxygen])

AC_OUTPUT

dnl Configuration output

echo ""
echo "  Package: ${PACKAGE_NAME} ${PACKAGE_VERSION}"
echo ""
echo "  Configuration"
echo "    XDM support.........: ${have_xdmcp}"
echo "    Build unit tests....: ${HAVE_CHECK}"
echo ""
echo "  Used CFLAGS:"
echo "    CPPFLAGS............: ${CPPFLAGS}"
echo "    CFLAGS..............: ${CFLAGS}"
echo "    Warning CFLAGS......: ${CWARNFLAGS}"
echo ""
echo "  Installation:"
echo "    Prefix..............: ${prefix}"
echo ""
