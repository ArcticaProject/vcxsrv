#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([libX11],
        1.2.99.901,
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
        libX11)
AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([dist-bzip2 foreign])

AM_MAINTAINER_MODE

AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_HEADER([include/X11/XlibConf.h])

# Require xorg-macros: XORG_CWARNFLAGS, XORG_CHANGELOG, XORG_WITH_LINT
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.2 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.2)

# Set common system defines for POSIX extensions, such as _GNU_SOURCE
# Must be called before any macros that run the compiler (like AC_PROG_LIBTOOL)
# to avoid autoconf errors.
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_LIBTOOL
DOLT
AC_PROG_CC
XORG_CWARNFLAGS

if test x"$CC_FOR_BUILD" = x; then
	if test x"$cross_compiling" = xyes; then
	       AC_CHECK_PROGS(CC_FOR_BUILD, gcc cc)
	else
	       CC_FOR_BUILD="$CC"
	fi
fi
AC_SUBST([CC_FOR_BUILD])

XORG_PROG_RAWCPP

# Build with XCB support?
AC_ARG_WITH(xcb,
	AC_HELP_STRING([--with-xcb], [use XCB for low-level protocol implementation]),
	[ac_cv_use_xcb=$withval], [ac_cv_use_xcb=yes])
AC_CACHE_CHECK([whether to use XCB], [ac_cv_use_xcb], [ac_cv_use_xcb=yes])
AM_CONDITIONAL(XCB, test x$ac_cv_use_xcb != xno)

# Checks for pkg-config packages
PKG_CHECK_MODULES(XPROTO, [xproto >= 7.0.13])
AC_SUBST(XPROTO_CFLAGS)

case "$ac_cv_use_xcb" in
no)
	X11_REQUIRES="xau xcmiscproto bigreqsproto"
	X11_EXTRA_DEPS="xau xdmcp"
	PKG_CHECK_MODULES(XDMCP, xdmcp)
	AC_CHECK_LIB(Xdmcp, XdmcpWrap, [xdmauth="yes"], [xdmauth="no"], [$XDMCP_LIBS])
	AC_DEFINE(USE_XCB, 0, [Use XCB for low-level protocol implementation])
	;;
*)
	X11_REQUIRES="xcb >= 1.1.92"
	X11_EXTRA_DEPS="xcb >= 1.1.92"
	xdmauth="no" # XCB handles all auth
	AC_DEFINE(USE_XCB, 1, [Use XCB for low-level protocol implementation])
	;;
esac
AC_SUBST(X11_EXTRA_DEPS)
PKG_CHECK_MODULES(X11, xextproto xtrans $X11_REQUIRES)

dnl Issue an error if xtrans.m4 was not found and XTRANS_CONNECTION_FLAGS macro
dnl was not expanded, since libX11 with no transport types is rather useless.
dnl
dnl If you're seeing an error here, be sure you installed the lib/xtrans module
dnl first and if it's not in the default location, that you set the ACLOCAL
dnl environment variable to find it, such as:
dnl	ACLOCAL="aclocal -I ${PREFIX}/share/aclocal"
m4_pattern_forbid(XTRANS_CONNECTION_FLAGS)

# Transport selection macro from xtrans.m4
XTRANS_CONNECTION_FLAGS

# Secure RPC detection macro from xtrans.m4
XTRANS_SECURE_RPC_FLAGS

# Preferred order to try transports for local connections
AC_MSG_CHECKING([what order to try transports in for local connections])
case $host_os in
	solaris*)
		# On Solaris 2.6 through 9, named pipes (LOCAL_TRANS) were
		# faster than Unix domain sockets, but on Solaris 10 & later,
		# Unix domain sockets are faster now.
		DEFAULT_LOCAL_TRANS="UNIX_TRANS,LOCAL_TRANS,TCP_TRANS"
		;;
	*)
		if test "$LOCALCONN" = "yes"; then
			DEFAULT_LOCAL_TRANS="LOCAL_TRANS,UNIX_TRANS,TCP_TRANS"
		else
			DEFAULT_LOCAL_TRANS="UNIX_TRANS,TCP_TRANS"
		fi
		;;
