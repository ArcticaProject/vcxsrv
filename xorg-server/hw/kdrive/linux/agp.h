/* COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2000, 2001 Nokia Home Communications

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

X Window System is a trademark of The Open Group */

/* agp.h - header file for KDrive AGP GART interface
 *
 * Author: Pontus Lidman <pontus.lidman@nokia.com>
 *
 */

#ifndef _AGP_H_
#define _AGP_H_

#include <X11/Xdefs.h>
#include <X11/Xmd.h>

/* These two definitions must be consistent with the kernel's,
   but using 1 or 2 in driver code is even uglier */
#define AGP_DCACHE_MEMORY 1
#define AGP_PHYS_MEMORY   2

typedef struct _AgpInfo {
	unsigned long	bridgeId;
	unsigned long	agpMode;
	unsigned long	base;
	unsigned long	size;
	unsigned long	totalPages;
	unsigned long	systemPages;
	unsigned long	usedPages;
} AgpInfo, *AgpInfoPtr;

extern Bool KdAgpGARTSupported(void);
extern AgpInfoPtr KdGetAGPInfo(int screenNum);
extern Bool KdAcquireGART(int screenNum);
extern Bool KdReleaseGART(int screenNum);
extern int KdAllocateGARTMemory(int screenNum, unsigned long size, int type,
				  unsigned long *physical);
extern Bool KdBindGARTMemory(int screenNum, int key, unsigned long offset);
extern Bool KdUnbindGARTMemory(int screenNum, int key);
extern Bool KdEnableAGP(int screenNum, CARD32 mode);

#endif /* _AGP_H_ */
