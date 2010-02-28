ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\Type1

FONT_FILES = \
	cursor.pfa \
	UTB_____.afm \
	UTBI____.afm \
	UTBI____.pfa \
	UTB_____.pfa \
	UTI_____.afm \
	UTI_____.pfa \
	UTRG____.afm \
	UTRG____.pfa \
	cour.afm \
	courb.afm \
	courbi.afm \
	courbi.pfa \
	courb.pfa \
	couri.afm \
	couri.pfa \
	cour.pfa \
	l047013t.afm \
	l047013t.pfa \
	l047016t.afm \
	l047016t.pfa \
	l047033t.afm \
	l047033t.pfa \
	l047036t.afm \
	l047036t.pfa \
	l048013t.afm \
	l048013t.pfa \
	l048016t.afm \
	l048016t.pfa \
	l048033t.afm \
	l048033t.pfa \
	l048036t.afm \
	l048036t.pfa \
	l049013t.afm \
	l049013t.pfa \
	l049016t.afm \
	l049016t.pfa \
	l049033t.afm \
	l049033t.pfa \
	l049036t.afm \
	l049036t.pfa \
	c0419bt_.afm \
	c0419bt_.pfb \
	c0582bt_.afm \
	c0582bt_.pfb \
	c0583bt_.afm \
	c0583bt_.pfb \
	c0611bt_.afm \
	c0611bt_.pfb \
	c0632bt_.afm \
	c0632bt_.pfb \
	c0633bt_.afm \
	c0633bt_.pfb \
	c0648bt_.afm \
	c0648bt_.pfb \
	c0649bt_.afm \
	c0649bt_.pfb

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

$(DESTDIR)\%.afm: %.afm
	copy $< $@

$(DESTDIR)\%.pfb: %.pfb
	copy $< $@

$(DESTDIR)\%.pfa: %.pfa
	copy $< $@
