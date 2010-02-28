ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\TTF


FONT_FILES = \
	luximbi.ttf \
	luximb.ttf \
	luximri.ttf \
	luximr.ttf \
	luxirbi.ttf \
	luxirb.ttf \
	luxirri.ttf \
	luxirr.ttf \
	luxisbi.ttf \
	luxisb.ttf \
	luxisri.ttf \
	luxisr.ttf \
	Vera.ttf \
	VeraBd.ttf \
	VeraBI.ttf \
	VeraIt.ttf \
	VeraMoBd.ttf \
	VeraMoBI.ttf \
	VeraMoIt.ttf \
	VeraMono.ttf \
	VeraSe.ttf \
	VeraSeBd.ttf \
	GohaTibebZemen.ttf

fontdir = .
font_DATA = $(FONT_FILES:%=$(DESTDIR)\%)

all: $(DESTDIR) $(DESTDIR)\fonts.dir

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.scale: $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(DESTDIR)\fonts.scale $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .
#	$(FCCACHE) $(DESTDIR)

$(DESTDIR)\%.ttf: %.ttf
	copy $< $@
