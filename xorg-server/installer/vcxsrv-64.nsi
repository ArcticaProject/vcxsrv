/*  This file is part of vcxsrv.
 *
 *  Copyright (C) 2009 Marc Haesen
 *
 *  vcxsrv is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vcxsrv is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vcxsrv.  If not, see <http://www.gnu.org/licenses/>.
*/
;--------------------------------

; The name of the installer
Name "VcXsrv (X2Go/Arctica Builds)"

; The file to write
OutFile "vcxsrv-64.1.15.2.5.x2go+arctica.installer.exe"

; The default installation directory
InstallDir $programfiles64\VcXsrv

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\VcXsrv "Install_Dir_64"

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

;--------------------------------
; The stuff to install
Section "VcXsrv (required)"

  SectionIn RO
  SectionIn 1 2 3

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Remove old opengl32.dll file if it extits
  IfFileExists "$INSTDIR\opengl32.dll" 0 +2
    Delete "$INSTDIR\opengl32.dll"
  IfFileExists "$INSTDIR\msvcr100.dll" 0 +2
    Delete "$INSTDIR\msvcr100.dll"
  IfFileExists "$INSTDIR\msvcp100.dll" 0 +2
    Delete "$INSTDIR\msvcp100.dll"
  IfFileExists "$INSTDIR\msvcr110.dll" 0 +2
    Delete "$INSTDIR\msvcr110.dll"
  IfFileExists "$INSTDIR\msvcp110.dll" 0 +2
    Delete "$INSTDIR\msvcp110.dll"

  ; Put files there
  File "..\obj64\servrelease\vcxsrv.exe"
  File "..\dix\protocol.txt"
  File "..\system.XWinrc"
  File "..\X0.hosts"
  File "..\..\xkbcomp\obj64\release\xkbcomp.exe"
  File "..\..\apps\xhost\obj64\release\xhost.exe"
  File "..\..\apps\xrdb\obj64\release\xrdb.exe"
  File "..\..\apps\xauth\obj64\release\xauth.exe"
  File "..\..\apps\xcalc\obj64\release\xcalc.exe"
  File "..\..\apps\xcalc\app-defaults\xcalc"
  File "..\..\apps\xcalc\app-defaults\xcalc-color"
  File "..\..\apps\xclock\obj64\release\xclock.exe"
  File "..\..\apps\xclock\app-defaults\xclock"
  File "..\..\apps\xclock\app-defaults\xclock-color"
  File "..\..\apps\xwininfo\obj64\release\xwininfo.exe"
  File "..\XKeysymDB"
  File "..\..\libX11\src\XErrorDB"
  File "..\..\libX11\src\xcms\Xcms.txt"
  File "..\XtErrorDB"
  File "..\.Xdefaults"
  File "..\hw\xwin\xlaunch\obj64\release\xlaunch.exe"
  File "..\..\tools\plink\obj64\release\plink.exe"
  File "..\..\mesalib\windows\VC8\mesa\x64\Release\swrast_dri.dll"
  File "..\hw\xwin\swrastwgl_dri\obj64\release\swrastwgl_dri.dll"
  File "..\..\dxtn\obj64\release\dxtn.dll"
  File "..\..\libxml2\bin64\libxml2-2.dll"
  File "..\..\libxml2\bin64\libgcc_s_sjlj-1.dll"
  File "..\..\libxml2\bin64\libiconv-2.dll"
  File "..\..\libxml2\bin64\libwinpthread-1.dll"
  File "..\..\zlib\obj64\release\zlib1.dll"
  File "..\..\libxcb\src\obj64\release\libxcb.dll"
  File "..\..\libXau\obj64\release\libXau.dll"
  File "..\..\libX11\obj64\release\libX11.dll"
  File "..\..\libXext\src\obj64\release\libXext.dll"
  File "..\..\libXmu\src\obj64\release\libXmu.dll"
  File "msvcr120.dll"
  File "msvcp120.dll"
  SetOutPath $INSTDIR\xkbdata
  File /r "..\xkbdata\*.*"
  SetOutPath $INSTDIR\locale
  File /r "..\locale\*.*"
  SetOutPath $INSTDIR\bitmaps
  File /r "..\bitmaps\*.*"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\VcXsrv "Install_Dir_64" $INSTDIR

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "DisplayName" "VcXsrv"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; Register the xlaunch file extension
  WriteRegStr HKCR ".xlaunch" "" "XLaunchFile"
  WriteRegStr HKCR "XLaunchFile" "" "XLaunch Configuration"
  WriteRegStr HKCR "XLaunchFile\DefaultIcon" "" "$INSTDIR\xlaunch.exe,0"
  WriteRegStr HKCR "XLaunchFile\shell" "" 'open'
  WriteRegStr HKCR "XLaunchFile\shell\open\command" "" '"$INSTDIR\XLaunch.exe" -run "%1"'
  WriteRegStr HKCR "XLaunchFile\shell\open\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "XLaunchFile\shell\open\ddeexec\Topic" "" "System"
  WriteRegStr HKCR "XLaunchFile\shell\edit\command" "" '"$INSTDIR\XLaunch.exe" -load "%1"'
  WriteRegStr HKCR "XLaunchFile\shell\edit\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "XLaunchFile\shell\edit\ddeexec\Topic" "" "System"
  WriteRegStr HKCR "XLaunchFile\shell\Validate\command" "" '"$INSTDIR\XLaunch.exe" -validate "%1"'
  WriteRegStr HKCR "XLaunchFile\shell\Validate\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "XLaunchFile\shell\Validate\ddeexec\Topic" "" "System"

  WriteRegStr HKCR "Applications\xlaunch.exe\shell" "" 'open'
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\open\command" "" '"$INSTDIR\XLaunch.exe" -run "%1"'
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\open\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\open\ddeexec\Topic" "" "System"
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\edit\command" "" '"$INSTDIR\XLaunch.exe" -load "%1"'
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\edit\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\edit\ddeexec\Topic" "" "System"
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\Validate\command" "" '"$INSTDIR\XLaunch.exe" -validate "%1"'
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\Validate\ddeexec\Application" "" "XLaunch"
  WriteRegStr HKCR "Applications\xlaunch.exe\shell\Validate\ddeexec\Topic" "" "System"

