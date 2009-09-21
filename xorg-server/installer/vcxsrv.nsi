; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "VcXsrv"

; The file to write
OutFile "vcxsrv.1.1.1.installer.exe"

; The default installation directory
InstallDir $PROGRAMFILES\VcXsrv

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\VcXsrv" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
InstType "Full"
InstType "Minimal"

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

SetPluginUnload alwaysoff
; ShowInstDetails show
XPStyle on

!define FUSION_REFCOUNT_UNINSTALL_SUBKEY_GUID {8cedc215-ac4b-488b-93c0-a50a49cb2fb8}

!include runtime

;--------------------------------

; The stuff to install
Section "VcXsrv (required)"

  SectionIn RO
  SectionIn 1 2

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put files there
  File "..\obj\servrelease\vcxsrv.exe"
  File "..\obj\servdebug\vcxsrv_dbg.exe"
  File "..\obj\servdebug\vcxsrv_dbg.pdb"
  File "..\protocol.txt"
  File "..\system.XWinrc"
  File "..\..\xkbcomp\obj\release\xkbcomp.exe"
  File "..\xkeysymdb"
  File "..\hw\xwin\xlaunch\obj\release\xlaunch.exe"
  File "..\..\tools\plink\obj\release\plink.exe"
  SetOutPath $INSTDIR\fonts
  File /r "..\fonts\*.*"
  SetOutPath $INSTDIR\xkbdata
  File /r "..\xkbdata\*.*"
  SetOutPath $INSTDIR\locale
  File /r "..\locale\*.*"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\VcXsrv "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "DisplayName" "VcXsrv"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  InitPluginsDir
  SetOutPath $PLUGINSDIR
  File "${MSVCR90_DLL}"
  File "${MSVCM90_DLL}"
  File "${MSVCP90_DLL}"
  File "${MSVC_CAT}"
  File "${MSVC_MANIFEST}"
  DetailPrint "Installing CRT assembly..."
  System::Call "sxs::CreateAssemblyCache(*i .r0, i 0) i.r1"
  StrCmp $1 0 0 fail
  # Fill a FUSION_INSTALL_REFERENCE.
  # fir.cbSize = sizeof(FUSION_INSTALL_REFERENCE) == 32
  # fir.dwFlags = 0
  # fir.guidScheme = FUSION_REFCOUNT_UNINSTALL_SUBKEY_GUID
  # fir.szIdentifier = "nsissxs"
  # fir.szNonCanonicalData = 0
  System::Call "*(i 32, i 0, i 2364391957, i 1217113163, i 178634899, i 3090139977, w 'nsissxs', w '') i.s"
  Pop $2
  # IAssemblyCache::InstallAssembly(0, manifestPath, fir)
  System::Call "$0->7(i 0, w '$PLUGINSDIR\${MSVC_MANIFEST_PART}', i r2) i.r1"
  System::Free $2
  StrCmp $1 0 0 fail2
  System::Call "$0->2()"
  Goto end
  
fail:
  DetailPrint "CreateAssemblyCache failed."
  DetailPrint $1
  Goto end

fail2:
  DetailPrint "InstallAssembly failed."
  DetailPrint $1
  Goto end
end:
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SectionIn 1

  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\VcXsrv"
  CreateShortCut "$SMPROGRAMS\VcXsrv\Uninstall VcXsrv.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\VcXsrv\VcXsrv.lnk" "$INSTDIR\vcxsrv.exe" " :1 -ac -terminate -lesspointer -multiwindow -clipboard +kb" "$INSTDIR\vcxsrv.exe" 0
  CreateShortCut "$SMPROGRAMS\VcXsrv\XLaunch.lnk" "$INSTDIR\xlaunch.exe" "" "$INSTDIR\xlaunch.exe" 0

SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcuts"
  SectionIn 1

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\VcXsrv.lnk" "$INSTDIR\vcxsrv.exe" " :1 -ac -terminate -lesspointer -multiwindow -clipboard +kb" "$INSTDIR\vcxsrv.exe" 0
  CreateShortCut "$DESKTOP\XLaunch.lnk" "$INSTDIR\xlaunch.exe" "" "$INSTDIR\xlaunch.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv"
  DeleteRegKey HKLM SOFTWARE\VcXsrv

  ; Remove files and uninstaller
  Delete "$INSTDIR\vcxsrv.exe"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\protocol.txt"
  Delete "$INSTDIR\system.XWinrc"
  Delete "$INSTDIR\xkbcomp.exe"
  Delete "$INSTDIR\xkeysymdb"
  Delete "$INSTDIR\xlaunch.exe"

  RMDir /r "$INSTDIR\fonts"
  RMDir /r "$INSTDIR\xkbdata"
  RMDir /r "$INSTDIR\locale"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\VcXsrv\*.*"
  Delete "$DESKTOP\VcXsrv.lnk"
  Delete "$DESKTOP\XLaunch.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\VcXsrv"
  RMDir "$INSTDIR"

  DetailPrint "Removing CRT assembly..."
  System::Call "sxs::CreateAssemblyCache(*i .r0, i 0) i.r1"
  StrCmp $1 0 0 fail
  System::Call "*(i 32, i 0, i 2364391957, i 1217113163, i 178634899, i 3090139977, w 'nsissxs', w '') i.s"
  Pop $2
  System::Call "$0->3(i 0, w 'Microsoft.VC90.CRT,version=$\"9.0.${MSVC_VERSION}$\",type=$\"win32$\",processorArchitecture=$\"x86$\",publicKeyToken=$\"${MSVC_PUBLICTOKEN}$\"', i r2, *i . r3) i.r1"
  StrCmp $1 0 0 fail2
  DetailPrint "Disposition returned is $3"
  System::Call "$0->2()"
  Goto end
  fail:
  DetailPrint "CreateAssemblyCache failed."
  DetailPrint $1
  Goto end
  fail2:
  DetailPrint "UninstallAssembly failed."
  DetailPrint $1
  Goto end
end:

SectionEnd

