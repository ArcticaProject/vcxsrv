ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

UCS2ANY = perl ucstoany.pl

INC_BDF_RULES=1

   # The following rule is to avoid parallel build problems
.PHONY: buildtoolsfirst
buildtoolsfirst: $(BDFTOPCF)

all: buildtoolsfirst

DESTDIR=$(MHMAKECONF)\xorg-server\fonts\terminus-font

R_88591	= ISO8859 1
R_88592	= ISO8859 2
R_88595	= ISO8859 5
R_88597	= ISO8859 7
R_88599	= ISO8859 9
R_W1251	= Microsoft CP1251
R_8859D	= ISO8859 13
R_8859F	= ISO8859 15
R_8859G	= ISO8859 16
R_PCIBM	= IBM CP437
R_KOI8R	= KOI8 R
R_KOI8U	= KOI8 U
R_BGMIK	= Bulgarian MIK
R_PT154	= Paratype PT154
R_PT254	= Paratype PT254
R_XOS42	= XOS4 2
R_10646	= ISO10646 1

PCF_12N	= ter-112n.pcf ter-212n.pcf ter-512n.pcf ter-712n.pcf ter-912n.pcf ter-c12n.pcf ter-d12n.pcf ter-f12n.pcf ter-g12n.pcf ter-k12n.pcf ter-p12n.pcf ter-u12n.pcf ter-x12n.pcf
PCF_12B	= ter-112b.pcf ter-212b.pcf ter-512b.pcf ter-712b.pcf ter-912b.pcf ter-c12b.pcf ter-d12b.pcf ter-f12b.pcf ter-g12b.pcf ter-k12b.pcf ter-p12b.pcf ter-u12b.pcf ter-x12b.pcf
PCF_14N	= ter-114n.pcf ter-214n.pcf ter-514n.pcf ter-714n.pcf ter-914n.pcf ter-c14n.pcf ter-d14n.pcf ter-f14n.pcf ter-g14n.pcf ter-k14n.pcf ter-p14n.pcf ter-u14n.pcf ter-x14n.pcf
PCF_14B	= ter-114b.pcf ter-214b.pcf ter-514b.pcf ter-714b.pcf ter-914b.pcf ter-c14b.pcf ter-d14b.pcf ter-f14b.pcf ter-g14b.pcf ter-k14b.pcf ter-p14b.pcf ter-u14b.pcf ter-x14b.pcf
PCF_16N	= ter-116n.pcf ter-216n.pcf ter-516n.pcf ter-716n.pcf ter-916n.pcf ter-c16n.pcf ter-d16n.pcf ter-f16n.pcf ter-g16n.pcf ter-k16n.pcf ter-p16n.pcf ter-u16n.pcf ter-x16n.pcf
PCF_16B	= ter-116b.pcf ter-216b.pcf ter-516b.pcf ter-716b.pcf ter-916b.pcf ter-c16b.pcf ter-d16b.pcf ter-f16b.pcf ter-g16b.pcf ter-k16b.pcf ter-p16b.pcf ter-u16b.pcf ter-x16b.pcf
PCF_20N	= ter-120n.pcf ter-220n.pcf ter-520n.pcf ter-720n.pcf ter-920n.pcf ter-c20n.pcf ter-d20n.pcf ter-f20n.pcf ter-g20n.pcf ter-k20n.pcf ter-p20n.pcf ter-u20n.pcf ter-x20n.pcf
PCF_20B	= ter-120b.pcf ter-220b.pcf ter-520b.pcf ter-720b.pcf ter-920b.pcf ter-c20b.pcf ter-d20b.pcf ter-f20b.pcf ter-g20b.pcf ter-k20b.pcf ter-p20b.pcf ter-u20b.pcf ter-x20b.pcf
PCF_22N	= ter-122n.pcf ter-222n.pcf ter-522n.pcf ter-722n.pcf ter-922n.pcf ter-c22n.pcf ter-d22n.pcf ter-f22n.pcf ter-g22n.pcf ter-k22n.pcf ter-p22n.pcf ter-u22n.pcf ter-x22n.pcf
PCF_22B	= ter-122b.pcf ter-222b.pcf ter-522b.pcf ter-722b.pcf ter-922b.pcf ter-c22b.pcf ter-d22b.pcf ter-f22b.pcf ter-g22b.pcf ter-k22b.pcf ter-p22b.pcf ter-u22b.pcf ter-x22b.pcf
PCF_24N	= ter-124n.pcf ter-224n.pcf ter-524n.pcf ter-724n.pcf ter-924n.pcf ter-c24n.pcf ter-d24n.pcf ter-f24n.pcf ter-g24n.pcf ter-k24n.pcf ter-p24n.pcf ter-u24n.pcf ter-x24n.pcf
PCF_24B	= ter-124b.pcf ter-224b.pcf ter-524b.pcf ter-724b.pcf ter-924b.pcf ter-c24b.pcf ter-d24b.pcf ter-f24b.pcf ter-g24b.pcf ter-k24b.pcf ter-p24b.pcf ter-u24b.pcf ter-x24b.pcf
PCF_28N	= ter-128n.pcf ter-228n.pcf ter-528n.pcf ter-728n.pcf ter-928n.pcf ter-c28n.pcf ter-d28n.pcf ter-f28n.pcf ter-g28n.pcf ter-k28n.pcf ter-p28n.pcf ter-u28n.pcf ter-x28n.pcf
PCF_28B	= ter-128b.pcf ter-228b.pcf ter-528b.pcf ter-728b.pcf ter-928b.pcf ter-c28b.pcf ter-d28b.pcf ter-f28b.pcf ter-g28b.pcf ter-k28b.pcf ter-p28b.pcf ter-u28b.pcf ter-x28b.pcf
PCF_32N	= ter-132n.pcf ter-232n.pcf ter-532n.pcf ter-732n.pcf ter-932n.pcf ter-c32n.pcf ter-d32n.pcf ter-f32n.pcf ter-g32n.pcf ter-k32n.pcf ter-p32n.pcf ter-u32n.pcf ter-x32n.pcf
PCF_32B	= ter-132b.pcf ter-232b.pcf ter-532b.pcf ter-732b.pcf ter-932b.pcf ter-c32b.pcf ter-d32b.pcf ter-f32b.pcf ter-g32b.pcf ter-k32b.pcf ter-p32b.pcf ter-u32b.pcf ter-x32b.pcf
PCF	= $(PCF_12N) $(PCF_12B) $(PCF_14N) $(PCF_14B) $(PCF_16N) $(PCF_16B) $(PCF_20N) $(PCF_20B) $(PCF_22N) $(PCF_22B) $(PCF_24N) $(PCF_24B) $(PCF_28N) $(PCF_28B) $(PCF_32N) $(PCF_32B)
PCF_N12	= $(PCF_12N) $(PCF_14N) $(PCF_14B) $(PCF_16N) $(PCF_16B) $(PCF_20N) $(PCF_20B) $(PCF_22N) $(PCF_22B) $(PCF_24N) $(PCF_24B) $(PCF_28N) $(PCF_28B) $(PCF_32N) $(PCF_32B)

