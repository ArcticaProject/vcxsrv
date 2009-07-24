
transform_files=layoutRename.lst variantRename.lst

transform_scripts= \
ln_s.sh \
lnv_s.sh \
ml_s.sh \
mlv_s.sh \
ml1_s.sh \
ml1v_s.sh \
ml1v1_s.sh

dist_parts=base.o_s.part

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

base.l2_s.part: ln_s.sh $(transform_files)
	sh $< 2

base.l3_s.part: ln_s.sh $(transform_files)
	sh $< 3

base.l4_s.part: ln_s.sh $(transform_files)
	sh $< 4

base.l2v2_s.part: lnv_s.sh $(transform_files)
	sh $< 2

base.l3v3_s.part: lnv_s.sh $(transform_files)
	sh $< 3

base.l4v4_s.part: lnv_s.sh $(transform_files)
	sh $< 4

base.ml_s.part: ml_s.sh $(transform_files)
	sh $<

base.ml1_s.part: ml1_s.sh $(transform_files)
	sh $<

base.mlv_s.part: mlv_s.sh $(transform_files)
	sh $<

base.ml1v1_s.part: ml1v1_s.sh $(transform_files)
	sh $<

EXTRA_DIST=base.lists.part $(transform_files) $(transform_scripts) $(dist_parts)

CLEANFILES=$(scripts)
