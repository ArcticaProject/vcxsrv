#!/bin/sh

INDIR=`dirname $0`
OUTFILE=base.ml_s.part

> $OUTFILE

awk '{ 
  printf "  *		%s			=	pc+%s\n", $1, $2; 
}' < $INDIR/layoutRename.lst >> $OUTFILE

awk '{ 
  printf "  *		%s(%s)			=	pc+%s(%s)\n", $1, $2, $3, $4; 
}' < $INDIR/variantRename.lst >> $OUTFILE
