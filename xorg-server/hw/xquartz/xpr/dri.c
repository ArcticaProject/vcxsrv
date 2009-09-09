/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
Copyright (c) 2002, 2009 Apple Computer, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Jens Owen <jens@valinux.com>
 *   Rickard E. (Rik) Faith <faith@valinux.com>
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef XFree86LOADER
#include "xf86.h"
#include "xf86_ansic.h"
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"
#define _APPLEDRI_SERVER_
#include "appledristr.h"
#include "swaprep.h"
#include "dri.h"
#include "dristruct.h"
#include "mi.h"
#include "mipointer.h"
#include "rootless.h"
#include "x-hash.h"
#include "x-hook.h"
#include "driWrap.h"

#include <AvailabilityMacros.h>

static int DRIScreenPrivKeyIndex;
static DevPrivateKey DRIScreenPrivKey = &DRIScreenPrivKeyIndex;
static int DRIWindowPrivKeyIndex;
static DevPrivateKey DRIWindowPrivKey = &DRIWindowPrivKeyIndex;
static int DRIPixmapPrivKeyIndex;
static DevPrivateKey DRIPixmapPrivKey = &DRIPixmapPrivKeyIndex;
static int DRIPixmapBufferPrivKeyIndex;
static DevPrivateKey DRIPixmapBufferPrivKey = &DRIPixmapBufferPrivKeyIndex;

static RESTYPE DRIDrawablePrivResType;

static x_hash_table *surface_hash;      /* maps surface ids -> drawablePrivs */

static Bool DRIFreePixmapImp(DrawablePtr pDrawable);

/* FIXME: don't hardcode this? */
#define CG_INFO_FILE "/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Resources/Info-macos.plist"

/* Corresponds to SU Jaguar Green */
#define CG_REQUIRED_MAJOR 1
#define CG_REQUIRED_MINOR 157
#define CG_REQUIRED_MICRO 11

typedef struct {
    DrawablePtr pDrawable;
    int refCount;
    int bytesPerPixel;
    int width;
    int height;
    char shmPath[PATH_MAX];
    int fd; /* From shm_open (for now) */
    size_t length; /* length of buffer */
    void *buffer;       
} DRIPixmapBuffer, *DRIPixmapBufferPtr;

/* Returns version as major.minor.micro in 10.10.10 fixed form */
static unsigned int
get_cg_version (void)
{
    static unsigned int version;

    FILE *fh;
    char *ptr;

    if (version != 0)
        return version;

    /* I tried CFBundleGetVersion, but it returns zero, so.. */

    fh = fopen (CG_INFO_FILE, "r");
    if (fh != NULL)
    {
        char buf[256];

        while (fgets (buf, sizeof (buf), fh) != NULL)
        {
            unsigned char c;

            if (!strstr (buf, "<key>CFBundleShortVersionString</key>")
                || fgets (buf, sizeof (buf), fh) == NULL)
            {
                continue;
            }

            ptr = strstr (buf, "<string>");
            if (ptr == NULL)
                continue;

            ptr += strlen ("<string>");

            /* Now PTR points to "MAJOR.MINOR.MICRO". */

            version = 0;

        again:
            switch ((c = *ptr++))
            {
            case '.':
                version = version * 1024;
                goto again;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                version = ((version & ~0x3ff)
                          + (version & 0x3ff) * 10 + (c - '0'));
                goto again;
            }
            break;
        }

        fclose (fh);
    }

    return version;
}

static Bool
test_cg_version (unsigned int major, unsigned int minor, unsigned int micro)
{
    unsigned int cg_ver = get_cg_version ();

    unsigned int cg_major = (cg_ver >> 20) & 0x3ff;
    unsigned int cg_minor = (cg_ver >> 10) & 0x3ff;
    unsigned int cg_micro =  cg_ver        & 0x3ff;

    if (cg_major > major)
        return TRUE;
    else if (cg_major < major)
        return FALSE;

    /* cg_major == major */

    if (cg_minor > minor)
        return TRUE;
    else if (cg_minor < minor)
        return FALSE;

    /* cg_minor == minor */

    if (cg_micro < micro)
        return FALSE;

    return TRUE;
}

