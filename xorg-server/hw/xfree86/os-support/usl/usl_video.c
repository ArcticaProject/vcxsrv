/*
 * Copyrught 2005 Kean Johnston <jkj@sco.com>
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell, David Dawes 
 * and Kean Johnston not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Thomas Roell, David Dawes and Kean Johnston make no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL, DAVID DAWES AND KEAN JOHNSTON DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THOMAS ROELLm DAVID WEXELBLAT
 * OR KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 */

#include "X.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

static Bool
linearVidMem(void)
{
  return TRUE;
}

static pointer
mapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
  pointer base;
  int fd;

  fd = open(DEV_MEM, (flags & VIDMEM_READONLY) ? O_RDONLY : O_RDWR);
  if (fd < 0) {
    FatalError("xf86MapVidMem: failed to open %s (%s)\n",
      DEV_MEM, strerror(errno));
  }
  base = mmap((caddr_t)0, Size, (flags & VIDMEM_READONLY) ?
	     PROT_READ : (PROT_READ | PROT_WRITE),
	     MAP_SHARED, fd, (off_t)Base);
  close(fd);

  if (base == MAP_FAILED) {
    FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
      "xf86MapVidMem", Size, Base, strerror(errno));
  }
  return(base);
}

/* ARGSUSED */
static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
  munmap(Base, Size);
}

/*
 * For some SVR4 versions, a 32-bit read is done for the first location
 * in each page when the page is first mapped.  If this is done while
 * memory access is enabled for regions that have read side-effects,
 * this can cause unexpected results, including lockups on some hardware.
 * This function is called to make sure each page is mapped while it is
 * safe to do so.
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

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
  pVidMem->linearSupported = linearVidMem();
  pVidMem->mapMem = mapVidMem;
  pVidMem->unmapMem = unmapVidMem;
  pVidMem->readSideEffects = readSideEffects;
  pVidMem->initialised = TRUE;
}

