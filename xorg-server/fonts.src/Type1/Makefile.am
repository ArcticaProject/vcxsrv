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
	cursor.pfa \
	UTB_____.afm \
	UTBI____.afm \
	UTBI____.pfa \
	UTB_____.pfa \
	UTI_____.afm \
	UTI_____.pfa \
	UTRG____.afm \
	UTRG____.pfa \
	cour.afm \
	courb.afm \
	courbi.afm \
	courbi.pfa \
	courb.pfa \
	couri.afm \
	couri.pfa \
	cour.pfa \
	l047013t.afm \
	l047013t.pfa \
	l047016t.afm \
	l047016t.pfa \
	l047033t.afm \
	l047033t.pfa \
	l047036t.afm \
	l047036t.pfa \
	l048013t.afm \
	l048013t.pfa \
	l048016t.afm \
	l048016t.pfa \
	l048033t.afm \
	l048033t.pfa \
	l048036t.afm \
	l048036t.pfa \
	l049013t.afm \
	l049013t.pfa \
	l049016t.afm \
	l049016t.pfa \
	l049033t.afm \
	l049033t.pfa \
	l049036t.afm \
	l049036t.pfa \
	c0419bt_.afm \
	c0419bt_.pfb \
	c0582bt_.afm \
	c0582bt_.pfb \
	c0583bt_.afm \
	c0583bt_.pfb \
	c0611bt_.afm \
	c0611bt_.pfb \
	c0632bt_.afm \
	c0632bt_.pfb \
	c0633bt_.afm \
	c0633bt_.pfb \
	c0648bt_.afm \
	c0648bt_.pfb \
	c0649bt_.afm \
	c0649bt_.pfb

fontdir = @FONTDIR@
font_DATA = $(FONT_FILES)

EXTRA_DIST = $(FONT_FILES) COPYRIGHT.BH Copyright COPYRIGHT.IBM autogen.sh

install-data-hook:
	@rm -f $(DESTDIR)$(fontdir)/fonts.scale
	$(MKFONTSCALE) $(DESTDIR)$(fontdir)
	@rm -f $(DESTDIR)$(fontdir)/font.dir
	$(MKFONTDIR) $(DESTDIR)$(fontdir)
	$(FCCACHE) $(DESTDIR)$(fontdir)

distuninstallcheck:
	@:

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