SectionEnd

; Optional section (can be disabled by the user)
Section "Fonts"
  SectionIn 1 3

  SetOutPath $INSTDIR\fonts
  CreateDirectory "$SMPROGRAMS\VcXsrv"
  File /r "..\fonts\*.*"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SectionIn 1 3

  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\VcXsrv"
  CreateShortCut "$SMPROGRAMS\VcXsrv\Uninstall VcXsrv.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\VcXsrv\VcXsrv.lnk" "$INSTDIR\vcxsrv.exe" " :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl" "$INSTDIR\vcxsrv.exe" 0
  CreateShortCut "$SMPROGRAMS\VcXsrv\XLaunch.lnk" "$INSTDIR\xlaunch.exe" "" "$INSTDIR\xlaunch.exe" 0

SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcuts"
  SectionIn 1 3

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\VcXsrv.lnk" "$INSTDIR\vcxsrv.exe" " :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl" "$INSTDIR\vcxsrv.exe" 0
  CreateShortCut "$DESKTOP\XLaunch.lnk" "$INSTDIR\xlaunch.exe" "" "$INSTDIR\xlaunch.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VcXsrv"
  DeleteRegKey HKLM SOFTWARE\VcXsrv

  ; Register the xlaunch file extension
  DeleteRegKey HKCR ".xlaunch"
  DeleteRegKey HKCR "XLaunchFile"
  DeleteRegKey HKCR "Applications\xlaunch.exe"

  ; Remove files and uninstaller
  RMDir /r "$INSTDIR"

  ; Remove shortcuts, if any
  RMDir /r "$SMPROGRAMS\VcXsrv"
  Delete "$DESKTOP\VcXsrv.lnk"
  Delete "$DESKTOP\XLaunch.lnk"

SectionEnd
