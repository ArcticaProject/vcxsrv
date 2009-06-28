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

#include <GL/glxtokens.h>
#include <string.h>
#include <windowstr.h>
#include <os.h>
#include <colormapst.h>

#include "privates.h"
#include "glxserver.h"
#include "glxutil.h"
#include "glxext.h"

static DevPrivateKey glxScreenPrivateKey = &glxScreenPrivateKey;

const char GLServerVersion[] = "1.4";
static const char GLServerExtensions[] = 
			"GL_ARB_depth_texture "
			"GL_ARB_draw_buffers "
			"GL_ARB_fragment_program "
			"GL_ARB_fragment_program_shadow "
			"GL_ARB_imaging "
			"GL_ARB_multisample "
			"GL_ARB_multitexture "
			"GL_ARB_occlusion_query "
			"GL_ARB_point_parameters "
			"GL_ARB_point_sprite "
			"GL_ARB_shadow "
			"GL_ARB_shadow_ambient "
			"GL_ARB_texture_border_clamp "
			"GL_ARB_texture_compression "
			"GL_ARB_texture_cube_map "
			"GL_ARB_texture_env_add "
			"GL_ARB_texture_env_combine "
			"GL_ARB_texture_env_crossbar "
			"GL_ARB_texture_env_dot3 "
			"GL_ARB_texture_mirrored_repeat "
			"GL_ARB_texture_non_power_of_two "
			"GL_ARB_transpose_matrix "
			"GL_ARB_vertex_program "
			"GL_ARB_window_pos "
			"GL_EXT_abgr "
			"GL_EXT_bgra "
 			"GL_EXT_blend_color "
			"GL_EXT_blend_equation_separate "
			"GL_EXT_blend_func_separate "
			"GL_EXT_blend_logic_op "
 			"GL_EXT_blend_minmax "
 			"GL_EXT_blend_subtract "
			"GL_EXT_clip_volume_hint "
			"GL_EXT_copy_texture "
			"GL_EXT_draw_range_elements "
			"GL_EXT_fog_coord "
			"GL_EXT_framebuffer_object "
			"GL_EXT_multi_draw_arrays "
			"GL_EXT_packed_pixels "
			"GL_EXT_paletted_texture "
			"GL_EXT_point_parameters "
			"GL_EXT_polygon_offset "
			"GL_EXT_rescale_normal "
			"GL_EXT_secondary_color "
			"GL_EXT_separate_specular_color "
			"GL_EXT_shadow_funcs "
			"GL_EXT_shared_texture_palette "
 			"GL_EXT_stencil_two_side "
			"GL_EXT_stencil_wrap "
			"GL_EXT_subtexture "
			"GL_EXT_texture "
			"GL_EXT_texture3D "
			"GL_EXT_texture_compression_dxt1 "
			"GL_EXT_texture_compression_s3tc "
			"GL_EXT_texture_edge_clamp "
 			"GL_EXT_texture_env_add "
 			"GL_EXT_texture_env_combine "
 			"GL_EXT_texture_env_dot3 "
 			"GL_EXT_texture_filter_anisotropic "
			"GL_EXT_texture_lod "
 			"GL_EXT_texture_lod_bias "
 			"GL_EXT_texture_mirror_clamp "
			"GL_EXT_texture_object "
			"GL_EXT_texture_rectangle "
			"GL_EXT_vertex_array "
			"GL_3DFX_texture_compression_FXT1 "
			"GL_APPLE_packed_pixels "
			"GL_ATI_draw_buffers "
			"GL_ATI_texture_env_combine3 "
			"GL_ATI_texture_mirror_once "
 			"GL_HP_occlusion_test "
			"GL_IBM_texture_mirrored_repeat "
			"GL_INGR_blend_func_separate "
			"GL_MESA_pack_invert "
			"GL_MESA_ycbcr_texture "
			"GL_NV_blend_square "
			"GL_NV_depth_clamp "
			"GL_NV_fog_distance "
			"GL_NV_fragment_program "
			"GL_NV_fragment_program_option "
			"GL_NV_fragment_program2 "
			"GL_NV_light_max_exponent "
			"GL_NV_multisample_filter_hint "
			"GL_NV_point_sprite "
			"GL_NV_texgen_reflection "
			"GL_NV_texture_compression_vtc "
			"GL_NV_texture_env_combine4 "
			"GL_NV_texture_expand_normal "
			"GL_NV_texture_rectangle "
			"GL_NV_vertex_program "
			"GL_NV_vertex_program1_1 "
			"GL_NV_vertex_program2 "
			"GL_NV_vertex_program2_option "
			"GL_NV_vertex_program3 "
			"GL_OES_compressed_paletted_texture "
			"GL_SGI_color_matrix "
			"GL_SGI_color_table "
			"GL_SGIS_generate_mipmap "
			"GL_SGIS_multisample "
			"GL_SGIS_point_parameters "
			"GL_SGIS_texture_border_clamp "
			"GL_SGIS_texture_edge_clamp "
			"GL_SGIS_texture_lod "
			"GL_SGIX_depth_texture "
			"GL_SGIX_shadow "
			"GL_SGIX_shadow_ambient "
			"GL_SUN_slice_accum "
			;

