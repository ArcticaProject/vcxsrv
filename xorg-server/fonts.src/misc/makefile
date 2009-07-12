ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\misc
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

BDF2UCS_FONT_FILES = \
	clR6x12 \
	4x6 \
	5x7 \
	5x8 \
	6x9 \
	6x10 \
	6x12 \
	6x13 \
	6x13B \
	6x13O \
	7x13 \
	7x13B \
	7x13O \
	7x14 \
	7x14B \
	8x13 \
	8x13B \
	8x13O \
	9x15 \
	9x15B \
	9x18 \
	9x18B \
	10x20
BDF2UCS_8_FONT_FILES = \
	clR6x12 \
	4x6 \
	5x7 \
	5x8 \
	6x9 \
	6x10 \
	6x12 \
	6x13 \
	6x13B \
	7x13 \
	7x13B \
	7x14 \
	7x14B \
	8x13 \
	8x13B \
	9x15 \
	9x15B \
	9x18 \
	9x18B \
	10x20
BDF2UCS_11_FONT_FILES = \
	6x13 \
	7x13 \
	7x13B \
	7x13O \
	7x14 \
	7x14B \
	9x15 \
	9x15B \
	9x18 \
	10x20
BDF2UCS_KOI8_FONT_FILES = \
	clR6x12 \
	4x6 \
	5x7 \
	5x8 \
	6x9 \
	6x10 \
	6x12 \
	6x13 \
	7x13 \
	7x14 \
	8x13 \
	9x15 \
	9x18 \
	10x20
BDF2UCS_JISX0201_FONT_FILES = \
	7x14
EXTRA_FONT_FILES = \
	12x13ja \
	18x18ja \
	18x18ko \
	k14 \
	nil2 \
	micro \
	jiskan16 \
	jiskan24 \
	gb16fs \
	gb16st \
	gb24st \
	deccurs \
	decsess \
	arabic24 \
	cursor \
	hanglg16 \
	hanglm16 \
	hanglm24 \
	cu12 \
	cu-alt12 \
	cu-arabic12 \
	cuarabic12 \
	cu-devnag12 \
	cudevnag12 \
	cu-lig12 \
	cu-pua12 \
	clB6x10 \
	clB6x12 \
	clB8x10 \
	clB8x12 \
	clB8x13 \
	clB8x14 \
	clB8x16 \
	clB8x8 \
	clB9x15 \
	clI6x12 \
	clI8x8 \
	clR4x6 \
	clR5x10 \
	clR5x6 \
	clR5x8 \
	clR6x10 \
	clR6x13 \
	clR6x6 \
	clR6x8 \
	clR7x10 \
	clR7x12 \
	clR7x14 \
	clR7x8 \
	clR8x10 \
	clR8x12 \
	clR8x13 \
	clR8x14 \
	clR8x16 \
	clR8x8 \
	clR9x15 \
	12x24 \
	12x24rk \
	8x16 \
	8x16rk \
	olcursor \
	olgl10 \
	olgl12 \
	olgl14 \
	olgl19

FONT_FILES = $(BDF2UCS_FONT_FILES) $(EXTRA_FONT_FILES)

BDF_FILES = $(FONT_FILES:%=%.bdf)
PCF_FILES = $(FONT_FILES:%=$(DESTDIR)\%.pcf.gz)

ISO8859_1_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-1.bdf)
ISO8859_1_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-1.pcf.gz)

ISO8859_2_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-2.bdf)
ISO8859_2_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-2.pcf.gz)

ISO8859_3_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-3.bdf)
ISO8859_3_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-3.pcf.gz)

ISO8859_4_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-4.bdf)
ISO8859_4_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-4.pcf.gz)

ISO8859_5_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-5.bdf)
ISO8859_5_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-5.pcf.gz)

ISO8859_7_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-7.bdf)
ISO8859_7_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-7.pcf.gz)

ISO8859_8_BDF_FILES = $(BDF2UCS_8_FONT_FILES:%=%-ISO8859-8.bdf)
ISO8859_8_PCF_FILES = $(BDF2UCS_8_FONT_FILES:%=$(DESTDIR)\%-ISO8859-8.pcf.gz)

ISO8859_9_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-9.bdf)
ISO8859_9_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-9.pcf.gz)

ISO8859_10_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-10.bdf)
ISO8859_10_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-10.pcf.gz)

ISO8859_11_BDF_FILES = $(BDF2UCS_11_FONT_FILES:%=%-ISO8859-11.bdf)
ISO8859_11_PCF_FILES = $(BDF2UCS_11_FONT_FILES:%=$(DESTDIR)\%-ISO8859-11.pcf.gz)

ISO8859_13_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-13.bdf)
ISO8859_13_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-13.pcf.gz)

