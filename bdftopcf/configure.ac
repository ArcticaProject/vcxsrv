dnl  Copyright 2005 Red Hat, Inc.
dnl 
dnl  Permission to use, copy, modify, distribute, and sell this software
dnl  and its documentation for any purpose is hereby granted without fee,
dnl  provided that the above copyright notice appear in all copies and
dnl  that both that copyright notice and this permission notice appear in
dnl  supporting documentation, and that the name of Red Hat not be used in
dnl  advertising or publicity pertaining to distribution of the software
dnl  without specific, written prior permission.  Red Hat makes no
dnl  representations about the suitability of this software for any
dnl  purpose.  It is provided "as is" without express or implied warranty.
dnl 
dnl  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
dnl  NO EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
dnl  OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
dnl  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
dnl  USE OR PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ([2.57])
AC_INIT(bdftopcf, [1.0.1], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], bdftopcf)
AM_INIT_AUTOMAKE([dist-bzip2 foreign])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

# Require xorg-macros version 1.1.0 or newer for XORG_WITH_LINT macro
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.1 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.1)

AC_PROG_CC
AC_PROG_INSTALL

# Checks for pkg-config packages
PKG_CHECK_MODULES(BDFTOPCF, xfont)
AC_SUBST(BDFTOPCF_CFLAGS)
AC_SUBST(BDFTOPCF_LIBS)

XORG_WITH_LINT
XORG_MANPAGE_SECTIONS
XORG_RELEASE_VERSION

AC_OUTPUT([Makefile])
