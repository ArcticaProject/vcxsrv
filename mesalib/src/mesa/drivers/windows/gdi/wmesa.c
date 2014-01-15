/*
 * Windows (Win32/Win64) device driver for Mesa
 *
 */

#include "wmesadef.h"
#include "colors.h"
#include "GL/wmesa.h"
#include <winuser.h>
#include "main/context.h"
#include "main/api_exec.h"
#include "main/extensions.h"
#include "main/framebuffer.h"
#include "main/renderbuffer.h"
#include "main/macros.h"
#include "main/version.h"
#include "main/vtxfmt.h"
#include "drivers/common/driverfuncs.h"
#include "drivers/common/meta.h"
#include "vbo/vbo.h"
#include "swrast/swrast.h"
#include "swrast/s_renderbuffer.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"


/* linked list of our Framebuffers (windows) */
static WMesaFramebuffer FirstFramebuffer = NULL;


/**
 * Create a new WMesaFramebuffer object which will correspond to the
 * given HDC (Window handle).
 */
static WMesaFramebuffer
wmesa_new_framebuffer(HDC hdc, struct gl_config *visual)
{
    WMesaFramebuffer pwfb
        = malloc(sizeof(struct wmesa_framebuffer));
    if (pwfb) {
        _mesa_initialize_window_framebuffer(&pwfb->Base, visual);
        pwfb->hDC = hdc;
        /* insert at head of list */
        pwfb->next = FirstFramebuffer;
        FirstFramebuffer = pwfb;
    }
    return pwfb;
}

/**
 * Given an hdc, free the corresponding WMesaFramebuffer
 */
static void
wmesa_free_framebuffer(HDC hdc)
{
    WMesaFramebuffer pwfb, prev;
    for (pwfb = FirstFramebuffer; pwfb; pwfb = pwfb->next) {
        if (pwfb->hDC == hdc)
            break;
	prev = pwfb;
    }
    if (pwfb) {
        struct gl_framebuffer *fb;
	if (pwfb == FirstFramebuffer)
	    FirstFramebuffer = pwfb->next;
	else
	    prev->next = pwfb->next;
        fb = &pwfb->Base;
        _mesa_reference_framebuffer(&fb, NULL); 
    }
}

/**
 * Given an hdc, return the corresponding WMesaFramebuffer
 */
static WMesaFramebuffer
wmesa_lookup_framebuffer(HDC hdc)
{
    WMesaFramebuffer pwfb;
    for (pwfb = FirstFramebuffer; pwfb; pwfb = pwfb->next) {
        if (pwfb->hDC == hdc)
            return pwfb;
    }
    return NULL;
}


/**
 * Given a struct gl_framebuffer, return the corresponding WMesaFramebuffer.
 */
static WMesaFramebuffer wmesa_framebuffer(struct gl_framebuffer *fb)
{
    return (WMesaFramebuffer) fb;
}


/**
 * Given a struct gl_context, return the corresponding WMesaContext.
 */
static WMesaContext wmesa_context(const struct gl_context *ctx)
{
    return (WMesaContext) ctx;
}


/*
 * Every driver should implement a GetString function in order to
 * return a meaningful GL_RENDERER string.
 */
static const GLubyte *wmesa_get_string(struct gl_context *ctx, GLenum name)
{
    return (name == GL_RENDERER) ? 
	(GLubyte *) "Mesa Windows GDI Driver" : NULL;
}


/*
 * Determine the pixel format based on the pixel size.
 */
static void wmSetPixelFormat(WMesaFramebuffer pwfb, HDC hDC)
{
    pwfb->cColorBits = GetDeviceCaps(hDC, BITSPIXEL);

    /* Only 16 and 32 bit targets are supported now */
    assert(pwfb->cColorBits == 0 ||
	   pwfb->cColorBits == 16 || 
	   pwfb->cColorBits == 24 || 
	   pwfb->cColorBits == 32);

    switch(pwfb->cColorBits){
    case 8:
	pwfb->pixelformat = PF_INDEX8;
	break;
    case 16:
	pwfb->pixelformat = PF_5R6G5B;
	break;
    case 24:
    case 32:
	pwfb->pixelformat = PF_8R8G8B;
	break;
    default:
	pwfb->pixelformat = PF_BADFORMAT;
    }
}


/**
 * Create DIB for back buffer.
 * We write into this memory with the span routines and then blit it
 * to the window on a buffer swap.
 */
