ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

UTIL_DIR = ..\font-util

load_makefile $(UTIL_DIR)\makefile MAKESERVER=0 DEBUG=$(DEBUG)

UCS2ANY = $(UTIL_DIR)\$(OBJDIR)\ucs2any.exe

   # The following rule is to avoid parallel build problems
.PHONY: buildtoolsfirst
buildtoolsfirst: $(BDFTOPCF) $(UCS2ANY)

all: buildtoolsfirst

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\75dpi

BDF2UCS_FONT_FILES = \
	lutBS08 \
	lutBS10 \
	lutBS12 \
	lutBS14 \
	lutBS18 \
	lutBS19 \
	lutBS24 \
	lutRS08 \
	lutRS10 \
	lutRS12 \
	lutRS14 \
	lutRS18 \
	lutRS19 \
	lutRS24 \
	UTB___10 \
	UTB___12 \
	UTB___14 \
	UTB___18 \
	UTB___24 \
	UTBI__10 \
	UTBI__12 \
	UTBI__14 \
	UTBI__18 \
	UTBI__24 \
	UTI___10 \
	UTI___12 \
	UTI___14 \
	UTI___18 \
	UTI___24 \
	UTRG__10 \
	UTRG__12 \
	UTRG__14 \
	UTRG__18 \
	UTRG__24 \
	courB08 \
	courB10 \
	courB12 \
	courB14 \
	courB18 \
	courB24 \
	courBO08 \
	courBO10 \
	courBO12 \
	courBO14 \
	courBO18 \
	courBO24 \
	courO08 \
	courO10 \
	courO12 \
	courO14 \
	courO18 \
	courO24 \
	courR08 \
	courR10 \
	courR12 \
	courR14 \
	courR18 \
	courR24 \
	helvB08 \
	helvB10 \
	helvB12 \
	helvB14 \
	helvB18 \
	helvB24 \
	helvBO08 \
	helvBO10 \
	helvBO12 \
	helvBO14 \
	helvBO18 \
	helvBO24 \
	helvO08 \
	helvO10 \
	helvO12 \
	helvO14 \
	helvO18 \
	helvO24 \
	helvR08 \
	helvR10 \
	helvR12 \
	helvR14 \
	helvR18 \
	helvR24 \
	ncenB08 \
	ncenB10 \
	ncenB12 \
	ncenB14 \
	ncenB18 \
	ncenB24 \
	ncenBI08 \
	ncenBI10 \
	ncenBI12 \
	ncenBI14 \
	ncenBI18 \
	ncenBI24 \
	ncenI08 \
	ncenI10 \
	ncenI12 \
	ncenI14 \
	ncenI18 \
	ncenI24 \
	ncenR08 \
	ncenR10 \
	ncenR12 \
	ncenR14 \
	ncenR18 \
	ncenR24 \
	timB08 \
	timB10 \
	timB12 \
	timB14 \
	timB18 \
	timB24 \
	timBI08 \
	timBI10 \
	timBI12 \
	timBI14 \
	timBI18 \
	timBI24 \
	timI08 \
	timI10 \
	timI12 \
	timI14 \
	timI18 \
	timI24 \
	timR08 \
	timR10 \
	timR12 \
	timR14 \
	timR18 \
	timR24 \
	lubB08 \
	lubB10 \
	lubB12 \
	lubB14 \
	lubB18 \
	lubB19 \
	lubB24 \
	lubBI08 \
	lubBI10 \
	lubBI12 \
	lubBI14 \
	lubBI18 \
	lubBI19 \
	lubBI24 \
	lubI08 \
	lubI10 \
	lubI12 \
	lubI14 \
	lubI18 \
	lubI19 \
	lubI24 \
	luBIS08 \
	luBIS10 \
	luBIS12 \
	luBIS14 \
	luBIS18 \
	luBIS19 \
	luBIS24 \
	lubR08 \
	lubR10 \
	lubR12 \
	lubR14 \
	lubR18 \
	lubR19 \
	lubR24 \
	luBS08 \
	luBS10 \
	luBS12 \
	luBS14 \
	luBS18 \
	luBS19 \
	luBS24 \
	luIS08 \
	luIS10 \
	luIS12 \
	luIS14 \
	luIS18 \
	luIS19 \
	luIS24 \
	luRS08 \
	luRS10 \
	luRS12 \
	luRS14 \
	luRS18 \
	luRS19 \
	luRS24
