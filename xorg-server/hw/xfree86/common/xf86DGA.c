/*
 * Copyright (c) 1998-2002 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 *
 * Written by Mark Vojkovich
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86str.h"
#include "xf86Priv.h"
#include "dgaproc.h"
#include <X11/extensions/xf86dgastr.h>
#include "colormapst.h"
#include "pixmapstr.h"
#include "inputstr.h"
#include "globals.h"
#include "servermd.h"
#include "micmap.h"
#ifdef XKB
#include <xkbsrv.h>
#endif
#include "xf86Xinput.h"
#include "exglobals.h"
#include "exevents.h"

#include "mi.h"

static int DGAScreenKeyIndex;
static DevPrivateKey DGAScreenKey;
static int mieq_installed = 0;

static Bool DGACloseScreen(int i, ScreenPtr pScreen);
static void DGADestroyColormap(ColormapPtr pmap);
static void DGAInstallColormap(ColormapPtr pmap);
static void DGAUninstallColormap(ColormapPtr pmap);
static void DGAHandleEvent(int screen_num, xEvent *event,
                           DeviceIntPtr device, int nevents);

static void
DGACopyModeInfo(
   DGAModePtr mode,
   XDGAModePtr xmode
);

_X_EXPORT int *XDGAEventBase = NULL;

#define DGA_GET_SCREEN_PRIV(pScreen) ((DGAScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, DGAScreenKey))


typedef struct _FakedVisualList{
   Bool free;
   VisualPtr pVisual;
   struct _FakedVisualList *next;
} FakedVisualList;


typedef struct {
   ScrnInfoPtr 		pScrn;
   int			numModes;
   DGAModePtr		modes;
   CloseScreenProcPtr	CloseScreen;
   DestroyColormapProcPtr DestroyColormap;
   InstallColormapProcPtr InstallColormap;
   UninstallColormapProcPtr UninstallColormap;
   DGADevicePtr		current;
   DGAFunctionPtr	funcs;
   int			input;
   ClientPtr		client;
   int			pixmapMode;
   FakedVisualList	*fakedVisuals;
   ColormapPtr 		dgaColormap;
   ColormapPtr		savedColormap;
   Bool			grabMouse;
   Bool			grabKeyboard;
} DGAScreenRec, *DGAScreenPtr;

_X_EXPORT Bool
DGAInit(
   ScreenPtr pScreen,
   DGAFunctionPtr funcs, 
   DGAModePtr modes,
   int num
){
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    DGAScreenPtr pScreenPriv;
    int i;

    if(!funcs || !funcs->SetMode || !funcs->OpenFramebuffer)
	return FALSE;

    if(!modes || num <= 0)
	return FALSE;

    DGAScreenKey = &DGAScreenKeyIndex;

    if(!(pScreenPriv = (DGAScreenPtr)xalloc(sizeof(DGAScreenRec))))
	return FALSE;

    pScreenPriv->pScrn = pScrn;
    pScreenPriv->numModes = num;
    pScreenPriv->modes = modes;
    pScreenPriv->current = NULL;    
    
    pScreenPriv->funcs = funcs;
    pScreenPriv->input = 0;
    pScreenPriv->client = NULL;
    pScreenPriv->fakedVisuals = NULL;
    pScreenPriv->dgaColormap = NULL;
    pScreenPriv->savedColormap = NULL;
    pScreenPriv->grabMouse = FALSE;
    pScreenPriv->grabKeyboard = FALSE;
    
    for(i = 0; i < num; i++)
	modes[i].num = i + 1;

#ifdef PANORAMIX
     if(!noPanoramiXExtension)
	for(i = 0; i < num; i++)
	    modes[i].flags &= ~DGA_PIXMAP_AVAILABLE;
#endif

    dixSetPrivate(&pScreen->devPrivates, DGAScreenKey, pScreenPriv);
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = DGACloseScreen;
    pScreenPriv->DestroyColormap = pScreen->DestroyColormap;
    pScreen->DestroyColormap = DGADestroyColormap;
    pScreenPriv->InstallColormap = pScreen->InstallColormap;
    pScreen->InstallColormap = DGAInstallColormap;
    pScreenPriv->UninstallColormap = pScreen->UninstallColormap;
    pScreen->UninstallColormap = DGAUninstallColormap;


    return TRUE;
}

/* DGAReInitModes allows the driver to re-initialize
 * the DGA mode list.
 */

