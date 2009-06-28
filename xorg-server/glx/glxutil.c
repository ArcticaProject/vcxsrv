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
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include "glxserver.h"
#include "glxutil.h"

/************************************************************************/
/* Context stuff */


/*
** associate a context with a drawable
*/
void
__glXAssociateContext(__GLXcontext *glxc)
{
    glxc->nextDrawPriv = glxc->drawPriv->drawGlxc;
    glxc->drawPriv->drawGlxc = glxc;

    __glXRefDrawable(glxc->drawPriv);
    

    glxc->nextReadPriv = glxc->readPriv->readGlxc;
    glxc->readPriv->readGlxc = glxc;

    __glXRefDrawable(glxc->readPriv);
}

/*
** Deassociate a context from a drawable
*/
void
__glXDeassociateContext(__GLXcontext *glxc)
{
    __GLXcontext *curr, *prev;

    prev = NULL;
    if (glxc->drawPriv) {
        for ( curr = glxc->drawPriv->drawGlxc; curr != NULL
	      ; prev = curr, curr = curr->nextDrawPriv ) {
	    if (curr == glxc) {
	        /* found context.  Deassociate. */
	        if (prev == NULL) {
		    glxc->drawPriv->drawGlxc = curr->nextDrawPriv;
	        } else {
		    prev->nextDrawPriv = curr->nextDrawPriv;
	        }
	        curr->nextDrawPriv = NULL;
	        __glXUnrefDrawable(glxc->drawPriv);
	        break;
	    }
        }
    }

    prev = NULL;
    if (glxc->readPriv) {
        for ( curr = glxc->readPriv->readGlxc
	      ; curr != NULL 
	      ; prev = curr, curr = curr->nextReadPriv ) {
	    if (curr == glxc) {
	        /* found context.  Deassociate. */
	        if (prev == NULL) {
		    glxc->readPriv->readGlxc = curr->nextReadPriv;
	        } else {
		    prev->nextReadPriv = curr->nextReadPriv;
	        }
	        curr->nextReadPriv = NULL;
	        __glXUnrefDrawable(glxc->readPriv);
	       break;
	   }
       }
    }
}

/*****************************************************************************/
/* Drawable private stuff */

void
__glXRefDrawable(__GLXdrawable *glxPriv)
{
    glxPriv->refCount++;
}

void
__glXUnrefDrawable(__GLXdrawable *glxPriv)
{
    glxPriv->refCount--;
    if (glxPriv->refCount == 0) {
	/* remove the drawable from the drawable list */
	FreeResourceByType(glxPriv->drawId, __glXDrawableRes, FALSE);
	glxPriv->destroy(glxPriv);
    }
}

GLboolean
__glXDrawableInit(__GLXdrawable *drawable,
		  __GLXscreen *screen, DrawablePtr pDraw, int type,
		  XID drawId, __GLXconfig *config)
{
    drawable->pDraw = pDraw;
    drawable->type = type;
    drawable->drawId = drawId;
    drawable->refCount = 1;
    drawable->config = config;
    drawable->eventMask = 0;

    return GL_TRUE;
}