X_88591	= uni/x11gr.uni uni/ascii-h.uni uni/win-1252.uni
X_88592	= uni/x11gr.uni uni/ascii-h.uni uni/empty.uni uni/8859-2.uni
X_88595	= uni/x11gr.uni uni/ascii-h.uni uni/empty.uni uni/8859-5.uni
X_88597	= uni/x11gr.uni uni/ascii-h.uni uni/empty.uni uni/8859-7.uni
X_88599	= uni/x11gr.uni uni/ascii-h.uni uni/win-1254.uni
X_W1251	= uni/x11gr.uni uni/ascii-h.uni uni/x11-1251.uni uni/win-1251.uni
X_8859D	= uni/x11gr.uni uni/ascii-h.uni uni/x11-1257.uni uni/8859-13.uni
X_8859F	= uni/x11gr.uni uni/ascii-h.uni uni/empty.uni uni/8859-15.uni
X_8859G	= uni/x11gr.uni uni/ascii-h.uni uni/empty.uni uni/8859-16.uni
X_KOI8R	= uni/x11gr.uni uni/ascii-h.uni uni/koi8-r.uni
X_KOI8U	= uni/x11gr.uni uni/ascii-h.uni uni/koi8-u.uni
X_PT154	= uni/x11gr.uni uni/ascii-h.uni uni/pt-154.uni
X_10646	= uni/x11gr.uni uni/10646-1.uni

