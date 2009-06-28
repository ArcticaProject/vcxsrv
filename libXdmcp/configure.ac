dnl 
dnl  $Id$
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

AC_PREREQ([2.57])
AC_INIT(libXdmcp, 1.0.2, [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],libXdmcp)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

# Require xorg-macros version 1.1.0 or newer for XORG_WITH_LINT macro
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.1 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.1)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET

AC_CHECK_FUNCS([srand48 lrand48])

AC_SEARCH_LIBS([recvfrom],[socket])

PKG_CHECK_MODULES(XDMCP, xproto)
AC_SUBST(XDMCP_CFLAGS)
AC_SUBST(XDMCP_LIBS)

if test -f ${srcdir}/Wraphelp.c; then
	AC_DEFINE(HASXDMAUTH,1,[Has Wraphelp.c needed for XDM AUTH protocols])
	HASXDMAUTH=yes
else
	HASXDMAUTH=no
fi

AM_CONDITIONAL(HASXDMAUTH,test x$HASXDMAUTH = xyes)

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
XORG_LINT_LIBRARY([Xdmcp])

XORG_RELEASE_VERSION

AC_OUTPUT([Makefile
           xdmcp.pc])