Bool
DRIScreenInit(ScreenPtr pScreen)
{
    DRIScreenPrivPtr    pDRIPriv;
    int                 i;

    pDRIPriv = (DRIScreenPrivPtr) xcalloc(1, sizeof(DRIScreenPrivRec));
    if (!pDRIPriv) {
	dixSetPrivate(&pScreen->devPrivates, DRIScreenPrivKey, NULL);
        return FALSE;
    }

    dixSetPrivate(&pScreen->devPrivates, DRIScreenPrivKey, pDRIPriv);
    pDRIPriv->directRenderingSupport = TRUE;
    pDRIPriv->nrWindows = 0;

    /* Need recent cg for window access update */
    if (!test_cg_version (CG_REQUIRED_MAJOR,
                          CG_REQUIRED_MINOR,
                          CG_REQUIRED_MICRO))
    {
        ErrorF ("[DRI] disabled direct rendering; requires CoreGraphics %d.%d.%d\n",
                CG_REQUIRED_MAJOR, CG_REQUIRED_MINOR, CG_REQUIRED_MICRO);

        pDRIPriv->directRenderingSupport = FALSE;

        /* Note we don't nuke the dri private, since we need it for
           managing indirect surfaces. */
    }

    /* Initialize drawable tables */
    for (i = 0; i < DRI_MAX_DRAWABLES; i++) {
        pDRIPriv->DRIDrawables[i] = NULL;
    }

    return TRUE;
}

Bool
DRIFinishScreenInit(ScreenPtr pScreen)
{
    DRIScreenPrivPtr  pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    /* Wrap DRI support */
    pDRIPriv->wrap.ValidateTree = pScreen->ValidateTree;
    pScreen->ValidateTree = DRIValidateTree;

    pDRIPriv->wrap.PostValidateTree = pScreen->PostValidateTree;
    pScreen->PostValidateTree = DRIPostValidateTree;

    pDRIPriv->wrap.WindowExposures = pScreen->WindowExposures;
    pScreen->WindowExposures = DRIWindowExposures;

    pDRIPriv->wrap.CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = DRICopyWindow;

    pDRIPriv->wrap.ClipNotify = pScreen->ClipNotify;
    pScreen->ClipNotify = DRIClipNotify;

    //    ErrorF("[DRI] screen %d installation complete\n", pScreen->myNum);

    return DRIWrapInit(pScreen);
}

void
DRICloseScreen(ScreenPtr pScreen)
{
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv && pDRIPriv->directRenderingSupport) {
        xfree(pDRIPriv);
	dixSetPrivate(&pScreen->devPrivates, DRIScreenPrivKey, NULL);
    }
}

Bool
DRIExtensionInit(void)
{
    DRIDrawablePrivResType = CreateNewResourceType(DRIDrawablePrivDelete);

    return TRUE;
}

void
DRIReset(void)
{
    /*
     * This stub routine is called when the X Server recycles, resources
     * allocated by DRIExtensionInit need to be managed here.
     *
     * Currently this routine is a stub because all the interesting resources
     * are managed via the screen init process.
     */
}

Bool
DRIQueryDirectRenderingCapable(ScreenPtr pScreen, Bool* isCapable)
{
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv)
        *isCapable = pDRIPriv->directRenderingSupport;
    else
        *isCapable = FALSE;

    return TRUE;
}

Bool
DRIAuthConnection(ScreenPtr pScreen, unsigned int magic)
{
#if 0
    /* FIXME: something? */

    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (drmAuthMagic(pDRIPriv->drmFD, magic)) return FALSE;
#endif
    return TRUE;
}