static BOOL wmCreateBackingStore(WMesaFramebuffer pwfb, long lxSize, long lySize)
{
    LPBITMAPINFO pbmi = &(pwfb->bmi);
    HDC          hic;

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = lxSize;
    pbmi->bmiHeader.biHeight= -lySize;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = GetDeviceCaps(pwfb->hDC, BITSPIXEL);
    pbmi->bmiHeader.biCompression = BI_RGB;
    pbmi->bmiHeader.biSizeImage = 0;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = 0;
    pbmi->bmiHeader.biClrImportant = 0;
    
    pwfb->cColorBits = pbmi->bmiHeader.biBitCount;
    pwfb->ScanWidth = (lxSize * (pwfb->cColorBits / 8) + 3) & ~3;
    
    hic = CreateIC("display", NULL, NULL, NULL);
    pwfb->dib_hDC = CreateCompatibleDC(hic);
    
    pwfb->hbmDIB = CreateDIBSection(hic,
				   &pwfb->bmi,
				   DIB_RGB_COLORS,
				   (void **)&(pwfb->pbPixels),
				   0,
				   0);
    pwfb->hOldBitmap = SelectObject(pwfb->dib_hDC, pwfb->hbmDIB);
    
    DeleteDC(hic);

    wmSetPixelFormat(pwfb, pwfb->hDC);
    return TRUE;
}


static void wmDeleteBackingStore(WMesaFramebuffer pwfb)
{
    if (pwfb->hbmDIB) {
	SelectObject(pwfb->dib_hDC, pwfb->hOldBitmap);
	DeleteDC(pwfb->dib_hDC);
	DeleteObject(pwfb->hbmDIB);
    }
}


/**
 * Find the width and height of the window named by hdc.
 */
static void
get_window_size(HDC hdc, GLuint *width, GLuint *height)
{
    if (WindowFromDC(hdc)) {
        RECT rect;
        GetClientRect(WindowFromDC(hdc), &rect);
        *width = rect.right - rect.left;
        *height = rect.bottom - rect.top;
    }
    else { /* Memory context */
        /* From contributed code - use the size of the desktop
         * for the size of a memory context (?) */
        *width = GetDeviceCaps(hdc, HORZRES);
        *height = GetDeviceCaps(hdc, VERTRES);
    }
}


static void
wmesa_get_buffer_size(struct gl_framebuffer *buffer, GLuint *width, GLuint *height)
{
    WMesaFramebuffer pwfb = wmesa_framebuffer(buffer);
    get_window_size(pwfb->hDC, width, height);
}


static void wmesa_flush(struct gl_context *ctx)
{
    WMesaFramebuffer pwfb = wmesa_framebuffer(ctx->WinSysDrawBuffer);

    if (ctx->Visual.doubleBufferMode == 1) {
	BitBlt(pwfb->hDC, 0, 0, pwfb->Base.Width, pwfb->Base.Height,
	       pwfb->dib_hDC, 0, 0, SRCCOPY);
    }
    else {
	/* Do nothing for single buffer */
    }
}


/**********************************************************************/
/*****                   CLEAR Functions                          *****/
/**********************************************************************/

/* 
 * Clear the color/depth/stencil buffers.
 */ 
