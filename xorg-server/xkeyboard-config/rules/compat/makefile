transform_files=layoutsMapping.lst variantsMapping.lst

scripts_dir=..\bin

SCRIPTS= \
base.ml2_s.part \
base.ml3_s.part \
base.ml4_s.part \
base.ml2v2_s.part \
base.ml3v3_s.part \
base.ml4v4_s.part \
base.ml_s.part \
base.ml1_s.part \
base.mlv_s.part \
base.ml1v1_s.part


SH=cmd /c

base.ml2_s.part: mln_s.bat $(transform_files)
	$(SH) $< 2

base.ml3_s.part: mln_s.bat $(transform_files)
	$(SH) $< 3

base.ml4_s.part: mln_s.bat $(transform_files)
	$(SH) $< 4

base.ml2v2_s.part: mlnvn_s.bat $(transform_files)
	$(SH) $< 2

base.ml3v3_s.part: mlnvn_s.bat $(transform_files)
	$(SH) $< 3

base.ml4v4_s.part: mlnvn_s.bat $(transform_files)
	$(SH) $< 4

base.ml_s.part: ml_s.bat $(transform_files)
	$(SH) $<

base.ml1_s.part: ml1_s.bat $(transform_files)
	$(SH) $<

base.mlv_s.part: mlv_s.bat $(transform_files)
	$(SH) $<

base.ml1v1_s.part: ml1v1_s.bat $(transform_files)
	$(SH) $<

%.bat: $(scripts_dir)\%.sh
	copy $< $@

all: $(SCRIPTS)
