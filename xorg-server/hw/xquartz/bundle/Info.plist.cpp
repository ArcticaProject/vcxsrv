<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
		<string>English</string>
	<key>CFBundleExecutable</key>
		<string>X11</string>
	<key>CFBundleGetInfoString</key>
		<string>LAUNCHD_ID_PREFIX.X11</string>
	<key>CFBundleIconFile</key>
		<string>X11.icns</string>
	<key>CFBundleIdentifier</key>
		<string>LAUNCHD_ID_PREFIX.X11</string>
	<key>CFBundleInfoDictionaryVersion</key>
		<string>6.0</string>
	<key>CFBundleName</key>
		<string>APPLE_APPLICATION_NAME</string>
	<key>CFBundlePackageType</key>
		<string>APPL</string>
	<key>CFBundleShortVersionString</key>
		<string>2.5.1</string>
	<key>CFBundleVersion</key>
		<string>2.5.1</string>
	<key>CFBundleSignature</key>
		<string>x11a</string>
	<key>CSResourcesFileMapped</key>
		<true/>
#ifdef XQUARTZ_SPARKLE
	<key>SUEnableAutomaticChecks</key>
		<true/>
	<key>SUPublicDSAKeyFile</key>
		<string>sparkle.pem</string>
        <key>SUFeedURL</key>
                <string>http://xquartz.macosforge.org/downloads/sparkle/release.xml</string>
#endif
	<key>NSHumanReadableCopyright</key>
		<string>© 2003-2010 Apple Inc.
© 2003 XFree86 Project, Inc.
© 2003-2010 X.org Foundation, Inc.
</string>
	<key>NSMainNibFile</key>
		<string>main</string>
	<key>NSPrincipalClass</key>
		<string>X11Application</string>
</dict>
</plist>
