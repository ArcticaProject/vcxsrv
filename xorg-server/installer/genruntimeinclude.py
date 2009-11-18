PublicToken="1fc8b3b9a1e18e3b"

Template=r"""!define MSVC_PUBLICTOKEN   "<PUBLICTOKEN>"
!define MSVC_VERSION       "<VERSION>"
!define MSVCR90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcr90.dll"
!define MSVCM90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcm90.dll"
!define MSVCP90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcp90.dll"
!define MSVC_CAT           "<WINSXSDIR>\manifests\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>.cat"
!define MSVC_MANIFEST_PART "x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>.manifest"
!define MSVC_MANIFEST      "<WINSXSDIR>\manifests\${MSVC_MANIFEST_PART}"

!define MSVC_VERSION_D       "<VERSION>"
!define MSVCR90_DLL_D        "<WINSXSDIR>\x86_Microsoft.VC90.DebugCRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION_D}_<DSUFFIX>\msvcr90d.dll"
!define MSVCM90_DLL_D        "<WINSXSDIR>\x86_Microsoft.VC90.DebugCRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION_D}_<DSUFFIX>\msvcm90d.dll"
!define MSVCP90_DLL_D        "<WINSXSDIR>\x86_Microsoft.VC90.DebugCRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION_D}_<DSUFFIX>\msvcp90d.dll"
!define MSVC_CAT_D           "<WINSXSDIR>\manifests\x86_Microsoft.VC90.DebugCRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION_D}_<DSUFFIX>.cat"
!define MSVC_MANIFEST_PART_D "x86_Microsoft.VC90.DebugCRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION_D}_<DSUFFIX>.manifest"
!define MSVC_MANIFEST_D      "<WINSXSDIR>\manifests\${MSVC_MANIFEST_PART_D}"
"""

import glob,re,sys,os

Files=glob.glob(r"c:\windows\winsxs\x86_Microsoft.VC90.CRT_%s_9.0.*"%PublicToken)
DebugFiles=glob.glob(r"c:\windows\winsxs\x86_Microsoft.VC90.DebugCRT_%s_9.0.*"%PublicToken)

SearchRe=re.compile(r"c:\\windows\\winsxs\\x86_Microsoft\.VC90\.CRT_%s_9\.0\.([0-9]+)\.([0-9]+)_(.*)"%PublicToken,re.I)
DebugSearchRe=re.compile(r"c:\\windows\\winsxs\\x86_Microsoft\.VC90\.DebugCRT_%s_9\.0\.([0-9]+)\.([0-9]+)_(.*)"%PublicToken,re.I)

# Release Dlls
MajorVersion=0
MinorVersion=0
LatestFile=None
#Now Select the one with the latest version
for File in Files:
  # Extract version
  Search=SearchRe.search(File)
  Major=int(Search.group(1))
  Minor=int(Search.group(2))
  if Major>MajorVersion:
    MajorVersion=Major
    MinorVersion=Minor
    LatestFile=File
    Suffix=Search.group(3)
  elif Major==MajorVersion and Minor>MinorVersion:
    MinorVersion=Minor
    LatestFile=File
    Suffix=Search.group(3)


Template=re.sub("<VERSION>","%d.%d"%(MajorVersion,MinorVersion),Template)
Template=re.sub("<SUFFIX>",Suffix,Template)

# Debug dlls
MajorVersion=0
MinorVersion=0
LatestFile=None
#Now Select the one with the latest version
for File in DebugFiles:
  # Extract version
  Search=DebugSearchRe.search(File)
  Major=int(Search.group(1))
  Minor=int(Search.group(2))
  if Major>MajorVersion:
    MajorVersion=Major
    MinorVersion=Minor
    LatestFile=File
    Suffix=Search.group(3)
  elif Major==MajorVersion and Minor>MinorVersion:
    MinorVersion=Minor
    LatestFile=File
    Suffix=Search.group(3)


Template=re.sub("<DVERSION>","%d.%d"%(MajorVersion,MinorVersion),Template)
Template=re.sub("<DSUFFIX>",Suffix,Template)

# Adjust common stuff
Template=re.sub("<PUBLICTOKEN>",PublicToken,Template)
WinSxsDir=os.path.join(os.getenv("systemroot"),"WinSxS")
Template=re.sub("<WINSXSDIR>",WinSxsDir,Template)

open("runtime","w").write(Template)
