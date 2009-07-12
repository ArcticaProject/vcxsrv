ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\TTF
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"


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
	VeraSeBd.ttf

fontdir = .
font_DATA = $(FONT_FILES:%=$(DESTDIR)\%)

all: $(DESTDIR) $(DESTDIR)\fonts.scale $(DESTDIR)\fonts.dir

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.scale: $(font_DATA)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(font_DATA)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .
#	$(FCCACHE) $(DESTDIR)

$(DESTDIR)\%.ttf: %.ttf
	copy $< $@
