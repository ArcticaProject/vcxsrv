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

if ENABLE_SPECS
doc_sources = libXaw.xml
dist_doc_DATA = $(doc_sources)				\
	AsciiSink.xml					\
	AsciiSource.xml					\
	AsciiText.xml					\
	Box.xml						\
	CH1.xml						\
	CH2.xml						\
	CH3.xml						\
	CH4.xml						\
	CH5.xml						\
	CH6.xml						\
	CH7.xml						\
	Command.xml					\
	Dialog.xml					\
	Form.xml					\
	Grip.xml					\
	Label.xml					\
	List.xml					\
	MenuButton.xml					\
	Paned.xml					\
	Panner.xml					\
	Porthole.xml					\
	Repeater.xml					\
	Scrollbar.xml					\
	SimpleMenu.xml					\
	Simple.xml					\
	SmeBSB.xml					\
	SmeLine.xml					\
	Sme.xml						\
	StripChart.xml					\
	Template_private_header_file.xml		\
	Template_public_header_file.xml			\
	Template_widget_source_file.xml			\
	Template.xml					\
	TextActions_default_translation_bindings.xml	\
	TextActions_text_widget_actions.xml		\
	TextActions.xml					\
	TextCustom.xml					\
	TextFuncs.xml					\
	TextSink.xml					\
	TextSource.xml					\
	Text.xml					\
	Toggle.xml					\
	TPage_Credits.xml				\
	Tree.xml					\
	Viewport.xml

if HAVE_XMLTO
doc_DATA = $(doc_sources:.xml=.html)

if HAVE_FOP
doc_DATA += $(doc_sources:.xml=.ps) $(doc_sources:.xml=.pdf)
endif

if HAVE_XMLTO_TEXT
doc_DATA += $(doc_sources:.xml=.txt)
endif

if HAVE_STYLESHEETS
XMLTO_FLAGS = -m $(XSL_STYLESHEET) \
	--stringparam html.stylesheet=$(STYLESHEET_SRCDIR)/xorg.css
endif

CLEANFILES = $(doc_DATA)

SUFFIXES = .xml .ps .pdf .txt .html

%.txt: %.xml $(dist_doc_DATA)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) txt $<

%.html: %.xml $(dist_doc_DATA)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) xhtml-nochunks $<

%.pdf: %.xml $(dist_doc_DATA)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop pdf $<

%.ps: %.xml $(dist_doc_DATA)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop ps $<

endif HAVE_XMLTO
endif ENABLE_SPECS
