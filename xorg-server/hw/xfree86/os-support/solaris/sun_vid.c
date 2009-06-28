/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
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
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/types.h> /* get __x86 definition if not set by compiler */

#if defined(__i386__) || defined(__i386) || defined(__x86)
#define _NEED_SYSI86
#endif
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

/***************************************************************************/
/* Video Memory Mapping section 					   */
/***************************************************************************/

char *apertureDevName = NULL;

_X_EXPORT Bool
xf86LinearVidMem(void)
{
	int	mmapFd;

	if (apertureDevName)
	    return TRUE;

	apertureDevName = "/dev/xsvc";
	if ((mmapFd = open(apertureDevName, O_RDWR)) < 0)
	{
	    apertureDevName = "/dev/fbs/aperture";
	    if((mmapFd = open(apertureDevName, O_RDWR)) < 0)
	    {
		xf86MsgVerb(X_WARNING, 0,
		    "xf86LinearVidMem: failed to open %s (%s)\n",
		    apertureDevName, strerror(errno));
		xf86MsgVerb(X_WARNING, 0,
		    "xf86LinearVidMem: either /dev/fbs/aperture or /dev/xsvc"
		    " device driver required\n");
		xf86MsgVerb(X_WARNING, 0,
		    "xf86LinearVidMem: linear memory access disabled\n");
		apertureDevName = NULL;
		return FALSE;
	    }
	}
	close(mmapFd);
	return TRUE;
}

_X_EXPORT pointer
xf86MapVidMem(int ScreenNum, int Flags, unsigned long Base, unsigned long Size)
{
	pointer base;
	int fd;
	char vtname[20];

	/*
	 * Solaris 2.1 x86 SVR4 (10/27/93)
	 * The server must treat the virtual terminal device file as the
	 * standard SVR4 /dev/pmem.
	 *
	 * Using the /dev/vtXX device as /dev/pmem only works for the
	 * A0000-FFFFF region - If we wish you mmap the linear aperture
	 * it requires a device driver.
	 *
	 * So what we'll do is use /dev/vtXX for the A0000-FFFFF stuff, and
	 * try to use the /dev/fbs/aperture or /dev/xsvc driver if the server
	 * tries to mmap anything > FFFFF.  Its very very unlikely that the
	 * server will try to mmap anything below FFFFF that can't be handled
	 * by /dev/vtXX.
	 *
	 * DWH - 2/23/94
	 * DWH - 1/31/99 (Gee has it really been 5 years?)
	 *
	 * Solaris 2.8 7/26/99
	 * Use /dev/xsvc for everything
	 *
	 * DWH - 7/26/99 - Solaris8/dev/xsvc changes
	 *
	 * TSI - 2001.09 - SPARC changes
	 */

#if defined(__i386__) && !defined(__SOL8__)
	if(Base < 0xFFFFF)
		sprintf(vtname, "/dev/vt%02d", xf86Info.vtno);
	else
#endif
	{
		if (!xf86LinearVidMem())
			FatalError("xf86MapVidMem:  no aperture device\n");

		strcpy(vtname, apertureDevName);
	}

	fd = open(vtname, (Flags & VIDMEM_READONLY) ? O_RDONLY : O_RDWR);
	if (fd < 0)
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   vtname, strerror(errno));

	base = mmap(NULL, Size,
		    (Flags & VIDMEM_READONLY) ?
			PROT_READ : (PROT_READ | PROT_WRITE),
		     MAP_SHARED, fd, (off_t)Base);
	close(fd);
	if (base == MAP_FAILED)
		FatalError("xf86MapVidMem:  mmap failure:  %s\n",
			   strerror(errno));

	return(base);
}

/* ARGSUSED */
_X_EXPORT void
xf86UnMapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap(Base, Size);
}

/***************************************************************************/
/* I/O Permissions section						   */
/***************************************************************************/

#if defined(__i386__) || defined(__i386) || defined(__x86)
static Bool ExtendedEnabled = FALSE;
#endif

_X_EXPORT Bool
xf86EnableIO(void)
{
#if defined(__i386__) || defined(__i386) || defined(__x86)
	if (ExtendedEnabled)
		return TRUE;

	if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0) {
		xf86Msg(X_WARNING,"xf86EnableIOPorts: Failed to set IOPL for I/O\n");
		return FALSE;
	}
	ExtendedEnabled = TRUE;
#endif /* i386 */
	return TRUE;
}

_X_EXPORT void
xf86DisableIO(void)
{
#if defined(__i386__) || defined(__i386) || defined(__x86)
	if(!ExtendedEnabled)
		return;

	sysi86(SI86V86, V86SC_IOPL, 0);

	ExtendedEnabled = FALSE;
#endif /* i386 */
}


/***************************************************************************/
/* Interrupt Handling section						   */
/***************************************************************************/

_X_EXPORT Bool xf86DisableInterrupts(void)
{
#if defined(__i386__) || defined(__i386) || defined(__x86)
	if (!ExtendedEnabled && (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0))
		return FALSE;

#ifdef __GNUC__
	__asm__ __volatile__("cli");
#else
	asm("cli");
#endif /* __GNUC__ */

	if (!ExtendedEnabled)
		sysi86(SI86V86, V86SC_IOPL, 0);
#endif /* i386 */

	return TRUE;
}

_X_EXPORT void xf86EnableInterrupts(void)
{
#if defined(__i386__) || defined(__i386) || defined(__x86)
	if (!ExtendedEnabled && (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0))
		return;

#ifdef __GNUC__
	__asm__ __volatile__("sti");
#else
	asm("sti");
#endif /* __GNUC__ */

	if (!ExtendedEnabled)
		sysi86(SI86V86, V86SC_IOPL, 0);
#endif /* i386 */
}

_X_EXPORT void
xf86MapReadSideEffects(int ScreenNum, int Flags, pointer Base,
	unsigned long Size)
{
}

_X_EXPORT Bool
xf86CheckMTRR(int ScreenNum)
{
	return FALSE;
}

