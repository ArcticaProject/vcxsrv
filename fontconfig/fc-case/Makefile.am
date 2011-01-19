#
# $Id $
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
#

CC = @CC_FOR_BUILD@
EXEEXT = @EXEEXT_FOR_BUILD@
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

INCLUDES=-I${top_srcdir}/src -I${top_srcdir} $(WARN_CFLAGS)

TMPL=fccase.tmpl.h
STMPL=${top_srcdir}/fc-case/${TMPL}
TARG=fccase.h

noinst_PROGRAMS=fc-case

noinst_HEADERS=$(TARG)

fc_case_SRCS= \
	fc-case.c \
	fccaseint.h \
	fccaseread.c

CASEFOLDING=CaseFolding.txt
SCASEFOLDING=${top_srcdir}/fc-case/CaseFolding.txt

EXTRA_DIST=$(TMPL) $(CASEFOLDING)

$(TARG): $(STMPL) fc-case$(EXEEXT) $(SCASEFOLDING)
	$(AM_V_GEN) $(RM) $(TARG); \
	./fc-case$(EXEEXT) $(SCASEFOLDING) < $(STMPL) > $(TARG).tmp && \
	mv $(TARG).tmp $(TARG)

ALIAS_FILES = fcalias.h fcaliastail.h

BUILT_SOURCES = $(ALIAS_FILES)

$(ALIAS_FILES): 
	touch $(ALIAS_FILES)

CLEANFILES=$(TARG) $(ALIAS_FILES)

