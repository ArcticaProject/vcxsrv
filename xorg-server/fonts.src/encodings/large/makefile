ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

INC_BDF_RULES=1

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\encodings\large

ENCODING_FILES =		\
	big5.eten-0.enc		\
	big5hkscs-0.enc		\
	cns11643-1.enc		\
	cns11643-2.enc		\
	cns11643-3.enc		\
	gb18030-0.enc		\
	gb18030.2000-0.enc	\
	gb18030.2000-1.enc	\
	gb2312.1980-0.enc	\
	gbk-0.enc		\
	jisx0201.1976-0.enc	\
	jisx0208.1990-0.enc	\
	jisx0212.1990-0.enc	\
	ksc5601.1987-0.enc	\
	ksc5601.1992-3.enc	\
	sun.unicode.india-0.enc

COMPRESSED = $(ENCODING_FILES:%=$(DESTDIR)\%.gz)

DATA_FILES = $(COMPRESSED)

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

all: $(DESTDIR)\encodings.dir

$(DESTDIR)\encodings.dir: $(DATA_FILES) $(MKFONTSCALE)
	cd "$(DESTDIR)" & $(MKFONTSCALE) -b -s -l -n -r -p .\large -e . .


