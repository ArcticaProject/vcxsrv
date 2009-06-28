#!/bin/sh

# $XFree86: xc/programs/Xserver/hw/xfree86/xf1bpp/mfbunmap.sh,v 1.1.2.1 1998/06/27 14:48:24 dawes Exp $
#
# This script recreates a header that undoes the effect of mfbmap.h
# This should only be rerun if there have been changes in the mfb code
#  that affect the external symbols.
#  It assumes that Xserver/mfb has been compiled.
# The output goes to stdout.
echo ""
echo "#ifdef _MFBMAP_H"
echo "#undef _MFBMAP_H"
echo ""

nm ../../../mfb/*.o | \
awk "{ if ((\$2 == \"D\") || (\$2 == \"T\") || (\$2 == \"C\")) print \$3 }" | \
sed s/^_// | \
grep -v "ModuleInit$" | \
sort | \
awk "{ print \"#undef \" \$1 }"

echo ""
echo "#endif"
