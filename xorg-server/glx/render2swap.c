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

/* #define NEED_REPLIES */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "unpack.h"
#include "indirect_size.h"
#include "indirect_dispatch.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"


void __glXDispSwap_Map1f(GLbyte *pc)
{
    GLint order, k;
    GLfloat u1, u2, *points;
    GLenum target;
    GLint compsize;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_INT(pc + 0);
    __GLX_SWAP_INT(pc + 12);
    __GLX_SWAP_FLOAT(pc + 4);
    __GLX_SWAP_FLOAT(pc + 8);
    
    target = *(GLenum *)(pc + 0); 
    order = *(GLint *)(pc + 12);
    u1 = *(GLfloat *)(pc + 4);
    u2 = *(GLfloat *)(pc + 8);
    points = (GLfloat *)(pc + 16);
    k = __glMap1f_size(target);

    if (order <= 0 || k < 0) {
	/* Erroneous command. */
	compsize = 0;
    } else {
	compsize = order * k;
    }
    __GLX_SWAP_FLOAT_ARRAY(points, compsize);

    CALL_Map1f( GET_DISPATCH(), (target, u1, u2, k, order, points) );
}

void __glXDispSwap_Map2f(GLbyte *pc)
{
    GLint uorder, vorder, ustride, vstride, k;
    GLfloat u1, u2, v1, v2, *points;
    GLenum target;
    GLint compsize;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_INT(pc + 0);
    __GLX_SWAP_INT(pc + 12);
    __GLX_SWAP_INT(pc + 24);
    __GLX_SWAP_FLOAT(pc + 4);
    __GLX_SWAP_FLOAT(pc + 8);
    __GLX_SWAP_FLOAT(pc + 16);
    __GLX_SWAP_FLOAT(pc + 20);
    
    target = *(GLenum *)(pc + 0); 
    uorder = *(GLint *)(pc + 12);
    vorder = *(GLint *)(pc + 24);
    u1 = *(GLfloat *)(pc + 4);
    u2 = *(GLfloat *)(pc + 8);
    v1 = *(GLfloat *)(pc + 16);
    v2 = *(GLfloat *)(pc + 20);
    points = (GLfloat *)(pc + 28);

    k = __glMap2f_size(target);
    ustride = vorder * k;
    vstride = k;

    if (vorder <= 0 || uorder <= 0 || k < 0) {
	/* Erroneous command. */
	compsize = 0;
    } else {
	compsize = uorder * vorder * k;
    }
    __GLX_SWAP_FLOAT_ARRAY(points, compsize);

    CALL_Map2f( GET_DISPATCH(), (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) );
}

void __glXDispSwap_Map1d(GLbyte *pc)
{
    GLint order, k, compsize;
    GLenum target;
    GLdouble u1, u2, *points;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_DOUBLE(pc + 0);
    __GLX_SWAP_DOUBLE(pc + 8);
    __GLX_SWAP_INT(pc + 16);
    __GLX_SWAP_INT(pc + 20);

    target = *(GLenum*) (pc + 16);
    order = *(GLint*) (pc + 20);
    k = __glMap1d_size(target);
    if (order <= 0 || k < 0) {
	/* Erroneous command. */
	compsize = 0;
    } else {
	compsize = order * k;
    }
    __GLX_GET_DOUBLE(u1,pc);
    __GLX_GET_DOUBLE(u2,pc+8);
    __GLX_SWAP_DOUBLE_ARRAY(pc+24, compsize);
    pc += 24;

#ifdef __GLX_ALIGN64
    if (((unsigned long)pc) & 7) {
	/*
	** Copy the doubles up 4 bytes, trashing the command but aligning
	** the data in the process
	*/
	__GLX_MEM_COPY(pc-4, pc, compsize*8);
	points = (GLdouble*) (pc - 4);
    } else {
	points = (GLdouble*) pc;
    }
#else
    points = (GLdouble*) pc;
#endif
    CALL_Map1d( GET_DISPATCH(), (target, u1, u2, k, order, points) );
}

void __glXDispSwap_Map2d(GLbyte *pc)
{
    GLdouble u1, u2, v1, v2, *points;
    GLint uorder, vorder, ustride, vstride, k, compsize;
    GLenum target;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_DOUBLE(pc + 0);
    __GLX_SWAP_DOUBLE(pc + 8);
    __GLX_SWAP_DOUBLE(pc + 16);
    __GLX_SWAP_DOUBLE(pc + 24);
    __GLX_SWAP_INT(pc + 32);
    __GLX_SWAP_INT(pc + 36);
    __GLX_SWAP_INT(pc + 40);

    target = *(GLenum *)(pc + 32);
    uorder = *(GLint *)(pc + 36);
    vorder = *(GLint *)(pc + 40);
    k = __glMap2d_size(target);
    if (vorder <= 0 || uorder <= 0 || k < 0) {
	/* Erroneous command. */
	compsize = 0;
    } else {
	compsize = uorder * vorder * k;
    }
    __GLX_GET_DOUBLE(u1,pc);
    __GLX_GET_DOUBLE(u2,pc+8);
    __GLX_GET_DOUBLE(v1,pc+16);
    __GLX_GET_DOUBLE(v2,pc+24);
    __GLX_SWAP_DOUBLE_ARRAY(pc+44, compsize);
    pc += 44;
    ustride = vorder * k;
    vstride = k;

#ifdef __GLX_ALIGN64
    if (((unsigned long)pc) & 7) {
	/*
	** Copy the doubles up 4 bytes, trashing the command but aligning
	** the data in the process
	*/
	__GLX_MEM_COPY(pc-4, pc, compsize*8);
	points = (GLdouble*) (pc - 4);
    } else {
	points = (GLdouble*) pc;
    }
#else
    points = (GLdouble*) pc;
#endif
    CALL_Map2d( GET_DISPATCH(), (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) );
}

