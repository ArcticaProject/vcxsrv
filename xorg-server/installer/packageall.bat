@echo off
if exist vcxsrv.*.installer.exe del vcxsrv.*.installer.exe
if "%VS2008%"=="1" goto vs2008

copy %systemroot%\system32\msvcr100.dll
copy %systemroot%\system32\msvcp100.dll
copy %systemroot%\system32\msvcr100d.dll
"C:\Program Files\NSIS\makensis.exe" vcxsrv.nsi
del msvcr100.dll
del msvcr100d.dll
del msvcp100.dll

goto end
:vs2008

python genruntimeinclude.py
"C:\Program Files\NSIS\makensis.exe" /DVS2008=1 vcxsrv.nsi
del runtime

:end

