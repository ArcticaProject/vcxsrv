AUTOMAKE_OPTIONS = foreign

SUBDIRS = compat geometry keycodes keymap po rules semantics symbols types docs

EXTRA_DIST=config.rpath COPYING CREDITS README \
       autogen.sh \
       xkeyboard-config.spec \
       xkeyboard-config.spec.in \
       intltool-extract.in intltool-merge.in intltool-update.in

DISTCLEANFILES=intltool-extract intltool-merge intltool-update

# ACLOCAL_AMFLAGS = -I m4

if CREATE_XKBCOMP_SYMLINK
install-xkbcomp:
	ln -sf $(XKBCOMP) $(DESTDIR)$(xkb_base)/xkbcomp

else
install-xkbcomp:

endif

install-data-hook: install-xkbcomp

sync-po:
	rsync -Lrtvz translationproject.org::tp/latest/xkeyboard-config/  po
