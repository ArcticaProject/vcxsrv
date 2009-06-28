/*
 * Copyright 1993 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#if defined(__powerpc__)

# if defined(USE_MACHINE_ABSOLUTE)
#   include <machine/absolute.h>
# else
#   define __USER_SPACE_INCLUDE
#   include <hw_absolute.h>
# endif

void ppcPciIoMap(int bus);
#endif

#if 0
#define DEBUG	
#endif

#ifdef HAS_MTRR_SUPPORT
#include <sys/memrange.h>
#define X_MTRR_ID "XFree86"

static pointer setWC(int, unsigned long, unsigned long, Bool, MessageType);
static void undoWC(int, pointer);
static Bool cleanMTRR(void);
static int devMemFd = -1;
#define MTRR_DEVICE	"/dev/mtrr"
#endif


#if !defined(NO_MMAP)
#include <sys/mman.h>

int smem_remove(char *name)
{
  return(0);
}

char *smem_create(char *name, char *arg_addr, long size, int mode)
{
  int fd;
  void *addr = 0;
  char *retval;
  size_t len = size;
  int prot = PROT_READ|PROT_WRITE|PROT_UNCACHE;
  int flags = MAP_SHARED;
  off_t off = (off_t)arg_addr;

  if ((fd = open("/dev/mem" , O_RDWR)) < 0)
  {
    retval = (char *)-1;
  }
  else
  {
    if (mode == SM_DETACH)
    {
      munmap(arg_addr, len);
      retval = 0;
    }
    else
    {
      if ((retval = mmap (addr, len, prot, flags, fd, off) ) == MAP_FAILED)
      {
        retval = (char *)-1;
      }
    }

    close(fd);
  }

  return(retval);
}

#endif


/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

typedef struct
{
	char	name[16];
	unsigned long	Base;
	unsigned long	Size;
	char	*ptr;
	int	RefCnt;
}
_SMEMS;

#define MAX_SMEMS	16

static _SMEMS	smems[MAX_SMEMS];


#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

static void
smemCleanup(void)
{
	int i;

	for (i = 0; i < MAX_SMEMS; i++) {
		if (*smems[i].name && smems[i].ptr) {
			(void)smem_create(NULL, smems[i].ptr, 0, SM_DETACH);
			(void)smem_remove(smems[i].name);
			*smems[i].name = '\0';
			smems[i].ptr = NULL;
			smems[i].Base = 0;
			smems[i].Size = 0;
			smems[i].RefCnt = 0;
		}
	}
}

static pointer
MapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	static int once;
	int	free_slot = -1;
	int	i;

	if (!once)
	{
		atexit(smemCleanup);
		once = 1;
	}
	for (i = 0; i < MAX_SMEMS; i++)
	{
		if (!*smems[i].name && free_slot == -1)
			free_slot = i;
		if (smems[i].Base == Base && smems[i].Size == Size 
		    && *smems[i].name) {
			smems[i].RefCnt++;
			return smems[i].ptr;
		}
	}
	if (i == MAX_SMEMS && free_slot == -1)
	{
		FatalError("MapVidMem: failed to smem_create Base %x Size %x (out of SMEMS entries)\n",
			Base, Size);
	}

	i = free_slot;
	sprintf(smems[i].name, "Video-%d", i);
	smems[i].Base = Base;
	smems[i].Size = Size;
	
        xf86MsgVerb(X_INFO, 3, "MapVidMem: Base=0x%x Size=0x%x\n",
        	Base, Size);

#if defined(__powerpc__)
	if (((unsigned long)Base & PHYS_IO_MEM_START) != PHYS_IO_MEM_START) {
		Base = Base | PHYS_IO_MEM_START;
	}
#endif

	smems[i].ptr = smem_create(smems[i].name, (char *)Base, Size, SM_READ|SM_WRITE);
	smems[i].RefCnt = 1;
	if (smems[i].ptr == NULL)
	{
		/* check if there is a stale segment around */
		if (smem_remove(smems[i].name) == 0) {
	        	xf86Msg(X_INFO,
			    "MapVidMem: removed stale smem_ segment %s\n",
		            smems[i].name);
			smems[i].ptr = smem_create(smems[i].name, 
						(char *)Base, Size, SM_READ|SM_WRITE);
		}
	        if (smems[i].ptr == NULL) {
			*smems[i].name = '\0';
			FatalError("MapVidMem: failed to smem_create Base %x Size %x (%s)\n",
				Base, Size, strerror(errno));
		}
	}
        xf86MsgVerb(X_INFO, 3, "MapVidMem: Base=0x%x Size=0x%x Ptr=0x%x\n",
        		 Base, Size, smems[i].ptr);
	return smems[i].ptr;
}

