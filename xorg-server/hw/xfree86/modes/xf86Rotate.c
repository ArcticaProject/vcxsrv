/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "xf86.h"
#include "xf86DDC.h"
#include "fb.h"
#include "windowstr.h"
#include "xf86Crtc.h"
#include "xf86Modes.h"
#include "xf86RandR12.h"
#include "X11/extensions/render.h"
#define DPMS_SERVER
#include "X11/extensions/dpms.h"
#include "X11/Xatom.h"

/* borrowed from composite extension, move to Render and publish? */

static VisualPtr
compGetWindowVisual (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    VisualID	    vid = wVisual (pWin);
    int		    i;

    for (i = 0; i < pScreen->numVisuals; i++)
	if (pScreen->visuals[i].vid == vid)
	    return &pScreen->visuals[i];
    return 0;
}

static PictFormatPtr
compWindowFormat (WindowPtr pWin)
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    
    return PictureMatchVisual (pScreen, pWin->drawable.depth,
			       compGetWindowVisual (pWin));
}

#define F(x)	IntToxFixed(x)

static void
PictureTransformIdentity (PictTransformPtr matrix)
{
    int	i;
    memset (matrix, '\0', sizeof (PictTransform));
    for (i = 0; i < 3; i++)
	matrix->matrix[i][i] = F(1);
}

static Bool
PictureTransformMultiply (PictTransformPtr dst, PictTransformPtr l, PictTransformPtr r)
{
    PictTransform   d;
    int		    dx, dy;
    int		    o;

    for (dy = 0; dy < 3; dy++)
	for (dx = 0; dx < 3; dx++)
	{
	    xFixed_48_16    v;
	    xFixed_32_32    partial;
	    v = 0;
	    for (o = 0; o < 3; o++)
	    {
		partial = (xFixed_32_32) l->matrix[dy][o] * (xFixed_32_32) r->matrix[o][dx];
		v += partial >> 16;
	    }
	    if (v > MAX_FIXED_48_16 || v < MIN_FIXED_48_16)
		return FALSE;
	    d.matrix[dy][dx] = (xFixed) v;
	}
    *dst = d;
    return TRUE;
}

static void
PictureTransformInitScale (PictTransformPtr t, xFixed sx, xFixed sy)
{
    memset (t, '\0', sizeof (PictTransform));
    t->matrix[0][0] = sx;
    t->matrix[1][1] = sy;
    t->matrix[2][2] = F (1);
}

static xFixed
fixed_inverse (xFixed x)
{
    return (xFixed) ((((xFixed_48_16) F(1)) * F(1)) / x);
}

static Bool
PictureTransformScale (PictTransformPtr forward,
		       PictTransformPtr reverse,
		       xFixed sx, xFixed sy)
{
    PictTransform   t;
    
    PictureTransformInitScale (&t, sx, sy);
    if (!PictureTransformMultiply (forward, &t, forward))
	return FALSE;
    PictureTransformInitScale (&t, fixed_inverse (sx), fixed_inverse (sy));
    if (!PictureTransformMultiply (reverse, reverse, &t))
	return FALSE;
    return TRUE;
}

static void
PictureTransformInitRotate (PictTransformPtr t, xFixed c, xFixed s)
{
    memset (t, '\0', sizeof (PictTransform));
    t->matrix[0][0] = c;
    t->matrix[0][1] = -s;
    t->matrix[1][0] = s;
    t->matrix[1][1] = c;
    t->matrix[2][2] = F (1);
}

static Bool
PictureTransformRotate (PictTransformPtr forward,
			PictTransformPtr reverse,
			xFixed c, xFixed s)
{
    PictTransform   t;
    PictureTransformInitRotate (&t, c, s);
    if (!PictureTransformMultiply (forward, &t, forward))
	return FALSE;
    
    PictureTransformInitRotate (&t, c, -s);
    if (!PictureTransformMultiply (reverse, reverse, &t))
	return FALSE;
    return TRUE;
}

static void
PictureTransformInitTranslate (PictTransformPtr t, xFixed tx, xFixed ty)
{
    memset (t, '\0', sizeof (PictTransform));
    t->matrix[0][0] = F (1);
    t->matrix[0][2] = tx;
    t->matrix[1][1] = F (1);
    t->matrix[1][2] = ty;
    t->matrix[2][2] = F (1);
}

