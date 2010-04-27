/*
 * midispcur.c
 *
 * machine independent cursor display routines
 */


/*

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

# include   <X11/X.h>
# include   "misc.h"
# include   "input.h"
# include   "cursorstr.h"
# include   "windowstr.h"
# include   "regionstr.h"
# include   "dixstruct.h"
# include   "scrnintstr.h"
# include   "servermd.h"
# include   "mipointer.h"
# include   "misprite.h"
# include   "gcstruct.h"

#ifdef ARGB_CURSOR
# include   "picturestr.h"
#endif

# include "inputstr.h"

/* per-screen private data */
static int miDCScreenKeyIndex;
static DevPrivateKey miDCScreenKey = &miDCScreenKeyIndex;

static Bool	miDCCloseScreen(int index, ScreenPtr pScreen);

/* per device per-screen private data */
static int miDCSpriteKeyIndex[MAXSCREENS];
static DevPrivateKey miDCSpriteKey = miDCSpriteKeyIndex;

typedef struct {
    GCPtr	    pSourceGC, pMaskGC;
    GCPtr	    pSaveGC, pRestoreGC;
    GCPtr	    pMoveGC;
    GCPtr	    pPixSourceGC, pPixMaskGC;
    PixmapPtr	    pSave, pTemp;
#ifdef ARGB_CURSOR
    PicturePtr	    pRootPicture;
    PicturePtr	    pTempPicture;
#endif
} miDCBufferRec, *miDCBufferPtr;

#define MIDCBUFFER(dev, screen) \
 ((DevHasCursor(dev)) ? \
  (miDCBufferPtr)dixLookupPrivate(&dev->devPrivates, miDCSpriteKey + (screen)->myNum) : \
  (miDCBufferPtr)dixLookupPrivate(&dev->u.master->devPrivates, miDCSpriteKey + (screen)->myNum))

/* 
 * The core pointer buffer will point to the index of the virtual core pointer
 * in the pCursorBuffers array. 
 */
typedef struct {
    CloseScreenProcPtr CloseScreen;
} miDCScreenRec, *miDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr		sourceBits;	    /* source bits */
    PixmapPtr		maskBits;	    /* mask bits */
#ifdef ARGB_CURSOR
    PicturePtr		pPicture;
#endif
} miDCCursorRec, *miDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool	miDCRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool	miDCUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool	miDCPutUpCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                CursorPtr pCursor, int x, int y, 
                                unsigned long source, unsigned long mask);
static Bool	miDCSaveUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                    int x, int y,
				    int w, int h);
static Bool	miDCRestoreUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                       int x, int y,
				       int w, int h);
static Bool	miDCMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                               CursorPtr pCursor, int x, int y, 
                               int w, int h, int dx, int dy,
			       unsigned long source, unsigned long mask);
static Bool	miDCChangeSave(DeviceIntPtr pDev, ScreenPtr pScreen, 
                               int x, int y, int w, int h,	
                               int dx, int dy);

static Bool     miDCDeviceInitialize(DeviceIntPtr pDev, ScreenPtr pScreen);
static void     miDCDeviceCleanup(DeviceIntPtr pDev, ScreenPtr pScreen);

static miSpriteCursorFuncRec miDCFuncs = {
    miDCRealizeCursor,
    miDCUnrealizeCursor,
    miDCPutUpCursor,
    miDCSaveUnderCursor,
    miDCRestoreUnderCursor,
    miDCMoveCursor,
    miDCChangeSave,
    miDCDeviceInitialize,
    miDCDeviceCleanup
};

Bool
miDCInitialize (ScreenPtr pScreen, miPointerScreenFuncPtr screenFuncs)
{
    miDCScreenPtr   pScreenPriv;

    pScreenPriv = xalloc (sizeof (miDCScreenRec));
    if (!pScreenPriv)
	return FALSE;


    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = miDCCloseScreen;

    dixSetPrivate(&pScreen->devPrivates, miDCScreenKey, pScreenPriv);

    if (!miSpriteInitialize (pScreen, &miDCFuncs, screenFuncs))
    {
	xfree ((pointer) pScreenPriv);
	return FALSE;
    }
    return TRUE;
}