static void clear(struct gl_context *ctx, GLbitfield mask)
{
#define FLIP(Y)  (ctx->DrawBuffer->Height - (Y) - 1)
    const GLint x = ctx->DrawBuffer->_Xmin;
    const GLint y = ctx->DrawBuffer->_Ymin;
    const GLint height = ctx->DrawBuffer->_Ymax - ctx->DrawBuffer->_Ymin;
    const GLint width  = ctx->DrawBuffer->_Xmax - ctx->DrawBuffer->_Xmin;

    WMesaContext pwc = wmesa_context(ctx);
    WMesaFramebuffer pwfb = wmesa_framebuffer(ctx->DrawBuffer);
    int done = 0;

    /* Let swrast do all the work if the masks are not set to
     * clear all channels. */
    if (!ctx->Color.ColorMask[0][0] ||
	!ctx->Color.ColorMask[0][1] ||
	!ctx->Color.ColorMask[0][2] ||
	!ctx->Color.ColorMask[0][3]) {
	_swrast_Clear(ctx, mask);
	return;
    }

    if (mask & BUFFER_BITS_COLOR) {
       /* setup the clearing color */
       const union gl_color_union color = ctx->Color.ClearColor;
       GLubyte col[3];
       UNCLAMPED_FLOAT_TO_UBYTE(col[0], color.f[0]);
       UNCLAMPED_FLOAT_TO_UBYTE(col[1], color.f[1]);
       UNCLAMPED_FLOAT_TO_UBYTE(col[2], color.f[2]);
       pwc->clearColorRef = RGB(col[0], col[1], col[2]);
       DeleteObject(pwc->clearPen);
       DeleteObject(pwc->clearBrush);
       pwc->clearPen = CreatePen(PS_SOLID, 1, pwc->clearColorRef); 
       pwc->clearBrush = CreateSolidBrush(pwc->clearColorRef); 
    }

    /* Back buffer */
    if (mask & BUFFER_BIT_BACK_LEFT) { 
	
	int     i, rowSize; 
	UINT    bytesPerPixel = pwfb->cColorBits / 8; 
	LPBYTE  lpb, clearRow;
	LPWORD  lpw;
	BYTE    bColor; 
	WORD    wColor; 
	BYTE    r, g, b; 
	DWORD   dwColor; 
	LPDWORD lpdw; 
	
	/* Try for a fast clear - clearing entire buffer with a single
	 * byte value. */
	if (width == ctx->DrawBuffer->Width &&
            height == ctx->DrawBuffer->Height) { /* entire buffer */
	    /* Now check for an easy clear value */
	    switch (bytesPerPixel) {
	    case 1:
		bColor = BGR8(GetRValue(pwc->clearColorRef), 
			      GetGValue(pwc->clearColorRef), 
			      GetBValue(pwc->clearColorRef));
		memset(pwfb->pbPixels, bColor, 
		       pwfb->ScanWidth * height);
		done = 1;
		break;
	    case 2:
		wColor = BGR16(GetRValue(pwc->clearColorRef), 
			       GetGValue(pwc->clearColorRef), 
			       GetBValue(pwc->clearColorRef)); 
		if (((wColor >> 8) & 0xff) == (wColor & 0xff)) {
		    memset(pwfb->pbPixels, wColor & 0xff, 
			   pwfb->ScanWidth * height);
		    done = 1;
		}
		break;
	    case 3:
		/* fall through */
	    case 4:
		if (GetRValue(pwc->clearColorRef) == 
		    GetGValue(pwc->clearColorRef) &&
		    GetRValue(pwc->clearColorRef) == 
		    GetBValue(pwc->clearColorRef)) {
		    memset(pwfb->pbPixels, 
			   GetRValue(pwc->clearColorRef), 
			   pwfb->ScanWidth * height);
		    done = 1;
		}
		break;
	    default:
		break;
	    }
	} /* all */

	if (!done) {
	    /* Need to clear a row at a time.  Begin by setting the first
	     * row in the area to be cleared to the clear color. */
	    
	    clearRow = pwfb->pbPixels + 
		pwfb->ScanWidth * FLIP(y) +
		bytesPerPixel * x; 
	    switch (bytesPerPixel) {
	    case 1:
		lpb = clearRow;
		bColor = BGR8(GetRValue(pwc->clearColorRef), 
			      GetGValue(pwc->clearColorRef), 
			      GetBValue(pwc->clearColorRef));
		memset(lpb, bColor, width);
		break;
	    case 2:
		lpw = (LPWORD)clearRow;
		wColor = BGR16(GetRValue(pwc->clearColorRef), 
			       GetGValue(pwc->clearColorRef), 
			       GetBValue(pwc->clearColorRef)); 
		for (i=0; i<width; i++)
		    *lpw++ = wColor;
		break;
	    case 3: 
		lpb = clearRow;
		r = GetRValue(pwc->clearColorRef); 
		g = GetGValue(pwc->clearColorRef); 
		b = GetBValue(pwc->clearColorRef); 
		for (i=0; i<width; i++) {
		    *lpb++ = b; 
		    *lpb++ = g; 
		    *lpb++ = r; 
		} 
		break;
	    case 4: 
		lpdw = (LPDWORD)clearRow; 
		dwColor = BGR32(GetRValue(pwc->clearColorRef), 
				GetGValue(pwc->clearColorRef), 
				GetBValue(pwc->clearColorRef)); 
		for (i=0; i<width; i++)
		    *lpdw++ = dwColor;
		break;
	    default:
		break;
	    } /* switch */
	    
	    /* copy cleared row to other rows in buffer */
	    lpb = clearRow - pwfb->ScanWidth;
	    rowSize = width * bytesPerPixel;
	    for (i=1; i<height; i++) { 
		memcpy(lpb, clearRow, rowSize); 
		lpb -= pwfb->ScanWidth; 
	    } 
	} /* not done */
	mask &= ~BUFFER_BIT_BACK_LEFT;
    } /* back buffer */ 

    /* front buffer */
    if (mask & BUFFER_BIT_FRONT_LEFT) { 
	HDC DC = pwc->hDC; 
	HPEN Old_Pen = SelectObject(DC, pwc->clearPen); 
	HBRUSH Old_Brush = SelectObject(DC, pwc->clearBrush);
	Rectangle(DC,
		  x,
		  FLIP(y) + 1,
		  x + width + 1,
		  FLIP(y) - height + 1);
	SelectObject(DC, Old_Pen); 
	SelectObject(DC, Old_Brush); 
	mask &= ~BUFFER_BIT_FRONT_LEFT;
    } /* front buffer */ 
    
    /* Call swrast if there is anything left to clear (like DEPTH) */ 
    if (mask) 
	_swrast_Clear(ctx, mask);
  
#undef FLIP
} 



