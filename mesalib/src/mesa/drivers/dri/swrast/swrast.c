/*
 * Copyright 2008, 2010 George Sapountzis <gsapountzis@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * DRI software rasterizer
 *
 * This is the mesa swrast module packaged into a DRI driver structure.
 *
 * The front-buffer is allocated by the loader. The loader provides read/write
 * callbacks for access to the front-buffer. The driver uses a scratch row for
 * front-buffer rendering to avoid repeated calls to the loader.
 *
 * The back-buffer is allocated by the driver and is private.
 */

#include "main/context.h"
#include "main/extensions.h"
#include "main/formats.h"
#include "main/framebuffer.h"
#include "main/imports.h"
#include "main/renderbuffer.h"
#include "swrast/swrast.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"
#include "vbo/vbo.h"
#include "drivers/common/driverfuncs.h"
#include "drivers/common/meta.h"
#include "utils.h"

#include "main/teximage.h"
#include "main/texfetch.h"
#include "main/texformat.h"
#include "main/texstate.h"

#include "swrast_priv.h"


/**
 * Screen and config-related functions
 */

static void swrastSetTexBuffer2(__DRIcontext *pDRICtx, GLint target,
				GLint texture_format, __DRIdrawable *dPriv)
{
    struct dri_context *dri_ctx;
    int x, y, w, h;
    __DRIscreen *sPriv = dPriv->driScreenPriv;
    struct gl_texture_unit *texUnit;
    struct gl_texture_object *texObj;
    struct gl_texture_image *texImage;
    uint32_t internalFormat;

    dri_ctx = pDRICtx->driverPrivate;

    internalFormat = (texture_format == __DRI_TEXTURE_FORMAT_RGB ? 3 : 4);

    texUnit = _mesa_get_current_tex_unit(&dri_ctx->Base);
    texObj = _mesa_select_tex_object(&dri_ctx->Base, texUnit, target);
    texImage = _mesa_get_tex_image(&dri_ctx->Base, texObj, target, 0);

    _mesa_lock_texture(&dri_ctx->Base, texObj);

    sPriv->swrast_loader->getDrawableInfo(dPriv, &x, &y, &w, &h, dPriv->loaderPrivate);

    _mesa_init_teximage_fields(&dri_ctx->Base, target, texImage,
			       w, h, 1, 0, internalFormat);

    if (texture_format == __DRI_TEXTURE_FORMAT_RGB)
	texImage->TexFormat = MESA_FORMAT_XRGB8888;
    else
	texImage->TexFormat = MESA_FORMAT_ARGB8888;

    _mesa_set_fetch_functions(texImage, 2);

    sPriv->swrast_loader->getImage(dPriv, x, y, w, h, (char *)texImage->Data,
				   dPriv->loaderPrivate);

    _mesa_unlock_texture(&dri_ctx->Base, texObj);
}

static void swrastSetTexBuffer(__DRIcontext *pDRICtx, GLint target,
			       __DRIdrawable *dPriv)
{
    swrastSetTexBuffer2(pDRICtx, target, __DRI_TEXTURE_FORMAT_RGBA, dPriv);
}

static const __DRItexBufferExtension swrastTexBufferExtension = {
    { __DRI_TEX_BUFFER, __DRI_TEX_BUFFER_VERSION },
    swrastSetTexBuffer,
    swrastSetTexBuffer2,
};

static const __DRIextension *dri_screen_extensions[] = {
    &swrastTexBufferExtension.base,
    NULL
};

