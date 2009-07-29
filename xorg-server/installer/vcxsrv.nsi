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
OutFile "vcxsrv.1.0.installer.exe"

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

;--------------------------------

; The stuff to install
Section "VcXsrv (required)"

  SectionIn RO
  SectionIn 1 2

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put files there
  File "..\obj\servrelease\vcxsrv.exe"
  File "..\protocol.txt"
  File "..\system.XWinrc"
  File "..\..\xkbcomp\obj\release\xkbcomp.exe"
  File "..\xkeysymdb"
  File "..\hw\xwin\xlaunch\obj\release\xlaunch.exe"
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

SectionEnd