ISO8859_14_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-14.bdf)
ISO8859_14_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-14.pcf.gz)

ISO8859_15_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-15.bdf)
ISO8859_15_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-15.pcf.gz)


ISO8859_16_BDF_FILES = $(BDF2UCS_FONT_FILES:%=%-ISO8859-16.bdf)
ISO8859_16_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-16.pcf.gz)

KOI8_R_BDF_FILES = $(BDF2UCS_KOI8_FONT_FILES:%=%-KOI8-R.bdf)
KOI8_R_PCF_FILES = $(BDF2UCS_KOI8_FONT_FILES:%=$(DESTDIR)\%-KOI8-R.pcf.gz)

JISX0201_BDF_FILES = $(BDF2UCS_JISX0201_FONT_FILES:%=%-JISX0201.1976-0.bdf)
JISX0201_PCF_FILES = $(BDF2UCS_JISX0201_FONT_FILES:%=$(DESTDIR)\%-JISX0201.1976-0.pcf.gz)

UTIL_DIR = ..\font-util
UCS2ANY = $(UTIL_DIR)\$(OBJDIR)\ucs2any.exe

load_makefile $(UTIL_DIR)\makefile MAKESERVER=0 DEBUG=$(DEBUG)

#%t.bdf: %.bdf
#	@rm -f $@
#	perl $(BDFTRUNCATE) 0x3200 < $< > $@

%-ISO8859-1.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-1 ISO8859-1

%-ISO8859-2.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-2 ISO8859-2

%-ISO8859-3.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-3 ISO8859-3

%-ISO8859-4.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-4 ISO8859-4

%-ISO8859-5.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-5 ISO8859-5

%-ISO8859-7.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-7 ISO8859-7

%-ISO8859-8.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-8 ISO8859-8

%-ISO8859-9.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-9 ISO8859-9

%-ISO8859-10.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-10 ISO8859-10

%-ISO8859-11.bdf: %.bdf
	@rm -f $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-11 ISO8859-11

%-ISO8859-13.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-13 ISO8859-13

%-ISO8859-14.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-14 ISO8859-14

%-ISO8859-15.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-15 ISO8859-15

%-ISO8859-16.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-16 ISO8859-16

%-KOI8-R.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-KOI8-R KOI8-R

%-JISX0201.1976-0.bdf: %.bdf
	@del -e $@
	$(UCS2ANY) $< $(UTIL_DIR)\map-JISX0201.1976-0 JISX0201.1976-0

#%.pcf.gz: %t.bdf
#	@del -e $@
#	$(BDFTOPCF) -t $< | gzip > $@

font_DATA = \
	$(PCF_FILES) \
	$(ISO8859_1_PCF_FILES) \
	$(ISO8859_2_PCF_FILES) \
	$(ISO8859_3_PCF_FILES) \
	$(ISO8859_4_PCF_FILES) \
	$(ISO8859_5_PCF_FILES) \
	$(ISO8859_7_PCF_FILES) \
	$(ISO8859_8_PCF_FILES) \
	$(ISO8859_9_PCF_FILES) \
	$(ISO8859_10_PCF_FILES) \
	$(ISO8859_11_PCF_FILES) \
	$(ISO8859_13_PCF_FILES) \
	$(ISO8859_14_PCF_FILES) \
	$(ISO8859_15_PCF_FILES) \
	$(ISO8859_16_PCF_FILES) \
	$(KOI8_R_PCF_FILES) \
	$(JISX0201_PCF_FILES)
BUILT_BDF_FILES = \
	$(ISO8859_1_BDF_FILES) \
	$(ISO8859_2_BDF_FILES) \
	$(ISO8859_3_BDF_FILES) \
	$(ISO8859_4_BDF_FILES) \
	$(ISO8859_5_BDF_FILES) \
	$(ISO8859_7_BDF_FILES) \
	$(ISO8859_8_BDF_FILES) \
	$(ISO8859_9_BDF_FILES) \
	$(ISO8859_10_BDF_FILES) \
	$(ISO8859_11_BDF_FILES) \
	$(ISO8859_13_BDF_FILES) \
	$(ISO8859_14_BDF_FILES) \
	$(ISO8859_15_BDF_FILES) \
	$(ISO8859_16_BDF_FILES) \
	$(KOI8_R_BDF_FILES) \
	$(JISX0201_BDF_FILES)

all: $(DESTDIR) $(UCS2ANY) $(DESTDIR)\fonts.scale $(DESTDIR)\fonts.dir $(DESTDIR)\fonts.alias

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.scale: $(font_DATA)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(font_DATA)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .

$(DESTDIR)\fonts.alias: fonts.alias
	copy $< $@