/*
** We have made the simplifying assuption that the same extensions are 
** supported across all screens in a multi-screen system.
*/
static char GLXServerVendorName[] = "SGI";
static char GLXServerVersion[] = "1.2";
static char GLXServerExtensions[] =
			"GLX_ARB_multisample "
			"GLX_EXT_visual_info "
			"GLX_EXT_visual_rating "
			"GLX_EXT_import_context "
                        "GLX_EXT_texture_from_pixmap "
			"GLX_OML_swap_method "
			"GLX_SGI_make_current_read "
#ifndef __APPLE__
			"GLX_SGIS_multisample "
                        "GLX_SGIX_hyperpipe "
                        "GLX_SGIX_swap_barrier "
#endif
			"GLX_SGIX_fbconfig "
			"GLX_MESA_copy_sub_buffer "
			;

/*
** This hook gets called when a window moves or changes size.
*/
static Bool glxPositionWindow(WindowPtr pWin, int x, int y)
{
    ScreenPtr pScreen;
    __GLXdrawable *glxPriv;
    Bool ret;
    __GLXscreen *pGlxScreen;

    /*
    ** Call wrapped position window routine
    */
    pScreen = pWin->drawable.pScreen;
    pGlxScreen = glxGetScreen(pScreen);
    pScreen->PositionWindow = pGlxScreen->PositionWindow;
    ret = (*pScreen->PositionWindow)(pWin, x, y);
    pScreen->PositionWindow = glxPositionWindow;

    /*
    ** Tell all contexts rendering into this window that the window size
    ** has changed.
    */
    glxPriv = (__GLXdrawable *) LookupIDByType(pWin->drawable.id,
					       __glXDrawableRes);
    if (glxPriv == NULL) {
	/*
	** This window is not being used by the OpenGL.
	*/
	return ret;
    }

    /*
    ** resize the drawable
    */
    /* first change the drawable size */
    if (glxPriv->resize(glxPriv) == GL_FALSE) {
	/* resize failed! */
	/* XXX: what can we possibly do here? */
	ret = False;
    }

    return ret;
}

/*
 * If your DDX driver wants to register support for swap barriers or hyperpipe
 * topology, it should call __glXHyperpipeInit() or __glXSwapBarrierInit()
 * with a dispatch table of functions to handle the requests.   In the XFree86
 * DDX, for example, you would call these near the bottom of the driver's
 * ScreenInit method, after DRI has been initialized.
 *
 * This should be replaced with a better method when we teach the server how
 * to load DRI drivers.
 */

