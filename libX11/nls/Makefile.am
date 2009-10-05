x11localedir = $(X11_LOCALEDATADIR)

EXTRA_DIST = locale.alias.pre compose.dir.pre locale.dir.pre

x11locale_DATA = locale.alias locale.dir compose.dir

CLEANFILES= \
	locale.alias locale.alias.l1 locale.alias.l2 \
	compose.dir compose.dir.l1 compose.dir.l2 \
	locale.dir locale.dir.l1 locale.dir.l2 \
	XLC_LOCALE

SUBDIRS= \
	am_ET.UTF-8		\
	armscii-8		\
	C			\
	el_GR.UTF-8		\
	en_US.UTF-8		\
	fi_FI.UTF-8		\
	georgian-academy	\
	georgian-ps		\
	ibm-cp1133		\
	iscii-dev		\
	isiri-3342		\
	iso8859-1		\
	iso8859-10		\
	iso8859-11		\
	iso8859-13		\
	iso8859-14		\
	iso8859-15		\
	iso8859-2		\
	iso8859-3		\
	iso8859-4		\
	iso8859-5		\
	iso8859-6		\
	iso8859-7		\
	iso8859-8		\
	iso8859-9		\
	iso8859-9e		\
	ja			\
	ja.JIS			\
	ja_JP.UTF-8		\
	ja.S90			\
	ja.SJIS			\
	ja.U90			\
	ko			\
	koi8-c			\
	koi8-r			\
	koi8-u			\
	ko_KR.UTF-8		\
	microsoft-cp1251	\
	microsoft-cp1255	\
	microsoft-cp1256	\
	mulelao-1		\
	nokhchi-1		\
	pt_BR.UTF-8		\
	ru_RU.UTF-8		\
	tatar-cyr		\
	th_TH			\
	th_TH.UTF-8		\
	tscii-0			\
	vi_VN.tcvn		\
	vi_VN.viscii		\
	zh_CN			\
	zh_CN.gb18030		\
	zh_CN.gbk		\
	zh_CN.UTF-8		\
	zh_HK.big5		\
	zh_HK.big5hkscs		\
	zh_HK.UTF-8		\
	zh_TW			\
	zh_TW.big5		\
	zh_TW.UTF-8

include $(top_srcdir)/cpprules.in

locale.alias: locale.alias.pre
	$(cpp_verbose)$(RAWCPP) $(RAWCPPFLAGS) $(CPP_FILES_FLAGS) < $(srcdir)/locale.alias.pre | $(CPP_SED_MAGIC) > locale.alias.l1
	sed -e '/^[^#][^        ]*:/s/://' -e '/^[^#].*[        ].*:/d' \
        < locale.alias.l1 > locale.alias.l2
	cat locale.alias.l2 locale.alias.l1 > locale.alias

compose.dir: compose.dir.pre
	$(cpp_verbose)$(RAWCPP) $(RAWCPPFLAGS) $(CPP_FILES_FLAGS) < $(srcdir)/compose.dir.pre | $(CPP_SED_MAGIC) > compose.dir.l1
	sed -e '/^[^#][^        ]*:/s/://' -e '/^[^#].*[        ].*:/d' \
        < compose.dir.l1 > compose.dir.l2
	cat compose.dir.l2 compose.dir.l1 > compose.dir

locale.dir: locale.dir.pre
	$(cpp_verbose)$(RAWCPP) $(RAWCPPFLAGS) $(CPP_FILES_FLAGS) < $(srcdir)/locale.dir.pre | $(CPP_SED_MAGIC) > locale.dir.l1
	sed -e '/^[^#][^        ]*:/s/://' -e '/^[^#].*[        ].*:/d' \
        < locale.dir.l1 > locale.dir.l2
	cat locale.dir.l2 locale.dir.l1 > locale.dir
