
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

AC_PREREQ([2.60])

AC_INIT([libfontenc], [1.1.0],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
        [libfontenc])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)
XORG_DEFAULT_OPTIONS
AM_CONFIG_HEADER(config.h)

# Check for progs
AC_PROG_LIBTOOL

# Require X.Org's font util macros 1.1 or later for XORG_FONTROOTDIR
m4_ifndef([XORG_FONT_MACROS_VERSION],
	  [m4_fatal([must install X.Org font-util 1.1 or later before running autoconf/autogen])])
XORG_FONT_MACROS_VERSION(1.1)

XORG_FONTSUBDIR([ENCODINGSDIR], [encodingsdir], [encodings])

# zlib
AC_CHECK_HEADER([zlib.h], [],
                AC_MSG_FAILURE([zlib.h is required to compile libfontenc]))
AC_CHECK_LIB(z, gzclose, [],
             AC_MSG_FAILURE([zlib is required to compile libfontenc]))

# Check for dependencies
PKG_CHECK_MODULES(FONTENC, xproto)

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
LINT_FLAGS="${LINT_FLAGS} ${FONTENC_CFLAGS}"
		  

AC_OUTPUT([Makefile
	   src/Makefile
           fontenc.pc])