static Bool
miDCCloseScreen (int index, ScreenPtr pScreen)
{
    miDCScreenPtr   pScreenPriv;

    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
miDCRealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    if (pCursor->bits->refcnt <= 1)
	dixSetPrivate(&pCursor->bits->devPrivates, CursorScreenKey(pScreen), NULL);
    return TRUE;
}

#ifdef ARGB_CURSOR

static VisualPtr
miDCGetWindowVisual (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    VisualID	    vid = wVisual (pWin);
    int		    i;

    for (i = 0; i < pScreen->numVisuals; i++)
	if (pScreen->visuals[i].vid == vid)
	    return &pScreen->visuals[i];
    return 0;
}

static PicturePtr
miDCMakePicture (PicturePtr *ppPicture, DrawablePtr pDraw, WindowPtr pWin)
{
    ScreenPtr	    pScreen = pDraw->pScreen;
    VisualPtr	    pVisual;
    PictFormatPtr   pFormat;
    XID		    subwindow_mode = IncludeInferiors;
    PicturePtr	    pPicture;
    int		    error;
    
    pVisual = miDCGetWindowVisual (pWin);
    if (!pVisual)
	return 0;
    pFormat = PictureMatchVisual (pScreen, pDraw->depth, pVisual);
    if (!pFormat)
	return 0;
    pPicture = CreatePicture (0, pDraw, pFormat,
			      CPSubwindowMode, &subwindow_mode,
			      serverClient, &error);
    *ppPicture = pPicture;
    return pPicture;
}
#endif

static miDCCursorPtr
miDCRealize (ScreenPtr pScreen, CursorPtr pCursor)
{
    miDCCursorPtr   pPriv;
    GCPtr	    pGC;
    XID		    gcvals[3];

    pPriv = xalloc (sizeof (miDCCursorRec));
    if (!pPriv)
	return NULL;
#ifdef ARGB_CURSOR
    if (pCursor->bits->argb)
    {
	PixmapPtr	pPixmap;
	PictFormatPtr	pFormat;
	int		error;
	
	pFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
	if (!pFormat)
	{
	    xfree ((pointer) pPriv);
	    return NULL;
	}
	
	pPriv->sourceBits = 0;
	pPriv->maskBits = 0;
	pPixmap = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width,
					    pCursor->bits->height, 32,
					    CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pPixmap)
	{
	    xfree ((pointer) pPriv);
	    return NULL;
	}
	pGC = GetScratchGC (32, pScreen);
	if (!pGC)
	{
	    (*pScreen->DestroyPixmap) (pPixmap);
	    xfree ((pointer) pPriv);
	    return NULL;
	}
	ValidateGC (&pPixmap->drawable, pGC);
	(*pGC->ops->PutImage) (&pPixmap->drawable, pGC, 32,
			       0, 0, pCursor->bits->width,
			       pCursor->bits->height,
			       0, ZPixmap, (char *) pCursor->bits->argb);
	FreeScratchGC (pGC);
	pPriv->pPicture = CreatePicture (0, &pPixmap->drawable,
					pFormat, 0, 0, serverClient, &error);
        (*pScreen->DestroyPixmap) (pPixmap);
	if (!pPriv->pPicture)
	{
	    xfree ((pointer) pPriv);
	    return NULL;
	}
	dixSetPrivate(&pCursor->bits->devPrivates, CursorScreenKey(pScreen), pPriv);
	return pPriv;
    }
    pPriv->pPicture = 0;
