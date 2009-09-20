
transform_files=layoutsMapping.lst variantsMapping.lst

dist_parts=base.o_s.part

scripts_dir=$(srcdir)/../bin

scripts= \
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

if USE_COMPAT_RULES

noinst_SCRIPTS=$(scripts) $(dist_parts)

else

noinst_SCRIPTS=

endif

base.l2_s.part: $(scripts_dir)/ln_s.sh $(transform_files)
	sh $< 2 $(srcdir)

base.l3_s.part: $(scripts_dir)/ln_s.sh $(transform_files)
	sh $< 3 $(srcdir)

base.l4_s.part: $(scripts_dir)/ln_s.sh $(transform_files)
	sh $< 4 $(srcdir)

base.l2v2_s.part: $(scripts_dir)/lnv_s.sh $(transform_files)
	sh $< 2 $(srcdir)

base.l3v3_s.part: $(scripts_dir)/lnv_s.sh $(transform_files)
	sh $< 3 $(srcdir)

base.l4v4_s.part: $(scripts_dir)/lnv_s.sh $(transform_files)
	sh $< 4 $(srcdir)

base.ml_s.part: $(scripts_dir)/ml_s.sh $(transform_files)
	sh $< $(srcdir)

base.ml1_s.part: $(scripts_dir)/ml1_s.sh $(transform_files)
	sh $< $(srcdir)

base.mlv_s.part: $(scripts_dir)/mlv_s.sh $(transform_files)
	sh $< $(srcdir)

base.ml1v1_s.part: $(scripts_dir)/ml1v1_s.sh $(transform_files)
	sh $< $(srcdir)

EXTRA_DIST=base.lists.part $(transform_files) $(dist_parts)

CLEANFILES=$(scripts)
