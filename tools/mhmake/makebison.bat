@echo off
setlocal

cd %1
"..\bison++" -d -S%3 -H%4 -h%5 -o%6 %2

set file=%6
set tempfile=%RANDOM%bison.tmp

move %file% %tempfile%
echo #include "stdafx.h" > %file%
type %tempfile% >> %file%
del /q %tempfile%

endlocal