#endif
    pPriv->sourceBits = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1, 0);
    if (!pPriv->sourceBits)
    {
	xfree ((pointer) pPriv);
	return NULL;
    }
    pPriv->maskBits =  (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1, 0);
    if (!pPriv->maskBits)
    {
	(*pScreen->DestroyPixmap) (pPriv->sourceBits);
	xfree ((pointer) pPriv);
	return NULL;
    }
    dixSetPrivate(&pCursor->bits->devPrivates, CursorScreenKey(pScreen), pPriv);

    /* create the two sets of bits, clipping as appropriate */

    pGC = GetScratchGC (1, pScreen);
    if (!pGC)
    {
	(void) miDCUnrealizeCursor (pScreen, pCursor);
	return NULL;
    }

    ValidateGC ((DrawablePtr)pPriv->sourceBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->source);
    gcvals[0] = GXand;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->sourceBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->mask);

    /* mask bits -- pCursor->mask & ~pCursor->source */
    gcvals[0] = GXcopy;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->mask);
    gcvals[0] = GXandInverted;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->source);
    FreeScratchGC (pGC);
    return pPriv;
}

static Bool
miDCUnrealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    miDCCursorPtr   pPriv;

    pPriv = (miDCCursorPtr)dixLookupPrivate(&pCursor->bits->devPrivates,
					    CursorScreenKey(pScreen));
    if (pPriv && (pCursor->bits->refcnt <= 1))
    {
	if (pPriv->sourceBits)
	    (*pScreen->DestroyPixmap) (pPriv->sourceBits);
	if (pPriv->maskBits)
	    (*pScreen->DestroyPixmap) (pPriv->maskBits);
#ifdef ARGB_CURSOR
	if (pPriv->pPicture)
	    FreePicture (pPriv->pPicture, 0);
#endif
	xfree ((pointer) pPriv);
	dixSetPrivate(&pCursor->bits->devPrivates, CursorScreenKey(pScreen), NULL);
    }
    return TRUE;
}

static void
miDCPutBits (
    DrawablePtr	    pDrawable,
    miDCCursorPtr   pPriv,
    GCPtr	    sourceGC,
    GCPtr	    maskGC,
    int             x_org,
    int             y_org,
    unsigned        w,
    unsigned        h,
    unsigned long   source,
    unsigned long   mask)
{
    XID	    gcvals[1];
    int     x, y;

    if (sourceGC->fgPixel != source)
    {
	gcvals[0] = source;
	DoChangeGC (sourceGC, GCForeground, gcvals, 0);
    }
    if (sourceGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, sourceGC);

    if(sourceGC->miTranslate) 
    {
        x = pDrawable->x + x_org;
        y = pDrawable->y + y_org;
    } 
    else
    {
        x = x_org;
        y = y_org;
    }

    (*sourceGC->ops->PushPixels) (sourceGC, pPriv->sourceBits, pDrawable, w, h, x, y);
    if (maskGC->fgPixel != mask)
    {
	gcvals[0] = mask;
	DoChangeGC (maskGC, GCForeground, gcvals, 0);
    }
    if (maskGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, maskGC);

    if(maskGC->miTranslate) 
    {
        x = pDrawable->x + x_org;
        y = pDrawable->y + y_org;
    } 
    else
    {
        x = x_org;
        y = y_org;
    }

    (*maskGC->ops->PushPixels) (maskGC, pPriv->maskBits, pDrawable, w, h, x, y);
}

static GCPtr
miDCMakeGC(WindowPtr pWin)
{
    GCPtr pGC;
    int   status;
    XID   gcvals[2];

    gcvals[0] = IncludeInferiors;
    gcvals[1] = FALSE;
    pGC = CreateGC((DrawablePtr)pWin,
		   GCSubwindowMode|GCGraphicsExposures, gcvals, &status,
		   (XID)0, serverClient);
    return pGC;
}


