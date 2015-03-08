VcXsrv Windows X Server (Custom builds by X2Go & Arctica)
------------------------------------------------

The X2Go Project and Arctica Project are maintaining a 1.15.2.x branch of VcXsrv.

VcXsrv is a Windows X-server based on the xorg git sources (like xming or cygwin's xwin), but compiled with Visual Studio.

The upstream VcXsrv project can be found on [SourceForge](http://sourceforge.net/projects/vcxsrv/). Their last release of VcXsrv 1.15.2.x was 1.15.2.0, and it lacks Windows XP support.

------

#### Differences from VcXsrv

The primary differences from upstream VcXsrv are:

1. Compatibility with Windows XP
2. Prompt security updates

There are frequent security updates because numerous 3rd-party components are in the source tree. In addition to xorg-server, there are components such as openssl, freetype2 & various X11 libraries.

Other changes are:

1. Only 32-bit builds (for the time being)
2. Fixes for issues when used with nx-libs
3. Improvements for contributors: Build Instructions are current & correct, we accept pull requests/patches, we respond to issues (although we cannot guarantee timely fixes), etc

------

#### Download builds:

http://code.x2go.org/releases/binary-win32/3rd-party/vcxsrv-modified-by-x2go-project/

The current version: 

vcxsrv.1.15.2.4-xp+vc2013+x2go1.installer.exe (43M)

http://code.x2go.org/releases/binary-win32/3rd-party/vcxsrv-modified-by-x2go-project/vcxsrv.1.15.2.4-xp+vc2013+x2go1.installer.exe

Note that the version string will be simplified with the next release.

------

#### Why this repo and these builds exist.

Contributors and users might be wondering why this repo and these builds exist. Why not upstream your changes?

1. The primary developer of this repo and these builds (Mike DePaulo) tried multiple times to submit merge requests (the SourceForge equivalent of pull requests) and bug reports to the upstream VcXsrv project. The upstream VcXsrv developer (marha) only replied once. He rejected the merge request (for a legit reason), and gave good advice. However, he did not reject the merge request via the actual SourceForge system, and he has not responded since.
2. SourceForge's support for git is terrible. For example, whenever I tried to merge request from branch Y on my personal repo to the master branch on the upstream VcXsrv repo, it tried to merge my master branch instead.
3. [SourceForge has recently done terrible things as GitHub has gained popularity](https://en.wikipedia.org/wiki/SourceForge#DevShare_adware_controversy)
4. Using a GitHub repo is much preferable to using a personal SourceForge repo in terms of visibility.
