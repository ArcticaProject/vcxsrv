#!/bin/sh

# $XFree86: xc/programs/Xserver/hw/xfree86/xf1bpp/mfbmap.sh,v 1.1.2.3 1998/06/27 14:48:23 dawes Exp $
#
# This script recreates the mapping list that maps the mfb external
#  symbols * to xf1bpp* (without "mfb")
# This should only be rerun if there have been changes in the mfb code
#  that affect the external symbols.
#  It assumes that Xserver/mfb has been compiled.
# The output goes to stdout.
echo ""
echo "#ifndef _MFBMAP_H"
echo "#define _MFBMAP_H"
echo ""

nm ../../../mfb/*.o | \
awk "{ if ((\$2 == \"D\") || (\$2 == \"T\") || (\$2 == \"C\")) print \$3 }" | \
sed s/^_// | \
grep -v "ModuleInit$" | \
sort | \
awk "{ print \"#define \" \$1 \" xf1bpp\"\$1 }" | \
sed s/xf1bppmfb/xf1bpp/

echo ""
echo "#endif"
