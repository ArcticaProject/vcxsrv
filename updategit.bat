setlocal on
set path=c:\program files\git\bin;%path%

if not isdir xserver          git clone git://anongit.freedesktop.org/xorg/xserver
if not isdir libxcb           git clone git://anongit.freedesktop.org/xcb/libxcb
if not isdir libxcb/xcb-proto git clone git://anongit.freedesktop.org/xcb/proto libxcb/xcb-proto
if not isdir xkeyboard-config git clone git://anongit.freedesktop.org/xkeyboard-config
if not isdir libX11           git clone git://anongit.freedesktop.org/xorg/lib/libX11
if not isdir libXdmcp         git clone git://anongit.freedesktop.org/xorg/lib/libXdmcp
if not isdir libXext          git clone git://anongit.freedesktop.org/xorg/lib/libXext
if not isdir libfontenc       git clone git://anongit.freedesktop.org/xorg/lib/libfontenc
if not isdir libXinerama      git clone git://anongit.freedesktop.org/xorg/lib/libXinerama
if not isdir libXau           git clone git://anongit.freedesktop.org/xorg/lib/libXau
if not isdir xkbcomp          git clone git://anongit.freedesktop.org/xorg/app/xkbcomp
if not isdir pixman           git clone git://anongit.freedesktop.org/pixman
if not isdir xextproto        git clone git://anongit.freedesktop.org/xorg/proto/xextproto
if not isdir randrproto       git clone git://anongit.freedesktop.org/xorg/proto/randrproto

pushd .
echo Updating xserver   |& tee /a d:\updategit.log
pushd xserver           |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating xkeyboard |& tee /a d:\updategit.log
pushd xkeyboard-config  |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating xcb       |& tee /a d:\updategit.log
pushd libxcb            |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating xcb-proto |& tee /a d:\updategit.log
pushd libxcb\xcb-proto  |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating libX11    |& tee /a d:\updategit.log
pushd libX11            |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating libXdmcp  |& tee /a d:\updategit.log
pushd libXdmcp          |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating libXext   |& tee /a d:\updategit.log
pushd libXext           |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating libfontenc|& tee /a d:\updategit.log
pushd libfontenc        |& tee /a d:\updategit.log
git pull                |& tee /a d:\updategit.log
popd
echo Updating libXinerama|& tee /a d:\updategit.log
pushd libXinerama        |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
echo Updating libXau     |& tee /a d:\updategit.log
pushd libXau             |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
echo Updating xkbcomp    |& tee /a d:\updategit.log
pushd xkbcomp            |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
echo Updating pixman     |& tee /a d:\updategit.log
pushd pixman             |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
echo Updating exextproto |& tee /a d:\updategit.log
pushd xextproto          |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
echo Updating randrproto |& tee /a d:\updategit.log
pushd randrproto         |& tee /a d:\updategit.log
git pull                 |& tee /a d:\updategit.log
popd
popd
setlocal off