void __glXHyperpipeInit(int screen, __GLXHyperpipeExtensionFuncs *funcs)
{
    __GLXscreen *pGlxScreen = glxGetScreen(screenInfo.screens[screen]);

    pGlxScreen->hyperpipeFuncs = funcs;
}

void __glXSwapBarrierInit(int screen, __GLXSwapBarrierExtensionFuncs *funcs)
{
    __GLXscreen *pGlxScreen = glxGetScreen(screenInfo.screens[screen]);

    pGlxScreen->swapBarrierFuncs = funcs;
}

static Bool
glxCloseScreen (int index, ScreenPtr pScreen)
{
    __GLXscreen *pGlxScreen = glxGetScreen(pScreen);

    pScreen->CloseScreen = pGlxScreen->CloseScreen;
    pScreen->PositionWindow = pGlxScreen->PositionWindow;

    pGlxScreen->destroy(pGlxScreen);

    return pScreen->CloseScreen(index, pScreen);
}

__GLXscreen *
glxGetScreen(ScreenPtr pScreen)
{
    return dixLookupPrivate(&pScreen->devPrivates, glxScreenPrivateKey);
}

void GlxSetVisualConfigs(int nconfigs, 
                         __GLXvisualConfig *configs, void **privates)
{
    /* We keep this stub around for the DDX drivers that still
     * call it. */
}

GLint glxConvertToXVisualType(int visualType)
{
    static const int x_visual_types[] = {
	TrueColor,   DirectColor,
	PseudoColor, StaticColor,
	GrayScale,   StaticGray
    };

    return ( (unsigned) (visualType - GLX_TRUE_COLOR) < 6 )
	? x_visual_types[ visualType - GLX_TRUE_COLOR ] : -1;
}


static void
filterOutNativeConfigs(__GLXscreen *pGlxScreen)
{
    __GLXconfig *m, *next, **last;
    ScreenPtr pScreen = pGlxScreen->pScreen;
    int i, depth;

    last = &pGlxScreen->fbconfigs;
    for (m = pGlxScreen->fbconfigs; m != NULL; m = next) {
	next = m->next;
	depth = m->redBits + m->blueBits + m->greenBits;

	for (i = 0; i < pScreen->numVisuals; i++) {
	    if (pScreen->visuals[i].nplanes == depth) {
		*last = m;
		last = &m->next;
		break;
	    }
	}
    }

    *last = NULL;
}

static XID
findVisualForConfig(ScreenPtr pScreen, __GLXconfig *m)
{
    int i;

    for (i = 0; i < pScreen->numVisuals; i++) {
	if (glxConvertToXVisualType(m->visualType) == pScreen->visuals[i].class)
	    return pScreen->visuals[i].vid;
    }

    return 0;
}

/* This code inspired by composite/compinit.c.  We could move this to
 * mi/ and share it with composite.*/