_X_EXPORT Bool
DGAReInitModes(
   ScreenPtr pScreen,
   DGAModePtr modes,
   int num
){
    DGAScreenPtr pScreenPriv;
    int i;

    /* No DGA? Ignore call (but don't make it look like it failed) */
    if(DGAScreenKey == NULL)
	return TRUE;
	
    pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

    /* Same as above */
    if(!pScreenPriv)
	return TRUE;

    /* Can't do this while DGA is active */
    if(pScreenPriv->current)
	return FALSE;

    /* Quick sanity check */
    if(!num) 
	modes = NULL;
    else if(!modes) 
	num = 0;

    pScreenPriv->numModes = num;
    pScreenPriv->modes = modes;

    /* This practically disables DGA. So be it. */
    if(!num)
	return TRUE;

    for(i = 0; i < num; i++)
	modes[i].num = i + 1;

#ifdef PANORAMIX
     if(!noPanoramiXExtension)
	for(i = 0; i < num; i++)
	    modes[i].flags &= ~DGA_PIXMAP_AVAILABLE;
#endif

     return TRUE;
}

static void
FreeMarkedVisuals(ScreenPtr pScreen)
{
    DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
    FakedVisualList *prev, *curr, *tmp;

    if(!pScreenPriv->fakedVisuals)
	return;

    prev = NULL;
    curr = pScreenPriv->fakedVisuals;

    while(curr) {
	if(curr->free) {
	    tmp = curr;
	    curr = curr->next;
	    if(prev)
		prev->next = curr;
	    else 
		pScreenPriv->fakedVisuals = curr;
	    xfree(tmp->pVisual);
	    xfree(tmp);
	} else {
	    prev = curr;
	    curr = curr->next;
	}
    }
}

static Bool 
DGACloseScreen(int i, ScreenPtr pScreen)
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

   if (XDGAEventBase) {
       mieqSetHandler(*XDGAEventBase + MotionNotify, NULL);
       mieqSetHandler(*XDGAEventBase + ButtonPress, NULL);
       mieqSetHandler(*XDGAEventBase + ButtonRelease, NULL);
       mieqSetHandler(*XDGAEventBase + KeyPress, NULL);
       mieqSetHandler(*XDGAEventBase + KeyRelease, NULL);
    }

   FreeMarkedVisuals(pScreen);

   pScreen->CloseScreen = pScreenPriv->CloseScreen;
   pScreen->DestroyColormap = pScreenPriv->DestroyColormap;
   pScreen->InstallColormap = pScreenPriv->InstallColormap;
   pScreen->UninstallColormap = pScreenPriv->UninstallColormap;

   /* DGAShutdown() should have ensured that no DGA
	screen were active by here */

   xfree(pScreenPriv);

   return((*pScreen->CloseScreen)(i, pScreen));
}


static void 
DGADestroyColormap(ColormapPtr pmap)
{
   ScreenPtr pScreen = pmap->pScreen;
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
   VisualPtr pVisual = pmap->pVisual;

   if(pScreenPriv->fakedVisuals) {
	FakedVisualList *curr = pScreenPriv->fakedVisuals;
	
	while(curr) {
	    if(curr->pVisual == pVisual) {
		/* We can't get rid of them yet since FreeColormap
		   still needs the pVisual during the cleanup */
		curr->free = TRUE;
		break;
	    }
	    curr = curr->next;
	}
   }  

   if(pScreenPriv->DestroyColormap) {
        pScreen->DestroyColormap = pScreenPriv->DestroyColormap;
        (*pScreen->DestroyColormap)(pmap);
        pScreen->DestroyColormap = DGADestroyColormap;
   }
}


static void 
DGAInstallColormap(ColormapPtr pmap)
{
    ScreenPtr pScreen = pmap->pScreen;
    DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

    if(pScreenPriv->current && pScreenPriv->dgaColormap) {
	if (pmap != pScreenPriv->dgaColormap) {
	    pScreenPriv->savedColormap = pmap;
	    pmap = pScreenPriv->dgaColormap;
	}
    }

    pScreen->InstallColormap = pScreenPriv->InstallColormap;
    (*pScreen->InstallColormap)(pmap);
    pScreen->InstallColormap = DGAInstallColormap;
}

static void 
DGAUninstallColormap(ColormapPtr pmap)
{
    ScreenPtr pScreen = pmap->pScreen;
    DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

    if(pScreenPriv->current && pScreenPriv->dgaColormap) {
	if (pmap == pScreenPriv->dgaColormap) {
	    pScreenPriv->dgaColormap = NULL;
	}
    }

    pScreen->UninstallColormap = pScreenPriv->UninstallColormap;
    (*pScreen->UninstallColormap)(pmap);
    pScreen->UninstallColormap = DGAUninstallColormap;
}

