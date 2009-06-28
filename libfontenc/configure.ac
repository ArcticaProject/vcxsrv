
dnl  Copyright 2005 Red Hat, Inc.
dnl 
dnl  Permission to use, copy, modify, distribute, and sell this software and its
dnl  documentation for any purpose is hereby granted without fee, provided that
dnl  the above copyright notice appear in all copies and that both that
dnl  copyright notice and this permission notice appear in supporting
dnl  documentation, and that the name of Red Hat not be used in
dnl  advertising or publicity pertaining to distribution of the software without
dnl  specific, written prior permission.  Red Hat makes no
dnl  representations about the suitability of this software for any purpose.  It
dnl  is provided "as is" without express or implied warranty.
dnl 
dnl  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
dnl  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
dnl  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
dnl  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
dnl  PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ([2.57])

AC_INIT(libfontenc, 1.0.4, [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], libfontenc)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

# Require xorg-macros version 1.1.0 or newer for XORG_WITH_LINT macro
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.1 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.1)

# Check for progs
AC_PROG_CC
AC_PROG_LIBTOOL

m4_ifdef([AS_HELP_STRING], , [m4_define([AS_HELP_STRING], m4_defn([AC_HELP_STRING]))])

DEFAULT_ENCODINGSDIR=${libdir}/X11/fonts/encodings
AC_ARG_WITH(encodingsdir,
	AS_HELP_STRING([--with-encodingsdir=ENCODINGSDIR], [Path to font encodings]),
	[ENCODINGSDIR="$withval"],
	[ENCODINGSDIR="$DEFAULT_ENCODINGSDIR"])
AC_SUBST(ENCODINGSDIR)

# zlib

AC_CHECK_LIB(z, gzclose) 

# Check for dependencies
PKG_CHECK_MODULES(FONTENC, xproto)

AC_SUBST(FONTENC_CFLAGS)
AC_SUBST(FONTENC_LIBS)

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
LINT_FLAGS="${LINT_FLAGS} ${FONTENC_CFLAGS}"
		  
XORG_RELEASE_VERSION

AC_OUTPUT([Makefile
	   src/Makefile
           fontenc.pc])
