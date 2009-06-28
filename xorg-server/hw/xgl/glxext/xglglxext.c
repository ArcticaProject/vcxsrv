/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"
#include "xglglx.h"
#include "xglglxext.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/internal/glcore.h>

#include "glxserver.h"
#include "glxdrawable.h"
#include "glxscreens.h"
#include "glxutil.h"
#include "unpack.h"
#include "g_disptab.h"
#include "glapitable.h"
#include "glxext.h"
#include "micmap.h"

#define XGL_MAX_TEXTURE_UNITS      8
#define XGL_MAX_ATTRIB_STACK_DEPTH 16

#define XGL_TEXTURE_1D_BIT	  (1 << 0)
#define XGL_TEXTURE_2D_BIT	  (1 << 1)
#define XGL_TEXTURE_3D_BIT	  (1 << 2)
#define XGL_TEXTURE_RECTANGLE_BIT (1 << 3)
#define XGL_TEXTURE_CUBE_MAP_BIT  (1 << 4)

typedef Bool	      (*GLXScreenProbeProc)    (int screen);
typedef __GLinterface *(*GLXCreateContextProc) (__GLimports      *imports,
						__GLcontextModes *modes,
						__GLinterface    *shareGC);
typedef void	      (*GLXCreateBufferProc)   (__GLXdrawablePrivate *glxPriv);
typedef GLboolean     (*GLXSwapBuffersProc)    (__GLXdrawablePrivate *glxPriv);
typedef int	      (*GLXBindBuffersProc)    (__GLXdrawablePrivate *glxPriv,
						int		     buffer);
typedef int	      (*GLXReleaseBuffersProc) (__GLXdrawablePrivate *glxPriv,
						int		     buffer);

typedef struct _xglGLXScreenInfo {
    GLXScreenProbeProc   screenProbe;
    GLXCreateContextProc createContext;
    GLXCreateBufferProc  createBuffer;
} xglGLXScreenInfoRec, *xglGLXScreenInfoPtr;

extern __GLXscreenInfo *__xglScreenInfoPtr;

static xglGLXScreenInfoRec screenInfoPriv;

//extern __GLXscreenInfo __glDDXScreenInfo;

typedef GLboolean (*GLResizeBuffersProc) (__GLdrawableBuffer   *buffer,
					  GLint		       x,
					  GLint		       y,
					  GLuint	       width,
					  GLuint	       height,
					  __GLdrawablePrivate  *glPriv,
					  GLuint	       bufferMask);
typedef void	  (*GLFreeBuffersProc)   (__GLdrawablePrivate  *glPriv);

typedef struct _xglGLBuffer {
    GLXSwapBuffersProc    swapBuffers;
    GLXBindBuffersProc    bindBuffers;
    GLXReleaseBuffersProc releaseBuffers;
    GLResizeBuffersProc   resizeBuffers;
    GLFreeBuffersProc     freeBuffers;
    ScreenPtr		  pScreen;
    DrawablePtr		  pDrawable;
    xglVisualPtr	  pVisual;
    glitz_drawable_t	  *drawable;
    glitz_surface_t	  *backSurface;
    PixmapPtr		  pPixmap;
    GCPtr		  pGC;
    RegionRec		  damage;
    void	          *private;
    int			  screenX, screenY;
    int			  xOff, yOff;
    int			  yFlip;
} xglGLBufferRec, *xglGLBufferPtr;

typedef int xglGLXVisualConfigRec, *xglGLXVisualConfigPtr;
typedef struct _xglDisplayList *xglDisplayListPtr;

#define XGL_LIST_OP_CALLS 0
#define XGL_LIST_OP_DRAW  1
#define XGL_LIST_OP_GL    2
#define XGL_LIST_OP_LIST  3

typedef struct _xglGLOp {
    void (*glProc) (struct _xglGLOp *pOp);
    union {
	GLenum     enumeration;
	GLbitfield bitfield;
	GLsizei    size;
	struct {
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	} rect;
	struct {
	    GLenum target;
	    GLuint texture;
	} bind_texture;
	struct {
	    GLenum  target;
	    GLenum  pname;
	    GLfloat params[4];
	} tex_parameter_fv;
	struct {
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	    GLenum  type;
	} copy_pixels;
	struct {
	    GLenum  target;
	    GLint   level;
	    GLenum  internalformat;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLint   border;
	} copy_tex_image_1d;
	struct {
	    GLenum  target;
	    GLint   level;
	    GLenum  internalformat;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	    GLint   border;
	} copy_tex_image_2d;
	struct {
	    GLenum  target;
	    GLint   level;
	    GLint   xoffset;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	} copy_tex_sub_image_1d;
	struct {
	    GLenum  target;
	    GLint   level;
	    GLint   xoffset;
	    GLint   yoffset;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	} copy_tex_sub_image_2d;
	struct {
	    GLenum  target;
	    GLenum  internalformat;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	} copy_color_table;
	struct {
	    GLenum  target;
	    GLsizei start;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	} copy_color_sub_table;
	struct {
	    GLenum  target;
	    GLenum  internalformat;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	} copy_convolution_filter_1d;
	struct {
	    GLenum  target;
	    GLenum  internalformat;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	} copy_convolution_filter_2d;
	struct {
	    GLenum  target;
	    GLint   level;
	    GLint   xoffset;
	    GLint   yoffset;
	    GLint   zoffset;
	    GLint   x;
	    GLint   y;
	    GLsizei width;
	    GLsizei height;
	} copy_tex_sub_image_3d;
	struct {
	    GLfloat x;
	    GLfloat y;
	    GLfloat z;
	} window_pos_3f;
    } u;
} xglGLOpRec, *xglGLOpPtr;

typedef struct _xglListOp {
    int type;
    union {
	GLuint	   list;
	xglGLOpPtr gl;
    } u;
} xglListOpRec, *xglListOpPtr;

typedef struct _xglDisplayList {
    xglListOpPtr pOp;
    int		 nOp;
    int		 size;
} xglDisplayListRec;

typedef struct _xglTexObj {
    GLuint		   key;
    GLuint		   name;
    PixmapPtr		   pPixmap;
    glitz_texture_object_t *object;
    int			   refcnt;
} xglTexObjRec, *xglTexObjPtr;

typedef struct _xglTexUnit {
    GLbitfield   enabled;
    xglTexObjPtr p1D;
    xglTexObjPtr p2D;
    xglTexObjPtr p3D;
    xglTexObjPtr pRect;
    xglTexObjPtr pCubeMap;
} xglTexUnitRec, *xglTexUnitPtr;

typedef struct _xglGLAttributes {
    GLbitfield	  mask;
    GLenum	  drawBuffer;
    GLenum	  readBuffer;
    xRectangle	  viewport;
    xRectangle	  scissor;
    GLboolean	  scissorTest;
    xglTexUnitRec texUnits[XGL_MAX_TEXTURE_UNITS];
} xglGLAttributesRec, *xglGLAttributesPtr;

typedef struct _xglGLContext {
    __GLinterface	      iface;
    __GLinterface	      *mIface;
    int			      refcnt;
    struct _xglGLContext      *shared;
    glitz_context_t	      *context;
    struct _glapi_table	      glRenderTable;
    PFNGLACTIVETEXTUREARBPROC ActiveTextureARB;
    PFNGLWINDOWPOS3FMESAPROC  WindowPos3fMESA;
    Bool		      needInit;
    xglGLBufferPtr	      pDrawBuffer;
    xglGLBufferPtr	      pReadBuffer;
    int			      drawXoff, drawYoff;
    __GLdrawablePrivate	      *readPriv;
    __GLdrawablePrivate	      *drawPriv;
    char		      *versionString;
    GLenum		      errorValue;
    GLboolean		      doubleBuffer;
    GLint		      depthBits;
    GLint		      stencilBits;
    xglHashTablePtr	      texObjects;
    xglHashTablePtr	      displayLists;
    GLuint		      list;
    GLenum		      listMode;
    GLuint		      beginCnt;
    xglDisplayListPtr	      pList;
    GLuint		      groupList;
    xglGLAttributesRec	      attrib;
    xglGLAttributesRec	      attribStack[XGL_MAX_ATTRIB_STACK_DEPTH];
    int			      nAttribStack;
    int			      activeTexUnit;
    GLint		      maxTexUnits;
    GLint		      maxListNesting;
    GLint		      maxAttribStackDepth;
} xglGLContextRec, *xglGLContextPtr;

static xglGLContextPtr cctx = NULL;

static void
xglSetCurrentContext (xglGLContextPtr pContext);

#define XGL_GLX_DRAW_PROLOGUE_WITHOUT_TEXTURES(pBox, nBox, pScissorBox)	  \
    (pBox) = REGION_RECTS (cctx->pDrawBuffer->pGC->pCompositeClip);	  \
    (nBox) = REGION_NUM_RECTS (cctx->pDrawBuffer->pGC->pCompositeClip);	  \
    (pScissorBox)->x1 = cctx->attrib.scissor.x + cctx->pDrawBuffer->xOff; \
    (pScissorBox)->x2 = (pScissorBox)->x1 + cctx->attrib.scissor.width;	  \
    (pScissorBox)->y2 = cctx->attrib.scissor.y + cctx->pDrawBuffer->yOff; \
    (pScissorBox)->y2 = cctx->pDrawBuffer->yFlip - (pScissorBox)->y2;	  \
    (pScissorBox)->y1 = (pScissorBox)->y2 - cctx->attrib.scissor.height

#define XGL_GLX_DRAW_PROLOGUE(pBox, nBox, pScissorBox)		      \
    XGL_GLX_DRAW_PROLOGUE_WITHOUT_TEXTURES (pBox, nBox, pScissorBox); \
    xglSetupTextures ()

#define XGL_GLX_DRAW_BOX(pBox1, pBox2)			    \
    (pBox1)->x1 = cctx->pDrawBuffer->screenX + (pBox2)->x1; \
    (pBox1)->y1 = cctx->pDrawBuffer->screenY + (pBox2)->y1; \
    (pBox1)->x2 = cctx->pDrawBuffer->screenX + (pBox2)->x2; \
    (pBox1)->y2 = cctx->pDrawBuffer->screenY + (pBox2)->y2

#define XGL_GLX_INTERSECT_BOX(pBox1, pBox2) \
    {					    \
	if ((pBox1)->x1 < (pBox2)->x1)	    \
	    (pBox1)->x1 = (pBox2)->x1;	    \
	if ((pBox1)->y1 < (pBox2)->y1)	    \
	    (pBox1)->y1 = (pBox2)->y1;	    \
	if ((pBox1)->x2 > (pBox2)->x2)	    \
	    (pBox1)->x2 = (pBox2)->x2;	    \
	if ((pBox1)->y2 > (pBox2)->y2)	    \
	    (pBox1)->y2 = (pBox2)->y2;	    \
    }

#define XGL_GLX_SET_SCISSOR_BOX(pBox)		      \
    glScissor ((pBox)->x1,			      \
	       cctx->pDrawBuffer->yFlip - (pBox)->y2, \
	       (pBox)->x2 - (pBox)->x1,		      \
	       (pBox)->y2 - (pBox)->y1)

#define XGL_GLX_DRAW_DAMAGE(pBox, pRegion)				 \
    if (cctx->attrib.drawBuffer != GL_BACK)				 \
    {									 \
	(pRegion)->extents.x1 = (pBox)->x1 - cctx->pDrawBuffer->screenX; \
	(pRegion)->extents.y1 = (pBox)->y1 - cctx->pDrawBuffer->screenY; \
	(pRegion)->extents.x2 = (pBox)->x2 - cctx->pDrawBuffer->screenX; \
	(pRegion)->extents.y2 = (pBox)->y2 - cctx->pDrawBuffer->screenY; \
	(pRegion)->data = (RegDataPtr) NULL;				 \
	REGION_UNION (cctx->pDrawBuffer->pGC->pScreen,			 \
		      &cctx->pDrawBuffer->damage,			 \
		      &cctx->pDrawBuffer->damage,			 \
		      pRegion);						 \
	xglAddBitDamage (cctx->pDrawBuffer->pDrawable, pRegion);	 \
    }

static void
xglRecordError (GLenum error)
{
    if (cctx->errorValue == GL_NO_ERROR)
	cctx->errorValue = error;
}

static xglDisplayListPtr
xglCreateList (void)
{
    xglDisplayListPtr pDisplayList;

    pDisplayList = xalloc (sizeof (xglDisplayListRec));
    if (!pDisplayList)
	return NULL;

    pDisplayList->pOp  = NULL;
    pDisplayList->nOp  = 0;
    pDisplayList->size = 0;

    return pDisplayList;
}

static void
xglDestroyList (xglDisplayListPtr pDisplayList)
{
    xglListOpPtr pOp = pDisplayList->pOp;
    int		 nOp = pDisplayList->nOp;

    while (nOp--)
    {
	switch (pOp->type) {
	case XGL_LIST_OP_CALLS:
	case XGL_LIST_OP_DRAW:
	    glDeleteLists (pOp->u.list, 1);
	    break;
	case XGL_LIST_OP_GL:
	    xfree (pOp->u.gl);
	    break;
	case XGL_LIST_OP_LIST:
	    break;
	}

	pOp++;
    }

    if (pDisplayList->pOp)
	xfree (pDisplayList->pOp);

    xfree (pDisplayList);
}

static Bool
xglResizeList (xglDisplayListPtr pDisplayList,
	       int		 nOp)
{
    if (pDisplayList->size < nOp)
    {
	int size = pDisplayList->nOp ? pDisplayList->nOp : 4;

	while (size < nOp)
	    size <<= 1;

	pDisplayList->pOp = xrealloc (pDisplayList->pOp,
				      sizeof (xglListOpRec) * size);
	if (!pDisplayList->pOp)
	    return FALSE;

	pDisplayList->size = size;
    }

    return TRUE;
}

static void
xglStartList (int    type,
	      GLenum mode)
{
    if (!xglResizeList (cctx->pList, cctx->pList->nOp + 1))
    {
	xglRecordError (GL_OUT_OF_MEMORY);
	return;
    }

    cctx->pList->pOp[cctx->pList->nOp].type   = type;
    cctx->pList->pOp[cctx->pList->nOp].u.list = glGenLists (1);

    glNewList (cctx->pList->pOp[cctx->pList->nOp].u.list, mode);

    cctx->pList->nOp++;
}

static void
xglGLOp (xglGLOpPtr pOp)
{
    if (cctx->list)
    {
	xglGLOpPtr pGLOp;

	pGLOp = xalloc (sizeof (xglGLOpRec));
	if (!pGLOp)
	{
	    xglRecordError (GL_OUT_OF_MEMORY);
	    return;
	}

	if (!xglResizeList (cctx->pList, cctx->pList->nOp + 1))
	{
	    xfree (pGLOp);
	    xglRecordError (GL_OUT_OF_MEMORY);
	    return;
	}

	glEndList ();

	*pGLOp = *pOp;

	cctx->pList->pOp[cctx->pList->nOp].type = XGL_LIST_OP_GL;
	cctx->pList->pOp[cctx->pList->nOp].u.gl = pGLOp;
	cctx->pList->nOp++;

	if (cctx->listMode == GL_COMPILE_AND_EXECUTE)
	    (*pOp->glProc) (pOp);

	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
    }
    else
	(*pOp->glProc) (pOp);
}

static void
xglViewportProc (xglGLOpPtr pOp)
{
    cctx->attrib.viewport.x	 = pOp->u.rect.x;
    cctx->attrib.viewport.y	 = pOp->u.rect.y;
    cctx->attrib.viewport.width  = pOp->u.rect.width;
    cctx->attrib.viewport.height = pOp->u.rect.height;

    glViewport (pOp->u.rect.x + cctx->pDrawBuffer->xOff,
		pOp->u.rect.y + cctx->pDrawBuffer->yOff,
		pOp->u.rect.width,
		pOp->u.rect.height);
}

static void
xglViewport (GLint   x,
	     GLint   y,
	     GLsizei width,
	     GLsizei height)
{
    xglGLOpRec gl;

    gl.glProc = xglViewportProc;

    gl.u.rect.x	     = x;
    gl.u.rect.y	     = y;
    gl.u.rect.width  = width;
    gl.u.rect.height = height;

    xglGLOp (&gl);
}

static void
xglScissorProc (xglGLOpPtr pOp)
{
    cctx->attrib.scissor.x	= pOp->u.rect.x;
    cctx->attrib.scissor.y	= pOp->u.rect.y;
    cctx->attrib.scissor.width  = pOp->u.rect.width;
    cctx->attrib.scissor.height = pOp->u.rect.height;
}

static void
xglScissor (GLint   x,
	    GLint   y,
	    GLsizei width,
	    GLsizei height)
{
    xglGLOpRec gl;

    gl.glProc = xglScissorProc;

    gl.u.rect.x	     = x;
    gl.u.rect.y	     = y;
    gl.u.rect.width  = width;
    gl.u.rect.height = height;

    xglGLOp (&gl);
}

static void
xglDrawBufferProc (xglGLOpPtr pOp)
{
    glitz_drawable_buffer_t buffers[2];

    switch (pOp->u.enumeration) {
    case GL_FRONT:
	buffers[0] = GLITZ_DRAWABLE_BUFFER_FRONT_COLOR;
	glitz_context_draw_buffers (cctx->context, buffers, 1);
	break;
    case GL_FRONT_AND_BACK:
	buffers[0] = GLITZ_DRAWABLE_BUFFER_FRONT_COLOR;
	if (cctx->doubleBuffer)
	{
	    buffers[1] = GLITZ_DRAWABLE_BUFFER_BACK_COLOR;
	    glitz_context_draw_buffers (cctx->context, buffers, 2);
	}
	else
	    glitz_context_draw_buffers (cctx->context, buffers, 1);
	break;
    case GL_BACK:
	if (!cctx->doubleBuffer)
	{
	    xglRecordError (GL_INVALID_OPERATION);
	    return;
	}
	buffers[0] = GLITZ_DRAWABLE_BUFFER_BACK_COLOR;
	glitz_context_draw_buffers (cctx->context, buffers, 1);
	break;
    default:
	xglRecordError (GL_INVALID_ENUM);
	return;
    }

    cctx->attrib.drawBuffer = pOp->u.enumeration;
}

static void
xglDrawBuffer (GLenum mode)
{
    xglGLOpRec gl;

    gl.glProc = xglDrawBufferProc;

    gl.u.enumeration = mode;

    xglGLOp (&gl);
}