int
xf86SetDGAMode(
   int index,
   int num,
   DGADevicePtr devRet
){
   ScreenPtr pScreen = screenInfo.screens[index];
   DGAScreenPtr pScreenPriv;
   ScrnInfoPtr pScrn;
   DGADevicePtr device;
   PixmapPtr pPix = NULL;
   DGAModePtr pMode = NULL;

   /* First check if DGAInit was successful on this screen */
   if (DGAScreenKey == NULL)
	return BadValue;
   pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
   if (!pScreenPriv)
	return BadValue;
   pScrn = pScreenPriv->pScrn;

   if(!num) {
	if(pScreenPriv->current) {
	    PixmapPtr oldPix = pScreenPriv->current->pPix;
	    if(oldPix) {
		if(oldPix->drawable.id)
		    FreeResource(oldPix->drawable.id, RT_NONE);
		else
		    (*pScreen->DestroyPixmap)(oldPix);
	    }
	    xfree(pScreenPriv->current);
	    pScreenPriv->current = NULL;
	    pScrn->vtSema = TRUE;
	    (*pScreenPriv->funcs->SetMode)(pScrn, NULL);
	    if(pScreenPriv->savedColormap) {
	        (*pScreen->InstallColormap)(pScreenPriv->savedColormap);
		pScreenPriv->savedColormap = NULL;
	    }
	    pScreenPriv->dgaColormap = NULL;
	    (*pScrn->EnableDisableFBAccess)(index, TRUE);

	    FreeMarkedVisuals(pScreen);
	}
      
        pScreenPriv->grabMouse = FALSE;
        pScreenPriv->grabKeyboard = FALSE;

	return Success;
   }

   if(!pScrn->vtSema && !pScreenPriv->current) /* Really switched away */
	return BadAlloc;
      
   if((num > 0) && (num <= pScreenPriv->numModes))
	pMode = &(pScreenPriv->modes[num - 1]);
   else
	return BadValue;

   if(!(device = (DGADevicePtr)xalloc(sizeof(DGADeviceRec))))
	return BadAlloc;

   if(!pScreenPriv->current) {
	Bool oldVTSema = pScrn->vtSema;

	pScrn->vtSema = FALSE;  /* kludge until we rewrite VT switching */
	(*pScrn->EnableDisableFBAccess)(index, FALSE);
	pScrn->vtSema = oldVTSema;
   } 

   if(!(*pScreenPriv->funcs->SetMode)(pScrn, pMode)) {
	xfree(device);
	return BadAlloc;
   }

   pScrn->currentMode = pMode->mode;

   if(!pScreenPriv->current && !pScreenPriv->input) {
	/* if it's multihead we need to warp the cursor off of
	   our screen so it doesn't get trapped  */
   } 

   pScrn->vtSema = FALSE;

   if(pScreenPriv->current) {
	PixmapPtr oldPix = pScreenPriv->current->pPix;
	if(oldPix) {
	    if(oldPix->drawable.id)
		FreeResource(oldPix->drawable.id, RT_NONE);
	    else
		(*pScreen->DestroyPixmap)(oldPix);
	}
	xfree(pScreenPriv->current);
	pScreenPriv->current = NULL;
   } 

   if(pMode->flags & DGA_PIXMAP_AVAILABLE) {
	if((pPix = (*pScreen->CreatePixmap)(pScreen, 0, 0, pMode->depth, 0))) {
	    (*pScreen->ModifyPixmapHeader)(pPix, 
			pMode->pixmapWidth, pMode->pixmapHeight,
			pMode->depth, pMode->bitsPerPixel, 
			pMode->bytesPerScanline,
 			(pointer)(pMode->address));
        }
   }

   devRet->mode = device->mode = pMode;
   devRet->pPix = device->pPix = pPix;
   pScreenPriv->current = device;
   pScreenPriv->pixmapMode = FALSE;
   pScreenPriv->grabMouse = TRUE;
   pScreenPriv->grabKeyboard = TRUE;

   return Success;
}



/*********** exported ones ***************/

_X_EXPORT void
DGASetInputMode(int index, Bool keyboard, Bool mouse)
{
   ScreenPtr pScreen = screenInfo.screens[index];
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

   if (pScreenPriv)
   {
      pScreenPriv->grabMouse = mouse;
      pScreenPriv->grabKeyboard = keyboard;

      if (!mieq_installed) {
          mieqSetHandler(*XDGAEventBase + MotionNotify, DGAHandleEvent);
          mieqSetHandler(*XDGAEventBase + ButtonPress, DGAHandleEvent);
          mieqSetHandler(*XDGAEventBase + ButtonRelease, DGAHandleEvent);
          mieqSetHandler(*XDGAEventBase + KeyPress, DGAHandleEvent);
          mieqSetHandler(*XDGAEventBase + KeyRelease, DGAHandleEvent);
          mieq_installed = 1;
      }
   }
}

