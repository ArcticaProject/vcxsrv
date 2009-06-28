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

SGML_FILES = DESIGN.sgml

if BUILD_LINUXDOC
TXT_FILES  = $(SGML_FILES:%.sgml=%.txt)
PS_FILES   = $(SGML_FILES:%.sgml=%.ps)
if BUILD_PDFDOC
PDF_FILES  = $(SGML_FILES:%.sgml=%.pdf)
endif
HTML_FILES = $(SGML_FILES:%.sgml=%.html)

SUFFIXES = .sgml .txt .html .ps .pdf

.sgml.txt:
	@rm -f $@
	$(MAKE_TEXT) $<

.sgml.ps:
	@rm -f $@
	$(MAKE_PS) $<

.ps.pdf:
	@rm -f $@
	$(MAKE_PDF) $<

.sgml.html:
	@rm -f $@
	$(MAKE_HTML) $<

noinst_DATA = $(TXT_FILES) $(PS_FILES) $(PDF_FILES) $(HTML_FILES)
CLEANFILES = $(TXT_FILES) $(PS_FILES) $(PDF_FILES) $(HTML_FILES)
endif

EXTRA_DIST = $(SGML_FILES)
