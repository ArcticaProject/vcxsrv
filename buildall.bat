nasm -h > nul 2>&1
if %errorlevel% NEQ 0 goto nasmerror

echo on

MSBuild.exe freetype\freetypevc10.sln /t:Build /p:Configuration="Release Multithreaded" /p:Platform=Win32
if %errorlevel% NEQ 0 goto end
MSBuild.exe freetype\freetypevc10.sln /t:Build /p:Configuration="Debug Multithreaded" /p:Platform=Win32
if %errorlevel% NEQ 0 goto end
cd openssl
perl Configure VC-WIN32
if %errorlevel% NEQ 0 goto end
call ms\do_nasm.bat
if %errorlevel% NEQ 0 goto end
nmake -f ms\nt.mak
if %errorlevel% NEQ 0 goto end
nmake DEBUG=1 -f ms\nt.mak
if %errorlevel% NEQ 0 goto end
cd ..\pthreads
nmake VC-static
if %errorlevel% NEQ 0 goto end
nmake VC-static-debug
if %errorlevel% NEQ 0 goto end
cd ..
MSBuild.exe tools\mhmake\mhmakevc10.sln /t:Build /p:Configuration=Release /p:Platform=Win32
if %errorlevel% NEQ 0 goto end
MSBuild.exe tools\mhmake\mhmakevc10.sln /t:Build /p:Configuration=Debug /p:Platform=Win32
if %errorlevel% NEQ 0 goto end

set MHMAKECONF=%~dp0

tools\mhmake\release\mhmake.exe -C xorg-server MAKESERVER=1 DEBUG=1
if %errorlevel% NEQ 0 goto end
tools\mhmake\release\mhmake.exe -C xorg-server MAKESERVER=1
if %errorlevel% NEQ 0 goto end

cd xorg-server\installer
call packageall.bat

goto end

:nasmerror
echo Please put nasm in the path
:end
