#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libXext],
        [1.2.99.0],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
        [libXext])

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE

# Require xorg-macros minimum of 1.10 for DocBook XML documentation
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.10 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.10)
XORG_DEFAULT_OPTIONS
XORG_ENABLE_SPECS
XORG_WITH_XMLTO(0.0.20)
XORG_WITH_FOP
XORG_CHECK_SGML_DOCTOOLS(1.5)
AC_CONFIG_HEADERS([config.h])

# Determine .so library version per platform 
# based on SharedXextRev in monolith xc/config/cf/*Lib.tmpl
AC_CANONICAL_HOST
if test "x$XEXT_SOREV" = "x" ; then
    case $host_os in
	openbsd*)	XEXT_SOREV=8:0 		;;
	solaris*)       XEXT_SOREV=0 		;;
	*)              XEXT_SOREV=6:4:0 	;;
    esac
fi
AC_SUBST(XEXT_SOREV)

# Checks for programs.
AC_PROG_LIBTOOL

# Checks for pkg-config packages
PKG_CHECK_MODULES(XEXT, [xproto >= 7.0.13] [x11 >= 1.1.99.1] [xextproto >= 7.1.99])
AC_SUBST(XEXT_CFLAGS)
AC_SUBST(XEXT_LIBS)

XORG_CHECK_MALLOC_ZERO

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
XORG_LINT_LIBRARY([Xext])

AC_OUTPUT([Makefile
           man/Makefile
	   src/Makefile
	   specs/Makefile
	   xext.pc])