static __DRIconfig **
swrastFillInModes(__DRIscreen *psp,
		  unsigned pixel_bits, unsigned depth_bits,
		  unsigned stencil_bits, GLboolean have_back_buffer)
{
    __DRIconfig **configs;
    unsigned depth_buffer_factor;
    unsigned back_buffer_factor;
    GLenum fb_format;
    GLenum fb_type;

    /* GLX_SWAP_COPY_OML is only supported because the Intel driver doesn't
     * support pageflipping at all.
     */
    static const GLenum back_buffer_modes[] = {
	GLX_NONE, GLX_SWAP_UNDEFINED_OML
    };

    uint8_t depth_bits_array[4];
    uint8_t stencil_bits_array[4];
    uint8_t msaa_samples_array[1];

    depth_bits_array[0] = 0;
    depth_bits_array[1] = 0;
    depth_bits_array[2] = depth_bits;
    depth_bits_array[3] = depth_bits;

    /* Just like with the accumulation buffer, always provide some modes
     * with a stencil buffer.
     */
    stencil_bits_array[0] = 0;
    stencil_bits_array[1] = (stencil_bits == 0) ? 8 : stencil_bits;
    stencil_bits_array[2] = 0;
    stencil_bits_array[3] = (stencil_bits == 0) ? 8 : stencil_bits;

    msaa_samples_array[0] = 0;

    depth_buffer_factor = 4;
    back_buffer_factor = 2;

    switch (pixel_bits) {
    case 8:
	fb_format = GL_RGB;
	fb_type = GL_UNSIGNED_BYTE_2_3_3_REV;
	break;
    case 16:
	fb_format = GL_RGB;
	fb_type = GL_UNSIGNED_SHORT_5_6_5;
	break;
    case 24:
	fb_format = GL_BGR;
	fb_type = GL_UNSIGNED_INT_8_8_8_8_REV;
	break;
    case 32:
	fb_format = GL_BGRA;
	fb_type = GL_UNSIGNED_INT_8_8_8_8_REV;
	break;
    default:
	fprintf(stderr, "[%s:%u] bad depth %d\n", __func__, __LINE__,
		pixel_bits);
	return NULL;
    }

    configs = driCreateConfigs(fb_format, fb_type,
			       depth_bits_array, stencil_bits_array,
			       depth_buffer_factor, back_buffer_modes,
			       back_buffer_factor, msaa_samples_array, 1,
			       GL_TRUE);
    if (configs == NULL) {
	fprintf(stderr, "[%s:%u] Error creating FBConfig!\n", __func__,
		__LINE__);
	return NULL;
    }

    return configs;
}

static const __DRIconfig **
dri_init_screen(__DRIscreen * psp)
{
    __DRIconfig **configs8, **configs16, **configs24, **configs32;

    TRACE;

    psp->extensions = dri_screen_extensions;

    configs8  = swrastFillInModes(psp,  8,  8, 0, 1);
    configs16 = swrastFillInModes(psp, 16, 16, 0, 1);
    configs24 = swrastFillInModes(psp, 24, 24, 8, 1);
    configs32 = swrastFillInModes(psp, 32, 24, 8, 1);

    configs16 = driConcatConfigs(configs8, configs16);
    configs24 = driConcatConfigs(configs16, configs24);
    configs32 = driConcatConfigs(configs24, configs32);

    return (const __DRIconfig **)configs32;
}

static void
dri_destroy_screen(__DRIscreen * sPriv)
{
    TRACE;
}


/**
 * Framebuffer and renderbuffer-related functions.
 */

static GLuint
choose_pixel_format(const GLvisual *v)
{
    int depth = v->rgbBits;

    if (depth == 32
	&& v->redMask   == 0xff0000
	&& v->greenMask == 0x00ff00
	&& v->blueMask  == 0x0000ff)
	return PF_A8R8G8B8;
    else if (depth == 24
	     && v->redMask   == 0xff0000
	     && v->greenMask == 0x00ff00
	     && v->blueMask  == 0x0000ff)
	return PF_X8R8G8B8;
    else if (depth == 16
	     && v->redMask   == 0xf800
	     && v->greenMask == 0x07e0
	     && v->blueMask  == 0x001f)
	return PF_R5G6B5;
    else if (depth == 8
	     && v->redMask   == 0x07
	     && v->greenMask == 0x38
	     && v->blueMask  == 0xc0)
	return PF_R3G3B2;

    _mesa_problem( NULL, "unexpected format in %s", __FUNCTION__ );
    return 0;
}

static void
swrast_delete_renderbuffer(struct gl_renderbuffer *rb)
{
    TRACE;

    free(rb->Data);
    free(rb);
}

/* see bytes_per_line in libGL */
static INLINE int
bytes_per_line(unsigned pitch_bits, unsigned mul)
{
   unsigned mask = mul - 1;

   return ((pitch_bits + mask) & ~mask) / 8;
}

static GLboolean
swrast_alloc_front_storage(GLcontext *ctx, struct gl_renderbuffer *rb,
			   GLenum internalFormat, GLuint width, GLuint height)
{
    struct swrast_renderbuffer *xrb = swrast_renderbuffer(rb);

    TRACE;

    rb->Data = NULL;
    rb->Width = width;
    rb->Height = height;

    xrb->pitch = bytes_per_line(width * xrb->bpp, 32);

    return GL_TRUE;
}

static GLboolean
swrast_alloc_back_storage(GLcontext *ctx, struct gl_renderbuffer *rb,
			  GLenum internalFormat, GLuint width, GLuint height)
{
    struct swrast_renderbuffer *xrb = swrast_renderbuffer(rb);

    TRACE;

    free(rb->Data);

    swrast_alloc_front_storage(ctx, rb, internalFormat, width, height);

    rb->Data = malloc(height * xrb->pitch);

    return GL_TRUE;
}

