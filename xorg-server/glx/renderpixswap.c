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
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "unpack.h"
#include "indirect_dispatch.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"

void __glXDispSwap_SeparableFilter2D(GLbyte *pc)
{
    __GLXdispatchConvolutionFilterHeader *hdr =
				(__GLXdispatchConvolutionFilterHeader *) pc;
    GLint hdrlen, image1len;
    __GLX_DECLARE_SWAP_VARIABLES;

    hdrlen = __GLX_PAD(__GLX_CONV_FILT_CMD_HDR_SIZE);

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->internalformat);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_SWAP_BYTES, !hdr->swapBytes) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_LSB_FIRST, hdr->lsbFirst) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_ROW_LENGTH, hdr->rowLength) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_SKIP_ROWS, hdr->skipRows) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_SKIP_PIXELS, hdr->skipPixels) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_UNPACK_ALIGNMENT, hdr->alignment) );

    /* XXX check this usage - internal code called
    ** a version without the packing parameters
    */
    image1len = __glXImageSize(hdr->format, hdr->type, 0, hdr->width, 1, 1,
			       0, hdr->rowLength, 0, hdr->skipRows,
			       hdr->alignment);
    image1len = __GLX_PAD(image1len);


    CALL_SeparableFilter2D( GET_DISPATCH(), (hdr->target, hdr->internalformat,
		 hdr->width, hdr->height, hdr->format, hdr->type,
		 ((GLubyte *)hdr+hdrlen), ((GLubyte *)hdr+hdrlen+image1len)) );
}
