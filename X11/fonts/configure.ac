AC_PREREQ([2.57])
AC_INIT([FontsProto], [2.1.0], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Require xorg-macros: XORG_CHANGELOG
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.2 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.2)
XORG_RELEASE_VERSION
XORG_CHANGELOG

AC_OUTPUT([Makefile
           fontsproto.pc])
