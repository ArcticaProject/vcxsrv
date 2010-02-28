PERL	= perl
AWK	= awk
UCS2VGA	= $(PERL) ./ucstoany.pl +f
UNI2TXT	= $(AWK) '{ printf("0x%02X\tU+%s\n", FNR-1, $$1) }'
BDF2PSF	= $(PERL) ./bdftopsf.pl +u
BDF2RAW	= $(PERL) ./bdftopsf.pl -h
UCS2X11	= $(PERL) ./ucstoany.pl
BDF2PCF	= bdftopcf

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

PSF_12N	= ter-112n.psf ter-212n.psf ter-712n.psf ter-912n.psf ter-c12n.psf ter-d12n.psf ter-g12n.psf ter-i12n.psf ter-k12n.psf ter-m12n.psf ter-p12n.psf ter-u12n.psf ter-v12n.psf
PSF_14N	= ter-114n.psf ter-214n.psf ter-714n.psf ter-914n.psf ter-c14n.psf ter-d14n.psf ter-g14n.psf ter-i14n.psf ter-k14n.psf ter-m14n.psf ter-p14n.psf ter-u14n.psf ter-v14n.psf
PSF_14B	= ter-114b.psf ter-214b.psf ter-714b.psf ter-914b.psf ter-c14b.psf ter-d14b.psf ter-g14b.psf ter-i14b.psf ter-k14b.psf ter-m14b.psf ter-p14b.psf ter-u14b.psf ter-v14b.psf
PSF_14F	= ter-114f.psf ter-214f.psf ter-714f.psf ter-914f.psf ter-c14f.psf ter-d14f.psf ter-g14f.psf ter-i14f.psf ter-k14f.psf ter-m14f.psf ter-p14f.psf ter-u14f.psf ter-v14f.psf
PSF_16N	= ter-116n.psf ter-216n.psf ter-716n.psf ter-916n.psf ter-c16n.psf ter-d16n.psf ter-g16n.psf ter-i16n.psf ter-k16n.psf ter-m16n.psf ter-p16n.psf ter-u16n.psf ter-v16n.psf
PSF_16B	= ter-116b.psf ter-216b.psf ter-716b.psf ter-916b.psf ter-c16b.psf ter-d16b.psf ter-g16b.psf ter-i16b.psf ter-k16b.psf ter-m16b.psf ter-p16b.psf ter-u16b.psf ter-v16b.psf
PSF_16F	= ter-116f.psf ter-216f.psf ter-716f.psf ter-916f.psf ter-c16f.psf ter-d16f.psf ter-g16f.psf ter-i16f.psf ter-k16f.psf ter-m16f.psf ter-p16f.psf ter-u16f.psf ter-v16f.psf
PSF_20N	= ter-120n.psf ter-220n.psf ter-720n.psf ter-920n.psf ter-c20n.psf ter-d20n.psf ter-g20n.psf ter-i20n.psf ter-k20n.psf ter-m20n.psf ter-p20n.psf ter-u20n.psf ter-v20n.psf
PSF_20B	= ter-120b.psf ter-220b.psf ter-720b.psf ter-920b.psf ter-c20b.psf ter-d20b.psf ter-g20b.psf ter-i20b.psf ter-k20b.psf ter-m20b.psf ter-p20b.psf ter-u20b.psf ter-v20b.psf
PSF_22N	= ter-122n.psf ter-222n.psf ter-722n.psf ter-922n.psf ter-c22n.psf ter-d22n.psf ter-g22n.psf ter-i22n.psf ter-k22n.psf ter-m22n.psf ter-p22n.psf ter-u22n.psf ter-v22n.psf
PSF_22B	= ter-122b.psf ter-222b.psf ter-722b.psf ter-922b.psf ter-c22b.psf ter-d22b.psf ter-g22b.psf ter-i22b.psf ter-k22b.psf ter-m22b.psf ter-p22b.psf ter-u22b.psf ter-v22b.psf
PSF_24N	= ter-124n.psf ter-224n.psf ter-724n.psf ter-924n.psf ter-c24n.psf ter-d24n.psf ter-g24n.psf ter-i24n.psf ter-k24n.psf ter-m24n.psf ter-p24n.psf ter-u24n.psf ter-v24n.psf
PSF_24B	= ter-124b.psf ter-224b.psf ter-724b.psf ter-924b.psf ter-c24b.psf ter-d24b.psf ter-g24b.psf ter-i24b.psf ter-k24b.psf ter-m24b.psf ter-p24b.psf ter-u24b.psf ter-v24b.psf
PSF_28N	= ter-128n.psf ter-228n.psf ter-728n.psf ter-928n.psf ter-c28n.psf ter-d28n.psf ter-g28n.psf ter-i28n.psf ter-k28n.psf ter-m28n.psf ter-p28n.psf ter-u28n.psf ter-v28n.psf
PSF_28B	= ter-128b.psf ter-228b.psf ter-728b.psf ter-928b.psf ter-c28b.psf ter-d28b.psf ter-g28b.psf ter-i28b.psf ter-k28b.psf ter-m28b.psf ter-p28b.psf ter-u28b.psf ter-v28b.psf
PSF_32N	= ter-132n.psf ter-232n.psf ter-732n.psf ter-932n.psf ter-c32n.psf ter-d32n.psf ter-g32n.psf ter-i32n.psf ter-k32n.psf ter-m32n.psf ter-p32n.psf ter-u32n.psf ter-v32n.psf
PSF_32B	= ter-132b.psf ter-232b.psf ter-732b.psf ter-932b.psf ter-c32b.psf ter-d32b.psf ter-g32b.psf ter-i32b.psf ter-k32b.psf ter-m32b.psf ter-p32b.psf ter-u32b.psf ter-v32b.psf
PSF	= $(PSF_12N) $(PSF_14N) $(PSF_14B) $(PSF_14F) $(PSF_16N) $(PSF_16B) $(PSF_16F) $(PSF_20N) $(PSF_20B) $(PSF_22N) $(PSF_22B) $(PSF_24N) $(PSF_24B) $(PSF_28N) $(PSF_28B) $(PSF_32N) $(PSF_32B)

TXT_WIN	= cp1252.txt cp1250.txt cp1253.txt cp1254.txt cp1251.txt cp1257.txt pt154.txt
TXT_MIX	= 8859-16.txt cp437.txt koi8-r.txt koi8-u.txt mik.txt pt254.txt
TXT	= $(TXT_WIN) $(TXT_MIX)

RAW_14N	= ter-114n.raw ter-214n.raw ter-514n.raw ter-714n.raw ter-914n.raw ter-c14n.raw ter-d14n.raw ter-f14n.raw ter-g14n.raw ter-i14n.raw ter-k14n.raw ter-p14n.raw ter-u14n.raw
RAW_14B	= ter-114b.raw ter-214b.raw ter-514b.raw ter-714b.raw ter-914b.raw ter-c14b.raw ter-d14b.raw ter-f14b.raw ter-g14b.raw ter-i14b.raw ter-k14b.raw ter-p14b.raw ter-u14b.raw
RAW_14F	= ter-114f.raw ter-214f.raw ter-514f.raw ter-714f.raw ter-914f.raw ter-c14f.raw ter-d14f.raw ter-f14f.raw ter-g14f.raw ter-i14f.raw ter-k14f.raw ter-p14f.raw ter-u14f.raw
RAW_16N	= ter-116n.raw ter-216n.raw ter-516n.raw ter-716n.raw ter-916n.raw ter-c16n.raw ter-d16n.raw ter-f16n.raw ter-g16n.raw ter-i16n.raw ter-k16n.raw ter-p16n.raw ter-u16n.raw
RAW_16B	= ter-116b.raw ter-216b.raw ter-516b.raw ter-716b.raw ter-916b.raw ter-c16b.raw ter-d16b.raw ter-f16b.raw ter-g16b.raw ter-i16b.raw ter-k16b.raw ter-p16b.raw ter-u16b.raw
RAW_16F	= ter-116f.raw ter-216f.raw ter-516f.raw ter-716f.raw ter-916f.raw ter-c16f.raw ter-d16f.raw ter-f16f.raw ter-g16f.raw ter-i16f.raw ter-k16f.raw ter-p16f.raw ter-u16f.raw
RAW	= $(RAW_14N) $(RAW_14B) $(RAW_14F) $(RAW_16N) $(RAW_16B) $(RAW_16F)

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

