setlocal on
goto noclone

git clone git://anongit.freedesktop.org/xorg/xserver
git clone git://anongit.freedesktop.org/xcb/libxcb
git clone git://anongit.freedesktop.org/xcb/proto libxcb/xcb-proto
git clone git://anongit.freedesktop.org/xkeyboard-config
git clone git://anongit.freedesktop.org/xorg/lib/libX11
git clone git://anongit.freedesktop.org/xorg/lib/libXdmcp
git clone git://anongit.freedesktop.org/xorg/lib/libXext
git clone git://anongit.freedesktop.org/xorg/lib/libfontenc
git clone git://anongit.freedesktop.org/xorg/lib/libXinerama
git clone git://anongit.freedesktop.org/xorg/lib/libXau

:noclone
pushd .
set path=c:\program files\git\bin;%path%
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
popd
setlocal off