esac

AC_ARG_WITH(local-transport-order,
	AC_HELP_STRING([--with-local-transport-order=LIST], [preference sorted list of transport types to try for local connections]),
	[LOCAL_TRANSPORT_LIST=$withval],
	[LOCAL_TRANSPORT_LIST=$DEFAULT_LOCAL_TRANS])
AC_DEFINE_UNQUOTED([LOCAL_TRANSPORT_LIST], [$LOCAL_TRANSPORT_LIST],
   [preference sorted list of transport types to try for local connections])
AC_MSG_RESULT([$LOCAL_TRANSPORT_LIST])

# Check for dlopen
AC_MSG_CHECKING([if run-time linking is supported])
AC_SEARCH_LIBS(dlopen,[dl svld])
if test "x$ac_cv_search_dlopen" = xno; then
	AC_SEARCH_LIBS(shl_load,[dld])
	if test "x$ac_cv_search_shl_load" != xno; then
		AC_DEFINE(HAVE_SHL_LOAD,1,
			  [Use shl_load to load shared libraries])
		AC_CHECK_HEADERS([dl.h])
	fi
else
	AC_DEFINE(HAVE_DLOPEN,1,[Use dlopen to load shared libraries])
	AC_CHECK_HEADERS([dlfcn.h])
fi
if test x$ac_cv_header_dlcfn_h -o x$ac_cv_header_dl_h; then
	HAVE_LOADABLE_MODULES=yes
else
	HAVE_LOADABLE_MODULES=no
fi
AC_MSG_RESULT($HAVE_LOADABLE_MODULES)

AC_MSG_CHECKING([if loadable i18n module support should be enabled])
AC_ARG_ENABLE(loadable-i18n,
	      AC_HELP_STRING([--enable-loadable-i18n],
	       [Controls loadable i18n module support]),
	       [XLIB_LOADABLE_I18N=$enableval],
	       [XLIB_LOADABLE_I18N="no"])
if test x$XLIB_LOADABLE_I18N = xyes; then
	if test x$HAVE_LOADABLE_MODULES = xno; then
		AC_MSG_ERROR([Loadable module support is required to enable loadable i18n module support])
	fi
	AC_DEFINE(USE_DYNAMIC_LC,1,
		  [Split some i18n functions into loadable modules])
	AC_SUBST(I18N_MODULE_LIBS,'${top_builddir}/src/libX11.la')
fi
AC_MSG_RESULT($XLIB_LOADABLE_I18N)

AM_CONDITIONAL(XLIB_LOADABLE_I18N, test x$XLIB_LOADABLE_I18N = xyes)

AC_MSG_CHECKING([if loadable Xcursor library support should be enabled])
AC_ARG_ENABLE(loadable-xcursor,
	      AC_HELP_STRING([--disable-loadable-xcursor],
	       [Controls loadable xcursor library support]),
	       [XLIB_LOADABLE_XCURSOR=$enableval],
	       [XLIB_LOADABLE_XCURSOR=$HAVE_LOADABLE_MODULES])
if test x$XLIB_LOADABLE_XCURSOR = xyes; then
	AC_DEFINE(USE_DYNAMIC_XCURSOR,1,
		  [Use the X cursor library to load cursors])
fi
AC_MSG_RESULT($XLIB_LOADABLE_XCURSOR)

# Checks for header files.
AC_HEADER_STDC
dnl AC_CHECK_HEADERS([stdio.h stdlib.h math.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strtol])
# Used in lcFile.c (see also --enable-xlocaledir settings below)
XLOCALEDIR_IS_SAFE="no"
AC_CHECK_FUNC([issetugid], [XLOCALEDIR_IS_SAFE="yes"]
	AC_DEFINE(HASSETUGID,1,[Has issetugid() function]))
