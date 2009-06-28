/*
 * Abstraction of the AGP GART interface.
 *
 * This version is for both Linux and FreeBSD.
 *
 * Copyright © 2000-2001 Nokia Home Communications
 * Copyright © 2000 VA Linux Systems, Inc.
 
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

 */

/*
 * Author: Pontus Lidman <pontus.lidman@nokia.com> (adaption to KDrive) and others
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include <X11/X.h>
#include "misc.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "agp.h"

#if defined(linux)
#include <asm/ioctl.h>

#include <linux/agpgart.h>

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/ioctl.h>
#include <sys/agpio.h>
#endif

#ifndef AGP_DEVICE
#define AGP_DEVICE		"/dev/agpgart"
#endif
/* AGP page size is independent of the host page size. */
#ifndef AGP_PAGE_SIZE
#define AGP_PAGE_SIZE		4096
#endif
#define AGPGART_MAJOR_VERSION	0
#define AGPGART_MINOR_VERSION	99

static int gartFd = -1;
static int acquiredScreen = -1;

/*
 * Open /dev/agpgart.  Keep it open until server exit.
 */

static Bool
GARTInit(void)
{
	static Bool initDone = FALSE;
	struct _agp_info agpinf;

	if (initDone)
		return (gartFd != -1);

	initDone = TRUE;

	if (gartFd == -1)
		gartFd = open(AGP_DEVICE, O_RDWR, 0);
	else
		return FALSE;

	if (gartFd == -1) {
            fprintf(stderr, "Unable to open " AGP_DEVICE " (%s)\n",
                    strerror(errno)); 
            return FALSE;
	}

	KdAcquireGART(-1);
	/* Check the kernel driver version. */
	if (ioctl(gartFd, AGPIOC_INFO, &agpinf) != 0) {
            fprintf(stderr, "GARTInit: AGPIOC_INFO failed (%s)\n", 
                    strerror(errno)); 
            close(gartFd);
            gartFd = -1;
            return FALSE;
	}
	KdReleaseGART(-1);

#if defined(linux)
	/* Per Dave Jones, every effort will be made to keep the
	 * agpgart interface backwards compatible, so allow all
	 * future versions.
	 */
	if (
#if (AGPGART_MAJOR_VERSION > 0) /* quiet compiler */
	    agpinf.version.major < AGPGART_MAJOR_VERSION ||
#endif
	    (agpinf.version.major == AGPGART_MAJOR_VERSION &&
	     agpinf.version.minor < AGPGART_MINOR_VERSION)) {
            fprintf(stderr, 
                    "Kernel agpgart driver version is not current" 
                    " (%d.%d vs %d.%d)\n", 
                    agpinf.version.major, agpinf.version.minor, 
                    AGPGART_MAJOR_VERSION, AGPGART_MINOR_VERSION); 
            close(gartFd);
            gartFd = -1;
            return FALSE;
	}
#endif
	
	return TRUE;
}

Bool
KdAgpGARTSupported()
{
	return GARTInit();
}

AgpInfoPtr
KdGetAGPInfo(int screenNum)
{
	struct _agp_info agpinf;
	AgpInfoPtr info;

	if (!GARTInit())
		return NULL;


	if ((info = calloc(sizeof(AgpInfo), 1)) == NULL) {
            fprintf(stderr, "Failed to allocate AgpInfo\n"); 
            return NULL;
	}

	if (ioctl(gartFd, AGPIOC_INFO, &agpinf) != 0) {
            fprintf(stderr, 
                    "xf86GetAGPInfo: AGPIOC_INFO failed (%s)\n", 
                    strerror(errno)); 
            return NULL;
	}

	info->bridgeId = agpinf.bridge_id;
	info->agpMode = agpinf.agp_mode;
	info->base = agpinf.aper_base;
	info->size = agpinf.aper_size;
	info->totalPages = agpinf.pg_total;
	info->systemPages = agpinf.pg_system;
	info->usedPages = agpinf.pg_used;

	return info;
}

/*
 * XXX If multiple screens can acquire the GART, should we have a reference
 * count instead of using acquiredScreen?
 */

