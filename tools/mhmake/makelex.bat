@echo off
setlocal
if not exist src\parser mkdir src\parser

"flex++" -8 -Ssrc\flexskel.cc -Hsrc\flexskel.h -hsrc\parser\mhmakelexer.h -osrc\parser\mhmakelexer.cpp src\mhmakelexer.l

set file=src\parser\mhmakelexer.cpp
set tempfile=src\parser\temp12345.5678

move %file% %tempfile%
echo #include "stdafx.h" > %file%
type %tempfile% >> %file%
del /q %tempfile%

endlocal