static struct swrast_renderbuffer *
swrast_new_renderbuffer(const GLvisual *visual, GLboolean front)
{
    struct swrast_renderbuffer *xrb = calloc(1, sizeof *xrb);
    GLuint pixel_format;

    TRACE;

    if (!xrb)
	return NULL;

    _mesa_init_renderbuffer(&xrb->Base, 0);

    pixel_format = choose_pixel_format(visual);

    xrb->Base.Delete = swrast_delete_renderbuffer;
    if (front) {
	xrb->Base.AllocStorage = swrast_alloc_front_storage;
	swrast_set_span_funcs_front(xrb, pixel_format);
    }
    else {
	xrb->Base.AllocStorage = swrast_alloc_back_storage;
	swrast_set_span_funcs_back(xrb, pixel_format);
    }

    switch (pixel_format) {
    case PF_A8R8G8B8:
	xrb->Base.Format = MESA_FORMAT_ARGB8888;
	xrb->Base.InternalFormat = GL_RGBA;
	xrb->Base._BaseFormat = GL_RGBA;
	xrb->Base.DataType = GL_UNSIGNED_BYTE;
	xrb->bpp = 32;
	break;
    case PF_X8R8G8B8:
	xrb->Base.Format = MESA_FORMAT_ARGB8888; /* XXX */
	xrb->Base.InternalFormat = GL_RGB;
	xrb->Base._BaseFormat = GL_RGB;
	xrb->Base.DataType = GL_UNSIGNED_BYTE;
	xrb->bpp = 32;
	break;
    case PF_R5G6B5:
	xrb->Base.Format = MESA_FORMAT_RGB565;
	xrb->Base.InternalFormat = GL_RGB;
	xrb->Base._BaseFormat = GL_RGB;
	xrb->Base.DataType = GL_UNSIGNED_BYTE;
	xrb->bpp = 16;
	break;
    case PF_R3G3B2:
	xrb->Base.Format = MESA_FORMAT_RGB332;
	xrb->Base.InternalFormat = GL_RGB;
	xrb->Base._BaseFormat = GL_RGB;
	xrb->Base.DataType = GL_UNSIGNED_BYTE;
	xrb->bpp = 8;
	break;
    default:
	return NULL;
    }

    return xrb;
}

static GLboolean
dri_create_buffer(__DRIscreen * sPriv,
		  __DRIdrawable * dPriv,
		  const __GLcontextModes * visual, GLboolean isPixmap)
{
    struct dri_drawable *drawable = NULL;
    GLframebuffer *fb;
    struct swrast_renderbuffer *frontrb, *backrb;

    TRACE;

    drawable = CALLOC_STRUCT(dri_drawable);
    if (drawable == NULL)
	goto drawable_fail;

    dPriv->driverPrivate = drawable;
    drawable->dPriv = dPriv;

    drawable->row = malloc(MAX_WIDTH * 4);
    if (drawable->row == NULL)
	goto drawable_fail;

    fb = &drawable->Base;

    /* basic framebuffer setup */
    _mesa_initialize_window_framebuffer(fb, visual);

    /* add front renderbuffer */
    frontrb = swrast_new_renderbuffer(visual, GL_TRUE);
    _mesa_add_renderbuffer(fb, BUFFER_FRONT_LEFT, &frontrb->Base);

    /* add back renderbuffer */
    if (visual->doubleBufferMode) {
	backrb = swrast_new_renderbuffer(visual, GL_FALSE);
	_mesa_add_renderbuffer(fb, BUFFER_BACK_LEFT, &backrb->Base);
    }

    /* add software renderbuffers */
    _mesa_add_soft_renderbuffers(fb,
				 GL_FALSE, /* color */
				 visual->haveDepthBuffer,
				 visual->haveStencilBuffer,
				 visual->haveAccumBuffer,
				 GL_FALSE, /* alpha */
				 GL_FALSE /* aux bufs */);

    return GL_TRUE;

drawable_fail:

    if (drawable)
	free(drawable->row);

    FREE(drawable);

    return GL_FALSE;
}

static void
dri_destroy_buffer(__DRIdrawable * dPriv)
{
    TRACE;

    if (dPriv) {
	struct dri_drawable *drawable = dri_drawable(dPriv);
	GLframebuffer *fb;

	free(drawable->row);

	fb = &drawable->Base;

	fb->DeletePending = GL_TRUE;
	_mesa_reference_framebuffer(&fb, NULL);
    }
}

