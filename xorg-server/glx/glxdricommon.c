/*
 * Copyright © 2008 Red Hat, Inc
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <GL/glxtokens.h>
#include <GL/internal/dri_interface.h>
#include <os.h>
#include "glxserver.h"
#include "glxcontext.h"
#include "glxscreens.h"
#include "glxdricommon.h"

static int
getUST(int64_t *ust)
{
    struct timeval  tv;
    
    if (ust == NULL)
	return -EFAULT;

    if (gettimeofday(&tv, NULL) == 0) {
	ust[0] = (tv.tv_sec * 1000000) + tv.tv_usec;
	return 0;
    } else {
	return -errno;
    }
}

const __DRIsystemTimeExtension systemTimeExtension = {
    { __DRI_SYSTEM_TIME, __DRI_SYSTEM_TIME_VERSION },
    getUST,
    NULL,
};
    
#define __ATTRIB(attrib, field) \
    { attrib, offsetof(__GLXconfig, field) }

static const struct { unsigned int attrib, offset; } attribMap[] = {
    __ATTRIB(__DRI_ATTRIB_BUFFER_SIZE,			rgbBits),
    __ATTRIB(__DRI_ATTRIB_LEVEL,			level),
    __ATTRIB(__DRI_ATTRIB_RED_SIZE,			redBits),
    __ATTRIB(__DRI_ATTRIB_GREEN_SIZE,			greenBits),
    __ATTRIB(__DRI_ATTRIB_BLUE_SIZE,			blueBits),
    __ATTRIB(__DRI_ATTRIB_ALPHA_SIZE,			alphaBits),
    __ATTRIB(__DRI_ATTRIB_DEPTH_SIZE,			depthBits),
    __ATTRIB(__DRI_ATTRIB_STENCIL_SIZE,			stencilBits),
    __ATTRIB(__DRI_ATTRIB_ACCUM_RED_SIZE,		accumRedBits),
    __ATTRIB(__DRI_ATTRIB_ACCUM_GREEN_SIZE,		accumGreenBits),
    __ATTRIB(__DRI_ATTRIB_ACCUM_BLUE_SIZE,		accumBlueBits),
    __ATTRIB(__DRI_ATTRIB_ACCUM_ALPHA_SIZE,		accumAlphaBits),
    __ATTRIB(__DRI_ATTRIB_SAMPLE_BUFFERS,		sampleBuffers),
    __ATTRIB(__DRI_ATTRIB_SAMPLES,			samples),
    __ATTRIB(__DRI_ATTRIB_DOUBLE_BUFFER,		doubleBufferMode),
    __ATTRIB(__DRI_ATTRIB_STEREO,			stereoMode),
    __ATTRIB(__DRI_ATTRIB_AUX_BUFFERS,			numAuxBuffers),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_TYPE,		transparentPixel),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_INDEX_VALUE,	transparentPixel),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_RED_VALUE,	transparentRed),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_GREEN_VALUE,	transparentGreen),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_BLUE_VALUE,	transparentBlue),
    __ATTRIB(__DRI_ATTRIB_TRANSPARENT_ALPHA_VALUE,	transparentAlpha),
    __ATTRIB(__DRI_ATTRIB_RED_MASK,			redMask),
    __ATTRIB(__DRI_ATTRIB_GREEN_MASK,			greenMask),
    __ATTRIB(__DRI_ATTRIB_BLUE_MASK,			blueMask),
    __ATTRIB(__DRI_ATTRIB_ALPHA_MASK,			alphaMask),
    __ATTRIB(__DRI_ATTRIB_MAX_PBUFFER_WIDTH,		maxPbufferWidth),
    __ATTRIB(__DRI_ATTRIB_MAX_PBUFFER_HEIGHT,		maxPbufferHeight),
    __ATTRIB(__DRI_ATTRIB_MAX_PBUFFER_PIXELS,		maxPbufferPixels),
    __ATTRIB(__DRI_ATTRIB_OPTIMAL_PBUFFER_WIDTH,	optimalPbufferWidth),
    __ATTRIB(__DRI_ATTRIB_OPTIMAL_PBUFFER_HEIGHT,	optimalPbufferHeight),
    __ATTRIB(__DRI_ATTRIB_SWAP_METHOD,			swapMethod),
    __ATTRIB(__DRI_ATTRIB_BIND_TO_TEXTURE_RGB,		bindToTextureRgb),
    __ATTRIB(__DRI_ATTRIB_BIND_TO_TEXTURE_RGBA,		bindToTextureRgba),
    __ATTRIB(__DRI_ATTRIB_BIND_TO_MIPMAP_TEXTURE,	bindToMipmapTexture),
    __ATTRIB(__DRI_ATTRIB_YINVERTED,			yInverted),
};

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

static void
setScalar(__GLXconfig *config, unsigned int attrib, unsigned int value)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(attribMap); i++)
	if (attribMap[i].attrib == attrib) {
	    *(unsigned int *) ((char *) config + attribMap[i].offset) = value;
	    return;
	}
}

static __GLXconfig *
createModeFromConfig(const __DRIcoreExtension *core,
		     const __DRIconfig *driConfig,
		     unsigned int visualType)
{
    __GLXDRIconfig *config;
    unsigned int attrib, value;
    int i;

    config = xalloc(sizeof *config);

    config->driConfig = driConfig;

    i = 0;
    while (core->indexConfigAttrib(driConfig, i++, &attrib, &value)) {
	switch (attrib) {
	case __DRI_ATTRIB_RENDER_TYPE:
	    config->config.renderType = 0;
	    if (value & __DRI_ATTRIB_RGBA_BIT)
		config->config.renderType |= GLX_RGBA_BIT;
	    if (value & __DRI_ATTRIB_COLOR_INDEX_BIT)
		config->config.renderType |= GLX_COLOR_INDEX_BIT;
	    break;
	case __DRI_ATTRIB_CONFIG_CAVEAT:
	    if (value & __DRI_ATTRIB_NON_CONFORMANT_CONFIG)
		config->config.visualRating = GLX_NON_CONFORMANT_CONFIG;
	    else if (value & __DRI_ATTRIB_SLOW_BIT)
		config->config.visualRating = GLX_SLOW_CONFIG;
	    else
		config->config.visualRating = GLX_NONE;
	    break;
	case __DRI_ATTRIB_BIND_TO_TEXTURE_TARGETS:
	    config->config.bindToTextureTargets = 0;
	    if (value & __DRI_ATTRIB_TEXTURE_1D_BIT)
		config->config.bindToTextureTargets |= GLX_TEXTURE_1D_BIT_EXT;
	    if (value & __DRI_ATTRIB_TEXTURE_2D_BIT)
		config->config.bindToTextureTargets |= GLX_TEXTURE_2D_BIT_EXT;
	    if (value & __DRI_ATTRIB_TEXTURE_RECTANGLE_BIT)
		config->config.bindToTextureTargets |= GLX_TEXTURE_RECTANGLE_BIT_EXT;
	    break;	
	default:
	    setScalar(&config->config, attrib, value);
	    break;
	}
    }

    config->config.next = NULL;
    config->config.xRenderable = GL_TRUE;
    config->config.visualType = visualType;
    config->config.drawableType = GLX_WINDOW_BIT | GLX_PIXMAP_BIT;

    return &config->config;
}

__GLXconfig *
glxConvertConfigs(const __DRIcoreExtension *core, const __DRIconfig **configs)
{
    __GLXconfig head, *tail;
    int i;

    tail = &head;
    head.next = NULL;

    for (i = 0; configs[i]; i++) {
	tail->next = createModeFromConfig(core,
					  configs[i], GLX_TRUE_COLOR);
	if (tail->next == NULL)
	    break;

	tail = tail->next;
    }

    for (i = 0; configs[i]; i++) {
	tail->next = createModeFromConfig(core,
					  configs[i], GLX_DIRECT_COLOR);
	if (tail->next == NULL)
	    break;

	tail = tail->next;
    }

    return head.next;
}