static VisualPtr
AddScreenVisuals(ScreenPtr pScreen, int count, int d)
{
    XID		*installedCmaps, *vids, vid;
    int		 numInstalledCmaps, numVisuals, i, j;
    VisualPtr	 visuals;
    ColormapPtr	 installedCmap;
    DepthPtr	 depth;

    depth = NULL;
    for (i = 0; i < pScreen->numDepths; i++) {
	if (pScreen->allowedDepths[i].depth == d) {
	    depth = &pScreen->allowedDepths[i];
	    break;
	}
    }
    if (depth == NULL)
	return NULL;

    /* Find the installed colormaps */
    installedCmaps = xalloc (pScreen->maxInstalledCmaps * sizeof (XID));
    if (!installedCmaps)
	return NULL;

    numInstalledCmaps = pScreen->ListInstalledColormaps(pScreen, installedCmaps);

    /* realloc the visual array to fit the new one in place */
    numVisuals = pScreen->numVisuals;
    visuals = xrealloc(pScreen->visuals, (numVisuals + count) * sizeof(VisualRec));
    if (!visuals) {
	xfree(installedCmaps);
	return NULL;
    }

    vids = xrealloc(depth->vids, (depth->numVids + count) * sizeof(XID));
    if (vids == NULL) {
	xfree(installedCmaps);
	xfree(visuals);
	return NULL;
    }

    /*
     * Fix up any existing installed colormaps -- we'll assume that
     * the only ones created so far have been installed.  If this
     * isn't true, we'll have to walk the resource database looking
     * for all colormaps.
     */
    for (i = 0; i < numInstalledCmaps; i++) {
	installedCmap = LookupIDByType (installedCmaps[i], RT_COLORMAP);
	if (!installedCmap)
	    continue;
	j = installedCmap->pVisual - pScreen->visuals;
	installedCmap->pVisual = &visuals[j];
    }

    xfree(installedCmaps);

    for (i = 0; i < count; i++) {
	vid = FakeClientID(0);
	visuals[pScreen->numVisuals + i].vid = vid;
	vids[depth->numVids + i] = vid;
    }

    pScreen->visuals = visuals;
    pScreen->numVisuals += count;
    depth->vids = vids;
    depth->numVids += count;

    /* Return a pointer to the first of the added visuals. */ 
    return pScreen->visuals + pScreen->numVisuals - count;
}

static int
findFirstSet(unsigned int v)
{
    int i;

    for (i = 0; i < 32; i++)
	if (v & (1 << i))
	    return i;

    return -1;
}

static void
initGlxVisual(VisualPtr visual, __GLXconfig *config)
{
    int maxBits;
    maxBits = max(config->redBits, max(config->greenBits, config->blueBits));

    config->visualID = visual->vid;
    visual->class = glxConvertToXVisualType(config->visualType);
    visual->bitsPerRGBValue = maxBits;
    visual->ColormapEntries = 1 << maxBits;
    visual->nplanes = config->redBits + config->greenBits + config->blueBits;

    visual->redMask = config->redMask;
    visual->greenMask = config->greenMask;
    visual->blueMask = config->blueMask;
    visual->offsetRed = findFirstSet(config->redMask);
    visual->offsetGreen = findFirstSet(config->greenMask);
    visual->offsetBlue = findFirstSet(config->blueMask);
}

typedef struct {
    GLboolean doubleBuffer;
    GLboolean depthBuffer;
    GLboolean stencilBuffer;
} FBConfigTemplateRec, *FBConfigTemplatePtr;

static __GLXconfig *
pickFBConfig(__GLXscreen *pGlxScreen, FBConfigTemplatePtr template, int class)
{
    __GLXconfig *config;

    for (config = pGlxScreen->fbconfigs; config != NULL; config = config->next) {
	if (config->visualRating != GLX_NONE)
	    continue;
	if (glxConvertToXVisualType(config->visualType) != class)
	    continue;
	if ((config->doubleBufferMode > 0) != template->doubleBuffer)
	    continue;
	if ((config->depthBits > 0) != template->depthBuffer)
	    continue;
	if ((config->stencilBits > 0) != template->stencilBuffer)
	    continue;

	return config;
    }

    return NULL;
}

static void
addMinimalSet(__GLXscreen *pGlxScreen)
{
    __GLXconfig *config;
    VisualPtr visuals;
    int i, j;
    FBConfigTemplateRec best = { GL_TRUE, GL_TRUE, GL_TRUE };
    FBConfigTemplateRec good = { GL_TRUE, GL_TRUE, GL_FALSE };
    FBConfigTemplateRec minimal = { GL_FALSE, GL_FALSE, GL_FALSE };

    pGlxScreen->visuals = xcalloc(pGlxScreen->pScreen->numVisuals,
				  sizeof (__GLXconfig *));
    if (pGlxScreen->visuals == NULL) {
	ErrorF("Failed to allocate for minimal set of GLX visuals\n");
	return;
    }

    visuals = pGlxScreen->pScreen->visuals;
    for (i = 0, j = 0; i < pGlxScreen->pScreen->numVisuals; i++) {
	if (visuals[i].nplanes == 32)
	    config = pickFBConfig(pGlxScreen, &minimal, visuals[i].class);
	else {
	    config = pickFBConfig(pGlxScreen, &best, visuals[i].class);
	    if (config == NULL)
		config = pickFBConfig(pGlxScreen, &good, visuals[i].class);
        }
	if (config == NULL)
	    config = pGlxScreen->fbconfigs;
	if (config == NULL)
	    continue;

	pGlxScreen->visuals[j] = config;
	config->visualID = visuals[i].vid;
	j++;
    }

    pGlxScreen->numVisuals = j;
}

