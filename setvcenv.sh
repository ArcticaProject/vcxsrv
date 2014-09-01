# Reference for MSVC 2012 with XP Compatibility:
# http://blogs.msdn.com/b/vcblog/archive/2012/10/08/10357555.aspx?PageIndex=2
# Note that LINK is specified in various makefiles and scripts

export MHMAKECONF=`cygpath -wa .`

export VisualStudioVersion="12.0"

export CL="/D_USING_V110_SDK71_"

export VCINSTALLDIR="c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\\"
export VS100COMNTOOLS="C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\\"
export VS110COMNTOOLS="C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\\"
export VS120COMNTOOLS="C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\\"
export VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 12.0\\"
export WindowsSdkDir="C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1A\\"
export WindowsSdkDir_35="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\\"
export WindowsSdkDir_old="C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1\bin\\"
export WindowsSdkDir_71A="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\bin\\"
export WindowsSdkDir_80="C:\Program Files (x86)\Microsoft SDKs\Windows\v8.0\bin\\"
export LIB="c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\lib;c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib"
export INCLUDE="c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"

export PATH="/cygdrive/c/Program Files/Microsoft SDKs/Windows/v7.1A/Bin:/vcxsrv:/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin:/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/IDE:/cygdrive/c/Windows/Microsoft.NET/Framework/v4.0.30319:/cygdrive/c/Program Files (x86)/Windows Kits/8.1/bin/x86/:$PATH"
