bin_SCRIPTS = x11app

.PHONY: x11app

x11app:
	xcodebuild CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" ARCHS="$(X11APP_ARCHS)"

install-data-hook:
	xcodebuild install DSTROOT="/$(DESTDIR)" INSTALL_PATH="$(APPLE_APPLICATIONS_DIR)" DEPLOYMENT_LOCATION=YES SKIP_INSTALL=NO ARCHS="$(X11APP_ARCHS)"

clean-local:
	rm -rf build

resourcedir=$(libdir)/X11/xserver
resource_DATA = Xquartz.plist

EXTRA_DIST = \
	$(resource_DATA) \
	Info.plist \
	X11.icns \
	bundle-main.c \
	X11.xcodeproj/project.pbxproj \
	Dutch.lproj/InfoPlist.strings \
	Dutch.lproj/Localizable.strings \
	Dutch.lproj/main.nib/keyedobjects.nib \
	English.lproj/InfoPlist.strings \
	English.lproj/Localizable.strings \
	English.lproj/main.nib/designable.nib \
	English.lproj/main.nib/keyedobjects.nib \
	French.lproj/InfoPlist.strings \
	French.lproj/Localizable.strings \
	French.lproj/main.nib/keyedobjects.nib \
	German.lproj/InfoPlist.strings \
	German.lproj/Localizable.strings \
	German.lproj/main.nib/keyedobjects.nib \
	Italian.lproj/InfoPlist.strings \
	Italian.lproj/Localizable.strings \
	Italian.lproj/main.nib/keyedobjects.nib \
	Japanese.lproj/InfoPlist.strings \
	Japanese.lproj/Localizable.strings \
	Japanese.lproj/main.nib/keyedobjects.nib \
	Spanish.lproj/InfoPlist.strings \
	Spanish.lproj/Localizable.strings \
	Spanish.lproj/main.nib/keyedobjects.nib \
	da.lproj/InfoPlist.strings \
	da.lproj/Localizable.strings \
	da.lproj/main.nib/keyedobjects.nib \
	fi.lproj/InfoPlist.strings \
	fi.lproj/Localizable.strings \
	fi.lproj/main.nib/keyedobjects.nib \
	ko.lproj/InfoPlist.strings \
	ko.lproj/Localizable.strings \
	ko.lproj/main.nib/keyedobjects.nib \
	no.lproj/InfoPlist.strings \
	no.lproj/Localizable.strings \
	no.lproj/main.nib/keyedobjects.nib \
	pl.lproj/InfoPlist.strings \
	pl.lproj/Localizable.strings \
	pl.lproj/main.nib/keyedobjects.nib \
	pt.lproj/InfoPlist.strings \
	pt.lproj/Localizable.strings \
	pt.lproj/main.nib/keyedobjects.nib \
	pt_PT.lproj/InfoPlist.strings \
	pt_PT.lproj/Localizable.strings \
	pt_PT.lproj/main.nib/keyedobjects.nib \
	ru.lproj/InfoPlist.strings \
	ru.lproj/Localizable.strings \
	ru.lproj/main.nib/keyedobjects.nib \
	sv.lproj/InfoPlist.strings \
	sv.lproj/Localizable.strings \
	sv.lproj/main.nib/keyedobjects.nib \
	zh_CN.lproj/InfoPlist.strings \
	zh_CN.lproj/Localizable.strings \
	zh_CN.lproj/main.nib/keyedobjects.nib \
	zh_TW.lproj/InfoPlist.strings \
	zh_TW.lproj/Localizable.strings \
	zh_TW.lproj/main.nib/keyedobjects.nib