static void
xglReadBufferProc (xglGLOpPtr pOp)
{
    switch (pOp->u.enumeration) {
    case GL_FRONT:
	glitz_context_read_buffer (cctx->context,
				   GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
	break;
    case GL_BACK:
	if (!cctx->doubleBuffer)
	{
	    xglRecordError (GL_INVALID_OPERATION);
	    return;
	}
	glitz_context_read_buffer (cctx->context,
				   GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
	break;
    default:
	xglRecordError (GL_INVALID_ENUM);
	return;
    }

    cctx->attrib.readBuffer = pOp->u.enumeration;
}

static void
xglReadBuffer (GLenum mode)
{
    xglGLOpRec gl;

    gl.glProc = xglReadBufferProc;

    gl.u.enumeration = mode;

    xglGLOp (&gl);
}

static void
xglDisableProc (xglGLOpPtr pOp)
{
    xglTexUnitPtr pTexUnit = &cctx->attrib.texUnits[cctx->activeTexUnit];

    switch (pOp->u.enumeration) {
    case GL_SCISSOR_TEST:
	cctx->attrib.scissorTest = GL_FALSE;
	return;
    case GL_TEXTURE_1D:
	pTexUnit->enabled &= ~XGL_TEXTURE_1D_BIT;
	break;
    case GL_TEXTURE_2D:
	pTexUnit->enabled &= ~XGL_TEXTURE_2D_BIT;
	break;
    case GL_TEXTURE_3D:
	pTexUnit->enabled &= ~XGL_TEXTURE_3D_BIT;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	pTexUnit->enabled &= ~XGL_TEXTURE_RECTANGLE_BIT;
	break;
    case GL_TEXTURE_CUBE_MAP_ARB:
	pTexUnit->enabled &= ~XGL_TEXTURE_CUBE_MAP_BIT;
	break;
    default:
	break;
    }

    glDisable (pOp->u.enumeration);
}

static void
xglDisable (GLenum cap)
{
    xglGLOpRec gl;

    gl.glProc = xglDisableProc;

    gl.u.enumeration = cap;

    xglGLOp (&gl);
}

static void
xglEnableProc (xglGLOpPtr pOp)
{
    xglTexUnitPtr pTexUnit = &cctx->attrib.texUnits[cctx->activeTexUnit];

    switch (pOp->u.enumeration) {
    case GL_SCISSOR_TEST:
	cctx->attrib.scissorTest = GL_TRUE;
	return;
    case GL_DEPTH_TEST:
	if (!cctx->depthBits)
	    return;
    case GL_STENCIL_TEST:
	if (!cctx->stencilBits)
	    return;
    case GL_TEXTURE_1D:
	pTexUnit->enabled |= XGL_TEXTURE_1D_BIT;
	break;
    case GL_TEXTURE_2D:
	pTexUnit->enabled |= XGL_TEXTURE_2D_BIT;
	break;
    case GL_TEXTURE_3D:
	pTexUnit->enabled |= XGL_TEXTURE_3D_BIT;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	pTexUnit->enabled |= XGL_TEXTURE_RECTANGLE_BIT;
	break;
    case GL_TEXTURE_CUBE_MAP_ARB:
	pTexUnit->enabled |= XGL_TEXTURE_CUBE_MAP_BIT;
	break;
    default:
	break;
    }

    glEnable (pOp->u.enumeration);
}

static void
xglEnable (GLenum cap)
{
    xglGLOpRec gl;

    gl.glProc = xglEnableProc;

    gl.u.enumeration = cap;

    xglGLOp (&gl);
}

static void
xglDeleteTexObj (xglTexObjPtr pTexObj)
{
    if (pTexObj->pPixmap)
    {
	ScreenPtr pScreen = pTexObj->pPixmap->drawable.pScreen;

	(*pScreen->DestroyPixmap) (pTexObj->pPixmap);

	glitz_texture_object_destroy (pTexObj->object);
    }

    if (pTexObj->name)
    {
	glDeleteTextures (1, &pTexObj->name);
    }

    pTexObj->key     = 0;
    pTexObj->name    = 0;
    pTexObj->pPixmap = NULL;
    pTexObj->object  = NULL;
}

static void
xglRefTexObj (xglTexObjPtr pTexObj)
{
    if (!pTexObj)
	return;

    pTexObj->refcnt++;
}

static void
xglUnrefTexObj (xglTexObjPtr pTexObj)
{
    if (!pTexObj)
	return;

    pTexObj->refcnt--;
    if (pTexObj->refcnt)
	return;

    xglDeleteTexObj (pTexObj);

    xfree (pTexObj);
}

static void
xglPushAttribProc (xglGLOpPtr pOp)
{
    xglGLAttributesPtr pAttrib;

    if (cctx->nAttribStack == cctx->maxAttribStackDepth)
    {
	xglRecordError (GL_STACK_OVERFLOW);
	return;
    }

    pAttrib = &cctx->attribStack[cctx->nAttribStack];

    *pAttrib = cctx->attrib;
    pAttrib->mask = pOp->u.bitfield;

    if (pOp->u.bitfield & GL_TEXTURE_BIT)
    {
	int i;

	for (i = 0; i < cctx->maxTexUnits; i++)
	{
	    xglRefTexObj (pAttrib->texUnits[i].p1D);
	    xglRefTexObj (pAttrib->texUnits[i].p2D);
	    xglRefTexObj (pAttrib->texUnits[i].p3D);
	    xglRefTexObj (pAttrib->texUnits[i].pRect);
	    xglRefTexObj (pAttrib->texUnits[i].pCubeMap);
	}
    }

    cctx->nAttribStack++;

    glPushAttrib (pOp->u.bitfield);
}

static void
xglPushAttrib (GLbitfield mask)
{
    xglGLOpRec gl;

    gl.glProc = xglPushAttribProc;

    gl.u.bitfield = mask;

    xglGLOp (&gl);
}

static void
xglPopAttribProc (xglGLOpPtr pOp)
{
    xglGLAttributesPtr pAttrib;
    GLbitfield	       mask;

    if (!cctx->nAttribStack)
    {
	xglRecordError (GL_STACK_UNDERFLOW);
	return;
    }

    cctx->nAttribStack--;

    pAttrib = &cctx->attribStack[cctx->nAttribStack];
    mask = pAttrib->mask;

    if (mask & GL_COLOR_BUFFER_BIT)
	xglDrawBuffer (pAttrib->drawBuffer);

    if (mask & GL_PIXEL_MODE_BIT)
	xglReadBuffer (pAttrib->readBuffer);

    if (mask & GL_SCISSOR_BIT)
    {
	xglScissor (pAttrib->scissor.x,
		    pAttrib->scissor.y,
		    pAttrib->scissor.width,
		    pAttrib->scissor.height);

	if (pAttrib->scissorTest)
	    xglEnable (GL_SCISSOR_TEST);
	else
	    xglDisable (GL_SCISSOR_TEST);
    }
    else if (mask & GL_ENABLE_BIT)
    {
	if (pAttrib->scissorTest)
	    xglEnable (GL_SCISSOR_TEST);
	else
	    xglDisable (GL_SCISSOR_TEST);
    }

    if (mask & GL_VIEWPORT_BIT)
	xglViewport (pAttrib->viewport.x,
		     pAttrib->viewport.y,
		     pAttrib->viewport.width,
		     pAttrib->viewport.height);

    if (mask & GL_TEXTURE_BIT)
    {
	int i;

	for (i = 0; i < cctx->maxTexUnits; i++)
	{
	    xglUnrefTexObj (cctx->attrib.texUnits[i].p1D);
	    xglUnrefTexObj (cctx->attrib.texUnits[i].p2D);
	    xglUnrefTexObj (cctx->attrib.texUnits[i].p3D);
	    xglUnrefTexObj (cctx->attrib.texUnits[i].pRect);
	    xglUnrefTexObj (cctx->attrib.texUnits[i].pCubeMap);

	    cctx->attrib.texUnits[i] = pAttrib->texUnits[i];
	}
    }
    else if (mask & GL_ENABLE_BIT)
    {
	int i;

	for (i = 0; i < cctx->maxTexUnits; i++)
	    cctx->attrib.texUnits[i].enabled = pAttrib->texUnits[i].enabled;
    }

    glPopAttrib ();
}

static void
xglPopAttrib (void)
{
    xglGLOpRec gl;

    gl.glProc = xglPopAttribProc;

    xglGLOp (&gl);
}

static GLuint
xglActiveTextureBinding (GLenum target)
{
    xglTexObjPtr pTexObj;

    switch (target) {
    case GL_TEXTURE_BINDING_1D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p1D;
	break;
    case GL_TEXTURE_BINDING_2D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	break;
    case GL_TEXTURE_BINDING_3D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p3D;
	break;
    case GL_TEXTURE_BINDING_RECTANGLE_NV:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
	break;
    case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pCubeMap;
	break;
    default:
	return 0;
    }

    if (pTexObj)
	return pTexObj->key;

    return 0;
}

#define DOUBLE_TO_BOOLEAN(X) ((X) ? GL_TRUE : GL_FALSE)
#define INT_TO_BOOLEAN(I)    ((I) ? GL_TRUE : GL_FALSE)
#define ENUM_TO_BOOLEAN(E)   ((E) ? GL_TRUE : GL_FALSE)

static void
xglGetBooleanv (GLenum	  pname,
		GLboolean *params)
{
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
    {
	GLdouble v[4];

	glGetDoublev (GL_CURRENT_RASTER_POSITION, v);

	params[0] = DOUBLE_TO_BOOLEAN (v[0] - (GLdouble) cctx->drawXoff);
	params[1] = DOUBLE_TO_BOOLEAN (v[1] - (GLdouble) cctx->drawYoff);
	params[2] = DOUBLE_TO_BOOLEAN (v[2]);
	params[3] = DOUBLE_TO_BOOLEAN (v[3]);
    } break;
    case GL_DOUBLEBUFFER:
	params[0] = cctx->doubleBuffer;
	break;
    case GL_DEPTH_BITS:
	params[0] = INT_TO_BOOLEAN (cctx->depthBits);
	break;
    case GL_STENCIL_BITS:
	params[0] = INT_TO_BOOLEAN (cctx->stencilBits);
	break;
    case GL_DRAW_BUFFER:
	params[0] = ENUM_TO_BOOLEAN (cctx->attrib.drawBuffer);
	break;
    case GL_READ_BUFFER:
	params[0] = ENUM_TO_BOOLEAN (cctx->attrib.readBuffer);
	break;
    case GL_SCISSOR_BOX:
	params[0] = INT_TO_BOOLEAN (cctx->attrib.scissor.x);
	params[1] = INT_TO_BOOLEAN (cctx->attrib.scissor.y);
	params[2] = INT_TO_BOOLEAN (cctx->attrib.scissor.width);
	params[3] = INT_TO_BOOLEAN (cctx->attrib.scissor.height);
	break;
    case GL_SCISSOR_TEST:
	params[0] = cctx->attrib.scissorTest;
	break;
    case GL_VIEWPORT:
	params[0] = INT_TO_BOOLEAN (cctx->attrib.viewport.x);
	params[1] = INT_TO_BOOLEAN (cctx->attrib.viewport.y);
	params[2] = INT_TO_BOOLEAN (cctx->attrib.viewport.width);
	params[3] = INT_TO_BOOLEAN (cctx->attrib.viewport.height);
	break;
    case GL_TEXTURE_BINDING_1D:
    case GL_TEXTURE_BINDING_2D:
    case GL_TEXTURE_BINDING_3D:
    case GL_TEXTURE_BINDING_RECTANGLE_NV:
    case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
	/* should be safe to fall-through here */
    default:
	glGetBooleanv (pname, params);
    }
}

#define INT_TO_DOUBLE(I)     ((GLdouble) (I))
#define ENUM_TO_DOUBLE(E)    ((GLdouble) (E))
#define BOOLEAN_TO_DOUBLE(B) ((B) ? 1.0F : 0.0F)

static void
xglGetDoublev (GLenum	pname,
	       GLdouble *params)
{
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
	glGetDoublev (GL_CURRENT_RASTER_POSITION, params);

	params[0] -= (GLdouble) cctx->drawXoff;
	params[1] -= (GLdouble) cctx->drawYoff;
	break;
    case GL_DOUBLEBUFFER:
	params[0] = BOOLEAN_TO_DOUBLE (cctx->doubleBuffer);
	break;
    case GL_DEPTH_BITS:
	params[0] = INT_TO_DOUBLE (cctx->depthBits);
	break;
    case GL_STENCIL_BITS:
	params[0] = INT_TO_DOUBLE (cctx->stencilBits);
	break;
    case GL_DRAW_BUFFER:
	params[0] = ENUM_TO_DOUBLE (cctx->attrib.drawBuffer);
	break;
    case GL_READ_BUFFER:
	params[0] = ENUM_TO_DOUBLE (cctx->attrib.readBuffer);
	break;
    case GL_SCISSOR_BOX:
	params[0] = cctx->attrib.scissor.x;
	params[1] = cctx->attrib.scissor.y;
	params[2] = cctx->attrib.scissor.width;
	params[3] = cctx->attrib.scissor.height;
	break;
    case GL_SCISSOR_TEST:
	params[0] = BOOLEAN_TO_DOUBLE (cctx->attrib.scissorTest);
	break;
    case GL_VIEWPORT:
	params[0] = cctx->attrib.viewport.x;
	params[1] = cctx->attrib.viewport.y;
	params[2] = cctx->attrib.viewport.width;
	params[3] = cctx->attrib.viewport.height;
	break;
    case GL_TEXTURE_BINDING_1D:
    case GL_TEXTURE_BINDING_2D:
    case GL_TEXTURE_BINDING_3D:
    case GL_TEXTURE_BINDING_RECTANGLE_NV:
    case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
	params[0] = xglActiveTextureBinding (pname);
	break;
    case GL_MAX_TEXTURE_UNITS_ARB:
	params[0] = cctx->maxTexUnits;
	break;
    case GL_MAX_ATTRIB_STACK_DEPTH:
	params[0] = cctx->maxAttribStackDepth;
	break;
    default:
	glGetDoublev (pname, params);
    }
}

#define INT_TO_FLOAT(I)     ((GLfloat) (I))
#define ENUM_TO_FLOAT(E)    ((GLfloat) (E))
#define BOOLEAN_TO_FLOAT(B) ((B) ? 1.0F : 0.0F)

static void
xglGetFloatv (GLenum  pname,
	      GLfloat *params)
{
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
	glGetFloatv (GL_CURRENT_RASTER_POSITION, params);

	params[0] -= (GLfloat) cctx->drawXoff;
	params[1] -= (GLfloat) cctx->drawYoff;
	break;
    case GL_DOUBLEBUFFER:
	params[0] = BOOLEAN_TO_FLOAT (cctx->doubleBuffer);
	break;
    case GL_DEPTH_BITS:
	params[0] = INT_TO_FLOAT (cctx->depthBits);
	break;
    case GL_STENCIL_BITS:
	params[0] = INT_TO_FLOAT (cctx->stencilBits);
	break;
    case GL_DRAW_BUFFER:
	params[0] = ENUM_TO_FLOAT (cctx->attrib.drawBuffer);
	break;
    case GL_READ_BUFFER:
	params[0] = ENUM_TO_FLOAT (cctx->attrib.readBuffer);
	break;
    case GL_SCISSOR_BOX:
	params[0] = cctx->attrib.scissor.x;
	params[1] = cctx->attrib.scissor.y;
	params[2] = cctx->attrib.scissor.width;
	params[3] = cctx->attrib.scissor.height;
	break;
    case GL_SCISSOR_TEST:
	params[0] = BOOLEAN_TO_FLOAT (cctx->attrib.scissorTest);
	break;
    case GL_VIEWPORT:
	params[0] = cctx->attrib.viewport.x;
	params[1] = cctx->attrib.viewport.y;
	params[2] = cctx->attrib.viewport.width;
	params[3] = cctx->attrib.viewport.height;
	break;
    case GL_TEXTURE_BINDING_1D:
    case GL_TEXTURE_BINDING_2D:
    case GL_TEXTURE_BINDING_3D:
    case GL_TEXTURE_BINDING_RECTANGLE_NV:
    case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
	params[0] = xglActiveTextureBinding (pname);
	break;
    case GL_MAX_TEXTURE_UNITS_ARB:
	params[0] = cctx->maxTexUnits;
	break;
    case GL_MAX_ATTRIB_STACK_DEPTH:
	params[0] = cctx->maxAttribStackDepth;
	break;
    default:
	glGetFloatv (pname, params);
    }
}

#define ENUM_TO_INT(E)    ((GLint) (E))
#define BOOLEAN_TO_INT(B) ((GLint) (B))

static void
xglGetIntegerv (GLenum pname,
		GLint  *params)
{
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
	glGetIntegerv (GL_CURRENT_RASTER_POSITION, params);

	params[0] -= (GLint) cctx->drawXoff;
	params[1] -= (GLint) cctx->drawYoff;
	break;
    case GL_DOUBLEBUFFER:
	params[0] = BOOLEAN_TO_INT (cctx->doubleBuffer);
	break;
    case GL_DEPTH_BITS:
	params[0] = cctx->depthBits;
	break;
    case GL_STENCIL_BITS:
	params[0] = cctx->stencilBits;
	break;
    case GL_DRAW_BUFFER:
	params[0] = ENUM_TO_INT (cctx->attrib.drawBuffer);
	break;
    case GL_READ_BUFFER:
	params[0] = ENUM_TO_INT (cctx->attrib.readBuffer);
	break;
    case GL_SCISSOR_BOX:
	params[0] = cctx->attrib.scissor.x;
	params[1] = cctx->attrib.scissor.y;
	params[2] = cctx->attrib.scissor.width;
	params[3] = cctx->attrib.scissor.height;
	break;
    case GL_SCISSOR_TEST:
	params[0] = BOOLEAN_TO_INT (cctx->attrib.scissorTest);
	break;
    case GL_VIEWPORT:
	params[0] = cctx->attrib.viewport.x;
	params[1] = cctx->attrib.viewport.y;
	params[2] = cctx->attrib.viewport.width;
	params[3] = cctx->attrib.viewport.height;
	break;
    case GL_TEXTURE_BINDING_1D:
    case GL_TEXTURE_BINDING_2D:
    case GL_TEXTURE_BINDING_3D:
    case GL_TEXTURE_BINDING_RECTANGLE_NV:
    case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
	params[0] = xglActiveTextureBinding (pname);
	break;
    case GL_MAX_TEXTURE_UNITS_ARB:
	params[0] = cctx->maxTexUnits;
	break;
    case GL_MAX_ATTRIB_STACK_DEPTH:
	params[0] = cctx->maxAttribStackDepth;
	break;
    default:
	glGetIntegerv (pname, params);
    }
}

static GLboolean
xglIsEnabled (GLenum cap)
{
    switch (cap) {
    case GL_SCISSOR_TEST:
	return cctx->attrib.scissorTest;
    default:
	return glIsEnabled (cap);
    }
}

static GLenum
xglGetError (void)
{
    GLenum error = cctx->errorValue;

    if (error != GL_NO_ERROR)
    {
	cctx->errorValue = GL_NO_ERROR;
	return error;
    }

    return glGetError ();
}

static const GLubyte *
xglGetString (GLenum name)
{
    switch (name) {
    case GL_VERSION:
	if (!cctx->versionString)
	{
	    static char *version = "1.2 (%s)";
	    char	*nativeVersion = (char *) glGetString (GL_VERSION);

	    cctx->versionString = xalloc (strlen (version) +
					  strlen (nativeVersion));
	    if (cctx->versionString)
		sprintf (cctx->versionString, version, nativeVersion);
	}
	return (GLubyte *) cctx->versionString;
    default:
	return glGetString (name);
    }
}

static void
xglGenTextures (GLsizei n,
		GLuint  *textures)
{
    xglTexObjPtr pTexObj;
    GLuint	 name;

    name = xglHashFindFreeKeyBlock (cctx->shared->texObjects, n);

    glGenTextures (n, textures);

    while (n--)
    {
	pTexObj = xalloc (sizeof (xglTexObjRec));
	if (pTexObj)
	{
	    pTexObj->key     = name;
	    pTexObj->name    = *textures;
	    pTexObj->pPixmap = NULL;
	    pTexObj->object  = NULL;
	    pTexObj->refcnt  = 1;

	    xglHashInsert (cctx->shared->texObjects, name, pTexObj);
	}
	else
	{
	    xglRecordError (GL_OUT_OF_MEMORY);
	}

	*textures++ = name++;
    }
}

static void
xglBindTextureProc (xglGLOpPtr pOp)
{
    xglTexObjPtr *ppTexObj;

    switch (pOp->u.bind_texture.target) {
    case GL_TEXTURE_1D:
	ppTexObj = &cctx->attrib.texUnits[cctx->activeTexUnit].p1D;
	break;
    case GL_TEXTURE_2D:
	ppTexObj = &cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	break;
    case GL_TEXTURE_3D:
	ppTexObj = &cctx->attrib.texUnits[cctx->activeTexUnit].p3D;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	ppTexObj = &cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
	break;
    case GL_TEXTURE_CUBE_MAP_ARB:
	ppTexObj = &cctx->attrib.texUnits[cctx->activeTexUnit].pCubeMap;
	break;
    default:
	xglRecordError (GL_INVALID_ENUM);
	return;
    }

    if (pOp->u.bind_texture.texture)
    {
	if (!*ppTexObj || pOp->u.bind_texture.texture != (*ppTexObj)->key)
	{
	    xglTexObjPtr pTexObj;

	    pTexObj = (xglTexObjPtr)
		xglHashLookup (cctx->shared->texObjects,
			       pOp->u.bind_texture.texture);
	    if (!pTexObj)
	    {
		pTexObj = xalloc (sizeof (xglTexObjRec));
		if (!pTexObj)
		{
		    xglRecordError (GL_OUT_OF_MEMORY);
		    return;
		}

		pTexObj->key     = pOp->u.bind_texture.texture;
		pTexObj->pPixmap = NULL;
		pTexObj->object  = NULL;
		pTexObj->refcnt  = 1;

		glGenTextures (1, &pTexObj->name);

		xglHashInsert (cctx->shared->texObjects,
			       pOp->u.bind_texture.texture,
			       pTexObj);
	    }

	    xglRefTexObj (pTexObj);
	    xglUnrefTexObj (*ppTexObj);
	    *ppTexObj = pTexObj;

	    glBindTexture (pOp->u.bind_texture.target, pTexObj->name);
	}
    }
    else
    {
	xglUnrefTexObj (*ppTexObj);
	*ppTexObj = NULL;

	glBindTexture (pOp->u.bind_texture.target, 0);
    }
}

static void
xglBindTexture (GLenum target,
		GLuint texture)
{
    xglGLOpRec gl;

    gl.glProc = xglBindTextureProc;

    gl.u.bind_texture.target  = target;
    gl.u.bind_texture.texture = texture;

    xglGLOp (&gl);
}

static void
xglSetupTextures (void)
{
    xglGLContextPtr pContext = cctx;
    xglTexUnitPtr   pTexUnit;
    xglTexObjPtr    pTexObj[XGL_MAX_TEXTURE_UNITS];
    int		    i, activeTexUnit;

    for (i = 0; i < pContext->maxTexUnits; i++)
    {
	pTexObj[i] = NULL;

	pTexUnit = &pContext->attrib.texUnits[i];
	if (pTexUnit->enabled)
	{
	    if (pTexUnit->enabled & XGL_TEXTURE_RECTANGLE_BIT)
		pTexObj[i] = pTexUnit->pRect;
	    else if (pTexUnit->enabled & XGL_TEXTURE_2D_BIT)
		pTexObj[i] = pTexUnit->p2D;

	    if (pTexObj[i] && pTexObj[i]->pPixmap)
	    {
		if (!xglSyncSurface (&pTexObj[i]->pPixmap->drawable))
		    pTexObj[i] = NULL;
	    }
	    else
		pTexObj[i] = NULL;
	}
    }

    if (pContext != cctx)
    {
	XGL_SCREEN_PRIV (pContext->pDrawBuffer->pGC->pScreen);

	glitz_drawable_finish (pScreenPriv->drawable);

	xglSetCurrentContext (pContext);
    }

    activeTexUnit = cctx->activeTexUnit;
    for (i = 0; i < pContext->maxTexUnits; i++)
    {
	if (pTexObj[i])
	{
	    if (i != activeTexUnit)
	    {
		cctx->ActiveTextureARB (GL_TEXTURE0_ARB + i);
		activeTexUnit = i;
	    }
	    glitz_context_bind_texture (cctx->context, pTexObj[i]->object);
	}
    }

    if (activeTexUnit != cctx->activeTexUnit)
	cctx->ActiveTextureARB (cctx->activeTexUnit);
}

static GLboolean
xglAreTexturesResident (GLsizei	     n,
			const GLuint *textures,
			GLboolean    *residences)
{
    GLboolean allResident = GL_TRUE;
    int	      i, j;

    if (n < 0)
    {
	xglRecordError (GL_INVALID_VALUE);
	return GL_FALSE;
    }

    if (!textures || !residences)
	return GL_FALSE;

    for (i = 0; i < n; i++)
    {
	xglTexObjPtr pTexObj;
	GLboolean    resident;

	if (!textures[i])
	{
	    xglRecordError (GL_INVALID_VALUE);
	    return GL_FALSE;
	}

	pTexObj = (xglTexObjPtr) xglHashLookup (cctx->shared->texObjects,
						textures[i]);
	if (!pTexObj)
	{
	    xglRecordError (GL_INVALID_VALUE);
	    return GL_FALSE;
	}

	if (pTexObj->name == 0 ||
	    glAreTexturesResident (1, &pTexObj->name, &resident))
	{
	    if (!allResident)
		residences[i] = GL_TRUE;
	}
	else
	{
	    if (allResident)
	    {
		allResident = GL_FALSE;

		for (j = 0; j < i; j++)
		    residences[j] = GL_TRUE;
	    }
	    residences[i] = GL_FALSE;
	}
    }

    return allResident;
}