static void
DRIUpdateSurface(DRIDrawablePrivPtr pDRIDrawablePriv, DrawablePtr pDraw)
{
    xp_window_changes wc;
    unsigned int flags = 0;

    if (pDRIDrawablePriv->sid == 0)
        return;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    wc.depth = (pDraw->bitsPerPixel == 32 ? XP_DEPTH_ARGB8888
                : pDraw->bitsPerPixel == 16 ? XP_DEPTH_RGB555 : XP_DEPTH_NIL);
    if (wc.depth != XP_DEPTH_NIL)
        flags |= XP_DEPTH;
#endif

    if (pDraw->type == DRAWABLE_WINDOW) {
        WindowPtr pWin = (WindowPtr) pDraw;
        WindowPtr pTopWin = TopLevelParent(pWin);

        wc.x = pWin->drawable.x - (pTopWin->drawable.x - pTopWin->borderWidth);
        wc.y = pWin->drawable.y - (pTopWin->drawable.y - pTopWin->borderWidth);
        wc.width = pWin->drawable.width + 2 * pWin->borderWidth;
        wc.height = pWin->drawable.height + 2 * pWin->borderWidth;
        wc.bit_gravity = XP_GRAVITY_NONE;

        wc.shape_nrects = REGION_NUM_RECTS(&pWin->clipList);
        wc.shape_rects = REGION_RECTS(&pWin->clipList);
        wc.shape_tx = - (pTopWin->drawable.x - pTopWin->borderWidth);
        wc.shape_ty = - (pTopWin->drawable.y - pTopWin->borderWidth);

        flags |= XP_BOUNDS | XP_SHAPE;

    } else if (pDraw->type == DRAWABLE_PIXMAP) {
        wc.x = 0;
        wc.y = 0;
        wc.width = pDraw->width;
        wc.height = pDraw->height;
        wc.bit_gravity = XP_GRAVITY_NONE;
        flags |= XP_BOUNDS;
    }

    xp_configure_surface(pDRIDrawablePriv->sid, flags, &wc);
}

/* Return NULL if an error occurs. */
static DRIDrawablePrivPtr
CreateSurfaceForWindow(ScreenPtr pScreen, WindowPtr pWin, xp_window_id *widPtr) {
    DRIDrawablePrivPtr pDRIDrawablePriv;
    xp_window_id wid = 0;

    *widPtr = 0;

    pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);

    if (pDRIDrawablePriv == NULL) {
	xp_error err;
	xp_window_changes wc;
	
	/* allocate a DRI Window Private record */
	if (!(pDRIDrawablePriv = xalloc(sizeof(*pDRIDrawablePriv)))) {
	    return NULL;
	}
	
	pDRIDrawablePriv->pDraw = (DrawablePtr)pWin;
	pDRIDrawablePriv->pScreen = pScreen;
	pDRIDrawablePriv->refCount = 0;
	pDRIDrawablePriv->drawableIndex = -1;
	pDRIDrawablePriv->notifiers = NULL;
	
	/* find the physical window */
	wid = x_cvt_vptr_to_uint(RootlessFrameForWindow(pWin, TRUE));

	if (wid == 0) {
	    xfree(pDRIDrawablePriv);
	    return NULL;
	}
	
	/* allocate the physical surface */
	err = xp_create_surface(wid, &pDRIDrawablePriv->sid);

	if (err != Success) {
	    xfree(pDRIDrawablePriv);
	    return NULL;
	}

	/* Make it visible */
	wc.stack_mode = XP_MAPPED_ABOVE;
	wc.sibling = 0;
	err = xp_configure_surface(pDRIDrawablePriv->sid, XP_STACKING, &wc);

	if (err != Success) {
	    xp_destroy_surface(pDRIDrawablePriv->sid);
	    xfree(pDRIDrawablePriv);
	    return NULL;
	}

	/* save private off of preallocated index */
	dixSetPrivate(&pWin->devPrivates, DRIWindowPrivKey,
		      pDRIDrawablePriv);
    }

    *widPtr = wid;

    return pDRIDrawablePriv;
}

