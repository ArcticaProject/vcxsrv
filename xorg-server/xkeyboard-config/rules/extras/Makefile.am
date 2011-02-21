transform_files=layoutsMapping.lst variantsMapping.lst

scripts_dir=$(srcdir)/../bin

scripts= \
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

noinst_SCRIPTS=$(scripts)

base.ml2_s.part: $(scripts_dir)/mln_s.sh $(transform_files)
	sh $< 2 $(srcdir)

base.ml3_s.part: $(scripts_dir)/mln_s.sh $(transform_files)
	sh $< 3 $(srcdir)

base.ml4_s.part: $(scripts_dir)/mln_s.sh $(transform_files)
	sh $< 4 $(srcdir)

base.ml2v2_s.part: $(scripts_dir)/mlnvn_s.sh $(transform_files)
	sh $< 2 $(srcdir)

base.ml3v3_s.part: $(scripts_dir)/mlnvn_s.sh $(transform_files)
	sh $< 3 $(srcdir)

base.ml4v4_s.part: $(scripts_dir)/mlnvn_s.sh $(transform_files)
	sh $< 4 $(srcdir)

base.ml_s.part: $(scripts_dir)/ml_s.sh $(transform_files)
	sh $< $(srcdir)

base.ml1_s.part: $(scripts_dir)/ml1_s.sh $(transform_files)
	sh $< $(srcdir)

base.mlv_s.part: $(scripts_dir)/mlv_s.sh $(transform_files)
	sh $< $(srcdir)

base.ml1v1_s.part: $(scripts_dir)/ml1v1_s.sh $(transform_files)
	sh $< $(srcdir)

EXTRA_DIST=$(transform_files)

CLEANFILES=$(scripts)
