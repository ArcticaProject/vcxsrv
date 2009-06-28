/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#define NEED_REPLIES
#define FONT_PCF
#include "glxserver.h"
#include <GL/glxtokens.h>
#include <pixmapstr.h>
#include <windowstr.h>
#include "glxutil.h"

/************************************************************************/

void __glXNop(void) {}

/************************************************************************/

/* Memory Allocation for GLX */

void *
__glXMalloc(size_t size)
{
    void *addr;

    if (size == 0) {
	return NULL;
    }
    addr = (void *) xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    return addr;
}

void *
__glXCalloc(size_t numElements, size_t elementSize)
{
    void *addr;
    size_t size;

    if ((numElements == 0) || (elementSize == 0)) {
	return NULL;
    }
    size = numElements * elementSize;
    addr = (void *) xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    memset(addr, 0, size);
    return addr;
}

void *
__glXRealloc(void *addr, size_t newSize)
{
    void *newAddr;

    if (addr) {
	if (newSize == 0) {
	    xfree(addr);
	    return NULL;
	} else {
	    newAddr = xrealloc(addr, newSize);
	}
    } else {
	if (newSize == 0) {
	    return NULL;
	} else {
	    newAddr = xalloc(newSize);
	}
    }
    if (newAddr == NULL) {
	return NULL;	/* XXX: out of memory */
    }

    return newAddr;
}

void
__glXFree(void *addr)
{
    if (addr) {
	xfree(addr);
    }
}
