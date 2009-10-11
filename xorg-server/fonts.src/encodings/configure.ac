AC_PREREQ([2.57])
AC_INIT([encodings], [1.0.3], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)
XORG_DEFAULT_OPTIONS

AC_PROG_INSTALL

# Require X.Org's font util macros 1.1 or later
m4_ifndef([XORG_FONT_MACROS_VERSION],
	  [m4_fatal([must install X.Org font-util 1.1 or later before running autoconf/autogen])])
XORG_FONT_MACROS_VERSION(1.1)

# mkfontscale
XORG_FONT_REQUIRED_PROG(MKFONTSCALE, mkfontscale)

# Choose whether or not to compress encodings - default is to compress all

AC_MSG_CHECKING([if small encodings should be compressed])
AC_ARG_ENABLE(gzip-small-encodings,
	AC_HELP_STRING([--disable-gzip-small-encodings],
			[Disable compression of small encoding files]),
	[GZIP_SMALL="$enableval"], [GZIP_SMALL="yes"]) 
AM_CONDITIONAL(GZIP_SMALL, [test x$GZIP_SMALL = xyes])
AC_MSG_RESULT([${GZIP_SMALL}])

AC_MSG_CHECKING([if large encodings should be compressed])
AC_ARG_ENABLE(gzip-large-encodings,
	AC_HELP_STRING([--disable-gzip-large-encodings],
			[Disable compression of large encoding files]),
	[GZIP_LARGE="$enableval"], [GZIP_LARGE="yes"]) 
AM_CONDITIONAL(GZIP_LARGE, [test x$GZIP_LARGE = xyes])
AC_MSG_RESULT([${GZIP_LARGE}])

# gzip

if test x$GZIP_SMALL = xyes -o x$GZIP_LARGE = xyes ; then
    AC_PATH_PROG(GZIP, gzip)
    if [[ -z $GZIP ]]; then
	AC_MSG_ERROR([gzip is needed to compress font encodings])
    fi
fi

XORG_FONTSUBDIR([ENCODINGSDIR], [encodingsdir], [encodings])

AC_OUTPUT([Makefile large/Makefile])