/* Return NULL if an error occurs. */
static DRIDrawablePrivPtr
CreateSurfaceForPixmap(ScreenPtr pScreen, PixmapPtr pPix) {
    DRIDrawablePrivPtr pDRIDrawablePriv;
     
    pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP(pPix);

    if (pDRIDrawablePriv == NULL) {
	xp_error err;

	/* allocate a DRI Window Private record */
	if (!(pDRIDrawablePriv = xcalloc(1, sizeof(*pDRIDrawablePriv)))) {
	    return NULL;
	}
	
	pDRIDrawablePriv->pDraw = (DrawablePtr)pPix;
	pDRIDrawablePriv->pScreen = pScreen;
	pDRIDrawablePriv->refCount = 0;
	pDRIDrawablePriv->drawableIndex = -1;
	pDRIDrawablePriv->notifiers = NULL;
	
	/* Passing a null window id to Xplugin in 10.3+ asks for
	   an accelerated offscreen surface. */
	
	err = xp_create_surface(0, &pDRIDrawablePriv->sid);
	if (err != Success) {
	    xfree(pDRIDrawablePriv);
	    return NULL;
	}

	/* 
	 * The DRIUpdateSurface will be called to resize the surface
	 * after this function, if the export is successful.
	 */

	/* save private off of preallocated index */
	dixSetPrivate(&pPix->devPrivates, DRIPixmapPrivKey,
		      pDRIDrawablePriv);
    }
    
    return pDRIDrawablePriv;
}


