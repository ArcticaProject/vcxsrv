@echo off

set OUTFILE=base.ml_s.part

del %OUTFILE%

awk "{ printf """  *		%%s			=	pc+%%s\n""", $1, $2; }" layoutRename.lst >> %OUTFILE%

awk "{ printf """  *		%%s(%%s)			=	pc+%%s(%%s)\n""", $1, $2, $3, $4;}" variantRename.lst >> %OUTFILE%
