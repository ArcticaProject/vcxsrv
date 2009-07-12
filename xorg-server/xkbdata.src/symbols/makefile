ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=symbols

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)
$(DESTDIR):
	if not isdir "$(DESTDIR)" mkdir /s "$(DESTDIR)"

SUBDIRS = pc macintosh sun fujitsu nec digital hp sgi sony xfree68

dist_xkbdata_symbols_DATA = \
	al \
	altwin \
	am \
	apple \
	ar \
	az \
	be \
	ben \
	bg \
	br \
	bs \
	by \
	ca \
	ca_enhanced \
	capslock \
	compose \
	ctrl \
	cz \
	cz_qwerty \
	czsk \
	de \
	de_CH \
	dev \
	dk \
	dvorak \
	ee \
	el \
	en_US \
	es \
	eurosign \
	fi \
	fo \
	fr \
	fr_CH \
	gb \
	ge_la \
	ge_ru \
	group \
	guj \
	gur \
	hr \
	hr_US \
	hu \
	hu_qwerty \
	hu_US \
	ie \
	il \
	il_phonetic \
	inet \
	ir \
	is \
	iso9995-3 \
	it \
	iu \
	jp \
	kan \
	keypad \
	la \
	level3 \
	lo \
	lock \
	lt \
	lt_a \
	lt_p \
	lt_std \
	lv \
	mk \
	ml \
	mm \
	mn \
	mt \
	mt_us \
	nl \
	no \
	ogham \
	ori \
	pc104 \
	pl \
	pl2 \
	pt \
	ralt \
	README \
	ro \
	ro2 \
	ru \
	sapmi \
	se \
	se_FI \
	se_NO \
	se_SE \
	si \
	sk \
	sk_qwerty \
	sr \
	srvr_ctrl \
	syr \
	syr_phonetic \
	tel \
	th \
	th_pat \
	th_tis \
	tj \
	tml \
	tr \
	tr_f \
	ua \
	us \
	us_group2 \
	us_group3 \
	us_intl \
	uz \
	vn \
	yu

DATA_FILES=$(dist_xkbdata_symbols_DATA:%=$(DESTDIR)\%)

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

.PHONY: extrastuff

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
