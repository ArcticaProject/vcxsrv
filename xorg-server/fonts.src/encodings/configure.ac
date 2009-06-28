AC_PREREQ([2.57])
AC_INIT([encodings], [1.0.2], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

m4_ifdef([AS_HELP_STRING], , [m4_define([AS_HELP_STRING], m4_defn([AC_HELP_STRING]))])

# mkfontscale

AC_PATH_PROG(MKFONTSCALE, mkfontscale)
if [[ -z $MKFONTSCALE ]]; then
   AC_MSG_ERROR([mkfontscale is needed to build fontenc. Please install the app/mkfontscale package from X.Org.])
fi

# Choose whether or not to compress encodings - default is to compress all

AC_ARG_ENABLE(gzip-small-encodings,
	AS_HELP_STRING([--disable-gzip-small-encodings],
			[Disable compression of small encoding files]),
	[GZIP_SMALL="$enableval"], [GZIP_SMALL="yes"]) 
AM_CONDITIONAL(GZIP_SMALL, [test x$GZIP_SMALL = xyes])

AC_ARG_ENABLE(gzip-large-encodings,
	AS_HELP_STRING([--disable-gzip-large-encodings],
			[Disable compression of large encoding files]),
	[GZIP_LARGE="$enableval"], [GZIP_LARGE="yes"]) 
AM_CONDITIONAL(GZIP_LARGE, [test x$GZIP_LARGE = xyes])

# gzip

if test x$GZIP_SMALL = xyes -o x$GZIP_LARGE = xyes ; then
    AC_PATH_PROG(GZIP, gzip)
    if [[ -z $GZIP ]]; then
	AC_MSG_ERROR([gzip is needed to compress font encodings])
    fi
fi

DEFAULT_ENCODINGSDIR=${libdir}/X11/fonts/encodings
AC_ARG_WITH(encodingsdir,
	AS_HELP_STRING([--with-encodingsdir=ENCODINGSDIR], [Path to install font encodings]),
	[ENCODINGSDIR="$withval"],
	[ENCODINGSDIR="$DEFAULT_ENCODINGSDIR"])
AC_SUBST(ENCODINGSDIR)

XORG_RELEASE_VERSION

AC_OUTPUT([Makefile large/Makefile])
