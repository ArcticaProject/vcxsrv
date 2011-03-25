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

if ENABLE_DOCS
dist_doc_DATA = Xmu.xml xlogo.svg

if HAVE_XMLTO
doc_DATA = Xmu.html

if HAVE_FOP
doc_DATA += Xmu.pdf Xmu.ps
endif

if HAVE_XMLTO_TEXT
doc_DATA += Xmu.txt
endif

CLEANFILES = Xmu.txt Xmu.html Xmu.pdf Xmu.ps xorg.css

if HAVE_STYLESHEETS
XMLTO_FLAGS = 			\
	-m $(XSL_STYLESHEET)	\
	--stringparam img.src.path=$(abs_builddir)/	\
	--stringparam html.stylesheet=$(STYLESHEET_SRCDIR)/xorg.css
endif

XML_SOURCEFILES =  $(srcdir)/Xmu.xml $(XSL_STYLESHEET)

Xmu.txt: $(XML_SOURCEFILES)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) txt $(srcdir)/Xmu.xml

Xmu.html: $(XML_SOURCEFILES)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) xhtml-nochunks $(srcdir)/Xmu.xml

Xmu.pdf: $(XML_SOURCEFILES)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop pdf $(srcdir)/Xmu.xml

Xmu.ps: $(XML_SOURCEFILES)
	$(AM_V_GEN)$(XMLTO) $(XMLTO_FLAGS) --with-fop ps $(srcdir)/Xmu.xml
endif HAVE_XMLTO
endif ENABLE_DOCS