_X_EXPORT Bool
DGAChangePixmapMode(int index, int *x, int *y, int mode)
{
   DGAScreenPtr pScreenPriv;
   DGADevicePtr pDev;
   DGAModePtr   pMode;
   PixmapPtr    pPix;

   if(DGAScreenKey == NULL)
	return FALSE;

   pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   if(!pScreenPriv || !pScreenPriv->current || !pScreenPriv->current->pPix)
	return FALSE;

   pDev = pScreenPriv->current;
   pPix = pDev->pPix;
   pMode = pDev->mode;

   if(mode) {
	int shift = 2;

	if(*x > (pMode->pixmapWidth - pMode->viewportWidth))
	    *x = pMode->pixmapWidth - pMode->viewportWidth;
	if(*y > (pMode->pixmapHeight - pMode->viewportHeight))
	    *y = pMode->pixmapHeight - pMode->viewportHeight;

	switch(xf86Screens[index]->bitsPerPixel) {
	case 16: shift = 1;  break;
	case 32: shift = 0;  break;
	default: break;
	}

	if(BITMAP_SCANLINE_PAD == 64)
	    shift++;

	*x = (*x >> shift) << shift;

	pPix->drawable.x = *x; 
	pPix->drawable.y = *y; 
	pPix->drawable.width = pMode->viewportWidth; 
	pPix->drawable.height = pMode->viewportHeight; 
   } else {
	pPix->drawable.x = 0; 
	pPix->drawable.y = 0; 
	pPix->drawable.width = pMode->pixmapWidth; 
	pPix->drawable.height = pMode->pixmapHeight; 
   }
   pPix->drawable.serialNumber = NEXT_SERIAL_NUMBER;
   pScreenPriv->pixmapMode = mode;

   return TRUE;
}

_X_EXPORT Bool
DGAAvailable(int index) 
{
   if(DGAScreenKey == NULL)
	return FALSE;
   
   if (!xf86NoSharedResources(((ScrnInfoPtr)dixLookupPrivate(
				   &screenInfo.screens[index]->devPrivates,
				   xf86ScreenKey))->scrnIndex, MEM))
       return FALSE;
   
   if(DGA_GET_SCREEN_PRIV(screenInfo.screens[index]))
	return TRUE;

   return FALSE;
}

_X_EXPORT Bool
DGAActive(int index) 
{
   DGAScreenPtr pScreenPriv;

   if(DGAScreenKey == NULL)
	return FALSE;

   pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   if(pScreenPriv && pScreenPriv->current)
	return TRUE;

   return FALSE;
}



/* Called by the event code in case the server is abruptly terminated */

void 
DGAShutdown()
{
    ScrnInfoPtr pScrn;
    int i;

    if(DGAScreenKey == NULL)
	return;

    for(i = 0; i < screenInfo.numScreens; i++) {
	pScrn = xf86Screens[i];

	(void)(*pScrn->SetDGAMode)(pScrn->scrnIndex, 0, NULL);
    }
}

/* Called by the extension to initialize a mode */

_X_EXPORT int
DGASetMode(
   int index,
   int num,
   XDGAModePtr mode,
   PixmapPtr *pPix
){
    ScrnInfoPtr pScrn = xf86Screens[index];
    DGADeviceRec device;
    int ret;

    /* We rely on the extension to check that DGA is available */ 

    ret = (*pScrn->SetDGAMode)(index, num, &device);
    if((ret == Success) && num) {
	DGACopyModeInfo(device.mode, mode);
	*pPix = device.pPix;
    }

    return ret;
}

/* Called from the extension to let the DDX know which events are requested */

_X_EXPORT void
DGASelectInput(
   int index,
   ClientPtr client,
   long mask
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   /* We rely on the extension to check that DGA is available */
   pScreenPriv->client = client;
   pScreenPriv->input = mask;
}

_X_EXPORT int 
DGAGetViewportStatus(int index) 
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   /* We rely on the extension to check that DGA is active */ 

   if (!pScreenPriv->funcs->GetViewport)
      return 0;

   return (*pScreenPriv->funcs->GetViewport)(pScreenPriv->pScrn);
}

_X_EXPORT int
DGASetViewport(
   int index,
   int x, int y,
   int mode
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   if (pScreenPriv->funcs->SetViewport)
      (*pScreenPriv->funcs->SetViewport)(pScreenPriv->pScrn, x, y, mode);
   return Success;
}