static void
xglDeleteTextures (GLsizei	n,
		   const GLuint *textures)
{
    xglTexObjPtr pTexObj;

    while (n--)
    {
	if (!*textures)
	    continue;

	pTexObj = (xglTexObjPtr) xglHashLookup (cctx->shared->texObjects,
						*textures);
	if (pTexObj)
	{
	    xglDeleteTexObj (pTexObj);
	    xglUnrefTexObj (pTexObj);
	    xglHashRemove (cctx->shared->texObjects, *textures);
	}
	textures++;
    }
}

static GLboolean
xglIsTexture (GLuint texture)
{
    xglTexObjPtr pTexObj;

    if (!texture)
	return GL_FALSE;

    pTexObj = (xglTexObjPtr) xglHashLookup (cctx->shared->texObjects, texture);
    if (pTexObj)
	return GL_TRUE;

    return GL_FALSE;
}

static void
xglPrioritizeTextures (GLsizei	      n,
		       const GLuint   *textures,
		       const GLclampf *priorities)
{
    xglTexObjPtr pTexObj;
    int		 i;

    if (n < 0)
    {
	xglRecordError (GL_INVALID_VALUE);
	return;
    }

    if (!priorities)
	return;

    for (i = 0; i < n; i++)
    {
	if (!textures[i])
	    continue;

	pTexObj = (xglTexObjPtr) xglHashLookup (cctx->shared->texObjects,
						textures[i]);
	if (pTexObj && pTexObj->name)
	    glPrioritizeTextures (1, &pTexObj->name, &priorities[i]);
    }
}

static glitz_texture_filter_t
xglTextureFilter (GLenum param)
{
    switch (param) {
    case GL_LINEAR:
	return GLITZ_TEXTURE_FILTER_LINEAR;
    case GL_NEAREST:
    default:
	return GLITZ_TEXTURE_FILTER_NEAREST;
    }
}

static glitz_texture_wrap_t
xglTextureWrap (GLenum param)
{
    switch (param) {
    case GL_CLAMP_TO_EDGE:
	return GLITZ_TEXTURE_WRAP_CLAMP_TO_EDGE;
    case GL_CLAMP_TO_BORDER:
	return GLITZ_TEXTURE_WRAP_CLAMP_TO_BORDER;
    case GL_REPEAT:
	return GLITZ_TEXTURE_WRAP_REPEAT;
    case GL_MIRRORED_REPEAT:
	return GLITZ_TEXTURE_WRAP_MIRRORED_REPEAT;
    case GL_CLAMP:
    default:
	return GLITZ_TEXTURE_WRAP_CLAMP;
    }
}

static void
xglTexParameterfvProc (xglGLOpPtr pOp)
{
    xglTexObjPtr pTexObj;

    glTexParameterfv (pOp->u.tex_parameter_fv.target,
		      pOp->u.tex_parameter_fv.pname,
		      pOp->u.tex_parameter_fv.params);

    switch (pOp->u.tex_parameter_fv.target) {
    case GL_TEXTURE_2D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
	break;
    default:
	pTexObj = NULL;
	break;
    }

    if (pTexObj && pTexObj->pPixmap)
    {
	GLfloat *params = pOp->u.tex_parameter_fv.params;

	switch (pOp->u.tex_parameter_fv.pname) {
	case GL_TEXTURE_MIN_FILTER:
	    glitz_texture_object_set_filter (pTexObj->object,
					     GLITZ_TEXTURE_FILTER_TYPE_MIN,
					     xglTextureFilter (params[0]));
	    break;
	case GL_TEXTURE_MAG_FILTER:
	    glitz_texture_object_set_filter (pTexObj->object,
					     GLITZ_TEXTURE_FILTER_TYPE_MAG,
					     xglTextureFilter (params[0]));
	    break;
	case GL_TEXTURE_WRAP_S:
	    glitz_texture_object_set_wrap (pTexObj->object,
					   GLITZ_TEXTURE_WRAP_TYPE_S,
					   xglTextureWrap (params[0]));
	    break;
	case GL_TEXTURE_WRAP_T:
	    glitz_texture_object_set_wrap (pTexObj->object,
					   GLITZ_TEXTURE_WRAP_TYPE_T,
					   xglTextureWrap (params[0]));
	    break;
	case GL_TEXTURE_BORDER_COLOR: {
	    glitz_color_t color;

	    color.red   = params[0] * 0xffff;
	    color.green = params[1] * 0xffff;
	    color.blue  = params[2] * 0xffff;
	    color.alpha = params[3] * 0xffff;

	    glitz_texture_object_set_border_color (pTexObj->object, &color);
	}
	default:
	    break;
	}
    }
}

static void
xglTexParameterfv (GLenum	 target,
		   GLenum	 pname,
		   const GLfloat *params)
{
    xglGLOpRec gl;

    gl.glProc = xglTexParameterfvProc;

    gl.u.tex_parameter_fv.target = target;
    gl.u.tex_parameter_fv.pname  = pname;

    switch (pname) {
    case GL_TEXTURE_BORDER_COLOR:
	gl.u.tex_parameter_fv.params[3] = params[3];
	gl.u.tex_parameter_fv.params[2] = params[2];
	gl.u.tex_parameter_fv.params[1] = params[1];
	/* fall-through */
    default:
	gl.u.tex_parameter_fv.params[0] = params[0];
	break;
    }

    xglGLOp (&gl);
}

static void
xglTexParameteriv (GLenum      target,
		   GLenum      pname,
		   const GLint *params)
{
    xglGLOpRec gl;

    gl.glProc = xglTexParameterfvProc;

    gl.u.tex_parameter_fv.target = target;
    gl.u.tex_parameter_fv.pname  = pname;

    switch (pname) {
    case GL_TEXTURE_BORDER_COLOR:
	gl.u.tex_parameter_fv.params[3] = (GLfloat) params[3] / INT_MAX;
	gl.u.tex_parameter_fv.params[2] = (GLfloat) params[2] / INT_MAX;
	gl.u.tex_parameter_fv.params[1] = (GLfloat) params[1] / INT_MAX;
	gl.u.tex_parameter_fv.params[0] = (GLfloat) params[0] / INT_MAX;
	break;
    default:
	gl.u.tex_parameter_fv.params[0] = params[0];
	break;
    }

    xglGLOp (&gl);
}

static void
xglTexParameterf (GLenum  target,
		  GLenum  pname,
		  GLfloat param)
{
    xglTexParameterfv (target, pname, (const GLfloat *) &param);
}

static void
xglTexParameteri (GLenum target,
		  GLenum pname,
		  GLint  param)
{
    xglTexParameteriv (target, pname, (const GLint *) &param);
}

static void
xglGetTexLevelParameterfv (GLenum  target,
			   GLint   level,
			   GLenum  pname,
			   GLfloat *params)
{
    xglTexObjPtr pTexObj;

    switch (target) {
    case GL_TEXTURE_2D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
	break;
    default:
	pTexObj = NULL;
	break;
    }

    if (pTexObj && pTexObj->pPixmap)
    {
	glitz_context_bind_texture (cctx->context, pTexObj->object);

	glGetTexLevelParameterfv (target, level, pname, params);
	glBindTexture (target, pTexObj->name);
    }
    else
	glGetTexLevelParameterfv (target, level, pname, params);
}

static void
xglGetTexLevelParameteriv (GLenum target,
			   GLint  level,
			   GLenum pname,
			   GLint  *params)
{
    xglTexObjPtr pTexObj;

    switch (target) {
    case GL_TEXTURE_2D:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	break;
    case GL_TEXTURE_RECTANGLE_NV:
	pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
	break;
    default:
	pTexObj = NULL;
	break;
    }

    if (pTexObj && pTexObj->pPixmap)
    {
	glitz_context_bind_texture (cctx->context, pTexObj->object);

	glGetTexLevelParameteriv (target, level, pname, params);
	glBindTexture (target, pTexObj->name);
    }
    else
	glGetTexLevelParameteriv (target, level, pname, params);
}

static GLuint
xglGenLists (GLsizei range)
{
    xglDisplayListPtr pDisplayList;
    GLuint	      first, name;

    first = xglHashFindFreeKeyBlock (cctx->shared->displayLists, range);

    name = first;
    for (name = first; range--; name++)
    {
	pDisplayList = xglCreateList ();
	if (pDisplayList)
	{
	    xglHashInsert (cctx->shared->displayLists, name, pDisplayList);
	}
	else
	{
	    xglRecordError (GL_OUT_OF_MEMORY);
	}
    }

    return first;
}

static void
xglNewList (GLuint list,
	    GLenum mode)
{
    if (!list)
    {
	xglRecordError (GL_INVALID_VALUE);
	return;
    }

    if (cctx->list)
    {
	xglRecordError (GL_INVALID_OPERATION);
	return;
    }

    cctx->pList = xglCreateList ();
    if (!cctx->pList)
    {
	xglRecordError (GL_OUT_OF_MEMORY);
	return;
    }

    cctx->list     = list;
    cctx->listMode = mode;

    xglStartList (XGL_LIST_OP_CALLS, mode);
}

static void
xglEndList (void)
{
    xglDisplayListPtr pDisplayList;

    if (!cctx->list)
    {
	xglRecordError (GL_INVALID_OPERATION);
	return;
    }

    glEndList ();

    pDisplayList = (xglDisplayListPtr)
	xglHashLookup (cctx->shared->displayLists, cctx->list);
    if (pDisplayList)
    {
	xglHashRemove (cctx->shared->displayLists, cctx->list);
	xglDestroyList (pDisplayList);
    }

    xglHashInsert (cctx->shared->displayLists, cctx->list, cctx->pList);

    cctx->list = 0;
}

static void
xglDrawList (GLuint list)
{
    RegionRec region;
    BoxRec    scissor, box;
    BoxPtr    pBox;
    int	      nBox;

    XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

    while (nBox--)
    {
	XGL_GLX_DRAW_BOX (&box, pBox);

	pBox++;

	if (cctx->attrib.scissorTest)
	    XGL_GLX_INTERSECT_BOX (&box, &scissor);

	if (box.x1 < box.x2 && box.y1 < box.y2)
	{
	    XGL_GLX_SET_SCISSOR_BOX (&box);

	    glCallList (list);

	    XGL_GLX_DRAW_DAMAGE (&box, &region);
	}
    }
}

static void
xglCallDisplayList (GLuint list,
		    int	   nesting)
{
    if (nesting > cctx->maxListNesting)
	return;

    if (!list)
    {
	xglRecordError (GL_INVALID_VALUE);
	return;
    }

    if (cctx->list)
    {
	if (!xglResizeList (cctx->pList, cctx->pList->nOp + 1))
	{
	    xglRecordError (GL_OUT_OF_MEMORY);
	    return;
	}

	cctx->pList->pOp[cctx->pList->nOp].type   = XGL_LIST_OP_LIST;
	cctx->pList->pOp[cctx->pList->nOp].u.list = list;
	cctx->pList->nOp++;
    }
    else
    {
	xglDisplayListPtr pDisplayList;

	pDisplayList = (xglDisplayListPtr)
	    xglHashLookup (cctx->shared->displayLists, list);
	if (pDisplayList)
	{
	    xglListOpPtr pOp = pDisplayList->pOp;
	    int		 nOp = pDisplayList->nOp;

	    while (nOp--)
	    {
		switch (pOp->type) {
		case XGL_LIST_OP_CALLS:
		    glCallList (pOp->u.list);
		    break;
		case XGL_LIST_OP_DRAW:
		    xglDrawList (pOp->u.list);
		    break;
		case XGL_LIST_OP_GL:
		    (*pOp->u.gl->glProc) (pOp->u.gl);
		    break;
		case XGL_LIST_OP_LIST:
		    xglCallDisplayList (pOp->u.list, nesting + 1);
		    break;
		}

		pOp++;
	    }
	}
    }
}

static void
xglCallList (GLuint list)
{
    xglCallDisplayList (list, 1);
}

static void
xglCallLists (GLsizei	   n,
	      GLenum	   type,
	      const GLvoid *lists)
{
    GLuint list;
    GLint  base, i;

    glGetIntegerv (GL_LIST_BASE, &base);

    for (i = 0; i < n; i++)
    {
	switch (type) {
	case GL_BYTE:
	    list = (GLuint) *(((GLbyte *) lists) + n);
	    break;
	case GL_UNSIGNED_BYTE:
	    list = (GLuint) *(((GLubyte *) lists) + n);
	    break;
	case GL_SHORT:
	    list = (GLuint) *(((GLshort *) lists) + n);
	    break;
	case GL_UNSIGNED_SHORT:
	    list = (GLuint) *(((GLushort *) lists) + n);
	    break;
	case GL_INT:
	    list = (GLuint) *(((GLint *) lists) + n);
	    break;
	case GL_UNSIGNED_INT:
	    list = (GLuint) *(((GLuint *) lists) + n);
	    break;
	case GL_FLOAT:
	    list = (GLuint) *(((GLfloat *) lists) + n);
	    break;
	case GL_2_BYTES:
	{
	    GLubyte *ubptr = ((GLubyte *) lists) + 2 * n;
	    list = (GLuint) *ubptr * 256 + (GLuint) *(ubptr + 1);
	} break;
	case GL_3_BYTES:
	{
	    GLubyte *ubptr = ((GLubyte *) lists) + 3 * n;
	    list = (GLuint) * ubptr * 65536
		+ (GLuint) * (ubptr + 1) * 256
		+ (GLuint) * (ubptr + 2);
	} break;
	case GL_4_BYTES:
	{
	    GLubyte *ubptr = ((GLubyte *) lists) + 4 * n;
	    list = (GLuint) * ubptr * 16777216
		+ (GLuint) * (ubptr + 1) * 65536
		+ (GLuint) * (ubptr + 2) * 256
		+ (GLuint) * (ubptr + 3);
	} break;
	default:
	    xglRecordError (GL_INVALID_ENUM);
	    return;
	}

	xglCallDisplayList (base + list, 1);
    }
}

static void
xglDeleteLists (GLuint  list,
		GLsizei range)
{
    xglDisplayListPtr pDisplayList;
    GLint	      i;

    if (range < 0)
    {
	xglRecordError (GL_INVALID_VALUE);
	return;
    }

    for (i = list; i < list + range; i++)
    {
	if (!i)
	    continue;

	pDisplayList = (xglDisplayListPtr)
	    xglHashLookup (cctx->shared->displayLists, i);
	if (pDisplayList)
	{
	    xglHashRemove (cctx->shared->displayLists, i);
	    xglDestroyList (pDisplayList);
	}
    }
}

static GLboolean
xglIsList (GLuint list)
{
    xglDisplayListPtr pDisplayList;

    if (!list)
	return GL_FALSE;

    pDisplayList = (xglDisplayListPtr)
	xglHashLookup (cctx->shared->displayLists, list);
    if (pDisplayList)
	return GL_TRUE;

    return GL_FALSE;
}

static void
xglFlush (void)
{
    glFlush ();

    if (cctx && cctx->pDrawBuffer->pDrawable)
    {
	xglGLBufferPtr pBuffer = cctx->pDrawBuffer;

	if (REGION_NOTEMPTY (pBuffer->pDrawable->pScreen, &pBuffer->damage))
	{
	    XGL_DRAWABLE_PIXMAP_PRIV (pBuffer->pDrawable);

	    DamageDamageRegion (pBuffer->pDrawable, &pBuffer->damage);
	    REGION_EMPTY (pBuffer->pDrawable->pScreen, &pBuffer->damage);

	    pPixmapPriv->damageBox = miEmptyBox;
	}
    }
}

static void
xglFinish (void)
{
    glFinish ();

    if (cctx && cctx->pDrawBuffer->pDrawable)
    {
	xglGLBufferPtr pBuffer = cctx->pDrawBuffer;

	if (REGION_NOTEMPTY (pBuffer->pDrawable->pScreen, &pBuffer->damage))
	{
	    XGL_DRAWABLE_PIXMAP_PRIV (pBuffer->pDrawable);

	    DamageDamageRegion (pBuffer->pDrawable, &pBuffer->damage);
	    REGION_EMPTY (pBuffer->pDrawable->pScreen, &pBuffer->damage);

	    pPixmapPriv->damageBox = miEmptyBox;
	}
    }
}

