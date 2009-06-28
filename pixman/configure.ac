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

#   Pixman versioning scheme
#
#   - The version in git has an odd MICRO version number
#
#   - Released versions both development and stable have an even MICRO 
#     version number
#
#   - Released development versions have an odd MINOR number
#
#   - Released stable versions have an event MINOR number
#
#   - Versions that break ABI must have a new MAJOR number
#
#   - If you break the ABI, then at least this must be done:
#
#        - increment MAJOR
#
#        - In the first development release where you break ABI, find
#          all instances of "pixman-n" and change them to pixman-(n+1)
#
#          This needs to be done at least in 
#                    configure.ac
#                    all Makefile.am's
#                    pixman-n.pc.in
#
#      This ensures that binary incompatible versions can be installed
#      in parallel.  See http://www106.pair.com/rhp/parallel.html for
#      more information
#

m4_define([pixman_major], 0)
m4_define([pixman_minor], 12)
m4_define([pixman_micro], 0)

m4_define([pixman_version],[pixman_major.pixman_minor.pixman_micro])

AC_INIT(pixman, pixman_version, "sandmann@daimi.au.dk", pixman)
AM_INIT_AUTOMAKE([dist-bzip2])

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_LIBTOOL
AC_CHECK_FUNCS([getisax])
AC_C_BIGENDIAN
AC_C_INLINE
# 
# We ignore pixman_major in the version here because the major version should
# always be encoded in the actual library name. Ie., the soname is:
#
#      pixman-$(pixman_major).0.minor.micro
#
m4_define([lt_current], [pixman_minor])
m4_define([lt_revision], [pixman_micro])
m4_define([lt_age], [pixman_minor])

LT_VERSION_INFO="lt_current:lt_revision:lt_age"

PIXMAN_VERSION_MAJOR=pixman_major()
AC_SUBST(PIXMAN_VERSION_MAJOR)
PIXMAN_VERSION_MINOR=pixman_minor()
AC_SUBST(PIXMAN_VERSION_MINOR)
PIXMAN_VERSION_MICRO=pixman_micro()
AC_SUBST(PIXMAN_VERSION_MICRO)

AC_SUBST(LT_VERSION_INFO)

# Check for dependencies
#PKG_CHECK_MODULES(DEP, x11)

changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac fi changequote([,])dnl

AC_PATH_PROG(PERL, perl, no)
if test "x$PERL" = xno; then
    AC_MSG_ERROR([Perl is required to build pixman.])
fi
AC_SUBST(PERL)

dnl =========================================================================
dnl -fvisibility stuff

have_gcc4=no
AC_MSG_CHECKING(for -fvisibility)
AC_COMPILE_IFELSE([
#if defined(__GNUC__) && (__GNUC__ >= 4)
#else
error Need GCC 4.0 for visibility
#endif
int main () { return 0; } 
], have_gcc4=yes)

if test "x$have_gcc4" = "xyes"; then
   CFLAGS="$CFLAGS -fvisibility=hidden"
fi
AC_MSG_RESULT($have_gcc4)

have_sunstudio8=no
AC_MSG_CHECKING([for -xldscope (Sun compilers)])
AC_COMPILE_IFELSE([
#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#else
error Need Sun Studio 8 for visibility
#endif
int main () { return 0; } 
], have_sunstudio8=yes)

if test "x$have_sunstudio8" = "xyes"; then
   CFLAGS="$CFLAGS -xldscope=hidden"
fi
AC_MSG_RESULT($have_sunstudio8)

dnl ===========================================================================
dnl Check for MMX

MMX_CFLAGS="-mmmx -Winline"

have_mmx_intrinsics=no
AC_MSG_CHECKING(whether to use MMX intrinsics)
xserver_save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS $MMX_CFLAGS"
AC_COMPILE_IFELSE([
#if defined(__GNUC__) && (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
error "Need GCC >= 3.4 for MMX intrinsics"
#endif
#include <mmintrin.h>
int main () {
    __m64 v = _mm_cvtsi32_si64 (1);
    return _mm_cvtsi64_si32 (v);
}], have_mmx_intrinsics=yes)
CFLAGS=$xserver_save_CFLAGS

AC_ARG_ENABLE(mmx,
   [AC_HELP_STRING([--disable-mmx],
                   [disable MMX fast paths])],
   [enable_mmx=$enableval], [enable_mmx=auto])

if test $enable_mmx = no ; then
   have_mmx_intrinsics=disabled
fi

if test $have_mmx_intrinsics = yes ; then
   AC_DEFINE(USE_MMX, 1, [use MMX compiler intrinsics])