static int
BitsClear(CARD32 data)
{
   int bits = 0;
   CARD32 mask;

   for(mask = 1; mask; mask <<= 1) {
	if(!(data & mask)) bits++;
	else break;
   }

   return bits;
}

_X_EXPORT int
DGACreateColormap(int index, ClientPtr client, int id, int mode, int alloc)
{
   ScreenPtr pScreen = screenInfo.screens[index];
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
   FakedVisualList *fvlp;
   VisualPtr pVisual;
   DGAModePtr pMode;
   ColormapPtr pmap;

   if(!mode || (mode > pScreenPriv->numModes))
	return BadValue;

   if((alloc != AllocNone) && (alloc != AllocAll))
	return BadValue;

   pMode = &(pScreenPriv->modes[mode - 1]);

   if(!(pVisual = xalloc(sizeof(VisualRec))))
	return BadAlloc;

   pVisual->vid = FakeClientID(0);
   pVisual->class = pMode->visualClass;
   pVisual->nplanes = pMode->depth;
   pVisual->ColormapEntries = 1 << pMode->depth;
   pVisual->bitsPerRGBValue = (pMode->depth + 2) / 3;

   switch (pVisual->class) {
   case PseudoColor:
   case GrayScale:
   case StaticGray:
	pVisual->bitsPerRGBValue = 8; /* not quite */
	pVisual->redMask     = 0;
	pVisual->greenMask   = 0;
	pVisual->blueMask    = 0;
	pVisual->offsetRed   = 0;
	pVisual->offsetGreen = 0;
	pVisual->offsetBlue  = 0;
	break;
   case DirectColor:
   case TrueColor:
	pVisual->ColormapEntries = 1 << pVisual->bitsPerRGBValue;
                /* fall through */
   case StaticColor:
	pVisual->redMask = pMode->red_mask;
	pVisual->greenMask = pMode->green_mask;
	pVisual->blueMask = pMode->blue_mask;
	pVisual->offsetRed   = BitsClear(pVisual->redMask);
	pVisual->offsetGreen = BitsClear(pVisual->greenMask);
	pVisual->offsetBlue  = BitsClear(pVisual->blueMask);
   }

   if(!(fvlp = xalloc(sizeof(FakedVisualList)))) {
	xfree(pVisual);
	return BadAlloc;
   }

   fvlp->free = FALSE;
   fvlp->pVisual = pVisual;
   fvlp->next = pScreenPriv->fakedVisuals;
   pScreenPriv->fakedVisuals = fvlp;

   LEGAL_NEW_RESOURCE(id, client);

   return CreateColormap(id, pScreen, pVisual, &pmap, alloc, client->index);
}

/*  Called by the extension to install a colormap on DGA active screens */

_X_EXPORT void
DGAInstallCmap(ColormapPtr cmap)
{
    ScreenPtr pScreen = cmap->pScreen;
    DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

    /* We rely on the extension to check that DGA is active */ 

    if(!pScreenPriv->dgaColormap) 
	pScreenPriv->savedColormap = miInstalledMaps[pScreen->myNum];

    pScreenPriv->dgaColormap = cmap;    

    (*pScreen->InstallColormap)(cmap);
}

_X_EXPORT int
DGASync(int index)
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   
   /* We rely on the extension to check that DGA is active */

   if (pScreenPriv->funcs->Sync)
      (*pScreenPriv->funcs->Sync)(pScreenPriv->pScrn);

   return Success;
}

_X_EXPORT int
DGAFillRect(
   int index,
   int x, int y, int w, int h,
   unsigned long color
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   
   /* We rely on the extension to check that DGA is active */

   if(pScreenPriv->funcs->FillRect && 
	(pScreenPriv->current->mode->flags & DGA_FILL_RECT)) {

	(*pScreenPriv->funcs->FillRect)(pScreenPriv->pScrn, x, y, w, h, color);
	return Success;
   }
   return BadMatch;
}

_X_EXPORT int
DGABlitRect(
   int index,
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   
   /* We rely on the extension to check that DGA is active */

   if(pScreenPriv->funcs->BlitRect &&
	(pScreenPriv->current->mode->flags & DGA_BLIT_RECT)) {

	(*pScreenPriv->funcs->BlitRect)(pScreenPriv->pScrn, 	
		srcx, srcy, w, h, dstx, dsty);
	return Success;
   }
   return BadMatch;
}