/**********************************************************************/
/*****                   BUFFER Functions                         *****/
/**********************************************************************/




static void
wmesa_delete_renderbuffer(struct gl_context *ctx, struct gl_renderbuffer *rb)
{
    _mesa_delete_renderbuffer(ctx, rb);
}


/**
 * This is called by Mesa whenever it determines that the window size
 * has changed.  Do whatever's needed to cope with that.
 */
static GLboolean
wmesa_renderbuffer_storage(struct gl_context *ctx, 
			   struct gl_renderbuffer *rb,
			   GLenum internalFormat, 
			   GLuint width, 
			   GLuint height)
{
    rb->Width = width;
    rb->Height = height;
    return GL_TRUE;
}


/**
 * Called by ctx->Driver.ResizeBuffers()
 * Resize the front/back colorbuffers to match the latest window size.
 */
static void
wmesa_resize_buffers(struct gl_context *ctx, struct gl_framebuffer *buffer,
                     GLuint width, GLuint height)
{
    WMesaFramebuffer pwfb = wmesa_framebuffer(buffer);

    if (pwfb->Base.Width != width || pwfb->Base.Height != height) {
	/* Realloc back buffer */
	if (ctx->Visual.doubleBufferMode == 1) {
	    wmDeleteBackingStore(pwfb);
	    wmCreateBackingStore(pwfb, width, height);
	}
    }
    _mesa_resize_framebuffer(ctx, buffer, width, height);
}


/**
 * Called by glViewport.
 * This is a good time for us to poll the current window size and adjust
 * our renderbuffers to match the current window size.
 * Remember, we have no opportunity to respond to conventional
 * resize events since the driver has no event loop.
 * Thus, we poll.
 * MakeCurrent also ends up making a call here, so that ensures
 * we get the viewport set correctly, even if the app does not call
 * glViewport and relies on the defaults.
 */
static void wmesa_viewport(struct gl_context *ctx)
{
    GLuint new_width, new_height;

    wmesa_get_buffer_size(ctx->WinSysDrawBuffer, &new_width, &new_height);

    /**
     * Resize buffers if the window size changed.
     */
    wmesa_resize_buffers(ctx, ctx->WinSysDrawBuffer, new_width, new_height);
    ctx->NewState |= _NEW_BUFFERS;  /* to update scissor / window bounds */
}




/**
 * Called when the driver should update it's state, based on the new_state
 * flags.
 */
static void wmesa_update_state(struct gl_context *ctx, GLuint new_state)
{
    _swrast_InvalidateState(ctx, new_state);
    _swsetup_InvalidateState(ctx, new_state);
    _vbo_InvalidateState(ctx, new_state);
    _tnl_InvalidateState(ctx, new_state);

    /* TODO - This code is not complete yet because I 
     * don't know what to do for all state updates.
     */

    if (new_state & _NEW_BUFFERS) {
    }
}





/**********************************************************************/
/*****                   WMESA Functions                          *****/
/**********************************************************************/

