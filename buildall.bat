nasm >& nul
if errorlevel NE 0 goto nasmerror

echo on

devenv.com freetype\freetypevc10.sln /build "Release Multithreaded|Win32"
devenv.com freetype\freetypevc10.sln /build "Debug Multithreaded|Win32"
cd openssl
perl Configure VC-WIN32
call ms\do_nasm.bat
nmake -f ms\nt.mak
nmake DEBUG=1 -f ms\nt.mak
cd ..\pthreads
nmake VC-static
nmake VC-static-debug
cd ..
devenv.com tools\mhmake\mhmakevc10.sln /build "Release|Win32"
devenv.com tools\mhmake\mhmakevc10.sln /build "Debug|Win32"

set MHMAKECONF=%~dp0

tools\mhmake\release\mhmake.exe -C xorg-server MAKESERVER=1 DEBUG=1 vcxsrv_dbg.exe
tools\mhmake\release\mhmake.exe -C xorg-server MAKESERVER=1

cd xorg-server\installer
call packageall.bat

goto end

:nasmerror
echo Please put nasm in the path
:end