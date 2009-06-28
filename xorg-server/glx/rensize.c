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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <GL/gl.h>
#include "glxserver.h"
#include "GL/glxproto.h"
#include "unpack.h"
#include "indirect_size.h"
#include "indirect_reqsize.h"

#define SWAPL(a) \
  (((a & 0xff000000U)>>24) | ((a & 0xff0000U)>>8) | \
   ((a & 0xff00U)<<8) | ((a & 0xffU)<<24))

static int Map1Size( GLint k, GLint order)
{
    if (order <= 0 || k < 0) return -1;
    return k * order;
}

int __glXMap1dReqSize( const GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint order, k;

    target = *(GLenum*) (pc + 16);
    order = *(GLint*) (pc + 20);
    if (swap) {
	target = SWAPL( target );
	order = SWAPL( order );
    }
    k = __glMap1d_size( target );
    return 8 * Map1Size( k, order );
}

int __glXMap1fReqSize( const GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint order, k;

    target = *(GLenum *)(pc + 0);
    order = *(GLint *)(pc + 12);
    if (swap) {
	target = SWAPL( target );
	order = SWAPL( order );
    }
    k = __glMap1f_size(target);
    return 4 * Map1Size(k, order);
}

static int Map2Size(int k, int majorOrder, int minorOrder)
{
    if (majorOrder <= 0 || minorOrder <= 0 || k < 0) return -1;
    return k * majorOrder * minorOrder;
}

int __glXMap2dReqSize( const GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint uorder, vorder, k;

    target = *(GLenum *)(pc + 32);
    uorder = *(GLint *)(pc + 36);
    vorder = *(GLint *)(pc + 40);
    if (swap) {
	target = SWAPL( target );
	uorder = SWAPL( uorder );
	vorder = SWAPL( vorder );
    }
    k = __glMap2d_size( target );
    return 8 * Map2Size( k, uorder, vorder );
}

int __glXMap2fReqSize( const GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint uorder, vorder, k;

    target = *(GLenum *)(pc + 0);
    uorder = *(GLint *)(pc + 12);
    vorder = *(GLint *)(pc + 24);
    if (swap) {
	target = SWAPL( target );
	uorder = SWAPL( uorder );
	vorder = SWAPL( vorder );
    }
    k = __glMap2f_size( target );
    return 4 * Map2Size( k, uorder, vorder );
}

/**
 * Calculate the size of an image.
 * 
 * The size of an image sent to the server from the client or sent from the
 * server to the client is calculated.  The size is based on the dimensions
 * of the image, the type of pixel data, padding in the image, and the
 * alignment requirements of the image.
 * 
 * \param format       Format of the pixels.  Same as the \c format parameter
 *                     to \c glTexImage1D
 * \param type         Type of the pixel data.  Same as the \c type parameter
 *                     to \c glTexImage1D
 * \param target       Typically the texture target of the image.  If the
 *                     target is one of \c GL_PROXY_*, the size returned is
 *                     always zero. For uses that do not have a texture target
 *                     (e.g, glDrawPixels), zero should be specified.
 * \param w            Width of the image data.  Must be >= 1.
 * \param h            Height of the image data.  Must be >= 1, even for 1D
 *                     images.
 * \param d            Depth of the image data.  Must be >= 1, even for 1D or
 *                     2D images.
 * \param imageHeight  If non-zero, defines the true height of a volumetric
 *                     image.  This value will be used instead of \c h for
 *                     calculating the size of the image.
 * \param rowLength    If non-zero, defines the true width of an image.  This
 *                     value will be used instead of \c w for calculating the
 *                     size of the image.
 * \param skipImages   Number of extra layers of image data in a volumtric
 *                     image that are to be skipped before the real data.
 * \param skipRows     Number of extra rows of image data in an image that are
 *                     to be skipped before the real data.
 * \param alignment    Specifies the alignment for the start of each pixel row
 *                     in memory.  This value must be one of 1, 2, 4, or 8.
 *
 * \returns
 * The size of the image is returned.  If the specified \c format and \c type
 * are invalid, -1 is returned.  If \c target is one of \c GL_PROXY_*, zero
 * is returned.
 */
