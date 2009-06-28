dnl Copyright 2005 Red Hat, Inc
dnl
dnl Permission to use, copy, modify, distribute, and sell this software and its
dnl documentation for any purpose is hereby granted without fee, provided that
dnl the above copyright notice appear in all copies and that both that
dnl copyright notice and this permission notice appear in supporting
dnl documentation.
dnl
dnl The above copyright notice and this permission notice shall be included
dnl in all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
dnl OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
dnl ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
dnl OTHER DEALINGS IN THE SOFTWARE.
dnl
dnl Except as contained in this notice, the name of the copyright holders shall
dnl not be used in advertising or otherwise to promote the sale, use or
dnl other dealings in this Software without prior written authorization
dnl from the copyright holders.
dnl

# XORG_FONT_CHECK_{maps}()
# ------------------------
# These macros add --enable/disable-{maps} where {maps} are ISO8859-*,
# JISX0201 or KOI8_R.  By default, they are all enabled.

AC_DEFUN([XORG_FONT_CHECK_ISO8859_1],[
	AC_ARG_ENABLE(iso8859-1,
		AS_HELP_STRING([--disable-iso8859-1],
				[Build ISO8859-1 fonts (default: yes)]),
		[ISO8859_1=$enableval],
		[ISO8859_1=yes])
	AC_MSG_CHECKING([whether to build ISO8859-1 fonts])
	AC_MSG_RESULT([$ISO8859_1])
	AM_CONDITIONAL(ISO8859_1, [test "x$ISO8859_1" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_2],[
	AC_ARG_ENABLE(iso8859-2,
		AS_HELP_STRING([--disable-iso8859-2],
				[Build ISO8859-2 fonts (default: yes)]),
		[ISO8859_2=$enableval],
		[ISO8859_2=yes])
	AC_MSG_CHECKING([whether to build ISO8859-2 fonts])
	AC_MSG_RESULT([$ISO8859_2])
	AM_CONDITIONAL(ISO8859_2, [test "x$ISO8859_2" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_3],[
	AC_ARG_ENABLE(iso8859-3,
		AS_HELP_STRING([--disable-iso8859-3],
				[Build ISO8859-3 fonts (default: yes)]),
		[ISO8859_3=$enableval],
		[ISO8859_3=yes])
	AC_MSG_CHECKING([whether to build ISO8859-3 fonts])
	AC_MSG_RESULT([$ISO8859_3])
	AM_CONDITIONAL(ISO8859_3, [test "x$ISO8859_3" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_4],[
	AC_ARG_ENABLE(iso8859-4,
		AS_HELP_STRING([--disable-iso8859-4],
				[Build ISO8859-4 fonts (default: yes)]),
		[ISO8859_4=$enableval],
		[ISO8859_4=yes])
	AC_MSG_CHECKING([whether to build ISO8859-4 fonts])
	AC_MSG_RESULT([$ISO8859_4])
	AM_CONDITIONAL(ISO8859_4, [test "x$ISO8859_4" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_5],[
	AC_ARG_ENABLE(iso8859-5,
		AS_HELP_STRING([--disable-iso8859-5],
				[Build ISO8859-5 fonts (default: yes)]),
		[ISO8859_5=$enableval],
		[ISO8859_5=yes])
	AC_MSG_CHECKING([whether to build ISO8859-5 fonts])
	AC_MSG_RESULT([$ISO8859_5])
	AM_CONDITIONAL(ISO8859_5, [test "x$ISO8859_5" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_6],[
	AC_ARG_ENABLE(iso8859-6,
		AS_HELP_STRING([--disable-iso8859-6],
				[Build ISO8859-6 fonts (default: yes)]),
		[ISO8859_6=$enableval],
		[ISO8859_6=yes])
	AC_MSG_CHECKING([whether to build ISO8859-6 fonts])
	AC_MSG_RESULT([$ISO8859_6])
	AM_CONDITIONAL(ISO8859_6, [test "x$ISO8859_6" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_7],[
	AC_ARG_ENABLE(iso8859-7,
		AS_HELP_STRING([--disable-iso8859-7],
				[Build ISO8859-7 fonts (default: yes)]),
		[ISO8859_7=$enableval],
		[ISO8859_7=yes])
	AC_MSG_CHECKING([whether to build ISO8859-7 fonts])
	AC_MSG_RESULT([$ISO8859_7])
	AM_CONDITIONAL(ISO8859_7, [test "x$ISO8859_7" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_8],[
	AC_ARG_ENABLE(iso8859-8,
		AS_HELP_STRING([--disable-iso8859-8],
				[Build ISO8859-8 fonts (default: yes)]),
		[ISO8859_8=$enableval],
		[ISO8859_8=yes])
	AC_MSG_CHECKING([whether to build ISO8859-8 fonts])
	AC_MSG_RESULT([$ISO8859_8])
	AM_CONDITIONAL(ISO8859_8, [test "x$ISO8859_8" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_9],[
	AC_ARG_ENABLE(iso8859-9,
		AS_HELP_STRING([--disable-iso8859-9],
				[Build ISO8859-9 fonts (default: yes)]),
		[ISO8859_9=$enableval],
		[ISO8859_9=yes])
	AC_MSG_CHECKING([whether to build ISO8859-9 fonts])
	AC_MSG_RESULT([$ISO8859_9])
	AM_CONDITIONAL(ISO8859_9, [test "x$ISO8859_9" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_10],[
	AC_ARG_ENABLE(iso8859-10,
		AS_HELP_STRING([--disable-iso8859-10],
				[Build ISO8859-10 fonts (default: yes)]),
		[ISO8859_10=$enableval],
		[ISO8859_10=yes])
	AC_MSG_CHECKING([whether to build ISO8859-10 fonts])
	AC_MSG_RESULT([$ISO8859_10])
	AM_CONDITIONAL(ISO8859_10, [test "x$ISO8859_10" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_11],[
	AC_ARG_ENABLE(iso8859-11,
		AS_HELP_STRING([--disable-iso8859-11],
				[Build ISO8859-11 fonts (default: yes)]),
		[ISO8859_11=$enableval],
		[ISO8859_11=yes])
	AC_MSG_CHECKING([whether to build ISO8859-11 fonts])
	AC_MSG_RESULT([$ISO8859_11])
	AM_CONDITIONAL(ISO8859_11, [test "x$ISO8859_11" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_12],[
	AC_ARG_ENABLE(iso8859-12,
		AS_HELP_STRING([--disable-iso8859-12],
				[Build ISO8859-12 fonts (default: yes)]),
		[ISO8859_12=$enableval],
		[ISO8859_12=yes])
	AC_MSG_CHECKING([whether to build ISO8859-12 fonts])
	AC_MSG_RESULT([$ISO8859_12])
	AM_CONDITIONAL(ISO8859_12, [test "x$ISO8859_12" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_13],[
	AC_ARG_ENABLE(iso8859-13,
		AS_HELP_STRING([--disable-iso8859-13],
				[Build ISO8859-13 fonts (default: yes)]),
		[ISO8859_13=$enableval],
		[ISO8859_13=yes])
	AC_MSG_CHECKING([whether to build ISO8859-13 fonts])
	AC_MSG_RESULT([$ISO8859_13])
	AM_CONDITIONAL(ISO8859_13, [test "x$ISO8859_13" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_14],[
	AC_ARG_ENABLE(iso8859-14,
		AS_HELP_STRING([--disable-iso8859-14],
				[Build ISO8859-14 fonts (default: yes)]),
		[ISO8859_14=$enableval],
		[ISO8859_14=yes])
	AC_MSG_CHECKING([whether to build ISO8859-14 fonts])
	AC_MSG_RESULT([$ISO8859_14])
	AM_CONDITIONAL(ISO8859_14, [test "x$ISO8859_14" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_15],[
	AC_ARG_ENABLE(iso8859-15,
		AS_HELP_STRING([--disable-iso8859-15],
				[Build ISO8859-15 fonts (default: yes)]),
		[ISO8859_15=$enableval],
		[ISO8859_15=yes])
	AC_MSG_CHECKING([whether to build ISO8859-15 fonts])
	AC_MSG_RESULT([$ISO8859_15])
	AM_CONDITIONAL(ISO8859_15, [test "x$ISO8859_15" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_ISO8859_16],[
	AC_ARG_ENABLE(iso8859-16,
		AS_HELP_STRING([--disable-iso8859-16],
				[Build ISO8859-16 fonts (default: yes)]),
		[ISO8859_16=$enableval],
		[ISO8859_16=yes])
	AC_MSG_CHECKING([whether to build ISO8859-16 fonts])
	AC_MSG_RESULT([$ISO8859_16])
	AM_CONDITIONAL(ISO8859_16, [test "x$ISO8859_16" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_JISX0201],[
	AC_ARG_ENABLE(jisx0201,
		AS_HELP_STRING([--disable-jisx0201],
				[Build JISX0201 fonts (default: yes)]),
		[JISX0201=$enableval],
		[JISX0201=yes])
	AC_MSG_CHECKING([whether to build JISX0201 fonts])
	AC_MSG_RESULT([$JISX0201])
	AM_CONDITIONAL(JISX0201, [test "x$JISX0201" = xyes])
])

AC_DEFUN([XORG_FONT_CHECK_KOI8_R],[
	AC_ARG_ENABLE(koi8-r,
		AS_HELP_STRING([--disable-koi8-r],
				[Build KOI8-R fonts (default: yes)]),
		[KOI8_R=$enableval],
		[KOI8_R=yes])
	AC_MSG_CHECKING([whether to build KOI8-R fonts])
	AC_MSG_RESULT([$KOI8_R])
	AM_CONDITIONAL(KOI8_R, [test "x$KOI8_R" = xyes])
])
