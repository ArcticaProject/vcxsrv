ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\Speedo

FONT_FILES = \
	font0419.spd \
	font0582.spd \
	font0583.spd \
	font0611.spd \
	font0648.spd \
	font0649.spd \
	font0709.spd \
	font0710.spd \
	fonts.scale

fontdir = .
font_DATA = $(FONT_FILES:%=$(DESTDIR)\%)

$(DESTDIR)\fonts.dir: $(DESTDIR)\fonts.scale

all: $(DESTDIR) $(DESTDIR)\fonts.dir

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.dir: $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .

$(DESTDIR)\%.spd: %.spd
	copy $< $@

$(DESTDIR)\%.scale: %.scale
	copy $< $@
