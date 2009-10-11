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
	koi12x24b \
	koi12x24 \
	koi5x8 \
	koi6x13b \
	koi6x13 \
	koi6x9 \
	koi7x14b \
	koi7x14 \
	koi8x13 \
	koi8x16b \
	koi8x16 \
	koi9x15b \
	koi9x15 \
	koi9x18b \
	koi9x18

BDF_FILES = $(FONT_FILES:%=%.bdf)
PCF_FILES = $(FONT_FILES:%=%.pcf$(COMPRESS_SUFFIX))

SUFFIXES = .bdf .pcf$(COMPRESS_SUFFIX)

.bdf.pcf$(COMPRESS_SUFFIX):
	@rm -f $@
	$(AM_V_GEN)$(BDFTOPCF) -t $< | $(COMPRESS) > $@

fontdir = @FONTDIR@
font_DATA = $(PCF_FILES)
CLEANFILES = $(font_DATA)

EXTRA_DIST = $(BDF_FILES)

install-data-hook:
	@rm -f $(DESTDIR)$(fontdir)/font.dir
	$(MKFONTDIR) $(DESTDIR)$(fontdir)
	@RUN_FCCACHE@

distuninstallcheck:
	@:

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
