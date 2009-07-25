/*
 * Copyright (c) 2007, 2008 Apple Inc.
 * Copyright (c) 2004 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Greg Parker. All Rights Reserved.
 *
 * Portions of this file are copied from Mesa's xf86glx.c,
 * which contains the following copyright:
 *
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "dri.h"

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLContext.h>

#include <GL/gl.h>
#include <GL/glxproto.h>
#include <windowstr.h>
#include <resource.h>
#include <GL/glxint.h>
#include <GL/glxtokens.h>
#include <scrnintstr.h>
#include <glxserver.h>
#include <glxscreens.h>
#include <glxdrawable.h>
#include <glxcontext.h>
#include <glxext.h>
#include <glxutil.h>
#include <glxscreens.h>
#include <GL/internal/glcore.h>

#include "capabilities.h"
#include "visualConfigs.h"

/* Based originally on code from indirect.c which was based on code from i830_dri.c. */
void setVisualConfigs(void) {
    int numConfigs = 0;
    __GLXvisualConfig *visualConfigs = NULL;
    void **visualPrivates = NULL;
    struct glCapabilities caps;
    struct glCapabilitiesConfig *conf = NULL;
    int stereo, depth, aux, buffers, stencil, accum, color, msample;
    int i = 0; 
    
    if(getGlCapabilities(&caps)) {
	ErrorF("error from getGlCapabilities()!\n");
	return;
    }
    
    /*
      conf->stereo is 0 or 1, but we need at least 1 iteration of the loop, 
      so we treat a true conf->stereo as 2.

      The depth size is 0 or 24.  Thus we do 2 iterations for that.

      conf->aux_buffers (when available/non-zero) result in 2 iterations instead of 1.

      conf->buffers indicates whether we have single or double buffering.
     
      conf->total_stencil_bit_depths
       
      conf->total_color_buffers indicates the RGB/RGBA color depths.
      
      conf->total_accum_buffers iterations for accum (with at least 1 if equal to 0) 
	
      conf->total_depth_buffer_depths 

      conf->multisample_buffers iterations (with at least 1 if equal to 0).  We add 1
      for the 0 multisampling config.
      
     */

    assert(NULL != caps.configurations);
    conf = caps.configurations;
  
    numConfigs = 0;

    for(conf = caps.configurations; conf; conf = conf->next) {
	if(conf->total_color_buffers <= 0)
	    continue;

	numConfigs += (conf->stereo ? 2 : 1) 
	    * (conf->aux_buffers ? 2 : 1) 
	    * conf->buffers
	    * ((conf->total_stencil_bit_depths > 0) ? conf->total_stencil_bit_depths : 1)
	    * conf->total_color_buffers
	    * ((conf->total_accum_buffers > 0) ? conf->total_accum_buffers : 1)
	    * conf->total_depth_buffer_depths
	    * (conf->multisample_buffers + 1);
    }

    visualConfigs = xcalloc(sizeof(*visualConfigs), numConfigs);

    if(NULL == visualConfigs) {
	ErrorF("xcalloc failure when allocating visualConfigs\n");
	freeGlCapabilities(&caps);
	return;
    }
    
    visualPrivates = xcalloc(sizeof(void *), numConfigs);

    if(NULL == visualPrivates) {
	ErrorF("xcalloc failure when allocating visualPrivates");
	freeGlCapabilities(&caps);
	xfree(visualConfigs);
	return;
    }
    
    i = 0; /* current buffer */
    for(conf = caps.configurations; conf; conf = conf->next) {
	for(stereo = 0; stereo < (conf->stereo ? 2 : 1); ++stereo) {
	    for(aux = 0; aux < (conf->aux_buffers ? 2 : 1); ++aux) {
		for(buffers = 0; buffers < conf->buffers; ++buffers) {
		    for(stencil = 0; stencil < ((conf->total_stencil_bit_depths > 0) ? 
						conf->total_stencil_bit_depths : 1); ++stencil) {
			for(color = 0; color < conf->total_color_buffers; ++color) {
			    for(accum = 0; accum < ((conf->total_accum_buffers > 0) ?
						    conf->total_accum_buffers : 1); ++accum) {
				for(depth = 0; depth < conf->total_depth_buffer_depths; ++depth) {
				    for(msample = 0; msample < (conf->multisample_buffers + 1); ++msample) {
					visualConfigs[i].vid = (VisualID)(-1);
					visualConfigs[i].class = TrueColor;
					
					visualConfigs[i].rgba = true;
					visualConfigs[i].redSize = conf->color_buffers[color].r;
					visualConfigs[i].greenSize = conf->color_buffers[color].g;
					visualConfigs[i].blueSize = conf->color_buffers[color].b;

					if(GLCAPS_COLOR_BUF_INVALID_VALUE == conf->color_buffers[color].a) {
					    /* This visual has no alpha. */
					    visualConfigs[i].alphaSize = 0;
					} else {
					    visualConfigs[i].alphaSize = conf->color_buffers[color].a;
					}
	
					/* 
					 * If the .a/alpha value is unset, then don't add it to the
					 * bufferSize specification.  The INVALID_VALUE indicates that it
					 * was unset.
					 * 
					 * This prevents odd bufferSizes, such as 14.
					 */
					if(GLCAPS_COLOR_BUF_INVALID_VALUE == conf->color_buffers[color].a) {
					    visualConfigs[i].bufferSize = conf->color_buffers[color].r +
						conf->color_buffers[color].g + conf->color_buffers[color].b;
					} else {
					    visualConfigs[i].bufferSize = conf->color_buffers[color].r +
						conf->color_buffers[color].g + conf->color_buffers[color].b +
						conf->color_buffers[color].a;
					}

					/*
					 * I'm uncertain about these masks.
					 * I don't think we actually care what the values are in our
					 * libGL, so it doesn't seem to make a difference.
					 */
					visualConfigs[i].redMask = -1;
					visualConfigs[i].greenMask = -1;
					visualConfigs[i].blueMask = -1;
					visualConfigs[i].alphaMask = -1;
					
					if(conf->total_accum_buffers > 0) {
					    visualConfigs[i].accumRedSize = conf->accum_buffers[accum].r;
					    visualConfigs[i].accumGreenSize = conf->accum_buffers[accum].g;
					    visualConfigs[i].accumBlueSize = conf->accum_buffers[accum].b;
					    if(GLCAPS_COLOR_BUF_INVALID_VALUE != conf->accum_buffers[accum].a) {
						visualConfigs[i].accumAlphaSize = conf->accum_buffers[accum].a;
					    } else {
						visualConfigs[i].accumAlphaSize = 0;
					    }
					} else {
					    visualConfigs[i].accumRedSize = 0;
					    visualConfigs[i].accumGreenSize = 0;
					    visualConfigs[i].accumBlueSize = 0;
					    visualConfigs[i].accumAlphaSize = 0;
					}
					
					visualConfigs[i].doubleBuffer = buffers ? TRUE : FALSE;
					visualConfigs[i].stereo = stereo ? TRUE : FALSE;

  					visualConfigs[i].depthSize = conf->depth_buffers[depth];
				
					if(conf->total_stencil_bit_depths > 0) {
					    visualConfigs[i].stencilSize = conf->stencil_bit_depths[stencil];
					} else {
					    visualConfigs[i].stencilSize = 0;
					}
					visualConfigs[i].auxBuffers = aux ? conf->aux_buffers : 0;
					visualConfigs[i].level = 0;
				
					if(conf->accelerated) {
					    visualConfigs[i].visualRating = GLX_NONE;
					} else {
					    visualConfigs[i].visualRating = GLX_SLOW_VISUAL_EXT;
					}
					
					visualConfigs[i].transparentPixel = GLX_NONE;
					visualConfigs[i].transparentRed = GLX_NONE;
					visualConfigs[i].transparentGreen = GLX_NONE;
					visualConfigs[i].transparentBlue = GLX_NONE;
					visualConfigs[i].transparentAlpha = GLX_NONE;
					visualConfigs[i].transparentIndex = GLX_NONE;
					
					if(msample > 0) {
					    visualConfigs[i].multiSampleSize = conf->multisample_samples;
					    visualConfigs[i].nMultiSampleBuffers = conf->multisample_buffers;
					} else {
					    visualConfigs[i].multiSampleSize = 0;
					    visualConfigs[i].nMultiSampleBuffers = 0;
					}
										
					++i;
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }

    if (i != numConfigs) {
	ErrorF("numConfigs calculation error in setVisualConfigs!  numConfigs is %d  i is %d\n", numConfigs, i);
	abort();
    }

    freeGlCapabilities(&caps);

    GlxSetVisualConfigs(numConfigs, visualConfigs, visualPrivates);
}