AC_CHECK_FUNC([getresuid], [XLOCALEDIR_IS_SAFE="yes"]
	AC_DEFINE(HASGETRESUID,1,[Has getresuid() & getresgid() functions]))
# Used in Font.c
AC_CHECK_FUNC([shmat], AC_DEFINE(HAS_SHM,1,[Has shm*() functions]))

# Checks for system services
dnl AC_PATH_XTRA

# arch specific things
WCHAR32="1"
case $target_alias in
  *os2*) os2="true" ; WCHAR32="0" ;;
  *) ;;
esac
AC_SUBST(WCHAR32)

AM_CONDITIONAL(OS2, test x$os2 = xtrue)

AC_ARG_WITH(launchd, AS_HELP_STRING([--with-launchd], [Build with support for Apple's launchd (default: auto)]), [LAUNCHD=$withval], [LAUNCHD=auto])
if test "x$LAUNCHD" = xauto; then
	unset LAUNCHD
	AC_CHECK_PROG(LAUNCHD, [launchd], [yes], [no])
fi

if test "x$LAUNCHD" = xyes ; then
	AC_DEFINE(HAVE_LAUNCHD, 1, [launchd support available])
	AC_DEFINE(TRANS_REOPEN, 1, [launchd support available])
fi

AC_ARG_ENABLE(xthreads,
              AC_HELP_STRING([--disable-xthreads],
                [Disable Xlib support for Multithreading]),
              [xthreads=$enableval],[xthreads=yes])

AC_CHECK_LIB(c, getpwuid_r, [mtsafeapi="yes"], [mtsafeapi="no"])

case x$xthreads in
xyes)
	AC_DEFINE(XTHREADS,1,[Whether libX11 is compiled with thread support])
	if test x$mtsafeapi = xyes
	then
	AC_DEFINE(XUSE_MTSAFE_API,1,[Whether libX11 needs to use MT safe API's])
	fi
	;;
*)
	;;
esac

AC_CHECK_LIB(c, pthread_self, [thrstubs="no"], [thrstubs="yes"])
AM_CONDITIONAL(THRSTUBS, test x$thrstubs = xyes)

dnl XXX incomplete, please fill this in
if test x$xthreads = xyes ; then
    case $host_os in
    linux*|openbsd*|gnu*|k*bsd*-gnu)
        XTHREADLIB=-lpthread ;;
    netbsd*)
	XTHREAD_CFLAGS="-D_POSIX_THREAD_SAFE_FUNCTIONS"
	XTHREADLIB="-lpthread" ;;
    freebsd*)
        XTHREAD_CFLAGS="-D_THREAD_SAFE"
        XTHREADLIB="-pthread" ;;
    dragonfly*)
        XTHREADLIB="-pthread" ;;
    solaris*)
	XTHREAD_CFLAGS="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS" ;;
    esac
fi
AC_SUBST(XTHREADLIB)
AC_SUBST(XTHREAD_CFLAGS)
X11_CFLAGS="$X11_CFLAGS $XTHREAD_CFLAGS"

case x$xdmauth in
xyes)
	XDMCP_CFLAGS="$XDMCP_CFLAGS -DHASXDMAUTH"
	;;
xno)
	XDMCP_LIBS=""
	;;
esac
AC_SUBST(XDMCP_CFLAGS)
AC_SUBST(XDMCP_LIBS)

AC_CHECK_FUNC(poll, [AC_DEFINE(USE_POLL, 1, [poll() function is available])], )

#
# Find keysymdef.h
#
AC_MSG_CHECKING([keysymdef.h])
dir=`pkg-config --variable=includedir xproto`
KEYSYMDEF="$dir/X11/keysymdef.h"
if test -f "$KEYSYMDEF"; then
        AC_MSG_RESULT([$KEYSYMDEF])
