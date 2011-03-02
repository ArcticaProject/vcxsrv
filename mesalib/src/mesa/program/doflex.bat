@echo off
setlocal

cd "%~dp0"

set M4=..\..\..\..\tools\mhmake\m4.exe

set path=..\..\..\..\tools\mhmake;%path%

copy "..\..\..\..\tools\mhmake\flex++.exe" flex.exe

 
flex.exe --never-interactive --outfile=lex.yy.c program_lexer.l

del flex.exe

endlocal

