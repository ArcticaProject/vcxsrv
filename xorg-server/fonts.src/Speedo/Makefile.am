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

FONT_FILES = \
	font0419.spd \
	font0582.spd \
	font0583.spd \
	font0611.spd \
	font0648.spd \
	font0649.spd \
	font0709.spd \
	font0710.spd \
	fonts.scale

fontdir = @FONTDIR@
font_DATA = $(FONT_FILES)

EXTRA_DIST = $(FONT_FILES) COPYRIGHT autogen.sh

# Note that the fonts.scale is not generated; rather, it is copied above
install-data-hook:
	@rm -f $(DESTDIR)$(fontdir)/font.dir
	$(MKFONTDIR) $(DESTDIR)$(fontdir)

distuninstallcheck:
	@:
