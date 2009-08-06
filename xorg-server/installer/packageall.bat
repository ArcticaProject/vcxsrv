if exist vcxsrv.*.installer.exe del vcxsrv.*.installer.exe
python genruntimeinclude.py
"C:\Program Files\NSIS\makensis.exe" vcxsrv.nsi