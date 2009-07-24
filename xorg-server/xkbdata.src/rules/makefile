ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\rules
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

BASE_PARTS_NO_COMPAT = base.hdr.part base.lists.part \
HDR base.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR base.mlv_s.part \
HDR base.ml_s.part \
HDR base.ml1_s.part \
HDR \
HDR base.l2_s.part \
HDR base.l3_s.part \
HDR base.l4_s.part \
HDR HDR HDR \
HDR base.m_s.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

EVDEV_PARTS_NO_COMPAT = base.hdr.part base.lists.part \
HDR evdev.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part  \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR base.mlv_s.part \
HDR base.ml_s.part \
HDR base.ml1_s.part \
HDR \
HDR base.l2_s.part \
HDR base.l3_s.part \
HDR base.l4_s.part \
HDR HDR HDR \
HDR evdev.m_s.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

BASE_PARTS = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR base.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part      \
HDR base.o_k.part \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR compat/base.mlv_s.part base.mlv_s.part \
HDR compat/base.ml_s.part base.ml_s.part  \
HDR compat/base.ml1_s.part base.ml1_s.part \
HDR compat/base.ml1v1_s.part \
HDR compat/base.l2_s.part base.l2_s.part  \
HDR compat/base.l3_s.part base.l3_s.part  \
HDR compat/base.l4_s.part base.l4_s.part  \
HDR compat/base.l2v2_s.part \
HDR compat/base.l3v3_s.part \
HDR compat/base.l4v4_s.part \
HDR base.m_s.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.lo_s.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR compat/base.o_s.part base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

EVDEV_PARTS = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR evdev.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part      \
HDR base.o_k.part \
HDR base.ml_g.part \
HDR base.m_g.part \
HDR compat/base.mlv_s.part base.mlv_s.part \
HDR compat/base.ml_s.part base.ml_s.part  \
HDR compat/base.ml1_s.part base.ml1_s.part \
HDR compat/base.ml1v1_s.part \
HDR compat/base.l2_s.part base.l2_s.part  \
HDR compat/base.l3_s.part base.l3_s.part  \
HDR compat/base.l4_s.part base.l4_s.part  \
HDR compat/base.l2v2_s.part \
HDR compat/base.l3v3_s.part \
HDR compat/base.l4v4_s.part \
HDR evdev.m_s.part \
HDR base.ml_c.part \
HDR base.ml1_c.part \
HDR base.m_t.part \
HDR base.lo_s.part \
HDR base.l1o_s.part \
HDR base.l2o_s.part \
HDR base.l3o_s.part \
HDR base.l4o_s.part \
HDR compat/base.o_s.part base.o_s.part \
HDR base.o_c.part \
HDR base.o_t.part

RULES_FILES = base evdev xfree98

LST_FILES = base.lst evdev.lst

%.lst: %.xml
	perl xml2lst.pl < $< > $@

base: $(BASE_PARTS_NO_COMPAT)
	merge $@ $(BASE_PARTS)

evdev: $(EVDEV_PARTS_NO_COMPAT)
	merge $@ $(EVDEV_PARTS) 

ALL_RULES_FILES = $(RULES_FILES) $(LST_FILES) \
xkb.dtd README \
	sgi \
	sgi.lst \
	sun \
	sun.lst \
	xfree98 \
	xfree98.lst \
	xorg \
	xorg-it.lst \
	xorg.lst \
	xorg.xml 

rules_DATA = $(ALL_RULES_FILES)

XML_IN_FILES = base.xml.in evdev.xml.in
xml_DATA = $(XML_IN_FILES:%.xml.in=%.xml) 

$(DESTDIR)\%.xml: %.xml.in
	copy $< $@



DATA_FILES=$(rules_DATA:%=$(DESTDIR)\%) $(xml_DATA:%=$(DESTDIR)\%)

include ..\xkbrules.mak
