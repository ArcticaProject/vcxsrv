setlocal
if not isdir src\parser mkdir src\parser

bison++ -d -Ssrc\bison.cc -Hsrc\bison.h -hsrc\parser\mhmakeparser.h -osrc\parser\mhmakeparser.cpp src\mhmakeparser.y

set file=src\parser\mhmakeparser.cpp
set tempfile=src\parser\temp12345.5678
GOSUB AddStdAfx

goto END

:AddStdAfx
rename /q %file %tempfile
echo #include "stdafx.h" > %file
type %tempfile >> %file
del /q %tempfile
RETURN

:END
endlocal