static void
xglClear (GLbitfield mask)
{
    GLenum mode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glClear (mask);
	glEndList ();

	mode = cctx->listMode;
    }
    else
	mode = GL_COMPILE_AND_EXECUTE;

    if (mode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE_WITHOUT_TEXTURES (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glClear (mask);

		if (mask & GL_COLOR_BUFFER_BIT)
		    XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglAccum (GLenum  op,
	  GLfloat value)
{
    if (op == GL_RETURN)
    {
	GLenum listMode;

	if (cctx->list)
	{
	    glEndList ();
	    xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	    glAccum (GL_RETURN, value);
	    glEndList ();

	    listMode = cctx->listMode;
	}
	else
	    listMode = GL_COMPILE_AND_EXECUTE;

	if (listMode == GL_COMPILE_AND_EXECUTE)
	{
	    RegionRec region;
	    BoxRec    scissor, box;
	    BoxPtr    pBox;
	    int	      nBox;

	    XGL_GLX_DRAW_PROLOGUE_WITHOUT_TEXTURES (pBox, nBox, &scissor);

	    while (nBox--)
	    {
		XGL_GLX_DRAW_BOX (&box, pBox);

		pBox++;

		if (cctx->attrib.scissorTest)
		    XGL_GLX_INTERSECT_BOX (&box, &scissor);

		if (box.x1 < box.x2 && box.y1 < box.y2)
		{
		    XGL_GLX_SET_SCISSOR_BOX (&box);

		    glAccum (GL_RETURN, value);

		    XGL_GLX_DRAW_DAMAGE (&box, &region);
		}
	    }
	}

	if (cctx->list)
	    xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
    }
    else
	glAccum (op, value);
}

static void
xglDrawArrays (GLenum  mode,
	       GLint   first,
	       GLsizei count)
{
    GLenum listMode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glDrawArrays (mode, first, count);
	glEndList ();

	listMode = cctx->listMode;
    }
    else
	listMode = GL_COMPILE_AND_EXECUTE;

    if (listMode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glDrawArrays (mode, first, count);

		XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglDrawElements (GLenum	      mode,
		 GLsizei      count,
		 GLenum	      type,
		 const GLvoid *indices)
{
    GLenum listMode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glDrawElements (mode, count, type, indices);
	glEndList ();

	listMode = cctx->listMode;
    }
    else
	listMode = GL_COMPILE_AND_EXECUTE;

    if (listMode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glDrawElements (mode, count, type, indices);

		XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglDrawPixels (GLsizei	    width,
	       GLsizei	    height,
	       GLenum	    format,
	       GLenum	    type,
	       const GLvoid *pixels)
{
    GLenum listMode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glDrawPixels (width, height, format, type, pixels);
	glEndList ();

	listMode = cctx->listMode;
    }
    else
	listMode = GL_COMPILE_AND_EXECUTE;

    if (listMode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glDrawPixels (width, height, format, type, pixels);

		if (format != GL_STENCIL_INDEX)
		    XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglBitmap (GLsizei	 width,
	   GLsizei	 height,
	   GLfloat	 xorig,
	   GLfloat	 yorig,
	   GLfloat	 xmove,
	   GLfloat	 ymove,
	   const GLubyte *bitmap)
{
    GLenum listMode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glBitmap (width, height, xorig, yorig, 0, 0, bitmap);
	glEndList ();

	listMode = cctx->listMode;
    }
    else
	listMode = GL_COMPILE_AND_EXECUTE;

    if (listMode == GL_COMPILE_AND_EXECUTE && width && height)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glBitmap (width, height, xorig, yorig, 0, 0, bitmap);

		XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);

    glBitmap (0, 0, 0, 0, xmove, ymove, NULL);
}

static void
xglRectdv (const GLdouble *v1,
	   const GLdouble *v2)
{
    GLenum listMode;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
	glRectdv (v1, v2);
	glEndList ();

	listMode = cctx->listMode;
    }
    else
	listMode = GL_COMPILE_AND_EXECUTE;

    if (listMode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;

	XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		XGL_GLX_SET_SCISSOR_BOX (&box);

		glRectdv (v1, v2);

		XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglRectfv (const GLfloat *v1,
	   const GLfloat *v2)
{
    GLdouble dv1[2];
    GLdouble dv2[2];

    dv1[0] = (GLdouble) v1[0];
    dv1[1] = (GLdouble) v1[1];
    dv2[0] = (GLdouble) v2[0];
    dv2[1] = (GLdouble) v2[1];

    xglRectdv (dv1, dv2);
}

static void
xglRectiv (const GLint *v1,
	   const GLint *v2)
{
    GLdouble dv1[2];
    GLdouble dv2[2];

    dv1[0] = (GLdouble) v1[0];
    dv1[1] = (GLdouble) v1[1];
    dv2[0] = (GLdouble) v2[0];
    dv2[1] = (GLdouble) v2[1];

    xglRectdv (dv1, dv2);
}

static void
xglRectsv (const GLshort *v1,
	   const GLshort *v2)
{
    GLdouble dv1[2];
    GLdouble dv2[2];

    dv1[0] = (GLdouble) v1[0];
    dv1[1] = (GLdouble) v1[1];
    dv2[0] = (GLdouble) v2[0];
    dv2[1] = (GLdouble) v2[1];

    xglRectdv (dv1, dv2);
}

static void
xglBegin (GLenum mode)
{
    if (mode > GL_POLYGON)
    {
	xglRecordError (GL_INVALID_ENUM);
	return;
    }

    if (cctx->beginCnt)
    {
	xglRecordError (GL_INVALID_OPERATION);
	return;
    }

    cctx->beginCnt++;

    if (cctx->list)
    {
	glEndList ();
	xglStartList (XGL_LIST_OP_DRAW, GL_COMPILE);
    }
    else
    {
	if (REGION_NUM_RECTS (cctx->pDrawBuffer->pGC->pCompositeClip) == 1)
	{
	    BoxRec scissor, box;
	    BoxPtr pBox;
	    int    nBox;

	    XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	    XGL_GLX_DRAW_BOX (&box, pBox);

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    XGL_GLX_SET_SCISSOR_BOX (&box);
	}
	else
	{
	    if (!cctx->groupList)
		cctx->groupList = glGenLists (1);

	    glNewList (cctx->groupList, GL_COMPILE);
	}
    }

    glBegin (mode);
}

static void
xglEnd (void)
{
    if (!cctx->beginCnt)
    {
	xglRecordError (GL_INVALID_OPERATION);
	return;
    }

    cctx->beginCnt--;

    glEnd ();

    if (!cctx->list || cctx->listMode == GL_COMPILE_AND_EXECUTE)
    {
	RegionRec region;
	BoxRec    scissor, box;
	BoxPtr    pBox;
	int	  nBox;
	GLuint	  list = 0;

	if (cctx->list)
	{
	    XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

	    list = cctx->pList->pOp[cctx->pList->nOp - 1].u.list;
	}
	else
	{
	    if (REGION_NUM_RECTS (cctx->pDrawBuffer->pGC->pCompositeClip) == 1)
	    {
		XGL_GLX_DRAW_PROLOGUE_WITHOUT_TEXTURES (pBox, nBox, &scissor);
	    }
	    else
	    {
		XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

		list = cctx->groupList;
	    }
	}

	if (list)
	    glEndList ();

	while (nBox--)
	{
	    XGL_GLX_DRAW_BOX (&box, pBox);

	    pBox++;

	    if (cctx->attrib.scissorTest)
		XGL_GLX_INTERSECT_BOX (&box, &scissor);

	    if (box.x1 < box.x2 && box.y1 < box.y2)
	    {
		if (list)
		{
		    XGL_GLX_SET_SCISSOR_BOX (&box);

		    glCallList (list);
		}

		XGL_GLX_DRAW_DAMAGE (&box, &region);
	    }
	}
    }
    else
    {
	glEndList ();
    }

    if (cctx->list)
	xglStartList (XGL_LIST_OP_CALLS, cctx->listMode);
}

static void
xglCopyPixelsProc (xglGLOpPtr pOp)
{
    RegionRec region;
    BoxRec    scissor, box;
    BoxPtr    pBox;
    int	      nBox;

    XGL_GLX_DRAW_PROLOGUE (pBox, nBox, &scissor);

    while (nBox--)
    {
	XGL_GLX_DRAW_BOX (&box, pBox);

	pBox++;

	if (cctx->attrib.scissorTest)
	    XGL_GLX_INTERSECT_BOX (&box, &scissor);

	if (box.x1 < box.x2 && box.y1 < box.y2)
	{
	    XGL_GLX_SET_SCISSOR_BOX (&box);

	    glCopyPixels (pOp->u.copy_pixels.x + cctx->pReadBuffer->xOff,
			  pOp->u.copy_pixels.y + cctx->pReadBuffer->yOff,
			  pOp->u.copy_pixels.width,
			  pOp->u.copy_pixels.height,
			  pOp->u.copy_pixels.type);

	    if (pOp->u.copy_pixels.type == GL_COLOR)
		XGL_GLX_DRAW_DAMAGE (&box, &region);
	}
    }
}

static void
xglCopyPixels (GLint   x,
	       GLint   y,
	       GLsizei width,
	       GLsizei height,
	       GLenum  type)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyPixelsProc;

    gl.u.copy_pixels.x	    = x;
    gl.u.copy_pixels.y	    = y;
    gl.u.copy_pixels.width  = width;
    gl.u.copy_pixels.height = height;
    gl.u.copy_pixels.type   = type;

    xglGLOp (&gl);
}

static void
xglReadPixels (GLint   x,
	       GLint   y,
	       GLsizei width,
	       GLsizei height,
	       GLenum  format,
	       GLenum  type,
	       GLvoid  *pixels)
{
    glReadPixels (x + cctx->pReadBuffer->xOff,
		  y + cctx->pReadBuffer->yOff,
		  width, height, format, type, pixels);
}

static void
xglCopyTexImage1DProc (xglGLOpPtr pOp)
{
    glCopyTexImage1D (pOp->u.copy_tex_image_1d.target,
		      pOp->u.copy_tex_image_1d.level,
		      pOp->u.copy_tex_image_1d.internalformat,
		      pOp->u.copy_tex_image_1d.x + cctx->pReadBuffer->xOff,
		      pOp->u.copy_tex_image_1d.y + cctx->pReadBuffer->yOff,
		      pOp->u.copy_tex_image_1d.width,
		      pOp->u.copy_tex_image_1d.border);
}

static void
xglCopyTexImage1D (GLenum  target,
		   GLint   level,
		   GLenum  internalformat,
		   GLint   x,
		   GLint   y,
		   GLsizei width,
		   GLint   border)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyTexImage1DProc;

    gl.u.copy_tex_image_1d.target	  = target;
    gl.u.copy_tex_image_1d.level	  = level;
    gl.u.copy_tex_image_1d.internalformat = internalformat;
    gl.u.copy_tex_image_1d.x		  = x;
    gl.u.copy_tex_image_1d.y		  = y;
    gl.u.copy_tex_image_1d.width	  = width;
    gl.u.copy_tex_image_1d.border	  = border;

    xglGLOp (&gl);
}

static void
xglCopyTexImage2DProc (xglGLOpPtr pOp)
{
    glCopyTexImage2D (pOp->u.copy_tex_image_2d.target,
		      pOp->u.copy_tex_image_2d.level,
		      pOp->u.copy_tex_image_2d.internalformat,
		      pOp->u.copy_tex_image_2d.x + cctx->pReadBuffer->xOff,
		      pOp->u.copy_tex_image_2d.y + cctx->pReadBuffer->yOff,
		      pOp->u.copy_tex_image_2d.width,
		      pOp->u.copy_tex_image_2d.height,
		      pOp->u.copy_tex_image_2d.border);
}

static void
xglCopyTexImage2D (GLenum  target,
		   GLint   level,
		   GLenum  internalformat,
		   GLint   x,
		   GLint   y,
		   GLsizei width,
		   GLsizei height,
		   GLint   border)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyTexImage2DProc;

    gl.u.copy_tex_image_2d.target	  = target;
    gl.u.copy_tex_image_2d.level	  = level;
    gl.u.copy_tex_image_2d.internalformat = internalformat;
    gl.u.copy_tex_image_2d.x		  = x;
    gl.u.copy_tex_image_2d.y		  = y;
    gl.u.copy_tex_image_2d.width	  = width;
    gl.u.copy_tex_image_2d.height	  = height;
    gl.u.copy_tex_image_2d.border	  = border;

    xglGLOp (&gl);
}

static void
xglCopyTexSubImage1DProc (xglGLOpPtr pOp)
{
    glCopyTexSubImage1D (pOp->u.copy_tex_sub_image_1d.target,
			 pOp->u.copy_tex_sub_image_1d.level,
			 pOp->u.copy_tex_sub_image_1d.xoffset,
			 pOp->u.copy_tex_sub_image_1d.x +
			 cctx->pReadBuffer->xOff,
			 pOp->u.copy_tex_sub_image_1d.y +
			 cctx->pReadBuffer->yOff,
			 pOp->u.copy_tex_sub_image_1d.width);
}

static void
xglCopyTexSubImage1D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyTexSubImage1DProc;

    gl.u.copy_tex_sub_image_1d.target  = target;
    gl.u.copy_tex_sub_image_1d.level   = level;
    gl.u.copy_tex_sub_image_1d.xoffset = xoffset;
    gl.u.copy_tex_sub_image_1d.x       = x;
    gl.u.copy_tex_sub_image_1d.y       = y;
    gl.u.copy_tex_sub_image_1d.width   = width;

    xglGLOp (&gl);
}

static void
xglCopyTexSubImage2DProc (xglGLOpPtr pOp)
{
    glCopyTexSubImage2D (pOp->u.copy_tex_sub_image_2d.target,
			 pOp->u.copy_tex_sub_image_2d.level,
			 pOp->u.copy_tex_sub_image_2d.xoffset,
			 pOp->u.copy_tex_sub_image_2d.yoffset,
			 pOp->u.copy_tex_sub_image_2d.x +
			 cctx->pReadBuffer->xOff,
			 pOp->u.copy_tex_sub_image_2d.y +
			 cctx->pReadBuffer->yOff,
			 pOp->u.copy_tex_sub_image_2d.width,
			 pOp->u.copy_tex_sub_image_2d.height);
}

static void
xglCopyTexSubImage2D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   yoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width,
		      GLsizei height)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyTexSubImage2DProc;

    gl.u.copy_tex_sub_image_2d.target  = target;
    gl.u.copy_tex_sub_image_2d.level   = level;
    gl.u.copy_tex_sub_image_2d.xoffset = xoffset;
    gl.u.copy_tex_sub_image_2d.yoffset = yoffset;
    gl.u.copy_tex_sub_image_2d.x       = x;
    gl.u.copy_tex_sub_image_2d.y       = y;
    gl.u.copy_tex_sub_image_2d.width   = width;
    gl.u.copy_tex_sub_image_2d.height  = height;

    xglGLOp (&gl);
}

static void
xglCopyColorTableProc (xglGLOpPtr pOp)
{
    glCopyColorTable (pOp->u.copy_color_table.target,
		      pOp->u.copy_color_table.internalformat,
		      pOp->u.copy_color_table.x + cctx->pReadBuffer->xOff,
		      pOp->u.copy_color_table.y + cctx->pReadBuffer->yOff,
		      pOp->u.copy_color_table.width);
}

static void
xglCopyColorTable (GLenum  target,
		   GLenum  internalformat,
		   GLint   x,
		   GLint   y,
		   GLsizei width)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyColorTableProc;

    gl.u.copy_color_table.target	 = target;
    gl.u.copy_color_table.internalformat = internalformat;
    gl.u.copy_color_table.x		 = x;
    gl.u.copy_color_table.y		 = y;
    gl.u.copy_color_table.width		 = width;

    xglGLOp (&gl);
}

static void
xglCopyColorSubTableProc (xglGLOpPtr pOp)
{
    glCopyColorTable (pOp->u.copy_color_sub_table.target,
		      pOp->u.copy_color_sub_table.start,
		      pOp->u.copy_color_sub_table.x + cctx->pReadBuffer->xOff,
		      pOp->u.copy_color_sub_table.y + cctx->pReadBuffer->yOff,
		      pOp->u.copy_color_sub_table.width);
}

static void
xglCopyColorSubTable (GLenum  target,
		      GLsizei start,
		      GLint   x,
		      GLint   y,
		      GLsizei width)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyColorSubTableProc;

    gl.u.copy_color_sub_table.target = target;
    gl.u.copy_color_sub_table.start  = start;
    gl.u.copy_color_sub_table.x	     = x;
    gl.u.copy_color_sub_table.y	     = y;
    gl.u.copy_color_sub_table.width  = width;

    xglGLOp (&gl);
}

static void
xglCopyConvolutionFilter1DProc (xglGLOpPtr pOp)
{
    GLenum internalformat = pOp->u.copy_convolution_filter_1d.internalformat;

    glCopyConvolutionFilter1D (pOp->u.copy_convolution_filter_1d.target,
			       internalformat,
			       pOp->u.copy_convolution_filter_1d.x +
			       cctx->pReadBuffer->xOff,
			       pOp->u.copy_convolution_filter_1d.y +
			       cctx->pReadBuffer->yOff,
			       pOp->u.copy_convolution_filter_1d.width);
}

static void
xglCopyConvolutionFilter1D (GLenum  target,
			    GLenum  internalformat,
			    GLint   x,
			    GLint   y,
			    GLsizei width)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyConvolutionFilter1DProc;

    gl.u.copy_convolution_filter_1d.target	   = target;
    gl.u.copy_convolution_filter_1d.internalformat = internalformat;
    gl.u.copy_convolution_filter_1d.x		   = x;
    gl.u.copy_convolution_filter_1d.y		   = y;
    gl.u.copy_convolution_filter_1d.width	   = width;

    xglGLOp (&gl);
}

static void
xglCopyConvolutionFilter2DProc (xglGLOpPtr pOp)
{
    GLenum internalformat = pOp->u.copy_convolution_filter_2d.internalformat;

    glCopyConvolutionFilter2D (pOp->u.copy_convolution_filter_2d.target,
			       internalformat,
			       pOp->u.copy_convolution_filter_2d.x +
			       cctx->pReadBuffer->xOff,
			       pOp->u.copy_convolution_filter_2d.y +
			       cctx->pReadBuffer->yOff,
			       pOp->u.copy_convolution_filter_2d.width,
			       pOp->u.copy_convolution_filter_2d.height);
}

static void
xglCopyConvolutionFilter2D (GLenum  target,
			    GLenum  internalformat,
			    GLint   x,
			    GLint   y,
			    GLsizei width,
			    GLsizei height)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyConvolutionFilter2DProc;

    gl.u.copy_convolution_filter_2d.target	   = target;
    gl.u.copy_convolution_filter_2d.internalformat = internalformat;
    gl.u.copy_convolution_filter_2d.x		   = x;
    gl.u.copy_convolution_filter_2d.y		   = y;
    gl.u.copy_convolution_filter_2d.width	   = width;
    gl.u.copy_convolution_filter_2d.height	   = height;

    xglGLOp (&gl);
}

static void
xglCopyTexSubImage3DProc (xglGLOpPtr pOp)
{
    glCopyTexSubImage3D (pOp->u.copy_tex_sub_image_3d.target,
			 pOp->u.copy_tex_sub_image_3d.level,
			 pOp->u.copy_tex_sub_image_3d.xoffset,
			 pOp->u.copy_tex_sub_image_3d.yoffset,
			 pOp->u.copy_tex_sub_image_3d.zoffset,
			 pOp->u.copy_tex_sub_image_3d.x +
			 cctx->pReadBuffer->xOff,
			 pOp->u.copy_tex_sub_image_3d.y +
			 cctx->pReadBuffer->yOff,
			 pOp->u.copy_tex_sub_image_3d.width,
			 pOp->u.copy_tex_sub_image_3d.height);
}

static void
xglCopyTexSubImage3D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   yoffset,
		      GLint   zoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width,
		      GLsizei height)
{
    xglGLOpRec gl;

    gl.glProc = xglCopyTexSubImage3DProc;

    gl.u.copy_tex_sub_image_3d.target  = target;
    gl.u.copy_tex_sub_image_3d.level   = level;
    gl.u.copy_tex_sub_image_3d.xoffset = xoffset;
    gl.u.copy_tex_sub_image_3d.yoffset = yoffset;
    gl.u.copy_tex_sub_image_3d.zoffset = zoffset;
    gl.u.copy_tex_sub_image_3d.x       = x;
    gl.u.copy_tex_sub_image_3d.y       = y;
    gl.u.copy_tex_sub_image_3d.width   = width;
    gl.u.copy_tex_sub_image_3d.height  = height;

    xglGLOp (&gl);
}

/* GL_ARB_multitexture */
static void
xglNoOpActiveTextureARB (GLenum texture) {}
static void
xglActiveTextureARBProc (xglGLOpPtr pOp)
{
    GLenum texUnit;

    texUnit = pOp->u.enumeration - GL_TEXTURE0;
    if (texUnit < 0 || texUnit >= cctx->maxTexUnits)
    {
	xglRecordError (GL_INVALID_ENUM);
    }
    else
    {
	cctx->activeTexUnit = texUnit;
	(*cctx->ActiveTextureARB) (pOp->u.enumeration);
    }
}
static void
xglActiveTextureARB (GLenum texture)
{
    xglGLOpRec gl;

    gl.glProc = xglActiveTextureARBProc;

    gl.u.enumeration = texture;

    xglGLOp (&gl);
}
static void
xglNoOpClientActiveTextureARB (GLenum texture) {}
static void
xglNoOpMultiTexCoord1dvARB (GLenum target, const GLdouble *v) {}
static void
xglNoOpMultiTexCoord1fvARB (GLenum target, const GLfloat *v) {}
static void
xglNoOpMultiTexCoord1ivARB (GLenum target, const GLint *v) {}
static void
xglNoOpMultiTexCoord1svARB (GLenum target, const GLshort *v) {}
static void
xglNoOpMultiTexCoord2dvARB (GLenum target, const GLdouble *v) {}
static void
xglNoOpMultiTexCoord2fvARB (GLenum target, const GLfloat *v) {}
static void
xglNoOpMultiTexCoord2ivARB (GLenum target, const GLint *v) {}
static void
xglNoOpMultiTexCoord2svARB (GLenum target, const GLshort *v) {}
static void
xglNoOpMultiTexCoord3dvARB (GLenum target, const GLdouble *v) {}
static void
xglNoOpMultiTexCoord3fvARB (GLenum target, const GLfloat *v) {}
static void
xglNoOpMultiTexCoord3ivARB (GLenum target, const GLint *v) {}
static void
xglNoOpMultiTexCoord3svARB (GLenum target, const GLshort *v) {}
static void
xglNoOpMultiTexCoord4dvARB (GLenum target, const GLdouble *v) {}
static void
xglNoOpMultiTexCoord4fvARB (GLenum target, const GLfloat *v) {}
static void
xglNoOpMultiTexCoord4ivARB (GLenum target, const GLint *v) {}
static void
xglNoOpMultiTexCoord4svARB (GLenum target, const GLshort *v) {}

/* GL_ARB_multisample */
static void
xglNoOpSampleCoverageARB (GLclampf value, GLboolean invert) {}

/* GL_EXT_texture_object */
static GLboolean
xglNoOpAreTexturesResidentEXT (GLsizei n,
			       const GLuint *textures,
			       GLboolean *residences)
{
    return GL_FALSE;
}
static void
xglNoOpGenTexturesEXT (GLsizei n, GLuint *textures) {}
static GLboolean
xglNoOpIsTextureEXT (GLuint texture)
{
    return GL_FALSE;
}

/* GL_SGIS_multisample */
static void
xglNoOpSampleMaskSGIS (GLclampf value, GLboolean invert) {}
static void
xglNoOpSamplePatternSGIS (GLenum pattern) {}

/* GL_EXT_point_parameters */
static void
xglNoOpPointParameterfEXT (GLenum pname, GLfloat param) {}
static void
xglNoOpPointParameterfvEXT (GLenum pname, const GLfloat *params) {}

/* GL_MESA_window_pos */
static void
xglNoOpWindowPos3fMESA (GLfloat x, GLfloat y, GLfloat z) {}
static void
xglWindowPos3fMESAProc (xglGLOpPtr pOp)
{
    (*cctx->WindowPos3fMESA) (pOp->u.window_pos_3f.x + cctx->pDrawBuffer->xOff,
			      pOp->u.window_pos_3f.y + cctx->pDrawBuffer->yOff,
			      pOp->u.window_pos_3f.z);
}
static void
xglWindowPos3fMESA (GLfloat x, GLfloat y, GLfloat z)
{
    xglGLOpRec gl;

    gl.glProc = xglWindowPos3fMESAProc;

    gl.u.window_pos_3f.x = x;
    gl.u.window_pos_3f.y = y;
    gl.u.window_pos_3f.z = z;

    xglGLOp (&gl);
}

/* GL_EXT_blend_func_separate */
static void
xglNoOpBlendFuncSeparateEXT (GLenum sfactorRGB, GLenum dfactorRGB,
			     GLenum sfactorAlpha, GLenum dfactorAlpha) {}

/* GL_EXT_fog_coord */
static void
xglNoOpFogCoordfvEXT (const GLfloat *coord) {}
static void
xglNoOpFogCoorddvEXT (const GLdouble *coord) {}
static void
xglNoOpFogCoordPointerEXT (GLenum type, GLsizei stride,
			   const GLvoid *pointer) {}

/* GL_EXT_secondary_color */
static void
xglNoOpSecondaryColor3bvEXT (const GLbyte *v) {}
static void
xglNoOpSecondaryColor3dvEXT (const GLdouble *v) {}
static void
xglNoOpSecondaryColor3fvEXT (const GLfloat *v) {}
static void
xglNoOpSecondaryColor3ivEXT (const GLint *v) {}
static void
xglNoOpSecondaryColor3svEXT (const GLshort *v) {}
static void
xglNoOpSecondaryColor3ubvEXT (const GLubyte *v) {}
static void
xglNoOpSecondaryColor3uivEXT (const GLuint *v) {}
static void
xglNoOpSecondaryColor3usvEXT (const GLushort *v) {}
static void
xglNoOpSecondaryColorPointerEXT (GLint size, GLenum type, GLsizei stride,
				 const GLvoid *pointer) {}

/* GL_NV_point_sprite */
static void
xglNoOpPointParameteriNV (GLenum pname, GLint params) {}
static void
xglNoOpPointParameterivNV (GLenum pname, const GLint *params) {}

/* GL_EXT_stencil_two_side */
static void
xglNoOpActiveStencilFaceEXT (GLenum face) {}

/* GL_EXT_framebuffer_object */
static GLboolean
xglNoOpIsRenderbufferEXT (GLuint renderbuffer)
{
    return FALSE;
}
static void
xglNoOpBindRenderbufferEXT (GLenum target, GLuint renderbuffer) {}
static void
xglNoOpDeleteRenderbuffersEXT (GLsizei n, const GLuint *renderbuffers) {}
static void
xglNoOpGenRenderbuffersEXT (GLsizei n, GLuint *renderbuffers) {}
static void
xglNoOpRenderbufferStorageEXT (GLenum target, GLenum internalformat,
			       GLsizei width, GLsizei height) {}
static void
xglNoOpGetRenderbufferParameterivEXT (GLenum target, GLenum pname,
				      GLint *params) {}
static GLboolean
xglNoOpIsFramebufferEXT (GLuint framebuffer)
{
    return FALSE;
}
static void
xglNoOpBindFramebufferEXT (GLenum target, GLuint framebuffer) {}
static void
xglNoOpDeleteFramebuffersEXT (GLsizei n, const GLuint *framebuffers) {}
static void
xglNoOpGenFramebuffersEXT (GLsizei n, GLuint *framebuffers) {}
static GLenum
xglNoOpCheckFramebufferStatusEXT (GLenum target)
{
    return GL_FRAMEBUFFER_UNSUPPORTED_EXT;
}
static void
xglNoOpFramebufferTexture1DEXT (GLenum target, GLenum attachment,
				GLenum textarget, GLuint texture,
				GLint level) {}
static void
xglNoOpFramebufferTexture2DEXT (GLenum target, GLenum attachment,
				GLenum textarget, GLuint texture,
				GLint level) {}
static void
xglNoOpFramebufferTexture3DEXT (GLenum target, GLenum attachment,
				GLenum textarget, GLuint texture,
				GLint level, GLint zoffset) {}
static void
xglNoOpFramebufferRenderbufferEXT (GLenum target, GLenum attachment,
				   GLenum renderbuffertarget,
				   GLuint renderbuffer) {}
static void
xglNoOpGetFramebufferAttachmentParameterivEXT (GLenum target,
					       GLenum attachment,
					       GLenum pname,
					       GLint *params) {}
static void
xglNoOpGenerateMipmapEXT (GLenum target) {}

static struct _glapi_table __glNativeRenderTable = {
    xglNewList,
    xglEndList,
    xglCallList,
    xglCallLists,
    xglDeleteLists,
    xglGenLists,
    glListBase,
    xglBegin,
    xglBitmap,
    0, /* glColor3b */
    glColor3bv,
    0, /* glColor3d */
    glColor3dv,
    0, /* glColor3f */
    glColor3fv,
    0, /* glColor3i */
    glColor3iv,
    0, /* glColor3s */
    glColor3sv,
    0, /* glColor3ub */
    glColor3ubv,
    0, /* glColor3ui */
    glColor3uiv,
    0, /* glColor3us */
    glColor3usv,
    0, /* glColor4b */
    glColor4bv,
    0, /* glColor4d */
    glColor4dv,
    0, /* glColor4f */
    glColor4fv,
    0, /* glColor4i */
    glColor4iv,
    0, /* glColor4s */
    glColor4sv,
    0, /* glColor4ub */
    glColor4ubv,
    0, /* glColor4ui */
    glColor4uiv,
    0, /* glColor4us */
    glColor4usv,
    0, /* glEdgeFlag */
    glEdgeFlagv,
    xglEnd,
    0, /* glIndexd */
    glIndexdv,
    0, /* glIndexf */
    glIndexfv,
    0, /* glIndexi */
    glIndexiv,
    0, /* glIndexs */
    glIndexsv,
    0, /* glNormal3b */
    glNormal3bv,
    0, /* glNormal3d */
    glNormal3dv,
    0, /* glNormal3f */
    glNormal3fv,
    0, /* glNormal3i */
    glNormal3iv,
    0, /* glNormal3s */
    glNormal3sv,
    0, /* glRasterPos2d */
    glRasterPos2dv,
    0, /* glRasterPos2f */
    glRasterPos2fv,
    0, /* glRasterPos2i */
    glRasterPos2iv,
    0, /* glRasterPos2s */
    glRasterPos2sv,
    0, /* glRasterPos3d */
    glRasterPos3dv,
    0, /* glRasterPos3f */
    glRasterPos3fv,
    0, /* glRasterPos3i */
    glRasterPos3iv,
    0, /* glRasterPos3s */
    glRasterPos3sv,
    0, /* glRasterPos4d */
    glRasterPos4dv,
    0, /* glRasterPos4f */
    glRasterPos4fv,
    0, /* glRasterPos4i */
    glRasterPos4iv,
    0, /* glRasterPos4s */
    glRasterPos4sv,
    0, /* glRectd */
    xglRectdv,
    0, /* glRectf */
    xglRectfv,
    0, /* glRecti */
    xglRectiv,
    0, /* glRects */
    xglRectsv,
    0, /* glTexCoord1d */
    glTexCoord1dv,
    0, /* glTexCoord1f */
    glTexCoord1fv,
    0, /* glTexCoord1i */
    glTexCoord1iv,
    0, /* glTexCoord1s */
    glTexCoord1sv,
    0, /* glTexCoord2d */
    glTexCoord2dv,
    0, /* glTexCoord2f */
    glTexCoord2fv,
    0, /* glTexCoord2i */
    glTexCoord2iv,
    0, /* glTexCoord2s */
    glTexCoord2sv,
    0, /* glTexCoord3d */
    glTexCoord3dv,
    0, /* glTexCoord3f */
    glTexCoord3fv,
    0, /* glTexCoord3i */
    glTexCoord3iv,
    0, /* glTexCoord3s */
    glTexCoord3sv,
    0, /* glTexCoord4d */
    glTexCoord4dv,
    0, /* glTexCoord4f */
    glTexCoord4fv,
    0, /* glTexCoord4i */
    glTexCoord4iv,
    0, /* glTexCoord4s */
    glTexCoord4sv,
    0, /* glVertex2d */
    glVertex2dv,
    0, /* glVertex2f */
    glVertex2fv,
    0, /* glVertex2i */
    glVertex2iv,
    0, /* glVertex2s */
    glVertex2sv,
    0, /* glVertex3d */
    glVertex3dv,
    0, /* glVertex3f */
    glVertex3fv,
    0, /* glVertex3i */
    glVertex3iv,
    0, /* glVertex3s */
    glVertex3sv,
    0, /* glVertex4d */
    glVertex4dv,
    0, /* glVertex4f */
    glVertex4fv,
    0, /* glVertex4i */
    glVertex4iv,
    0, /* glVertex4s */
    glVertex4sv,
    glClipPlane,
    glColorMaterial,
    glCullFace,
    glFogf,
    glFogfv,
    glFogi,
    glFogiv,
    glFrontFace,
    glHint,
    glLightf,
    glLightfv,
    glLighti,
    glLightiv,
    glLightModelf,
    glLightModelfv,
    glLightModeli,
    glLightModeliv,
    glLineStipple,
    glLineWidth,
    glMaterialf,
    glMaterialfv,
    glMateriali,
    glMaterialiv,
    glPointSize,
    glPolygonMode,
    glPolygonStipple,
    xglScissor,
    glShadeModel,
    xglTexParameterf,
    xglTexParameterfv,
    xglTexParameteri,
    xglTexParameteriv,
    glTexImage1D,
    glTexImage2D,
    glTexEnvf,
    glTexEnvfv,
    glTexEnvi,
    glTexEnviv,
    glTexGend,
    glTexGendv,
    glTexGenf,
    glTexGenfv,
    glTexGeni,
    glTexGeniv,
    glFeedbackBuffer,
    glSelectBuffer,
    glRenderMode,
    glInitNames,
    glLoadName,
    glPassThrough,
    glPopName,
    glPushName,
    xglDrawBuffer,
    xglClear,
    glClearAccum,
    glClearIndex,
    glClearColor,
    glClearStencil,
    glClearDepth,
    glStencilMask,
    glColorMask,
    glDepthMask,
    glIndexMask,
    xglAccum,
    xglDisable,
    xglEnable,
    xglFinish,
    xglFlush,
    xglPopAttrib,
    xglPushAttrib,
    glMap1d,
    glMap1f,
    glMap2d,
    glMap2f,
    glMapGrid1d,
    glMapGrid1f,
    glMapGrid2d,
    glMapGrid2f,
    0, /* glEvalCoord1d */
    glEvalCoord1dv,
    0, /* glEvalCoord1f */
    glEvalCoord1fv,
    0, /* glEvalCoord2d */
    glEvalCoord2dv,
    0, /* glEvalCoord2f */
    glEvalCoord2fv,
    glEvalMesh1,
    glEvalPoint1,
    glEvalMesh2,
    glEvalPoint2,
    glAlphaFunc,
    glBlendFunc,
    glLogicOp,
    glStencilFunc,
    glStencilOp,
    glDepthFunc,
    glPixelZoom,
    glPixelTransferf,
    glPixelTransferi,
    glPixelStoref,
    glPixelStorei,
    glPixelMapfv,
    glPixelMapuiv,
    glPixelMapusv,
    xglReadBuffer,
    xglCopyPixels,
    xglReadPixels,
    xglDrawPixels,
    xglGetBooleanv,
    glGetClipPlane,
    xglGetDoublev,
    xglGetError,
    xglGetFloatv,
    xglGetIntegerv,
    glGetLightfv,
    glGetLightiv,
    glGetMapdv,
    glGetMapfv,
    glGetMapiv,
    glGetMaterialfv,
    glGetMaterialiv,
    glGetPixelMapfv,
    glGetPixelMapuiv,
    glGetPixelMapusv,
    glGetPolygonStipple,
    xglGetString,
    glGetTexEnvfv,
    glGetTexEnviv,
    glGetTexGendv,
    glGetTexGenfv,
    glGetTexGeniv,
    glGetTexImage,
    glGetTexParameterfv,
    glGetTexParameteriv,
    xglGetTexLevelParameterfv,
    xglGetTexLevelParameteriv,
    xglIsEnabled,
    xglIsList,
    glDepthRange,
    glFrustum,
    glLoadIdentity,
    glLoadMatrixf,
    glLoadMatrixd,
    glMatrixMode,
    glMultMatrixf,
    glMultMatrixd,
    glOrtho,
    glPopMatrix,
    glPushMatrix,
    glRotated,
    glRotatef,
    glScaled,
    glScalef,
    glTranslated,
    glTranslatef,
    xglViewport,
    glArrayElement,
    xglBindTexture,
    glColorPointer,
    glDisableClientState,
    xglDrawArrays,
    xglDrawElements,
    glEdgeFlagPointer,
    glEnableClientState,
    glIndexPointer,
    0, /* glIndexub */
    glIndexubv,
    glInterleavedArrays,
    glNormalPointer,
    glPolygonOffset,
    glTexCoordPointer,
    glVertexPointer,
    xglAreTexturesResident,
    xglCopyTexImage1D,
    xglCopyTexImage2D,
    xglCopyTexSubImage1D,
    xglCopyTexSubImage2D,
    xglDeleteTextures,
    xglGenTextures,
    glGetPointerv,
    xglIsTexture,
    xglPrioritizeTextures,
    glTexSubImage1D,
    glTexSubImage2D,
    glPopClientAttrib,
    glPushClientAttrib,
    glBlendColor,
    glBlendEquation,
    0, /* glDrawRangeElements */
    glColorTable,
    glColorTableParameterfv,
    glColorTableParameteriv,
    xglCopyColorTable,
    glGetColorTable,
    glGetColorTableParameterfv,
    glGetColorTableParameteriv,
    glColorSubTable,
    xglCopyColorSubTable,
    glConvolutionFilter1D,
    glConvolutionFilter2D,
    glConvolutionParameterf,
    glConvolutionParameterfv,
    glConvolutionParameteri,
    glConvolutionParameteriv,
    xglCopyConvolutionFilter1D,
    xglCopyConvolutionFilter2D,
    glGetConvolutionFilter,
    glGetConvolutionParameterfv,
    glGetConvolutionParameteriv,
    glGetSeparableFilter,
    glSeparableFilter2D,
    glGetHistogram,
    glGetHistogramParameterfv,
    glGetHistogramParameteriv,
    glGetMinmax,
    glGetMinmaxParameterfv,
    glGetMinmaxParameteriv,
    glHistogram,
    glMinmax,
    glResetHistogram,
    glResetMinmax,
    glTexImage3D,
    glTexSubImage3D,
    xglCopyTexSubImage3D,
    xglNoOpActiveTextureARB,
    xglNoOpClientActiveTextureARB,
    0, /* glMultiTexCoord1dARB */
    xglNoOpMultiTexCoord1dvARB,
    0, /* glMultiTexCoord1fARB */
    xglNoOpMultiTexCoord1fvARB,
    0, /* glMultiTexCoord1iARB */
    xglNoOpMultiTexCoord1ivARB,
    0, /* glMultiTexCoord1sARB */
    xglNoOpMultiTexCoord1svARB,
    0, /* glMultiTexCoord2dARB */
    xglNoOpMultiTexCoord2dvARB,
    0, /* glMultiTexCoord2fARB */
    xglNoOpMultiTexCoord2fvARB,
    0, /* glMultiTexCoord2iARB */
    xglNoOpMultiTexCoord2ivARB,
    0, /* glMultiTexCoord2sARB */
    xglNoOpMultiTexCoord2svARB,
    0, /* glMultiTexCoord3dARB */
    xglNoOpMultiTexCoord3dvARB,
    0, /* glMultiTexCoord3fARB */
    xglNoOpMultiTexCoord3fvARB,
    0, /* glMultiTexCoord3iARB */
    xglNoOpMultiTexCoord3ivARB,
    0, /* glMultiTexCoord3sARB */
    xglNoOpMultiTexCoord3svARB,
    0, /* glMultiTexCoord4dARB */
    xglNoOpMultiTexCoord4dvARB,
    0, /* glMultiTexCoord4fARB */
    xglNoOpMultiTexCoord4fvARB,
    0, /* glMultiTexCoord4iARB */
    xglNoOpMultiTexCoord4ivARB,
    0, /* glMultiTexCoord4sARB */
    xglNoOpMultiTexCoord4svARB,
    0, /* glLoadTransposeMatrixfARB */
    0, /* glLoadTransposeMatrixdARB */
    0, /* glMultTransposeMatrixfARB */
    0, /* glMultTransposeMatrixdARB */
    xglNoOpSampleCoverageARB,
    0, /* glDrawBuffersARB */
    0, /* glPolygonOffsetEXT */
    0, /* glGetTexFilterFuncSGIS */
    0, /* glTexFilterFuncSGIS */
    0, /* glGetHistogramEXT */
    0, /* glGetHistogramParameterfvEXT */
    0, /* glGetHistogramParameterivEXT */
    0, /* glGetMinmaxEXT */
    0, /* glGetMinmaxParameterfvEXT */
    0, /* glGetMinmaxParameterivEXT */
    0, /* glGetConvolutionFilterEXT */
    0, /* glGetConvolutionParameterfvEXT */
    0, /* glGetConvolutionParameterivEXT */
    0, /* glGetSeparableFilterEXT */
    0, /* glGetColorTableSGI */
    0, /* glGetColorTableParameterfvSGI */
    0, /* glGetColorTableParameterivSGI */
    0, /* glPixelTexGenSGIX */
    0, /* glPixelTexGenParameteriSGIS */
    0, /* glPixelTexGenParameterivSGIS */
    0, /* glPixelTexGenParameterfSGIS */
    0, /* glPixelTexGenParameterfvSGIS */
    0, /* glGetPixelTexGenParameterivSGIS */
    0, /* glGetPixelTexGenParameterfvSGIS */
    0, /* glTexImage4DSGIS */
    0, /* glTexSubImage4DSGIS */
    xglNoOpAreTexturesResidentEXT,
    xglNoOpGenTexturesEXT,
    xglNoOpIsTextureEXT,
    0, /* glDetailTexFuncSGIS */
    0, /* glGetDetailTexFuncSGIS */
    0, /* glSharpenTexFuncSGIS */
    0, /* glGetSharpenTexFuncSGIS */
    xglNoOpSampleMaskSGIS,
    xglNoOpSamplePatternSGIS,
    0, /* glColorPointerEXT */
    0, /* glEdgeFlagPointerEXT */
    0, /* glIndexPointerEXT */
    0, /* glNormalPointerEXT */
    0, /* glTexCoordPointerEXT */
    0, /* glVertexPointerEXT */
    0, /* glSpriteParameterfSGIX */
    0, /* glSpriteParameterfvSGIX */
    0, /* glSpriteParameteriSGIX */
    0, /* glSpriteParameterivSGIX */
    xglNoOpPointParameterfEXT,
    xglNoOpPointParameterfvEXT,
    0, /* glGetInstrumentsSGIX */
    0, /* glInstrumentsBufferSGIX */
    0, /* glPollInstrumentsSGIX */
    0, /* glReadInstrumentsSGIX */
    0, /* glStartInstrumentsSGIX */
    0, /* glStopInstrumentsSGIX */
    0, /* glFrameZoomSGIX */
    0, /* glTagSampleBufferSGIX */
    0, /* glReferencePlaneSGIX */
    0, /* glFlushRasterSGIX */
    0, /* glGetListParameterfvSGIX */
    0, /* glGetListParameterivSGIX */
    0, /* glListParameterfSGIX */
    0, /* glListParameterfvSGIX */
    0, /* glListParameteriSGIX */
    0, /* glListParameterivSGIX */
    0, /* glFragmentColorMaterialSGIX */
    0, /* glFragmentLightfSGIX */
    0, /* glFragmentLightfvSGIX */
    0, /* glFragmentLightiSGIX */
    0, /* glFragmentLightivSGIX */
    0, /* glFragmentLightModelfSGIX */
    0, /* glFragmentLightModelfvSGIX */
    0, /* glFragmentLightModeliSGIX */
    0, /* glFragmentLightModelivSGIX */
    0, /* glFragmentMaterialfSGIX */
    0, /* glFragmentMaterialfvSGIX */
    0, /* glFragmentMaterialiSGIX */
    0, /* glFragmentMaterialivSGIX */
    0, /* glGetFragmentLightfvSGIX */
    0, /* glGetFragmentLightivSGIX */
    0, /* glGetFragmentMaterialfvSGIX */
    0, /* glGetFragmentMaterialivSGIX */
    0, /* glLightEnviSGIX */
    0, /* glVertexWeightfEXT */
    0, /* glVertexWeightfvEXT */
    0, /* glVertexWeightPointerEXT */
    0, /* glFlushVertexArrayRangeNV */
    0, /* glVertexArrayRangeNV */
    0, /* glCombinerParameterfvNV */
    0, /* glCombinerParameterfNV */
    0, /* glCombinerParameterivNV */
    0, /* glCombinerParameteriNV */
    0, /* glCombinerInputNV */
    0, /* glCombinerOutputNV */
    0, /* glFinalCombinerInputNV */
    0, /* glGetCombinerInputParameterfvNV */
    0, /* glGetCombinerInputParameterivNV */
    0, /* glGetCombinerOutputParameterfvNV */
    0, /* glGetCombinerOutputParameterivNV */
    0, /* glGetFinalCombinerInputParameterfvNV */
    0, /* glGetFinalCombinerInputParameterivNV */
    0, /* glResizeBuffersMESA */
    0, /* glWindowPos2dMESA */
    0, /* glWindowPos2dvMESA */
    0, /* glWindowPos2fMESA */
    0, /* glWindowPos2fvMESA */
    0, /* glWindowPos2iMESA */
    0, /* glWindowPos2ivMESA */
    0, /* glWindowPos2sMESA */
    0, /* glWindowPos2svMESA */
    0, /* glWindowPos3dMESA */
    0, /* glWindowPos3dvMESA */
    xglNoOpWindowPos3fMESA,
    0, /* glWindowPos3fvMESA */
    0, /* glWindowPos3iMESA */
    0, /* glWindowPos3ivMESA */
    0, /* glWindowPos3sMESA */
    0, /* glWindowPos3svMESA */
    0, /* glWindowPos4dMESA */
    0, /* glWindowPos4dvMESA */
    0, /* glWindowPos4fMESA */
    0, /* glWindowPos4fvMESA */
    0, /* glWindowPos4iMESA */
    0, /* glWindowPos4ivMESA */
    0, /* glWindowPos4sMESA */
    0, /* glWindowPos4svMESA */
    xglNoOpBlendFuncSeparateEXT,
    0, /* glIndexMaterialEXT */
    0, /* glIndexFuncEXT */
    0, /* glLockArraysEXT */
    0, /* glUnlockArraysEXT */
    0, /* glCullParameterdvEXT */
    0, /* glCullParameterfvEXT */
    0, /* glHintPGI */
    0, /* glFogCoordfEXT */
    xglNoOpFogCoordfvEXT,
    0, /* glFogCoorddEXT */
    xglNoOpFogCoorddvEXT,
    xglNoOpFogCoordPointerEXT,
    0, /* glGetColorTableEXT */
    0, /* glGetColorTableParameterivEXT */
    0, /* glGetColorTableParameterfvEXT */
    0, /* glTbufferMask3DFX */
    0, /* glCompressedTexImage3DARB */
    0, /* glCompressedTexImage2DARB */
    0, /* glCompressedTexImage1DARB */
    0, /* glCompressedTexSubImage3DARB */
    0, /* glCompressedTexSubImage2DARB */
    0, /* glCompressedTexSubImage1DARB */
    0, /* glGetCompressedTexImageARB */
    0, /* glSecondaryColor3bEXT */
    xglNoOpSecondaryColor3bvEXT,
    0, /* glSecondaryColor3dEXT */
    xglNoOpSecondaryColor3dvEXT,
    0, /* glSecondaryColor3fEXT */
    xglNoOpSecondaryColor3fvEXT,
    0, /* glSecondaryColor3iEXT */
    xglNoOpSecondaryColor3ivEXT,
    0, /* glSecondaryColor3sEXT */
    xglNoOpSecondaryColor3svEXT,
    0, /* glSecondaryColor3ubEXT */
    xglNoOpSecondaryColor3ubvEXT,
    0, /* glSecondaryColor3uiEXT */
    xglNoOpSecondaryColor3uivEXT,
    0, /* glSecondaryColor3usEXT */
    xglNoOpSecondaryColor3usvEXT,
    xglNoOpSecondaryColorPointerEXT,
    0, /* glAreProgramsResidentNV */
    0, /* glBindProgramNV */
    0, /* glDeleteProgramsNV */
    0, /* glExecuteProgramNV */
    0, /* glGenProgramsNV */
    0, /* glGetProgramParameterdvNV */
    0, /* glGetProgramParameterfvNV */
    0, /* glGetProgramivNV */
    0, /* glGetProgramStringNV */
    0, /* glGetTrackMatrixivNV */
    0, /* glGetVertexAttribdvARB */
    0, /* glGetVertexAttribfvARB */
    0, /* glGetVertexAttribivARB */
    0, /* glGetVertexAttribPointervNV */
    0, /* glIsProgramNV */
    0, /* glLoadProgramNV */
    0, /* glProgramParameter4dNV */
    0, /* glProgramParameter4dvNV */
    0, /* glProgramParameter4fNV */
    0, /* glProgramParameter4fvNV */
    0, /* glProgramParameters4dvNV */
    0, /* glProgramParameters4fvNV */
    0, /* glRequestResidentProgramsNV */
    0, /* glTrackMatrixNV */
    0, /* glVertexAttribPointerNV */
    0, /* glVertexAttrib1dARB */
    0, /* glVertexAttrib1dvARB */
    0, /* glVertexAttrib1fARB */
    0, /* glVertexAttrib1fvARB */
    0, /* glVertexAttrib1sARB */
    0, /* glVertexAttrib1svARB */
    0, /* glVertexAttrib2dARB */
    0, /* glVertexAttrib2dvARB */
    0, /* glVertexAttrib2fARB */
    0, /* glVertexAttrib2fvARB */
    0, /* glVertexAttrib2sARB */
    0, /* glVertexAttrib2svARB */
    0, /* glVertexAttrib3dARB */
    0, /* glVertexAttrib3dvARB */
    0, /* glVertexAttrib3fARB */
    0, /* glVertexAttrib3fvARB */
    0, /* glVertexAttrib3sARB */
    0, /* glVertexAttrib3svARB */
    0, /* glVertexAttrib4dARB */
    0, /* glVertexAttrib4dvARB */
    0, /* glVertexAttrib4fARB */
    0, /* glVertexAttrib4fvARB */
    0, /* glVertexAttrib4sARB */
    0, /* glVertexAttrib4svARB */
    0, /* glVertexAttrib4NubARB */
    0, /* glVertexAttrib4NubvARB */
    0, /* glVertexAttribs1dvNV */
    0, /* glVertexAttribs1fvNV */
    0, /* glVertexAttribs1svNV */
    0, /* glVertexAttribs2dvNV */
    0, /* glVertexAttribs2fvNV */
    0, /* glVertexAttribs2svNV */
    0, /* glVertexAttribs3dvNV */
    0, /* glVertexAttribs3fvNV */
    0, /* glVertexAttribs3svNV */
    0, /* glVertexAttribs4dvNV */
    0, /* glVertexAttribs4fvNV */
    0, /* glVertexAttribs4svNV */
    0, /* glVertexAttribs4ubvNV */
    xglNoOpPointParameteriNV,
    xglNoOpPointParameterivNV,
    0, /* glMultiDrawArraysEXT */
    0, /* glMultiDrawElementsEXT */
    xglNoOpActiveStencilFaceEXT,
    0, /* glDeleteFencesNV */
    0, /* glGenFencesNV */
    0, /* glIsFenceNV */
    0, /* glTestFenceNV */
    0, /* glGetFenceivNV */
    0, /* glFinishFenceNV */
    0, /* glSetFenceNV */
    0, /* glVertexAttrib4bvARB */
    0, /* glVertexAttrib4ivARB */
    0, /* glVertexAttrib4ubvARB */
    0, /* glVertexAttrib4usvARB */
    0, /* glVertexAttrib4uivARB */
    0, /* glVertexAttrib4NbvARB */
    0, /* glVertexAttrib4NsvARB */
    0, /* glVertexAttrib4NivARB */
    0, /* glVertexAttrib4NusvARB */
    0, /* glVertexAttrib4NuivARB */
    0, /* glVertexAttribPointerARB */
    0, /* glEnableVertexAttribArrayARB */
    0, /* glDisableVertexAttribArrayARB */
    0, /* glProgramStringARB */
    0, /* glProgramEnvParameter4dARB */
    0, /* glProgramEnvParameter4dvARB */
    0, /* glProgramEnvParameter4fARB */
    0, /* glProgramEnvParameter4fvARB */
    0, /* glProgramLocalParameter4dARB */
    0, /* glProgramLocalParameter4dvARB */
    0, /* glProgramLocalParameter4fARB */
    0, /* glProgramLocalParameter4fvARB */
    0, /* glGetProgramEnvParameterdvARB */
    0, /* glGetProgramEnvParameterfvARB */
    0, /* glGetProgramLocalParameterdvARB */
    0, /* glGetProgramLocalParameterfvARB */
    0, /* glGetProgramivARB */
    0, /* glGetProgramStringARB */
    0, /* glProgramNamedParameter4fNV */
    0, /* glProgramNamedParameter4dNV */
    0, /* glProgramNamedParameter4fvNV */
    0, /* glProgramNamedParameter4dvNV */
    0, /* glGetProgramNamedParameterfvNV */
    0, /* glGetProgramNamedParameterdvNV */
    0, /* glBindBufferARB */
    0, /* glBufferDataARB */
    0, /* glBufferSubDataARB */
    0, /* glDeleteBuffersARB */
    0, /* glGenBuffersARB */
    0, /* glGetBufferParameterivARB */
    0, /* glGetBufferPointervARB */
    0, /* glGetBufferSubDataARB */
    0, /* glIsBufferARB */
    0, /* glMapBufferARB */
    0, /* glUnmapBufferARB */
    0, /* glDepthBoundsEXT */
    0, /* glGenQueriesARB */
    0, /* glDeleteQueriesARB */
    0, /* glIsQueryARB */
    0, /* glBeginQueryARB */
    0, /* glEndQueryARB */
    0, /* glGetQueryivARB */
    0, /* glGetQueryObjectivARB */
    0, /* glGetQueryObjectuivARB */
    0, /* glMultiModeDrawArraysIBM */
    0, /* glMultiModeDrawElementsIBM */
    0, /* glBlendEquationSeparateEXT */
    0, /* glDeleteObjectARB */
    0, /* glGetHandleARB */
    0, /* glDetachObjectARB */
    0, /* glCreateShaderObjectARB */
    0, /* glShaderSourceARB */
    0, /* glCompileShaderARB */
    0, /* glCreateProgramObjectARB */
    0, /* glAttachObjectARB */
    0, /* glLinkProgramARB */
    0, /* glUseProgramObjectARB */
    0, /* glValidateProgramARB */
    0, /* glUniform1fARB */
    0, /* glUniform2fARB */
    0, /* glUniform3fARB */
    0, /* glUniform4fARB */
    0, /* glUniform1iARB */
    0, /* glUniform2iARB */
    0, /* glUniform3iARB */
    0, /* glUniform4iARB */
    0, /* glUniform1fvARB */
    0, /* glUniform2fvARB */
    0, /* glUniform3fvARB */
    0, /* glUniform4fvARB */
    0, /* glUniform1ivARB */
    0, /* glUniform2ivARB */
    0, /* glUniform3ivARB */
    0, /* glUniform4ivARB */
    0, /* glUniformMatrix2fvARB */
    0, /* glUniformMatrix3fvARB */
    0, /* glUniformMatrix4fvARB */
    0, /* glGetObjectParameterfvARB */
    0, /* glGetObjectParameterivARB */
    0, /* glGetInfoLogARB */
    0, /* glGetAttachedObjectsARB */
    0, /* glGetUniformLocationARB */
    0, /* glGetActiveUniformARB */
    0, /* glGetUniformfvARB */
    0, /* glGetUniformivARB */
    0, /* glGetShaderSourceARB */
    0, /* glBindAttribLocationARB */
    0, /* glGetActiveAttribARB */
    0, /* glGetAttribLocationARB */
    0, /* glGetVertexAttribdvNV */
    0, /* glGetVertexAttribfvNV */
    0, /* glGetVertexAttribivNV */
    0, /* glVertexAttrib1dNV */
    0, /* glVertexAttrib1dvNV */
    0, /* glVertexAttrib1fNV */
    0, /* glVertexAttrib1fvNV */
    0, /* glVertexAttrib1sNV */
    0, /* glVertexAttrib1svNV */
    0, /* glVertexAttrib2dNV */
    0, /* glVertexAttrib2dvNV */
    0, /* glVertexAttrib2fNV */
    0, /* glVertexAttrib2fvNV */
    0, /* glVertexAttrib2sNV */
    0, /* glVertexAttrib2svNV */
    0, /* glVertexAttrib3dNV */
    0, /* glVertexAttrib3dvNV */
    0, /* glVertexAttrib3fNV */
    0, /* glVertexAttrib3fvNV */
    0, /* glVertexAttrib3sNV */
    0, /* glVertexAttrib3svNV */
    0, /* glVertexAttrib4dNV */
    0, /* glVertexAttrib4dvNV */
    0, /* glVertexAttrib4fNV */
    0, /* glVertexAttrib4fvNV */
    0, /* glVertexAttrib4sNV */
    0, /* glVertexAttrib4svNV */
    0, /* glVertexAttrib4ubNV */
    0, /* glVertexAttrib4ubvNV */
    0, /* glGenFragmentShadersATI */
    0, /* glBindFragmentShaderATI */
    0, /* glDeleteFragmentShaderATI */
    0, /* glBeginFragmentShaderATI */
    0, /* glEndFragmentShaderATI */
    0, /* glPassTexCoordATI */
    0, /* glSampleMapATI */
    0, /* glColorFragmentOp1ATI */
    0, /* glColorFragmentOp2ATI */
    0, /* glColorFragmentOp3ATI */
    0, /* glAlphaFragmentOp1ATI */
    0, /* glAlphaFragmentOp2ATI */
    0, /* glAlphaFragmentOp3ATI */
    0, /* glSetFragmentShaderConstantATI */
    xglNoOpIsRenderbufferEXT,
    xglNoOpBindRenderbufferEXT,
    xglNoOpDeleteRenderbuffersEXT,
    xglNoOpGenRenderbuffersEXT,
    xglNoOpRenderbufferStorageEXT,
    xglNoOpGetRenderbufferParameterivEXT,
    xglNoOpIsFramebufferEXT,
    xglNoOpBindFramebufferEXT,
    xglNoOpDeleteFramebuffersEXT,
    xglNoOpGenFramebuffersEXT,
    xglNoOpCheckFramebufferStatusEXT,
    xglNoOpFramebufferTexture1DEXT,
    xglNoOpFramebufferTexture2DEXT,
    xglNoOpFramebufferTexture3DEXT,
    xglNoOpFramebufferRenderbufferEXT,
    xglNoOpGetFramebufferAttachmentParameterivEXT,
    xglNoOpGenerateMipmapEXT,
    0, /* glStencilFuncSeparate */
    0, /* glStencilOpSeparate */
    0, /* glStencilMaskSeparate */
    0, /* glGetQueryObjecti64vEXT */
    0  /* glGetQueryObjectui64vEXT */
};

static void
xglInitExtensions (xglGLContextPtr pContext)
{
    const char *extensions;

    extensions = (const char *) glGetString (GL_EXTENSIONS);

    if (strstr (extensions, "GL_ARB_multitexture"))
    {
	pContext->ActiveTextureARB =
	    (PFNGLACTIVETEXTUREARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glActiveTextureARB");
	pContext->glRenderTable.ClientActiveTextureARB =
	    (PFNGLCLIENTACTIVETEXTUREARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glClientActiveTextureARB");
	pContext->glRenderTable.MultiTexCoord1dvARB =
	    (PFNGLMULTITEXCOORD1DVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord1dvARB");
	pContext->glRenderTable.MultiTexCoord1fvARB =
	    (PFNGLMULTITEXCOORD1FVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord1fvARB");
	pContext->glRenderTable.MultiTexCoord1ivARB =
	    (PFNGLMULTITEXCOORD1IVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord1ivARB");
	pContext->glRenderTable.MultiTexCoord1svARB =
	    (PFNGLMULTITEXCOORD1SVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord1svARB");
	pContext->glRenderTable.MultiTexCoord2dvARB =
	    (PFNGLMULTITEXCOORD2DVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord2dvARB");
	pContext->glRenderTable.MultiTexCoord2fvARB =
	    (PFNGLMULTITEXCOORD2FVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord2fvARB");
	pContext->glRenderTable.MultiTexCoord2ivARB =
	    (PFNGLMULTITEXCOORD2IVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord2ivARB");
	pContext->glRenderTable.MultiTexCoord2svARB =
	    (PFNGLMULTITEXCOORD2SVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord2svARB");
	pContext->glRenderTable.MultiTexCoord3dvARB =
	    (PFNGLMULTITEXCOORD3DVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord3dvARB");
	pContext->glRenderTable.MultiTexCoord3fvARB =
	    (PFNGLMULTITEXCOORD3FVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord3fvARB");
	pContext->glRenderTable.MultiTexCoord3ivARB =
	    (PFNGLMULTITEXCOORD3IVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord3ivARB");
	pContext->glRenderTable.MultiTexCoord3svARB =
	    (PFNGLMULTITEXCOORD3SVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord3svARB");
	pContext->glRenderTable.MultiTexCoord4dvARB =
	    (PFNGLMULTITEXCOORD4DVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord4dvARB");
	pContext->glRenderTable.MultiTexCoord4fvARB =
	    (PFNGLMULTITEXCOORD4FVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord4fvARB");
	pContext->glRenderTable.MultiTexCoord4ivARB =
	    (PFNGLMULTITEXCOORD4IVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord4ivARB");
	pContext->glRenderTable.MultiTexCoord4svARB =
	    (PFNGLMULTITEXCOORD4SVARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glMultiTexCoord4svARB");

	glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &pContext->maxTexUnits);
	if (pContext->maxTexUnits > XGL_MAX_TEXTURE_UNITS)
	    pContext->maxTexUnits = XGL_MAX_TEXTURE_UNITS;

	pContext->glRenderTable.ActiveTextureARB = xglActiveTextureARB;
    }
    else
	pContext->maxTexUnits = 1;

    if (strstr (extensions, "GL_ARB_multisample"))
    {
	pContext->glRenderTable.SampleCoverageARB =
	    (PFNGLSAMPLECOVERAGEARBPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSampleCoverageARB");
    }

    if (strstr (extensions, "GL_EXT_texture_object"))
    {
	pContext->glRenderTable.AreTexturesResidentEXT =
	    xglAreTexturesResident;
	pContext->glRenderTable.GenTexturesEXT = xglGenTextures;
	pContext->glRenderTable.IsTextureEXT = xglIsTexture;
    }

    if (strstr (extensions, "GL_SGIS_multisample"))
    {
	pContext->glRenderTable.SampleMaskSGIS =
	    (PFNGLSAMPLEMASKSGISPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSampleMaskSGIS");
	pContext->glRenderTable.SamplePatternSGIS =
	    (PFNGLSAMPLEPATTERNSGISPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSamplePatternSGIS");
    }

    if (strstr (extensions, "GL_EXT_point_parameters"))
    {
	pContext->glRenderTable.PointParameterfEXT =
	    (PFNGLPOINTPARAMETERFEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glPointParameterfEXT");
	pContext->glRenderTable.PointParameterfvEXT =
	    (PFNGLPOINTPARAMETERFVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glPointParameterfvEXT");
    }

    if (strstr (extensions, "GL_MESA_window_pos"))
    {
	pContext->WindowPos3fMESA =
	    (PFNGLWINDOWPOS3FMESAPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glWindowPos3fMESA");

	pContext->glRenderTable.WindowPos3fMESA = xglWindowPos3fMESA;
    }

    if (strstr (extensions, "GL_EXT_blend_func_separate"))
    {
	pContext->glRenderTable.BlendFuncSeparateEXT =
	    (PFNGLBLENDFUNCSEPARATEEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glBlendFuncSeparateEXT");
    }

    if (strstr (extensions, "GL_EXT_fog_coord"))
    {
	pContext->glRenderTable.FogCoordfvEXT =
	    (PFNGLFOGCOORDFVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFogCoordfvEXT");
	pContext->glRenderTable.FogCoorddvEXT =
	    (PFNGLFOGCOORDDVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFogCoorddvEXT");
	pContext->glRenderTable.FogCoordPointerEXT =
	    (PFNGLFOGCOORDPOINTEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFogCoordPointerEXT");
    }

    if (strstr (extensions, "GL_EXT_secondary_color"))
    {
	pContext->glRenderTable.SecondaryColor3bvEXT =
	    (PFNGLSECONDARYCOLOR3BVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3bvEXT");
	pContext->glRenderTable.SecondaryColor3dvEXT =
	    (PFNGLSECONDARYCOLOR3DVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3dvEXT");
	pContext->glRenderTable.SecondaryColor3fvEXT =
	    (PFNGLSECONDARYCOLOR3FVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3fvEXT");
	pContext->glRenderTable.SecondaryColor3ivEXT =
	    (PFNGLSECONDARYCOLOR3IVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3ivEXT");
	pContext->glRenderTable.SecondaryColor3svEXT =
	    (PFNGLSECONDARYCOLOR3SVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3svEXT");
	pContext->glRenderTable.SecondaryColor3ubvEXT =
	    (PFNGLSECONDARYCOLOR3UBVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3ubvEXT");
	pContext->glRenderTable.SecondaryColor3uivEXT =
	    (PFNGLSECONDARYCOLOR3UIVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3uivEXT");
	pContext->glRenderTable.SecondaryColor3usvEXT =
	    (PFNGLSECONDARYCOLOR3USVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColor3usvEXT");
	pContext->glRenderTable.SecondaryColorPointerEXT =
	    (PFNGLSECONDARYCOLORPOINTEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glSecondaryColorPointerEXT");
    }

    if (strstr (extensions, "GL_NV_point_sprite"))
    {
	pContext->glRenderTable.PointParameteriNV =
	    (PFNGLPOINTPARAMETERINVPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glPointParameteriNV");
	pContext->glRenderTable.PointParameterivNV =
	    (PFNGLPOINTPARAMETERIVNVPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glPointParameterivNV");
    }

    if (strstr (extensions, "GL_EXT_stencil_two_side"))
    {
	pContext->glRenderTable.ActiveStencilFaceEXT =
	    (PFNGLACTIVESTENCILFACEEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glActiveStencilFaceEXT");
    }

    if (strstr (extensions, "GL_EXT_framebuffer_object"))
    {
	pContext->glRenderTable.IsRenderbufferEXT =
	    (PFNGLISRENDERBUFFEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glIsRenderbufferEXT");
	pContext->glRenderTable.BindRenderbufferEXT =
	    (PFNGLBINDRENDERBUFFEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glBindRenderbufferEXT");
	pContext->glRenderTable.DeleteRenderbuffersEXT =
	    (PFNGLDELETERENDERBUFFERSEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glDeleteRenderbuffersEXT");
	pContext->glRenderTable.GenRenderbuffersEXT =
	    (PFNGLGENRENDERBUFFERSEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glGenRenderbuffersEXT");
	pContext->glRenderTable.RenderbufferStorageEXT =
	    (PFNGLRENDERBUFFERSTORAGEEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glRenderbufferStorageEXT");
	pContext->glRenderTable.GetRenderbufferParameterivEXT =
	    (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glGetRenderbufferParameterivEXT");
	pContext->glRenderTable.IsFramebufferEXT =
	    (PFNGLISFRAMEBUFFEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glIsFramebufferEXT");
	pContext->glRenderTable.BindFramebufferEXT =
	    (PFNGLBINDFRAMEBUFFEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glBindFramebufferEXT");
	pContext->glRenderTable.DeleteFramebuffersEXT =
	    (PFNGLDELETEFRAMEBUFFERSEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glDeleteFramebuffersEXT");
	pContext->glRenderTable.GenFramebuffersEXT =
	    (PFNGLGENFRAMEBUFFERSEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glGenFramebuffersEXT");
	pContext->glRenderTable.CheckFramebufferStatusEXT =
	    (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glCheckFramebufferStatusEXT");
	pContext->glRenderTable.FramebufferTexture1DEXT =
	    (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFramebufferTexture1DEXT");
	pContext->glRenderTable.FramebufferTexture2DEXT =
	    (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFramebufferTexture2DEXT");
	pContext->glRenderTable.FramebufferTexture3DEXT =
	    (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFramebufferTexture3DEXT");
	pContext->glRenderTable.FramebufferRenderbufferEXT =
	    (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glFramebufferRenderbufferEXT");
	pContext->glRenderTable.GetFramebufferAttachmentParameterivEXT =
	    (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glGetFramebufferAttachment"
					    "ParameterivEXT");
	pContext->glRenderTable.GenerateMipmapEXT =
	    (PFNGLGENERATEMIPMAPEXTPROC)
	    glitz_context_get_proc_address (pContext->context,
					    "glGenerateMipmapEXT");
    }
}

static void
xglSetCurrentContext (xglGLContextPtr pContext)
{
    cctx = pContext;

    glitz_context_make_current (cctx->context, cctx->pDrawBuffer->drawable);

    GlxSetRenderTables (&cctx->glRenderTable);
}

static void
xglFreeContext (xglGLContextPtr pContext)
{
    int i;

    pContext->refcnt--;
    if (pContext->shared == pContext)
	pContext->refcnt--;

    if (pContext->refcnt)
	return;

    if (pContext->shared != pContext)
	xglFreeContext (pContext->shared);

    if (pContext->texObjects)
    {
	xglTexObjPtr pTexObj;
	GLuint	     key;

	do {
	    key = xglHashFirstEntry (pContext->texObjects);
	    if (key)
	    {
		pTexObj = (xglTexObjPtr) xglHashLookup (pContext->texObjects,
							key);
		if (pTexObj)
		    xglUnrefTexObj (pTexObj);

		xglHashRemove (pContext->texObjects, key);
	    }
	} while (key);

	xglDeleteHashTable (pContext->texObjects);
    }

    if (pContext->displayLists)
    {
	xglDisplayListPtr pDisplayList;
	GLuint		  key;

	do {
	    key = xglHashFirstEntry (pContext->displayLists);
	    if (key)
	    {
		pDisplayList = (xglDisplayListPtr)
		    xglHashLookup (pContext->displayLists, key);
		if (pDisplayList)
		    xglDestroyList (pDisplayList);

		xglHashRemove (pContext->displayLists, key);
	    }
	} while (key);

	xglDeleteHashTable (pContext->displayLists);
    }

    for (i = 0; i < pContext->maxTexUnits; i++)
    {
	xglUnrefTexObj (pContext->attrib.texUnits[i].p1D);
	xglUnrefTexObj (pContext->attrib.texUnits[i].p2D);
	xglUnrefTexObj (pContext->attrib.texUnits[i].p3D);
	xglUnrefTexObj (pContext->attrib.texUnits[i].pRect);
	xglUnrefTexObj (pContext->attrib.texUnits[i].pCubeMap);
    }

    if (pContext->groupList)
	glDeleteLists (pContext->groupList, 1);

    if (pContext->context)
	glitz_context_destroy (pContext->context);

    if (pContext->versionString)
	xfree (pContext->versionString);

    xfree (pContext);
}

static GLboolean
xglDestroyContext (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    xglFreeContext (pContext);

    if (!iface)
	return GL_TRUE;

    return (*iface->exports.destroyContext) ((__GLcontext *) iface);
}

static GLboolean
xglLoseCurrent (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    GlxFlushContextCache ();
    GlxSetRenderTables (0);

    if (!iface)
	return GL_TRUE;

    return (*iface->exports.loseCurrent) ((__GLcontext *) iface);
}

static GLboolean
xglMakeCurrent (__GLcontext *gc)
{
    xglGLContextPtr	pContext = (xglGLContextPtr) gc;
    __GLinterface	*iface = &pContext->iface;
    __GLinterface	*mIface = pContext->mIface;
    __GLdrawablePrivate *drawPriv = iface->imports.getDrawablePrivate (gc);
    __GLdrawablePrivate *readPriv = iface->imports.getReadablePrivate (gc);
    xglGLBufferPtr	pDrawBufferPriv = drawPriv->private;
    xglGLBufferPtr	pReadBufferPriv = readPriv->private;
    GLboolean		status = GL_TRUE;

    if (pReadBufferPriv->pDrawable && pDrawBufferPriv->pDrawable)
    {
	XID values[2] = { ClipByChildren, 0 };
	int status;

#ifdef COMPOSITE
	/* XXX: temporary hack for root window drawing using
	   IncludeInferiors */
	if (pDrawBufferPriv->pDrawable->type == DRAWABLE_WINDOW &&
	    (!((WindowPtr) (pDrawBufferPriv->pDrawable))->parent))
	    values[0] = IncludeInferiors;
#endif

	/* this happens if client previously used this context with a buffer
	   not supported by the native GL stack */
	if (!pContext->context)
	    return GL_FALSE;

	/* XXX: GLX_SGI_make_current_read disabled for now */
	if (pDrawBufferPriv != pReadBufferPriv)
	    return GL_FALSE;

	if (!pReadBufferPriv->pGC)
	    pReadBufferPriv->pGC =
		CreateGC (pReadBufferPriv->pDrawable,
			  GCSubwindowMode | GCGraphicsExposures, values,
			  &status);

	ValidateGC (pReadBufferPriv->pDrawable, pReadBufferPriv->pGC);

	if (!pDrawBufferPriv->pGC)
	    pDrawBufferPriv->pGC =
		CreateGC (pDrawBufferPriv->pDrawable,
			  GCSubwindowMode | GCGraphicsExposures, values,
			  &status);

	ValidateGC (pDrawBufferPriv->pDrawable, pDrawBufferPriv->pGC);

	pReadBufferPriv->pPixmap = (PixmapPtr) 0;
	pDrawBufferPriv->pPixmap = (PixmapPtr) 0;

	pContext->pReadBuffer = pReadBufferPriv;
	pContext->pDrawBuffer = pDrawBufferPriv;

	pContext->readPriv = readPriv;
	pContext->drawPriv = drawPriv;

	/* from now on this context can only be used with native GL stack */
	if (mIface)
	{
	    (*mIface->exports.destroyContext) ((__GLcontext *) mIface);
	    pContext->mIface = NULL;
	}
    }
    else
    {
	/* this happens if client previously used this context with a buffer
	   supported by the native GL stack */
	if (!mIface)
	    return GL_FALSE;

	drawPriv->private = pDrawBufferPriv->private;
	readPriv->private = pReadBufferPriv->private;

	status = (*mIface->exports.makeCurrent) ((__GLcontext *) mIface);

	drawPriv->private = pDrawBufferPriv;
	readPriv->private = pReadBufferPriv;

	/* from now on this context can not be used with native GL stack */
	if (status == GL_TRUE && pContext->context)
	{
	    glitz_context_destroy (pContext->context);
	    pContext->context = NULL;
	}
    }

    return status;
}

static GLboolean
xglShareContext (__GLcontext *gc,
		 __GLcontext *gcShare)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    xglGLContextPtr pContextShare = (xglGLContextPtr) gcShare;
    __GLinterface   *iface = pContext->mIface;
    __GLinterface   *ifaceShare = pContextShare->mIface;

    if (!iface || !ifaceShare)
	return GL_TRUE;

    return (*iface->exports.shareContext) ((__GLcontext *) iface,
					   (__GLcontext *) ifaceShare);
}

static GLboolean
xglCopyContext (__GLcontext	  *dst,
		const __GLcontext *src,
		GLuint		  mask)
{
    xglGLContextPtr   pDst = (xglGLContextPtr) dst;
    xglGLContextPtr   pSrc = (xglGLContextPtr) src;
    const __GLcontext *srcCtx = (const __GLcontext *) pSrc->mIface;
    __GLinterface     *dstIface = (__GLinterface *) pDst->mIface;
    GLboolean	      status = GL_TRUE;

    if (pSrc->context && pDst->context)
	glitz_context_copy (pSrc->context, pDst->context, mask);
    else
	status = GL_FALSE;

    if (dstIface && srcCtx)
	status = (*dstIface->exports.copyContext) ((__GLcontext *) dstIface,
						   srcCtx,
						   mask);

    return status;
}

static Bool
xglResizeBuffer (__GLdrawablePrivate *glPriv,
		 int		      x,
		 int		      y,
		 unsigned int	      width,
		 unsigned int	      height)
{
    xglGLBufferPtr pBufferPriv = glPriv->private;
    DrawablePtr    pDrawable = pBufferPriv->pDrawable;

    XGL_SCREEN_PRIV (pDrawable->pScreen);
    XGL_DRAWABLE_PIXMAP (pBufferPriv->pDrawable);

    if (pPixmap != pScreenPriv->pScreenPixmap)
    {
	if (!xglCreatePixmapSurface (pPixmap))
	    return FALSE;

	if (pBufferPriv->drawable == pScreenPriv->drawable)
	{
	    if (pBufferPriv->backSurface)
		glitz_surface_destroy (pBufferPriv->backSurface);

	    glitz_drawable_destroy (pBufferPriv->drawable);

	    pBufferPriv->drawable    = NULL;
	    pBufferPriv->backSurface = NULL;
	}

	if (pBufferPriv->drawable)
	{
	    glitz_drawable_update_size (pBufferPriv->drawable,
					pPixmap->drawable.width,
					pPixmap->drawable.height);
	}
	else
	{
	    glitz_drawable_format_t *format;

	    format = pBufferPriv->pVisual->format.drawable;
	    if (pBufferPriv->pVisual->pbuffer)
	    {
		pBufferPriv->drawable =
		    glitz_create_pbuffer_drawable (pScreenPriv->drawable,
						   format,
						   pPixmap->drawable.width,
						   pPixmap->drawable.height);
	    }
	    else
	    {
		pBufferPriv->drawable =
		    glitz_create_drawable (pScreenPriv->drawable, format,
					   pPixmap->drawable.width,
					   pPixmap->drawable.height);

		if (!pBufferPriv->drawable)
		    return FALSE;

		if (format->doublebuffer)
		{
		    glitz_format_t *backFormat;

		    backFormat = pBufferPriv->pVisual->format.surface;

		    pBufferPriv->backSurface =
			glitz_surface_create (pScreenPriv->drawable, backFormat,
					      pPixmap->drawable.width,
					      pPixmap->drawable.height,
					      0, NULL);
		    if (pBufferPriv->backSurface)
			glitz_surface_attach (pBufferPriv->backSurface,
					      pBufferPriv->drawable,
					      GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
		}
	    }
	}
    }
    else
    {
	glitz_drawable_reference (pScreenPriv->drawable);

	if (pBufferPriv->backSurface)
	    glitz_surface_destroy (pBufferPriv->backSurface);

	if (pBufferPriv->drawable)
	    glitz_drawable_destroy (pBufferPriv->drawable);

	pBufferPriv->drawable    = pScreenPriv->drawable;
	pBufferPriv->backSurface = NULL;
    }

    ValidateGC (pDrawable, pBufferPriv->pGC);

    return TRUE;
}

static GLboolean
xglForceCurrent (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;
    GLboolean	    status = GL_TRUE;

    if (pContext && pContext->context)
    {
	__GLdrawablePrivate *readPriv, *drawPriv;

	readPriv = pContext->readPriv;
	drawPriv = pContext->drawPriv;

	drawPriv->lockDP (drawPriv, gc);
	if (readPriv != drawPriv)
	    readPriv->lockDP (readPriv, gc);

	cctx = pContext;

	if (cctx->pReadBuffer->pDrawable && cctx->pDrawBuffer->pDrawable)
	{
	    DrawablePtr pDrawable = cctx->pReadBuffer->pDrawable;
	    PixmapPtr   pReadPixmap, pDrawPixmap;

	    XGL_SCREEN_PRIV (pDrawable->pScreen);

	    if (pDrawable->type != DRAWABLE_PIXMAP)
	    {
		pReadPixmap = XGL_GET_WINDOW_PIXMAP (pDrawable);
		cctx->pReadBuffer->screenX = __XGL_OFF_X_WIN (pReadPixmap);
		cctx->pReadBuffer->screenY = __XGL_OFF_Y_WIN (pReadPixmap);
		cctx->pReadBuffer->xOff = pDrawable->x +
		    __XGL_OFF_X_WIN (pReadPixmap);
		cctx->pReadBuffer->yOff = pReadPixmap->drawable.height -
		    ((pDrawable->y + __XGL_OFF_Y_WIN (pReadPixmap)) +
		     pDrawable->height);
		cctx->pReadBuffer->yFlip = pReadPixmap->drawable.height;
	    }
	    else
	    {
		pReadPixmap = (PixmapPtr) pDrawable;
		cctx->pReadBuffer->screenX = cctx->pReadBuffer->screenY = 0;
		cctx->pReadBuffer->xOff = cctx->pReadBuffer->yOff = 0;
		cctx->pReadBuffer->yFlip = pDrawable->height;
	    }

	    pDrawable = cctx->pDrawBuffer->pDrawable;
	    if (pDrawable->type != DRAWABLE_PIXMAP)
	    {
		pDrawPixmap = XGL_GET_WINDOW_PIXMAP (pDrawable);
		cctx->pDrawBuffer->screenX = __XGL_OFF_X_WIN (pDrawPixmap);
		cctx->pDrawBuffer->screenY = __XGL_OFF_Y_WIN (pDrawPixmap);
		cctx->pDrawBuffer->xOff = pDrawable->x +
		    __XGL_OFF_X_WIN (pDrawPixmap);
		cctx->pDrawBuffer->yOff = pDrawPixmap->drawable.height -
		    ((pDrawable->y + __XGL_OFF_Y_WIN (pDrawPixmap)) +
		     pDrawable->height);
		cctx->pDrawBuffer->yFlip = pDrawPixmap->drawable.height;
	    }
	    else
	    {
		pDrawPixmap = (PixmapPtr) pDrawable;
		cctx->pDrawBuffer->screenX = cctx->pDrawBuffer->screenY = 0;
		cctx->pDrawBuffer->xOff = cctx->pDrawBuffer->yOff = 0;
		cctx->pDrawBuffer->yFlip = pDrawable->height;
	    }

	    /* buffer changed */
	    if (cctx->pDrawBuffer->pPixmap != pDrawPixmap ||
		cctx->pReadBuffer->pPixmap != pReadPixmap)
	    {
		if (!xglResizeBuffer (drawPriv,
				      pDrawable->x,
				      pDrawable->y,
				      pDrawable->width,
				      pDrawable->height))
		{
		    drawPriv->unlockDP (drawPriv);
		    if (readPriv != drawPriv)
			readPriv->unlockDP (readPriv);

		    return FALSE;
		}

		if (!xglResizeBuffer (readPriv,
				      cctx->pReadBuffer->pDrawable->x,
				      cctx->pReadBuffer->pDrawable->y,
				      cctx->pReadBuffer->pDrawable->width,
				      cctx->pReadBuffer->pDrawable->height))
		{
		    drawPriv->unlockDP (drawPriv);
		    if (readPriv != drawPriv)
			readPriv->unlockDP (readPriv);

		    return FALSE;
		}

		cctx->pReadBuffer->pPixmap = pReadPixmap;
		cctx->pDrawBuffer->pPixmap = pDrawPixmap;
	    }

	    if (!xglSyncSurface (pContext->pDrawBuffer->pDrawable))
	    {
		drawPriv->unlockDP (drawPriv);
		if (readPriv != drawPriv)
		    readPriv->unlockDP (readPriv);

		return FALSE;
	    }

	    if (pDrawPixmap != pScreenPriv->pScreenPixmap)
	    {
		XGL_PIXMAP_PRIV (pDrawPixmap);

		glitz_surface_attach (pPixmapPriv->surface,
				      pContext->pDrawBuffer->drawable,
				      GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

		if (pPixmapPriv->target)
		    pPixmapPriv->target = xglPixmapTargetOut;
	    }

	    xglSetCurrentContext (pContext);

	    if (cctx->needInit)
	    {
		int i;

		xglInitExtensions (cctx);

		glGetIntegerv (GL_MAX_LIST_NESTING, &cctx->maxListNesting);
		glGetIntegerv (GL_MAX_ATTRIB_STACK_DEPTH,
			       &cctx->maxAttribStackDepth);
		if (cctx->maxAttribStackDepth > XGL_MAX_ATTRIB_STACK_DEPTH)
		    cctx->maxAttribStackDepth = XGL_MAX_ATTRIB_STACK_DEPTH;

		cctx->attrib.scissorTest = GL_FALSE;
		cctx->attrib.scissor.x = cctx->attrib.scissor.y = 0;
		cctx->attrib.scissor.width =
		    cctx->pDrawBuffer->pDrawable->width;
		cctx->attrib.scissor.height =
		    cctx->pDrawBuffer->pDrawable->height;
		cctx->attrib.viewport = cctx->attrib.scissor;

		cctx->activeTexUnit = 0;

		for (i = 0; i < cctx->maxTexUnits; i++)
		{
		    cctx->attrib.texUnits[i].enabled = 0;

		    cctx->attrib.texUnits[i].p1D      = NULL;
		    cctx->attrib.texUnits[i].p2D      = NULL;
		    cctx->attrib.texUnits[i].p3D      = NULL;
		    cctx->attrib.texUnits[i].pRect    = NULL;
		    cctx->attrib.texUnits[i].pCubeMap = NULL;
		}

		glEnable (GL_SCISSOR_TEST);

		cctx->needInit = FALSE;
	    }

	    /* update viewport and raster position */
	    if (cctx->pDrawBuffer->xOff != cctx->drawXoff ||
		cctx->pDrawBuffer->yOff != cctx->drawYoff)
	    {
		glViewport (cctx->attrib.viewport.x + cctx->pDrawBuffer->xOff,
			    cctx->attrib.viewport.y + cctx->pDrawBuffer->yOff,
			    cctx->attrib.viewport.width,
			    cctx->attrib.viewport.height);

		glBitmap (0, 0, 0, 0,
			  cctx->pDrawBuffer->xOff - cctx->drawXoff,
			  cctx->pDrawBuffer->yOff - cctx->drawYoff,
			  NULL);

		cctx->drawXoff = cctx->pDrawBuffer->xOff;
		cctx->drawYoff = cctx->pDrawBuffer->yOff;
	    }

	    xglDrawBuffer (cctx->attrib.drawBuffer);
	    xglReadBuffer (cctx->attrib.readBuffer);
	}
	else
	{
	    xglSetCurrentContext (pContext);
	}

	drawPriv->unlockDP (drawPriv);
	if (readPriv != drawPriv)
	    readPriv->unlockDP (readPriv);
    }
    else
    {
	cctx = NULL;
	status = (*iface->exports.forceCurrent) ((__GLcontext *) iface);
    }

    return status;
}

static GLboolean
xglNotifyResize (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    if (!iface)
	return GL_TRUE;

    return (*iface->exports.notifyResize) ((__GLcontext *) iface);
}

static void
xglNotifyDestroy (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    pContext->pReadBuffer->pDrawable = 0;
    pContext->pDrawBuffer->pDrawable = 0;

    if (iface)
	(*iface->exports.notifyDestroy) ((__GLcontext *) iface);
}

static void
xglNotifySwapBuffers (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    if (iface)
	(*iface->exports.notifySwapBuffers) ((__GLcontext *) iface);
}

static struct __GLdispatchStateRec *
xglDispatchExec (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    if (!iface)
	return NULL;

    return (*iface->exports.dispatchExec) ((__GLcontext *) iface);
}

static void
xglBeginDispatchOverride (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    if (iface)
	(*iface->exports.beginDispatchOverride) ((__GLcontext *) iface);
}

static void
xglEndDispatchOverride (__GLcontext *gc)
{
    xglGLContextPtr pContext = (xglGLContextPtr) gc;
    __GLinterface   *iface = pContext->mIface;

    if (iface)
	(*iface->exports.endDispatchOverride) ((__GLcontext *) iface);
}

static void
xglLoseCurrentContext (void *closure)
{
    if (closure == cctx)
    {
	cctx = NULL;

	GlxFlushContextCache ();
	GlxSetRenderTables (0);
    }
}

static __GLinterface *
xglCreateContext (__GLimports      *imports,
		  __GLcontextModes *modes,
		  __GLinterface    *shareGC)
{
    glitz_drawable_format_t *format;
    xglGLContextPtr	    pShareContext = (xglGLContextPtr) shareGC;
    xglGLContextPtr	    pContext;
    __GLinterface	    *shareIface = NULL;
    __GLinterface	    *iface;
    __GLXcontext	    *glxCtx = (__GLXcontext *) imports->other;

    XGL_SCREEN_PRIV (glxCtx->pScreen);

    pContext = xalloc (sizeof (xglGLContextRec));
    if (!pContext)
	return NULL;

    format = glitz_drawable_get_format (pScreenPriv->drawable);
    pContext->context = glitz_context_create (pScreenPriv->drawable, format);
    glitz_context_set_user_data (pContext->context, pContext,
				 xglLoseCurrentContext);

    pContext->glRenderTable = __glNativeRenderTable;
    pContext->needInit	    = TRUE;
    pContext->versionString = NULL;
    pContext->errorValue    = GL_NO_ERROR;
    pContext->shared	    = NULL;
    pContext->list	    = 0;
    pContext->groupList	    = 0;
    pContext->beginCnt	    = 0;
    pContext->nAttribStack  = 0;
    pContext->refcnt	    = 1;
    pContext->doubleBuffer  = glxCtx->modes->doubleBufferMode;
    pContext->depthBits     = glxCtx->modes->depthBits;
    pContext->stencilBits   = glxCtx->modes->stencilBits;
    pContext->drawXoff	    = 0;
    pContext->drawYoff	    = 0;
    pContext->maxTexUnits   = 0;

    if (pContext->doubleBuffer)
    {
	pContext->attrib.drawBuffer = GL_BACK;
	pContext->attrib.readBuffer = GL_BACK;
    }
    else
    {
	pContext->attrib.drawBuffer = GL_FRONT;
	pContext->attrib.readBuffer = GL_FRONT;
    }

    pContext->attrib.scissorTest = GL_FALSE;

    if (shareGC)
    {
	pContext->texObjects   = NULL;
	pContext->displayLists = NULL;

	pContext->shared = pShareContext->shared;
	shareIface = pShareContext->mIface;
    }
    else
    {
	pContext->texObjects = xglNewHashTable ();
	if (!pContext->texObjects)
	{
	    xglFreeContext (pContext);
	    return NULL;
	}

	pContext->displayLists = xglNewHashTable ();
	if (!pContext->displayLists)
	{
	    xglFreeContext (pContext);
	    return NULL;
	}

	pContext->shared = pContext;
    }

    pContext->shared->refcnt++;

    iface = (*screenInfoPriv.createContext) (imports, modes, shareIface);
    if (!iface)
    {
	xglFreeContext (pContext);
	return NULL;
    }

    pContext->mIface = iface;
    pContext->iface.imports = *imports;

    pContext->iface.exports.destroyContext	  = xglDestroyContext;
    pContext->iface.exports.loseCurrent		  = xglLoseCurrent;
    pContext->iface.exports.makeCurrent		  = xglMakeCurrent;
    pContext->iface.exports.shareContext	  = xglShareContext;
    pContext->iface.exports.copyContext		  = xglCopyContext;
    pContext->iface.exports.forceCurrent	  = xglForceCurrent;
    pContext->iface.exports.notifyResize	  = xglNotifyResize;
    pContext->iface.exports.notifyDestroy	  = xglNotifyDestroy;
    pContext->iface.exports.notifySwapBuffers     = xglNotifySwapBuffers;
    pContext->iface.exports.dispatchExec	  = xglDispatchExec;
    pContext->iface.exports.beginDispatchOverride = xglBeginDispatchOverride;
    pContext->iface.exports.endDispatchOverride   = xglEndDispatchOverride;

    return (__GLinterface *) pContext;
}

static GLboolean
xglSwapBuffers (__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate	*glPriv = &glxPriv->glPriv;
    xglGLBufferPtr	pBufferPriv = glPriv->private;
    DrawablePtr		pDrawable = pBufferPriv->pDrawable;
    GLboolean		status = GL_TRUE;

    if (pDrawable)
    {
	if (glPriv->modes->doubleBufferMode)
	{
	    glitz_surface_t *surface;
	    int		    xOff, yOff;
	    GCPtr	    pGC = pBufferPriv->pGC;
	    BoxPtr	    pBox = REGION_RECTS (pGC->pCompositeClip);
	    int		    nBox = REGION_NUM_RECTS (pGC->pCompositeClip);

	    XGL_GET_DRAWABLE (pDrawable, surface, xOff, yOff);

	    glitz_drawable_swap_buffer_region (pBufferPriv->drawable,
					       xOff, yOff,
					       (glitz_box_t *) pBox, nBox);

	    xglAddBitDamage (pDrawable, pGC->pCompositeClip);
	    DamageDamageRegion (pDrawable, pGC->pCompositeClip);
	    REGION_EMPTY (pGC->pScreen, &pBufferPriv->damage);
	}
    }
    else if (pBufferPriv->private)
    {
	glPriv->private = pBufferPriv->private;
	status = (*pBufferPriv->swapBuffers) (glxPriv);
	glPriv->private = pBufferPriv;
    }

    return status;
}

static GLboolean
xglResizeBuffers (__GLdrawableBuffer  *buffer,
		  GLint		      x,
		  GLint		      y,
		  GLuint	      width,
		  GLuint	      height,
		  __GLdrawablePrivate *glPriv,
		  GLuint	      bufferMask)
{
    xglGLBufferPtr pBufferPriv = glPriv->private;
    DrawablePtr    pDrawable = pBufferPriv->pDrawable;
    GLboolean	   status = GL_TRUE;

    if (pDrawable)
    {
	if (!xglResizeBuffer (glPriv, x, y, width, height))
	    return GL_FALSE;
    }
    else if (pBufferPriv->private)
    {
	glPriv->private = pBufferPriv->private;
	status = (*pBufferPriv->resizeBuffers) (buffer,
						x, y, width, height,
						glPriv,
						bufferMask);
	glPriv->private = pBufferPriv;
    }

    return status;
}

static int
xglBindBuffers (__GLXdrawablePrivate *glxPriv,
		int		     buffer)
{
    __GLdrawablePrivate	*glPriv = &glxPriv->glPriv;
    xglGLBufferPtr	pBufferPriv = glPriv->private;

    if (cctx)
    {
	xglTexUnitPtr pTexUnit = &cctx->attrib.texUnits[cctx->activeTexUnit];
	xglTexObjPtr  pTexObj = NULL;
	DrawablePtr   pDrawable;

	/* XXX: front left buffer is only supported so far */
	if (buffer != GLX_FRONT_LEFT_EXT)
	    return BadMatch;

	/* Must be a GLXpixmap */
	if (!glxPriv->pGlxPixmap)
	    return BadDrawable;

	pDrawable = glxPriv->pGlxPixmap->pDraw;

	switch (glxPriv->texTarget) {
	case GLX_TEXTURE_RECTANGLE_EXT:
	    pTexObj = pTexUnit->pRect;
	    break;
	case GLX_TEXTURE_2D_EXT:
	    pTexObj = pTexUnit->p2D;
	    break;
	default:
	    break;
	}

	if (pTexObj)
	{
	    glitz_texture_object_t *object;

	    XGL_SCREEN_PRIV (pDrawable->pScreen);
	    XGL_DRAWABLE_PIXMAP (pDrawable);
	    XGL_PIXMAP_PRIV (pPixmap);

	    if (pPixmap == pScreenPriv->pScreenPixmap)
		return BadDrawable;

	    object = glitz_texture_object_create (pPixmapPriv->surface);
	    if (object)
	    {
		pPixmap->refcnt++;

		if (pTexObj->pPixmap)
		    (*pDrawable->pScreen->DestroyPixmap) (pTexObj->pPixmap);

		if (pTexObj->object)
		    glitz_texture_object_destroy (pTexObj->object);

		pTexObj->pPixmap = pPixmap;
		pTexObj->object  = object;

		return Success;
	    }
	}
    }
    else if (pBufferPriv->private)
    {
	int status;

	glPriv->private = pBufferPriv->private;
	status = (*pBufferPriv->bindBuffers) (glxPriv, buffer);
	glPriv->private = pBufferPriv;

	return status;
    }

    return BadDrawable;
}

static int
xglReleaseBuffers (__GLXdrawablePrivate *glxPriv,
		   int		        buffer)
{
    __GLdrawablePrivate	*glPriv = &glxPriv->glPriv;
    xglGLBufferPtr	pBufferPriv = glPriv->private;

    if (cctx)
    {
	xglTexObjPtr pTexObj;

	/* XXX: front left buffer is only supported so far */
	if (buffer != GLX_FRONT_LEFT_EXT)
	    return BadMatch;

	/* Must be a GLXpixmap */
	if (glxPriv->pGlxPixmap)
	{
	    DrawablePtr pDrawable = glxPriv->pGlxPixmap->pDraw;

	    XGL_DRAWABLE_PIXMAP (pDrawable);

	    pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].p2D;
	    if (pTexObj && pTexObj->pPixmap == pPixmap)
	    {
		(*pDrawable->pScreen->DestroyPixmap) (pTexObj->pPixmap);
		pTexObj->pPixmap = NULL;
		glitz_texture_object_destroy (pTexObj->object);
		pTexObj->object = NULL;

		return Success;
	    }
	    else
	    {
		pTexObj = cctx->attrib.texUnits[cctx->activeTexUnit].pRect;
		if (pTexObj && pTexObj->pPixmap == pPixmap)
		{
		    (*pDrawable->pScreen->DestroyPixmap) (pTexObj->pPixmap);
		    pTexObj->pPixmap = NULL;
		    glitz_texture_object_destroy (pTexObj->object);
		    pTexObj->object = NULL;

		    return Success;
		}
	    }
	}
    }
    else if (pBufferPriv->private)
    {
	int status;

	glPriv->private = pBufferPriv->private;
	status = (*pBufferPriv->releaseBuffers) (glxPriv, buffer);
	glPriv->private = pBufferPriv;

	return status;
    }

    return BadDrawable;
}
static void
xglFreeBuffers (__GLdrawablePrivate *glPriv)
{
    xglGLBufferPtr pBufferPriv = glPriv->private;

    glPriv->private = pBufferPriv->private;

    if (pBufferPriv->freeBuffers)
	(*pBufferPriv->freeBuffers) (glPriv);

    if (pBufferPriv->pGC)
	FreeGC (pBufferPriv->pGC, (GContext) 0);

    if (pBufferPriv->backSurface)
	glitz_surface_destroy (pBufferPriv->backSurface);

    if (pBufferPriv->drawable)
	glitz_drawable_destroy (pBufferPriv->drawable);

    xfree (pBufferPriv);
}

static void
xglCreateBuffer (__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate	*glPriv = &glxPriv->glPriv;
    DrawablePtr	        pDrawable = glxPriv->pDraw;
    ScreenPtr		pScreen = pDrawable->pScreen;
    xglGLBufferPtr	pBufferPriv;
    xglVisualPtr	v;

    XGL_SCREEN_PRIV (pScreen);
    XGL_DRAWABLE_PIXMAP (pDrawable);

    pBufferPriv = xalloc (sizeof (xglGLBufferRec));
    if (!pBufferPriv)
	FatalError ("xglCreateBuffer: No memory\n");

    pBufferPriv->pScreen   = pScreen;
    pBufferPriv->pDrawable = NULL;
    pBufferPriv->pPixmap   = NULL;
    pBufferPriv->pGC	   = NULL;

    pBufferPriv->swapBuffers = NULL;

    pBufferPriv->bindBuffers    = NULL;
    pBufferPriv->releaseBuffers = NULL;

    pBufferPriv->resizeBuffers = NULL;
    pBufferPriv->private       = NULL;
    pBufferPriv->freeBuffers   = NULL;

    pBufferPriv->drawable    = NULL;
    pBufferPriv->backSurface = NULL;

    REGION_INIT (pScreen, &pBufferPriv->damage, NullBox, 0);

    pBufferPriv->pVisual = 0;

    /* glx acceleration */
    if (pScreenPriv->accel.glx.enabled &&
	xglCheckPixmapSize (pPixmap, &pScreenPriv->accel.glx.size))
    {
	for (v = pScreenPriv->pGlxVisual; v; v = v->next)
	{
	    glitz_drawable_format_t *format;

	    if (pScreenPriv->accel.glx.pbuffer != v->pbuffer)
		continue;

	    format = v->format.drawable;
	    if (!format)
		continue;

	    if (format->color.red_size   != glxPriv->modes->redBits   ||
		format->color.green_size != glxPriv->modes->greenBits ||
		format->color.blue_size  != glxPriv->modes->blueBits)
		continue;

	    if (format->color.alpha_size < glxPriv->modes->alphaBits   ||
		format->depth_size	 < glxPriv->modes->depthBits   ||
		format->stencil_size     < glxPriv->modes->stencilBits ||
		format->doublebuffer     < glxPriv->modes->doubleBufferMode)
		continue;

	    /* this is good enought for pbuffers */
	    if (v->pbuffer)
		break;

	    /* we want an exact match for non-pbuffer formats */
	    if (format->color.alpha_size == glxPriv->modes->alphaBits   &&
		format->depth_size	 == glxPriv->modes->depthBits   &&
		format->stencil_size     == glxPriv->modes->stencilBits &&
		format->doublebuffer     == glxPriv->modes->doubleBufferMode)
		break;
	}

	pBufferPriv->pVisual = v;
    }

    if ((pDrawable->type == DRAWABLE_WINDOW)

#ifdef COMPOSITE
	&& (pBufferPriv->pVisual

	    /* this is a root window, can't be redirected */
	    || (!((WindowPtr) pDrawable)->parent))
#endif

	)
    {
	pBufferPriv->pDrawable = pDrawable;
    }
    else
    {
	(*screenInfoPriv.createBuffer) (glxPriv);

	/* Wrap the swap buffers routine */
	pBufferPriv->swapBuffers = glxPriv->swapBuffers;

	/* Wrap the render texture routines */
	pBufferPriv->bindBuffers    = glxPriv->bindBuffers;
	pBufferPriv->releaseBuffers = glxPriv->releaseBuffers;

	/* Wrap the front buffer's resize routine */
	pBufferPriv->resizeBuffers = glPriv->frontBuffer.resize;

	/* Save Xgl's private buffer structure */
	pBufferPriv->freeBuffers = glPriv->freePrivate;
	pBufferPriv->private	 = glPriv->private;
    }

    glxPriv->texTarget = GLX_NO_TEXTURE_EXT;

    /* We enable render texture for all GLXPixmaps right now. Eventually, this
       should only be enabled when fbconfig attribute GLX_RENDER_TEXTURE_RGB or
       GLX_RENDER_TEXTURE_RGBA is set to TRUE. */
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
	XGL_DRAWABLE_PIXMAP (pDrawable);

	if (xglCreatePixmapSurface (pPixmap))
	{
	    glitz_texture_object_t *texture;

	    XGL_PIXMAP_PRIV (pPixmap);

	    texture = glitz_texture_object_create (pPixmapPriv->surface);
	    if (texture)
	    {
		switch (glitz_texture_object_get_target (texture)) {
		case GLITZ_TEXTURE_TARGET_2D:
		    glxPriv->texTarget = GLX_TEXTURE_2D_EXT;
		    break;
		case GLITZ_TEXTURE_TARGET_RECT:
		    glxPriv->texTarget = GLX_TEXTURE_RECTANGLE_EXT;
		    break;
		}

		glitz_texture_object_destroy (texture);
	    }
	}
    }

    glxPriv->swapBuffers = xglSwapBuffers;

    glxPriv->bindBuffers    = xglBindBuffers;
    glxPriv->releaseBuffers = xglReleaseBuffers;
    glPriv->frontBuffer.resize = xglResizeBuffers;

    glPriv->private	= (void *) pBufferPriv;
    glPriv->freePrivate	= xglFreeBuffers;
}

static Bool
xglScreenProbe (int screen)
{
    ScreenPtr    pScreen = screenInfo.screens[screen];
    xglVisualPtr pVisual;
    Bool         status;
    int          i;

    XGL_SCREEN_PRIV (pScreen);

    status = (*screenInfoPriv.screenProbe) (screen);

    /* Create Xgl GLX visuals */
    for (i = 0; i < __xglScreenInfoPtr->numVisuals; i++)
    {
	pVisual = xglFindVisualWithId (pScreen, pScreen->visuals[i].vid);
	if (pVisual)
	{
	    glitz_drawable_format_t templ, *format, *screenFormat;
	    unsigned long	    mask;

	    templ.color        = pVisual->format.surface->color;
	    templ.depth_size   = __xglScreenInfoPtr->modes[i].depthBits;
	    templ.stencil_size = __xglScreenInfoPtr->modes[i].stencilBits;
	    templ.doublebuffer = __xglScreenInfoPtr->modes[i].doubleBufferMode;
	    templ.samples      = 1;

	    mask =
		GLITZ_FORMAT_FOURCC_MASK       |
		GLITZ_FORMAT_RED_SIZE_MASK     |
		GLITZ_FORMAT_GREEN_SIZE_MASK   |
		GLITZ_FORMAT_BLUE_SIZE_MASK    |
		GLITZ_FORMAT_ALPHA_SIZE_MASK   |
		GLITZ_FORMAT_DEPTH_SIZE_MASK   |
		GLITZ_FORMAT_STENCIL_SIZE_MASK |
		GLITZ_FORMAT_DOUBLEBUFFER_MASK |
		GLITZ_FORMAT_SAMPLES_MASK;

	    format = glitz_find_drawable_format (pScreenPriv->drawable,
						 mask, &templ, 0);
	    if (format)
	    {
		xglVisualPtr v, new, *prev;

		new = xalloc (sizeof (xglVisualRec));
		if (new)
		{
		    new->next    = 0;
		    new->vid     = pVisual->vid;
		    new->pPixel  = pVisual->pPixel;
		    new->pbuffer = FALSE;

		    new->format.surface  = pVisual->format.surface;
		    new->format.drawable = format;

		    prev = &pScreenPriv->pGlxVisual;
		    while ((v = *prev))
			prev = &v->next;

		    *prev = new;
		}
	    }

	    /* use same drawable format as screen for pbuffers */
	    screenFormat = glitz_drawable_get_format (pScreenPriv->drawable);
	    templ.id = screenFormat->id;

	    mask =
		GLITZ_FORMAT_ID_MASK	     |
		GLITZ_FORMAT_FOURCC_MASK     |
		GLITZ_FORMAT_RED_SIZE_MASK   |
		GLITZ_FORMAT_GREEN_SIZE_MASK |
		GLITZ_FORMAT_BLUE_SIZE_MASK  |
		GLITZ_FORMAT_SAMPLES_MASK;

	    format = glitz_find_pbuffer_format (pScreenPriv->drawable,
						mask, &templ, 0);
	    if (format)
	    {
		xglVisualPtr v, new, *prev;

		new = xalloc (sizeof (xglVisualRec));
		if (new)
		{
		    new->next    = 0;
		    new->vid     = pVisual->vid;
		    new->pPixel  = pVisual->pPixel;
		    new->pbuffer = TRUE;

		    new->format.surface  = pVisual->format.surface;
		    new->format.drawable = format;

		    prev = &pScreenPriv->pGlxVisual;
		    while ((v = *prev))
			prev = &v->next;

		    *prev = new;
		}
	    }
	}
    }

    /* Wrap createBuffer */
    if (__xglScreenInfoPtr->createBuffer != xglCreateBuffer)
    {
	screenInfoPriv.createBuffer    = __xglScreenInfoPtr->createBuffer;
	__xglScreenInfoPtr->createBuffer = xglCreateBuffer;
    }

    /* Wrap createContext */
    if (__xglScreenInfoPtr->createContext != xglCreateContext)
    {
	screenInfoPriv.createContext    = __xglScreenInfoPtr->createContext;
	__xglScreenInfoPtr->createContext = xglCreateContext;
    }

    return status;
}

Bool
xglInitVisualConfigs (ScreenPtr pScreen)
{
    miInitVisualsProcPtr    initVisualsProc = NULL;
    VisualPtr		    visuals;
    int			    nvisuals;
    DepthPtr		    depths;
    int			    ndepths;
    int			    rootDepth;
    VisualID		    defaultVis;
    glitz_drawable_format_t *format;
    xglVisualPtr	    pVisual;
    __GLXvisualConfig	    *pConfig;
    xglGLXVisualConfigPtr   pConfigPriv, *ppConfigPriv;
    XID			    *installedCmaps;
    ColormapPtr		    installedCmap;
    int			    numInstalledCmaps;
    int			    numConfig = 1;
    int			    bpp, i;

    XGL_SCREEN_PRIV (pScreen);

    if (xglScreenInfo.depth != 16 && xglScreenInfo.depth != 24)
	return FALSE;

    for (pVisual = xglVisuals; pVisual; pVisual = pVisual->next)
    {
	if (pVisual->pPixel->depth == xglScreenInfo.depth)
	    break;
    }

    if (!pVisual)
	return FALSE;

    bpp = pVisual->pPixel->masks.bpp;

    format = glitz_drawable_get_format (pScreenPriv->drawable);
    if (format->doublebuffer)
	numConfig *= 2;

    pConfig = xcalloc (sizeof (__GLXvisualConfig), numConfig);
    if (!pConfig)
	return FALSE;

    pConfigPriv = xcalloc (sizeof (xglGLXVisualConfigRec), numConfig);
    if (!pConfigPriv)
    {
	xfree (pConfig);
	return FALSE;
    }

    ppConfigPriv = xcalloc (sizeof (xglGLXVisualConfigPtr), numConfig);
    if (!ppConfigPriv)
    {
	xfree (pConfigPriv);
	xfree (pConfig);
	return FALSE;
    }

    installedCmaps = xalloc (pScreen->maxInstalledCmaps * sizeof (XID));
    if (!installedCmaps)
    {
	xfree (ppConfigPriv);
	xfree (pConfigPriv);
	xfree (pConfig);
	return FALSE;
    }

    for (i = 0; i < numConfig; i++)
    {
	ppConfigPriv[i] = &pConfigPriv[i];

	pConfig[i].vid   = (VisualID) (-1);
	pConfig[i].class = -1;
	pConfig[i].rgba  = TRUE;

	pConfig[i].redSize   = format->color.red_size;
	pConfig[i].greenSize = format->color.green_size;
	pConfig[i].blueSize  = format->color.blue_size;
	pConfig[i].alphaSize = format->color.alpha_size;

	pConfig[i].redMask   = pVisual->pPixel->masks.red_mask;
	pConfig[i].greenMask = pVisual->pPixel->masks.green_mask;
	pConfig[i].blueMask  = pVisual->pPixel->masks.blue_mask;
	pConfig[i].alphaMask = pVisual->pPixel->masks.alpha_mask;

	if (i == 1)
	{
	    pConfig[i].doubleBuffer = FALSE;
	    pConfig[i].depthSize    = 0;
	    pConfig[i].stencilSize  = 0;
	}
	else
	{
	    pConfig[i].doubleBuffer = TRUE;
	    pConfig[i].depthSize    = format->depth_size;
	    pConfig[i].stencilSize  = format->stencil_size;
	}

	pConfig[i].stereo = FALSE;

	if (pScreen->rootDepth == 16)
	    pConfig[i].bufferSize = 16;
	else
	    pConfig[i].bufferSize = 32;

	pConfig[i].auxBuffers = 0;
	pConfig[i].level      = 0;

	pConfig[i].visualRating = GLX_NONE;

	pConfig[i].transparentPixel = GLX_NONE;
	pConfig[i].transparentRed   = 0;
	pConfig[i].transparentGreen = 0;
	pConfig[i].transparentBlue  = 0;
	pConfig[i].transparentAlpha = 0;
	pConfig[i].transparentIndex = 0;
    }

    GlxSetVisualConfigs (numConfig, pConfig, (void **) ppConfigPriv);

    /* Wrap screenProbe */
    if (__xglScreenInfoPtr->screenProbe != xglScreenProbe)
    {
	screenInfoPriv.screenProbe    = __xglScreenInfoPtr->screenProbe;
	__xglScreenInfoPtr->screenProbe = xglScreenProbe;
    }

    visuals    = pScreen->visuals;
    nvisuals   = pScreen->numVisuals;
    depths     = pScreen->allowedDepths;
    ndepths    = pScreen->numDepths;
    rootDepth  = pScreen->rootDepth;
    defaultVis = pScreen->rootVisual;

    /* Find installed colormaps */
    numInstalledCmaps = (*pScreen->ListInstalledColormaps) (pScreen,
							    installedCmaps);

    GlxWrapInitVisuals (&initVisualsProc);
    GlxInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootDepth,
		    &defaultVis, ((unsigned long) 1 << (bpp - 1)), 8, -1);

    /* Fix up any existing installed colormaps. */
    for (i = 0; i < numInstalledCmaps; i++)
    {
	int j;

	installedCmap = LookupIDByType (installedCmaps[i], RT_COLORMAP);
	if (!installedCmap)
	    continue;

	j = installedCmap->pVisual - pScreen->visuals;
	installedCmap->pVisual = &visuals[j];
    }

    pScreen->visuals       = visuals;
    pScreen->numVisuals    = nvisuals;
    pScreen->allowedDepths = depths;
    pScreen->numDepths     = ndepths;
    pScreen->rootDepth     = rootDepth;
    pScreen->rootVisual    = defaultVis;

#ifndef NGLXLOG
    xglInitGlxLog ();
#endif

    xfree (installedCmaps);
    xfree (pConfigPriv);
    xfree (pConfig);

    return TRUE;
}
