@echo off
setlocal

cd "%~dp0"

set M4=..\..\..\tools\mhmake\m4.exe

set BISON_PKGDATADIR=../../../tools/mhmake/src/bisondata

..\..\..\tools\mhmake\bison.exe -v -o glsl_parser.cpp -p "_mesa_glsl_" --defines=glsl_parser.h glsl_parser.yy

..\..\..\tools\mhmake\bison.exe -v -o glcpp/glcpp-parse.c --defines=glcpp/glcpp-parse.h glcpp/glcpp-parse.y


endlocal