WMesaContext WMesaCreateContext(HDC hDC, 
				HPALETTE* Pal,
				GLboolean rgb_flag,
				GLboolean db_flag,
				GLboolean alpha_flag)
{
    WMesaContext c;
    struct dd_function_table functions;
    GLint red_bits, green_bits, blue_bits, alpha_bits;
    struct gl_context *ctx;
    struct gl_config *visual;

    (void) Pal;
    
    /* Indexed mode not supported */
    if (!rgb_flag)
	return NULL;

    /* Allocate wmesa context */
    c = CALLOC_STRUCT(wmesa_context);
    if (!c)
	return NULL;

#if 0
    /* I do not understand this contributed code */
    /* Support memory and device contexts */
    if(WindowFromDC(hDC) != NULL) {
	c->hDC = GetDC(WindowFromDC(hDC)); /* huh ???? */
    }
    else {
	c->hDC = hDC;
    }
#else
    c->hDC = hDC;
#endif

    /* Get data for visual */
    /* Dealing with this is actually a bit of overkill because Mesa will end
     * up treating all color component size requests less than 8 by using 
     * a single byte per channel.  In addition, the interface to the span
     * routines passes colors as an entire byte per channel anyway, so there
     * is nothing to be saved by telling the visual to be 16 bits if the device
     * is 16 bits.  That is, Mesa is going to compute colors down to 8 bits per
     * channel anyway.
     * But we go through the motions here anyway.
     */
    switch (GetDeviceCaps(c->hDC, BITSPIXEL)) {
    case 16:
	red_bits = green_bits = blue_bits = 5;
	alpha_bits = 0;
	break;
    default:
	red_bits = green_bits = blue_bits = 8;
	alpha_bits = 8;
	break;
    }
    /* Create visual based on flags */
    visual = _mesa_create_visual(db_flag,    /* db_flag */
                                 GL_FALSE,   /* stereo */
                                 red_bits, green_bits, blue_bits, /* color RGB */
                                 alpha_flag ? alpha_bits : 0, /* color A */
                                 DEFAULT_SOFTWARE_DEPTH_BITS, /* depth_bits */
                                 8,          /* stencil_bits */
                                 16,16,16,   /* accum RGB */
                                 alpha_flag ? 16 : 0, /* accum A */
                                 1);         /* num samples */
    
    if (!visual) {
	free(c);
	return NULL;
    }

    /* Set up driver functions */
    _mesa_init_driver_functions(&functions);
    functions.GetString = wmesa_get_string;
    functions.UpdateState = wmesa_update_state;
    functions.Flush = wmesa_flush;
    functions.Clear = clear;
    functions.ResizeBuffers = wmesa_resize_buffers;
    functions.Viewport = wmesa_viewport;

    /* initialize the Mesa context data */
    ctx = &c->gl_ctx;
    _mesa_initialize_context(ctx, API_OPENGL_COMPAT, visual,
                             NULL, &functions);

    /* visual no longer needed - it was copied by _mesa_initialize_context() */
    _mesa_destroy_visual(visual);

    _mesa_enable_sw_extensions(ctx);
  
    _mesa_meta_init(ctx);

    /* Initialize the software rasterizer and helper modules. */
    if (!_swrast_CreateContext(ctx) ||
        !_vbo_CreateContext(ctx) ||
        !_tnl_CreateContext(ctx) ||
	!_swsetup_CreateContext(ctx)) {
	_mesa_free_context_data(ctx);
	free(c);
	return NULL;
    }
    _swsetup_Wakeup(ctx);
    TNL_CONTEXT(ctx)->Driver.RunPipeline = _tnl_run_pipeline;

    _mesa_compute_version(ctx);

    /* Exec table initialization requires the version to be computed */
    _mesa_initialize_dispatch_tables(ctx);
    _mesa_initialize_vbo_vtxfmt(ctx);

    return c;
}


void WMesaDestroyContext( WMesaContext pwc )
{
    struct gl_context *ctx = &pwc->gl_ctx;
    WMesaFramebuffer pwfb;
    GET_CURRENT_CONTEXT(cur_ctx);

    if (cur_ctx == ctx) {
        /* unbind current if deleting current context */
        WMesaMakeCurrent(NULL, NULL);
    }

    /* clean up frame buffer resources */
    pwfb = wmesa_lookup_framebuffer(pwc->hDC);
    if (pwfb) {
	if (ctx->Visual.doubleBufferMode == 1)
	    wmDeleteBackingStore(pwfb);
	wmesa_free_framebuffer(pwc->hDC);
    }

    /* Release for device, not memory contexts */
    if (WindowFromDC(pwc->hDC) != NULL)
    {
      ReleaseDC(WindowFromDC(pwc->hDC), pwc->hDC);
    }
    DeleteObject(pwc->clearPen); 
    DeleteObject(pwc->clearBrush); 
    
    _mesa_meta_free(ctx);

    _swsetup_DestroyContext(ctx);
    _tnl_DestroyContext(ctx);
    _vbo_DestroyContext(ctx);
    _swrast_DestroyContext(ctx);
    
    _mesa_free_context_data(ctx);
    free(pwc);
}


