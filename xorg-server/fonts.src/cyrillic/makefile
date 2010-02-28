ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\cyrillic

FONT_FILES = \
	proof9x16 \
	koi12x24b \
	koi12x24 \
	koi5x8 \
	koi6x13b \
	koi6x13 \
	koi6x9 \
	koi7x14b \
	koi7x14 \
	koi8x13 \
	koi8x16b \
	koi8x16 \
	koi9x15b \
	koi9x15 \
	koi9x18b \
	koi9x18 \
	screen8x16b \
	screen8x16 \
	crox1cb \
	crox1c \
	crox1cbo \
	crox1co \
	crox1hb \
	crox1h \
	crox1hbo \
	crox1ho \
	crox1tb \
	crox1t \
	crox1tbo \
	crox1to \
	crox2cb \
	crox2c \
	crox2cbo \
	crox2co \
	crox2hb \
	crox2h \
	crox2hbo \
	crox2ho \
	crox2tb \
	crox2t \
	crox2tbo \
	crox2to \
	crox3cb \
	crox3c \
	crox3cbo \
	crox3co \
	crox3hb \
	crox3h \
	crox3hbo \
	crox3ho \
	crox3tb \
	crox3t \
	crox3tbo \
	crox3to \
	crox4hb \
	crox4h \
	crox4hbo \
	crox4ho \
	crox4tb \
	crox4t \
	crox4tbo \
	crox4to \
	crox5hb \
	crox5h \
	crox5hbo \
	crox5ho \
	crox5tb \
	crox5t \
	crox5tbo \
	crox5to \
	crox6hb \
	crox6h \
	crox6hbo \
	crox6ho \
	crox6tb \
	crox6t \
	crox6tbo \
	crox6to \
	koi10x16b \
	koi10x20 \
	koi6x10 \
	koinil2

BDF_FILES = $(FONT_FILES:%=%.bdf)
PCF_FILES = $(FONT_FILES:%=$(DESTDIR)\%.pcf.gz)

font_DATA = $(PCF_FILES)

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

all: $(DESTDIR) $(MKFONTSCALE) $(DESTDIR)\fonts.dir $(DESTDIR)\fonts.alias

$(DESTDIR)\fonts.scale: $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(DESTDIR)\fonts.scale $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .


$(DESTDIR)\fonts.alias: fonts.alias
	copy $< $@