else
	AC_MSG_ERROR([Cannot find keysymdef.h])
fi
AC_SUBST(KEYSYMDEF)

AM_CONDITIONAL(UDC, test xfalse = xtrue)

AC_ARG_ENABLE(xcms,
              AC_HELP_STRING([--disable-xcms],
                [Disable Xlib support for CMS *EXPERIMENTAL*]),
              [XCMS=$enableval],[XCMS=yes])
AM_CONDITIONAL(XCMS, [test x$XCMS = xyes ])
if test x"$XCMS" = "xyes"; then
	AC_DEFINE(XCMS,1,[Include support for XCMS])
fi

AC_ARG_ENABLE(xlocale,
              AC_HELP_STRING([--disable-xlocale],
                [Disable Xlib locale implementation *EXPERIMENTAL*]),
              [XLOCALE=$enableval],[XLOCALE=yes])

AM_CONDITIONAL(XLOCALE, [ test x$XLOCALE = xyes ])
if test x"$XLOCALE" = "xyes"; then
	AC_DEFINE(XLOCALE,1,[support for X Locales])
fi

# This disables XLOCALEDIR.  Set it if you're using BuildLoadableXlibI18n,
# don't have either issetugid() or getresuid(), and you need to protect
# clients that are setgid or setuid to an id other than 0.
AC_MSG_CHECKING([if XLOCALEDIR support should be enabled])
AC_ARG_ENABLE(xlocaledir,
	      AC_HELP_STRING([--enable-xlocaledir],
		[Enable XLOCALEDIR environment variable support]),
	      [ENABLE_XLOCALEDIR=$enableval],[ENABLE_XLOCALEDIR=$XLOCALEDIR_IS_SAFE])
if test "x$ENABLE_XLOCALEDIR" = "xno"; then
	AC_DEFINE(NO_XLOCALEDIR,1,[Disable XLOCALEDIR environment variable])
fi
AC_MSG_RESULT($ENABLE_XLOCALEDIR)

AC_ARG_ENABLE(xf86bigfont,
	      AC_HELP_STRING([--disable-xf86bigfont],
		[Disable XF86BigFont extension support]),
	      [XF86BIGFONT=$enableval],[XF86BIGFONT="yes"])
if test "x$XF86BIGFONT" = "xyes"; then
    PKG_CHECK_MODULES(BIGFONT, xf86bigfontproto,
         AC_DEFINE(XF86BIGFONT,1,[Enable XF86BIGFONT extension]),XF86BIGFONT="no")
    AC_SUBST(BIGFONT_CFLAGS)
    AC_SUBST(BIGFONT_LIBS)
fi

AC_ARG_ENABLE(xkb,
              AC_HELP_STRING([--disable-xkb],
                [Disable XKB support *EXPERIMENTAL*]),
              [XKB=$enableval],[XKB=yes])

XORG_MANPAGE_SECTIONS

AC_ARG_ENABLE(man-pages,
	      AC_HELP_STRING([--enable-man-pages=section],
			     [Choose manual section for installing man pages]),
		[LIBMAN=$enableval],[LIBMAN=yes])

if test "x$LIBMAN" != "xyes"; then
	LIB_MAN_SUFFIX=$LIBMAN
fi

AM_CONDITIONAL(MANPAGES, [ test x$LIBMAN '!=' xno ])

AM_CONDITIONAL(XKB, [ test x$XKB = xyes ])
if test x"$XKB" = "xyes"; then
   XKBPROTO_REQUIRES="kbproto"
   PKG_CHECK_MODULES(XKBPROTO, kbproto inputproto)
   AC_DEFINE(XKB,1,[Use XKB])
   X11_CFLAGS="$X11_CFLAGS $XKBPROTO_CFLAGS"
   X11_LIBS="$X11_LIBS $XKBPROTO_LIBS"
else
   XKBPROTO_REQUIRES=""
