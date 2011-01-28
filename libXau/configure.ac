dnl 
dnl  Copyright © 2003 Keith Packard, Noah Levitt
dnl 
dnl  Permission to use, copy, modify, distribute, and sell this software and its
dnl  documentation for any purpose is hereby granted without fee, provided that
dnl  the above copyright notice appear in all copies and that both that
dnl  copyright notice and this permission notice appear in supporting
dnl  documentation, and that the name of Keith Packard not be used in
dnl  advertising or publicity pertaining to distribution of the software without
dnl  specific, written prior permission.  Keith Packard makes no
dnl  representations about the suitability of this software for any purpose.  It
dnl  is provided "as is" without express or implied warranty.
dnl 
dnl  KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
dnl  EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
dnl  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
dnl  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
dnl  PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ([2.60])
AC_INIT(libXau, 1.0.6, [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],libXau)
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

AC_PROG_INSTALL
AC_PROG_LN_S
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PROG_SED

PKG_CHECK_MODULES(XAU, xproto)

AC_ARG_ENABLE(xthreads,
              AC_HELP_STRING([--disable-xthreads],
                [Disable libXau support for Multithreading]),
              [xthreads=$enableval],[xthreads=yes])

if test "x$xthreads" = "xyes" ; then
    AC_DEFINE(XTHREADS,1,[Whether libXau is compiled with thread support])
    AC_CHECK_LIB(c, gethostbyname_r, [mtsafe=yes])
    if test "x$mtsafe" = "x" ; then
	AC_CHECK_LIB(nsl, gethostbyname_r, [mtsafe=yes])
    fi
    if test "x$mtsafe" = "xyes" ; then
	AC_DEFINE(XUSE_MTSAFE_API, 1, 
		  [Whether libXau needs to use MT safe API's])
    fi

dnl XXX incomplete, please fill this in
    case $host_os in
    solaris*)
	XTHREAD_CFLAGS="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS" ;;
    esac
    XAU_CFLAGS="$XAU_CFLAGS $XTHREAD_CFLAGS"
fi

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
XORG_LINT_LIBRARY([Xau])
LINT_FLAGS="${LINT_FLAGS} ${XAU_CFLAGS}"


AC_OUTPUT([Makefile
           man/Makefile
           xau.pc])