static void
addTypicalSet(__GLXscreen *pGlxScreen)
{
    addMinimalSet(pGlxScreen);
}

static void
addFullSet(__GLXscreen *pGlxScreen)
{
    __GLXconfig *config;
    VisualPtr visuals;
    int i, depth;

    pGlxScreen->visuals =
	xcalloc(pGlxScreen->numFBConfigs, sizeof (__GLXconfig *));
    if (pGlxScreen->visuals == NULL) {
	ErrorF("Failed to allocate for full set of GLX visuals\n");
	return;
    }

    config = pGlxScreen->fbconfigs;
    depth = config->redBits + config->greenBits + config->blueBits;
    visuals = AddScreenVisuals(pGlxScreen->pScreen, pGlxScreen->numFBConfigs, depth);
    if (visuals == NULL) {
	xfree(pGlxScreen->visuals);
	return;
    }

    pGlxScreen->numVisuals = pGlxScreen->numFBConfigs;
    for (i = 0, config = pGlxScreen->fbconfigs; config; config = config->next, i++) {
	pGlxScreen->visuals[i] = config;
	initGlxVisual(&visuals[i], config);
    }
}

static int glxVisualConfig = GLX_ALL_VISUALS;

void GlxSetVisualConfig(int config)
{
    glxVisualConfig = config;
}

void __glXScreenInit(__GLXscreen *pGlxScreen, ScreenPtr pScreen)
{
    __GLXconfig *m;
    int i;

    pGlxScreen->pScreen       = pScreen;
    pGlxScreen->GLextensions  = xstrdup(GLServerExtensions);
    pGlxScreen->GLXvendor     = xstrdup(GLXServerVendorName);
    pGlxScreen->GLXversion    = xstrdup(GLXServerVersion);
    pGlxScreen->GLXextensions = xstrdup(GLXServerExtensions);

    pGlxScreen->PositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow = glxPositionWindow;
 
    pGlxScreen->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = glxCloseScreen;

    filterOutNativeConfigs(pGlxScreen);

    i = 0;
    for (m = pGlxScreen->fbconfigs; m != NULL; m = m->next) {
	m->fbconfigID = FakeClientID(0);
	m->visualID = findVisualForConfig(pScreen, m);
	i++;
    }
    pGlxScreen->numFBConfigs = i;

    /* Select a subset of fbconfigs that we send to the client when it
     * asks for the glx visuals.  All the fbconfigs here have a valid
     * value for visual ID and each visual ID is only present once.
     * This runs before composite adds its extra visual so we have to
     * remember the number of visuals here.*/

    switch (glxVisualConfig) {
    case GLX_MINIMAL_VISUALS:
	addMinimalSet(pGlxScreen);
	break;
    case GLX_TYPICAL_VISUALS:
	addTypicalSet(pGlxScreen);
	break;
    case GLX_ALL_VISUALS:
	addFullSet(pGlxScreen);
	break;
    }

    dixSetPrivate(&pScreen->devPrivates, glxScreenPrivateKey, pGlxScreen);
}

void __glXScreenDestroy(__GLXscreen *screen)
{
    xfree(screen->GLXvendor);
    xfree(screen->GLXversion);
    xfree(screen->GLXextensions);
    xfree(screen->GLextensions);
}