else
   MMX_CFLAGS=
fi

AC_MSG_RESULT($have_mmx_intrinsics)
if test $enable_mmx = yes && test $have_mmx_intrinsics = no ; then
   AC_MSG_ERROR([MMX intrinsics not detected])
fi

AM_CONDITIONAL(USE_MMX, test $have_mmx_intrinsics = yes)

dnl ===========================================================================
dnl Check for SSE2

SSE2_CFLAGS="-mmmx -msse2 -Winline"

have_sse2_intrinsics=no
AC_MSG_CHECKING(whether to use SSE2 intrinsics)
xserver_save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -msse2 $SSE2_CFLAGS"

AC_COMPILE_IFELSE([
#if defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2))
#   if !defined(__amd64__) && !defined(__x86_64__)
#      error "Need GCC >= 4.2 for SSE2 intrinsics on x86"
#   endif
#endif
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
int main () {
    __m128i a, b, c;
	c = _mm_xor_si128 (a, b);
    return 0;
}], have_sse2_intrinsics=yes)
CFLAGS=$xserver_save_CFLAGS

AC_ARG_ENABLE(sse2,
   [AC_HELP_STRING([--disable-sse2],
                   [disable SSE2 fast paths])],
   [enable_sse2=$enableval], [enable_sse2=auto])

if test $enable_sse2 = no ; then
   have_sse2_intrinsics=disabled
fi

if test $have_sse2_intrinsics = yes ; then
   AC_DEFINE(USE_SSE2, 1, [use SSE2 compiler intrinsics])
fi

AC_MSG_RESULT($have_sse2_intrinsics)
if test $enable_sse2 = yes && test $have_sse2_intrinsics = no ; then
   AC_MSG_ERROR([SSE2 intrinsics not detected])
fi

AM_CONDITIONAL(USE_SSE2, test $have_sse2_intrinsics = yes)

dnl ========================================================
AC_SUBST(MMX_CFLAGS)
AC_SUBST(SSE2_CFLAGS)

dnl Check for VMX/Altivec
if test -n "`$CC -v 2>&1 | grep version | grep Apple`"; then
    VMX_CFLAGS="-faltivec"
else
    VMX_CFLAGS="-maltivec -mabi=altivec"
fi

have_vmx_intrinsics=no
AC_MSG_CHECKING(whether to use VMX/Altivec intrinsics)
xserver_save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS $VMX_CFLAGS"
AC_COMPILE_IFELSE([
#if defined(__GNUC__) && (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
error "Need GCC >= 3.4 for sane altivec support"
#endif
#include <altivec.h>
int main () {
    vector unsigned int v = vec_splat_u32 (1);
    v = vec_sub (v, v);
    return 0;
}], have_vmx_intrinsics=yes)
CFLAGS=$xserver_save_CFLAGS

AC_ARG_ENABLE(vmx,
   [AC_HELP_STRING([--disable-vmx],
                   [disable VMX fast paths])],
   [enable_vmx=$enableval], [enable_vmx=auto])

if test $enable_vmx = no ; then
   have_vmx_intrinsics=disabled
fi

if test $have_vmx_intrinsics = yes ; then
   AC_DEFINE(USE_VMX, 1, [use VMX compiler intrinsics])
else
   VMX_CFLAGS=
fi

AC_MSG_RESULT($have_vmx_intrinsics)
if test $enable_vmx = yes && test $have_vmx_intrinsics = no ; then
   AC_MSG_ERROR([VMX intrinsics not detected])
fi

AC_SUBST(VMX_CFLAGS)

AM_CONDITIONAL(USE_VMX, test $have_vmx_intrinsics = yes)

AC_ARG_ENABLE(gtk,
   [AC_HELP_STRING([--enable-gtk],
                   [enable tests using GTK+ [default=auto]])],
   [enable_gtk=$enableval], [enable_gtk=auto])

PKG_PROG_PKG_CONFIG
if test $enable_gtk = auto ; then
   PKG_CHECK_EXISTS([gtk+-2.0], [enable_gtk=yes], [enable_gtk=no])
fi
if test $enable_gtk = yes ; then
   PKG_CHECK_MODULES(GTK, [gtk+-2.0])
fi

AM_CONDITIONAL(HAVE_GTK, [test "x$enable_gtk" = xyes])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(DEP_CFLAGS)
AC_SUBST(DEP_LIBS)
		  
AC_OUTPUT([pixman-1.pc
           pixman-1-uninstalled.pc
           Makefile
	   pixman/Makefile
	   pixman/pixman-version.h
	   test/Makefile])
