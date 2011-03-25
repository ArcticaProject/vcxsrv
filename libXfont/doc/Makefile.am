#
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

# Developer documentation - not installed on system
EXTRA_DIST = fontlib.xml

if ENABLE_DEVEL_DOCS
doc_sources = fontlib.xml

if HAVE_XMLTO
noinst_DATA = $(doc_sources:.xml=.html)

if HAVE_FOP
noinst_DATA += $(doc_sources:.xml=.ps) $(doc_sources:.xml=.pdf)
endif

if HAVE_XMLTO_TEXT
noinst_DATA += $(doc_sources:.xml=.txt)
endif

if HAVE_STYLESHEETS
XMLTO_FLAGS = -m $(XSL_STYLESHEET)

noinst_DATA += xorg.css
xorg.css: $(STYLESHEET_SRCDIR)/xorg.css
	$(AM_V_GEN)cp -pf $(STYLESHEET_SRCDIR)/xorg.css $@
endif

CLEANFILES = $(noinst_DATA)

SUFFIXES = .xml .ps .pdf .txt .html

.xml.txt:
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) txt $<

.xml.html:
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) xhtml-nochunks $<

.xml.pdf:
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop pdf $<

.xml.ps:
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop ps $<

endif HAVE_XMLTO
endif ENABLE_DEVEL_DOCS