_X_EXPORT int
DGABlitTransRect(
   int index,
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty,
   unsigned long color
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   
   /* We rely on the extension to check that DGA is active */

   if(pScreenPriv->funcs->BlitTransRect && 
	(pScreenPriv->current->mode->flags & DGA_BLIT_RECT_TRANS)) {

	(*pScreenPriv->funcs->BlitTransRect)(pScreenPriv->pScrn, 	
		srcx, srcy, w, h, dstx, dsty, color);
	return Success;
   }
   return BadMatch;
}


_X_EXPORT int
DGAGetModes(int index)
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   /* We rely on the extension to check that DGA is available */

   return pScreenPriv->numModes;
}


_X_EXPORT int
DGAGetModeInfo(
  int index,
  XDGAModePtr mode,
  int num
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   /* We rely on the extension to check that DGA is available */

   if((num <= 0) || (num > pScreenPriv->numModes))
	return BadValue;

   DGACopyModeInfo(&(pScreenPriv->modes[num - 1]), mode);

   return Success;
}


static void
DGACopyModeInfo(
   DGAModePtr mode,
   XDGAModePtr xmode
){
   DisplayModePtr dmode = mode->mode;

   xmode->num = mode->num;
   xmode->name = dmode->name;
   xmode->VSync_num = (int)(dmode->VRefresh * 1000.0); 
   xmode->VSync_den = 1000;
   xmode->flags = mode->flags;
   xmode->imageWidth = mode->imageWidth;
   xmode->imageHeight = mode->imageHeight;
   xmode->pixmapWidth = mode->pixmapWidth;
   xmode->pixmapHeight = mode->pixmapHeight;
   xmode->bytesPerScanline = mode->bytesPerScanline;
   xmode->byteOrder = mode->byteOrder;
   xmode->depth = mode->depth;
   xmode->bitsPerPixel = mode->bitsPerPixel;
   xmode->red_mask = mode->red_mask;
   xmode->green_mask = mode->green_mask;
   xmode->blue_mask = mode->blue_mask;
   xmode->visualClass = mode->visualClass;
   xmode->viewportWidth = mode->viewportWidth;
   xmode->viewportHeight = mode->viewportHeight;
   xmode->xViewportStep = mode->xViewportStep;
   xmode->yViewportStep = mode->yViewportStep;
   xmode->maxViewportX = mode->maxViewportX;
   xmode->maxViewportY = mode->maxViewportY;
   xmode->viewportFlags = mode->viewportFlags;
   xmode->reserved1 = mode->reserved1;
   xmode->reserved2 = mode->reserved2;
   xmode->offset = mode->offset;

   if(dmode->Flags & V_INTERLACE) xmode->flags |= DGA_INTERLACED;
   if(dmode->Flags & V_DBLSCAN) xmode->flags |= DGA_DOUBLESCAN;
}


Bool 
DGAVTSwitch(void)
{
    ScreenPtr pScreen;
    int i;

    for(i = 0; i < screenInfo.numScreens; i++) {
       pScreen = screenInfo.screens[i];	

       /* Alternatively, this could send events to DGA clients */

       if(DGAScreenKey) {
	   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);

	   if(pScreenPriv && pScreenPriv->current)
		return FALSE;
       }
    }

   return TRUE;
}

Bool
DGAStealKeyEvent(DeviceIntPtr dev, int index, int key_code, int is_down)
{
   DGAScreenPtr pScreenPriv;
   dgaEvent    de;
    
   if(DGAScreenKey == NULL) /* no DGA */
        return FALSE;

   if (key_code < 8 || key_code > 255)
       return FALSE;

   pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   if(!pScreenPriv || !pScreenPriv->grabKeyboard) /* no direct mode */
        return FALSE; 

    de.u.u.type = *XDGAEventBase + (is_down ? KeyPress : KeyRelease);
    de.u.u.detail = key_code;
    de.u.event.time = GetTimeInMillis();
    mieqEnqueue (dev, (xEvent *) &de);

   return TRUE;
}  

static int  DGAMouseX, DGAMouseY;

Bool
DGAStealMotionEvent(DeviceIntPtr dev, int index, int dx, int dy)
{
   DGAScreenPtr pScreenPriv;
    dgaEvent    de;

   if(DGAScreenKey == NULL) /* no DGA */
        return FALSE;
    
   pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   if(!pScreenPriv || !pScreenPriv->grabMouse) /* no direct mode */
        return FALSE;

    DGAMouseX += dx;
    if (DGAMouseX < 0)
        DGAMouseX = 0;
    else if (DGAMouseX > screenInfo.screens[index]->width)
        DGAMouseX = screenInfo.screens[index]->width;
    DGAMouseY += dy;
    if (DGAMouseY < 0)
        DGAMouseY = 0;
    else if (DGAMouseY > screenInfo.screens[index]->height)
        DGAMouseY = screenInfo.screens[index]->height;
    de.u.u.type = *XDGAEventBase + MotionNotify;
    de.u.u.detail = 0;
    de.u.event.time = GetTimeInMillis();
    de.u.event.dx = dx;
    de.u.event.dy = dy;
    de.u.event.pad1 = DGAMouseX;
    de.u.event.pad2 = DGAMouseY;
    mieqEnqueue (dev, (xEvent *) &de);
    return TRUE;
}

