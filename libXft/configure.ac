#
#  Copyright © 2003 Keith Packard, Noah Levitt
#
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Keith Packard not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Keith Packard makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
#
#  KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.
#

# Initialize Autoconf
AC_PREREQ([2.60])
#
# This is the package version number, not the shared library
# version.  This version number will be substituted into Xft.h
# Please bump the minor library number at each release as well.
#
AC_INIT([libXft], [2.2.0],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [libXft])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Initialize libtool
AC_PROG_LIBTOOL

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Set library version for Xft.h from package version set in AC_INIT
# copied from PACKAGE_VERSION_* settings in XORG_VERSION
AC_CONFIG_HEADERS([include/X11/Xft/Xft.h])
AC_DEFINE_UNQUOTED([XFT_MAJOR],
                [`echo $PACKAGE_VERSION | cut -d . -f 1`],
                [Major version of Xft])
AC_DEFINE_UNQUOTED([XFT_MINOR],
                [`echo $PACKAGE_VERSION | cut -d . -f 2 | cut -d - -f 1`],
                [Minor version of Xft])
AC_DEFINE_UNQUOTED([XFT_REVISION],
                [`echo $PACKAGE_VERSION | cut -d . -f 3 | cut -d - -f 1`],
                [Micro revision of Xft])

XFT_LT_VERSION=`echo $PACKAGE_VERSION | tr '.' ':'`
AC_SUBST([XFT_LT_VERSION])

#
# Check for Xrender
#
PKG_CHECK_MODULES(XRENDER, xrender >= 0.8.2, [xrender_found_with_pkgconfig=yes],
                  [xrender_found_with_pkgconfig=no])
case "$xrender_found_with_pkgconfig" in
no)
	PKG_CHECK_MODULES(XRENDER, xrender >= 0, [old_xrender_found_with_pkgconfig=yes],
			  [old_xrender_found_with_pkgconfig=no])
	case "$old_xrender_found_with_pkgconfig" in
	yes)
		XRENDER_LIBS="$XRENDER_LIBS -lXext -lX11"
		;;
	*)
	        # checks for X
	        AC_PATH_X

	        XRENDER_CFLAGS="-I$x_includes"
	        XRENDER_LIBS="-L$x_libraries -lXrender -lXext -lX11"

	        saved_CPPFLAGS="$CPPFLAGS"
	        CPPFLAGS="$CPPFLAGS $XRENDER_CFLAGS"
	        AC_CHECK_HEADERS([X11/extensions/Xrender.h], [], [AC_MSG_ERROR([Xrender.h not found.])])
	        CPPFLAGS="$saved_CPPFLAGS"

	        saved_LIBS="$LIBS"
	        LIBS="$LIBS $XRENDER_LIBS"
	        AC_CHECK_FUNCS([XRenderFindStandardFormat], [], [AC_MSG_ERROR([libXrender not found or too old.])])
	        LIBS="$saved_LIBS"
		;;
	esac
        ;;
esac

# Check freetype configuration
AC_ARG_WITH(freetype-config, [  --with-freetype-config=PROG   Use FreeType configuration program PROG], freetype_config=$withval, freetype_config=auto)

if test "$freetype_config" = "auto"; then
	PKG_CHECK_MODULES(FREETYPE, freetype2,
		freetype_config=no, freetype_config=yes)
fi

if test "$freetype_config" = "yes"; then
	AC_PATH_PROG(ft_config,freetype-config,no)
	if test "$ft_config" = "no"; then
		AC_MSG_ERROR([You must have freetype installed; see http://www.freetype.org/])
	fi
else
	ft_config="$freetype_config"
fi

if test "$freetype_config" != "no"; then
	FREETYPE_CFLAGS="`$ft_config --cflags`"
	FREETYPE_LIBS="`$ft_config --libs`"
fi

# Check fontconfig configuration
PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.2)

AC_SUBST(XRENDER_CFLAGS)
AC_SUBST(XRENDER_LIBS)
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

fontconfig_save_libs="$LIBS"
fontconfig_save_cflags="$CFLAGS"
LIBS="$LIBS $FREETYPE_LIBS"
CFLAGS="$CFLAGS $FREETYPE_CFLAGS"
AC_CHECK_FUNCS(FT_Get_Next_Char FT_Get_BDF_Property FT_Get_PS_Font_Info FT_Has_PS_Glyph_Names FT_GlyphSlot_Embolden)
AC_CHECK_MEMBER(FT_Bitmap_Size.y_ppem,
		HAVE_FT_BITMAP_SIZE_Y_PPEM=1,
		HAVE_FT_BITMAP_SIZE_Y_PPEM=0,
[#include <ft2build.h>
#include FT_FREETYPE_H])
AC_DEFINE_UNQUOTED(HAVE_FT_BITMAP_SIZE_Y_PPEM,$HAVE_FT_BITMAP_SIZE_Y_PPEM,
		   [FT_Bitmap_Size structure includes y_ppem field])

LIBS="$fontconfig_save_libs"
CFLAGS="$fontconfig_save_cflags"

if test "$VERSION" = "" ; then
       VERSION=$PACKAGE_VERSION;
fi

AC_CONFIG_FILES([Makefile
		xft.pc
		src/Makefile
		man/Makefile])
AC_OUTPUT
