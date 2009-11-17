Template=r"""<?xml version='1.0' encoding='UTF-8' standalone='yes'?>
<assembly xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0'>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level='asInvoker' uiAccess='false' />
      </requestedPrivileges>
    </security>
  </trustInfo>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity type="win32" name="Microsoft.VC90.<DEBUG>CRT" version="9.0.<VERSION>" processorArchitecture="x86" publicKeyToken="1fc8b3b9a1e18e3b" />
    </dependentAssembly>
  </dependency>
</assembly>
"""

import glob,re,sys

if len(sys.argv)==3 and sys.argv[2]=="1":
  Files=glob.glob(r"c:\windows\winsxs\x86_Microsoft.VC90.DebugCRT_1fc8b3b9a1e18e3b_9.0.*")
  SearchRe=re.compile(r"c:\\windows\\winsxs\\x86_Microsoft\.VC90\.DebugCRT_1fc8b3b9a1e18e3b_9\.0\.([0-9]+)\.([0-9]+)_",re.I)
else:
  Files=glob.glob(r"c:\windows\winsxs\x86_Microsoft.VC90.CRT_1fc8b3b9a1e18e3b_9.0.*")
  SearchRe=re.compile(r"c:\\windows\\winsxs\\x86_Microsoft\.VC90\.CRT_1fc8b3b9a1e18e3b_9\.0\.([0-9]+)\.([0-9]+)_",re.I)


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
  elif Major==MajorVersion and Minor>MinorVersion:
    MinorVersion=Minor
    LatestFile=File


Template=re.sub("<VERSION>","%d.%d"%(MajorVersion,MinorVersion),Template)

if len(sys.argv)==3 and sys.argv[2]=="1":
  Template=re.sub("<DEBUG>","Debug",Template)
else:
  Template=re.sub("<DEBUG>","",Template)

open(sys.argv[1],"w").write(Template)
