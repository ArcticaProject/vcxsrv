TRANSFORM_FILES=layoutsMapping.lst variantsMapping.lst

scripts_dir=..\bin

SCRIPTS= \
base.l2_s.part \
base.l3_s.part \
base.l4_s.part \
base.l2v2_s.part \
base.l3v3_s.part \
base.l4v4_s.part \
base.ml_s.part \
base.ml1_s.part \
base.mlv_s.part \
base.ml1v1_s.part \
base.ml_s.part

SH=cmd /c

%.bat: $(scripts_dir)\%.sh
	copy $< $@

base.l2_s.part: ln_s.bat $(TRANSFORM_FILES)
	$(SH) $< 2

base.l3_s.part: ln_s.bat $(TRANSFORM_FILES)
	$(SH) $< 3

base.l4_s.part: ln_s.bat $(TRANSFORM_FILES)
	$(SH) $< 4

base.l2v2_s.part: lnv_s.bat $(TRANSFORM_FILES)
	$(SH) $< 2

base.l3v3_s.part: lnv_s.bat $(TRANSFORM_FILES)
	$(SH) $< 3

base.l4v4_s.part: lnv_s.bat $(TRANSFORM_FILES)
	$(SH) $< 4

base.ml_s.part: ml_s.bat $(TRANSFORM_FILES)
	$(SH) $<

base.ml1_s.part: ml1_s.bat $(TRANSFORM_FILES)
	$(SH) $<

base.mlv_s.part: mlv_s.bat $(TRANSFORM_FILES)
	$(SH) $<

base.ml1v1_s.part: ml1v1_s.bat $(TRANSFORM_FILES)
	$(SH) $<

all=$(SCRIPTS)
