/*  This file is part of vcxsrv.
 *
 *  Copyright (C) 2014 marha@users.sourceforge.net
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

!define NAME_STRING "VcXsrv (X2Go/Arctica Builds)"
!define VERSION "1.17.0.0-2-bug3test2"

; The name of the installer
Name "${NAME_STRING}"

; The file to write
OutFile "vcxsrv-64-debug.${VERSION}.x2go+arctica.installer.exe"

; The default installation directory
InstallDir $PROGRAMFILES64\VcXsrv

; NSIS 2.46 defaults to zlib.
; Setting this reduces the size of a build from 7.0 to 4.6 MB (34%)
SetCompressor /SOLID lzma

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\VcXsrv "Install_Dir_64"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

VIProductVersion "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${NAME_STRING}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "VcXsrv windows xserver (X2Go/Arctica Builds)"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
InstType "Full"

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
Section "VcXsrv debug exe and dlls"

  SectionIn RO
  SectionIn 1

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  IfFileExists "$INSTDIR\msvcr100d.dll" 0 +2
    Delete "$INSTDIR\msvcr100d.dll"
  IfFileExists "$INSTDIR\msvcp100d.dll" 0 +2
    Delete "$INSTDIR\msvcp100d.dll"
  IfFileExists "$INSTDIR\msvcr110d.dll" 0 +2
    Delete "$INSTDIR\msvcr110d.dll"
  IfFileExists "$INSTDIR\msvcp110d.dll" 0 +2
    Delete "$INSTDIR\msvcp110d.dll"

  ; Put files there
  File "..\obj64\servdebug\vcxsrv.exe"
  File "..\..\xkbcomp\obj64\debug\xkbcomp.exe"
  File "..\..\apps\xhost\obj64\debug\xhost.exe"
  File "..\..\apps\xrdb\obj64\debug\xrdb.exe"
  File "..\..\apps\xauth\obj64\debug\xauth.exe"
  File "..\..\apps\xcalc\obj64\debug\xcalc.exe"
  File "..\..\apps\xclock\obj64\debug\xclock.exe"
  File "..\..\apps\xwininfo\obj64\debug\xwininfo.exe"
  File "..\hw\xwin\xlaunch\obj64\debug\xlaunch.exe"
  File "..\..\tools\plink\obj64\debug\plink.exe"
  File "..\..\mesalib\windows\VC8\mesa\x64\Debug\swrast_dri.dll"
  File "..\hw\xwin\swrastwgl_dri\obj64\debug\swrastwgl_dri.dll"
  File "..\..\dxtn\obj64\debug\dxtn.dll"
  File "..\..\zlib\obj64\debug\zlib1.dll"
  File "..\..\libxcb\src\obj64\debug\libxcb.dll"
  File "..\..\libXau\obj64\debug\libXau.dll"
  File "..\..\libX11\obj64\debug\libX11.dll"
  File "..\..\libXext\src\obj64\debug\libXext.dll"
  File "..\..\libXmu\src\obj64\debug\libXmu.dll"
  File "msvcr120d.dll"
  File "msvcp120d.dll"

  WriteRegStr HKLM SOFTWARE\VcXsrv "Install_Dir_64" $INSTDIR
SectionEnd