static void
UnMapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	int	i;

	xf86MsgVerb(X_INFO, 3, "UnMapVidMem: Base/Ptr=0x%x Size=0x%x\n",
		Base, Size);
	for (i = 0; i < MAX_SMEMS; i++)
	{
		if (*smems[i].name && smems[i].ptr == Base 
			&& smems[i].Size == Size)
		{
			if (--smems[i].RefCnt > 0)
				return;

			(void)smem_create(NULL, smems[i].ptr, 0, SM_DETACH);
			xf86MsgVerb(X_INFO, 3,
                           "UnMapVidMem: smem_create(%s, 0x%08x, ... "
                           "SM_DETACH)\n", smems[i].name, smems[i].ptr);
			(void)smem_remove(smems[i].name);
			*smems[i].name = '\0';
			smems[i].RefCnt = 0;
			return;
		}
	}
	xf86MsgVerb(X_WARNING, 2,
		"UnMapVidMem: no SMEM found for Base = %lx Size = %lx\n",
	       	Base, Size);
}


void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
  pVidMem->linearSupported = TRUE;
  pVidMem->mapMem = MapVidMem;
  pVidMem->unmapMem = UnMapVidMem;
  pVidMem->setWC = 0;
  pVidMem->undoWC = 0;
#ifdef HAS_MTRR_SUPPORT
  if (cleanMTRR()) {
	pVidMem->setWC = setWC;
	pVidMem->undoWC = undoWC;
  }
#endif
  pVidMem->initialised = TRUE;
}


/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

_X_EXPORT Bool
xf86DisableInterrupts()
{
	return(TRUE);
}

_X_EXPORT void
xf86EnableInterrupts()
{
	return;
}

/***************************************************************************/
/* I/O Permissions section for PowerPC                                     */
/***************************************************************************/

#if defined(__powerpc__)

_X_EXPORT volatile unsigned char *ioBase = MAP_FAILED;
volatile unsigned char *pciConfBase = MAP_FAILED;

static int IOEnabled;


static void
removeIOSmem(void)
{
	smem_create(NULL, (char *) ioBase, 0, SM_DETACH);
	smem_remove("IOBASE");
	ioBase = MAP_FAILED;	
}

_X_EXPORT Bool
xf86EnableIO()
{
	if (IOEnabled++ == 0) {
	    ioBase = (unsigned char *) smem_create("IOBASE",
       			(char *)PHYS_ISA_IO_SPACE, 64*1024, SM_READ|SM_WRITE);
	       	if (ioBase == MAP_FAILED) {
       			--IOEnabled;
			xf86Msg(X_WARNING,"xf86EnableIO: Failed to map I/O\n");
			return FALSE;
       		} else {
#ifdef DEBUG
			ErrorF("xf86EnableIO: mapped I/O at vaddr 0x%08x\n",
				ioBase);
#endif
			atexit(removeIOSmem);
		}
	}        
	return TRUE;
}

_X_EXPORT void
xf86DisableIO()
{
	if (!IOEnabled)
		return;

        if (--IOEnabled == 0) 
        	removeIOSmem();
	return;
}

#if 0
void
xf86DisableIOPrivs(void)
{
	return;
}
#endif
void
ppcPciIoMap(int bus)
{
	xf86EnableIO();
}

#endif


#ifdef HAS_MTRR_SUPPORT
/* memory range (MTRR) support for LynxOS (taken from BSD MTRR support) */

/*
 * This code is experimental.  Some parts may be overkill, and other parts
 * may be incomplete.
 */

/*
 * getAllRanges returns the full list of memory ranges with attributes set.
 */

static struct mem_range_desc *
getAllRanges(int *nmr)
{
	struct mem_range_desc *mrd;
	struct mem_range_op mro;

	/*
	 * Find how many ranges there are.  If this fails, then the kernel
	 * probably doesn't have MTRR support.
	 */
	mro.mo_arg[0] = 0;
	if (ioctl(devMemFd, MEMRANGE_GET, &mro))
		return NULL;
	*nmr = mro.mo_arg[0];
	mrd = xnfalloc(*nmr * sizeof(struct mem_range_desc));
	mro.mo_arg[0] = *nmr;
	mro.mo_desc = mrd;
	if (ioctl(devMemFd, MEMRANGE_GET, &mro)) {
		xfree(mrd);
		return NULL;
	}
	return mrd;
}

/*
 * cleanMTRR removes any memory attribute that may be left by a previous
 * X server.  Normally there won't be any, but this takes care of the
 * case where a server crashed without being able finish cleaning up.
 */

