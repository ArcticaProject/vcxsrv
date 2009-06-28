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
#include "glxserver.h"
#include "unpack.h"
#include "g_disptab.h"

void __glXDispSwap_PolygonStipple(GLbyte *pc)
{
    __GLXpixelHeader *hdr = (__GLXpixelHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_Bitmap(GLbyte *pc)
{
    __GLXdispatchBitmapHeader *hdr = (__GLXdispatchBitmapHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->xorig);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->yorig);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->xmove);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->ymove);

    hdr->swapBytes = !hdr->swapBytes;

}

void __glXDispSwap_TexImage1D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->components);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->border);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_TexImage2D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->components);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->border);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_TexImage3D(GLbyte *pc)
{
    __GLXdispatchTexImage3DHeader *hdr = (__GLXdispatchTexImage3DHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->imageHeight);
    __GLX_SWAP_INT((GLbyte *)&hdr->imageDepth);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipImages);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipVolumes);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->internalformat);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->depth);
    __GLX_SWAP_INT((GLbyte *)&hdr->size4d);
    __GLX_SWAP_INT((GLbyte *)&hdr->border);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_DrawPixels(GLbyte *pc)
{
    __GLXdispatchDrawPixelsHeader *hdr = (__GLXdispatchDrawPixelsHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_TexSubImage1D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->xoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_TexSubImage2D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->xoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->yoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_TexSubImage3D(GLbyte *pc)
{
    __GLXdispatchTexSubImage3DHeader *hdr =
				(__GLXdispatchTexSubImage3DHeader *) pc;

    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->imageHeight);
    __GLX_SWAP_INT((GLbyte *)&hdr->imageDepth);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipImages);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipVolumes);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->xoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->yoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->zoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->depth);
    __GLX_SWAP_INT((GLbyte *)&hdr->size4d);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_ColorTable(GLbyte *pc)
{
    __GLXdispatchColorTableHeader *hdr =
				(__GLXdispatchColorTableHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->internalformat);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_ColorSubTable(GLbyte *pc)
{
    __GLXdispatchColorSubTableHeader *hdr =
				(__GLXdispatchColorSubTableHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->start);
    __GLX_SWAP_INT((GLbyte *)&hdr->count);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_ConvolutionFilter1D(GLbyte *pc)
{
    __GLXdispatchConvolutionFilterHeader *hdr =
				(__GLXdispatchConvolutionFilterHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->internalformat);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);

    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    hdr->swapBytes = !hdr->swapBytes;
}

void __glXDispSwap_ConvolutionFilter2D(GLbyte *pc)
{
    __GLXdispatchConvolutionFilterHeader *hdr =
				(__GLXdispatchConvolutionFilterHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

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
    hdr->swapBytes = !hdr->swapBytes;
}

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
    hdr->swapBytes = !hdr->swapBytes;
}