static void
dri_swap_buffers(__DRIdrawable * dPriv)
{
    __DRIscreen *sPriv = dPriv->driScreenPriv;

    GET_CURRENT_CONTEXT(ctx);

    struct dri_drawable *drawable = dri_drawable(dPriv);
    GLframebuffer *fb;
    struct swrast_renderbuffer *frontrb, *backrb;

    TRACE;

    fb = &drawable->Base;

    frontrb =
	swrast_renderbuffer(fb->Attachment[BUFFER_FRONT_LEFT].Renderbuffer);
    backrb =
	swrast_renderbuffer(fb->Attachment[BUFFER_BACK_LEFT].Renderbuffer);

    /* check for signle-buffered */
    if (backrb == NULL)
	return;

    /* check if swapping currently bound buffer */
    if (ctx && ctx->DrawBuffer == fb) {
	/* flush pending rendering */
	_mesa_notifySwapBuffers(ctx);
    }

    sPriv->swrast_loader->putImage(dPriv, __DRI_SWRAST_IMAGE_OP_SWAP,
				   0, 0,
				   frontrb->Base.Width,
				   frontrb->Base.Height,
				   backrb->Base.Data,
				   dPriv->loaderPrivate);
}


/**
 * General device driver functions.
 */

static void
get_window_size( GLframebuffer *fb, GLsizei *w, GLsizei *h )
{
    __DRIdrawable *dPriv = swrast_drawable(fb)->dPriv;
    __DRIscreen *sPriv = dPriv->driScreenPriv;
    int x, y;

    sPriv->swrast_loader->getDrawableInfo(dPriv,
					  &x, &y, w, h,
					  dPriv->loaderPrivate);
}

static void
swrast_check_and_update_window_size( GLcontext *ctx, GLframebuffer *fb )
{
    GLsizei width, height;

    get_window_size(fb, &width, &height);
    if (fb->Width != width || fb->Height != height) {
	_mesa_resize_framebuffer(ctx, fb, width, height);
    }
}

static const GLubyte *
get_string(GLcontext *ctx, GLenum pname)
{
    (void) ctx;
    switch (pname) {
	case GL_VENDOR:
	    return (const GLubyte *) "Mesa Project";
	case GL_RENDERER:
	    return (const GLubyte *) "Software Rasterizer";
	default:
	    return NULL;
    }
}

static void
update_state( GLcontext *ctx, GLuint new_state )
{
    /* not much to do here - pass it on */
    _swrast_InvalidateState( ctx, new_state );
    _swsetup_InvalidateState( ctx, new_state );
    _vbo_InvalidateState( ctx, new_state );
    _tnl_InvalidateState( ctx, new_state );
}

static void
viewport(GLcontext *ctx, GLint x, GLint y, GLsizei w, GLsizei h)
{
    GLframebuffer *draw = ctx->WinSysDrawBuffer;
    GLframebuffer *read = ctx->WinSysReadBuffer;

    swrast_check_and_update_window_size(ctx, draw);
    swrast_check_and_update_window_size(ctx, read);
}

static gl_format swrastChooseTextureFormat(GLcontext * ctx,
					   GLint internalFormat,
					   GLenum format,
					   GLenum type)
{
    if (internalFormat == GL_RGB)
	return MESA_FORMAT_XRGB8888;
    return _mesa_choose_tex_format(ctx, internalFormat, format, type);
}

static void
swrast_init_driver_functions(struct dd_function_table *driver)
{
    driver->GetString = get_string;
    driver->UpdateState = update_state;
    driver->GetBufferSize = NULL;
    driver->Viewport = viewport;
    driver->ChooseTextureFormat = swrastChooseTextureFormat;
}


/**
 * Context-related functions.
 */

