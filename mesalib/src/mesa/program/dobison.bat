@echo off
setlocal

cd "%~dp0"

set M4=..\..\..\..\tools\mhmake\m4.exe

set BISON_PKGDATADIR=../../../../tools/mhmake/src/bisondata

..\..\..\..\tools\mhmake\bison.exe -v -d --output=program_parse.tab.c program_parse.y

endlocal