static Bool
PictureTransformTranslate (PictTransformPtr forward,
			   PictTransformPtr reverse,
			   xFixed tx, xFixed ty)
{
    PictTransform   t;
    PictureTransformInitTranslate (&t, tx, ty);
    if (!PictureTransformMultiply (forward, &t, forward))
	return FALSE;
    
    PictureTransformInitTranslate (&t, -tx, -ty);
    if (!PictureTransformMultiply (reverse, reverse, &t))
	return FALSE;
    return TRUE;
}

static void
PictureTransformBounds (BoxPtr b, PictTransformPtr matrix)
{
    PictVector	v[4];
    int		i;
    int		x1, y1, x2, y2;

    v[0].vector[0] = F (b->x1);    v[0].vector[1] = F (b->y1);	v[0].vector[2] = F(1);
    v[1].vector[0] = F (b->x2);    v[1].vector[1] = F (b->y1);	v[1].vector[2] = F(1);
    v[2].vector[0] = F (b->x2);    v[2].vector[1] = F (b->y2);	v[2].vector[2] = F(1);
    v[3].vector[0] = F (b->x1);    v[3].vector[1] = F (b->y2);	v[3].vector[2] = F(1);
    for (i = 0; i < 4; i++)
    {
	PictureTransformPoint (matrix, &v[i]);
	x1 = xFixedToInt (v[i].vector[0]);
	y1 = xFixedToInt (v[i].vector[1]);
	x2 = xFixedToInt (xFixedCeil (v[i].vector[0]));
	y2 = xFixedToInt (xFixedCeil (v[i].vector[1]));
	if (i == 0)
	{
	    b->x1 = x1; b->y1 = y1;
	    b->x2 = x2; b->y2 = y2;
	}
	else
	{
	    if (x1 < b->x1) b->x1 = x1;
	    if (y1 < b->y1) b->y1 = y1;
	    if (x2 > b->x2) b->x2 = x2;
	    if (y2 > b->y2) b->y2 = y2;
	}
    }
}

static Bool
PictureTransformIsIdentity(PictTransform *t)
{
    return ((t->matrix[0][0] == t->matrix[1][1]) &&
            (t->matrix[0][0] == t->matrix[2][2]) &&
            (t->matrix[0][0] != 0) &&
            (t->matrix[0][1] == 0) &&
            (t->matrix[0][2] == 0) &&
            (t->matrix[1][0] == 0) &&
            (t->matrix[1][2] == 0) &&
            (t->matrix[2][0] == 0) &&
            (t->matrix[2][1] == 0));
}

#define toF(x)	((float) (x) / 65536.0f)

static void
PictureTransformErrorF (PictTransform *t)
{
    ErrorF ("{ { %f %f %f } { %f %f %f } { %f %f %f } }",
	    toF(t->matrix[0][0]), toF(t->matrix[0][1]), toF(t->matrix[0][2]), 
	    toF(t->matrix[1][0]), toF(t->matrix[1][1]), toF(t->matrix[1][2]), 
	    toF(t->matrix[2][0]), toF(t->matrix[2][1]), toF(t->matrix[2][2]));
}

static Bool
PictureTransformIsInverse (char *where, PictTransform *a, PictTransform *b)
{
    PictTransform   t;

    PictureTransformMultiply (&t, a, b);
    if (!PictureTransformIsIdentity (&t))
    {
	ErrorF ("%s: ", where);
	PictureTransformErrorF (a);
	ErrorF (" * ");
	PictureTransformErrorF (b);
	ErrorF (" = ");
	PictureTransformErrorF (a);
	ErrorF ("\n");
	return FALSE;
    }
    return TRUE;
}

static void
xf86RotateCrtcRedisplay (xf86CrtcPtr crtc, RegionPtr region)
{
    ScrnInfoPtr		scrn = crtc->scrn;
    ScreenPtr		screen = scrn->pScreen;
    WindowPtr		root = WindowTable[screen->myNum];
    PixmapPtr		dst_pixmap = crtc->rotatedPixmap;
    PictFormatPtr	format = compWindowFormat (WindowTable[screen->myNum]);
    int			error;
    PicturePtr		src, dst;
    int			n = REGION_NUM_RECTS(region);
    BoxPtr		b = REGION_RECTS(region);
    XID			include_inferiors = IncludeInferiors;
    
    src = CreatePicture (None,
			 &root->drawable,
			 format,
			 CPSubwindowMode,
			 &include_inferiors,
			 serverClient,
			 &error);
    if (!src)
	return;

    dst = CreatePicture (None,
			 &dst_pixmap->drawable,
			 format,
			 0L,
			 NULL,
			 serverClient,
			 &error);
    if (!dst)
	return;

    error = SetPictureTransform (src, &crtc->crtc_to_framebuffer);
    if (error)
	return;

    while (n--)
    {
	BoxRec	dst_box;

	dst_box = *b;
	PictureTransformBounds (&dst_box, &crtc->framebuffer_to_crtc);
	CompositePicture (PictOpSrc,
			  src, NULL, dst,
			  dst_box.x1, dst_box.y1, 0, 0, dst_box.x1, dst_box.y1,
			  dst_box.x2 - dst_box.x1,
			  dst_box.y2 - dst_box.y1);
	b++;
    }
    FreePicture (src, None);
    FreePicture (dst, None);
}