BASETARGETNAME=$(notdir $(@:%.gz=%))

$(DESTDIR)\ter-112n.pcf.gz: ter-u12n.bdf $(X_88591)
	$(UCS2ANY) ter-u12n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-112b.pcf.gz: ter-u12b.bdf $(X_88591)
	$(UCS2ANY) ter-u12b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-212n.pcf.gz: ter-u12n.bdf $(X_88592)
	$(UCS2ANY) ter-u12n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-212b.pcf.gz: ter-u12b.bdf $(X_88592)
	$(UCS2ANY) ter-u12b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-512n.pcf.gz: ter-u12n.bdf $(X_88595)
	$(UCS2ANY) ter-u12n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-512b.pcf.gz: ter-u12b.bdf $(X_88595)
	$(UCS2ANY) ter-u12b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-712n.pcf.gz: ter-u12n.bdf $(X_88597)
	$(UCS2ANY) ter-u12n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-712b.pcf.gz: ter-u12b.bdf $(X_88597)
	$(UCS2ANY) ter-u12b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-912n.pcf.gz: ter-u12n.bdf $(X_88599)
	$(UCS2ANY) ter-u12n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-912b.pcf.gz: ter-u12b.bdf $(X_88599)
	$(UCS2ANY) ter-u12b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c12n.pcf.gz: ter-u12n.bdf $(X_W1251)
	$(UCS2ANY) ter-u12n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c12b.pcf.gz: ter-u12b.bdf $(X_W1251)
	$(UCS2ANY) ter-u12b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d12n.pcf.gz: ter-u12n.bdf $(X_8859D)
	$(UCS2ANY) ter-u12n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d12b.pcf.gz: ter-u12b.bdf $(X_8859D)
	$(UCS2ANY) ter-u12b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f12n.pcf.gz: ter-u12n.bdf $(X_8859F)
	$(UCS2ANY) ter-u12n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f12b.pcf.gz: ter-u12b.bdf $(X_8859F)
	$(UCS2ANY) ter-u12b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g12n.pcf.gz: ter-u12n.bdf $(X_8859G)
	$(UCS2ANY) ter-u12n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g12b.pcf.gz: ter-u12b.bdf $(X_8859G)
	$(UCS2ANY) ter-u12b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k12n.pcf.gz: ter-u12n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u12n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k12b.pcf.gz: ter-u12b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u12b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p12n.pcf.gz: ter-u12n.bdf $(X_PT154)
	$(UCS2ANY) ter-u12n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p12b.pcf.gz: ter-u12b.bdf $(X_PT154)
	$(UCS2ANY) ter-u12b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u12n.pcf.gz: ter-u12n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u12n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u12b.pcf.gz: ter-u12b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u12b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x12n.pcf.gz: ter-u12n.bdf $(X_10646)
	$(UCS2ANY) ter-u12n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x12b.pcf.gz: ter-u12b.bdf $(X_10646)
	$(UCS2ANY) ter-u12b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-114n.pcf.gz: ter-u14n.bdf $(X_88591)
	$(UCS2ANY) ter-u14n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-114b.pcf.gz: ter-u14b.bdf $(X_88591)
	$(UCS2ANY) ter-u14b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-214n.pcf.gz: ter-u14n.bdf $(X_88592)
	$(UCS2ANY) ter-u14n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-214b.pcf.gz: ter-u14b.bdf $(X_88592)
	$(UCS2ANY) ter-u14b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-514n.pcf.gz: ter-u14n.bdf $(X_88595)
	$(UCS2ANY) ter-u14n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-514b.pcf.gz: ter-u14b.bdf $(X_88595)
	$(UCS2ANY) ter-u14b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-714n.pcf.gz: ter-u14n.bdf $(X_88597)
	$(UCS2ANY) ter-u14n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-714b.pcf.gz: ter-u14b.bdf $(X_88597)
	$(UCS2ANY) ter-u14b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-914n.pcf.gz: ter-u14n.bdf $(X_88599)
	$(UCS2ANY) ter-u14n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-914b.pcf.gz: ter-u14b.bdf $(X_88599)
	$(UCS2ANY) ter-u14b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c14n.pcf.gz: ter-u14n.bdf $(X_W1251)
	$(UCS2ANY) ter-u14n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c14b.pcf.gz: ter-u14b.bdf $(X_W1251)
	$(UCS2ANY) ter-u14b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d14n.pcf.gz: ter-u14n.bdf $(X_8859D)
	$(UCS2ANY) ter-u14n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d14b.pcf.gz: ter-u14b.bdf $(X_8859D)
	$(UCS2ANY) ter-u14b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f14n.pcf.gz: ter-u14n.bdf $(X_8859F)
	$(UCS2ANY) ter-u14n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f14b.pcf.gz: ter-u14b.bdf $(X_8859F)
	$(UCS2ANY) ter-u14b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g14n.pcf.gz: ter-u14n.bdf $(X_8859G)
	$(UCS2ANY) ter-u14n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g14b.pcf.gz: ter-u14b.bdf $(X_8859G)
	$(UCS2ANY) ter-u14b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k14n.pcf.gz: ter-u14n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u14n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k14b.pcf.gz: ter-u14b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u14b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p14n.pcf.gz: ter-u14n.bdf $(X_PT154)
	$(UCS2ANY) ter-u14n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p14b.pcf.gz: ter-u14b.bdf $(X_PT154)
	$(UCS2ANY) ter-u14b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u14n.pcf.gz: ter-u14n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u14n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u14b.pcf.gz: ter-u14b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u14b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x14n.pcf.gz: ter-u14n.bdf $(X_10646)
	$(UCS2ANY) ter-u14n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x14b.pcf.gz: ter-u14b.bdf $(X_10646)
	$(UCS2ANY) ter-u14b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-116n.pcf.gz: ter-u16n.bdf $(X_88591)
	$(UCS2ANY) ter-u16n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-116b.pcf.gz: ter-u16b.bdf $(X_88591)
	$(UCS2ANY) ter-u16b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-216n.pcf.gz: ter-u16n.bdf $(X_88592)
	$(UCS2ANY) ter-u16n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-216b.pcf.gz: ter-u16b.bdf $(X_88592)
	$(UCS2ANY) ter-u16b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-516n.pcf.gz: ter-u16n.bdf $(X_88595)
	$(UCS2ANY) ter-u16n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-516b.pcf.gz: ter-u16b.bdf $(X_88595)
	$(UCS2ANY) ter-u16b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-716n.pcf.gz: ter-u16n.bdf $(X_88597)
	$(UCS2ANY) ter-u16n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-716b.pcf.gz: ter-u16b.bdf $(X_88597)
	$(UCS2ANY) ter-u16b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-916n.pcf.gz: ter-u16n.bdf $(X_88599)
	$(UCS2ANY) ter-u16n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-916b.pcf.gz: ter-u16b.bdf $(X_88599)
	$(UCS2ANY) ter-u16b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c16n.pcf.gz: ter-u16n.bdf $(X_W1251)
	$(UCS2ANY) ter-u16n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c16b.pcf.gz: ter-u16b.bdf $(X_W1251)
	$(UCS2ANY) ter-u16b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d16n.pcf.gz: ter-u16n.bdf $(X_8859D)
	$(UCS2ANY) ter-u16n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d16b.pcf.gz: ter-u16b.bdf $(X_8859D)
	$(UCS2ANY) ter-u16b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f16n.pcf.gz: ter-u16n.bdf $(X_8859F)
	$(UCS2ANY) ter-u16n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f16b.pcf.gz: ter-u16b.bdf $(X_8859F)
	$(UCS2ANY) ter-u16b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g16n.pcf.gz: ter-u16n.bdf $(X_8859G)
	$(UCS2ANY) ter-u16n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g16b.pcf.gz: ter-u16b.bdf $(X_8859G)
	$(UCS2ANY) ter-u16b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k16n.pcf.gz: ter-u16n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u16n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k16b.pcf.gz: ter-u16b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u16b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p16n.pcf.gz: ter-u16n.bdf $(X_PT154)
	$(UCS2ANY) ter-u16n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p16b.pcf.gz: ter-u16b.bdf $(X_PT154)
	$(UCS2ANY) ter-u16b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u16n.pcf.gz: ter-u16n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u16n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u16b.pcf.gz: ter-u16b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u16b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x16n.pcf.gz: ter-u16n.bdf $(X_10646)
	$(UCS2ANY) ter-u16n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x16b.pcf.gz: ter-u16b.bdf $(X_10646)
	$(UCS2ANY) ter-u16b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-120n.pcf.gz: ter-u20n.bdf $(X_88591)
	$(UCS2ANY) ter-u20n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-120b.pcf.gz: ter-u20b.bdf $(X_88591)
	$(UCS2ANY) ter-u20b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-220n.pcf.gz: ter-u20n.bdf $(X_88592)
	$(UCS2ANY) ter-u20n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-220b.pcf.gz: ter-u20b.bdf $(X_88592)
	$(UCS2ANY) ter-u20b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-520n.pcf.gz: ter-u20n.bdf $(X_88595)
	$(UCS2ANY) ter-u20n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-520b.pcf.gz: ter-u20b.bdf $(X_88595)
	$(UCS2ANY) ter-u20b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-720n.pcf.gz: ter-u20n.bdf $(X_88597)
	$(UCS2ANY) ter-u20n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-720b.pcf.gz: ter-u20b.bdf $(X_88597)
	$(UCS2ANY) ter-u20b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-920n.pcf.gz: ter-u20n.bdf $(X_88599)
	$(UCS2ANY) ter-u20n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-920b.pcf.gz: ter-u20b.bdf $(X_88599)
	$(UCS2ANY) ter-u20b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c20n.pcf.gz: ter-u20n.bdf $(X_W1251)
	$(UCS2ANY) ter-u20n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c20b.pcf.gz: ter-u20b.bdf $(X_W1251)
	$(UCS2ANY) ter-u20b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d20n.pcf.gz: ter-u20n.bdf $(X_8859D)
	$(UCS2ANY) ter-u20n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d20b.pcf.gz: ter-u20b.bdf $(X_8859D)
	$(UCS2ANY) ter-u20b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f20n.pcf.gz: ter-u20n.bdf $(X_8859F)
	$(UCS2ANY) ter-u20n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f20b.pcf.gz: ter-u20b.bdf $(X_8859F)
	$(UCS2ANY) ter-u20b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g20n.pcf.gz: ter-u20n.bdf $(X_8859G)
	$(UCS2ANY) ter-u20n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g20b.pcf.gz: ter-u20b.bdf $(X_8859G)
	$(UCS2ANY) ter-u20b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k20n.pcf.gz: ter-u20n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u20n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k20b.pcf.gz: ter-u20b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u20b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p20n.pcf.gz: ter-u20n.bdf $(X_PT154)
	$(UCS2ANY) ter-u20n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p20b.pcf.gz: ter-u20b.bdf $(X_PT154)
	$(UCS2ANY) ter-u20b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u20n.pcf.gz: ter-u20n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u20n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u20b.pcf.gz: ter-u20b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u20b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x20n.pcf.gz: ter-u20n.bdf $(X_10646)
	$(UCS2ANY) ter-u20n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x20b.pcf.gz: ter-u20b.bdf $(X_10646)
	$(UCS2ANY) ter-u20b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-122n.pcf.gz: ter-u22n.bdf $(X_88591)
	$(UCS2ANY) ter-u22n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-122b.pcf.gz: ter-u22b.bdf $(X_88591)
	$(UCS2ANY) ter-u22b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-222n.pcf.gz: ter-u22n.bdf $(X_88592)
	$(UCS2ANY) ter-u22n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-222b.pcf.gz: ter-u22b.bdf $(X_88592)
	$(UCS2ANY) ter-u22b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-522n.pcf.gz: ter-u22n.bdf $(X_88595)
	$(UCS2ANY) ter-u22n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-522b.pcf.gz: ter-u22b.bdf $(X_88595)
	$(UCS2ANY) ter-u22b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-722n.pcf.gz: ter-u22n.bdf $(X_88597)
	$(UCS2ANY) ter-u22n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-722b.pcf.gz: ter-u22b.bdf $(X_88597)
	$(UCS2ANY) ter-u22b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-922n.pcf.gz: ter-u22n.bdf $(X_88599)
	$(UCS2ANY) ter-u22n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-922b.pcf.gz: ter-u22b.bdf $(X_88599)
	$(UCS2ANY) ter-u22b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c22n.pcf.gz: ter-u22n.bdf $(X_W1251)
	$(UCS2ANY) ter-u22n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c22b.pcf.gz: ter-u22b.bdf $(X_W1251)
	$(UCS2ANY) ter-u22b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d22n.pcf.gz: ter-u22n.bdf $(X_8859D)
	$(UCS2ANY) ter-u22n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d22b.pcf.gz: ter-u22b.bdf $(X_8859D)
	$(UCS2ANY) ter-u22b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f22n.pcf.gz: ter-u22n.bdf $(X_8859F)
	$(UCS2ANY) ter-u22n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f22b.pcf.gz: ter-u22b.bdf $(X_8859F)
	$(UCS2ANY) ter-u22b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g22n.pcf.gz: ter-u22n.bdf $(X_8859G)
	$(UCS2ANY) ter-u22n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g22b.pcf.gz: ter-u22b.bdf $(X_8859G)
	$(UCS2ANY) ter-u22b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k22n.pcf.gz: ter-u22n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u22n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k22b.pcf.gz: ter-u22b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u22b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p22n.pcf.gz: ter-u22n.bdf $(X_PT154)
	$(UCS2ANY) ter-u22n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p22b.pcf.gz: ter-u22b.bdf $(X_PT154)
	$(UCS2ANY) ter-u22b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u22n.pcf.gz: ter-u22n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u22n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u22b.pcf.gz: ter-u22b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u22b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x22n.pcf.gz: ter-u22n.bdf $(X_10646)
	$(UCS2ANY) ter-u22n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x22b.pcf.gz: ter-u22b.bdf $(X_10646)
	$(UCS2ANY) ter-u22b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-124n.pcf.gz: ter-u24n.bdf $(X_88591)
	$(UCS2ANY) ter-u24n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-124b.pcf.gz: ter-u24b.bdf $(X_88591)
	$(UCS2ANY) ter-u24b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-224n.pcf.gz: ter-u24n.bdf $(X_88592)
	$(UCS2ANY) ter-u24n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-224b.pcf.gz: ter-u24b.bdf $(X_88592)
	$(UCS2ANY) ter-u24b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-524n.pcf.gz: ter-u24n.bdf $(X_88595)
	$(UCS2ANY) ter-u24n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-524b.pcf.gz: ter-u24b.bdf $(X_88595)
	$(UCS2ANY) ter-u24b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-724n.pcf.gz: ter-u24n.bdf $(X_88597)
	$(UCS2ANY) ter-u24n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-724b.pcf.gz: ter-u24b.bdf $(X_88597)
	$(UCS2ANY) ter-u24b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-924n.pcf.gz: ter-u24n.bdf $(X_88599)
	$(UCS2ANY) ter-u24n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-924b.pcf.gz: ter-u24b.bdf $(X_88599)
	$(UCS2ANY) ter-u24b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c24n.pcf.gz: ter-u24n.bdf $(X_W1251)
	$(UCS2ANY) ter-u24n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c24b.pcf.gz: ter-u24b.bdf $(X_W1251)
	$(UCS2ANY) ter-u24b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d24n.pcf.gz: ter-u24n.bdf $(X_8859D)
	$(UCS2ANY) ter-u24n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d24b.pcf.gz: ter-u24b.bdf $(X_8859D)
	$(UCS2ANY) ter-u24b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f24n.pcf.gz: ter-u24n.bdf $(X_8859F)
	$(UCS2ANY) ter-u24n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f24b.pcf.gz: ter-u24b.bdf $(X_8859F)
	$(UCS2ANY) ter-u24b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g24n.pcf.gz: ter-u24n.bdf $(X_8859G)
	$(UCS2ANY) ter-u24n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g24b.pcf.gz: ter-u24b.bdf $(X_8859G)
	$(UCS2ANY) ter-u24b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k24n.pcf.gz: ter-u24n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u24n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k24b.pcf.gz: ter-u24b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u24b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p24n.pcf.gz: ter-u24n.bdf $(X_PT154)
	$(UCS2ANY) ter-u24n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p24b.pcf.gz: ter-u24b.bdf $(X_PT154)
	$(UCS2ANY) ter-u24b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u24n.pcf.gz: ter-u24n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u24n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u24b.pcf.gz: ter-u24b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u24b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x24n.pcf.gz: ter-u24n.bdf $(X_10646)
	$(UCS2ANY) ter-u24n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x24b.pcf.gz: ter-u24b.bdf $(X_10646)
	$(UCS2ANY) ter-u24b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-128n.pcf.gz: ter-u28n.bdf $(X_88591)
	$(UCS2ANY) ter-u28n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-128b.pcf.gz: ter-u28b.bdf $(X_88591)
	$(UCS2ANY) ter-u28b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-228n.pcf.gz: ter-u28n.bdf $(X_88592)
	$(UCS2ANY) ter-u28n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-228b.pcf.gz: ter-u28b.bdf $(X_88592)
	$(UCS2ANY) ter-u28b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-528n.pcf.gz: ter-u28n.bdf $(X_88595)
	$(UCS2ANY) ter-u28n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-528b.pcf.gz: ter-u28b.bdf $(X_88595)
	$(UCS2ANY) ter-u28b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-728n.pcf.gz: ter-u28n.bdf $(X_88597)
	$(UCS2ANY) ter-u28n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-728b.pcf.gz: ter-u28b.bdf $(X_88597)
	$(UCS2ANY) ter-u28b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-928n.pcf.gz: ter-u28n.bdf $(X_88599)
	$(UCS2ANY) ter-u28n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-928b.pcf.gz: ter-u28b.bdf $(X_88599)
	$(UCS2ANY) ter-u28b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c28n.pcf.gz: ter-u28n.bdf $(X_W1251)
	$(UCS2ANY) ter-u28n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c28b.pcf.gz: ter-u28b.bdf $(X_W1251)
	$(UCS2ANY) ter-u28b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d28n.pcf.gz: ter-u28n.bdf $(X_8859D)
	$(UCS2ANY) ter-u28n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d28b.pcf.gz: ter-u28b.bdf $(X_8859D)
	$(UCS2ANY) ter-u28b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f28n.pcf.gz: ter-u28n.bdf $(X_8859F)
	$(UCS2ANY) ter-u28n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f28b.pcf.gz: ter-u28b.bdf $(X_8859F)
	$(UCS2ANY) ter-u28b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g28n.pcf.gz: ter-u28n.bdf $(X_8859G)
	$(UCS2ANY) ter-u28n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g28b.pcf.gz: ter-u28b.bdf $(X_8859G)
	$(UCS2ANY) ter-u28b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k28n.pcf.gz: ter-u28n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u28n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k28b.pcf.gz: ter-u28b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u28b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p28n.pcf.gz: ter-u28n.bdf $(X_PT154)
	$(UCS2ANY) ter-u28n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p28b.pcf.gz: ter-u28b.bdf $(X_PT154)
	$(UCS2ANY) ter-u28b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u28n.pcf.gz: ter-u28n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u28n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u28b.pcf.gz: ter-u28b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u28b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x28n.pcf.gz: ter-u28n.bdf $(X_10646)
	$(UCS2ANY) ter-u28n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x28b.pcf.gz: ter-u28b.bdf $(X_10646)
	$(UCS2ANY) ter-u28b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-132n.pcf.gz: ter-u32n.bdf $(X_88591)
	$(UCS2ANY) ter-u32n.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-132b.pcf.gz: ter-u32b.bdf $(X_88591)
	$(UCS2ANY) ter-u32b.bdf $(R_88591) $(X_88591) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-232n.pcf.gz: ter-u32n.bdf $(X_88592)
	$(UCS2ANY) ter-u32n.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-232b.pcf.gz: ter-u32b.bdf $(X_88592)
	$(UCS2ANY) ter-u32b.bdf $(R_88592) $(X_88592) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-532n.pcf.gz: ter-u32n.bdf $(X_88595)
	$(UCS2ANY) ter-u32n.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-532b.pcf.gz: ter-u32b.bdf $(X_88595)
	$(UCS2ANY) ter-u32b.bdf $(R_88595) $(X_88595) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-732n.pcf.gz: ter-u32n.bdf $(X_88597)
	$(UCS2ANY) ter-u32n.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-732b.pcf.gz: ter-u32b.bdf $(X_88597)
	$(UCS2ANY) ter-u32b.bdf $(R_88597) $(X_88597) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-932n.pcf.gz: ter-u32n.bdf $(X_88599)
	$(UCS2ANY) ter-u32n.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-932b.pcf.gz: ter-u32b.bdf $(X_88599)
	$(UCS2ANY) ter-u32b.bdf $(R_88599) $(X_88599) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c32n.pcf.gz: ter-u32n.bdf $(X_W1251)
	$(UCS2ANY) ter-u32n.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-c32b.pcf.gz: ter-u32b.bdf $(X_W1251)
	$(UCS2ANY) ter-u32b.bdf $(R_W1251) $(X_W1251) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d32n.pcf.gz: ter-u32n.bdf $(X_8859D)
	$(UCS2ANY) ter-u32n.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-d32b.pcf.gz: ter-u32b.bdf $(X_8859D)
	$(UCS2ANY) ter-u32b.bdf $(R_8859D) $(X_8859D) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f32n.pcf.gz: ter-u32n.bdf $(X_8859F)
	$(UCS2ANY) ter-u32n.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-f32b.pcf.gz: ter-u32b.bdf $(X_8859F)
	$(UCS2ANY) ter-u32b.bdf $(R_8859F) $(X_8859F) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g32n.pcf.gz: ter-u32n.bdf $(X_8859G)
	$(UCS2ANY) ter-u32n.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-g32b.pcf.gz: ter-u32b.bdf $(X_8859G)
	$(UCS2ANY) ter-u32b.bdf $(R_8859G) $(X_8859G) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k32n.pcf.gz: ter-u32n.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u32n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-k32b.pcf.gz: ter-u32b.bdf $(X_KOI8R)
	$(UCS2ANY) ter-u32b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p32n.pcf.gz: ter-u32n.bdf $(X_PT154)
	$(UCS2ANY) ter-u32n.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-p32b.pcf.gz: ter-u32b.bdf $(X_PT154)
	$(UCS2ANY) ter-u32b.bdf $(R_PT154) $(X_PT154) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u32n.pcf.gz: ter-u32n.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u32n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-u32b.pcf.gz: ter-u32b.bdf $(X_KOI8U)
	$(UCS2ANY) ter-u32b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x32n.pcf.gz: ter-u32n.bdf $(X_10646)
	$(UCS2ANY) ter-u32n.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)

$(DESTDIR)\ter-x32b.pcf.gz: ter-u32b.bdf $(X_10646)
	$(UCS2ANY) ter-u32b.bdf $(R_10646) $(X_10646) | $(BDFTOPCF) -t -o $(BASETARGETNAME)
	gzip -c $(BASETARGETNAME) > $@
	del $(BASETARGETNAME)


font_DATA = $(PCF:%=$(DESTDIR)\%.gz) $(PCF_N12:%=$(DESTDIR)\%.gz)

destdir: $(DESTDIR)

all: $(DESTDIR) $(DESTDIR)\fonts.dir $(DESTDIR)\fonts.alias

MKFONTSCALE=$(MHMAKECONF)\mkfontscale\$(OBJDIR)\mkfontscale.exe

load_makefile $(MHMAKECONF)\mkfontscale\makefile MAKESERVER=0 DEBUG=$(DEBUG)

$(DESTDIR)\fonts.scale: $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) .

$(DESTDIR)\fonts.dir: $(DESTDIR)\fonts.scale $(font_DATA) $(MKFONTSCALE)
	del -e $@
	cd $(DESTDIR) & $(MKFONTSCALE) -b -s -l .

$(DESTDIR)\fonts.alias: xfonts-terminus.alias
	copy $< $@