fi
AC_SUBST(XKBPROTO_REQUIRES)

AC_FUNC_MMAP()
composecache_default=$ac_cv_func_mmap_fixed_mapped
AC_CHECK_FUNC(nl_langinfo, , [composecache_default=no])
AC_ARG_ENABLE(composecache,
              AC_HELP_STRING([--disable-composecache],
                [Disable compose table cache support]),
              [COMPOSECACHE=$enableval],[COMPOSECACHE=$composecache_default])
if test x"$COMPOSECACHE" = "xyes"; then
	AC_DEFINE(COMPOSECACHE,1,[Include compose table cache support])
fi

dnl Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
XORG_LINT_LIBRARY([X11])
LINT_FLAGS="${LINT_FLAGS} ${X11_CFLAGS} ${XPROTO_CFLAGS}"

if test "x$GCC" = "xyes"; then
	GCC_WARNINGS="-Wall -Wpointer-arith -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -fno-strict-aliasing"
	X11_CFLAGS="$GCC_WARNINGS $X11_CFLAGS"
fi

X11_DATADIR="${datadir}/X11"
AC_DEFINE_DIR(X11_DATADIR, X11_DATADIR, [Location of libX11 data])
AC_SUBST(X11_DATADIR)

X11_LIBDIR="${libdir}/X11"
AC_DEFINE_DIR(X11_LIBDIR, X11_LIBDIR, [Location of libX11 library data])
AC_SUBST(X11_LIBDIR)

X11_CFLAGS="$CWARNFLAGS $X11_CFLAGS $XPROTO_CFLAGS"
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

#
# Yes, it would be nice to put the locale data in
# /usr/share, but the locale stuff includes loadable
# libraries which must be located in the same directory
# as the other locale data, so for now, everything lives
# in ${libdir}
#

X11_LOCALEDATADIR="${X11_DATADIR}/locale"
AC_DEFINE_DIR(XLOCALEDATADIR, X11_LOCALEDATADIR, [Location of libX11 locale data])
AC_SUBST(X11_LOCALEDATADIR)

AC_ARG_WITH(locale-lib-dir,	AS_HELP_STRING([--with-locale-lib-dir=DIR],
 [Directory where locale libraries files are installed (default: $libdir/X11/locale)]),
				[ X11_LOCALELIBDIR="$withval" ],
				[ X11_LOCALELIBDIR="${X11_LIBDIR}/locale" ])
AC_DEFINE_DIR(XLOCALELIBDIR, X11_LOCALELIBDIR, [Location of libX11 locale libraries])
AC_SUBST(X11_LOCALELIBDIR)

X11_LOCALEDIR="${X11_LOCALEDATADIR}"
AC_DEFINE_DIR(XLOCALEDIR, X11_LOCALEDIR, [Location of libX11 locale data])
AC_SUBST(X11_LOCALEDIR)

XKEYSYMDB="${X11_DATADIR}/XKeysymDB"
AC_DEFINE_DIR(XKEYSYMDB, XKEYSYMDB, [Location of keysym database])

XERRORDB="${X11_DATADIR}/XErrorDB"
AC_DEFINE_DIR(XERRORDB, XERRORDB, [Location of error message database])


XORG_CHECK_MALLOC_ZERO
XORG_RELEASE_VERSION
XORG_CHANGELOG