static void
xf86CrtcDamageShadow (xf86CrtcPtr crtc)
{
    ScrnInfoPtr	pScrn = crtc->scrn;
    BoxRec	damage_box;
    RegionRec   damage_region;
    ScreenPtr	pScreen = pScrn->pScreen;

    damage_box.x1 = crtc->x;
    damage_box.x2 = crtc->x + xf86ModeWidth (&crtc->mode, crtc->rotation);
    damage_box.y1 = crtc->y;
    damage_box.y2 = crtc->y + xf86ModeHeight (&crtc->mode, crtc->rotation);
    REGION_INIT (pScreen, &damage_region, &damage_box, 1);
    DamageDamageRegion (&(*pScreen->GetScreenPixmap)(pScreen)->drawable,
			&damage_region);
    REGION_UNINIT (pScreen, &damage_region);
}

static void
xf86RotatePrepare (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int			c;

    for (c = 0; c < xf86_config->num_crtc; c++)
    {
	xf86CrtcPtr crtc = xf86_config->crtc[c];
	
	if (crtc->rotatedData && !crtc->rotatedPixmap)
	{
	    crtc->rotatedPixmap = crtc->funcs->shadow_create (crtc,
							     crtc->rotatedData,
							     crtc->mode.HDisplay,
							     crtc->mode.VDisplay);
	    if (!xf86_config->rotation_damage_registered)
	    {
		/* Hook damage to screen pixmap */
		DamageRegister (&(*pScreen->GetScreenPixmap)(pScreen)->drawable,
				xf86_config->rotation_damage);
		xf86_config->rotation_damage_registered = TRUE;
	    }
	    
	    xf86CrtcDamageShadow (crtc);
	}
    }
}

static Bool
xf86RotateRedisplay(ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    DamagePtr		damage = xf86_config->rotation_damage;
    RegionPtr		region;

    if (!damage)
	return FALSE;
    xf86RotatePrepare (pScreen);
    region = DamageRegion(damage);
    if (REGION_NOTEMPTY(pScreen, region)) 
    {
	int			c;
	SourceValidateProcPtr	SourceValidate;

	/*
	 * SourceValidate is used by the software cursor code
	 * to pull the cursor off of the screen when reading
	 * bits from the frame buffer. Bypassing this function
	 * leaves the software cursor in place
	 */
	SourceValidate = pScreen->SourceValidate;
	pScreen->SourceValidate = NULL;

	for (c = 0; c < xf86_config->num_crtc; c++)
	{
	    xf86CrtcPtr	    crtc = xf86_config->crtc[c];

	    if (crtc->rotation != RR_Rotate_0 && crtc->enabled)
	    {
		RegionRec   crtc_damage;

		/* compute portion of damage that overlaps crtc */
		REGION_INIT(pScreen, &crtc_damage, &crtc->bounds, 1);
		REGION_INTERSECT (pScreen, &crtc_damage, &crtc_damage, region);
		
		/* update damaged region */
		if (REGION_NOTEMPTY(pScreen, &crtc_damage))
    		    xf86RotateCrtcRedisplay (crtc, &crtc_damage);
		
		REGION_UNINIT (pScreen, &crtc_damage);
	    }
	}
	pScreen->SourceValidate = SourceValidate;
	DamageEmpty(damage);
    }
    return TRUE;
}

