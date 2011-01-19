#
# fontconfig/doc/Makefile.am
#
# Copyright © 2003 Keith Packard
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of Keith Packard not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  Keith Packard makes no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

CC = @CC_FOR_BUILD@
EXEEXT = @EXEEXT_FOR_BUILD@
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

DOC_SRC = $(srcdir)
DOC_MODULE = fontconfig
DOC2HTML = docbook2html
DOC2TXT = docbook2txt
DOC2MAN = docbook2man
DOC2PDF = docbook2pdf

TXT = fontconfig-user.txt fontconfig-devel.txt
PDF = fontconfig-user.pdf fontconfig-devel.pdf
HTML_FILES = fontconfig-user.html
HTML_DIR = fontconfig-devel
SGML = fontconfig-user.sgml fontconfig-devel.sgml
FNCS_TMPL = ${DOC_SRC}/func.sgml

DOC_FUNCS_FNCS=\
	fcatomic.fncs \
	fcblanks.fncs \
	fccache.fncs \
	fccharset.fncs \
	fcconfig.fncs \
	fcconstant.fncs \
	fcdircache.fncs \
	fcfile.fncs \
	fcfontset.fncs \
	fcformat.fncs \
	fcfreetype.fncs \
	fcinit.fncs \
	fclangset.fncs \
	fcmatrix.fncs \
	fcobjectset.fncs \
	fcobjecttype.fncs \
	fcpattern.fncs \
	fcstring.fncs \
	fcstrset.fncs \
	fcvalue.fncs

DOC_FUNCS_SGML=\
	fcatomic.sgml \
	fcblanks.sgml \
	fccache.sgml \
	fccharset.sgml \
	fcconfig.sgml \
	fcconstant.sgml \
	fcdircache.sgml \
	fcfile.sgml \
	fcfontset.sgml \
	fcformat.sgml \
	fcfreetype.sgml \
	fcinit.sgml \
	fclangset.sgml \
	fcmatrix.sgml \
	fcobjectset.sgml \
	fcobjecttype.sgml \
	fcpattern.sgml \
	fcstring.sgml \
	fcstrset.sgml \
	fcvalue.sgml

man5_MANS=fonts-conf.5 
man3_MANS=$(DOCMAN3)

noinst_PROGRAMS=edit-sgml
edit_sgml_SOURCES=edit-sgml.c

DOC_FILES=$(TXT) $(PDF) $(HTML_FILES)
LOCAL_DOCS=$(man3_MANS) $(man5_MANS) $(DOC_FILES) $(HTML_DIR)/*

check_SCRIPTS=check-missing-doc
TESTS_ENVIRONMENT=top_srcdir=${top_srcdir} sh
TESTS=check-missing-doc

EXTRA_DIST=$(LOCAL_DOCS) $(SGML) $(DOC_FUNCS_FNCS) $(check_SCRIPTS) func.sgml confdir.sgml.in

SUFFIXES=.fncs .sgml .txt .html

if USEDOCBOOK

.fncs.sgml:
	$(RM) $@
	./edit-sgml$(EXEEXT) $(FNCS_TMPL) < '$<' > $*.sgml

.sgml.txt:
	$(RM) $@
	$(DOC2TXT) $<

.sgml.pdf:
	$(RM) $@
	$(DOC2PDF) $<

$(man3_MANS): func.refs

func.refs: local-fontconfig-devel.sgml $(DOC_FUNCS_SGML) version.sgml confdir.sgml
	$(RM) func.refs
	$(DOC2MAN) -o devel-man local-fontconfig-devel.sgml && \
	mv devel-man/manpage.refs func.refs && \
	mv devel-man/*.3 . && \
	$(RM) devel-man/manpage.* && \
	rmdir devel-man

local-fontconfig-devel.sgml: fontconfig-devel.sgml
	$(LN_S) $< $@

$(DOC_FUNCS_SGML): edit-sgml$(EXEEXT) $(FNCS_TMPL)

fonts-conf.5: local-fontconfig-user.sgml version.sgml confdir.sgml
	$(RM) $@
	$(DOC2MAN) local-fontconfig-user.sgml && \
	$(RM) manpage.*
	
local-fontconfig-user.sgml: fontconfig-user.sgml
	$(LN_S) $< $@

all-local: $(LOCAL_DOCS)

clean-local:
	$(RM) $(man3_MANS) $(man5_MANS) $(DOC_FILES) $(DOC_FUNCS_SGML) func.refs
	$(RM) -r $(HTML_DIR)

$(HTML_DIR): local-fontconfig-devel.sgml $(DOC_FUNCS_SGML) version.sgml confdir.sgml
	$(RM) -r $(HTML_DIR)
	$(DOC2HTML) -V '%use-id-as-filename%' -o $(HTML_DIR) local-fontconfig-devel.sgml

fontconfig-devel.txt: local-fontconfig-devel.sgml $(DOC_FUNCS_SGML) version.sgml confdir.sgml
	$(RM) $@
	$(DOC2TXT) local-fontconfig-devel.sgml
	mv local-fontconfig-devel.txt $@

fontconfig-devel.pdf: local-fontconfig-devel.sgml $(DOC_FUNCS_SGML) version.sgml confdir.sgml
	$(RM) $@
	$(top_srcdir)/missing --run $(DOC2PDF) $< && mv local-$@ $@ \
	|| echo Failed to generate $@ >&2; \
	(test -f $@ || echo $(DOC2PDF) is required to generate this file >> $@)

fontconfig-user.html: local-fontconfig-user.sgml version.sgml confdir.sgml
	$(RM) $@ local-$@ $@.tmp
	$(DOC2HTML) -u local-fontconfig-user.sgml > $@.tmp
	-test -f local-$@ && mv local-$@ $@
	-test -f $@ || mv $@.tmp $@
	-test -f $@.tmp && $(RM) $@.tmp

fontconfig-user.txt: local-fontconfig-user.sgml version.sgml confdir.sgml
	$(RM) $@
	$(DOC2TXT) local-fontconfig-user.sgml
	mv local-fontconfig-user.txt $@

fontconfig-user.pdf: local-fontconfig-user.sgml version.sgml confdir.sgml
	$(RM) $@
	$(top_srcdir)/missing --run $(DOC2PDF) $< && mv local-$@ $@ \
	|| echo Failed to generate $@ >&2; \
	(test -f $@ || echo $(DOC2PDF) is required to generate this file >> $@)

STRIPNL=awk '{ if (NR > 1) printf ("\n"); printf ("%s", $$0); }'
confdir.sgml: ${DOC_SRC}/confdir.sgml.in
	sed "s,@CONFDIR\@,${CONFDIR}," < ${DOC_SRC}/confdir.sgml.in | $(STRIPNL) > confdir.sgml

CLEANFILES=confdir.sgml local-fontconfig-user.sgml local-fontconfig-devel.sgml

htmldoc_DATA = $(HTML_DIR)/*

$(HTML_DIR)/*: $(HTML_DIR)

else

htmldoc_DATA = $(srcdir)/$(HTML_DIR)/*

all-local:
clean-local:
endif

htmldocdir=$(docdir)/$(HTML_DIR)

doc_DATA = $(DOC_FILES)