static void swapArray(GLint numVals, GLenum datatype,
                      GLint stride, GLint numVertexes, GLbyte *pc)
{
    int i,j;
    __GLX_DECLARE_SWAP_VARIABLES;

    switch (datatype) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	/* don't need to swap */
	return;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	for (i=0; i<numVertexes; i++) {
	    GLshort *pVal = (GLshort *) pc;
	    for (j=0; j<numVals; j++) {
		__GLX_SWAP_SHORT(&pVal[j]);
	    }
	    pc += stride;
	}
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
	for (i=0; i<numVertexes; i++) {
	    GLint *pVal = (GLint *) pc;
	    for (j=0; j<numVals; j++) {
		__GLX_SWAP_INT(&pVal[j]);
	    }
	    pc += stride;
	}
	break;
      case GL_FLOAT:
	for (i=0; i<numVertexes; i++) {
	    GLfloat *pVal = (GLfloat *) pc;
	    for (j=0; j<numVals; j++) {
		__GLX_SWAP_FLOAT(&pVal[j]);
	    }
	    pc += stride;
	}
	break;
      case GL_DOUBLE:
	for (i=0; i<numVertexes; i++) {
	    GLdouble *pVal = (GLdouble *) pc;
	    for (j=0; j<numVals; j++) {
		__GLX_SWAP_DOUBLE(&pVal[j]);
	    }
	    pc += stride;
	}
	break;
      default:
	return;
    }
}

void __glXDispSwap_DrawArrays(GLbyte *pc)
{
    __GLXdispatchDrawArraysHeader *hdr = (__GLXdispatchDrawArraysHeader *)pc;
    __GLXdispatchDrawArraysComponentHeader *compHeader;
    GLint numVertexes = hdr->numVertexes;
    GLint numComponents = hdr->numComponents;
    GLenum primType = hdr->primType;
    GLint stride = 0;
    int i;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&numVertexes);
    __GLX_SWAP_INT(&numComponents);
    __GLX_SWAP_INT(&primType);

    pc += sizeof(__GLXdispatchDrawArraysHeader);
    compHeader = (__GLXdispatchDrawArraysComponentHeader *) pc;

    /* compute stride (same for all component arrays) */
    for (i=0; i<numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;
	GLenum component = compHeader[i].component;

	__GLX_SWAP_INT(&datatype);
	__GLX_SWAP_INT(&numVals);
	__GLX_SWAP_INT(&component);

        stride += __GLX_PAD(numVals * __glXTypeSize(datatype));
    }

    pc += numComponents * sizeof(__GLXdispatchDrawArraysComponentHeader);

    /* set up component arrays */
    for (i=0; i<numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;
	GLenum component = compHeader[i].component;

	__GLX_SWAP_INT(&datatype);
	__GLX_SWAP_INT(&numVals);
	__GLX_SWAP_INT(&component);

	swapArray(numVals, datatype, stride, numVertexes, pc);

        switch (component) {
          case GL_VERTEX_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_VERTEX_ARRAY) );
	    CALL_VertexPointer( GET_DISPATCH(), (numVals, datatype, stride, pc) );
            break;
          case GL_NORMAL_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_NORMAL_ARRAY) );
	    CALL_NormalPointer( GET_DISPATCH(), (datatype, stride, pc) );
            break;
          case GL_COLOR_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_COLOR_ARRAY) );
	    CALL_ColorPointer( GET_DISPATCH(), (numVals, datatype, stride, pc) );
            break;
          case GL_INDEX_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_INDEX_ARRAY) );
	    CALL_IndexPointer( GET_DISPATCH(), (datatype, stride, pc) );
            break;
          case GL_TEXTURE_COORD_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_TEXTURE_COORD_ARRAY) );
	    CALL_TexCoordPointer( GET_DISPATCH(), (numVals, datatype, stride, pc) );
            break;
          case GL_EDGE_FLAG_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_EDGE_FLAG_ARRAY) );
	    CALL_EdgeFlagPointer( GET_DISPATCH(), (stride, (const GLboolean *)pc) );
            break;
          case GL_SECONDARY_COLOR_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_SECONDARY_COLOR_ARRAY) );
	    CALL_SecondaryColorPointerEXT( GET_DISPATCH(), (numVals, datatype, stride, pc) );
            break;
          case GL_FOG_COORD_ARRAY:
	    CALL_EnableClientState( GET_DISPATCH(), (GL_FOG_COORD_ARRAY) );
	    CALL_FogCoordPointerEXT( GET_DISPATCH(), (datatype, stride, pc) );
            break;
          default:
            break;
	}

        pc += __GLX_PAD(numVals * __glXTypeSize(datatype));
    }

    CALL_DrawArrays( GET_DISPATCH(), (primType, 0, numVertexes) );

    /* turn off anything we might have turned on */
    CALL_DisableClientState( GET_DISPATCH(), (GL_VERTEX_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_NORMAL_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_COLOR_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_INDEX_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_TEXTURE_COORD_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_EDGE_FLAG_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_SECONDARY_COLOR_ARRAY) );
    CALL_DisableClientState( GET_DISPATCH(), (GL_FOG_COORD_ARRAY) );
}

void __glXDispSwap_DrawArraysEXT(GLbyte *pc)
{
   __glXDispSwap_DrawArrays(pc);
}
