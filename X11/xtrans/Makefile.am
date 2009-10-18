Xtransincludedir = $(includedir)/X11/Xtrans
Xtransinclude_HEADERS = \
	Xtrans.h \
	Xtrans.c \
	Xtransint.h \
	Xtranslcl.c \
	Xtranssock.c \
	Xtranstli.c \
	Xtransutil.c \
	transport.c

aclocaldir = $(datadir)/aclocal
aclocal_DATA = xtrans.m4

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = xtrans.pc

EXTRA_DIST = xtrans.pc.in ${aclocal_DATA} ChangeLog Xtrans.mm

MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog

# Rules to convert documentation from troff to other formats
doc_sources = Xtrans.mm

if HAVE_PS2PDF
printable_format = .pdf
else
printable_format = .ps
endif

if BUILD_DOCS
doc_DATA =	$(doc_sources:.mm=.txt) \
		$(doc_sources:.mm=$(printable_format)) \
		$(doc_sources:.mm=.html)

CLEANFILES = $(doc_DATA)
MOSTLYCLEANFILES = index.*

# Pass version string as a troff string for substitution
GROFF_DEFS = -dxV="$(PACKAGE_STRING)"

# -t to run through tbl
GROFF_FLAGS = -t -mm $(GROFF_DEFS)

SUFFIXES = .mm .ps .txt .html .pdf

.mm.ps:
	-$(AM_V_GEN) $(GROFF) -Tps $(GROFF_FLAGS) $< 2> index.$@.raw > $@
	@if grep '^[^1-9.]' index.$@.raw | grep -v warning; then exit 1; \
	 else test $$? -le 1; fi

.mm.txt:
	$(AM_V_GEN) env GROFF_NO_SGR=TRUE $(GROFF) -Tutf8 $(GROFF_FLAGS) \
	 $< 2> index.$@.raw > $@

.mm.html:
	$(AM_V_GEN) $(GROFF) -Thtml $(GROFF_FLAGS) $< 2> index.$@.raw > $@

.ps.pdf:
	$(AM_V_GEN) $(PS2PDF) $< $@

endif BUILD_DOCS