EEE	= ter-x12n.pcf ter-x12b.pcf ter-x14n.pcf ter-x14b.pcf ter-x16n.pcf ter-x16b.pcf ter-x20n.pcf ter-x20b.pcf ter-x22n.pcf ter-x22b.pcf
EEE_N12	= ter-x12n.pcf ter-x14n.pcf ter-x14b.pcf ter-x16n.pcf ter-x16b.pcf ter-x20n.pcf ter-x20b.pcf ter-x22n.pcf ter-x22b.pcf

all: $(PSF) $(PCF)
psf: $(PSF)
txt: $(TXT)
raw: $(RAW)
pcf: $(PCF)
n12: $(PCF_N12)
eee: $(EEE)
e12: $(EEE_N12)

DESTDIR	=
prefix	= /usr/local
psfdir	= $(prefix)/share/consolefonts
x11dir	= $(prefix)/share/fonts/terminus

install: $(PSF) $(PCF)
	mkdir -p $(DESTDIR)$(psfdir)
	for i in $(PSF) ; do gzip -c $$i > $(DESTDIR)$(psfdir)/$$i.gz ; done
	mkdir -p $(DESTDIR)$(x11dir)
	for i in $(PCF) ; do gzip -c $$i > $(DESTDIR)$(x11dir)/$$i.gz ; done

uninstall:
	for i in $(PSF) ; do rm -f $(DESTDIR)$(psfdir)/$$i.gz ; done
	for i in $(PCF) ; do rm -f $(DESTDIR)$(x11dir)/$$i.gz ; done

fontdir:
	mkfontscale $(DESTDIR)$(x11dir)
	mkfontdir $(DESTDIR)$(x11dir)
	fc-cache $(DESTDIR)$(x11dir)

V_88591	= uni/vgagr.uni uni/ascii-h.uni uni/win-1252.uni
V_88592	= uni/vgagr.uni uni/ascii-h.uni uni/vga-1250.uni uni/8859-2.uni
V_88597	= uni/vgagr.uni uni/ascii-h.uni uni/vga-1253.uni uni/8859-7.uni
V_88599	= uni/vgagr.uni uni/ascii-h.uni uni/win-1254.uni
V_W1251	= uni/vgagr.uni uni/ascii-h.uni uni/vga-1251.uni uni/win-1251.uni
V_8859D	= uni/vgagr.uni uni/ascii-h.uni uni/vga-1257.uni uni/8859-13.uni
V_8859G	= uni/vgagr.uni uni/ascii-h.uni uni/nls-1250.uni uni/8859-16.uni
V_PCIBM	= uni/cntrl.uni uni/ascii-h.uni uni/ibm-437.uni
V_KOI8R	= uni/cntrl.uni uni/ascii-h.uni uni/koibm8-r.uni
F_KOI8R	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-r.uni
V_KOI8U	= uni/cntrl.uni uni/ascii-h.uni uni/koibm8-u.uni
F_KOI8U	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-u.uni
V_BGMIK	= uni/cntrl.uni uni/ascii-h.uni uni/bg-mik.uni
V_PT154	= uni/vgagr.uni uni/ascii-h.uni uni/pt-154.uni
V_XOS42	= uni/xos4-2.uni

D_88591	= dup/vgagr.dup dup/ascii-h.dup
D_88592	= dup/vgagr.dup dup/ascii-h.dup
D_88597	= dup/vgagr.dup dup/ascii-h.dup
D_88599	= dup/vgagr.dup dup/ascii-h.dup
D_W1251	= dup/vgagr.dup dup/ascii-h.dup
D_8859D	= dup/vgagr.dup dup/ascii-h.dup
D_8859G	= dup/vgagr.dup dup/ascii-h.dup
D_PCIBM	= dup/cntrl.dup dup/ascii-h.dup dup/ibm-437.dup
D_KOI8R	= dup/cntrl.dup dup/ascii-h.dup dup/koi8.dup
D_KOI8U	= dup/cntrl.dup dup/ascii-h.dup dup/koi8.dup
D_BGMIK	= dup/cntrl.dup dup/ascii-h.dup dup/ibm-437.dup
D_PT154	= dup/vgagr.dup dup/ascii-h.dup
D_XOS42	= dup/vgagr.dup dup/xos4-2.dup

