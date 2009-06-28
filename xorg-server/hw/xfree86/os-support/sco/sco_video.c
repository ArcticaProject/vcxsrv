/*
 * Copyright 2001 by J. Kean Johnston <jkj@sco.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name J. Kean Johnston not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  J. Kean Johnston makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * J. KEAN JOHNSTON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL J. KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* Re-written May 2001 to represent the current state of reality */

/*
 * This file contains the completely re-written SCO OpenServer video
 * routines for XFree86 4.x. Much of this is based on the SCO X server
 * code (which is an X11R5 server) and will probably only work on
 * OpenServer versions 5.0.5, 5.0.6 and later. Please send me (jkj@sco.com)
 * email if you have any questions.
 *
 * Ideally, you should use OSR5.0.6A or later, with the updated console
 * driver for 5.0.6A (its the default driver in 5.0.7 and later).
 * However, if you are running on an older system, this code will detect
 * that and adjust accordingly.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "input.h"
#include "scrnintstr.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86OSpriv.h"
#include "xf86_OSlib.h"

#include <sys/ci/ciioctl.h>
#define MPXNAME "/dev/atp1"
#define BASECPU 1

Bool mpxLock = TRUE;

#define USE_VASMETHOD	1

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static int sco_mcdone = 0, sco_ismc = 0;

/***************************************************************************/
/*
 * To map the video memory, we first need to see if we are on a multi-console
 * system. If we are, we need to try to use an existing video class in the
 * kernel. We do this by retrieving the list of currently defined classes
 * (via the new CONS_GETCLASS ioctl()) to see if we have a class that will
 * match the range of memory we desire. If we can't find one, we have an
 * error and we abort.
 *
 * If we are not using a multi-console, we can simply use mmap() to map in
 * the frame buffer, using the classs-access method as a fall-back only if
 * the mmap() fails (it shouldn't). We always set the appropriate pointers
 * in the config structure to point ot the right function to map and unmap
 * the video memory. An alternative to using mmap() is to use the new
 * CONS_ADDVAS call, which will use vasmalloc() and vasbind() in the kernel
 * to map the physical address to a virtual one, which it then returns.
 * I am not 100% sure if this is faster or not, but it may prove easier to
 * debug things. Just to be on the safe side, I have included both methods
 * here, and the mmap() method can be used by setting USE_VASMETHOD to 0
 * above.
 */

#if !defined(CONS_ADDVAS)
# undef USE_VASMETHOD
# define USE_VASMETHOD 0
#endif

static int
scoIsMultiConsole (void)
{
	int x;

	if (sco_mcdone)
		return sco_ismc;
	x = access ("/usr/lib/vidconf/.multiconsole", F_OK);
	if (x == 0)
		sco_ismc = 1;
	sco_mcdone = 1;
	return sco_ismc;
}

/*
 * This maps memory using mmap()
 */
static pointer
mapVidMemMMAP(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
  int fd;
  unsigned long realBase, alignOff;
  pointer base;

  fd = open (DEV_MEM, (flags & VIDMEM_READONLY) ? O_RDONLY : O_RDWR);
  if (fd < 0) {
    FatalError("xf86MapVidMem: failed to open %s (%s)\n", DEV_MEM,
       	strerror(errno));
    return 0; /* NOTREACHED */
  }

  realBase = Base & ~(getpagesize() - 1);
  alignOff = Base - realBase;

#ifdef DEBUG
  ErrorF("base: %lx, realBase: %lx, alignOff: %lx\n", Base,realBase,alignOff);
#endif

  base = mmap((caddr_t)0, Size + alignOff,
	      (flags & VIDMEM_READONLY) ? PROT_READ : (PROT_READ | PROT_WRITE),
  	      MAP_SHARED, fd, (off_t)realBase);
  close(fd);
  if (base == MAP_FAILED) {
    FatalError("xf86MapVidMem: Could not mmap framebuffer (0x%08x,0x%x) (%s)\n",
       	Base, Size, strerror(errno));
    return 0; /* NOTREACHED */
  }

#ifdef DEBUG
    ErrorF("base: %lx aligned base: %lx\n",base, base + alignOff);
#endif
    return (pointer)((char *)base + alignOff);
}

#if (USE_VASMETHOD)
/*
 * This maps memory using the virtual address space (VAS) console calls.
 */
static pointer
mapVidMemVAS(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
  struct vidvasmem vas;
  pointer base;

  vas.base = (long)Base;
  vas.size = (long)Size;

  base = (pointer)ioctl (xf86Info.consoleFd, CONS_ADDVAS, &vas);
  if (base == (pointer)-1) {
    return mapVidMemMMAP(ScreenNum, Base, Size, flags);
  }
  return base;
}
#endif /* USE_VASMETHOD */

