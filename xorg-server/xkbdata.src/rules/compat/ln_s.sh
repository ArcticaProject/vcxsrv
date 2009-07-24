@echo off
set variant=%1

set OUTFILE=base.l%variant%_s.part

del %OUTFILE%

gawk "{ if (index($2, """(""") == 0) { printf """  %%s		=	+%%s%%%%(v[%variant%]):%variant%\n""", $1, $2;} else { printf """  %%s		=	+%%s:%variant%\n""", $1, $2; } }" layoutRename.lst >> %OUTFILE%

gawk "{ printf """  %%s(%%s)	=	+%%s(%%s):%variant%\n""", $1, $2, $3, $4; }" variantRename.lst >> %OUTFILE%