static Bool
cleanMTRR()
{
	struct mem_range_desc *mrd;
	struct mem_range_op mro;
	int nmr, i;

	/* This shouldn't happen */
	if (devMemFd < 0) {
		if ((devMemFd = open(MTRR_DEVICE, O_RDONLY)) < 0) {
perror("open MTRR");
			return FALSE;
		}
	}

	if (!(mrd = getAllRanges(&nmr)))
		return FALSE;

	for (i = 0; i < nmr; i++) {
		if (strcmp(mrd[i].mr_owner, X_MTRR_ID) == 0 &&
		    (mrd[i].mr_flags & MDF_ACTIVE)) {
#ifdef DEBUG
			ErrorF("Clean for (0x%lx,0x%lx)\n",
				(unsigned long)mrd[i].mr_base,
				(unsigned long)mrd[i].mr_len);
#endif
			if (mrd[i].mr_flags & MDF_FIXACTIVE) {
				mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
				mrd[i].mr_flags = MDF_UNCACHEABLE;
			} else {
				mro.mo_arg[0] = MEMRANGE_SET_REMOVE;
			}
			mro.mo_desc = mrd + i;
			ioctl(devMemFd, MEMRANGE_SET, &mro);
		}
	}
#ifdef DEBUG
	sleep(10);
#endif
	xfree(mrd);
	return TRUE;
}

typedef struct x_RangeRec {
	struct mem_range_desc	mrd;
	Bool			wasWC;
	struct x_RangeRec *	next;
} RangeRec, *RangePtr;

static void
freeRangeList(RangePtr range)
{
	RangePtr rp;

	while (range) {
		rp = range;
		range = rp->next;
		xfree(rp);
	}
}

static RangePtr
dupRangeList(RangePtr list)
{
	RangePtr new = NULL, rp, p;

	rp = list;
	while (rp) {
		p = xnfalloc(sizeof(RangeRec));
		*p = *rp;
		p->next = new;
		new = p;
		rp = rp->next;
	}
	return new;
}

static RangePtr
sortRangeList(RangePtr list)
{
	RangePtr rp1, rp2, copy, sorted = NULL, minp, prev, minprev;
	unsigned long minBase;

	/* Sort by base address */
	rp1 = copy = dupRangeList(list);
	while (rp1) {
		minBase = rp1->mrd.mr_base;
		minp = rp1;
		minprev = NULL;
		prev = rp1;
		rp2 = rp1->next;
		while (rp2) {
			if (rp2->mrd.mr_base < minBase) {
				minBase = rp2->mrd.mr_base;
				minp = rp2;
				minprev = prev;
			}
			prev = rp2;
			rp2 = rp2->next;
		}
		if (minprev) {
			minprev->next = minp->next;
			rp1 = copy;
		} else {
			rp1 = minp->next;
		}
		minp->next = sorted;
		sorted = minp;
	}
	return sorted;
}

/*
 * findRanges returns a list of ranges that overlap the specified range.
 */

static void
findRanges(unsigned long base, unsigned long size, RangePtr *ucp, RangePtr *wcp)
{
	struct mem_range_desc *mrd;
	int nmr, i;
	RangePtr rp, *p;
	
	if (!(mrd = getAllRanges(&nmr)))
		return;

	for (i = 0; i < nmr; i++) {
		if ((mrd[i].mr_flags & MDF_ACTIVE) &&
		    mrd[i].mr_base < base + size &&
		    mrd[i].mr_base + mrd[i].mr_len > base) {
			if (mrd[i].mr_flags & MDF_WRITECOMBINE)
				p = wcp;
			else if (mrd[i].mr_flags & MDF_UNCACHEABLE)
				p = ucp;
			else
				continue;
			rp = xnfalloc(sizeof(RangeRec));
			rp->mrd = mrd[i];
			rp->next = *p;
			*p = rp;
		}
	}
	xfree(mrd);
}

/*
 * This checks if the existing overlapping ranges fully cover the requested
 * range.  Is this overkill?
 */

static Bool
fullCoverage(unsigned long base, unsigned long size, RangePtr overlap)
{
	RangePtr rp1, sorted = NULL;
	unsigned long end;

	sorted = sortRangeList(overlap);
	/* Look for gaps */
	rp1 = sorted;
	end = base + size;
	while (rp1) {
		if (rp1->mrd.mr_base > base) {
			freeRangeList(sorted);
			return FALSE;
		} else {
			base = rp1->mrd.mr_base + rp1->mrd.mr_len;
		}
		if (base >= end) {
			freeRangeList(sorted);
			return TRUE;
		}
		rp1 = rp1->next;
	}
	freeRangeList(sorted);
	return FALSE;
}

