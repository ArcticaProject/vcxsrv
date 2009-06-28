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
	proof9x16 \
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
	koi9x18 \
	screen8x16b \
	screen8x16 \
	crox1cb \
	crox1c \
	crox1cbo \
	crox1co \
	crox1hb \
	crox1h \
	crox1hbo \
	crox1ho \
	crox1tb \
	crox1t \
	crox1tbo \
	crox1to \
	crox2cb \
	crox2c \
	crox2cbo \
	crox2co \
	crox2hb \
	crox2h \
	crox2hbo \
	crox2ho \
	crox2tb \
	crox2t \
	crox2tbo \
	crox2to \
	crox3cb \
	crox3c \
	crox3cbo \
	crox3co \
	crox3hb \
	crox3h \
	crox3hbo \
	crox3ho \
	crox3tb \
	crox3t \
	crox3tbo \
	crox3to \
	crox4hb \
	crox4h \
	crox4hbo \
	crox4ho \
	crox4tb \
	crox4t \
	crox4tbo \
	crox4to \
	crox5hb \
	crox5h \
	crox5hbo \
	crox5ho \
	crox5tb \
	crox5t \
	crox5tbo \
	crox5to \
	crox6hb \
	crox6h \
	crox6hbo \
	crox6ho \
	crox6tb \
	crox6t \
	crox6tbo \
	crox6to \
	koi10x16b \
	koi10x20 \
	koi6x10 \
	koinil2

BDF_FILES = $(FONT_FILES:%=%.bdf)
PCF_FILES = $(FONT_FILES:%=%.pcf.gz)

SUFFIXES = .bdf .pcf.gz

.bdf.pcf.gz:
	@rm -f $@
	$(BDFTOPCF) -t $< | gzip > $@

fontdir = @FONTDIR@
font_DATA = $(PCF_FILES)
CLEANFILES = $(font_DATA)

EXTRA_DIST = $(BDF_FILES) COPYRIGHT autogen.sh

install-data-hook:
	@rm -f $(DESTDIR)$(fontdir)/fonts.scale
	$(MKFONTSCALE) $(DESTDIR)$(fontdir)
	@rm -f $(DESTDIR)$(fontdir)/font.dir
	$(MKFONTDIR) $(DESTDIR)$(fontdir)

distuninstallcheck:
	@:
