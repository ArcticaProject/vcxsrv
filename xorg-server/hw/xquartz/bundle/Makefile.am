include cpprules.in

CPP_FILES_FLAGS = \
	-DLAUNCHD_ID_PREFIX="$(LAUNCHD_ID_PREFIX)" \
	-DAPPLE_APPLICATION_NAME="$(APPLE_APPLICATION_NAME)"

if XQUARTZ_SPARKLE
CPP_FILES_FLAGS += -DXQUARTZ_SPARKLE
endif

install-data-hook:
	$(srcdir)/mk_bundke.sh $(srcdir) $(builddir) $(DESTDIR)$(APPLE_APPLICATIONS_DIR)/$(APPLE_APPLICATION_NAME).app install

uninstall-hook:
	$(RM) -rf $(DESTDIR)$(APPLE_APPLICATIONS_DIR)/$(APPLE_APPLICATION_NAME).app	

noinst_PRE = Info.plist.cpp
noinst_DATA = $(noinst_PRE:plist.cpp=plist)

CLEANFILES = $(noinst_DATA)

resourcedir=$(libdir)/X11/xserver
resource_DATA = Xquartz.plist

EXTRA_DIST = \
	mk_bundke.sh \
	X11.sh \
	Info.plist.cpp \
	PkgInfo \
	$(resource_DATA) \
	Resources/da.lproj/InfoPlist.strings \
	Resources/da.lproj/Localizable.strings \
	Resources/da.lproj/main.nib/keyedobjects.nib \
	Resources/Dutch.lproj/InfoPlist.strings \
	Resources/Dutch.lproj/Localizable.strings \
	Resources/Dutch.lproj/main.nib/keyedobjects.nib \
	Resources/English.lproj/InfoPlist.strings \
	Resources/English.lproj/Localizable.strings \
	Resources/English.lproj/main.nib/designable.nib \
	Resources/English.lproj/main.nib/keyedobjects.nib \
	Resources/fi.lproj/InfoPlist.strings \
	Resources/fi.lproj/Localizable.strings \
	Resources/fi.lproj/main.nib/keyedobjects.nib \
	Resources/French.lproj/InfoPlist.strings \
	Resources/French.lproj/Localizable.strings \
	Resources/French.lproj/main.nib/keyedobjects.nib \
	Resources/German.lproj/InfoPlist.strings \
	Resources/German.lproj/Localizable.strings \
	Resources/German.lproj/main.nib/keyedobjects.nib \
	Resources/Italian.lproj/InfoPlist.strings \
	Resources/Italian.lproj/Localizable.strings \
	Resources/Italian.lproj/main.nib/keyedobjects.nib \
	Resources/Japanese.lproj/InfoPlist.strings \
	Resources/Japanese.lproj/Localizable.strings \
	Resources/Japanese.lproj/main.nib/keyedobjects.nib \
	Resources/ko.lproj/InfoPlist.strings \
	Resources/ko.lproj/Localizable.strings \
	Resources/ko.lproj/main.nib/keyedobjects.nib \
	Resources/no.lproj/InfoPlist.strings \
	Resources/no.lproj/Localizable.strings \
	Resources/no.lproj/main.nib/keyedobjects.nib \
	Resources/pl.lproj/InfoPlist.strings \
	Resources/pl.lproj/Localizable.strings \
	Resources/pl.lproj/main.nib/keyedobjects.nib \
	Resources/pt.lproj/InfoPlist.strings \
	Resources/pt.lproj/Localizable.strings \
	Resources/pt.lproj/main.nib/keyedobjects.nib \
	Resources/pt_PT.lproj/InfoPlist.strings \
	Resources/pt_PT.lproj/Localizable.strings \
	Resources/pt_PT.lproj/main.nib/keyedobjects.nib \
	Resources/ru.lproj/InfoPlist.strings \
	Resources/ru.lproj/Localizable.strings \
	Resources/ru.lproj/main.nib/keyedobjects.nib \
	Resources/Spanish.lproj/InfoPlist.strings \
	Resources/Spanish.lproj/Localizable.strings \
	Resources/Spanish.lproj/main.nib/keyedobjects.nib \
	Resources/sv.lproj/InfoPlist.strings \
	Resources/sv.lproj/Localizable.strings \
	Resources/sv.lproj/main.nib/keyedobjects.nib \
	Resources/X11.icns \
	Resources/zh_CN.lproj/InfoPlist.strings \
	Resources/zh_CN.lproj/Localizable.strings \
	Resources/zh_CN.lproj/main.nib/keyedobjects.nib \
	Resources/zh_TW.lproj/InfoPlist.strings \
	Resources/zh_TW.lproj/Localizable.strings \
	Resources/zh_TW.lproj/main.nib/keyedobjects.nib