static Bool
miDCPutUpCursor (DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor,
                 int x, int y, unsigned long source, unsigned long mask)
{
    miDCScreenPtr   pScreenPriv;
    miDCCursorPtr   pPriv;
    miDCBufferPtr   pBuffer;
    WindowPtr	    pWin;

    pPriv = (miDCCursorPtr)dixLookupPrivate(&pCursor->bits->devPrivates,
					    CursorScreenKey(pScreen));
    if (!pPriv)
    {
	pPriv = miDCRealize(pScreen, pCursor);
	if (!pPriv)
	    return FALSE;
    }
    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pWin = WindowTable[pScreen->myNum];
    pBuffer = MIDCBUFFER(pDev, pScreen);

#ifdef ARGB_CURSOR
    if (pPriv->pPicture)
    {
	CompositePicture (PictOpOver,
			  pPriv->pPicture,
			  NULL,
			  pBuffer->pRootPicture,
			  0, 0, 0, 0, 
			  x, y, 
			  pCursor->bits->width,
			  pCursor->bits->height);
    }
    else
#endif
    {
	miDCPutBits ((DrawablePtr)pWin, pPriv,
		     pBuffer->pSourceGC, pBuffer->pMaskGC,
		     x, y, pCursor->bits->width, pCursor->bits->height,
		     source, mask);
    }
    return TRUE;
}

static Bool
miDCSaveUnderCursor (DeviceIntPtr pDev, ScreenPtr pScreen,
                     int x, int y, int w, int h)
{
    miDCScreenPtr   pScreenPriv;
    miDCBufferPtr   pBuffer;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pBuffer = MIDCBUFFER(pDev, pScreen);

    pSave = pBuffer->pSave;
    pWin = WindowTable[pScreen->myNum];
    if (!pSave || pSave->drawable.width < w || pSave->drawable.height < h)
    {
	if (pSave)
	    (*pScreen->DestroyPixmap) (pSave);
	pBuffer->pSave = pSave =
		(*pScreen->CreatePixmap) (pScreen, w, h, pScreen->rootDepth, 0);
	if (!pSave)
	    return FALSE;
    }

    pGC = pBuffer->pSaveGC;
    if (pSave->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pSave, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			    x, y, w, h, 0, 0);
    return TRUE;
}

static Bool
miDCRestoreUnderCursor (DeviceIntPtr pDev, ScreenPtr pScreen,
                        int x, int y, int w, int h)
{
    miDCScreenPtr   pScreenPriv;
    miDCBufferPtr   pBuffer;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pBuffer = MIDCBUFFER(pDev, pScreen);
    pSave = pBuffer->pSave;

    pWin = WindowTable[pScreen->myNum];
    if (!pSave)
	return FALSE;

    pGC = pBuffer->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}

static Bool
miDCChangeSave (DeviceIntPtr pDev, ScreenPtr pScreen,
                int x, int y, int w, int h, int dx, int dy)
{
    miDCScreenPtr   pScreenPriv;
    miDCBufferPtr   pBuffer;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    int		    sourcex, sourcey, destx, desty, copyw, copyh;

    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pBuffer = MIDCBUFFER(pDev, pScreen);

    pSave = pBuffer->pSave;
    pWin = WindowTable[pScreen->myNum];
    /*
     * restore the bits which are about to get trashed
     */
    if (!pSave)
	return FALSE;

    pGC = pBuffer->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);
    /*
     * copy the old bits to the screen.
     */
    if (dy > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, h - dy, w, dy, x + dx, y + h);
    }
    else if (dy < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, 0, w, -dy, x + dx, y + dy);
    }
    if (dy >= 0)
    {
	desty = y + dy;
	sourcey = 0;
	copyh = h - dy;
    }
    else
    {
	desty = y;
	sourcey = - dy;
	copyh = h + dy;
    }
    if (dx > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       w - dx, sourcey, dx, copyh, x + w, desty);
    }
    else if (dx < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, sourcey, -dx, copyh, x + dx, desty);
    }

    pGC = pBuffer->pSaveGC;
    if (pSave->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pSave, pGC);
    /*
     * move the bits that are still valid within the pixmap
     */
    if (dx >= 0)
    {
	sourcex = 0;
	destx = dx;
	copyw = w - dx;
    }
    else
    {
	destx = 0;
	sourcex = - dx;
	copyw = w + dx;
    }
    if (dy >= 0)
    {
	sourcey = 0;
	desty = dy;
	copyh = h - dy;
    }
    else
    {
	desty = 0;
	sourcey = -dy;
	copyh = h + dy;
    }
    (*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pSave, pGC,
			   sourcex, sourcey, copyw, copyh, destx, desty);
    /*
     * copy the new bits from the screen into the remaining areas of the
     * pixmap
     */
    if (dy > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, y, w, dy, 0, 0);
    }
    else if (dy < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, y + h + dy, w, -dy, 0, h + dy);
    }
    if (dy >= 0)
    {
	desty = dy;
	sourcey = y + dy;
	copyh = h - dy;
    }
    else
    {
	desty = 0;
	sourcey = y;
	copyh = h + dy;
    }
    if (dx > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, sourcey, dx, copyh, 0, desty);
    }
    else if (dx < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x + w + dx, sourcey, -dx, copyh, w + dx, desty);
    }
    return TRUE;
}

