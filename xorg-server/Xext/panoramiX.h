/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/


/* THIS IS NOT AN X PROJECT TEAM SPECIFICATION */

/*  
 *	PanoramiX definitions
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _PANORAMIX_H_
#define _PANORAMIX_H_

#define _PANORAMIX_SERVER
#include <X11/extensions/panoramiXext.h>
#undef _PANORAMIX_SERVER

#include "gcstruct.h"


typedef struct _PanoramiXData {
    int x;
    int y;
    int width;
    int height;
} PanoramiXData;

typedef struct _PanoramiXInfo {
    XID id ;
} PanoramiXInfo;

typedef struct {
    PanoramiXInfo info[MAXSCREENS];
    RESTYPE type;
    union {
	struct {
	    char   visibility;
	    char   class;
            char   root;
	} win;
	struct {
	    Bool shared;
	} pix;
#ifdef RENDER
	struct {
	    Bool root;
	} pict;
#endif
	char raw_data[4];
    } u;
} PanoramiXRes;

#define FOR_NSCREENS_FORWARD(j) for(j = 0; j < PanoramiXNumScreens; j++)
#define FOR_NSCREENS_BACKWARD(j) for(j = PanoramiXNumScreens - 1; j >= 0; j--)
#define FOR_NSCREENS(j) FOR_NSCREENS_FORWARD(j)

#define BREAK_IF(a) if ((a)) break
#define IF_RETURN(a,b) if ((a)) return (b)

#define FORCE_ROOT(a) { \
    int _j; \
    for (_j = PanoramiXNumScreens - 1; _j; _j--) \
        if ((a).root == WindowTable[_j]->drawable.id)   \
            break;                                      \
    (a).rootX += panoramiXdataPtr[_j].x;             \
    (a).rootY += panoramiXdataPtr[_j].y;             \
    (a).root = WindowTable[0]->drawable.id;          \
}

#define IS_SHARED_PIXMAP(r) (((r)->type == XRT_PIXMAP) && (r)->u.pix.shared)

#define SKIP_FAKE_WINDOW(a) if(!LookupIDByType(a, XRT_WINDOW)) return

#endif /* _PANORAMIX_H_ */