int __glXImageSize( GLenum format, GLenum type, GLenum target,
		    GLsizei w, GLsizei h, GLsizei d,
		    GLint imageHeight, GLint rowLength,
		    GLint skipImages, GLint skipRows, GLint alignment )
{
    GLint bytesPerElement, elementsPerGroup, groupsPerRow;
    GLint groupSize, rowSize, padding, imageSize;

    if (w < 0 || h < 0 || d < 0 ||
	(type == GL_BITMAP &&
	 (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX))) {
	return -1;
    }
    if (w==0 || h==0 || d == 0) return 0;

    switch( target ) {
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
    case GL_PROXY_TEXTURE_3D:
    case GL_PROXY_TEXTURE_4D_SGIS:
    case GL_PROXY_TEXTURE_CUBE_MAP:
    case GL_PROXY_TEXTURE_RECTANGLE_ARB:
    case GL_PROXY_HISTOGRAM:
    case GL_PROXY_COLOR_TABLE:
    case GL_PROXY_TEXTURE_COLOR_TABLE_SGI:
    case GL_PROXY_POST_CONVOLUTION_COLOR_TABLE:
    case GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE:
    case GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP:
	return 0;
    }

    if (type == GL_BITMAP) {
	if (rowLength > 0) {
	    groupsPerRow = rowLength;
	} else {
	    groupsPerRow = w;
	}
	rowSize = (groupsPerRow + 7) >> 3;
	padding = (rowSize % alignment);
	if (padding) {
	    rowSize += alignment - padding;
	}
	return ((h + skipRows) * rowSize);
    } else {
	switch(format) {
	  case GL_COLOR_INDEX:
	  case GL_STENCIL_INDEX:
	  case GL_DEPTH_COMPONENT:
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_ALPHA:
	  case GL_LUMINANCE:
	  case GL_INTENSITY:
	    elementsPerGroup = 1;
	    break;
	  case GL_422_EXT:
	  case GL_422_REV_EXT:
	  case GL_422_AVERAGE_EXT:
	  case GL_422_REV_AVERAGE_EXT:
	  case GL_DEPTH_STENCIL_NV:
	  case GL_DEPTH_STENCIL_MESA:
	  case GL_YCBCR_MESA:
	  case GL_LUMINANCE_ALPHA:
	    elementsPerGroup = 2;
	    break;
	  case GL_RGB:
	  case GL_BGR:
	    elementsPerGroup = 3;
	    break;
	  case GL_RGBA:
	  case GL_BGRA:
	  case GL_ABGR_EXT:
	    elementsPerGroup = 4;
	    break;
	  default:
	    return -1;
	}
	switch(type) {
	  case GL_UNSIGNED_BYTE:
	  case GL_BYTE:
	    bytesPerElement = 1;
	    break;
	  case GL_UNSIGNED_BYTE_3_3_2:
	  case GL_UNSIGNED_BYTE_2_3_3_REV:
	    bytesPerElement = 1;	    
	    elementsPerGroup = 1;
	    break;
	  case GL_UNSIGNED_SHORT:
	  case GL_SHORT:
	    bytesPerElement = 2;
	    break;
	  case GL_UNSIGNED_SHORT_5_6_5:
	  case GL_UNSIGNED_SHORT_5_6_5_REV:
	  case GL_UNSIGNED_SHORT_4_4_4_4:
 	  case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	  case GL_UNSIGNED_SHORT_5_5_5_1:
	  case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	  case GL_UNSIGNED_SHORT_8_8_APPLE:
	  case GL_UNSIGNED_SHORT_8_8_REV_APPLE:
	  case GL_UNSIGNED_SHORT_15_1_MESA:
	  case GL_UNSIGNED_SHORT_1_15_REV_MESA:
	    bytesPerElement = 2;
	    elementsPerGroup = 1;
	    break;
	  case GL_INT:
	  case GL_UNSIGNED_INT:
	  case GL_FLOAT:
	    bytesPerElement = 4;
	    break;
	  case GL_UNSIGNED_INT_8_8_8_8:
	  case GL_UNSIGNED_INT_8_8_8_8_REV:
	  case GL_UNSIGNED_INT_10_10_10_2:
	  case GL_UNSIGNED_INT_2_10_10_10_REV:
	  case GL_UNSIGNED_INT_24_8_NV:
	  case GL_UNSIGNED_INT_24_8_MESA:
	  case GL_UNSIGNED_INT_8_24_REV_MESA:
	    bytesPerElement = 4;
	    elementsPerGroup = 1;
	    break;
	  default:
	    return -1;
	}
	groupSize = bytesPerElement * elementsPerGroup;
	if (rowLength > 0) {
	    groupsPerRow = rowLength;
	} else {
	    groupsPerRow = w;
	}
	rowSize = groupsPerRow * groupSize;
	padding = (rowSize % alignment);
	if (padding) {
	    rowSize += alignment - padding;
	}
	if (imageHeight > 0) {
	    imageSize = (imageHeight + skipRows) * rowSize;
	} else {
	    imageSize = (h + skipRows) * rowSize;
	}
	return ((d + skipImages) * imageSize);
    }
}


