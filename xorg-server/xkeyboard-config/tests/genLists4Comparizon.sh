#!/bin/sh

#
# This script compares the group names which "have to be", according to the descriptions in base.xml -
# and actually existing in the symbol files. Some differences are ok (like extra double quotes or 
# extra escaping character) - but all the rest should be in sync.
#

ROOT="`dirname $0`/.."
F1=reg2ll.lst
F2=gn2ll.lst

xsltproc $ROOT/xslt/reg2ll.xsl $ROOT/rules/base.xml | sort | uniq > $F1

for i in $ROOT/symbols/*; do
  if [ -f $i ]; then
    id="`basename $i`"
    export id
    gawk 'BEGIN{
  FS = "\"";
  id = ENVIRON["id"];
  isDefault = 0;
}
/.*default.*/{
  isDefault = 1;
}
/xkb_symbols/{
  variant = $2;
}/^[[:space:]]*name\[Group1\][[:space:]]*=/{
  if (isDefault == 1)
  {
    printf "%s:\"%s\"\n",id,$2;
    isDefault=0;
  } else
  {
    name=$2;
    printf "%s(%s):\"%s\"\n", id, variant, name;
  }
}' $i
  fi
done | sort | uniq > $F2

diff $F1 $F2