Bool
DGAStealButtonEvent(DeviceIntPtr dev, int index, int button, int is_down)
{
    DGAScreenPtr pScreenPriv;
    dgaEvent de;

    if (DGAScreenKey == NULL)
        return FALSE;
    
    pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

    if (!pScreenPriv || !pScreenPriv->grabMouse)
        return FALSE;

    de.u.u.type = *XDGAEventBase + (is_down ? ButtonPress : ButtonRelease);
    de.u.u.detail = button;
    de.u.event.time = GetTimeInMillis();
    de.u.event.dx = 0;
    de.u.event.dy = 0;
    de.u.event.pad1 = DGAMouseX;
    de.u.event.pad2 = DGAMouseY;
    mieqEnqueue (dev, (xEvent *) &de);

    return TRUE;
}

/* We have the power to steal or modify events that are about to get queued */

Bool
DGAIsDgaEvent (xEvent *e)
{
    int	    coreEquiv;
    if (DGAScreenKey == NULL || XDGAEventBase == 0)
	return FALSE;
    coreEquiv = e->u.u.type - *XDGAEventBase;
    if (KeyPress <= coreEquiv && coreEquiv <= MotionNotify)
	return TRUE;
    return FALSE;
}

#define NoSuchEvent 0x80000000	/* so doesn't match NoEventMask */
static Mask filters[] =
{
	NoSuchEvent,		       /* 0 */
	NoSuchEvent,		       /* 1 */
	KeyPressMask,		       /* KeyPress */
	KeyReleaseMask,		       /* KeyRelease */
	ButtonPressMask,	       /* ButtonPress */
	ButtonReleaseMask,	       /* ButtonRelease */
	PointerMotionMask,	       /* MotionNotify (initial state) */
};

static void
DGAProcessKeyboardEvent (ScreenPtr pScreen, dgaEvent *de, DeviceIntPtr keybd)
{
    int		    coreEquiv;
    xEvent	    xi;
    KeyClassPtr	    keyc = keybd->key;
    DGAScreenPtr    pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
    DeviceIntPtr    pointer = GetPairedDevice(keybd);

    coreEquiv = de->u.u.type - *XDGAEventBase;

    /*
     * Fill in remaining event state
     */
    de->u.event.dx = 0;
    de->u.event.dy = 0;
    de->u.event.screen = pScreen->myNum;
    de->u.event.state = keyc->state | pointer->button->state;

    de->u.u.type = (IEventBase - 1) + coreEquiv; /* change to XI event */
    UpdateDeviceState(keybd, (xEvent*)de, 1);
    de->u.u.type = *XDGAEventBase + coreEquiv; /* change back */

    /*
     * Deliver the DGA event
     */
    if (pScreenPriv->client)
    {
	/* If the DGA client has selected input, then deliver based on the usual filter */
	TryClientEvents (pScreenPriv->client, keybd, (xEvent *) de, 1,
			 filters[coreEquiv], pScreenPriv->input, 0);
    }
    else
    {
	/* If the keyboard is actively grabbed, deliver a grabbed core event */
	if (keybd->deviceGrab.grab && !keybd->deviceGrab.fromPassiveGrab)
	{
	    xi.u.u.type                  = (IEventBase - 1) + coreEquiv;
	    xi.u.u.detail                = de->u.u.detail;
	    xi.u.keyButtonPointer.time   = de->u.event.time;
	    xi.u.keyButtonPointer.eventX = de->u.event.dx;
	    xi.u.keyButtonPointer.eventY = de->u.event.dy;
	    xi.u.keyButtonPointer.rootX  = de->u.event.dx;
	    xi.u.keyButtonPointer.rootY  = de->u.event.dy;
	    xi.u.keyButtonPointer.state  = de->u.event.state;
	    ((deviceKeyButtonPointer*)&xi)->deviceid = keybd->id;
	    DeliverGrabbedEvent (&xi, keybd, FALSE, 1);
	}
    }
}

