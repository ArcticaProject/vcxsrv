/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 * Copyright 1999 by David Holland <davidh@iquest.net>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the names of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#if defined(__i386__) || defined(__i386)
#define _NEED_SYSI86
#endif
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

extern char *apertureDevName;

/*
 * Read BIOS via mmap()ing physical memory.
 */
_X_EXPORT int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	int fd;
	unsigned char *ptr;
	char solx86_vtname[20];
	int psize;
	int mlen;

	/*
	 * Solaris 2.1 x86 SVR4 (10/27/93)
	 *	The server must treat the virtual terminal device file
	 *	as the standard SVR4 /dev/pmem. By default, then used VT
	 *	is considered the "default" file to open.
	 *
	 * Solaris 2.8 x86 (7/26/99) - DWH
	 *
	 *	Use /dev/xsvc for everything.
	 */
	psize = getpagesize();
	Offset += Base & (psize - 1);
	Base &= ~(psize - 1);
	mlen = (Offset + Len + psize - 1) & ~(psize - 1);
#if (defined(__i386__) || defined(__i386)) && !defined(__SOL8__)
	if (Base >= 0xA0000 && Base + mlen < 0xFFFFF && xf86Info.vtno >= 0)
		sprintf(solx86_vtname, "/dev/vt%02d", xf86Info.vtno);
	else
#endif
	{
		if (!xf86LinearVidMem())
			FatalError("xf86ReadBIOS: Could not mmap BIOS"
				   " [a=%lx]\n", Base);
		sprintf(solx86_vtname, apertureDevName);
	}

	if ((fd = open(solx86_vtname, O_RDONLY)) < 0)
	{
		xf86Msg(X_WARNING, "xf86ReadBIOS: Failed to open %s (%s)\n",
			solx86_vtname, strerror(errno));
		return(-1);
	}
	ptr = (unsigned char *)mmap((caddr_t)0, mlen, PROT_READ,
					MAP_SHARED, fd, (off_t)Base);
	if (ptr == MAP_FAILED)
	{
		xf86Msg(X_WARNING, "xf86ReadBIOS: %s mmap failed "
			"[0x%08lx, 0x%04x]\n",
			solx86_vtname, Base, mlen);
		close(fd);
		return -1;
	}

	(void)memcpy(Buf, (void *)(ptr + Offset), Len);
	(void)munmap((caddr_t)ptr, mlen);
	(void)close(fd);

	return Len;
}