static Bool
miDCMoveCursor (DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor,
                int x, int y, int w, int h, int dx, int dy,
                unsigned long source, unsigned long mask)
{
    miDCCursorPtr   pPriv;
    miDCScreenPtr   pScreenPriv;
    miDCBufferPtr   pBuffer;
    int		    status;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    XID		    gcval = FALSE;
    PixmapPtr	    pTemp;

    pPriv = (miDCCursorPtr)dixLookupPrivate(&pCursor->bits->devPrivates,
					    CursorScreenKey(pScreen));
    if (!pPriv)
    {
	pPriv = miDCRealize(pScreen, pCursor);
	if (!pPriv)
	    return FALSE;
    }
    pScreenPriv = (miDCScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						  miDCScreenKey);
    pWin = WindowTable[pScreen->myNum];
    pBuffer = MIDCBUFFER(pDev, pScreen);

    pTemp = pBuffer->pTemp;
    if (!pTemp ||
	pTemp->drawable.width != pBuffer->pSave->drawable.width ||
	pTemp->drawable.height != pBuffer->pSave->drawable.height)
    {
	if (pTemp)
	    (*pScreen->DestroyPixmap) (pTemp);
#ifdef ARGB_CURSOR
	if (pBuffer->pTempPicture)
	{
	    FreePicture (pBuffer->pTempPicture, 0);
	    pBuffer->pTempPicture = 0;
	}
#endif
	pBuffer->pTemp = pTemp = (*pScreen->CreatePixmap)
	    (pScreen, w, h, pBuffer->pSave->drawable.depth, 0);
	if (!pTemp)
	    return FALSE;
    }
    if (!pBuffer->pMoveGC)
    {
	pBuffer->pMoveGC = CreateGC ((DrawablePtr)pTemp,
	    GCGraphicsExposures, &gcval, &status, (XID)0, serverClient);
	if (!pBuffer->pMoveGC)
	    return FALSE;
    }
    /*
     * copy the saved area to a temporary pixmap
     */
    pGC = pBuffer->pMoveGC;
    if (pGC->serialNumber != pTemp->drawable.serialNumber)
	ValidateGC ((DrawablePtr) pTemp, pGC);
    (*pGC->ops->CopyArea)((DrawablePtr)pBuffer->pSave,
			  (DrawablePtr)pTemp, pGC, 0, 0, w, h, 0, 0);
    
    /*
     * draw the cursor in the temporary pixmap
     */
#ifdef ARGB_CURSOR
    if (pPriv->pPicture)
    {
	if (!pBuffer->pTempPicture)
            miDCMakePicture(&pBuffer->pTempPicture, &pTemp->drawable, pWin);

	CompositePicture (PictOpOver,
			  pPriv->pPicture,
			  NULL,
			  pBuffer->pTempPicture,
			  0, 0, 0, 0, 
			  dx, dy, 
			  pCursor->bits->width,
			  pCursor->bits->height);
    }
    else
#endif
    {
	miDCPutBits ((DrawablePtr)pTemp, pPriv,
		     pBuffer->pPixSourceGC, pBuffer->pPixMaskGC,
		     dx, dy, pCursor->bits->width, pCursor->bits->height,
		     source, mask);
    }

    pGC = pBuffer->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);

    (*pGC->ops->CopyArea) ((DrawablePtr) pTemp, (DrawablePtr) pWin,
			    pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}

