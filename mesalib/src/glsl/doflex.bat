@echo off
setlocal

cd "%~dp0"

set M4=..\..\..\tools\mhmake\m4.exe

set path=..\..\..\tools\mhmake;%path%

copy "..\..\..\tools\mhmake\flex++.exe" flex.exe

flex.exe --nounistd -oglsl_lexer.cpp glsl_lexer.ll
flex.exe --nounistd -oglcpp/glcpp-lex.c glcpp/glcpp-lex.l
del flex.exe

endlocal

