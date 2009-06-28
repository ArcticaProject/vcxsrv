#!/bin/sh

variant=$1

INDIR=`dirname $0`
OUTFILE=base.l${variant}v${variant}_s.part

> $OUTFILE

awk '{ 
  printf "  %s		%s	=	+%s(%s):'${variant}'\n", $1, $2, $3, $4; 
}' < $INDIR/variantRename.lst >> $OUTFILE