static GLboolean
dri_create_context(gl_api api,
		   const __GLcontextModes * visual,
		   __DRIcontext * cPriv, void *sharedContextPrivate)
{
    struct dri_context *ctx = NULL;
    struct dri_context *share = (struct dri_context *)sharedContextPrivate;
    GLcontext *mesaCtx = NULL;
    GLcontext *sharedCtx = NULL;
    struct dd_function_table functions;

    TRACE;

    ctx = CALLOC_STRUCT(dri_context);
    if (ctx == NULL)
	goto context_fail;

    cPriv->driverPrivate = ctx;
    ctx->cPriv = cPriv;

    /* build table of device driver functions */
    _mesa_init_driver_functions(&functions);
    swrast_init_driver_functions(&functions);

    if (share) {
	sharedCtx = &share->Base;
    }

    mesaCtx = &ctx->Base;

    /* basic context setup */
    if (!_mesa_initialize_context(mesaCtx, visual, sharedCtx, &functions, (void *) cPriv)) {
	goto context_fail;
    }

    /* do bounds checking to prevent segfaults and server crashes! */
    mesaCtx->Const.CheckArrayBounds = GL_TRUE;

    /* create module contexts */
    _swrast_CreateContext( mesaCtx );
    _vbo_CreateContext( mesaCtx );
    _tnl_CreateContext( mesaCtx );
    _swsetup_CreateContext( mesaCtx );
    _swsetup_Wakeup( mesaCtx );

    /* use default TCL pipeline */
    {
       TNLcontext *tnl = TNL_CONTEXT(mesaCtx);
       tnl->Driver.RunPipeline = _tnl_run_pipeline;
    }

    _mesa_enable_sw_extensions(mesaCtx);
    _mesa_enable_1_3_extensions(mesaCtx);
    _mesa_enable_1_4_extensions(mesaCtx);
    _mesa_enable_1_5_extensions(mesaCtx);
    _mesa_enable_2_0_extensions(mesaCtx);
    _mesa_enable_2_1_extensions(mesaCtx);

    _mesa_meta_init(mesaCtx);

    driInitExtensions( mesaCtx, NULL, GL_FALSE );

    return GL_TRUE;

context_fail:

    FREE(ctx);

    return GL_FALSE;
}

static void
dri_destroy_context(__DRIcontext * cPriv)
{
    TRACE;

    if (cPriv) {
	struct dri_context *ctx = dri_context(cPriv);
	GLcontext *mesaCtx;

	mesaCtx = &ctx->Base;

        _mesa_meta_free(mesaCtx);
	_swsetup_DestroyContext( mesaCtx );
	_swrast_DestroyContext( mesaCtx );
	_tnl_DestroyContext( mesaCtx );
	_vbo_DestroyContext( mesaCtx );
	_mesa_destroy_context( mesaCtx );
    }
}

static GLboolean
dri_make_current(__DRIcontext * cPriv,
		 __DRIdrawable * driDrawPriv,
		 __DRIdrawable * driReadPriv)
{
    GLcontext *mesaCtx;
    GLframebuffer *mesaDraw;
    GLframebuffer *mesaRead;
    TRACE;

    if (cPriv) {
	struct dri_context *ctx = dri_context(cPriv);
	struct dri_drawable *draw;
	struct dri_drawable *read;

	if (!driDrawPriv || !driReadPriv)
	    return GL_FALSE;

	draw = dri_drawable(driDrawPriv);
	read = dri_drawable(driReadPriv);
	mesaCtx = &ctx->Base;
	mesaDraw = &draw->Base;
	mesaRead = &read->Base;

	/* check for same context and buffer */
	if (mesaCtx == _mesa_get_current_context()
	    && mesaCtx->DrawBuffer == mesaDraw
	    && mesaCtx->ReadBuffer == mesaRead) {
	    return GL_TRUE;
	}

	_glapi_check_multithread();

	swrast_check_and_update_window_size(mesaCtx, mesaDraw);
	if (mesaRead != mesaDraw)
	    swrast_check_and_update_window_size(mesaCtx, mesaRead);

	_mesa_make_current( mesaCtx,
			    mesaDraw,
			    mesaRead );
    }
    else {
	/* unbind */
	_mesa_make_current( NULL, NULL, NULL );
    }

    return GL_TRUE;
}

static GLboolean
dri_unbind_context(__DRIcontext * cPriv)
{
    TRACE;
    (void) cPriv;

    /* Unset current context and dispath table */
    _mesa_make_current(NULL, NULL, NULL);

    return GL_TRUE;
}


const struct __DriverAPIRec driDriverAPI = {
    .InitScreen = dri_init_screen,
    .DestroyScreen = dri_destroy_screen,
    .CreateContext = dri_create_context,
    .DestroyContext = dri_destroy_context,
    .CreateBuffer = dri_create_buffer,
    .DestroyBuffer = dri_destroy_buffer,
    .SwapBuffers = dri_swap_buffers,
    .MakeCurrent = dri_make_current,
    .UnbindContext = dri_unbind_context,
};

/* This is the table of extensions that the loader will dlsym() for. */
PUBLIC const __DRIextension *__driDriverExtensions[] = {
    &driCoreExtension.base,
    &driSWRastExtension.base,
    NULL
};
