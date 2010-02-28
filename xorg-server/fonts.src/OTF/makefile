ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\OTF

FONT_FILES = \
	SyrCOMAdiabene.otf \
	SyrCOMAntioch.otf \
	SyrCOMBatnanBold.otf \
	SyrCOMBatnan.otf \
	SyrCOMCtesiphon.otf \
	SyrCOMEdessa.otf \
	SyrCOMJerusalemBold.otf \
	SyrCOMJerusalemItalic.otf \
	SyrCOMJerusalem.otf \
	SyrCOMJerusalemOutline.otf \
	SyrCOMKharput.otf \
	SyrCOMMalankara.otf \
	SyrCOMMardinBold.otf \
	SyrCOMMardin.otf \
	SyrCOMMidyat.otf \
	SyrCOMNisibin.otf \
	SyrCOMNisibinOutline.otf \
	SyrCOMQenNeshrin.otf \
	SyrCOMTalada.otf \
	SyrCOMTurAbdin.otf \
	SyrCOMUrhoyBold.otf \
	SyrCOMUrhoy.otf \
	GohaTibebZemen.otf

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

$(DESTDIR)\%.otf: %.otf
	copy $< $@
