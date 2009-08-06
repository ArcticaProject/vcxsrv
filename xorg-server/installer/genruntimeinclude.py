PublicToken="1fc8b3b9a1e18e3b"

Template=r"""!define MSVC_PUBLICTOKEN   "<PUBLICTOKEN>"
!define MSVC_VERSION       "<VERSION>"
!define MSVCR90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcr90.dll"
!define MSVCM90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcm90.dll"
!define MSVCP90_DLL        "<WINSXSDIR>\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>\msvcp90.dll"
!define MSVC_CAT           "<WINSXSDIR>\manifests\x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>.cat"
!define MSVC_MANIFEST_PART "x86_Microsoft.VC90.CRT_${MSVC_PUBLICTOKEN}_9.0.${MSVC_VERSION}_<SUFFIX>.manifest"
!define MSVC_MANIFEST      "<WINSXSDIR>\manifests\${MSVC_MANIFEST_PART}"
"""

import glob,re,sys,os

Files=glob.glob(r"c:\windows\winsxs\x86_Microsoft.VC90.CRT_%s_9.0.*"%PublicToken)

SearchRe=re.compile(r"c:\\windows\\winsxs\\x86_Microsoft\.VC90\.CRT_%s_9\.0\.([0-9]+)\.([0-9]+)_(.*)"%PublicToken,re.I)

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
Template=re.sub("<PUBLICTOKEN>",PublicToken,Template)
Template=re.sub("<SUFFIX>",Suffix,Template)

WinSxsDir=os.path.join(os.getenv("systemroot"),"WinSxS")
Template=re.sub("<WINSXSDIR>",WinSxsDir,Template)

if len(sys.argv)==3 and sys.argv[2]=="1":
  Template=re.sub("<DEBUG>","Debug",Template)
else:
  Template=re.sub("<DEBUG>","",Template)

open("runtime","w").write(Template)
