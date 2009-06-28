# Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, and/or sell copies of the Software, and to permit persons
# to whom the Software is furnished to do so, provided that the above
# copyright notice(s) and this permission notice appear in all copies of
# the Software and that both the above copyright notice(s) and this
# permission notice appear in supporting documentation.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
# OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
# INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
# FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
# Except as contained in this notice, the name of a copyright holder
# shall not be used in advertising or otherwise to promote the sale, use
# or other dealings in this Software without prior written authorization
# of the copyright holder.
# 

bin_PROGRAMS = xorgconfig

X11dir = $(libdir)/X11
dist_X11_DATA = Cards

xorgconfig_CFLAGS = @XORGCONFIG_DEP_CFLAGS@ \
	-DCARD_DATABASE_FILE='"$(X11dir)/Cards"' \
	-DPROJECTROOT='"$(PROJECTROOT)"' \
        -DFILEMANSUFFIX='"$(FILE_MAN_SUFFIX)"' \
        -DXVERSIONSTRING='"$(PACKAGE_STRING)"'

xorgconfig_LDADD = @XORGCONFIG_DEP_LIBS@

xorgconfig_SOURCES =	\
	cards.c		\
	cards.h		\
	xorgconfig.c

# Man page
include $(top_srcdir)/cpprules.in

appmandir = $(APP_MAN_DIR)

appman_PRE = xorgconfig.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRAMANDEFS = \
	-D__XCONFIGFILE__=$(__XCONFIGFILE__) \
	-D__XSERVERNAME__=$(XSERVERNAME)

BUILT_SOURCES = $(appman_PRE)
CLEANFILES = $(appman_PRE) $(appman_DATA)

SUFFIXES += .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	-rm -f $@
	$(LN_S) $< $@

EXTRA_DIST = \
	Cards98 \
	xorgconfig.man.pre