/**
 * Create a new color renderbuffer.
 */
static struct gl_renderbuffer *
wmesa_new_renderbuffer(void)
{
    struct gl_renderbuffer *rb = CALLOC_STRUCT(gl_renderbuffer);
    if (!rb)
        return NULL;

    _mesa_init_renderbuffer(rb, (GLuint)0);
    
    rb->_BaseFormat = GL_RGBA;
    rb->InternalFormat = GL_RGBA;
    rb->Delete = wmesa_delete_renderbuffer;
    rb->AllocStorage = wmesa_renderbuffer_storage;
    return rb;
}


void WMesaMakeCurrent(WMesaContext c, HDC hdc)
{
    WMesaFramebuffer pwfb;

    {
        /* return if already current */
        GET_CURRENT_CONTEXT(ctx);
        WMesaContext pwc = wmesa_context(ctx);
        if (pwc && c == pwc && pwc->hDC == hdc)
            return;
    }

    pwfb = wmesa_lookup_framebuffer(hdc);

    /* Lazy creation of framebuffers */
    if (c && !pwfb && hdc) {
        struct gl_renderbuffer *rb;
        struct gl_config *visual = &c->gl_ctx.Visual;
        GLuint width, height;

        get_window_size(hdc, &width, &height);

	c->clearPen = CreatePen(PS_SOLID, 1, 0); 
	c->clearBrush = CreateSolidBrush(0); 

        pwfb = wmesa_new_framebuffer(hdc, visual);

	/* Create back buffer if double buffered */
	if (visual->doubleBufferMode == 1) {
	    wmCreateBackingStore(pwfb, width, height);
	}
	
        /* make render buffers */
        if (visual->doubleBufferMode == 1) {
            rb = wmesa_new_renderbuffer();
            _mesa_add_renderbuffer(&pwfb->Base, BUFFER_BACK_LEFT, rb);
	}
        rb = wmesa_new_renderbuffer();
        _mesa_add_renderbuffer(&pwfb->Base, BUFFER_FRONT_LEFT, rb);

	/* Let Mesa own the Depth, Stencil, and Accum buffers */
        _swrast_add_soft_renderbuffers(&pwfb->Base,
                                       GL_FALSE, /* color */
                                       visual->depthBits > 0,
                                       visual->stencilBits > 0,
                                       visual->accumRedBits > 0,
                                       visual->alphaBits >0, 
                                       GL_FALSE);
    }

    if (c && pwfb)
	_mesa_make_current(&c->gl_ctx, &pwfb->Base, &pwfb->Base);
    else
        _mesa_make_current(NULL, NULL, NULL);
}


void WMesaSwapBuffers( HDC hdc )
{
    GET_CURRENT_CONTEXT(ctx);
    WMesaContext pwc = wmesa_context(ctx);
    WMesaFramebuffer pwfb = wmesa_lookup_framebuffer(hdc);

    if (!pwfb) {
        _mesa_problem(NULL, "wmesa: swapbuffers on unknown hdc");
        return;
    }

    /* If we're swapping the buffer associated with the current context
     * we have to flush any pending rendering commands first.
     */
    if (pwc->hDC == hdc) {
	_mesa_notifySwapBuffers(ctx);

	BitBlt(pwfb->hDC, 0, 0, pwfb->Base.Width, pwfb->Base.Height,
	       pwfb->dib_hDC, 0, 0, SRCCOPY);
    }
    else {
        /* XXX for now only allow swapping current window */
        _mesa_problem(NULL, "wmesa: can't swap non-current window");
    }
}

void WMesaShareLists(WMesaContext ctx_to_share, WMesaContext ctx)
{
	_mesa_share_state(&ctx->gl_ctx, &ctx_to_share->gl_ctx);	
}