Bool
DRICreateSurface(ScreenPtr pScreen, Drawable id,
                 DrawablePtr pDrawable, xp_client_id client_id,
                 xp_surface_id *surface_id, unsigned int ret_key[2],
                 void (*notify) (void *arg, void *data), void *notify_data)
{
    DRIScreenPrivPtr    pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    xp_window_id        wid = 0;
    DRIDrawablePrivPtr  pDRIDrawablePriv;

    if (pDrawable->type == DRAWABLE_WINDOW) {
	pDRIDrawablePriv = CreateSurfaceForWindow(pScreen, 
						  (WindowPtr)pDrawable, &wid);

	if(NULL == pDRIDrawablePriv)
	    return FALSE; /*error*/
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1030
    else if (pDrawable->type == DRAWABLE_PIXMAP) {
	pDRIDrawablePriv = CreateSurfaceForPixmap(pScreen, 
						  (PixmapPtr)pDrawable);

	if(NULL == pDRIDrawablePriv)
	    return FALSE; /*error*/
    }
#endif
    else { /* for GLX 1.3, a PBuffer */
        /* NOT_DONE */
        return FALSE;
    }
    
    
    /* Finish initialization of new surfaces */
    if (pDRIDrawablePriv->refCount == 0) {
        unsigned int key[2] = {0};
        xp_error err;

        /* try to give the client access to the surface */
        if (client_id != 0) {
	    /*
	     * Xplugin accepts a 0 wid if the surface id is offscreen, such 
	     * as for a pixmap.
	     */
            err = xp_export_surface(wid, pDRIDrawablePriv->sid,
                                    client_id, key);
            if (err != Success) {
                xp_destroy_surface(pDRIDrawablePriv->sid);
                xfree(pDRIDrawablePriv);
		
		/* 
		 * Now set the dix privates to NULL that were previously set.
		 * This prevents reusing an invalid pointer.
		 */
		if(pDrawable->type == DRAWABLE_WINDOW) {
		    WindowPtr pWin = (WindowPtr)pDrawable;
		    
		    dixSetPrivate(&pWin->devPrivates, DRIWindowPrivKey, NULL);
		} else if(pDrawable->type == DRAWABLE_PIXMAP) {
		    PixmapPtr pPix = (PixmapPtr)pDrawable;
		    
		    dixSetPrivate(&pPix->devPrivates, DRIPixmapPrivKey, NULL);
		}
		
                return FALSE;
            }
        }

        pDRIDrawablePriv->key[0] = key[0];
        pDRIDrawablePriv->key[1] = key[1];

        ++pDRIPriv->nrWindows;

        /* and stash it by surface id */
        if (surface_hash == NULL)
            surface_hash = x_hash_table_new(NULL, NULL, NULL, NULL);
        x_hash_table_insert(surface_hash,
                            x_cvt_uint_to_vptr(pDRIDrawablePriv->sid), pDRIDrawablePriv);

        /* track this in case this window is destroyed */
        AddResource(id, DRIDrawablePrivResType, (pointer)pDrawable);

        /* Initialize shape */
        DRIUpdateSurface(pDRIDrawablePriv, pDrawable);
    }

    pDRIDrawablePriv->refCount++;

    *surface_id = pDRIDrawablePriv->sid;

    if (ret_key != NULL) {
        ret_key[0] = pDRIDrawablePriv->key[0];
        ret_key[1] = pDRIDrawablePriv->key[1];
    }

    if (notify != NULL) {
        pDRIDrawablePriv->notifiers = x_hook_add(pDRIDrawablePriv->notifiers,
                                                 notify, notify_data);
    }

    return TRUE;
}

Bool
DRIDestroySurface(ScreenPtr pScreen, Drawable id, DrawablePtr pDrawable,
                  void (*notify) (void *, void *), void *notify_data)
{
    DRIDrawablePrivPtr  pDRIDrawablePriv;

    if (pDrawable->type == DRAWABLE_WINDOW) {
        pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW((WindowPtr)pDrawable);
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
        pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP((PixmapPtr)pDrawable);
    } else {
        return FALSE;
    }

    if (pDRIDrawablePriv != NULL) {
        if (notify != NULL) {
            pDRIDrawablePriv->notifiers = x_hook_remove(pDRIDrawablePriv->notifiers,
                                                        notify, notify_data);
        }
        if (--pDRIDrawablePriv->refCount <= 0) {
            /* This calls back to DRIDrawablePrivDelete
               which frees the private area */
            FreeResourceByType(id, DRIDrawablePrivResType, FALSE);
        }
    }

    return TRUE;
}

Bool
DRIDrawablePrivDelete(pointer pResource, XID id)
{
    DrawablePtr         pDrawable = (DrawablePtr)pResource;
    DRIScreenPrivPtr    pDRIPriv = DRI_SCREEN_PRIV(pDrawable->pScreen);
    DRIDrawablePrivPtr  pDRIDrawablePriv = NULL;
    WindowPtr           pWin = NULL;
    PixmapPtr           pPix = NULL;

    if (pDrawable->type == DRAWABLE_WINDOW) {
        pWin = (WindowPtr)pDrawable;
        pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
        pPix = (PixmapPtr)pDrawable;
        pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_PIXMAP(pPix);
    }

    if (pDRIDrawablePriv == NULL) {
	return DRIFreePixmapImp(pDrawable);
    }

    if (pDRIDrawablePriv->drawableIndex != -1) {
        /* release drawable table entry */
        pDRIPriv->DRIDrawables[pDRIDrawablePriv->drawableIndex] = NULL;
    }

    if (pDRIDrawablePriv->sid != 0) {
        xp_destroy_surface(pDRIDrawablePriv->sid);
        x_hash_table_remove(surface_hash, x_cvt_uint_to_vptr(pDRIDrawablePriv->sid));
    }

    if (pDRIDrawablePriv->notifiers != NULL)
        x_hook_free(pDRIDrawablePriv->notifiers);

    xfree(pDRIDrawablePriv);

    if (pDrawable->type == DRAWABLE_WINDOW) {
	dixSetPrivate(&pWin->devPrivates, DRIWindowPrivKey, NULL);
    } else if (pDrawable->type == DRAWABLE_PIXMAP) {
	dixSetPrivate(&pPix->devPrivates, DRIPixmapPrivKey, NULL);
    }

    --pDRIPriv->nrWindows;

    return TRUE;
}

void
DRIWindowExposures(WindowPtr pWin, RegionPtr prgn, RegionPtr bsreg)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);

    if (pDRIDrawablePriv) {
        /* FIXME: something? */
    }

    pScreen->WindowExposures = pDRIPriv->wrap.WindowExposures;

    (*pScreen->WindowExposures)(pWin, prgn, bsreg);

    pDRIPriv->wrap.WindowExposures = pScreen->WindowExposures;
    pScreen->WindowExposures = DRIWindowExposures;
}

void
DRICopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr pDRIDrawablePriv;

    if (pDRIPriv->nrWindows > 0) {
       pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin);
       if (pDRIDrawablePriv != NULL) {
            DRIUpdateSurface(pDRIDrawablePriv, &pWin->drawable);
       }
    }

    /* unwrap */
    pScreen->CopyWindow = pDRIPriv->wrap.CopyWindow;

    /* call lower layers */
    (*pScreen->CopyWindow)(pWin, ptOldOrg, prgnSrc);

    /* rewrap */
    pDRIPriv->wrap.CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = DRICopyWindow;
}