ter-112n.psf: ter-u12n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u12n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-212n.psf: ter-u12n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u12n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-712n.psf: ter-u12n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u12n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-912n.psf: ter-u12n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u12n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c12n.psf: ter-u12n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u12n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d12n.psf: ter-u12n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u12n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g12n.psf: ter-u12n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u12n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i12n.psf: ter-u12n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u12n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k12n.psf: ter-u12n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u12n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m12n.psf: ter-u12n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u12n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p12n.psf: ter-u12n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u12n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u12n.psf: ter-u12n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u12n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v12n.psf: ter-u12n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u12n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-114n.psf: ter-u14n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u14n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-114b.psf: ter-u14v.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u14v.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-114f.psf: ter-u14b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u14b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-214n.psf: ter-u14n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u14n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-214b.psf: ter-u14v.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u14v.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-214f.psf: ter-u14b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u14b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-714n.psf: ter-u14n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u14n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-714b.psf: ter-u14v.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u14v.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-714f.psf: ter-u14b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u14b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-914n.psf: ter-u14n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u14n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-914b.psf: ter-u14v.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u14v.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-914f.psf: ter-u14b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u14b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c14n.psf: ter-u14n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u14n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c14b.psf: ter-u14v.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u14v.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c14f.psf: ter-u14b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u14b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d14n.psf: ter-u14n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u14n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d14b.psf: ter-u14v.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u14v.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d14f.psf: ter-u14b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u14b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g14n.psf: ter-u14n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u14n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g14b.psf: ter-u14v.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u14v.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g14f.psf: ter-u14b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u14b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i14n.psf: ter-u14n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u14n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i14b.psf: ter-u14v.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u14v.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i14f.psf: ter-u14b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u14b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k14n.psf: ter-u14n.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u14n.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k14b.psf: ter-u14v.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u14v.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k14f.psf: ter-u14b.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u14b.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m14n.psf: ter-u14n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u14n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m14b.psf: ter-u14v.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u14v.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m14f.psf: ter-u14b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u14b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p14n.psf: ter-u14n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u14n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p14b.psf: ter-u14v.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u14v.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p14f.psf: ter-u14b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u14b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u14n.psf: ter-u14n.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u14n.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u14b.psf: ter-u14v.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u14v.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u14f.psf: ter-u14b.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u14b.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v14n.psf: ter-u14n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u14n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v14b.psf: ter-u14v.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u14v.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v14f.psf: ter-u14b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u14b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-116n.psf: ter-u16n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u16n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-116b.psf: ter-u16v.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u16v.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-116f.psf: ter-u16b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u16b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-216n.psf: ter-u16n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u16n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-216b.psf: ter-u16v.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u16v.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-216f.psf: ter-u16b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u16b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-716n.psf: ter-u16n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u16n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-716b.psf: ter-u16v.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u16v.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-716f.psf: ter-u16b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u16b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-916n.psf: ter-u16n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u16n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-916b.psf: ter-u16v.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u16v.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-916f.psf: ter-u16b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u16b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c16n.psf: ter-u16n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u16n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c16b.psf: ter-u16v.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u16v.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c16f.psf: ter-u16b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u16b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d16n.psf: ter-u16n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u16n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d16b.psf: ter-u16v.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u16v.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d16f.psf: ter-u16b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u16b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g16n.psf: ter-u16n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u16n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g16b.psf: ter-u16v.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u16v.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g16f.psf: ter-u16b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u16b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i16n.psf: ter-u16n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u16n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i16b.psf: ter-u16v.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u16v.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i16f.psf: ter-u16b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u16b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k16n.psf: ter-u16n.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u16n.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k16b.psf: ter-u16v.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u16v.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k16f.psf: ter-u16b.bdf $(V_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u16b.bdf $(R_KOI8R) $(V_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m16n.psf: ter-u16n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u16n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m16b.psf: ter-u16v.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u16v.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m16f.psf: ter-u16b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u16b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p16n.psf: ter-u16n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u16n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p16b.psf: ter-u16v.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u16v.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p16f.psf: ter-u16b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u16b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u16n.psf: ter-u16n.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u16n.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u16b.psf: ter-u16v.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u16v.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u16f.psf: ter-u16b.bdf $(V_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u16b.bdf $(R_KOI8U) $(V_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v16n.psf: ter-u16n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u16n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v16b.psf: ter-u16v.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u16v.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v16f.psf: ter-u16b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u16b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-120n.psf: ter-u20n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u20n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-120b.psf: ter-u20b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u20b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-220n.psf: ter-u20n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u20n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-220b.psf: ter-u20b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u20b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-720n.psf: ter-u20n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u20n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-720b.psf: ter-u20b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u20b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-920n.psf: ter-u20n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u20n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-920b.psf: ter-u20b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u20b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c20n.psf: ter-u20n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u20n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c20b.psf: ter-u20b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u20b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d20n.psf: ter-u20n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u20n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d20b.psf: ter-u20b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u20b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g20n.psf: ter-u20n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u20n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g20b.psf: ter-u20b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u20b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i20n.psf: ter-u20n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u20n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i20b.psf: ter-u20b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u20b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k20n.psf: ter-u20n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u20n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k20b.psf: ter-u20b.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u20b.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m20n.psf: ter-u20n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u20n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m20b.psf: ter-u20b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u20b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p20n.psf: ter-u20n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u20n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p20b.psf: ter-u20b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u20b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u20n.psf: ter-u20n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u20n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u20b.psf: ter-u20b.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u20b.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v20n.psf: ter-u20n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u20n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v20b.psf: ter-u20b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u20b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-122n.psf: ter-u22n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u22n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-122b.psf: ter-u22b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u22b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-222n.psf: ter-u22n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u22n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-222b.psf: ter-u22b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u22b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-722n.psf: ter-u22n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u22n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-722b.psf: ter-u22b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u22b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-922n.psf: ter-u22n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u22n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-922b.psf: ter-u22b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u22b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c22n.psf: ter-u22n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u22n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c22b.psf: ter-u22b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u22b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d22n.psf: ter-u22n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u22n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d22b.psf: ter-u22b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u22b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g22n.psf: ter-u22n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u22n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g22b.psf: ter-u22b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u22b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i22n.psf: ter-u22n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u22n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i22b.psf: ter-u22b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u22b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k22n.psf: ter-u22n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u22n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k22b.psf: ter-u22b.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u22b.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m22n.psf: ter-u22n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u22n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m22b.psf: ter-u22b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u22b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p22n.psf: ter-u22n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u22n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p22b.psf: ter-u22b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u22b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u22n.psf: ter-u22n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u22n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u22b.psf: ter-u22b.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u22b.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v22n.psf: ter-u22n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u22n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v22b.psf: ter-u22b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u22b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-124n.psf: ter-u24n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u24n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-124b.psf: ter-u24b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u24b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-224n.psf: ter-u24n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u24n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-224b.psf: ter-u24b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u24b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-724n.psf: ter-u24n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u24n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-724b.psf: ter-u24b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u24b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-924n.psf: ter-u24n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u24n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-924b.psf: ter-u24b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u24b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c24n.psf: ter-u24n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u24n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c24b.psf: ter-u24b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u24b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d24n.psf: ter-u24n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u24n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d24b.psf: ter-u24b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u24b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g24n.psf: ter-u24n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u24n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g24b.psf: ter-u24b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u24b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i24n.psf: ter-u24n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u24n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i24b.psf: ter-u24b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u24b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k24n.psf: ter-u24n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u24n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k24b.psf: ter-u24b.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u24b.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m24n.psf: ter-u24n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u24n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m24b.psf: ter-u24b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u24b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p24n.psf: ter-u24n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u24n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p24b.psf: ter-u24b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u24b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u24n.psf: ter-u24n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u24n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u24b.psf: ter-u24b.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u24b.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v24n.psf: ter-u24n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u24n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v24b.psf: ter-u24b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u24b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-128n.psf: ter-u28n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u28n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-128b.psf: ter-u28b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u28b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-228n.psf: ter-u28n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u28n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-228b.psf: ter-u28b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u28b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-728n.psf: ter-u28n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u28n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-728b.psf: ter-u28b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u28b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-928n.psf: ter-u28n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u28n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-928b.psf: ter-u28b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u28b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c28n.psf: ter-u28n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u28n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c28b.psf: ter-u28b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u28b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d28n.psf: ter-u28n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u28n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d28b.psf: ter-u28b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u28b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g28n.psf: ter-u28n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u28n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g28b.psf: ter-u28b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u28b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i28n.psf: ter-u28n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u28n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i28b.psf: ter-u28b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u28b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k28n.psf: ter-u28n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u28n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k28b.psf: ter-u28b.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u28b.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m28n.psf: ter-u28n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u28n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m28b.psf: ter-u28b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u28b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p28n.psf: ter-u28n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u28n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p28b.psf: ter-u28b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u28b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u28n.psf: ter-u28n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u28n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u28b.psf: ter-u28b.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u28b.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v28n.psf: ter-u28n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u28n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v28b.psf: ter-u28b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u28b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-132n.psf: ter-u32n.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u32n.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-132b.psf: ter-u32b.bdf $(V_88591) $(D_88591)
	$(UCS2VGA) ter-u32b.bdf $(R_88591) $(V_88591) | $(BDF2PSF) -o $@ - $(D_88591)

ter-232n.psf: ter-u32n.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u32n.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-232b.psf: ter-u32b.bdf $(V_88592) $(D_88592)
	$(UCS2VGA) ter-u32b.bdf $(R_88592) $(V_88592) | $(BDF2PSF) -o $@ - $(D_88592)

ter-732n.psf: ter-u32n.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u32n.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-732b.psf: ter-u32b.bdf $(V_88597) $(D_88597)
	$(UCS2VGA) ter-u32b.bdf $(R_88597) $(V_88597) | $(BDF2PSF) -o $@ - $(D_88597)

ter-932n.psf: ter-u32n.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u32n.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-932b.psf: ter-u32b.bdf $(V_88599) $(D_88599)
	$(UCS2VGA) ter-u32b.bdf $(R_88599) $(V_88599) | $(BDF2PSF) -o $@ - $(D_88599)

ter-c32n.psf: ter-u32n.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u32n.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-c32b.psf: ter-u32b.bdf $(V_W1251) $(D_W1251)
	$(UCS2VGA) ter-u32b.bdf $(R_W1251) $(V_W1251) | $(BDF2PSF) -o $@ - $(D_W1251)

ter-d32n.psf: ter-u32n.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u32n.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-d32b.psf: ter-u32b.bdf $(V_8859D) $(D_8859D)
	$(UCS2VGA) ter-u32b.bdf $(R_8859D) $(V_8859D) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g32n.psf: ter-u32n.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u32n.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-g32b.psf: ter-u32b.bdf $(V_8859G) $(D_8859G)
	$(UCS2VGA) ter-u32b.bdf $(R_8859G) $(V_8859G) | $(BDF2PSF) -o $@ - $(D_8859G)

ter-i32n.psf: ter-u32n.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u32n.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-i32b.psf: ter-u32b.bdf $(V_PCIBM) $(D_PCIBM)
	$(UCS2VGA) ter-u32b.bdf $(R_PCIBM) $(V_PCIBM) | $(BDF2PSF) -o $@ - $(D_PCIBM)

ter-k32n.psf: ter-u32n.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u32n.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-k32b.psf: ter-u32b.bdf $(F_KOI8R) $(D_KOI8R)
	$(UCS2VGA) ter-u32b.bdf $(R_KOI8R) $(F_KOI8R) | $(BDF2PSF) -o $@ - $(D_KOI8R)

ter-m32n.psf: ter-u32n.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u32n.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-m32b.psf: ter-u32b.bdf $(V_BGMIK) $(D_BGMIK)
	$(UCS2VGA) ter-u32b.bdf $(R_BGMIK) $(V_BGMIK) | $(BDF2PSF) -o $@ - $(D_BGMIK)

ter-p32n.psf: ter-u32n.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u32n.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-p32b.psf: ter-u32b.bdf $(V_PT154) $(D_PT154)
	$(UCS2VGA) ter-u32b.bdf $(R_PT154) $(V_PT154) | $(BDF2PSF) -o $@ - $(D_PT154)

ter-u32n.psf: ter-u32n.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u32n.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-u32b.psf: ter-u32b.bdf $(F_KOI8U) $(D_KOI8U)
	$(UCS2VGA) ter-u32b.bdf $(R_KOI8U) $(F_KOI8U) | $(BDF2PSF) -o $@ - $(D_KOI8U)

ter-v32n.psf: ter-u32n.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u32n.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

ter-v32b.psf: ter-u32b.bdf $(V_XOS42) $(D_XOS42)
	$(UCS2VGA) ter-u32b.bdf $(R_XOS42) $(V_XOS42) | $(BDF2PSF) -o $@ - $(D_XOS42)

install-psf: $(PSF)
	mkdir -p $(DESTDIR)$(psfdir)
	for i in $(PSF) ; do gzip -c $$i > $(DESTDIR)$(psfdir)/$$i.gz ; done

uninstall-psf:
	for i in $(PSF) ; do rm -f $(DESTDIR)$(psfdir)/$$i.gz ; done

T_W1252	= uni/cntrl.uni uni/ascii-h.uni uni/win-1252.uni
T_W1250	= uni/cntrl.uni uni/ascii-h.uni uni/win-1250.uni
T_W1253	= uni/cntrl.uni uni/ascii-h.uni uni/win-1253.uni
T_W1254	= uni/cntrl.uni uni/ascii-h.uni uni/win-1254.uni
T_W1251	= uni/cntrl.uni uni/ascii-h.uni uni/x11-1251.uni uni/win-1251.uni
T_W1257	= uni/cntrl.uni uni/ascii-h.uni uni/x11-1257.uni uni/win-1257.uni
T_8859G	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-16.uni
T_PCIBM	= uni/cntrl.uni uni/ascii-h.uni uni/ibm-437.uni
T_KOI8R	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-r.uni
T_KOI8U	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-u.uni
T_BGMIK	= uni/cntrl.uni uni/ascii-h.uni uni/bg-mik.uni
T_PT154	= uni/cntrl.uni uni/ascii-h.uni uni/pt-154.uni
T_PT254	= uni/cntrl.uni uni/ascii-h.uni uni/pt-254.uni

cp1252.txt: $(T_W1252)
	cat $(T_W1252) | $(UNI2TXT) > $@

cp1250.txt: $(T_W1250)
	cat $(T_W1250) | $(UNI2TXT) > $@

cp1253.txt: $(T_W1253)
	cat $(T_W1253) | $(UNI2TXT) > $@

cp1254.txt: $(T_W1254)
	cat $(T_W1254) | $(UNI2TXT) > $@

cp1251.txt: $(T_W1251)
	cat $(T_W1251) | $(UNI2TXT) > $@

cp1257.txt: $(T_W1257)
	cat $(T_W1257) | $(UNI2TXT) > $@

8859-16.txt: $(T_8859G)
	cat $(T_8859G) | $(UNI2TXT) > $@

cp437.txt: $(T_PCIBM)
	cat $(T_PCIBM) | $(UNI2TXT) > $@

koi8-r.txt: $(T_KOI8R)
	cat $(T_KOI8R) | $(UNI2TXT) > $@

koi8-u.txt: $(T_KOI8U)
	cat $(T_KOI8U) | $(UNI2TXT) > $@

mik.txt: $(T_BGMIK)
	cat $(T_BGMIK) | $(UNI2TXT) > $@

pt154.txt: $(T_PT154)
	cat $(T_PT154) | $(UNI2TXT) > $@

pt254.txt: $(T_PT254)
	cat $(T_PT254) | $(UNI2TXT) > $@

TESTDIR	=
acmdir	= $(prefix)/share/consoletrans

install-acm: $(TXT)
	mkdir -p $(DESTDIR)$(acmdir)
	for i in $(TXT) ; do \
		a=`echo $$i | sed -e "s/\.txt$$/.acm.gz/"` ; \
		if test ! -f $(TESTDIR)$(acmdir)/$$a ; then \
			tail -n 128 $$i | grep -v FFFF | gzip > $(DESTDIR)$(acmdir)/$$a ; \
		fi ; \
	done

unidir	= $(prefix)/share/kbd/consoletrans

install-uni: $(TXT)
	mkdir -p $(DESTDIR)$(unidir)
	for i in $(TXT) ; do \
		u=`echo $$i | sed -e "s/\.txt$$/_to_uni.trans/"` ; \
		if test ! -f $(TESTDIR)$(unidir)/$$u ; then \
			sed -e "s/FFFF/FFFD/" $$i > $(DESTDIR)$(unidir)/$$u ; \
		fi ; \
	done

ref	= $(psfdir)/README.terminus

install-ref: README
	sed -e"/^2\.4/,/^2\.5/p" -n README | grep -v "^2\." > $(DESTDIR)$(ref)

uninstall-ref:
	rm -f $(DESTDIR)$(ref)

B_88591	= uni/cntrl.uni uni/ascii-h.uni uni/win-1252.uni
B_88592	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-2.uni
B_88595	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-5.uni
B_88597	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-7.uni
B_88599	= uni/cntrl.uni uni/ascii-h.uni uni/win-1254.uni
B_W1251	= uni/cntrl.uni uni/ascii-h.uni uni/x11-1251.uni uni/win-1251.uni
B_8859D	= uni/cntrl.uni uni/ascii-h.uni uni/x11-1257.uni uni/8859-13.uni
B_8859F	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-15.uni
B_8859G	= uni/cntrl.uni uni/ascii-h.uni uni/empty.uni uni/8859-16.uni
B_PCIBM	= uni/cntrl.uni uni/ascii-h.uni uni/ibm-437.uni
B_KOI8R	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-r.uni
B_KOI8U	= uni/cntrl.uni uni/ascii-h.uni uni/koi8-u.uni
B_PT154	= uni/cntrl.uni uni/ascii-h.uni uni/pt-154.uni

ter-114n.raw: ter-u14n.bdf $(B_88591)
	$(UCS2VGA) ter-u14n.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-114b.raw: ter-u14v.bdf $(B_88591)
	$(UCS2VGA) ter-u14v.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-114f.raw: ter-u14b.bdf $(B_88591)
	$(UCS2VGA) ter-u14b.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-214n.raw: ter-u14n.bdf $(B_88592)
	$(UCS2VGA) ter-u14n.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-214b.raw: ter-u14v.bdf $(B_88592)
	$(UCS2VGA) ter-u14v.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-214f.raw: ter-u14b.bdf $(B_88592)
	$(UCS2VGA) ter-u14b.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-514n.raw: ter-u14n.bdf $(B_88595)
	$(UCS2VGA) ter-u14n.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-514b.raw: ter-u14v.bdf $(B_88595)
	$(UCS2VGA) ter-u14v.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-514f.raw: ter-u14b.bdf $(B_88595)
	$(UCS2VGA) ter-u14b.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-714n.raw: ter-u14n.bdf $(B_88597)
	$(UCS2VGA) ter-u14n.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-714b.raw: ter-u14v.bdf $(B_88597)
	$(UCS2VGA) ter-u14v.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-714f.raw: ter-u14b.bdf $(B_88597)
	$(UCS2VGA) ter-u14b.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-914n.raw: ter-u14n.bdf $(B_88599)
	$(UCS2VGA) ter-u14n.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-914b.raw: ter-u14v.bdf $(B_88599)
	$(UCS2VGA) ter-u14v.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-914f.raw: ter-u14b.bdf $(B_88599)
	$(UCS2VGA) ter-u14b.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-c14n.raw: ter-u14n.bdf $(B_W1251)
	$(UCS2VGA) ter-u14n.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-c14b.raw: ter-u14v.bdf $(B_W1251)
	$(UCS2VGA) ter-u14v.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-c14f.raw: ter-u14b.bdf $(B_W1251)
	$(UCS2VGA) ter-u14b.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-d14n.raw: ter-u14n.bdf $(B_8859D)
	$(UCS2VGA) ter-u14n.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-d14b.raw: ter-u14v.bdf $(B_8859D)
	$(UCS2VGA) ter-u14v.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-d14f.raw: ter-u14b.bdf $(B_8859D)
	$(UCS2VGA) ter-u14b.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-f14n.raw: ter-u14n.bdf $(B_8859F)
	$(UCS2VGA) ter-u14n.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-f14b.raw: ter-u14v.bdf $(B_8859F)
	$(UCS2VGA) ter-u14v.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-f14f.raw: ter-u14b.bdf $(B_8859F)
	$(UCS2VGA) ter-u14b.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-g14n.raw: ter-u14n.bdf $(B_8859G)
	$(UCS2VGA) ter-u14n.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-g14b.raw: ter-u14v.bdf $(B_8859G)
	$(UCS2VGA) ter-u14v.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-g14f.raw: ter-u14b.bdf $(B_8859G)
	$(UCS2VGA) ter-u14b.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-i14n.raw: ter-u14n.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u14n.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-i14b.raw: ter-u14v.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u14v.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-i14f.raw: ter-u14b.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u14b.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-k14n.raw: ter-u14n.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u14n.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-k14b.raw: ter-u14v.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u14v.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-k14f.raw: ter-u14b.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u14b.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-p14n.raw: ter-u14n.bdf $(B_PT154)
	$(UCS2VGA) ter-u14n.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-p14b.raw: ter-u14v.bdf $(B_PT154)
	$(UCS2VGA) ter-u14v.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-p14f.raw: ter-u14b.bdf $(B_PT154)
	$(UCS2VGA) ter-u14b.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-u14n.raw: ter-u14n.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u14n.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

ter-u14b.raw: ter-u14v.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u14v.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

ter-u14f.raw: ter-u14b.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u14b.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

ter-116n.raw: ter-u16n.bdf $(B_88591)
	$(UCS2VGA) ter-u16n.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-116b.raw: ter-u16v.bdf $(B_88591)
	$(UCS2VGA) ter-u16v.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-116f.raw: ter-u16b.bdf $(B_88591)
	$(UCS2VGA) ter-u16b.bdf $(R_88591) $(B_88591) | $(BDF2RAW) -o $@

ter-216n.raw: ter-u16n.bdf $(B_88592)
	$(UCS2VGA) ter-u16n.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-216b.raw: ter-u16v.bdf $(B_88592)
	$(UCS2VGA) ter-u16v.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-216f.raw: ter-u16b.bdf $(B_88592)
	$(UCS2VGA) ter-u16b.bdf $(R_88592) $(B_88592) | $(BDF2RAW) -o $@

ter-516n.raw: ter-u16n.bdf $(B_88595)
	$(UCS2VGA) ter-u16n.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-516b.raw: ter-u16v.bdf $(B_88595)
	$(UCS2VGA) ter-u16v.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-516f.raw: ter-u16b.bdf $(B_88595)
	$(UCS2VGA) ter-u16b.bdf $(R_88595) $(B_88595) | $(BDF2RAW) -o $@

ter-716n.raw: ter-u16n.bdf $(B_88597)
	$(UCS2VGA) ter-u16n.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-716b.raw: ter-u16v.bdf $(B_88597)
	$(UCS2VGA) ter-u16v.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-716f.raw: ter-u16b.bdf $(B_88597)
	$(UCS2VGA) ter-u16b.bdf $(R_88597) $(B_88597) | $(BDF2RAW) -o $@

ter-916n.raw: ter-u16n.bdf $(B_88599)
	$(UCS2VGA) ter-u16n.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-916b.raw: ter-u16v.bdf $(B_88599)
	$(UCS2VGA) ter-u16v.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-916f.raw: ter-u16b.bdf $(B_88599)
	$(UCS2VGA) ter-u16b.bdf $(R_88599) $(B_88599) | $(BDF2RAW) -o $@

ter-c16n.raw: ter-u16n.bdf $(B_W1251)
	$(UCS2VGA) ter-u16n.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-c16b.raw: ter-u16v.bdf $(B_W1251)
	$(UCS2VGA) ter-u16v.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-c16f.raw: ter-u16b.bdf $(B_W1251)
	$(UCS2VGA) ter-u16b.bdf $(R_W1251) $(B_W1251) | $(BDF2RAW) -o $@

ter-d16n.raw: ter-u16n.bdf $(B_8859D)
	$(UCS2VGA) ter-u16n.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-d16b.raw: ter-u16v.bdf $(B_8859D)
	$(UCS2VGA) ter-u16v.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-d16f.raw: ter-u16b.bdf $(B_8859D)
	$(UCS2VGA) ter-u16b.bdf $(R_8859D) $(B_8859D) | $(BDF2RAW) -o $@

ter-f16n.raw: ter-u16n.bdf $(B_8859F)
	$(UCS2VGA) ter-u16n.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-f16b.raw: ter-u16v.bdf $(B_8859F)
	$(UCS2VGA) ter-u16v.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-f16f.raw: ter-u16b.bdf $(B_8859F)
	$(UCS2VGA) ter-u16b.bdf $(R_8859F) $(B_8859F) | $(BDF2RAW) -o $@

ter-g16n.raw: ter-u16n.bdf $(B_8859G)
	$(UCS2VGA) ter-u16n.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-g16b.raw: ter-u16v.bdf $(B_8859G)
	$(UCS2VGA) ter-u16v.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-g16f.raw: ter-u16b.bdf $(B_8859G)
	$(UCS2VGA) ter-u16b.bdf $(R_8859G) $(B_8859G) | $(BDF2RAW) -o $@

ter-i16n.raw: ter-u16n.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u16n.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-i16b.raw: ter-u16v.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u16v.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-i16f.raw: ter-u16b.bdf $(B_PCIBM)
	$(UCS2VGA) ter-u16b.bdf $(R_PCIBM) $(B_PCIBM) | $(BDF2RAW) -o $@

ter-k16n.raw: ter-u16n.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u16n.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-k16b.raw: ter-u16v.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u16v.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-k16f.raw: ter-u16b.bdf $(B_KOI8R)
	$(UCS2VGA) ter-u16b.bdf $(R_KOI8R) $(B_KOI8R) | $(BDF2RAW) -o $@

ter-p16n.raw: ter-u16n.bdf $(B_PT154)
	$(UCS2VGA) ter-u16n.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-p16b.raw: ter-u16v.bdf $(B_PT154)
	$(UCS2VGA) ter-u16v.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-p16f.raw: ter-u16b.bdf $(B_PT154)
	$(UCS2VGA) ter-u16b.bdf $(R_PT154) $(B_PT154) | $(BDF2RAW) -o $@

ter-u16n.raw: ter-u16n.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u16n.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

ter-u16b.raw: ter-u16v.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u16v.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

ter-u16f.raw: ter-u16b.bdf $(B_KOI8U)
	$(UCS2VGA) ter-u16b.bdf $(R_KOI8U) $(B_KOI8U) | $(BDF2RAW) -o $@

rawdir	= $(prefix)/share/misc/pcvtfonts

RAW_14	= $(RAW_14N) $(RAW_14B) $(RAW_14F)
RAW_16	= $(RAW_16N) $(RAW_16B) $(RAW_16F)

point14	= echo $$i | sed -e "s/14//" -e "s/raw$$/814/"
point16	= echo $$i | sed -e "s/16//" -e "s/raw$$/816/"

install.raw: $(RAW)
	mkdir -p $(DESTDIR)$(rawdir)
	for i in $(RAW_14) ; do cp -f $$i $(DESTDIR)$(rawdir)/`$(point14)` ; done
	for i in $(RAW_16) ; do cp -f $$i $(DESTDIR)$(rawdir)/`$(point16)` ; done

uninstall.raw:
	for i in $(RAW_14) ; do rm -f $(DESTDIR)$(rawdir)/`$(point14)` ; done
	for i in $(RAW_16) ; do rm -f $(DESTDIR)$(rawdir)/`$(point16)` ; done

minus14	= echo $$i | sed -e "s/14//" -e "s/\.raw$$/-8x14/"
minus16	= echo $$i | sed -e "s/16//" -e "s/\.raw$$/-8x16/"

install-raw: $(RAW)
	mkdir -p $(DESTDIR)$(rawdir)
	for i in $(RAW_14) ; do cp -f $$i $(DESTDIR)$(rawdir)/`$(minus14)` ; done
	for i in $(RAW_16) ; do cp -f $$i $(DESTDIR)$(rawdir)/`$(minus16)` ; done

uninstall-raw:
	for i in $(RAW_14) ; do rm -f $(DESTDIR)$(rawdir)/`$(minus14)` ; done
	for i in $(RAW_16) ; do rm -f $(DESTDIR)$(rawdir)/`$(minus16)` ; done

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

ter-112n.pcf: ter-u12n.bdf $(X_88591)
	$(UCS2X11) ter-u12n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-112b.pcf: ter-u12b.bdf $(X_88591)
	$(UCS2X11) ter-u12b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-212n.pcf: ter-u12n.bdf $(X_88592)
	$(UCS2X11) ter-u12n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-212b.pcf: ter-u12b.bdf $(X_88592)
	$(UCS2X11) ter-u12b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-512n.pcf: ter-u12n.bdf $(X_88595)
	$(UCS2X11) ter-u12n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-512b.pcf: ter-u12b.bdf $(X_88595)
	$(UCS2X11) ter-u12b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-712n.pcf: ter-u12n.bdf $(X_88597)
	$(UCS2X11) ter-u12n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-712b.pcf: ter-u12b.bdf $(X_88597)
	$(UCS2X11) ter-u12b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-912n.pcf: ter-u12n.bdf $(X_88599)
	$(UCS2X11) ter-u12n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-912b.pcf: ter-u12b.bdf $(X_88599)
	$(UCS2X11) ter-u12b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c12n.pcf: ter-u12n.bdf $(X_W1251)
	$(UCS2X11) ter-u12n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c12b.pcf: ter-u12b.bdf $(X_W1251)
	$(UCS2X11) ter-u12b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d12n.pcf: ter-u12n.bdf $(X_8859D)
	$(UCS2X11) ter-u12n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d12b.pcf: ter-u12b.bdf $(X_8859D)
	$(UCS2X11) ter-u12b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f12n.pcf: ter-u12n.bdf $(X_8859F)
	$(UCS2X11) ter-u12n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f12b.pcf: ter-u12b.bdf $(X_8859F)
	$(UCS2X11) ter-u12b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g12n.pcf: ter-u12n.bdf $(X_8859G)
	$(UCS2X11) ter-u12n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g12b.pcf: ter-u12b.bdf $(X_8859G)
	$(UCS2X11) ter-u12b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k12n.pcf: ter-u12n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u12n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k12b.pcf: ter-u12b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u12b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p12n.pcf: ter-u12n.bdf $(X_PT154)
	$(UCS2X11) ter-u12n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p12b.pcf: ter-u12b.bdf $(X_PT154)
	$(UCS2X11) ter-u12b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u12n.pcf: ter-u12n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u12n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u12b.pcf: ter-u12b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u12b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x12n.pcf: ter-u12n.bdf $(X_10646)
	$(UCS2X11) ter-u12n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x12b.pcf: ter-u12b.bdf $(X_10646)
	$(UCS2X11) ter-u12b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-114n.pcf: ter-u14n.bdf $(X_88591)
	$(UCS2X11) ter-u14n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-114b.pcf: ter-u14b.bdf $(X_88591)
	$(UCS2X11) ter-u14b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-214n.pcf: ter-u14n.bdf $(X_88592)
	$(UCS2X11) ter-u14n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-214b.pcf: ter-u14b.bdf $(X_88592)
	$(UCS2X11) ter-u14b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-514n.pcf: ter-u14n.bdf $(X_88595)
	$(UCS2X11) ter-u14n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-514b.pcf: ter-u14b.bdf $(X_88595)
	$(UCS2X11) ter-u14b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-714n.pcf: ter-u14n.bdf $(X_88597)
	$(UCS2X11) ter-u14n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-714b.pcf: ter-u14b.bdf $(X_88597)
	$(UCS2X11) ter-u14b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-914n.pcf: ter-u14n.bdf $(X_88599)
	$(UCS2X11) ter-u14n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-914b.pcf: ter-u14b.bdf $(X_88599)
	$(UCS2X11) ter-u14b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c14n.pcf: ter-u14n.bdf $(X_W1251)
	$(UCS2X11) ter-u14n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c14b.pcf: ter-u14b.bdf $(X_W1251)
	$(UCS2X11) ter-u14b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d14n.pcf: ter-u14n.bdf $(X_8859D)
	$(UCS2X11) ter-u14n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d14b.pcf: ter-u14b.bdf $(X_8859D)
	$(UCS2X11) ter-u14b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f14n.pcf: ter-u14n.bdf $(X_8859F)
	$(UCS2X11) ter-u14n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f14b.pcf: ter-u14b.bdf $(X_8859F)
	$(UCS2X11) ter-u14b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g14n.pcf: ter-u14n.bdf $(X_8859G)
	$(UCS2X11) ter-u14n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g14b.pcf: ter-u14b.bdf $(X_8859G)
	$(UCS2X11) ter-u14b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k14n.pcf: ter-u14n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u14n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k14b.pcf: ter-u14b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u14b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p14n.pcf: ter-u14n.bdf $(X_PT154)
	$(UCS2X11) ter-u14n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p14b.pcf: ter-u14b.bdf $(X_PT154)
	$(UCS2X11) ter-u14b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u14n.pcf: ter-u14n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u14n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u14b.pcf: ter-u14b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u14b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x14n.pcf: ter-u14n.bdf $(X_10646)
	$(UCS2X11) ter-u14n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x14b.pcf: ter-u14b.bdf $(X_10646)
	$(UCS2X11) ter-u14b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-116n.pcf: ter-u16n.bdf $(X_88591)
	$(UCS2X11) ter-u16n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-116b.pcf: ter-u16b.bdf $(X_88591)
	$(UCS2X11) ter-u16b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-216n.pcf: ter-u16n.bdf $(X_88592)
	$(UCS2X11) ter-u16n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-216b.pcf: ter-u16b.bdf $(X_88592)
	$(UCS2X11) ter-u16b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-516n.pcf: ter-u16n.bdf $(X_88595)
	$(UCS2X11) ter-u16n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-516b.pcf: ter-u16b.bdf $(X_88595)
	$(UCS2X11) ter-u16b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-716n.pcf: ter-u16n.bdf $(X_88597)
	$(UCS2X11) ter-u16n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-716b.pcf: ter-u16b.bdf $(X_88597)
	$(UCS2X11) ter-u16b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-916n.pcf: ter-u16n.bdf $(X_88599)
	$(UCS2X11) ter-u16n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-916b.pcf: ter-u16b.bdf $(X_88599)
	$(UCS2X11) ter-u16b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c16n.pcf: ter-u16n.bdf $(X_W1251)
	$(UCS2X11) ter-u16n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c16b.pcf: ter-u16b.bdf $(X_W1251)
	$(UCS2X11) ter-u16b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d16n.pcf: ter-u16n.bdf $(X_8859D)
	$(UCS2X11) ter-u16n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d16b.pcf: ter-u16b.bdf $(X_8859D)
	$(UCS2X11) ter-u16b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f16n.pcf: ter-u16n.bdf $(X_8859F)
	$(UCS2X11) ter-u16n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f16b.pcf: ter-u16b.bdf $(X_8859F)
	$(UCS2X11) ter-u16b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g16n.pcf: ter-u16n.bdf $(X_8859G)
	$(UCS2X11) ter-u16n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g16b.pcf: ter-u16b.bdf $(X_8859G)
	$(UCS2X11) ter-u16b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k16n.pcf: ter-u16n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u16n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k16b.pcf: ter-u16b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u16b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p16n.pcf: ter-u16n.bdf $(X_PT154)
	$(UCS2X11) ter-u16n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p16b.pcf: ter-u16b.bdf $(X_PT154)
	$(UCS2X11) ter-u16b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u16n.pcf: ter-u16n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u16n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u16b.pcf: ter-u16b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u16b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x16n.pcf: ter-u16n.bdf $(X_10646)
	$(UCS2X11) ter-u16n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x16b.pcf: ter-u16b.bdf $(X_10646)
	$(UCS2X11) ter-u16b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-120n.pcf: ter-u20n.bdf $(X_88591)
	$(UCS2X11) ter-u20n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-120b.pcf: ter-u20b.bdf $(X_88591)
	$(UCS2X11) ter-u20b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-220n.pcf: ter-u20n.bdf $(X_88592)
	$(UCS2X11) ter-u20n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-220b.pcf: ter-u20b.bdf $(X_88592)
	$(UCS2X11) ter-u20b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-520n.pcf: ter-u20n.bdf $(X_88595)
	$(UCS2X11) ter-u20n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-520b.pcf: ter-u20b.bdf $(X_88595)
	$(UCS2X11) ter-u20b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-720n.pcf: ter-u20n.bdf $(X_88597)
	$(UCS2X11) ter-u20n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-720b.pcf: ter-u20b.bdf $(X_88597)
	$(UCS2X11) ter-u20b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-920n.pcf: ter-u20n.bdf $(X_88599)
	$(UCS2X11) ter-u20n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-920b.pcf: ter-u20b.bdf $(X_88599)
	$(UCS2X11) ter-u20b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c20n.pcf: ter-u20n.bdf $(X_W1251)
	$(UCS2X11) ter-u20n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c20b.pcf: ter-u20b.bdf $(X_W1251)
	$(UCS2X11) ter-u20b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d20n.pcf: ter-u20n.bdf $(X_8859D)
	$(UCS2X11) ter-u20n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d20b.pcf: ter-u20b.bdf $(X_8859D)
	$(UCS2X11) ter-u20b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f20n.pcf: ter-u20n.bdf $(X_8859F)
	$(UCS2X11) ter-u20n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f20b.pcf: ter-u20b.bdf $(X_8859F)
	$(UCS2X11) ter-u20b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g20n.pcf: ter-u20n.bdf $(X_8859G)
	$(UCS2X11) ter-u20n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g20b.pcf: ter-u20b.bdf $(X_8859G)
	$(UCS2X11) ter-u20b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k20n.pcf: ter-u20n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u20n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k20b.pcf: ter-u20b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u20b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p20n.pcf: ter-u20n.bdf $(X_PT154)
	$(UCS2X11) ter-u20n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p20b.pcf: ter-u20b.bdf $(X_PT154)
	$(UCS2X11) ter-u20b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u20n.pcf: ter-u20n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u20n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u20b.pcf: ter-u20b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u20b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x20n.pcf: ter-u20n.bdf $(X_10646)
	$(UCS2X11) ter-u20n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x20b.pcf: ter-u20b.bdf $(X_10646)
	$(UCS2X11) ter-u20b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-122n.pcf: ter-u22n.bdf $(X_88591)
	$(UCS2X11) ter-u22n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-122b.pcf: ter-u22b.bdf $(X_88591)
	$(UCS2X11) ter-u22b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-222n.pcf: ter-u22n.bdf $(X_88592)
	$(UCS2X11) ter-u22n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-222b.pcf: ter-u22b.bdf $(X_88592)
	$(UCS2X11) ter-u22b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-522n.pcf: ter-u22n.bdf $(X_88595)
	$(UCS2X11) ter-u22n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-522b.pcf: ter-u22b.bdf $(X_88595)
	$(UCS2X11) ter-u22b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-722n.pcf: ter-u22n.bdf $(X_88597)
	$(UCS2X11) ter-u22n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-722b.pcf: ter-u22b.bdf $(X_88597)
	$(UCS2X11) ter-u22b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-922n.pcf: ter-u22n.bdf $(X_88599)
	$(UCS2X11) ter-u22n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-922b.pcf: ter-u22b.bdf $(X_88599)
	$(UCS2X11) ter-u22b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c22n.pcf: ter-u22n.bdf $(X_W1251)
	$(UCS2X11) ter-u22n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c22b.pcf: ter-u22b.bdf $(X_W1251)
	$(UCS2X11) ter-u22b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d22n.pcf: ter-u22n.bdf $(X_8859D)
	$(UCS2X11) ter-u22n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d22b.pcf: ter-u22b.bdf $(X_8859D)
	$(UCS2X11) ter-u22b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f22n.pcf: ter-u22n.bdf $(X_8859F)
	$(UCS2X11) ter-u22n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f22b.pcf: ter-u22b.bdf $(X_8859F)
	$(UCS2X11) ter-u22b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g22n.pcf: ter-u22n.bdf $(X_8859G)
	$(UCS2X11) ter-u22n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g22b.pcf: ter-u22b.bdf $(X_8859G)
	$(UCS2X11) ter-u22b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k22n.pcf: ter-u22n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u22n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k22b.pcf: ter-u22b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u22b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p22n.pcf: ter-u22n.bdf $(X_PT154)
	$(UCS2X11) ter-u22n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p22b.pcf: ter-u22b.bdf $(X_PT154)
	$(UCS2X11) ter-u22b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u22n.pcf: ter-u22n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u22n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u22b.pcf: ter-u22b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u22b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x22n.pcf: ter-u22n.bdf $(X_10646)
	$(UCS2X11) ter-u22n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x22b.pcf: ter-u22b.bdf $(X_10646)
	$(UCS2X11) ter-u22b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-124n.pcf: ter-u24n.bdf $(X_88591)
	$(UCS2X11) ter-u24n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-124b.pcf: ter-u24b.bdf $(X_88591)
	$(UCS2X11) ter-u24b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-224n.pcf: ter-u24n.bdf $(X_88592)
	$(UCS2X11) ter-u24n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-224b.pcf: ter-u24b.bdf $(X_88592)
	$(UCS2X11) ter-u24b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-524n.pcf: ter-u24n.bdf $(X_88595)
	$(UCS2X11) ter-u24n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-524b.pcf: ter-u24b.bdf $(X_88595)
	$(UCS2X11) ter-u24b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-724n.pcf: ter-u24n.bdf $(X_88597)
	$(UCS2X11) ter-u24n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-724b.pcf: ter-u24b.bdf $(X_88597)
	$(UCS2X11) ter-u24b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-924n.pcf: ter-u24n.bdf $(X_88599)
	$(UCS2X11) ter-u24n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-924b.pcf: ter-u24b.bdf $(X_88599)
	$(UCS2X11) ter-u24b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c24n.pcf: ter-u24n.bdf $(X_W1251)
	$(UCS2X11) ter-u24n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c24b.pcf: ter-u24b.bdf $(X_W1251)
	$(UCS2X11) ter-u24b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d24n.pcf: ter-u24n.bdf $(X_8859D)
	$(UCS2X11) ter-u24n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d24b.pcf: ter-u24b.bdf $(X_8859D)
	$(UCS2X11) ter-u24b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f24n.pcf: ter-u24n.bdf $(X_8859F)
	$(UCS2X11) ter-u24n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f24b.pcf: ter-u24b.bdf $(X_8859F)
	$(UCS2X11) ter-u24b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g24n.pcf: ter-u24n.bdf $(X_8859G)
	$(UCS2X11) ter-u24n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g24b.pcf: ter-u24b.bdf $(X_8859G)
	$(UCS2X11) ter-u24b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k24n.pcf: ter-u24n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u24n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k24b.pcf: ter-u24b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u24b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p24n.pcf: ter-u24n.bdf $(X_PT154)
	$(UCS2X11) ter-u24n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p24b.pcf: ter-u24b.bdf $(X_PT154)
	$(UCS2X11) ter-u24b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u24n.pcf: ter-u24n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u24n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u24b.pcf: ter-u24b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u24b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x24n.pcf: ter-u24n.bdf $(X_10646)
	$(UCS2X11) ter-u24n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x24b.pcf: ter-u24b.bdf $(X_10646)
	$(UCS2X11) ter-u24b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-128n.pcf: ter-u28n.bdf $(X_88591)
	$(UCS2X11) ter-u28n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-128b.pcf: ter-u28b.bdf $(X_88591)
	$(UCS2X11) ter-u28b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-228n.pcf: ter-u28n.bdf $(X_88592)
	$(UCS2X11) ter-u28n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-228b.pcf: ter-u28b.bdf $(X_88592)
	$(UCS2X11) ter-u28b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-528n.pcf: ter-u28n.bdf $(X_88595)
	$(UCS2X11) ter-u28n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-528b.pcf: ter-u28b.bdf $(X_88595)
	$(UCS2X11) ter-u28b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-728n.pcf: ter-u28n.bdf $(X_88597)
	$(UCS2X11) ter-u28n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-728b.pcf: ter-u28b.bdf $(X_88597)
	$(UCS2X11) ter-u28b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-928n.pcf: ter-u28n.bdf $(X_88599)
	$(UCS2X11) ter-u28n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-928b.pcf: ter-u28b.bdf $(X_88599)
	$(UCS2X11) ter-u28b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c28n.pcf: ter-u28n.bdf $(X_W1251)
	$(UCS2X11) ter-u28n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c28b.pcf: ter-u28b.bdf $(X_W1251)
	$(UCS2X11) ter-u28b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d28n.pcf: ter-u28n.bdf $(X_8859D)
	$(UCS2X11) ter-u28n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d28b.pcf: ter-u28b.bdf $(X_8859D)
	$(UCS2X11) ter-u28b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f28n.pcf: ter-u28n.bdf $(X_8859F)
	$(UCS2X11) ter-u28n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f28b.pcf: ter-u28b.bdf $(X_8859F)
	$(UCS2X11) ter-u28b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g28n.pcf: ter-u28n.bdf $(X_8859G)
	$(UCS2X11) ter-u28n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g28b.pcf: ter-u28b.bdf $(X_8859G)
	$(UCS2X11) ter-u28b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k28n.pcf: ter-u28n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u28n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k28b.pcf: ter-u28b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u28b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p28n.pcf: ter-u28n.bdf $(X_PT154)
	$(UCS2X11) ter-u28n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p28b.pcf: ter-u28b.bdf $(X_PT154)
	$(UCS2X11) ter-u28b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u28n.pcf: ter-u28n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u28n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u28b.pcf: ter-u28b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u28b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x28n.pcf: ter-u28n.bdf $(X_10646)
	$(UCS2X11) ter-u28n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x28b.pcf: ter-u28b.bdf $(X_10646)
	$(UCS2X11) ter-u28b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-132n.pcf: ter-u32n.bdf $(X_88591)
	$(UCS2X11) ter-u32n.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-132b.pcf: ter-u32b.bdf $(X_88591)
	$(UCS2X11) ter-u32b.bdf $(R_88591) $(X_88591) | $(BDF2PCF) -o $@

ter-232n.pcf: ter-u32n.bdf $(X_88592)
	$(UCS2X11) ter-u32n.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-232b.pcf: ter-u32b.bdf $(X_88592)
	$(UCS2X11) ter-u32b.bdf $(R_88592) $(X_88592) | $(BDF2PCF) -o $@

ter-532n.pcf: ter-u32n.bdf $(X_88595)
	$(UCS2X11) ter-u32n.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-532b.pcf: ter-u32b.bdf $(X_88595)
	$(UCS2X11) ter-u32b.bdf $(R_88595) $(X_88595) | $(BDF2PCF) -o $@

ter-732n.pcf: ter-u32n.bdf $(X_88597)
	$(UCS2X11) ter-u32n.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-732b.pcf: ter-u32b.bdf $(X_88597)
	$(UCS2X11) ter-u32b.bdf $(R_88597) $(X_88597) | $(BDF2PCF) -o $@

ter-932n.pcf: ter-u32n.bdf $(X_88599)
	$(UCS2X11) ter-u32n.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-932b.pcf: ter-u32b.bdf $(X_88599)
	$(UCS2X11) ter-u32b.bdf $(R_88599) $(X_88599) | $(BDF2PCF) -o $@

ter-c32n.pcf: ter-u32n.bdf $(X_W1251)
	$(UCS2X11) ter-u32n.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-c32b.pcf: ter-u32b.bdf $(X_W1251)
	$(UCS2X11) ter-u32b.bdf $(R_W1251) $(X_W1251) | $(BDF2PCF) -o $@

ter-d32n.pcf: ter-u32n.bdf $(X_8859D)
	$(UCS2X11) ter-u32n.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-d32b.pcf: ter-u32b.bdf $(X_8859D)
	$(UCS2X11) ter-u32b.bdf $(R_8859D) $(X_8859D) | $(BDF2PCF) -o $@

ter-f32n.pcf: ter-u32n.bdf $(X_8859F)
	$(UCS2X11) ter-u32n.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-f32b.pcf: ter-u32b.bdf $(X_8859F)
	$(UCS2X11) ter-u32b.bdf $(R_8859F) $(X_8859F) | $(BDF2PCF) -o $@

ter-g32n.pcf: ter-u32n.bdf $(X_8859G)
	$(UCS2X11) ter-u32n.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-g32b.pcf: ter-u32b.bdf $(X_8859G)
	$(UCS2X11) ter-u32b.bdf $(R_8859G) $(X_8859G) | $(BDF2PCF) -o $@

ter-k32n.pcf: ter-u32n.bdf $(X_KOI8R)
	$(UCS2X11) ter-u32n.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-k32b.pcf: ter-u32b.bdf $(X_KOI8R)
	$(UCS2X11) ter-u32b.bdf $(R_KOI8R) $(X_KOI8R) | $(BDF2PCF) -o $@

ter-p32n.pcf: ter-u32n.bdf $(X_PT154)
	$(UCS2X11) ter-u32n.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-p32b.pcf: ter-u32b.bdf $(X_PT154)
	$(UCS2X11) ter-u32b.bdf $(R_PT154) $(X_PT154) | $(BDF2PCF) -o $@

ter-u32n.pcf: ter-u32n.bdf $(X_KOI8U)
	$(UCS2X11) ter-u32n.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-u32b.pcf: ter-u32b.bdf $(X_KOI8U)
	$(UCS2X11) ter-u32b.bdf $(R_KOI8U) $(X_KOI8U) | $(BDF2PCF) -o $@

ter-x32n.pcf: ter-u32n.bdf $(X_10646)
	$(UCS2X11) ter-u32n.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

ter-x32b.pcf: ter-u32b.bdf $(X_10646)
	$(UCS2X11) ter-u32b.bdf $(R_10646) $(X_10646) | $(BDF2PCF) -o $@

install-pcf: $(PCF)
	mkdir -p $(DESTDIR)$(x11dir)
	for i in $(PCF) ; do gzip -c $$i > $(DESTDIR)$(x11dir)/$$i.gz ; done

uninstall-pcf:
	for i in $(PCF) ; do rm -f $(DESTDIR)$(x11dir)/$$i.gz ; done

install-n12: $(PCF_N12)
	mkdir -p $(DESTDIR)$(x11dir)
	for i in $(PCF_N12) ; do gzip -c $$i > $(DESTDIR)$(x11dir)/$$i.gz ; done

uninstall-n12:
	for i in $(PCF_N12) ; do rm -f $(DESTDIR)$(x11dir)/$$i.gz ; done

clean:
	rm -f *.psf *.txt *.raw *.pcf
