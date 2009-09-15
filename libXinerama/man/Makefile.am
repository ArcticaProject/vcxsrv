#
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# 
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
# 
# Except as contained in this notice, the name of the copyright holders shall
# not be used in advertising or otherwise to promote the sale, use or
# other dealings in this Software without prior written authorization
# from the copyright holders.
# 

libmandir = $(LIB_MAN_DIR)

LIB_MAN_DIR_SUFFIX = $(LIB_MAN_DIR:@mandir@/man%=%)

libman_PRE = Xinerama.man

BUILT_SOURCES = shadows.DONE

EXTRA_DIST = $(libman_PRE)

CLEANFILES = $(libman_DATA) $(BUILT_SOURCES)

libman_DATA = $(libman_PRE:man=@LIB_MAN_SUFFIX@) \
	      $(Xinerama_shadows:=.@LIB_MAN_SUFFIX@)

SED = sed

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = \
	-e 's/__vendorversion__/"$(XORGRELSTRING)" "$(XORGMANNAME)"/' \
	-e 's/__xservername__/Xorg/g' \
	-e 's/__xconfigfile__/xorg.conf/g' \
	-e 's/__appmansuffix__/$(APP_MAN_SUFFIX)/g' \
	-e 's/__libmansuffix__/$(LIB_MAN_SUFFIX)/g' \
	-e 's/__filemansuffix__/$(FILE_MAN_SUFFIX)/g'

SUFFIXES = .$(LIB_MAN_SUFFIX) .man

.man.$(LIB_MAN_SUFFIX):
	sed $(MAN_SUBSTS) < $< > $@

# Generate man page shadow files (Replaces InstallManPageAliases from Imake)

Xinerama_shadows =		\
	XineramaQueryExtension	\
	XineramaQueryVersion	\
	XineramaIsActive	\
	XineramaQueryScreens

shadows.DONE:
	-rm -f $(Xinerama_shadows:=.@LIB_MAN_SUFFIX@)
	(for i in $(Xinerama_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/Xinerama.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
