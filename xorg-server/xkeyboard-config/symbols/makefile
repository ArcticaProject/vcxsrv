SUBDIRS = digital_vndr fujitsu_vndr hp_vndr macintosh_vndr nec_vndr nokia_vndr sgi_vndr sony_vndr sun_vndr xfree68_vndr extras

ifeq ($(MAKESERVER),1)
$(error Please do not specify MAKESERVER=1)
endif

THISDIR=symbols

DESTDIR=$(MHMAKECONF)\xorg-server\xkbdata\$(THISDIR)

dist_xkbdata_symbols_DATA = \
ad af al \
am ara at az \
ba bd be \
bg br brai \
bt bw by \
ca cd \
ch cn cz \
de dk \
ee es et epo \
fi fo fr \
gb ge gh gn \
gr hr hu \
ie il in iq \
ir is it jp \
ke kg kh \
kr kz \
la latam latin \
lk lt lv \
ma mao me \
mk ml mm \
mn mt mv \
ng nl no np \
pc pk pl pt \
ro rs ru \
se si sk sn \
sy th \
terminate \
tj tm tr tz \
ua us uz vn \
za \
altwin capslock compose ctrl empty eurosign rupeesign group inet \
keypad kpdl level3 level5 nbsp olpc shift srvr_ctrl typo

DATA_FILES=$(dist_xkbdata_symbols_DATA:%=$(DESTDIR)\%)

load_makefile $(SUBDIRS:%=%\makefile MAKESERVER=0 DEBUG=$(DEBUG);)

extrastuff: $(SUBDIRS:%=%\all)

include ..\xkbrules.mak