static void
xf86RotateBlockHandler(int screenNum, pointer blockData,
		       pointer pTimeout, pointer pReadmask)
{
    ScreenPtr		pScreen = screenInfo.screens[screenNum];
    ScrnInfoPtr		pScrn = xf86Screens[screenNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);

    pScreen->BlockHandler = xf86_config->BlockHandler;
    (*pScreen->BlockHandler) (screenNum, blockData, pTimeout, pReadmask);
    if (xf86RotateRedisplay(pScreen))
    {
	/* Re-wrap if rotation is still happening */
	xf86_config->BlockHandler = pScreen->BlockHandler;
	pScreen->BlockHandler = xf86RotateBlockHandler;
    }
}

static void
xf86RotateDestroy (xf86CrtcPtr crtc)
{
    ScrnInfoPtr		pScrn = crtc->scrn;
    ScreenPtr		pScreen = pScrn->pScreen;
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int			c;
    
    /* Free memory from rotation */
    if (crtc->rotatedPixmap || crtc->rotatedData)
    {
	crtc->funcs->shadow_destroy (crtc, crtc->rotatedPixmap, crtc->rotatedData);
	crtc->rotatedPixmap = NULL;
	crtc->rotatedData = NULL;
    }

    for (c = 0; c < xf86_config->num_crtc; c++)
	if (xf86_config->crtc[c]->rotatedPixmap ||
	    xf86_config->crtc[c]->rotatedData)
	    return;

    /*
     * Clean up damage structures when no crtcs are rotated
     */
    if (xf86_config->rotation_damage)
    {
	/* Free damage structure */
	if (xf86_config->rotation_damage_registered)
	{
	    DamageUnregister (&(*pScreen->GetScreenPixmap)(pScreen)->drawable,
			      xf86_config->rotation_damage);
	    xf86_config->rotation_damage_registered = FALSE;
	}
	DamageDestroy (xf86_config->rotation_damage);
	xf86_config->rotation_damage = NULL;
    }
}