static Bool
miDCDeviceInitialize(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miDCBufferPtr   pBuffer;
    WindowPtr       pWin;
    XID             gcval = FALSE;
    int             status;
    int             i;

    if (!DevHasCursor(pDev))
        return TRUE;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
        pScreen = screenInfo.screens[i];

        pBuffer = xalloc(sizeof(miDCBufferRec));
        if (!pBuffer)
            goto failure;

        dixSetPrivate(&pDev->devPrivates, miDCSpriteKey + pScreen->myNum, pBuffer);
        pWin = WindowTable[pScreen->myNum];

        pBuffer->pSourceGC = miDCMakeGC(pWin);
        if (!pBuffer->pSourceGC)
            goto failure;

        pBuffer->pMaskGC = miDCMakeGC(pWin);
        if (!pBuffer->pMaskGC)
            goto failure;

        pBuffer->pSaveGC = miDCMakeGC(pWin);
        if (!pBuffer->pSaveGC)
            goto failure;

        pBuffer->pRestoreGC = miDCMakeGC(pWin);
        if (!pBuffer->pRestoreGC)
            goto failure;

        pBuffer->pMoveGC = CreateGC ((DrawablePtr)pWin,
            GCGraphicsExposures, &gcval, &status, (XID)0, serverClient);
        if (!pBuffer->pMoveGC)
            goto failure;

        pBuffer->pPixSourceGC = CreateGC ((DrawablePtr)pWin,
            GCGraphicsExposures, &gcval, &status, (XID)0, serverClient);
        if (!pBuffer->pPixSourceGC)
            goto failure;

        pBuffer->pPixMaskGC = CreateGC ((DrawablePtr)pWin,
            GCGraphicsExposures, &gcval, &status, (XID)0, serverClient);
        if (!pBuffer->pPixMaskGC)
            goto failure;

#ifdef ARGB_CURSOR
        miDCMakePicture(&pBuffer->pRootPicture, &pWin->drawable, pWin);
        if (!pBuffer->pRootPicture)
            goto failure;

        pBuffer->pTempPicture = NULL;
#endif

        // these get (re)allocated lazily depending on the cursor size
        pBuffer->pSave = pBuffer->pTemp = NULL;
    }

    return TRUE;

failure:

    miDCDeviceCleanup(pDev, pScreen);

    return FALSE;
}

static void
miDCDeviceCleanup(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miDCBufferPtr   pBuffer;
    int             i;

    if (DevHasCursor(pDev))
    {
        for (i = 0; i < screenInfo.numScreens; i++)
        {
            pScreen = screenInfo.screens[i];

            pBuffer = MIDCBUFFER(pDev, pScreen);

            if (pBuffer)
            {
                if (pBuffer->pSourceGC) FreeGC(pBuffer->pSourceGC, (GContext) 0);
                if (pBuffer->pMaskGC) FreeGC(pBuffer->pMaskGC, (GContext) 0);
                if (pBuffer->pSaveGC) FreeGC(pBuffer->pSaveGC, (GContext) 0);
                if (pBuffer->pRestoreGC) FreeGC(pBuffer->pRestoreGC, (GContext) 0);
                if (pBuffer->pMoveGC) FreeGC(pBuffer->pMoveGC, (GContext) 0);
                if (pBuffer->pPixSourceGC) FreeGC(pBuffer->pPixSourceGC, (GContext) 0);
                if (pBuffer->pPixMaskGC) FreeGC(pBuffer->pPixMaskGC, (GContext) 0);

                if (pBuffer->pSave) (*pScreen->DestroyPixmap)(pBuffer->pSave);
                if (pBuffer->pTemp) (*pScreen->DestroyPixmap)(pBuffer->pTemp);

                xfree(pBuffer);
                dixSetPrivate(&pDev->devPrivates, miDCSpriteKey + pScreen->myNum, NULL);
            }
        }
    }
}
