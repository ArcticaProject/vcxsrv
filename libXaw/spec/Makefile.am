#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of the copyright holders shall
# not be used in advertising or otherwise to promote the sale, use or
# other dealings in this Software without prior written authorization
# from the copyright holders.
#

# Based on xc/doc/specs/Xaw/Makefile from X11R6.9

XDOCMACROS = macros.t
XIDXMACROS = indexmacros.t
EVERYWHERE = $(XDOCMACROS) strings.mit strings.xaw
CHAPTER1 = CH1
CHAPTER2 = CH2
CHAPTER3 = CH3.intro Command Grip Label List Panner Repeater \
           Scrollbar Simple StripChart Toggle
CHAPTER4 = CH4.intro SimpleMenu SmeBSB SmeLine Sme MenuButton
CHAPTER5 = CH5.intro TextActions TextFuncs AsciiText AsciiSource AsciiSink \
           TextCustom Text TextSource TextSink
CHAPTER6 = CH6.intro Box Dialog Form Paned Porthole Tree Viewport
CHAPTER7 = CH7.intro Template

spec_sources = \
        $(EVERYWHERE) TPage_Credits \
        $(CHAPTER1) $(CHAPTER2) $(CHAPTER3) $(CHAPTER4) \
        $(CHAPTER5) $(CHAPTER6) $(CHAPTER7)

spec_input = $(spec_sources:%=$(srcdir)/%)

EXTRA_DIST = $(spec_sources) $(XIDXMACROS) block.awk fixindex.awk widg.idxmac.t

if BUILD_DOCS
doc_DATA = widgets.ps widgets.index.ps widgets.txt widgets.html

CLEANFILES = $(doc_DATA)
MOSTLYCLEANFILES = index.*

GROFF_DEFS = -dxV="$(PACKAGE_STRING)"
GROFF_FLAGS = -t -ms $(GROFF_DEFS)

widgets.ps: $(spec_input)
	-$(AM_V_GEN) $(GROFF) -Tps $(GROFF_FLAGS) $(spec_input) \
	 2> index.raw > $@
	@if grep '^[^1-9.]' index.raw | grep -v warning; then exit 1; \
	 else test $$? -le 1; fi

widgets.txt: $(spec_input)
	$(AM_V_GEN) $(GROFF) -Tascii $(GROFF_FLAGS) $(spec_input) \
	 2> index.txt.raw > $@

widgets.html: $(spec_input)
	$(AM_V_GEN) $(GROFF) -Thtml $(GROFF_FLAGS) $(spec_input) \
	 2> index.html.raw > $@

index.raw: widgets.ps

index.pageno: index.raw
	$(AM_V_GEN)$(SED) -n '$$p' index.raw > $@

index.troff: index.raw
	$(AM_V_GEN)$(GREP) '^[1-9]' index.raw | \
	 (sort -f '-t:' -k2,2 -k1,1n 2>/dev/null || \
	  sort -f '-t:' +1 -3 +0n -1n) | \
	 $(AWK) -f $(srcdir)/fixindex.awk | \
	 $(AWK) -f $(srcdir)/block.awk > $@

widgets.index.ps: widg.idxmac.t index.troff index.pageno
	$(AM_V_GEN)$(GROFF) -me $(GROFF_DEFS) \
	 $(srcdir)/widg.idxmac.t $(srcdir)/$(XIDXMACROS) index.troff > $@


# Useful for running off part of the manual by hand,
# e.g., make part PART=Label
part: $(srcdir)/$(PART)
	$(GROFF) -Tps $(GROFF_FLAGS) $(EVERYWHERE) $(srcdir)/$(PART) > $(PART).ps

endif BUILD_DOCS
