
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

AC_INIT([libxkbfile], [1.0.7],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
        [libxkbfile])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)
XORG_DEFAULT_OPTIONS
AM_CONFIG_HEADER(config.h)

# Check for progs
AC_PROG_CC
AC_PROG_LIBTOOL


AC_CHECK_FUNC(strcasecmp, [], AC_DEFINE([NEED_STRCASECMP], 1,
                                        [Do not have 'strcasecmp'.]))
AC_CHECK_FUNC(getc_unlocked, AC_DEFINE([HAVE_UNLOCKED_STDIO], 1,
				       ['Unlocked stdio']), [])

# Check for dependencies
PKG_CHECK_MODULES(XKBFILE, x11 kbproto)
AC_SUBST(XKBFILE_CFLAGS)
AC_SUBST(XKBFILE_LIBS)
		  
AC_OUTPUT([Makefile
	   src/Makefile
           xkbfile.pc])
