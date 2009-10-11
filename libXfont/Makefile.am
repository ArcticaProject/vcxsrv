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

SUBDIRS=src

libXfontincludedir = $(includedir)/X11/fonts
libXfontinclude_HEADERS = \
	include/X11/fonts/bdfint.h   \
	include/X11/fonts/bitmap.h   \
	include/X11/fonts/bufio.h    \
	include/X11/fonts/fntfil.h   \
	include/X11/fonts/fntfilio.h \
	include/X11/fonts/fntfilst.h \
	include/X11/fonts/fontencc.h \
	include/X11/fonts/fontmisc.h \
	include/X11/fonts/fontshow.h \
	include/X11/fonts/fontutil.h \
	include/X11/fonts/fontxlfd.h \
	include/X11/fonts/pcf.h      \
	src/FreeType/ft.h	     \
	src/FreeType/ftfuncs.h
nodist_libXfontinclude_HEADERS = \
	include/X11/fonts/fontconf.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xfont.pc

EXTRA_DIST = xfont.pc.in ChangeLog include/X11/fonts/fontconf.h.in

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog

# Developer documentation - not installed on system
EXTRA_DIST += doc/fontlib.xml doc/fontlib.xsl doc/fontlib.css

if MAKE_DEVEL_DOCS
if HAVE_XMLTO
noinst_DATA = doc/fontlib.txt doc/fontlib.html $(srcdir)/doc/fontlib.css
CLEANFILES = doc/fontlib.txt doc/fontlib.html doc/fontlib.pdf doc/fontlib.ps

doc/fontlib.txt: $(srcdir)/doc/fontlib.xml $(srcdir)/doc/fontlib.xsl
	$(AM_V_GEN)$(XMLTO) -m $(srcdir)/doc/fontlib.xsl -o doc txt $(srcdir)/doc/fontlib.xml

doc/fontlib.html: $(srcdir)/doc/fontlib.xml $(srcdir)/doc/fontlib.xsl
	$(AM_V_GEN)$(XMLTO) -m $(srcdir)/doc/fontlib.xsl -o doc xhtml-nochunks $(srcdir)/doc/fontlib.xml

doc/fontlib.pdf: $(srcdir)/doc/fontlib.xml $(srcdir)/doc/fontlib.xsl
	$(AM_V_GEN)$(XMLTO) -m $(srcdir)/doc/fontlib.xsl -o doc pdf $(srcdir)/doc/fontlib.xml

doc/fontlib.ps: $(srcdir)/doc/fontlib.xml $(srcdir)/doc/fontlib.xsl
	$(AM_V_GEN)$(XMLTO) -m $(srcdir)/doc/fontlib.xsl -o doc ps $(srcdir)/doc/fontlib.xml
endif HAVE_XMLTO
endif MAKE_DEVEL_DOCS
