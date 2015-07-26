VcXsrv Windows X Server (Custom builds by X2Go & Arctica)
------------------------------------------------

The X2Go Project and Arctica Project are maintaining 1.15.2.x & 1.17.0.0-x branches of VcXsrv.

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
4. Better compression within the installer (for 1.17.0.0-x)

------

#### Download Builds:

Current versions of both 1.15.2.x & 1.17.0.0-x and be found on the [Releases](https://github.com/ArcticaProject/vcxsrv/releases) page on GitHub.:

`1.15.2.6` and earlier, as well as `1.17.0.0-1`, can be found [here](http://code.x2go.org/releases/binary-win32/3rd-party/vcxsrv-modified-by-x2go-project/).



------

#### Community Support

You can post *issues* using GitHub.

You can also ask questions about VcXsrv usage or discuss development on [the X2Go mailing lists.](http://lists.x2go.org/listinfo/) Note that this may change in the future.

#### Professional Support

[Professional Support is available from one of the X2Go Developers.](http://wiki.x2go.org/doku.php/doc:professional-support)

## Info for Developers

####  Why this repo and these builds exist.

You might be wondering why this repo and these builds exist. Why not upstream your changes?

1. The primary developer of this repo and these builds (Mike DePaulo) tried multiple times to submit merge requests (the SourceForge equivalent of pull requests) and bug reports to the upstream VcXsrv project. The upstream VcXsrv developer (marha) only replied once. He rejected the merge request (for a legit reason), and gave good advice. However, he did not reject the merge request via the actual SourceForge system, and he has not responded since.
2. SourceForge's support for git is terrible. For example, whenever I tried to merge request from branch Y on my personal repo to the master branch on the upstream VcXsrv repo, it tried to merge my master branch instead.
3. [SourceForge has recently done terrible things as GitHub has gained popularity](https://en.wikipedia.org/wiki/SourceForge#DevShare_adware_controversy)
4. Using a GitHub repo is much preferable to using a personal SourceForge repo in terms of visibility.

#### Explanation of Branches:

1. master - The upstream VcXsrv branch that upstream VcXsrv is built from. Currently on the 1.16.x version series.
2. release/1.15.2.x - the 1.15.2.x branch maintained by Arctica and X2Go.
3. release/1.17.0.0-x - the 1.17.0.0-x branch maintained by Arctica and X2Go.
4. release/external - Upstream copies the released (or git) versions of the various packages into this branch, which they call `released` . They then update the packages.txt file at the root.  This branch contains 0 modifications to those packages. Upstream then uses git's merge feature to merge these packages updates/upgrades into master, where upstream does make modifications to those packages.
5. release/external-1.15.2.x - same as `release/external`, but for `release/1.15.2.x` rather than `master`.
6. release/external-1.17.0.0-x - same as `release/external`, but for `release/1.17.0.0-x` rather than `master`.