EXTRA_FONT_FILES = \
	charB08 \
	charB10 \
	charB12 \
	charB14 \
	charB18 \
	charB24 \
	charBI08 \
	charBI10 \
	charBI12 \
	charBI14 \
	charBI18 \
	charBI24 \
	charI08 \
	charI10 \
	charI12 \
	charI14 \
	charI18 \
	charI24 \
	charR08 \
	charR10 \
	charR12 \
	charR14 \
	charR18 \
	charR24 \
	tech14 \
	techB14 \
	term14 \
	termB14 \
	symb08 \
	symb10 \
	symb12 \
	symb14 \
	symb18 \
	symb24
FONT_FILES = $(BDF2UCS_FONT_FILES) $(EXTRA_FONT_FILES)

BDF_FILES = $(FONT_FILES:%=%.bdf)
PCF_FILES = $(FONT_FILES:%=$(DESTDIR)\%.pcf.gz)

ISO8859_1_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-1.pcf.gz)

ISO8859_2_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-2.pcf.gz)

ISO8859_3_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-3.pcf.gz)

ISO8859_4_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-4.pcf.gz)

ISO8859_9_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-9.pcf.gz)

ISO8859_10_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-10.pcf.gz)

ISO8859_13_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-13.pcf.gz)

ISO8859_14_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-14.pcf.gz)

ISO8859_15_PCF_FILES = $(BDF2UCS_FONT_FILES:%=$(DESTDIR)\%-ISO8859-15.pcf.gz)

$(DESTDIR)\%-ISO8859-1.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-1.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-1 ISO8859-1
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-1.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-1.bdf)

$(DESTDIR)\%-ISO8859-2.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-2.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-2 ISO8859-2
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-2.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-2.bdf)

$(DESTDIR)\%-ISO8859-3.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-3.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-3 ISO8859-3
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-3.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-3.bdf)

$(DESTDIR)\%-ISO8859-4.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-4.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-4 ISO8859-4
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-4.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-4.bdf)

$(DESTDIR)\%-ISO8859-9.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-9.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-9 ISO8859-9
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-9.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-9.bdf)

$(DESTDIR)\%-ISO8859-10.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-10.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-10 ISO8859-10
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-10.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-10.bdf)

$(DESTDIR)\%-ISO8859-13.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-13.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-13 ISO8859-13
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-13.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-13.bdf)

$(DESTDIR)\%-ISO8859-14.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-14.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-14 ISO8859-14
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-14.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-14.bdf)

$(DESTDIR)\%-ISO8859-15.pcf.gz: %.bdf
	@del -e $(<:%.bdf=%-ISO8859-15.bdf)
	$(UCS2ANY) $< $(UTIL_DIR)\map-ISO8859-15 ISO8859-15
	@del -e $@
	$(BDFTOPCF) -t $(<:%.bdf=%-ISO8859-15.bdf) | gzip > $@
	@del $(<:%.bdf=%-ISO8859-15.bdf)

font_DATA = \
	$(PCF_FILES) \
	$(ISO8859_1_PCF_FILES) \
	$(ISO8859_2_PCF_FILES) \
	$(ISO8859_3_PCF_FILES) \
	$(ISO8859_4_PCF_FILES) \
	$(ISO8859_9_PCF_FILES) \
	$(ISO8859_10_PCF_FILES) \
	$(ISO8859_13_PCF_FILES) \
	$(ISO8859_14_PCF_FILES) \
	$(ISO8859_15_PCF_FILES)

all: $(DESTDIR) $(DESTDIR)\fonts.dir $(DESTDIR)\fonts.alias

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.scale: $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(DESTDIR)\fonts.scale $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .

$(DESTDIR)\fonts.alias: fonts.alias
	copy $< $@