static pointer
addWC(int screenNum, unsigned long base, unsigned long size, MessageType from)
{
	RangePtr uc = NULL, wc = NULL, retlist = NULL;
	struct mem_range_desc mrd;
	struct mem_range_op mro;

	findRanges(base, size, &uc, &wc);

	/* See of the full range is already WC */
	if (!uc && fullCoverage(base, size, wc)) {
		xf86DrvMsg(screenNum, from, 
		   "Write-combining range (0x%lx,0x%lx) was already set\n",
		    base, size);
		return NULL;
	}

	/* Otherwise, try to add the new range */
	mrd.mr_base = base;
	mrd.mr_len = size;
	strcpy(mrd.mr_owner, X_MTRR_ID);
	mrd.mr_flags = MDF_WRITECOMBINE;
	mro.mo_desc = &mrd;
	mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
	if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
		xf86DrvMsg(screenNum, X_WARNING,
			   "Failed to set write-combining range "
			   "(0x%lx,0x%lx)\n", base, size);
		return NULL;
	} else {
		xf86DrvMsg(screenNum, from,
			   "Write-combining range (0x%lx,0x%lx)\n", base, size);
		retlist = xnfalloc(sizeof(RangeRec));
		retlist->mrd = mrd;
		retlist->wasWC = FALSE;
		retlist->next = NULL;
		return retlist;
	}
}

static pointer
delWC(int screenNum, unsigned long base, unsigned long size, MessageType from)
{
	RangePtr uc = NULL, wc = NULL, retlist = NULL;
	struct mem_range_desc mrd;
	struct mem_range_op mro;

	findRanges(base, size, &uc, &wc);

	/*
	 * See of the full range is already not WC, or if there is full
	 * coverage from UC ranges.
	 */
	if (!wc || fullCoverage(base, size, uc)) {
		xf86DrvMsg(screenNum, from, 
		   "Write-combining range (0x%lx,0x%lx) was already clear\n",
		    base, size);
		return NULL;
	}

	/* Otherwise, try to add the new range */
	mrd.mr_base = base;
	mrd.mr_len = size;
	strcpy(mrd.mr_owner, X_MTRR_ID);
	mrd.mr_flags = MDF_UNCACHEABLE;
	mro.mo_desc = &mrd;
	mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
	if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
		xf86DrvMsg(screenNum, X_WARNING,
			   "Failed to remove write-combining range "
			   "(0x%lx,0x%lx)\n", base, size);
		/* XXX Should then remove all of the overlapping WC ranges */
		return NULL;
	} else {
		xf86DrvMsg(screenNum, from,
			   "Removed Write-combining range (0x%lx,0x%lx)\n",
			   base, size);
		retlist = xnfalloc(sizeof(RangeRec));
		retlist->mrd = mrd;
		retlist->wasWC = TRUE;
		retlist->next = NULL;
		return retlist;
	}
}

static pointer
setWC(int screenNum, unsigned long base, unsigned long size, Bool enable,
	MessageType from)
{
	if (enable)
		return addWC(screenNum, base, size, from);
	else
		return delWC(screenNum, base, size, from);
}

static void
undoWC(int screenNum, pointer list)
{
	RangePtr rp;
	struct mem_range_op mro;
	Bool failed;

	rp = list;
	while (rp) {
#ifdef DEBUG
		ErrorF("Undo for (0x%lx,0x%lx), %d\n",
			(unsigned long)rp->mrd.mr_base,
			(unsigned long)rp->mrd.mr_len, rp->wasWC);
#endif
		failed = FALSE;
		if (rp->wasWC) {
			mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
			rp->mrd.mr_flags = MDF_WRITECOMBINE;
			strcpy(rp->mrd.mr_owner, "unknown");
		} else {
			mro.mo_arg[0] = MEMRANGE_SET_REMOVE;
		}
		mro.mo_desc = &rp->mrd;

		if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
			if (!rp->wasWC) {
				mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
				rp->mrd.mr_flags = MDF_UNCACHEABLE;
				strcpy(rp->mrd.mr_owner, "unknown");
				if (ioctl(devMemFd, MEMRANGE_SET, &mro))
					failed = TRUE;
			} else
				failed = TRUE;
		}
		if (failed) {
			xf86DrvMsg(screenNum, X_WARNING,
				"Failed to restore MTRR range (0x%lx,0x%lx)\n",
				(unsigned long)rp->mrd.mr_base,
				(unsigned long)rp->mrd.mr_len);
		}
		rp = rp->next;
	}
}

#endif /* HAS_MTRR_SUPPORT */