struct vidclass vidclasslist[] = {
	{ "VBE", "", 0xf0000000, 0x2000000, 0 },
	{ "P9000", "", 0xc0000000, 0x400000, 0 },
	{ "TULIP", "", 0x80000000, 0x400000, 0 },
	{ "VIPER", "", 0xa0000000, 0x400000, 0 },
	{ "S3T", "", 0xa0000000, 0x200000, 0 },
	{ "S3DT", "", 0x4000000, 0x400000, 0 },
	{ "MGA", "", 0x2200000, 0x4000, 0 },
	{ "CLVGA", "", 0xa0000, 0x20000, 0 },
	{ "OLIVE", "", 0xd8000000, 0x400000, 0 },
	{ "S3C", "", 0xa0000, 0x10000, 0 },
	{ "MGAVLB", "", 0xac000, 0x34000, 0 },
	{ "ATI8514", "", 0xFF000, 0x1000, 0 },
	{ "GXREGS", "", 0xb0000, 0x10000, 0 },
	{ "GX", "", 0xa0000, 0x10000, 0 },
	{ "CT64300", "", 0xa0000000, 0x400000, 0 },
	{ "SVGA", "", 0xa0000, 0x20000, 0 },
	{ "S3V", "", 0xa0000000, 0x400000, 0 },
	{ "8514A", "", 0xFF000, 0x1000, 0 },
	{ "VGA", "", 0xa0000, 0x10000, 0 },
	{ 0 }
};

static pointer
mapVidMemVC(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
  struct vidclass *vcp;
  char *class = NULL;
  pointer base;

  for (vcp = vidclasslist; vcp->name; vcp++) {
    if ((vcp->base == Base) && (vcp->size == Size)) {
      class = vcp->name;
      break;
    }
  }

  if (class == NULL) {
    /*
     * As a fall-back, we will try and use the mmap() approach. This may
     * prove to be the wrong thing to do, but time and testing will tell.
     */
    ErrorF("xf86MapVidMem: No class map defined for (0x%08x,0x%08x)\n", Base, Size);
#if USE_VASMETHOD
    return mapVidMemVAS(ScreenNum, Base, Size, flags);
#else /* !USE_VASMETHOD */
    return mapVidMemMMAP(ScreenNum, Base, Size, flags);
#endif
  }

  /*
   * We found a suitable class. Try and use it.
   */
  base = (pointer)ioctl(xf86Info.consoleFd, MAP_CLASS, class);
  if ((int)base == -1) {
    FatalError("xf86MapVidMem: Failed to map video memory class `%s'\n", class);
    return 0; /* NOTREACHED */
  }

  return base;
}

/*
 * Unmapping the video memory is easy. We always call munmap(), as it is
 * safe to do so even if we haven't actually mapped in any pages via mmap().
 * In the case where we used the video class, we don't need to do anything
 * as the kernel will clean up the TSS when we exit, and will undo the
 * vasbind() that was done when the class was originally mapped. If we used
 * vasmap, we simply undo the map. Again, it is benign to call vasunmap
 * even if we got the frame buffer via some other mechanism (like mmap).
 */

static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
#if USE_VASMETHOD
  struct vidvasmem vas;
  int x;

  vas.base = (long)Base;
  vas.size = (long)Size;

  x = ioctl (xf86Info.consoleFd, CONS_DELVAS, &vas);
  if (x == 0)
    return;
#endif /* USE_VASMETHOD */

  munmap(Base, Size);
}

/*
 * Set things up to point to our local functions. When the kernel gets
 * MTRR support, we will need to add the required functions for that
 * here too. MTRR support will most likely appear in 5.0.8 or 5.1.0.
 *
 * We also want to lock the X server process to the base CPU in an MPX
 * system, since we will be going to IOPL 3. Most engine drivers can cope
 * with I/O access on any CPU but there are a few (AST Manhattan I believe)
 * that can't, so the server needs to be locked to CPU0.
 */
void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
  int mpx_fd;

  if (scoIsMultiConsole ()) {
    pVidMem->mapMem = mapVidMemVC;
  } else {
#if USE_VASMETHOD
    pVidMem->mapMem = mapVidMemVAS;
#else
    pVidMem->mapMem = mapVidMemMMAP;
#endif
  }

  pVidMem->unmapMem = unmapVidMem;
  pVidMem->linearSupported = TRUE;
  pVidMem->initialised = TRUE;

  if (mpxLock && (mpx_fd = open (MPXNAME, O_RDONLY)) > 0) {
    if (ioctl (mpx_fd, ACPU_XLOCK, BASECPU) < 0)
      ErrorF ("xf86OSInitVidMem: Can not bind to CPU 0 (%s)\n",
          strerror(errno));
    close (mpx_fd);
  }
}

