ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

load_makefile compat\makefile DEBUG=$(DEBUG)
load_makefile extras\makefile DEBUG=$(DEBUG)

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\rules

BASE_PARTS_NO_COMPAT = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR base.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
HDR base.o_k.part \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR compat/base.mlv_s.part   extras/base.mlv_s.part   base.mlv_s.part \
HDR compat/base.ml_s.part    extras/base.ml_s.part    base.ml_s.part  \
HDR compat/base.ml1_s.part   extras/base.ml1_s.part   base.ml1_s.part \
HDR compat/base.ml1v1_s.part extras/base.ml1v1_s.part \
HDR compat/base.ml2_s.part    extras/base.ml2_s.part    base.ml2_s.part  \
HDR compat/base.ml3_s.part    extras/base.ml3_s.part    base.ml3_s.part  \
HDR compat/base.ml4_s.part    extras/base.ml4_s.part    base.ml4_s.part  \
HDR compat/base.ml2v2_s.part  extras/base.ml2v2_s.part \
HDR compat/base.ml3v3_s.part  extras/base.ml3v3_s.part \
HDR compat/base.ml4v4_s.part  extras/base.ml4v4_s.part \
HDR base.m_s.part \
HDR compat/base.lv_c.part   \
HDR compat/base.l1v1_c.part \
HDR compat/base.l2v2_c.part \
HDR compat/base.l3v3_c.part \
HDR compat/base.l4v4_c.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.lo_s.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR compat/base.o_s.part     base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

EVDEV_PARTS_NO_COMPAT = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR evdev.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
HDR base.o_k.part \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR compat/base.mlv_s.part   extras/base.mlv_s.part   base.mlv_s.part \
HDR compat/base.ml_s.part    extras/base.ml_s.part    base.ml_s.part  \
HDR compat/base.ml1_s.part   extras/base.ml1_s.part   base.ml1_s.part \
HDR compat/base.ml1v1_s.part extras/base.ml1v1_s.part \
HDR compat/base.ml2_s.part    extras/base.ml2_s.part    base.ml2_s.part  \
HDR compat/base.ml3_s.part    extras/base.ml3_s.part    base.ml3_s.part  \
HDR compat/base.ml4_s.part    extras/base.ml4_s.part    base.ml4_s.part  \
HDR compat/base.ml2v2_s.part  extras/base.ml2v2_s.part \
HDR compat/base.ml3v3_s.part  extras/base.ml3v3_s.part \
HDR compat/base.ml4v4_s.part  extras/base.ml4v4_s.part \
HDR evdev.m_s.part \
HDR compat/base.lv_c.part   \
HDR compat/base.l1v1_c.part \
HDR compat/base.l2v2_c.part \
HDR compat/base.l3v3_c.part \
HDR compat/base.l4v4_c.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.lo_s.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR compat/base.o_s.part     base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

BASE_PARTS = base.hdr.part base.lists.part \
HDR base.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
HDR \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR extras/base.mlv_s.part   base.mlv_s.part \
HDR extras/base.ml_s.part    base.ml_s.part  \
HDR extras/base.ml1_s.part   base.ml1_s.part \
HDR extras/base.ml1v1_s.part \
HDR extras/base.ml2_s.part    base.ml2_s.part  \
HDR extras/base.ml3_s.part    base.ml3_s.part  \
HDR extras/base.ml4_s.part    base.ml4_s.part  \
HDR extras/base.ml2v2_s.part \
HDR extras/base.ml3v3_s.part \
HDR extras/base.ml4v4_s.part \
HDR base.m_s.part \
HDR \
HDR \
HDR \
HDR \
HDR \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

EVDEV_PARTS = base.hdr.part base.lists.part \
HDR evdev.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part  \
HDR \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR extras/base.mlv_s.part   base.mlv_s.part \
HDR extras/base.ml_s.part    base.ml_s.part  \
HDR extras/base.ml1_s.part   base.ml1_s.part \
HDR extras/base.ml1v1_s.part \
HDR extras/base.ml2_s.part    base.ml2_s.part  \
HDR extras/base.ml3_s.part    base.ml3_s.part  \
HDR extras/base.ml4_s.part    base.ml4_s.part  \
HDR extras/base.ml2v2_s.part \
HDR extras/base.ml3v3_s.part \
HDR extras/base.ml4v4_s.part \
HDR evdev.m_s.part \
HDR \
HDR \
HDR \
HDR \
HDR \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

RULES_FILES = base evdev xfree98

LST_FILES = base.lst evdev.lst

$(DESTDIR)\%.lst: $(DESTDIR)\%.xml
	perl xml2lst.pl < $< > $@

$(DESTDIR)\base: compat\all $(BASE_PARTS_NO_COMPAT)
	merge $@ $(BASE_PARTS)

$(DESTDIR)\evdev: compat\all $(EVDEV_PARTS_NO_COMPAT)
	merge $@ $(EVDEV_PARTS) 

ALL_RULES_FILES = $(RULES_FILES) $(LST_FILES) \
xkb.dtd README

COMPATFILES=xorg xorg.xml xorg.lst
$(DESTDIR)\xorg: $(DESTDIR)\base
	copy $< $@
$(DESTDIR)\xorg%: $(DESTDIR)\base%
	copy $< $@

rules_DATA = $(ALL_RULES_FILES) $(COMPATFILES)

xml_DATA = base.xml evdev.xml base.extras.xml evdev.extras.xml 

$(DESTDIR)\%.xml: %.xml.in
	copy $< $@

$(DESTDIR)\evdev.xml: base.xml.in
	cp $< $@

$(DESTDIR)\evdev.extras.xml: base.extras.xml.in
	cp $< $@

DATA_FILES=$(rules_DATA:%=$(DESTDIR)\%) $(xml_DATA:%=$(DESTDIR)\%)

include ..\xkbrules.mak
