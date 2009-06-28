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

bin_PROGRAMS = ioport

XFREE86_SRCDIR 	= $(top_srcdir)/hw/xfree86
DUMMYLIB_SRCDIR	= $(XFREE86_SRCDIR)/dummylib

INCLUDES = $(XORG_INCS) -I$(DUMMYLIB_SRCDIR)

ioport_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)
ioport_LDADD = \
	../../os-support/libxorgos.la \
	../../dummylib/libdummy-nonserver.a \
	${UTILS_SYS_LIBS} ${PCIACCESS_LIBS}


ioport_SOURCES =	\
	ioport.c

ioport_PROGRAM_LINKS = inb inw inl outb outw outl

all-local:
	@for i in $(ioport_PROGRAM_LINKS); do (set -x; \
		rm -f $$i; $(LN_S) ioport $$i); \
        done

install-exec-hook:
	@for i in $(ioport_PROGRAM_LINKS); do (set -x; \
		rm -f $(DESTDIR)$(bindir)/$$i; \
	        (cd $(DESTDIR)$(bindir); $(LN_S) ioport $$i)); \
        done