/* XXX this is used elsewhere - should it be exported from glxserver.h? */
int __glXTypeSize(GLenum enm)
{
  switch(enm) {
    case GL_BYTE:		return sizeof(GLbyte);
    case GL_UNSIGNED_BYTE:	return sizeof(GLubyte);
    case GL_SHORT:		return sizeof(GLshort);
    case GL_UNSIGNED_SHORT:	return sizeof(GLushort);
    case GL_INT:		return sizeof(GLint);
    case GL_UNSIGNED_INT:	return sizeof(GLint);
    case GL_FLOAT:		return sizeof(GLfloat);
    case GL_DOUBLE:		return sizeof(GLdouble);
    default:			return -1;
  }
}

int __glXDrawArraysReqSize( const GLbyte *pc, Bool swap )
{
    __GLXdispatchDrawArraysHeader *hdr = (__GLXdispatchDrawArraysHeader *) pc;
    __GLXdispatchDrawArraysComponentHeader *compHeader;
    GLint numVertexes = hdr->numVertexes;
    GLint numComponents = hdr->numComponents;
    GLint arrayElementSize = 0;
    int i;

    if (swap) {
	numVertexes = SWAPL( numVertexes );
	numComponents = SWAPL( numComponents );
    }

    pc += sizeof(__GLXdispatchDrawArraysHeader);
    compHeader = (__GLXdispatchDrawArraysComponentHeader *) pc;

    for (i=0; i<numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;
	GLint component = compHeader[i].component;

	if (swap) {
	    datatype = SWAPL( datatype );
	    numVals = SWAPL( numVals );
	    component = SWAPL( component );
	}

	switch (component) {
	  case GL_VERTEX_ARRAY:
	  case GL_COLOR_ARRAY:
	  case GL_TEXTURE_COORD_ARRAY:
	    break;
	  case GL_SECONDARY_COLOR_ARRAY:
	  case GL_NORMAL_ARRAY:
	    if (numVals != 3) {
		/* bad size */
		return -1;
	    }
	    break;
	  case GL_FOG_COORD_ARRAY:
	  case GL_INDEX_ARRAY:
	    if (numVals != 1) {
		/* bad size */
		return -1;
	    }
	    break;
	  case GL_EDGE_FLAG_ARRAY:
	    if ((numVals != 1) && (datatype != GL_UNSIGNED_BYTE)) {
		/* bad size or bad type */
		return -1;
	    }
	    break;
	  default:
	    /* unknown component type */
	    return -1;
	}

	arrayElementSize += __GLX_PAD(numVals * __glXTypeSize(datatype));

	pc += sizeof(__GLXdispatchDrawArraysComponentHeader);
    }

    return ((numComponents * sizeof(__GLXdispatchDrawArraysComponentHeader)) +
	    (numVertexes * arrayElementSize));
}

int __glXSeparableFilter2DReqSize( const GLbyte *pc, Bool swap )
{
    __GLXdispatchConvolutionFilterHeader *hdr =
			(__GLXdispatchConvolutionFilterHeader *) pc;

    GLint image1size, image2size;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint h = hdr->height;
    GLint rowLength = hdr->rowLength;
    GLint alignment = hdr->alignment;

    if (swap) {
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	h = SWAPL( h );
	rowLength = SWAPL( rowLength );
	alignment = SWAPL( alignment );
    }

    /* XXX Should rowLength be used for either or both image? */
    image1size = __glXImageSize( format, type, 0, w, 1, 1,
				 0, rowLength, 0, 0, alignment );
    image1size = __GLX_PAD(image1size);
    image2size = __glXImageSize( format, type, 0, h, 1, 1,
				 0, rowLength, 0, 0, alignment );
    return image1size + image2size;

}