static void
DGAProcessPointerEvent (ScreenPtr pScreen, dgaEvent *de, DeviceIntPtr mouse)
{
    ButtonClassPtr  butc = mouse->button;
    int		    coreEquiv;
    DGAScreenPtr    pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
    xEvent	    xi;

    coreEquiv = de->u.u.type - *XDGAEventBase;
    /*
     * Fill in remaining event state
     */
    de->u.event.screen = pScreen->myNum;
    de->u.event.state = butc->state | GetPairedDevice(mouse)->key->state;

    de->u.u.type = (IEventBase - 1) + coreEquiv; /* change to XI event */
    UpdateDeviceState(mouse, (xEvent*)de, 1);
    de->u.u.type = *XDGAEventBase + coreEquiv; /* change back */

    /*
     * Deliver the DGA event
     */
    if (pScreenPriv->client)
    {
	/* If the DGA client has selected input, then deliver based on the usual filter */
	TryClientEvents (pScreenPriv->client, mouse, (xEvent *) de, 1,
			 filters[coreEquiv], pScreenPriv->input, 0);
    }
    else
    {
	/* If the pointer is actively grabbed, deliver a grabbed core event */
	if (mouse->deviceGrab.grab && !mouse->deviceGrab.fromPassiveGrab)
	{
	    xi.u.u.type                   = (IEventBase - 1 ) + coreEquiv;
	    xi.u.u.detail                 = de->u.u.detail;
	    xi.u.keyButtonPointer.time    = de->u.event.time;
	    xi.u.keyButtonPointer.eventX  = de->u.event.dx;
	    xi.u.keyButtonPointer.eventY  = de->u.event.dy;
	    xi.u.keyButtonPointer.rootX   = de->u.event.dx;
	    xi.u.keyButtonPointer.rootY   = de->u.event.dy;
	    xi.u.keyButtonPointer.state   = de->u.event.state;
	    DeliverGrabbedEvent (&xi, mouse, FALSE, 1);
	}
    }
}

_X_EXPORT Bool 
DGAOpenFramebuffer(
   int index,
   char **name,
   unsigned char **mem,
   int *size,
   int *offset,
   int *flags
){
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   /* We rely on the extension to check that DGA is available */

   return (*pScreenPriv->funcs->OpenFramebuffer)(pScreenPriv->pScrn, 
				name, mem, size, offset, flags);
}

_X_EXPORT void
DGACloseFramebuffer(int index)
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);

   /* We rely on the extension to check that DGA is available */
   if(pScreenPriv->funcs->CloseFramebuffer)
	(*pScreenPriv->funcs->CloseFramebuffer)(pScreenPriv->pScrn);
}

/*  For DGA 1.0 backwards compatibility only */

_X_EXPORT int 
DGAGetOldDGAMode(int index)
{
   DGAScreenPtr pScreenPriv = DGA_GET_SCREEN_PRIV(screenInfo.screens[index]);
   ScrnInfoPtr pScrn = pScreenPriv->pScrn;
   DGAModePtr mode;
   int i, w, h, p;

   /* We rely on the extension to check that DGA is available */

   w = pScrn->currentMode->HDisplay;
   h = pScrn->currentMode->VDisplay;
   p = ((pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) + 3) & ~3L;

   for(i = 0; i < pScreenPriv->numModes; i++) {
	mode = &(pScreenPriv->modes[i]);
  	      
	if((mode->viewportWidth == w) && (mode->viewportHeight == h) &&
		(mode->bytesPerScanline == p) && 
		(mode->bitsPerPixel == pScrn->bitsPerPixel) &&
		(mode->depth == pScrn->depth)) {

		return mode->num;
	}
   }

   return 0;
}

static void
DGAHandleEvent(int screen_num, xEvent *event, DeviceIntPtr device, int nevents)
{
    dgaEvent	    *de = (dgaEvent *) event;
    ScreenPtr       pScreen = screenInfo.screens[screen_num];
    DGAScreenPtr    pScreenPriv;
    int		    coreEquiv;

    /* no DGA */
    if (DGAScreenKey == NULL || XDGAEventBase == 0)
	return;
    pScreenPriv = DGA_GET_SCREEN_PRIV(pScreen);
    
    /* DGA not initialized on this screen */
    if (!pScreenPriv)
	return;
    
    coreEquiv = de->u.u.type - *XDGAEventBase;
    /* Not a DGA event; shouldn't happen, but you never know. */
    if (coreEquiv < KeyPress || coreEquiv > MotionNotify)
	return;
    
    switch (coreEquiv) {
    case KeyPress:
    case KeyRelease:
	DGAProcessKeyboardEvent (pScreen, de, device);
	break;
    default:
	DGAProcessPointerEvent (pScreen, de, device);
	break;
    }
}
