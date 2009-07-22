@echo off

set OUTFILE=base.mlv_s.part

del %OUTFILE%

awk "{ printf """  *		%%s		%%s		=	pc+%%s(%%s)\n""", $1, $2, $3, $4; }" variantRename.lst >> %OUTFILE%