int
DRIValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    ScreenPtr pScreen = pParent->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    int returnValue;

    /* unwrap */
    pScreen->ValidateTree = pDRIPriv->wrap.ValidateTree;

    /* call lower layers */
    returnValue = (*pScreen->ValidateTree)(pParent, pChild, kind);

    /* rewrap */
    pDRIPriv->wrap.ValidateTree = pScreen->ValidateTree;
    pScreen->ValidateTree = DRIValidateTree;

    return returnValue;
}

void
DRIPostValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    ScreenPtr pScreen;
    DRIScreenPrivPtr pDRIPriv;

    if (pParent) {
        pScreen = pParent->drawable.pScreen;
    } else {
        pScreen = pChild->drawable.pScreen;
    }
    pDRIPriv = DRI_SCREEN_PRIV(pScreen);

    if (pDRIPriv->wrap.PostValidateTree) {
        /* unwrap */
        pScreen->PostValidateTree = pDRIPriv->wrap.PostValidateTree;

        /* call lower layers */
        (*pScreen->PostValidateTree)(pParent, pChild, kind);

        /* rewrap */
        pDRIPriv->wrap.PostValidateTree = pScreen->PostValidateTree;
        pScreen->PostValidateTree = DRIPostValidateTree;
    }
}

void
DRIClipNotify(WindowPtr pWin, int dx, int dy)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DRIScreenPrivPtr pDRIPriv = DRI_SCREEN_PRIV(pScreen);
    DRIDrawablePrivPtr  pDRIDrawablePriv;

    if ((pDRIDrawablePriv = DRI_DRAWABLE_PRIV_FROM_WINDOW(pWin))) {
        DRIUpdateSurface(pDRIDrawablePriv, &pWin->drawable);
    }

    if (pDRIPriv->wrap.ClipNotify) {
        pScreen->ClipNotify = pDRIPriv->wrap.ClipNotify;

        (*pScreen->ClipNotify)(pWin, dx, dy);

        pDRIPriv->wrap.ClipNotify = pScreen->ClipNotify;
        pScreen->ClipNotify = DRIClipNotify;
    }
}

/* This lets us get at the unwrapped functions so that they can correctly
 * call the lower level functions, and choose whether they will be
 * called at every level of recursion (eg in validatetree).
 */
DRIWrappedFuncsRec *
DRIGetWrappedFuncs(ScreenPtr pScreen)
{
    return &(DRI_SCREEN_PRIV(pScreen)->wrap);
}

void
DRIQueryVersion(int *majorVersion,
                int *minorVersion,
                int *patchVersion)
{
    *majorVersion = APPLE_DRI_MAJOR_VERSION;
    *minorVersion = APPLE_DRI_MINOR_VERSION;
    *patchVersion = APPLE_DRI_PATCH_VERSION;
}

void
DRISurfaceNotify(xp_surface_id id, int kind)
{
    DRIDrawablePrivPtr pDRIDrawablePriv = NULL;
    DRISurfaceNotifyArg arg;

    arg.id = id;
    arg.kind = kind;

    if (surface_hash != NULL)
    {
        pDRIDrawablePriv = x_hash_table_lookup(surface_hash,
                                               x_cvt_uint_to_vptr(id), NULL);
    }

    if (pDRIDrawablePriv == NULL)
        return;

    if (kind == AppleDRISurfaceNotifyDestroyed)
    {
        pDRIDrawablePriv->sid = 0;
        x_hash_table_remove(surface_hash, x_cvt_uint_to_vptr(id));
    }

    x_hook_run(pDRIDrawablePriv->notifiers, &arg);

    if (kind == AppleDRISurfaceNotifyDestroyed)
    {
        /* Kill off the handle. */

        FreeResourceByType(pDRIDrawablePriv->pDraw->id,
                           DRIDrawablePrivResType, FALSE);
    }
}

