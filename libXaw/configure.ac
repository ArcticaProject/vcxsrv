dnl Process this file with autoconf to create configure.

AC_PREREQ([2.57])

AC_INIT([libXaw],
	1.0.7,
	[https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
	libXaw)

AM_INIT_AUTOMAKE([dist-bzip2])

AM_CONFIG_HEADER(config.h)

# Require xorg-macros: XORG_WITH_LINT, XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)

# Check for progs
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_SED

AC_PATH_PROGS([GROFF], [groff], [none], [$PATH:/usr/gnu/bin])

AC_MSG_CHECKING([whether to build documentation])
AC_ARG_ENABLE(docs, AC_HELP_STRING([--enable-docs],
				   [Enable building of Xaw documentation]),
	      [build_docs="${enableval}"], [build_docs="auto"])

if test "x${build_docs}" = xauto; then
    if test "x${GROFF}" = xnone ; then
	build_docs=no
    else
	build_docs=yes
    fi
fi
AC_MSG_RESULT([${build_docs}])
if test "x${build_docs}" = xyes && test "x${GROFF}" = xnone ; then
    AC_MSG_ERROR([can't build documentation without groff])
fi

AM_CONDITIONAL(BUILD_DOCS, [test x$build_docs = xyes])

# Need to call this explicitly since the first call to PKG_CHECK_MODULES
# is in an if statement, and later calls would break if it's skipped.
PKG_PROG_PKG_CONFIG

XORG_DEFAULT_OPTIONS

#
# fix libtool to set SONAME to libXaw.so.$major
#
AC_CONFIG_COMMANDS([libtool_hack], [
	cp -f libtool libtool_
	test -z "$SED" && SED=sed
	$SED '1,/^soname_spec/{
/^soname_spec/i\
# X.Org hack to match monolithic Xaw SONAME\
xorglibxawname="libXaw"
/^soname_spec/s/libname/xorglibxawname/
}' libtool_ > libtool
	rm -f libtool_
])

# OSX/Win32 rules are different.
platform_win32=no
platform_darwin=no
LIBEXT=so
case $host_os in
    cygwin*|mingw*)
	platform_win32=yes
	;;
    darwin*)
	LIBEXT=dylib
	platform_darwin=yes
	;;
esac
AC_SUBST(LIBEXT)
AM_CONDITIONAL(PLATFORM_WIN32, test "x$platform_win32" = "xyes")
AM_CONDITIONAL(PLATFORM_DARWIN, test "x$platform_darwin" = "xyes")

# Whether to build Xaw6

AC_ARG_ENABLE(xaw6, AC_HELP_STRING([--disable-xaw6],
				[Disable building of libXaw.so.6]),
			[build_v6=$enableval], [build_v6=yes])

if test "x$build_v6" = xyes; then
   PKG_CHECK_MODULES(XAW6, xproto x11 xext xextproto xt xmu)
   AC_SUBST(XAW6_CFLAGS)
   AC_SUBST(XAW6_LIBS)
fi


# Whether to build Xaw7

AC_ARG_ENABLE(xaw7, AC_HELP_STRING([--disable-xaw7],
				[Disable building of libXaw.so.7]),
			[build_v7=$enableval], [build_v7=yes])

if test "x$build_v7" = xyes; then
   PKG_CHECK_MODULES(XAW7, xproto x11 xext xextproto xt xmu xpm)
   AC_SUBST(XAW7_CFLAGS)
   AC_SUBST(XAW7_LIBS)
fi


AM_CONDITIONAL(BUILD_XAW6, [test x$build_v6 = xyes])
AM_CONDITIONAL(BUILD_XAW7, [test x$build_v7 = xyes])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(wctype.h,
       	AC_DEFINE([HAS_WCTYPE_H],1,
			[Define to 1 if you have the <wctype.h> header file.]))
AC_CHECK_HEADER(wchar.h,
       	AC_DEFINE([HAS_WCHAR_H],1,
			[Define to 1 if you have the <wchar.h> header file.]))
AC_CHECK_HEADER(widec.h, [],
       	AC_DEFINE([NO_WIDEC_H],1,
			[Define to 1 if you DO NOT have the <widec.h> header file.]))

# Checks for functions
AC_CHECK_FUNCS([iswalnum])

AC_OUTPUT([Makefile
	  include/Makefile
	  man/Makefile
	  spec/Makefile
	  src/Makefile])

if test "x$build_v6" = xyes; then
   AC_OUTPUT(xaw6.pc)
fi

if test "x$build_v7" = xyes; then
   AC_OUTPUT(xaw7.pc)
fi
