#  Copyright 2005 Red Hat, Inc.
#
#  Permission to use, copy, modify, distribute, and sell this software
#  and its documentation for any purpose is hereby granted without
#  fee, provided that the above copyright notice appear in all copies
#  and that both that copyright notice and this permission notice
#  appear in supporting documentation, and that the name of Red Hat
#  not be used in advertising or publicity pertaining to distribution
#  of the software without specific, written prior permission.  Red
#  Hat makes no representations about the suitability of this software
#  for any purpose.  It is provided "as is" without express or implied
#  warranty.
#
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
#  NO EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
#  OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
#  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
#  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

bin_PROGRAMS = ucs2any
bin_SCRIPTS = bdftruncate

ucs2any_SOURCES = ucs2any.c

bdftruncate: $(top_srcdir)/bdftruncate.pl
	@rm -f $@
	cp $(top_srcdir)/bdftruncate.pl $@

aclocaldir = $(datadir)/aclocal
aclocal_DATA = fontutil.m4

mapfilesdir = @MAPDIR@
mapfiles_DATA = \
	map-ISO8859-1 \
	map-ISO8859-2 \
	map-ISO8859-3 \
	map-ISO8859-4 \
	map-ISO8859-5 \
	map-ISO8859-6 \
	map-ISO8859-7 \
	map-ISO8859-8 \
	map-ISO8859-9 \
	map-ISO8859-10 \
	map-ISO8859-11 \
	map-ISO8859-13 \
	map-ISO8859-14 \
	map-ISO8859-15 \
	map-ISO8859-16 \
	map-JISX0201.1976-0 \
	map-KOI8-R

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fontutil.pc

appmandir = $(APP_MAN_DIR)
appman_PRE = bdftruncate.man ucs2any.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

CLEANFILES = bdftruncate $(appman_DATA)

EXTRA_DIST = $(mapfiles_DATA) \
	$(appman_PRE) \
	autogen.sh \
	bdftruncate.pl \
	fontutil.pc.in \
	$(aclocal_DATA)

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

SED = sed

MAN_SUBSTS = \
        -e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
        -e 's|__appmansuffix__|$(APP_MAN_SUFFIX)|g'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	sed $(MAN_SUBSTS) < $< > $@
