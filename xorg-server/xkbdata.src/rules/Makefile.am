SUBDIRS = compat

if CREATE_RULES_SYMLINK
#  extra_rules_file = $(xkb_rules_symlink)
#  extra_dir_file   = $(extra_rules_file).xml 
#  extra_lst_file   = $(extra_rules_file).lst

install-data-hook:
	for file in `echo "$(xkb_rules_symlink)" | sed 's/,/ /'` ; do \
		extra_rules_file=$$file ; \
		extra_dir_file=$$extra_rules_file.xml ; \
		extra_lst_file=$$extra_rules_file.lst ; \
		ln -sf base     $(DESTDIR)$(rulesdir)/$$extra_rules_file ; \
		ln -sf base.lst $(DESTDIR)$(rulesdir)/$$extra_lst_file ; \
		ln -sf base.xml $(DESTDIR)$(rulesdir)/$$extra_dir_file ; \
	done

else
install-data-hook:

endif

base_parts_no_compat = base.hdr.part base.lists.part \
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

evdev_parts_no_compat = base.hdr.part base.lists.part \
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

if USE_COMPAT_RULES
base_parts = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR base.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
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

evdev_parts = base.hdr.part base.lists.part \
compat/base.lists.part \
HDR evdev.m_k.part \
HDR base.l1_k.part \
HDR base.l_k.part \
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
else
base_parts = $(base_parts_no_compat)
evdev_parts = $(base_parts_no_compat)
endif

base_parts_compat_dist = base.o_k.part base.lo_s.part

rules_files = base evdev xfree98

lst_files = base.lst evdev.lst

%.lst: %.xml
	perl $(srcdir)/xml2lst.pl < $< > $@

base: $(base_parts_no_compat)
	HDR="$(srcdir)/HDR" $(srcdir)/merge.sh $@ $(base_parts)

evdev: $(evdev_parts_no_compat)
	HDR="$(srcdir)/HDR" $(srcdir)/merge.sh $@ $(evdev_parts)

dist_files = xkb.dtd README

rules_DATA = $(rules_files) $(lst_files) $(dist_files)

xml_in_files = base.xml.in evdev.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml) 

$(srcdir)/evdev.xml.in: base.xml.in
	cp $< $@

EXTRA_DIST= $(xml_in_files) xfree98 \
            $(base_parts_no_compat) \
            $(evdev_parts_no_compat) \
            $(base_parts_compat_dist) \
            xml2lst.pl merge.sh $(dist_files)

CLEANFILES = base base.xml base.lst evdev evdev.xml evdev.lst
MAINTAINERCLEANFILES = $(srcdir)/evdev.xml.in

rulesdir = $(xkb_base)/rules
xmldir = $(rulesdir)

@INTLTOOL_XML_NOMERGE_RULE@