Bool
KdAcquireGART(int screenNum)
{
    if (screenNum != -1 && !GARTInit())
        return FALSE;
    
    if (screenNum == -1 || acquiredScreen != screenNum) {
        if (ioctl(gartFd, AGPIOC_ACQUIRE, 0) != 0) {
            fprintf(stderr, 
                    "AGPIOC_ACQUIRE failed (%s)\n", 
                    strerror(errno)); 
            return FALSE;
        }
        acquiredScreen = screenNum;
    }
    
    return TRUE;
}

Bool
KdReleaseGART(int screenNum)
{
	if (screenNum != -1 && !GARTInit())
		return FALSE;

	if (acquiredScreen == screenNum) {
		if (ioctl(gartFd, AGPIOC_RELEASE, 0) != 0) {
 			fprintf(stderr,
 				   "AGPIOC_RELEASE failed (%s)\n",
 				   strerror(errno)); 
			return FALSE;
		}
		acquiredScreen = -1;
		return TRUE;
	}
	return FALSE;
}

int
KdAllocateGARTMemory(int screenNum, unsigned long size, int type,
			unsigned long *physical)
{
	struct _agp_allocate alloc;
	int pages;

	/*
	 * Allocates "size" bytes of GART memory (rounds up to the next
	 * page multiple) or type "type".  A handle (key) for the allocated
	 * memory is returned.  On error, the return value is -1.
	 */

	if (!GARTInit() || acquiredScreen != screenNum)
		return -1;

	pages = (size / AGP_PAGE_SIZE);
	if (size % AGP_PAGE_SIZE != 0)
		pages++;

	/* XXX check for pages == 0? */

	alloc.pg_count = pages;
	alloc.type = type;

	if (ioctl(gartFd, AGPIOC_ALLOCATE, &alloc) != 0) {
            fprintf(stderr, "KdAllocateGARTMemory: " 
                    "allocation of %d pages failed\n\t(%s)\n", pages, 
                    strerror(errno)); 
		return -1;
	}

	if (physical)
		*physical = alloc.physical;

	return alloc.key;
}


/* Bind GART memory with "key" at "offset" */
Bool
KdBindGARTMemory(int screenNum, int key, unsigned long offset)
{
	struct _agp_bind bind;
	int pageOffset;

	if (!GARTInit() || acquiredScreen != screenNum)
		return FALSE;

	if (acquiredScreen != screenNum) {
            fprintf(stderr, 
                    "AGP not acquired by this screen\n"); 
            return FALSE;
	}

	if (offset % AGP_PAGE_SIZE != 0) {
            fprintf(stderr, "KdBindGARTMemory: " 
                    "offset (0x%lx) is not page-aligned (%d)\n", 
                    offset, AGP_PAGE_SIZE); 
            return FALSE;
	}
	pageOffset = offset / AGP_PAGE_SIZE;

	bind.pg_start = pageOffset;
	bind.key = key;

	if (ioctl(gartFd, AGPIOC_BIND, &bind) != 0) {
		fprintf(stderr, "KdBindGARTMemory: "
                        "binding of gart memory with key %d\n"
                        "\tat offset 0x%lx failed (%s)\n",
                        key, offset, strerror(errno));
		return FALSE;
	}

	return TRUE;
}


/* Unbind GART memory with "key" */
Bool
KdUnbindGARTMemory(int screenNum, int key)
{
	struct _agp_unbind unbind;

	if (!GARTInit() || acquiredScreen != screenNum)
		return FALSE;

	if (acquiredScreen != screenNum) {
		fprintf(stderr,
			   "AGP not acquired by this screen\n");
		return FALSE;
	}

	unbind.priority = 0;
	unbind.key = key;

	if (ioctl(gartFd, AGPIOC_UNBIND, &unbind) != 0) {
		fprintf(stderr, "KdUnbindGARTMemory: "
			   "unbinding of gart memory with key %d "
			   "failed (%s)\n", key, strerror(errno));
		return FALSE;
	}

	return TRUE;
}


/* XXX Interface may change. */
Bool
KdEnableAGP(int screenNum, CARD32 mode)
{
	agp_setup setup;

	if (!GARTInit() || acquiredScreen != screenNum)
		return FALSE;

	setup.agp_mode = mode;
	if (ioctl(gartFd, AGPIOC_SETUP, &setup) != 0) {
		fprintf(stderr, "KdEnableAGP: "
			   "AGPIOC_SETUP with mode %ld failed (%s)\n",
			   mode, strerror(errno));
		return FALSE;
	}

	return TRUE;
}