_X_EXPORT void
xf86RotateCloseScreen (ScreenPtr screen)
{
    ScrnInfoPtr		scrn = xf86Screens[screen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    int			c;

    for (c = 0; c < xf86_config->num_crtc; c++)
	xf86RotateDestroy (xf86_config->crtc[c]);
}

_X_EXPORT Bool
xf86CrtcRotate (xf86CrtcPtr crtc, DisplayModePtr mode, Rotation rotation)
{
    ScrnInfoPtr		pScrn = crtc->scrn;
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    /* if this is called during ScreenInit() we don't have pScrn->pScreen yet */
    ScreenPtr		pScreen = screenInfo.screens[pScrn->scrnIndex];
    PictTransform	crtc_to_fb, fb_to_crtc;
    
    PictureTransformIdentity (&crtc_to_fb);
    PictureTransformIdentity (&fb_to_crtc);
    PictureTransformIsInverse ("identity", &crtc_to_fb, &fb_to_crtc);
    if (rotation != RR_Rotate_0)
    {
	xFixed	rot_cos, rot_sin, rot_dx, rot_dy;
	xFixed	scale_x, scale_y, scale_dx, scale_dy;
	int	mode_w = crtc->mode.HDisplay;
	int	mode_h = crtc->mode.VDisplay;
	
	/* rotation */
	switch (rotation & 0xf) {
	default:
	case RR_Rotate_0:
	    rot_cos = F ( 1);	    rot_sin = F ( 0);
	    rot_dx  = F ( 0);	    rot_dy  = F ( 0);
	    break;
	case RR_Rotate_90:
	    rot_cos = F ( 0);	    rot_sin = F ( 1);
	    rot_dx =  F ( mode_h);  rot_dy  = F (0);
	    break;
	case RR_Rotate_180:
	    rot_cos = F (-1);	    rot_sin = F ( 0);
	    rot_dx  = F (mode_w);   rot_dy  = F ( mode_h);
	    break;
	case RR_Rotate_270:
	    rot_cos = F ( 0);	    rot_sin = F (-1);
	    rot_dx  = F ( 0);	    rot_dy  = F ( mode_w);
	    break;
	}
	
	PictureTransformRotate (&crtc_to_fb, &fb_to_crtc, rot_cos, rot_sin);
	PictureTransformIsInverse ("rotate", &crtc_to_fb, &fb_to_crtc);

	PictureTransformTranslate (&crtc_to_fb, &fb_to_crtc, rot_dx, rot_dy);
	PictureTransformIsInverse ("rotate translate", &crtc_to_fb, &fb_to_crtc);

	/* reflection */
	scale_x = F (1);
	scale_dx = 0;
	scale_y = F (1);
	scale_dy = 0;
	if (rotation & RR_Reflect_X)
	{
	    scale_x = F(-1);
	    if (rotation & (RR_Rotate_0|RR_Rotate_180))
		scale_dx = F(mode_w);
	    else
		scale_dx = F(mode_h);
	}
	if (rotation & RR_Reflect_Y)
	{
	    scale_y = F(-1);
	    if (rotation & (RR_Rotate_0|RR_Rotate_180))
		scale_dy = F(mode_h);
	    else
		scale_dy = F(mode_w);
	}
	
	PictureTransformScale (&crtc_to_fb, &fb_to_crtc, scale_x, scale_y);
	PictureTransformIsInverse ("scale", &crtc_to_fb, &fb_to_crtc);

	PictureTransformTranslate (&crtc_to_fb, &fb_to_crtc, scale_dx, scale_dy);
	PictureTransformIsInverse ("scale translate", &crtc_to_fb, &fb_to_crtc);

    }
    
    /*
     * If the untranslated transformation is the identity,
     * disable the shadow buffer
     */
    if (PictureTransformIsIdentity (&crtc_to_fb))
    {
	crtc->transform_in_use = FALSE;
	PictureTransformInitTranslate (&crtc->crtc_to_framebuffer, 
				       F (-crtc->x), F (-crtc->y));
	PictureTransformInitTranslate (&crtc->framebuffer_to_crtc,
				       F ( crtc->x), F ( crtc->y));
	xf86RotateDestroy (crtc);
    }
    else
    {
	PictureTransformTranslate (&crtc_to_fb, &fb_to_crtc, F(crtc->x), F(crtc->y));
	PictureTransformIsInverse ("offset", &crtc_to_fb, &fb_to_crtc);

	/* 
	 * these are the size of the shadow pixmap, which
	 * matches the mode, not the pre-rotated copy in the
	 * frame buffer
	 */
	int	    width = mode->HDisplay;
	int	    height = mode->VDisplay;
	void	    *shadowData = crtc->rotatedData;
	PixmapPtr   shadow = crtc->rotatedPixmap;
	int	    old_width = shadow ? shadow->drawable.width : 0;
	int	    old_height = shadow ? shadow->drawable.height : 0;
	
	/* Allocate memory for rotation */
	if (old_width != width || old_height != height)
	{
	    if (shadow || shadowData)
	    {
		crtc->funcs->shadow_destroy (crtc, shadow, shadowData);
		crtc->rotatedPixmap = NULL;
		crtc->rotatedData = NULL;
	    }
	    shadowData = crtc->funcs->shadow_allocate (crtc, width, height);
	    if (!shadowData)
		goto bail1;
	    crtc->rotatedData = shadowData;
	    /* shadow will be damaged in xf86RotatePrepare */
	}
	else
	{
	    /* mark shadowed area as damaged so it will be repainted */
	    xf86CrtcDamageShadow (crtc);
	}
	
	if (!xf86_config->rotation_damage)
	{
	    /* Create damage structure */
	    xf86_config->rotation_damage = DamageCreate (NULL, NULL,
						DamageReportNone,
						TRUE, pScreen, pScreen);
	    if (!xf86_config->rotation_damage)
		goto bail2;
	    
	    /* Wrap block handler */
	    xf86_config->BlockHandler = pScreen->BlockHandler;
	    pScreen->BlockHandler = xf86RotateBlockHandler;
	}
	if (0)
	{
    bail2:
	    if (shadow || shadowData)
	    {
		crtc->funcs->shadow_destroy (crtc, shadow, shadowData);
		crtc->rotatedPixmap = NULL;
		crtc->rotatedData = NULL;
	    }
    bail1:
	    if (old_width && old_height)
		crtc->rotatedPixmap = crtc->funcs->shadow_create (crtc,
								  NULL,
								  old_width,
								  old_height);
	    return FALSE;
	}
	crtc->transform_in_use = TRUE;
	crtc->crtc_to_framebuffer = crtc_to_fb;
	crtc->framebuffer_to_crtc = fb_to_crtc;
	crtc->bounds.x1 = 0;
	crtc->bounds.x2 = crtc->mode.HDisplay;
	crtc->bounds.y1 = 0;
	crtc->bounds.y2 = crtc->mode.VDisplay;
	PictureTransformBounds (&crtc->bounds, &crtc_to_fb);
    }
    
    /* All done */
    return TRUE;
}
