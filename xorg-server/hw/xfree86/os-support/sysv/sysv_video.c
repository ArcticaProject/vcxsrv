/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Wexelblat makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#ifndef SI86IOPL
#define SET_IOPL() sysi86(SI86V86,V86SC_IOPL,PS_IOPL)
#define RESET_IOPL() sysi86(SI86V86,V86SC_IOPL,0)
#else
#define SET_IOPL() sysi86(SI86IOPL,3)
#define RESET_IOPL() sysi86(SI86IOPL,0)
#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

/*
 * XXX Support for SVR3 will need to be reworked if needed.  In particular
 * the Region parameter is no longer passed, and will need to be dealt
 * with internally if required.
 * OK, i'll rework that thing ... (clean it up a lot)
 * SVR3 Support only with SVR3_MMAPDRV (mr)
 * 
 */

#ifdef HAS_SVR3_MMAPDRV
#ifndef MMAP_DEBUG
#define MMAP_DEBUG	3
#endif

struct kd_memloc MapDSC;
int mmapFd = -2;

static int
mmapStat(pointer Base, unsigned long Size) {

	int nmmreg,i=0,region=-1;
	mmapinfo_t *ibuf;

	nmmreg = ioctl(mmapFd, GETNMMREG);

	if(nmmreg <= 0)
	   xf86MsgVerb(X_INFO, MMAP_DEBUG,
			  "\nNo physical memory mapped currently.\n\n");
	else {
	  if((ibuf = (mmapinfo_t *)malloc(nmmreg*sizeof(mmapinfo_t))) == NULL) 
		xf86Msg(X_WARNING,
			  "Couldn't allocate memory 4 mmapinfo_t\n");
	  else {
	     if(ioctl(mmapFd, GETMMREG, ibuf) != -1)
		{ 
		   xf86MsgVerb(X_INFO, MMAP_DEBUG,
				"# mmapStat: [Size=%x,Base=%x]\n", Size, Base);
		   xf86MsgVerb(X_INFO, MMAP_DEBUG,
		     "#      Physical Address     Size      Reference Count\n");
		for(i = 0; i < nmmreg; i++) {
		   xf86MsgVerb(X_INFO, MMAP_DEBUG,
                      "%-4d   0x%08X         %5dk                %5d	",
          	      i, ibuf[i].physaddr, ibuf[i].length/1024, ibuf[i].refcnt);
		   if (ibuf[i].physaddr == Base || ibuf[i].length == Size ) {
			xf86MsgVerb(X_INFO, MMAP_DEBUG,"MATCH !!!");
			if (region==-1) region=i;
                      }
		   xf86ErrorFVerb(MMAP_DEBUG, "\n");
		}
		xf86ErrorFVerb(MMAP_DEBUG, "\n");
		}
	     free(ibuf);
	   }
	}
	if (region == -1 && nmmreg > 0) region=region * i;
	return(region);
}
#endif


static Bool
linearVidMem()
{
#ifdef SVR4
	return TRUE;
#elif defined(HAS_SVR3_MMAPDRV)
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
		    "# xf86LinearVidMem: MMAP 2.2.2 called\n");

	if(mmapFd >= 0) return TRUE;

	if ((mmapFd = open("/dev/mmap", O_RDWR)) != -1)
	{
	    if(ioctl(mmapFd, GETVERSION) < 0x0222) {
		xf86Msg(X_WARNING,
			"xf86LinearVidMem: MMAP 2.2.2 or above required\n");
		xf86ErrorF("\tlinear memory access disabled\n");
		return FALSE;
	    }
	    return TRUE;
	}
	xf86Msg(X_WARNING, "xf86LinearVidMem: failed to open /dev/mmap (%s)\n",
	        strerror(errno));
	xf86ErrorF("\tlinear memory access disabled\n");
	return FALSE;
#endif
}

static pointer
mapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	pointer base;
	int fd;

#if defined(SVR4)
	fd = open(DEV_MEM, (flags & VIDMEM_READONLY) ? O_RDONLY : O_RDWR);
	if (fd < 0)
	{
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   DEV_MEM, strerror(errno));
	}
	base = mmap((caddr_t)0, Size,
		    (flags & VIDMEM_READONLY) ?
		     PROT_READ : (PROT_READ | PROT_WRITE),
		    MAP_SHARED, fd, (off_t)Base);
	close(fd);
	if (base == MAP_FAILED)
	{
		FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, strerror(errno));
	}