AC_OUTPUT([Makefile
           include/Makefile
	   man/Makefile
           man/xkb/Makefile
           src/Makefile
           src/util/Makefile
	   src/xcms/Makefile
	   src/xlibi18n/Makefile
	   modules/Makefile
	   modules/im/Makefile
	   modules/im/ximcp/Makefile
	   modules/lc/Makefile
	   modules/lc/def/Makefile
	   modules/lc/gen/Makefile
	   modules/lc/Utf8/Makefile
	   modules/lc/xlocale/Makefile
	   modules/om/Makefile
	   modules/om/generic/Makefile
	   src/xkb/Makefile
	   nls/Makefile
	   nls/am_ET.UTF-8/Makefile
	   nls/armscii-8/Makefile
	   nls/C/Makefile
	   nls/el_GR.UTF-8/Makefile
	   nls/en_US.UTF-8/Makefile
	   nls/fi_FI.UTF-8/Makefile
	   nls/georgian-academy/Makefile
	   nls/georgian-ps/Makefile
	   nls/ibm-cp1133/Makefile
	   nls/iscii-dev/Makefile
	   nls/isiri-3342/Makefile
	   nls/iso8859-1/Makefile
	   nls/iso8859-10/Makefile
	   nls/iso8859-11/Makefile
	   nls/iso8859-13/Makefile
	   nls/iso8859-14/Makefile
	   nls/iso8859-15/Makefile
	   nls/iso8859-2/Makefile
	   nls/iso8859-3/Makefile
	   nls/iso8859-4/Makefile
	   nls/iso8859-5/Makefile
	   nls/iso8859-6/Makefile
	   nls/iso8859-7/Makefile
	   nls/iso8859-8/Makefile
	   nls/iso8859-9/Makefile
	   nls/iso8859-9e/Makefile
	   nls/ja/Makefile
	   nls/ja.JIS/Makefile
	   nls/ja_JP.UTF-8/Makefile
	   nls/ja.S90/Makefile
	   nls/ja.SJIS/Makefile
	   nls/ja.U90/Makefile
	   nls/ko/Makefile
	   nls/koi8-c/Makefile
	   nls/koi8-r/Makefile
	   nls/koi8-u/Makefile
	   nls/ko_KR.UTF-8/Makefile
	   nls/microsoft-cp1251/Makefile
	   nls/microsoft-cp1255/Makefile
	   nls/microsoft-cp1256/Makefile
	   nls/mulelao-1/Makefile
	   nls/nokhchi-1/Makefile
	   nls/pt_BR.UTF-8/Makefile
	   nls/ru_RU.UTF-8/Makefile
	   nls/tatar-cyr/Makefile
	   nls/th_TH/Makefile
	   nls/th_TH.UTF-8/Makefile
	   nls/tscii-0/Makefile
	   nls/vi_VN.tcvn/Makefile
	   nls/vi_VN.viscii/Makefile
	   nls/zh_CN/Makefile
	   nls/zh_CN.gb18030/Makefile
	   nls/zh_CN.gbk/Makefile
	   nls/zh_CN.UTF-8/Makefile
	   nls/zh_HK.big5/Makefile
	   nls/zh_HK.big5hkscs/Makefile
	   nls/zh_HK.UTF-8/Makefile
	   nls/zh_TW/Makefile
	   nls/zh_TW.big5/Makefile
	   nls/zh_TW.UTF-8/Makefile
	   x11.pc
	   x11-xcb.pc])

man_pages_suffix=$LIBMAN_SUFFIX
if test -z "$man_pages_suffix"; then
    man_pages_suffix=none
fi

echo ""
echo "X11 will be built with the following settings:"
echo " Loadable i18n module support:            "$XLIB_LOADABLE_I18N
echo " Loadable xcursor library support:        "$XLIB_LOADABLE_XCURSOR
echo " Use XCB:                                 "$ac_cv_use_xcb
echo " Threading support:                       "$xthreads
echo " Use Threads safe API:                    "$mtsafeapi
echo " Threads stubs in libX11:                 "$thrstubs
echo " XCMS:                                    "$XCMS
echo " Internationalization support:            "$XLOCALE
echo " XF86BigFont support:                     "$XF86BIGFONT
echo " XKB support:                             "$XKB
echo " XLOCALEDIR environment variable support: "$ENABLE_XLOCALEDIR
echo " Manual pages suffix:                     "$man_pages_suffix
echo " Compose table cache enabled:             "$COMPOSECACHE
echo ""
