AC_PREREQ([2.60])
AC_INIT([XExtProto], [7.2.0], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg])
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

AC_OUTPUT([Makefile
           specs/Makefile
           xextproto.pc])