#else /* SVR4 */
#ifdef HAS_SVR3_MMAPDRV

	xf86MsgVerb(X_INFO, MMAP_DEBUG, "# xf86MapVidMem: MMAP 2.2.2 called\n");
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"MMAP_VERSION: 0x%x\n",ioctl(mmapFd, GETVERSION));
	if (ioctl(mmapFd, GETVERSION) == -1)
	{
		xf86LinearVidMem();
	}
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"MMAP_VERSION: 0x%x\n",ioctl(mmapFd, GETVERSION));
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
		"xf86MapVidMem: Screen: %d\n", ScreenNum);
	mmapStat(Base,Size);
	/* To force the MMAP driver to provide the address */
	base = (pointer)0;
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"xf86MapVidMem: [s=%x,a=%x]\n", Size, Base);
	MapDSC.vaddr    = (char *)base;
	MapDSC.physaddr = (char *)Base;
	MapDSC.length   = Size;
	MapDSC.ioflg    = 1;
	if(mmapFd >= 0)
	{
	    if((base = (pointer)ioctl(mmapFd, MAP, &MapDSC)) == (pointer)-1)
	    {
		FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, strerror(errno));
		/* NOTREACHED */
	    }

	    /* Next time we want the same address! */
	    MapDSC.vaddr    = (char *)base;
	}

	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"MapDSC.vaddr   : 0x%x\n", MapDSC.vaddr);
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"MapDSC.physaddr: 0x%x\n", MapDSC.physaddr);
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"MapDSC.length  : %d\n", MapDSC.length);
	mmapStat(Base,Size);
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"xf86MapVidMem: [s=%x,a=%x,b=%x]\n", Size, Base, base);
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
			"xf86MapVidMem: SUCCEED Mapping FrameBuffer \n");
#endif /* HAS_SVR3_MMAPDRV */
#endif /* SVR4 */
	return(base);
}

/* ARGSUSED */
static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
#if defined (SVR4)
	munmap(Base, Size);
#else /* SVR4 */
#ifdef HAS_SVR3_MMAPDRV
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
	        "# xf86UnMapVidMem: UNMapping FrameBuffer\n");
	mmapStat(Base,Size);
	ioctl(mmapFd, UNMAPRM , Base);
	mmapStat(Base,Size);
	xf86MsgVerb(X_INFO, MMAP_DEBUG,
		"# xf86UnMapVidMem: Screen: %d [v=%x]\n", ScreenNum, Base);
#endif /* HAS_SVR3_MMAPDRV */
#endif /* SVR4 */
	return;
}

#if defined(SVR4) && defined(__i386__) && !defined(sun)
/*
 * For some SVR4 versions, a 32-bit read is done for the first location
 * in each page when the page is first mapped.  If this is done while
 * memory access is enabled for regions that have read side-effects,
 * this can cause unexpected results, including lockups on some hardware.
 * This function is called to make sure each page is mapped while it is
 * safe to do so.
 */

/*
 * XXX Should get this the correct way (see os/xalloc.c), but since this is
 * for one platform I'll be lazy.
 */
#define X_PAGE_SIZE 4096

static void
readSideEffects(int ScreenNum, pointer Base, unsigned long Size)
{
	unsigned long base, end, addr;
	CARD32 val;

	base = (unsigned long)Base;
	end = base + Size;

	for (addr = base; addr < end; addr += X_PAGE_SIZE)
		val = *(volatile CARD32 *)addr;
}
#endif

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
	pVidMem->linearSupported = linearVidMem();
	pVidMem->mapMem = mapVidMem;
	pVidMem->unmapMem = unmapVidMem;
#if defined(SVR4) && defined(__i386__) && !defined(sun)
	pVidMem->readSideEffects = readSideEffects;
#endif
	pVidMem->initialised = TRUE;
}
	
/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

_X_EXPORT Bool
xf86EnableIO()
{
	int i;

	if (ExtendedEnabled)
		return TRUE;

	if (SET_IOPL() < 0)
	{
	    xf86Msg(X_WARNING,
			"xf86EnableIO: Failed to set IOPL for extended I/O\n");
	    return FALSE;
	}
	ExtendedEnabled = TRUE;

	return TRUE;
}
	
_X_EXPORT void
xf86DisableIO()
{
	if (!ExtendedEnabled)
		return;

	RESET_IOPL();
	ExtendedEnabled = FALSE;

	return;
}
