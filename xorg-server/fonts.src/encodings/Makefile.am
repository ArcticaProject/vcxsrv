SUBDIRS = large

encodingsdir = @ENCODINGSDIR@

ENCODING_FILES =		\
	adobe-dingbats.enc	\
	adobe-standard.enc	\
	adobe-symbol.enc	\
	armscii-8.enc		\
	ascii-0.enc		\
	dec-special.enc		\
	ibm-cp437.enc		\
	ibm-cp850.enc		\
	ibm-cp852.enc		\
	ibm-cp866.enc		\
	iso8859-11.enc		\
	iso8859-13.enc		\
	iso8859-16.enc		\
	iso8859-6.16.enc	\
	iso8859-6.8x.enc	\
	microsoft-cp1250.enc	\
	microsoft-cp1251.enc	\
	microsoft-cp1252.enc	\
	microsoft-cp1253.enc	\
	microsoft-cp1254.enc	\
	microsoft-cp1255.enc	\
	microsoft-cp1256.enc	\
	microsoft-cp1257.enc	\
	microsoft-cp1258.enc	\
	microsoft-win3.1.enc	\
	mulearabic-0.enc	\
	mulearabic-1.enc	\
	mulearabic-2.enc	\
	mulelao-1.enc		\
	suneu-greek.enc		\
	tcvn-0.enc		\
	tis620-2.enc		\
	viscii1.1-1.enc

EXTRA_DIST = $(ENCODING_FILES) ChangeLog

COMPRESSED = $(ENCODING_FILES:%=%.gz)

CLEANFILES = encodings.dir

if GZIP_SMALL
DATA_FILES = $(COMPRESSED)
CLEANFILES += $(COMPRESSED)
else
DATA_FILES = $(ENCODING_FILES)
endif

SUFFIXES = .enc .enc.gz

.enc.enc.gz:
	$(AM_V_GEN)$(GZIP) -c < $< > $@

encodings.dir: $(DATA_FILES)
	$(AM_V_GEN)$(MKFONTSCALE) -b -s -l -n -r -p $(encodingsdir) -e . -e large .

encodings_DATA = $(DATA_FILES) encodings.dir

MAINTAINERCLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog

