/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>
#include "glxserver.h"
#include "glxutil.h"
#include <GL/glxtokens.h>
#include <unpack.h>
#include "g_disptab.h"
#include <pixmapstr.h>
#include <windowstr.h>
#include "glxext.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"
#include "indirect_dispatch.h"
#include "indirect_table.h"
#include "indirect_util.h"


/************************************************************************/

/*
** Byteswapping versions of GLX commands.  In most cases they just swap
** the incoming arguments and then call the unswapped routine.  For commands
** that have replies, a separate swapping routine for the reply is provided;
** it is called at the end of the unswapped routine.
*/

int __glXDispSwap_CreateContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateContextReq *req = (xGLXCreateContextReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->visual);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->shareList);

    return __glXDisp_CreateContext(cl, pc);
}

int __glXDispSwap_CreateNewContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateNewContextReq *req = (xGLXCreateNewContextReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->renderType);
    __GLX_SWAP_INT(&req->shareList);

    return __glXDisp_CreateNewContext(cl, pc);
}

int __glXDispSwap_CreateContextWithConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateContextWithConfigSGIXReq *req =
	(xGLXCreateContextWithConfigSGIXReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->renderType);
    __GLX_SWAP_INT(&req->shareList);

    return __glXDisp_CreateContextWithConfigSGIX(cl, pc);
}

int __glXDispSwap_DestroyContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyContextReq *req = (xGLXDestroyContextReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);

    return __glXDisp_DestroyContext(cl, pc);
}

int __glXDispSwap_MakeCurrent(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeCurrentReq *req = (xGLXMakeCurrentReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->drawable);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->oldContextTag);

    return __glXDisp_MakeCurrent(cl, pc);
}

int __glXDispSwap_MakeContextCurrent(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeContextCurrentReq *req = (xGLXMakeContextCurrentReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->drawable);
    __GLX_SWAP_INT(&req->readdrawable);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->oldContextTag);

    return __glXDisp_MakeContextCurrent(cl, pc);
}

int __glXDispSwap_MakeCurrentReadSGI(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeCurrentReadSGIReq *req = (xGLXMakeCurrentReadSGIReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->drawable);
    __GLX_SWAP_INT(&req->readable);
    __GLX_SWAP_INT(&req->context);
    __GLX_SWAP_INT(&req->oldContextTag);

    return __glXDisp_MakeCurrentReadSGI(cl, pc);
}

int __glXDispSwap_IsDirect(__GLXclientState *cl, GLbyte *pc)
{
    xGLXIsDirectReq *req = (xGLXIsDirectReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);

    return __glXDisp_IsDirect(cl, pc);
}

int __glXDispSwap_QueryVersion(__GLXclientState *cl, GLbyte *pc)
{
    xGLXQueryVersionReq *req = (xGLXQueryVersionReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->majorVersion);
    __GLX_SWAP_INT(&req->minorVersion);

    return __glXDisp_QueryVersion(cl, pc);
}

int __glXDispSwap_WaitGL(__GLXclientState *cl, GLbyte *pc)
{
    xGLXWaitGLReq *req = (xGLXWaitGLReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);

    return __glXDisp_WaitGL(cl, pc);
}

int __glXDispSwap_WaitX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXWaitXReq *req = (xGLXWaitXReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);

    return __glXDisp_WaitX(cl, pc);
}

int __glXDispSwap_CopyContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCopyContextReq *req = (xGLXCopyContextReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->source);
    __GLX_SWAP_INT(&req->dest);
    __GLX_SWAP_INT(&req->mask);

    return __glXDisp_CopyContext(cl, pc);
}

int __glXDispSwap_GetVisualConfigs(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetVisualConfigsReq *req = (xGLXGetVisualConfigsReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->screen);
    return __glXDisp_GetVisualConfigs(cl, pc);
}

int __glXDispSwap_GetFBConfigs(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetFBConfigsReq *req = (xGLXGetFBConfigsReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->screen);
    return __glXDisp_GetFBConfigs(cl, pc);
}

int __glXDispSwap_GetFBConfigsSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetFBConfigsSGIXReq *req = (xGLXGetFBConfigsSGIXReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->screen);
    return __glXDisp_GetFBConfigsSGIX(cl, pc);
}

int __glXDispSwap_CreateGLXPixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateGLXPixmapReq *req = (xGLXCreateGLXPixmapReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->visual);
    __GLX_SWAP_INT(&req->pixmap);
    __GLX_SWAP_INT(&req->glxpixmap);

    return __glXDisp_CreateGLXPixmap(cl, pc);
}

int __glXDispSwap_CreatePixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreatePixmapReq *req = (xGLXCreatePixmapReq *) pc;
    CARD32 *attribs;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->pixmap);
    __GLX_SWAP_INT(&req->glxpixmap);
    __GLX_SWAP_INT(&req->numAttribs);
    attribs = (CARD32*)(req + 1);
    __GLX_SWAP_INT_ARRAY(attribs, req->numAttribs);

    return __glXDisp_CreatePixmap(cl, pc);
}

int __glXDispSwap_CreateGLXPixmapWithConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateGLXPixmapWithConfigSGIXReq *req = 
	(xGLXCreateGLXPixmapWithConfigSGIXReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->pixmap);
    __GLX_SWAP_INT(&req->glxpixmap);

    return __glXDisp_CreateGLXPixmapWithConfigSGIX(cl, pc);
}

int __glXDispSwap_DestroyGLXPixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyGLXPixmapReq *req = (xGLXDestroyGLXPixmapReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->glxpixmap);

    return __glXDisp_DestroyGLXPixmap(cl, pc);
}

int __glXDispSwap_DestroyPixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyGLXPixmapReq *req = (xGLXDestroyGLXPixmapReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->glxpixmap);

    return __glXDisp_DestroyGLXPixmap(cl, pc);
}

int __glXDispSwap_QueryContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXQueryContextReq *req = (xGLXQueryContextReq *) pc;    
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->context);

    return __glXDisp_QueryContext(cl, pc);
}

int __glXDispSwap_CreatePbuffer(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreatePbufferReq *req = (xGLXCreatePbufferReq *) pc;    
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    CARD32 *attribs;

    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->pbuffer);
    __GLX_SWAP_INT(&req->numAttribs);
    attribs = (CARD32*)(req + 1);
    __GLX_SWAP_INT_ARRAY(attribs, req->numAttribs);

    return __glXDisp_CreatePbuffer(cl, pc);
}

int __glXDispSwap_CreateGLXPbufferSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateGLXPbufferSGIXReq *req = (xGLXCreateGLXPbufferSGIXReq *) pc;    
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->pbuffer);
    __GLX_SWAP_INT(&req->width);
    __GLX_SWAP_INT(&req->height);

    return __glXDisp_CreateGLXPbufferSGIX(cl, pc);
}

int __glXDispSwap_DestroyPbuffer(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyPbufferReq *req = (xGLXDestroyPbufferReq *) req;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->pbuffer);

    return __glXDisp_DestroyPbuffer(cl, pc);
}

int __glXDispSwap_DestroyGLXPbufferSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyGLXPbufferSGIXReq *req = (xGLXDestroyGLXPbufferSGIXReq *) req;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->pbuffer);

    return __glXDisp_DestroyGLXPbufferSGIX(cl, pc);
}

int __glXDispSwap_ChangeDrawableAttributes(__GLXclientState *cl, GLbyte *pc)
{
    xGLXChangeDrawableAttributesReq *req =
	(xGLXChangeDrawableAttributesReq *) req;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    CARD32 *attribs;

    __GLX_SWAP_INT(&req->drawable);
    __GLX_SWAP_INT(&req->numAttribs);
    attribs = (CARD32*)(req + 1);
    __GLX_SWAP_INT_ARRAY(attribs, req->numAttribs);

    return __glXDisp_ChangeDrawableAttributes(cl, pc);
}

int __glXDispSwap_ChangeDrawableAttributesSGIX(__GLXclientState *cl,
					       GLbyte *pc)
{
    xGLXChangeDrawableAttributesSGIXReq *req =
	(xGLXChangeDrawableAttributesSGIXReq *) req;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    CARD32 *attribs;

    __GLX_SWAP_INT(&req->drawable);
    __GLX_SWAP_INT(&req->numAttribs);
    attribs = (CARD32*)(req + 1);
    __GLX_SWAP_INT_ARRAY(attribs, req->numAttribs);

    return __glXDisp_ChangeDrawableAttributesSGIX(cl, pc);
}

int __glXDispSwap_CreateWindow(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateWindowReq *req = (xGLXCreateWindowReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    CARD32 *attribs;

    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->fbconfig);
    __GLX_SWAP_INT(&req->window);
    __GLX_SWAP_INT(&req->glxwindow);
    __GLX_SWAP_INT(&req->numAttribs);
    attribs = (CARD32*)(req + 1);
    __GLX_SWAP_INT_ARRAY(attribs, req->numAttribs);

    return __glXDisp_CreateWindow(cl, pc);
}

int __glXDispSwap_DestroyWindow(__GLXclientState *cl, GLbyte *pc)
{
    xGLXDestroyWindowReq *req = (xGLXDestroyWindowReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&req->glxwindow);

    return __glXDisp_DestroyWindow(cl, pc);
}

int __glXDispSwap_SwapBuffers(__GLXclientState *cl, GLbyte *pc)
{
    xGLXSwapBuffersReq *req = (xGLXSwapBuffersReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(&req->drawable);

    return __glXDisp_SwapBuffers(cl, pc);
}

int __glXDispSwap_UseXFont(__GLXclientState *cl, GLbyte *pc)
{
    xGLXUseXFontReq *req = (xGLXUseXFontReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(&req->font);
    __GLX_SWAP_INT(&req->first);
    __GLX_SWAP_INT(&req->count);
    __GLX_SWAP_INT(&req->listBase);

    return __glXDisp_UseXFont(cl, pc);
}


int __glXDispSwap_QueryExtensionsString(__GLXclientState *cl, GLbyte *pc)
{
    xGLXQueryExtensionsStringReq *req = (xGLXQueryExtensionsStringReq *)pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->screen);

    return __glXDisp_QueryExtensionsString(cl, pc);
}

int __glXDispSwap_QueryServerString(__GLXclientState *cl, GLbyte *pc)
{
    xGLXQueryServerStringReq *req = (xGLXQueryServerStringReq *)pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->screen);
    __GLX_SWAP_INT(&req->name);

    return __glXDisp_QueryServerString(cl, pc);
}

int __glXDispSwap_ClientInfo(__GLXclientState *cl, GLbyte *pc)
{
    xGLXClientInfoReq *req = (xGLXClientInfoReq *)pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->major);
    __GLX_SWAP_INT(&req->minor);
    __GLX_SWAP_INT(&req->numbytes);

    return __glXDisp_ClientInfo(cl, pc);
}

int __glXDispSwap_QueryContextInfoEXT(__GLXclientState *cl, GLbyte *pc)
{
    xGLXQueryContextInfoEXTReq *req = (xGLXQueryContextInfoEXTReq *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->context);

    return __glXDisp_QueryContextInfoEXT(cl, pc);
}

int __glXDispSwap_BindTexImageEXT(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateReq *req = (xGLXVendorPrivateReq *) pc;
    GLXDrawable		 *drawId;
    int			 *buffer;
    
    __GLX_DECLARE_SWAP_VARIABLES;

    pc += __GLX_VENDPRIV_HDR_SIZE;

    drawId = ((GLXDrawable *) (pc));
    buffer = ((int *)	      (pc + 4));
    
    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(drawId);
    __GLX_SWAP_INT(buffer);

    return __glXDisp_BindTexImageEXT(cl, (GLbyte *)pc);
}

int __glXDispSwap_ReleaseTexImageEXT(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateReq *req = (xGLXVendorPrivateReq *) pc;
    GLXDrawable		 *drawId;
    int			 *buffer;
    
    __GLX_DECLARE_SWAP_VARIABLES;

    pc += __GLX_VENDPRIV_HDR_SIZE;

    drawId = ((GLXDrawable *) (pc));
    buffer = ((int *)	      (pc + 4));
    
    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(drawId);
    __GLX_SWAP_INT(buffer);

    return __glXDisp_ReleaseTexImageEXT(cl, (GLbyte *)pc);
}

int __glXDispSwap_CopySubBufferMESA(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateReq *req = (xGLXVendorPrivateReq *) pc;
    GLXDrawable		 *drawId;
    int			 *buffer;

    __GLX_DECLARE_SWAP_VARIABLES;

    (void) drawId;
    (void) buffer;

    pc += __GLX_VENDPRIV_HDR_SIZE;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(pc);
    __GLX_SWAP_INT(pc + 4);
    __GLX_SWAP_INT(pc + 8);
    __GLX_SWAP_INT(pc + 12);
    __GLX_SWAP_INT(pc + 16);

    return __glXDisp_CopySubBufferMESA(cl, pc);

}

int __glXDispSwap_GetDrawableAttributesSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateWithReplyReq *req = (xGLXVendorPrivateWithReplyReq *)pc;
    CARD32 *data;
    
    __GLX_DECLARE_SWAP_VARIABLES;

    data = (CARD32 *) (req + 1);
    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->contextTag);
    __GLX_SWAP_INT(data);

    return __glXDisp_GetDrawableAttributesSGIX(cl, pc);
}

int __glXDispSwap_GetDrawableAttributes(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetDrawableAttributesReq *req = (xGLXGetDrawableAttributesReq *)pc;
    
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->drawable);

    return __glXDisp_GetDrawableAttributes(cl, pc);
}


/************************************************************************/

/*
** Swap replies.
*/

void __glXSwapMakeCurrentReply(ClientPtr client, xGLXMakeCurrentReply *reply)
{
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->contextTag);
    WriteToClient(client, sz_xGLXMakeCurrentReply, (char *)reply);
}

void __glXSwapIsDirectReply(ClientPtr client, xGLXIsDirectReply *reply)
{
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    WriteToClient(client, sz_xGLXIsDirectReply, (char *)reply);
}

void __glXSwapQueryVersionReply(ClientPtr client, xGLXQueryVersionReply *reply)
{
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->majorVersion);
    __GLX_SWAP_INT(&reply->minorVersion);
    WriteToClient(client, sz_xGLXQueryVersionReply, (char *)reply);
}

void glxSwapQueryExtensionsStringReply(ClientPtr client,
				       xGLXQueryExtensionsStringReply *reply, char *buf)
{
    int length = reply->length;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->n);
    WriteToClient(client, sz_xGLXQueryExtensionsStringReply, (char *)reply);
    __GLX_SWAP_INT_ARRAY((int *)buf, length);
    WriteToClient(client, length << 2, buf);
}

void glxSwapQueryServerStringReply(ClientPtr client,
				   xGLXQueryServerStringReply *reply, char *buf)
{
    int length = reply->length;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->n);
    WriteToClient(client, sz_xGLXQueryServerStringReply, (char *)reply);
    /** no swap is needed for an array of chars **/
    /* __GLX_SWAP_INT_ARRAY((int *)buf, length); */
    WriteToClient(client, length << 2, buf);
}

void __glXSwapQueryContextInfoEXTReply(ClientPtr client, xGLXQueryContextInfoEXTReply *reply, int *buf)
{
    int length = reply->length;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->n);
    WriteToClient(client, sz_xGLXQueryContextInfoEXTReply, (char *)reply);
    __GLX_SWAP_INT_ARRAY((int *)buf, length);
    WriteToClient(client, length << 2, (char *)buf);
}

void __glXSwapGetDrawableAttributesReply(ClientPtr client,
					 xGLXGetDrawableAttributesReply *reply, CARD32 *buf)
{
    int length = reply->length;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    __GLX_SWAP_SHORT(&reply->sequenceNumber);
    __GLX_SWAP_INT(&reply->length);
    __GLX_SWAP_INT(&reply->numAttribs);
    WriteToClient(client, sz_xGLXGetDrawableAttributesReply, (char *)reply);
    __GLX_SWAP_INT_ARRAY((int *)buf, length);
    WriteToClient(client, length << 2, (char *)buf);
}

/************************************************************************/

/*
** Render and Renderlarge are not in the GLX API.  They are used by the GLX
** client library to send batches of GL rendering commands.
*/

int __glXDispSwap_Render(__GLXclientState *cl, GLbyte *pc)
{
    return __glXDisp_Render(cl, pc);
}

/*
** Execute a large rendering request (one that spans multiple X requests).
*/
int __glXDispSwap_RenderLarge(__GLXclientState *cl, GLbyte *pc)
{
    return __glXDisp_RenderLarge(cl, pc);
}

/************************************************************************/

/*
** No support is provided for the vendor-private requests other than
** allocating these entry points in the dispatch table.
*/

int __glXDispSwap_VendorPrivate(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateReq *req;
    GLint vendorcode;
    __GLXdispatchVendorPrivProcPtr proc;

    __GLX_DECLARE_SWAP_VARIABLES;

    req = (xGLXVendorPrivateReq *) pc;
    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->vendorCode);

    vendorcode = req->vendorCode;

    proc = (__GLXdispatchVendorPrivProcPtr)
      __glXGetProtocolDecodeFunction(& VendorPriv_dispatch_info,
				     vendorcode, 1);
    if (proc != NULL) {
	(*proc)(cl, (GLbyte*)req);
	return Success;
    }

    cl->client->errorValue = req->vendorCode;
    return __glXError(GLXUnsupportedPrivateRequest);
}


int __glXDispSwap_VendorPrivateWithReply(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateWithReplyReq *req;
    GLint vendorcode;
    __GLXdispatchVendorPrivProcPtr proc;

    __GLX_DECLARE_SWAP_VARIABLES;

    req = (xGLXVendorPrivateWithReplyReq *) pc;
    __GLX_SWAP_SHORT(&req->length);
    __GLX_SWAP_INT(&req->vendorCode);

    vendorcode = req->vendorCode;

    proc = (__GLXdispatchVendorPrivProcPtr)
      __glXGetProtocolDecodeFunction(& VendorPriv_dispatch_info,
				     vendorcode, 1);
    if (proc != NULL) {
	return (*proc)(cl, (GLbyte*)req);
    }

    cl->client->errorValue = req->vendorCode;
    return __glXError(GLXUnsupportedPrivateRequest);
}