Bool DRICreatePixmap(ScreenPtr pScreen, Drawable id,
		     DrawablePtr pDrawable, char *path,
		     size_t pathmax) 
{
    DRIPixmapBufferPtr shared;
    PixmapPtr pPix;
    
    if(pDrawable->type != DRAWABLE_PIXMAP)
	return FALSE;

    pPix = (PixmapPtr)pDrawable;

    shared = xalloc(sizeof(*shared));
    if(NULL == shared) {
        FatalError("failed to allocate DRIPixmapBuffer in %s\n", __func__);
    }
        
    shared->pDrawable = pDrawable;
    shared->refCount = 1;

    if(pDrawable->bitsPerPixel >= 24) {
	shared->bytesPerPixel = 4;
    } else if(pDrawable->bitsPerPixel <= 16) {
	shared->bytesPerPixel = 2;
    }
    
    shared->width = pDrawable->width;
    shared->height = pDrawable->height;
    
    if(-1 == snprintf(shared->shmPath, sizeof(shared->shmPath),
                      "%d_0x%lx", getpid(),
                      (unsigned long)id)) {
        FatalError("buffer overflow in %s\n", __func__);
    }
    
    shared->fd = shm_open(shared->shmPath, 
                          O_RDWR | O_EXCL | O_CREAT, 
                          S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
    
    if(-1 == shared->fd) {
	xfree(shared);
        return FALSE;
    }   
    
    shared->length = shared->width * shared->height * shared->bytesPerPixel;
    
    if(-1 == ftruncate(shared->fd, shared->length)) {
	ErrorF("failed to ftruncate (extend) file.");
	shm_unlink(shared->shmPath);
	close(shared->fd);
	xfree(shared);
	return FALSE;
    }

    shared->buffer = mmap(NULL, shared->length,
                          PROT_READ | PROT_WRITE,
                          MAP_FILE | MAP_SHARED, shared->fd, 0);
    
    if(MAP_FAILED == shared->buffer) {
	ErrorF("failed to mmap shared memory.");
	shm_unlink(shared->shmPath);
	close(shared->fd);
	xfree(shared);
	return FALSE;
    }
    
    strncpy(path, shared->shmPath, pathmax);
    path[pathmax - 1] = '\0';
    
    dixSetPrivate(&pPix->devPrivates, DRIPixmapBufferPrivKey, shared);

    AddResource(id, DRIDrawablePrivResType, (pointer)pDrawable);

    return TRUE;
}


Bool DRIGetPixmapData(DrawablePtr pDrawable, int *width, int *height,
		      int *pitch, int *bpp, void **ptr) {
    PixmapPtr pPix;
    DRIPixmapBufferPtr shared;

    if(pDrawable->type != DRAWABLE_PIXMAP)
	return FALSE;

    pPix = (PixmapPtr)pDrawable;

    shared = dixLookupPrivate(&pPix->devPrivates, DRIPixmapBufferPrivKey);

    if(NULL == shared)
	return FALSE;

    assert(pDrawable->width == shared->width);
    assert(pDrawable->height == shared->height);
    
    *width = shared->width;
    *height = shared->height;
    *bpp = shared->bytesPerPixel;
    *pitch = shared->width * shared->bytesPerPixel;
    *ptr = shared->buffer;    

    return TRUE;
}

static Bool
DRIFreePixmapImp(DrawablePtr pDrawable) {
    DRIPixmapBufferPtr shared;
    PixmapPtr pPix;

    if(pDrawable->type != DRAWABLE_PIXMAP)
	return FALSE;

    pPix = (PixmapPtr)pDrawable;

    shared = dixLookupPrivate(&pPix->devPrivates, DRIPixmapBufferPrivKey);

    if(NULL == shared)
	return FALSE;

    close(shared->fd);
    munmap(shared->buffer, shared->length);
    shm_unlink(shared->shmPath);
    xfree(shared);

    dixSetPrivate(&pPix->devPrivates, DRIPixmapBufferPrivKey, (pointer)NULL);

    return TRUE;
}

void 
DRIDestroyPixmap(DrawablePtr pDrawable) {
    if(DRIFreePixmapImp(pDrawable))
	FreeResourceByType(pDrawable->id, DRIDrawablePrivResType, FALSE);

}
