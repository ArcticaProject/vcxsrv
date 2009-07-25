/*
 * XFree86 Xv DDX written by Mark Vojkovich (markv@valinux.com)
 */
/*
 * Copyright (c) 1998-2003 by The XFree86 Project, Inc.
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
 */


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"

#include <X11/X.h>
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "regionstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "mivalidate.h"
#include "validate.h"
#include "resource.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>
#include "xvdix.h"
#include "xvmodproc.h"

#include "xf86xvpriv.h"


/* XvScreenRec fields */

static Bool xf86XVCloseScreen(int, ScreenPtr);
static int xf86XVQueryAdaptors(ScreenPtr, XvAdaptorPtr *, int *);

/* XvAdaptorRec fields */

static int xf86XVAllocatePort(unsigned long, XvPortPtr, XvPortPtr*);
static int xf86XVFreePort(XvPortPtr);
static int xf86XVPutVideo(ClientPtr, DrawablePtr,XvPortPtr, GCPtr,
				INT16, INT16, CARD16, CARD16,
				INT16, INT16, CARD16, CARD16);
static int xf86XVPutStill(ClientPtr, DrawablePtr,XvPortPtr, GCPtr,
				INT16, INT16, CARD16, CARD16,
				INT16, INT16, CARD16, CARD16);
static int xf86XVGetVideo(ClientPtr, DrawablePtr,XvPortPtr, GCPtr,
				INT16, INT16, CARD16, CARD16,
				INT16, INT16, CARD16, CARD16);
static int xf86XVGetStill(ClientPtr, DrawablePtr,XvPortPtr, GCPtr,
				INT16, INT16, CARD16, CARD16,
				INT16, INT16, CARD16, CARD16);
static int xf86XVStopVideo(ClientPtr, XvPortPtr, DrawablePtr);
static int xf86XVSetPortAttribute(ClientPtr, XvPortPtr, Atom, INT32);
static int xf86XVGetPortAttribute(ClientPtr, XvPortPtr, Atom, INT32*);
static int xf86XVQueryBestSize(ClientPtr, XvPortPtr, CARD8,
				CARD16, CARD16,CARD16, CARD16,
				unsigned int*, unsigned int*);
static int xf86XVPutImage(ClientPtr, DrawablePtr, XvPortPtr, GCPtr,
				INT16, INT16, CARD16, CARD16,
				INT16, INT16, CARD16, CARD16,
				XvImagePtr, unsigned char*, Bool,
				CARD16, CARD16);
static int xf86XVQueryImageAttributes(ClientPtr, XvPortPtr, XvImagePtr,
				CARD16*, CARD16*, int*, int*);


/* ScreenRec fields */

static Bool xf86XVDestroyWindow(WindowPtr pWin);
static void xf86XVWindowExposures(WindowPtr pWin, RegionPtr r1, RegionPtr r2);
static void xf86XVClipNotify(WindowPtr pWin, int dx, int dy);

/* ScrnInfoRec functions */

static Bool xf86XVEnterVT(int, int);
static void xf86XVLeaveVT(int, int);
static void xf86XVAdjustFrame(int index, int x, int y, int flags);

/* misc */

static Bool xf86XVInitAdaptors(ScreenPtr, XF86VideoAdaptorPtr*, int);


static int XF86XVWindowKeyIndex;
static DevPrivateKey XF86XVWindowKey = &XF86XVWindowKeyIndex;
static int XF86XvScreenKeyIndex;
DevPrivateKey XF86XvScreenKey = &XF86XvScreenKeyIndex;
static unsigned long PortResource = 0;

DevPrivateKey (*XvGetScreenKeyProc)(void) = NULL;
unsigned long (*XvGetRTPortProc)(void) = NULL;
int (*XvScreenInitProc)(ScreenPtr) = NULL;

#define GET_XV_SCREEN(pScreen) \
    ((XvScreenPtr)dixLookupPrivate(&(pScreen)->devPrivates, XF86XvScreenKey))

#define GET_XF86XV_SCREEN(pScreen) \
    ((XF86XVScreenPtr)(GET_XV_SCREEN(pScreen)->devPriv.ptr))

#define GET_XF86XV_WINDOW(pWin) \
    ((XF86XVWindowPtr)dixLookupPrivate(&(pWin)->devPrivates, XF86XVWindowKey))

static xf86XVInitGenericAdaptorPtr *GenDrivers = NULL;
static int NumGenDrivers = 0;

_X_EXPORT int
xf86XVRegisterGenericAdaptorDriver(
    xf86XVInitGenericAdaptorPtr InitFunc
){
  xf86XVInitGenericAdaptorPtr *newdrivers;

  newdrivers = xrealloc(GenDrivers, sizeof(xf86XVInitGenericAdaptorPtr) *
			(1 + NumGenDrivers));
  if (!newdrivers)
    return 0;
  GenDrivers = newdrivers;

  GenDrivers[NumGenDrivers++] = InitFunc;

  return 1;
}

_X_EXPORT int
xf86XVListGenericAdaptors(
    ScrnInfoPtr pScrn,
    XF86VideoAdaptorPtr **adaptors
){
    int i,j,n,num;
    XF86VideoAdaptorPtr *DrivAdap,*new;

    num = 0;
    *adaptors = NULL;
    /*
     * The v4l driver registers itself first, but can use surfaces registered
     * by other drivers.  So, call the v4l driver last.
     */
    for (i = NumGenDrivers; --i >= 0; ) {
	DrivAdap = NULL;
	n = (*GenDrivers[i])(pScrn, &DrivAdap);
	if (0 == n)
	    continue;
	new = xrealloc(*adaptors, sizeof(XF86VideoAdaptorPtr) * (num+n));
	if (NULL == new)
	    continue;
	*adaptors = new;
	for (j = 0; j < n; j++, num++)
	    (*adaptors)[num] = DrivAdap[j];
    }
    return num;
}


/****************  Offscreen surface stuff *******************/

typedef struct {
   XF86OffscreenImagePtr images;
   int num;
} OffscreenImageRec;

static OffscreenImageRec OffscreenImages[MAXSCREENS];

_X_EXPORT Bool
xf86XVRegisterOffscreenImages(
    ScreenPtr pScreen,
    XF86OffscreenImagePtr images,
    int num
){
    OffscreenImages[pScreen->myNum].num = num;
    OffscreenImages[pScreen->myNum].images = images;

    return TRUE;
}

_X_EXPORT XF86OffscreenImagePtr
xf86XVQueryOffscreenImages(
   ScreenPtr pScreen,
   int *num
){
   *num = OffscreenImages[pScreen->myNum].num;
   return OffscreenImages[pScreen->myNum].images;
}


_X_EXPORT XF86VideoAdaptorPtr
xf86XVAllocateVideoAdaptorRec(ScrnInfoPtr pScrn)
{
    return xcalloc(1, sizeof(XF86VideoAdaptorRec));
}

_X_EXPORT void
xf86XVFreeVideoAdaptorRec(XF86VideoAdaptorPtr ptr)
{
    xfree(ptr);
}


_X_EXPORT Bool
xf86XVScreenInit(
   ScreenPtr pScreen,
   XF86VideoAdaptorPtr *adaptors,
   int num
){
  ScrnInfoPtr pScrn;
  XF86XVScreenPtr ScreenPriv;
  XvScreenPtr pxvs;

  if(num <= 0 ||
     !XvGetScreenKeyProc || !XvGetRTPortProc || !XvScreenInitProc)
	return FALSE;

  if(Success != (*XvScreenInitProc)(pScreen)) return FALSE;

  XF86XvScreenKey = (*XvGetScreenKeyProc)();
  PortResource = (*XvGetRTPortProc)();

  pxvs = GET_XV_SCREEN(pScreen);

  /* Anyone initializing the Xv layer must provide these two.
     The Xv di layer calls them without even checking if they exist! */

  pxvs->ddCloseScreen = xf86XVCloseScreen;
  pxvs->ddQueryAdaptors = xf86XVQueryAdaptors;

  /* The Xv di layer provides us with a private hook so that we don't
     have to allocate our own screen private.  They also provide
     a CloseScreen hook so that we don't have to wrap it.  I'm not
     sure that I appreciate that.  */

  ScreenPriv = xalloc(sizeof(XF86XVScreenRec));
  pxvs->devPriv.ptr = (pointer)ScreenPriv;

  if(!ScreenPriv) return FALSE;

  pScrn = xf86Screens[pScreen->myNum];

  ScreenPriv->videoGC = NULL;  /* for the helper */

  ScreenPriv->DestroyWindow = pScreen->DestroyWindow;
  ScreenPriv->WindowExposures = pScreen->WindowExposures;
  ScreenPriv->ClipNotify = pScreen->ClipNotify;
  ScreenPriv->EnterVT = pScrn->EnterVT;
  ScreenPriv->LeaveVT = pScrn->LeaveVT;
  ScreenPriv->AdjustFrame = pScrn->AdjustFrame;

  pScreen->DestroyWindow = xf86XVDestroyWindow;
  pScreen->WindowExposures = xf86XVWindowExposures;
  pScreen->ClipNotify = xf86XVClipNotify;
  pScrn->EnterVT = xf86XVEnterVT;
  pScrn->LeaveVT = xf86XVLeaveVT;
  if(pScrn->AdjustFrame)
     pScrn->AdjustFrame = xf86XVAdjustFrame;

  if(!xf86XVInitAdaptors(pScreen, adaptors, num))
	return FALSE;

  return TRUE;
}

static void
xf86XVFreeAdaptor(XvAdaptorPtr pAdaptor)
{
   int i;

   xfree(pAdaptor->name);

   if(pAdaptor->pEncodings) {
      XvEncodingPtr pEncode = pAdaptor->pEncodings;

      for(i = 0; i < pAdaptor->nEncodings; i++, pEncode++)
	  xfree(pEncode->name);
      xfree(pAdaptor->pEncodings);
   }

   xfree(pAdaptor->pFormats);

   if(pAdaptor->pPorts) {
      XvPortPtr pPort = pAdaptor->pPorts;
      XvPortRecPrivatePtr pPriv;

      for(i = 0; i < pAdaptor->nPorts; i++, pPort++) {
	  pPriv = (XvPortRecPrivatePtr)pPort->devPriv.ptr;
	  if(pPriv) {
	     if(pPriv->clientClip)
		REGION_DESTROY(pAdaptor->pScreen, pPriv->clientClip);
	     if(pPriv->pCompositeClip && pPriv->FreeCompositeClip)
		REGION_DESTROY(pAdaptor->pScreen, pPriv->pCompositeClip);
	     xfree(pPriv);
	  }
      }
      xfree(pAdaptor->pPorts);
   }

   if(pAdaptor->nAttributes) {
      XvAttributePtr pAttribute = pAdaptor->pAttributes;

      for(i = 0; i < pAdaptor->nAttributes; i++, pAttribute++)
	  xfree(pAttribute->name);
      xfree(pAdaptor->pAttributes);
   }

   xfree(pAdaptor->pImages);
   xfree(pAdaptor->devPriv.ptr);
}

static Bool
xf86XVInitAdaptors(
   ScreenPtr pScreen,
   XF86VideoAdaptorPtr *infoPtr,
   int number
) {
  XvScreenPtr pxvs = GET_XV_SCREEN(pScreen);
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  XF86VideoAdaptorPtr adaptorPtr;
  XvAdaptorPtr pAdaptor, pa;
  XvAdaptorRecPrivatePtr adaptorPriv;
  int na, numAdaptor;
  XvPortRecPrivatePtr portPriv;
  XvPortPtr pPort, pp;
  int numPort;
  XF86AttributePtr attributePtr;
  XvAttributePtr pAttribute, pat;
  XF86VideoFormatPtr formatPtr;
  XvFormatPtr pFormat, pf;
  int numFormat, totFormat;
  XF86VideoEncodingPtr encodingPtr;
  XvEncodingPtr pEncode, pe;
  XF86ImagePtr imagePtr;
  XvImagePtr pImage, pi;
  int numVisuals;
  VisualPtr pVisual;
  int i;

  pxvs->nAdaptors = 0;
  pxvs->pAdaptors = NULL;

  if(!(pAdaptor = xcalloc(number, sizeof(XvAdaptorRec))))
      return FALSE;

  for(pa = pAdaptor, na = 0, numAdaptor = 0; na < number; na++, adaptorPtr++) {
      adaptorPtr = infoPtr[na];

      if(!adaptorPtr->StopVideo || !adaptorPtr->SetPortAttribute ||
	 !adaptorPtr->GetPortAttribute || !adaptorPtr->QueryBestSize)
	   continue;

      /* client libs expect at least one encoding */
      if(!adaptorPtr->nEncodings || !adaptorPtr->pEncodings)
	   continue;

      pa->type = adaptorPtr->type;

      if(!adaptorPtr->PutVideo && !adaptorPtr->GetVideo)
	 pa->type &= ~XvVideoMask;

      if(!adaptorPtr->PutStill && !adaptorPtr->GetStill)
	 pa->type &= ~XvStillMask;

      if(!adaptorPtr->PutImage || !adaptorPtr->QueryImageAttributes)
	 pa->type &= ~XvImageMask;

      if(!adaptorPtr->PutVideo && !adaptorPtr->PutImage &&
							  !adaptorPtr->PutStill)
	 pa->type &= ~XvInputMask;

      if(!adaptorPtr->GetVideo && !adaptorPtr->GetStill)
	 pa->type &= ~XvOutputMask;

      if(!(adaptorPtr->type & (XvPixmapMask | XvWindowMask)))
	  continue;
      if(!(adaptorPtr->type & (XvImageMask | XvVideoMask | XvStillMask)))
	  continue;

      pa->pScreen = pScreen;
      pa->ddAllocatePort = xf86XVAllocatePort;
      pa->ddFreePort = xf86XVFreePort;
      pa->ddPutVideo = xf86XVPutVideo;
      pa->ddPutStill = xf86XVPutStill;
      pa->ddGetVideo = xf86XVGetVideo;
      pa->ddGetStill = xf86XVGetStill;
      pa->ddStopVideo = xf86XVStopVideo;
      pa->ddPutImage = xf86XVPutImage;
      pa->ddSetPortAttribute = xf86XVSetPortAttribute;
      pa->ddGetPortAttribute = xf86XVGetPortAttribute;
      pa->ddQueryBestSize = xf86XVQueryBestSize;
      pa->ddQueryImageAttributes = xf86XVQueryImageAttributes;
      if((pa->name = xalloc(strlen(adaptorPtr->name) + 1)))
	  strcpy(pa->name, adaptorPtr->name);

      if(adaptorPtr->nEncodings &&
	(pEncode = xcalloc(adaptorPtr->nEncodings, sizeof(XvEncodingRec)))) {

	for(pe = pEncode, encodingPtr = adaptorPtr->pEncodings, i = 0;
	    i < adaptorPtr->nEncodings; pe++, i++, encodingPtr++)
	{
	    pe->id = encodingPtr->id;
	    pe->pScreen = pScreen;
	    if((pe->name = xalloc(strlen(encodingPtr->name) + 1)))
		strcpy(pe->name, encodingPtr->name);
	    pe->width = encodingPtr->width;
	    pe->height = encodingPtr->height;
	    pe->rate.numerator = encodingPtr->rate.numerator;
	    pe->rate.denominator = encodingPtr->rate.denominator;
	}
	pa->nEncodings = adaptorPtr->nEncodings;
	pa->pEncodings = pEncode;
      }

      if(adaptorPtr->nImages &&
	 (pImage = xcalloc(adaptorPtr->nImages, sizeof(XvImageRec)))) {

	  for(i = 0, pi = pImage, imagePtr = adaptorPtr->pImages;
	      i < adaptorPtr->nImages; i++, pi++, imagePtr++)
	  {
	     pi->id = imagePtr->id;
	     pi->type = imagePtr->type;
	     pi->byte_order = imagePtr->byte_order;
	     memcpy(pi->guid, imagePtr->guid, 16);
	     pi->bits_per_pixel = imagePtr->bits_per_pixel;
	     pi->format = imagePtr->format;
	     pi->num_planes = imagePtr->num_planes;
	     pi->depth = imagePtr->depth;
	     pi->red_mask = imagePtr->red_mask;
	     pi->green_mask = imagePtr->green_mask;
	     pi->blue_mask = imagePtr->blue_mask;
	     pi->y_sample_bits = imagePtr->y_sample_bits;
	     pi->u_sample_bits = imagePtr->u_sample_bits;
	     pi->v_sample_bits = imagePtr->v_sample_bits;
	     pi->horz_y_period = imagePtr->horz_y_period;
	     pi->horz_u_period = imagePtr->horz_u_period;
	     pi->horz_v_period = imagePtr->horz_v_period;
	     pi->vert_y_period = imagePtr->vert_y_period;
	     pi->vert_u_period = imagePtr->vert_u_period;
	     pi->vert_v_period = imagePtr->vert_v_period;
	     memcpy(pi->component_order, imagePtr->component_order, 32);
	     pi->scanline_order = imagePtr->scanline_order;
	  }
	  pa->nImages = adaptorPtr->nImages;
	  pa->pImages = pImage;
      }

      if(adaptorPtr->nAttributes &&
	(pAttribute = xcalloc(adaptorPtr->nAttributes, sizeof(XvAttributeRec))))
      {
	for(pat = pAttribute, attributePtr = adaptorPtr->pAttributes, i = 0;
	    i < adaptorPtr->nAttributes; pat++, i++, attributePtr++)
	{
	    pat->flags = attributePtr->flags;
	    pat->min_value = attributePtr->min_value;
	    pat->max_value = attributePtr->max_value;
	    if((pat->name = xalloc(strlen(attributePtr->name) + 1)))
		strcpy(pat->name, attributePtr->name);
	}
	pa->nAttributes = adaptorPtr->nAttributes;
	pa->pAttributes = pAttribute;
      }


      totFormat = adaptorPtr->nFormats;

      if(!(pFormat = xcalloc(totFormat, sizeof(XvFormatRec)))) {
	  xf86XVFreeAdaptor(pa);
	  continue;
      }
      for(pf = pFormat, i = 0, numFormat = 0, formatPtr = adaptorPtr->pFormats;
	  i < adaptorPtr->nFormats; i++, formatPtr++)
      {
	  numVisuals = pScreen->numVisuals;
	  pVisual = pScreen->visuals;

	  while(numVisuals--) {
	      if((pVisual->class == formatPtr->class) &&
		 (pVisual->nplanes == formatPtr->depth)) {

		   if(numFormat >= totFormat) {
			void *moreSpace;
			totFormat *= 2;
			moreSpace = xrealloc(pFormat,
					     totFormat * sizeof(XvFormatRec));
			if(!moreSpace) break;
			pFormat = moreSpace;
			pf = pFormat + numFormat;
		   }

		   pf->visual = pVisual->vid;
		   pf->depth = formatPtr->depth;

		   pf++;
		   numFormat++;
	      }
	      pVisual++;
	  }
      }
      pa->nFormats = numFormat;
      pa->pFormats = pFormat;
      if(!numFormat) {
	  xf86XVFreeAdaptor(pa);
	  continue;
      }

      if(!(adaptorPriv = xcalloc(1, sizeof(XvAdaptorRecPrivate)))) {
	  xf86XVFreeAdaptor(pa);
	  continue;
      }

      adaptorPriv->flags = adaptorPtr->flags;
      adaptorPriv->PutVideo = adaptorPtr->PutVideo;
      adaptorPriv->PutStill = adaptorPtr->PutStill;
      adaptorPriv->GetVideo = adaptorPtr->GetVideo;
      adaptorPriv->GetStill = adaptorPtr->GetStill;
      adaptorPriv->StopVideo = adaptorPtr->StopVideo;
      adaptorPriv->SetPortAttribute = adaptorPtr->SetPortAttribute;
      adaptorPriv->GetPortAttribute = adaptorPtr->GetPortAttribute;
      adaptorPriv->QueryBestSize = adaptorPtr->QueryBestSize;
      adaptorPriv->QueryImageAttributes = adaptorPtr->QueryImageAttributes;
      adaptorPriv->PutImage = adaptorPtr->PutImage;
      adaptorPriv->ReputImage = adaptorPtr->ReputImage;

      pa->devPriv.ptr = (pointer)adaptorPriv;

      if(!(pPort = xcalloc(adaptorPtr->nPorts, sizeof(XvPortRec)))) {
	  xf86XVFreeAdaptor(pa);
	  continue;
      }
      for(pp = pPort, i = 0, numPort = 0;
	  i < adaptorPtr->nPorts; i++) {

	  if(!(pp->id = FakeClientID(0)))
		continue;

	  if(!(portPriv = xcalloc(1, sizeof(XvPortRecPrivate))))
		continue;

	  if(!AddResource(pp->id, PortResource, pp)) {
		xfree(portPriv);
		continue;
	  }

	  pp->pAdaptor = pa;
	  pp->pNotify = (XvPortNotifyPtr)NULL;
	  pp->pDraw = (DrawablePtr)NULL;
	  pp->client = (ClientPtr)NULL;
	  pp->grab.client = (ClientPtr)NULL;
	  pp->time = currentTime;
	  pp->devPriv.ptr = portPriv;

	  portPriv->pScrn = pScrn;
	  portPriv->AdaptorRec = adaptorPriv;
	  portPriv->DevPriv.ptr = adaptorPtr->pPortPrivates[i].ptr;

	  pp++;
	  numPort++;
      }
      pa->nPorts = numPort;
      pa->pPorts = pPort;
      if(!numPort) {
	  xf86XVFreeAdaptor(pa);
	  continue;
      }

      pa->base_id = pPort->id;

      pa++;
      numAdaptor++;
  }

  if(numAdaptor) {
      pxvs->nAdaptors = numAdaptor;
      pxvs->pAdaptors = pAdaptor;
  } else {
     xfree(pAdaptor);
     return FALSE;
  }

  return TRUE;
}

/* Video should be clipped to the intersection of the window cliplist
   and the client cliplist specified in the GC for which the video was
   initialized.  When we need to reclip a window, the GC that started
   the video may not even be around anymore.  That's why we save the
   client clip from the GC when the video is initialized.  We then
   use xf86XVUpdateCompositeClip to calculate the new composite clip
   when we need it.  This is different from what DEC did.  They saved
   the GC and used it's clip list when they needed to reclip the window,
   even if the client clip was different from the one the video was
   initialized with.  If the original GC was destroyed, they had to stop
   the video.  I like the new method better (MArk).

   This function only works for windows.  Will need to rewrite when
   (if) we support pixmap rendering.
*/

static void
xf86XVUpdateCompositeClip(XvPortRecPrivatePtr portPriv)
{
   RegionPtr	pregWin, pCompositeClip;
   WindowPtr	pWin;
   ScreenPtr	pScreen;
   Bool		freeCompClip = FALSE;

   if(portPriv->pCompositeClip)
	return;

   pWin = (WindowPtr)portPriv->pDraw;
   pScreen = pWin->drawable.pScreen;

   /* get window clip list */
   if(portPriv->subWindowMode == IncludeInferiors) {
	pregWin = NotClippedByChildren(pWin);
	freeCompClip = TRUE;
   } else
	pregWin = &pWin->clipList;

   if(!portPriv->clientClip) {
	portPriv->pCompositeClip = pregWin;
	portPriv->FreeCompositeClip = freeCompClip;
	return;
   }

   pCompositeClip = REGION_CREATE(pScreen, NullBox, 1);
   REGION_COPY(pScreen, pCompositeClip, portPriv->clientClip);
   REGION_TRANSLATE(pScreen, pCompositeClip,
			portPriv->pDraw->x + portPriv->clipOrg.x,
			portPriv->pDraw->y + portPriv->clipOrg.y);
   REGION_INTERSECT(pScreen, pCompositeClip, pregWin, pCompositeClip);

   portPriv->pCompositeClip = pCompositeClip;
   portPriv->FreeCompositeClip = TRUE;

   if(freeCompClip) {
	REGION_DESTROY(pScreen, pregWin);
   }
}

/* Save the current clientClip and update the CompositeClip whenever
   we have a fresh GC */

static void
xf86XVCopyClip(
   XvPortRecPrivatePtr portPriv,
   GCPtr pGC
){
    /* copy the new clip if it exists */
    if((pGC->clientClipType == CT_REGION) && pGC->clientClip) {
	if(!portPriv->clientClip)
	    portPriv->clientClip = REGION_CREATE(pScreen, NullBox, 1);
	/* Note: this is in window coordinates */
	REGION_COPY(pScreen, portPriv->clientClip, pGC->clientClip);
    } else if(portPriv->clientClip) { /* free the old clientClip */
	REGION_DESTROY(pScreen, portPriv->clientClip);
	portPriv->clientClip = NULL;
    }

    /* get rid of the old clip list */
    if(portPriv->pCompositeClip && portPriv->FreeCompositeClip) {
	REGION_DESTROY(pScreen, portPriv->pCompositeClip);
    }

    portPriv->clipOrg = pGC->clipOrg;
    portPriv->pCompositeClip = pGC->pCompositeClip;
    portPriv->FreeCompositeClip = FALSE;
    portPriv->subWindowMode = pGC->subWindowMode;
}

static int
xf86XVRegetVideo(XvPortRecPrivatePtr portPriv)
{
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  ScreenPtr pScreen;
  int ret = Success;
  Bool clippedAway = FALSE;

  pScreen = portPriv->pDraw->pScreen;
  xf86XVUpdateCompositeClip(portPriv);

  /* translate the video region to the screen */
  WinBox.x1 = portPriv->pDraw->x + portPriv->drw_x;
  WinBox.y1 = portPriv->pDraw->y + portPriv->drw_y;
  WinBox.x2 = WinBox.x1 + portPriv->drw_w;
  WinBox.y2 = WinBox.y1 + portPriv->drw_h;

  /* clip to the window composite clip */
  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, portPriv->pCompositeClip);

  /* that's all if it's totally obscured */
  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
	clippedAway = TRUE;
	goto CLIP_VIDEO_BAILOUT;
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->GetVideo)(portPriv->pScrn,
			portPriv->vid_x, portPriv->vid_y,
			WinBox.x1, WinBox.y1,
			portPriv->vid_w, portPriv->vid_h,
			portPriv->drw_w, portPriv->drw_h,
			&ClipRegion, portPriv->DevPriv.ptr,
			portPriv->pDraw);

  if(ret == Success)
	portPriv->isOn = XV_ON;

CLIP_VIDEO_BAILOUT:

  if((clippedAway || (ret != Success)) && portPriv->isOn == XV_ON) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  /* This clip was copied and only good for one shot */
  if(!portPriv->FreeCompositeClip)
     portPriv->pCompositeClip = NULL;

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}


static int
xf86XVReputVideo(XvPortRecPrivatePtr portPriv)
{
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  ScreenPtr pScreen;
  int ret = Success;
  Bool clippedAway = FALSE;

  pScreen = portPriv->pDraw->pScreen;

  xf86XVUpdateCompositeClip(portPriv);

  /* translate the video region to the screen */
  WinBox.x1 = portPriv->pDraw->x + portPriv->drw_x;
  WinBox.y1 = portPriv->pDraw->y + portPriv->drw_y;
  WinBox.x2 = WinBox.x1 + portPriv->drw_w;
  WinBox.y2 = WinBox.y1 + portPriv->drw_h;

  /* clip to the window composite clip */
  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, portPriv->pCompositeClip);

  /* clip and translate to the viewport */
  if(portPriv->AdaptorRec->flags & VIDEO_CLIP_TO_VIEWPORT) {
     RegionRec VPReg;
     BoxRec VPBox;

     VPBox.x1 = portPriv->pScrn->frameX0;
     VPBox.y1 = portPriv->pScrn->frameY0;
     VPBox.x2 = portPriv->pScrn->frameX1;
     VPBox.y2 = portPriv->pScrn->frameY1;

     REGION_INIT(pScreen, &VPReg, &VPBox, 1);
     REGION_INTERSECT(pScreen, &ClipRegion, &ClipRegion, &VPReg);
     REGION_UNINIT(pScreen, &VPReg);
  }

  /* that's all if it's totally obscured */
  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
	clippedAway = TRUE;
	goto CLIP_VIDEO_BAILOUT;
  }

  /* bailout if we have to clip but the hardware doesn't support it */
  if(portPriv->AdaptorRec->flags & VIDEO_NO_CLIPPING) {
     BoxPtr clipBox = REGION_RECTS(&ClipRegion);
     if(  (REGION_NUM_RECTS(&ClipRegion) != 1) ||
	  (clipBox->x1 != WinBox.x1) || (clipBox->x2 != WinBox.x2) ||
	  (clipBox->y1 != WinBox.y1) || (clipBox->y2 != WinBox.y2))
     {
	    clippedAway = TRUE;
	    goto CLIP_VIDEO_BAILOUT;
     }
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->PutVideo)(portPriv->pScrn,
			portPriv->vid_x, portPriv->vid_y,
			WinBox.x1, WinBox.y1,
			portPriv->vid_w, portPriv->vid_h,
			portPriv->drw_w, portPriv->drw_h,
			&ClipRegion, portPriv->DevPriv.ptr,
			portPriv->pDraw);

  if(ret == Success) portPriv->isOn = XV_ON;

CLIP_VIDEO_BAILOUT:

  if((clippedAway || (ret != Success)) && (portPriv->isOn == XV_ON)) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  /* This clip was copied and only good for one shot */
  if(!portPriv->FreeCompositeClip)
     portPriv->pCompositeClip = NULL;

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}

static int
xf86XVReputImage(XvPortRecPrivatePtr portPriv)
{
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  ScreenPtr pScreen;
  int ret = Success;
  Bool clippedAway = FALSE;

  pScreen = portPriv->pDraw->pScreen;

  xf86XVUpdateCompositeClip(portPriv);

  /* translate the video region to the screen */
  WinBox.x1 = portPriv->pDraw->x + portPriv->drw_x;
  WinBox.y1 = portPriv->pDraw->y + portPriv->drw_y;
  WinBox.x2 = WinBox.x1 + portPriv->drw_w;
  WinBox.y2 = WinBox.y1 + portPriv->drw_h;

  /* clip to the window composite clip */
  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, portPriv->pCompositeClip);

  /* clip and translate to the viewport */
  if(portPriv->AdaptorRec->flags & VIDEO_CLIP_TO_VIEWPORT) {
     RegionRec VPReg;
     BoxRec VPBox;

     VPBox.x1 = portPriv->pScrn->frameX0;
     VPBox.y1 = portPriv->pScrn->frameY0;
     VPBox.x2 = portPriv->pScrn->frameX1;
     VPBox.y2 = portPriv->pScrn->frameY1;

     REGION_INIT(pScreen, &VPReg, &VPBox, 1);
     REGION_INTERSECT(pScreen, &ClipRegion, &ClipRegion, &VPReg);
     REGION_UNINIT(pScreen, &VPReg);
  }

  /* that's all if it's totally obscured */
  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
	clippedAway = TRUE;
	goto CLIP_VIDEO_BAILOUT;
  }

  /* bailout if we have to clip but the hardware doesn't support it */
  if(portPriv->AdaptorRec->flags & VIDEO_NO_CLIPPING) {
     BoxPtr clipBox = REGION_RECTS(&ClipRegion);
     if(  (REGION_NUM_RECTS(&ClipRegion) != 1) ||
	  (clipBox->x1 != WinBox.x1) || (clipBox->x2 != WinBox.x2) ||
	  (clipBox->y1 != WinBox.y1) || (clipBox->y2 != WinBox.y2))
     {
	    clippedAway = TRUE;
	    goto CLIP_VIDEO_BAILOUT;
     }
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->ReputImage)(portPriv->pScrn,
			WinBox.x1, WinBox.y1,
			&ClipRegion, portPriv->DevPriv.ptr,
			portPriv->pDraw);

  portPriv->isOn = (ret == Success) ? XV_ON : XV_OFF;

CLIP_VIDEO_BAILOUT:

  if((clippedAway || (ret != Success)) && (portPriv->isOn == XV_ON)) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  /* This clip was copied and only good for one shot */
  if(!portPriv->FreeCompositeClip)
     portPriv->pCompositeClip = NULL;

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}


static int
xf86XVReputAllVideo(WindowPtr pWin, pointer data)
{
    XF86XVWindowPtr WinPriv = GET_XF86XV_WINDOW(pWin);

    while(WinPriv) {
	if(WinPriv->PortRec->type == XvInputMask)
	    xf86XVReputVideo(WinPriv->PortRec);
	else
	    xf86XVRegetVideo(WinPriv->PortRec);
	WinPriv = WinPriv->next;
    }

    return WT_WALKCHILDREN;
}

static int
xf86XVEnlistPortInWindow(WindowPtr pWin, XvPortRecPrivatePtr portPriv)
{
   XF86XVWindowPtr winPriv, PrivRoot;

   winPriv = PrivRoot = GET_XF86XV_WINDOW(pWin);

  /* Enlist our port in the window private */
   while(winPriv) {
	if(winPriv->PortRec == portPriv) /* we're already listed */
	    break;
	winPriv = winPriv->next;
   }

   if(!winPriv) {
	winPriv = xcalloc(1, sizeof(XF86XVWindowRec));
	if(!winPriv) return BadAlloc;
	winPriv->PortRec = portPriv;
	winPriv->next = PrivRoot;
	dixSetPrivate(&pWin->devPrivates, XF86XVWindowKey, winPriv);
   }

   portPriv->pDraw = (DrawablePtr)pWin;

   return Success;
}


static void
xf86XVRemovePortFromWindow(WindowPtr pWin, XvPortRecPrivatePtr portPriv)
{
     XF86XVWindowPtr winPriv, prevPriv = NULL;

     winPriv = GET_XF86XV_WINDOW(pWin);

     while(winPriv) {
	if(winPriv->PortRec == portPriv) {
	    if(prevPriv)
		prevPriv->next = winPriv->next;
	    else
		dixSetPrivate(&pWin->devPrivates, XF86XVWindowKey,
			      winPriv->next);
	    xfree(winPriv);
	    break;
	}
	prevPriv = winPriv;
	winPriv = winPriv->next;
     }
     portPriv->pDraw = NULL;
}

/****  ScreenRec fields ****/

static Bool
xf86XVDestroyWindow(WindowPtr pWin)
{
  ScreenPtr pScreen = pWin->drawable.pScreen;
  XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
  XF86XVWindowPtr tmp, WinPriv = GET_XF86XV_WINDOW(pWin);
  int ret;

  while(WinPriv) {
     XvPortRecPrivatePtr pPriv = WinPriv->PortRec;

     if(pPriv->isOn > XV_OFF) {
	(*pPriv->AdaptorRec->StopVideo)(
			pPriv->pScrn, pPriv->DevPriv.ptr, TRUE);
	pPriv->isOn = XV_OFF;
     }

     pPriv->pDraw = NULL;
     tmp = WinPriv;
     if(WinPriv->pGC) {
       FreeGC(WinPriv->pGC, 0);
     }
     WinPriv = WinPriv->next;
     xfree(tmp);
  }

  dixSetPrivate(&pWin->devPrivates, XF86XVWindowKey, NULL);

  pScreen->DestroyWindow = ScreenPriv->DestroyWindow;
  ret = (*pScreen->DestroyWindow)(pWin);
  pScreen->DestroyWindow = xf86XVDestroyWindow;

  return ret;
}


static void
xf86XVWindowExposures(WindowPtr pWin, RegionPtr reg1, RegionPtr reg2)
{
  ScreenPtr pScreen = pWin->drawable.pScreen;
  XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
  XF86XVWindowPtr WinPriv = GET_XF86XV_WINDOW(pWin);
  XF86XVWindowPtr pPrev;
  XvPortRecPrivatePtr pPriv;
  Bool AreasExposed;

  AreasExposed = (WinPriv && reg1 && REGION_NOTEMPTY(pScreen, reg1));

  pScreen->WindowExposures = ScreenPriv->WindowExposures;
  (*pScreen->WindowExposures)(pWin, reg1, reg2);
  pScreen->WindowExposures = xf86XVWindowExposures;

  /* filter out XClearWindow/Area */
  if (!pWin->valdata) return;

  pPrev = NULL;

  while(WinPriv) {
     pPriv = WinPriv->PortRec;

     /* Reput anyone with a reput function */

     switch(pPriv->type) {
     case XvInputMask:
	xf86XVReputVideo(pPriv);
	break;
     case XvOutputMask:
	xf86XVRegetVideo(pPriv);
	break;
     default:  /* overlaid still/image*/
	if (pPriv->AdaptorRec->ReputImage)
	   xf86XVReputImage(pPriv);
	else if(AreasExposed) {
	    XF86XVWindowPtr tmp;

	    if (pPriv->isOn == XV_ON) {
		(*pPriv->AdaptorRec->StopVideo)(
		    pPriv->pScrn, pPriv->DevPriv.ptr, FALSE);
		pPriv->isOn = XV_PENDING;
	    }
	    pPriv->pDraw = NULL;

	    if(!pPrev)
		dixSetPrivate(&pWin->devPrivates, XF86XVWindowKey,
			      WinPriv->next);
	    else
	       pPrev->next = WinPriv->next;
	    tmp = WinPriv;
	    WinPriv = WinPriv->next;
	    xfree(tmp);
	    continue;
	}
	break;
     }
     pPrev = WinPriv;
     WinPriv = WinPriv->next;
  }
}


static void
xf86XVClipNotify(WindowPtr pWin, int dx, int dy)
{
  ScreenPtr pScreen = pWin->drawable.pScreen;
  XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
  XF86XVWindowPtr WinPriv = GET_XF86XV_WINDOW(pWin);
  XF86XVWindowPtr tmp, pPrev = NULL;
  XvPortRecPrivatePtr pPriv;
  Bool visible = (pWin->visibility == VisibilityUnobscured) ||
		 (pWin->visibility == VisibilityPartiallyObscured);

  while(WinPriv) {
     pPriv = WinPriv->PortRec;

     if(!pPriv) goto next;
 
     if(pPriv->pCompositeClip && pPriv->FreeCompositeClip)
	REGION_DESTROY(pScreen, pPriv->pCompositeClip);

     pPriv->pCompositeClip = NULL;

     /* Stop everything except images, but stop them too if the
	window isn't visible.  But we only remove the images. */

     if(pPriv->type || !visible) {
	if(pPriv->isOn == XV_ON) {
	    (*pPriv->AdaptorRec->StopVideo)(
			pPriv->pScrn, pPriv->DevPriv.ptr, FALSE);
	    pPriv->isOn = XV_PENDING;
	}

	if(!pPriv->type) {  /* overlaid still/image */
	    pPriv->pDraw = NULL;

	    if(!pPrev)
		dixSetPrivate(&pWin->devPrivates, XF86XVWindowKey,
			      WinPriv->next);
	    else
	       pPrev->next = WinPriv->next;
	    tmp = WinPriv;
	    WinPriv = WinPriv->next;
	    xfree(tmp);
	    continue;
	}
     }

next:
     pPrev = WinPriv;
     WinPriv = WinPriv->next;
  }

  if(ScreenPriv->ClipNotify) {
      pScreen->ClipNotify = ScreenPriv->ClipNotify;
      (*pScreen->ClipNotify)(pWin, dx, dy);
      pScreen->ClipNotify = xf86XVClipNotify;
  }
}



/**** Required XvScreenRec fields ****/

static Bool
xf86XVCloseScreen(int i, ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  XvScreenPtr pxvs = GET_XV_SCREEN(pScreen);
  XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
  XvAdaptorPtr pa;
  int c;

  /* Clear offscreen images */
  memset(&OffscreenImages[pScreen->myNum], 0, sizeof(OffscreenImages[0]));

  if(!ScreenPriv) return TRUE;

  if(ScreenPriv->videoGC) {
     FreeGC(ScreenPriv->videoGC, 0);
     ScreenPriv->videoGC = NULL;
  }

  pScreen->DestroyWindow = ScreenPriv->DestroyWindow;
  pScreen->WindowExposures = ScreenPriv->WindowExposures;
  pScreen->ClipNotify = ScreenPriv->ClipNotify;

  pScrn->EnterVT = ScreenPriv->EnterVT;
  pScrn->LeaveVT = ScreenPriv->LeaveVT;
  pScrn->AdjustFrame = ScreenPriv->AdjustFrame;

  for(c = 0, pa = pxvs->pAdaptors; c < pxvs->nAdaptors; c++, pa++) {
       xf86XVFreeAdaptor(pa);
  }

  xfree(pxvs->pAdaptors);
  xfree(ScreenPriv);
  return TRUE;
}


static int
xf86XVQueryAdaptors(
   ScreenPtr pScreen,
   XvAdaptorPtr *p_pAdaptors,
   int *p_nAdaptors
){
  XvScreenPtr pxvs = GET_XV_SCREEN(pScreen);

  *p_nAdaptors = pxvs->nAdaptors;
  *p_pAdaptors = pxvs->pAdaptors;

  return (Success);
}


/**** ScrnInfoRec fields ****/

static Bool
xf86XVEnterVT(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
    Bool ret;

    ret = (*ScreenPriv->EnterVT)(index, flags);

    if(ret) WalkTree(pScreen, xf86XVReputAllVideo, 0);

    return ret;
}

static void
xf86XVLeaveVT(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    XvScreenPtr pxvs = GET_XV_SCREEN(pScreen);
    XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
    XvAdaptorPtr pAdaptor;
    XvPortPtr pPort;
    XvPortRecPrivatePtr pPriv;
    int i, j;

    for(i = 0; i < pxvs->nAdaptors; i++) {
	pAdaptor = &pxvs->pAdaptors[i];
	for(j = 0; j < pAdaptor->nPorts; j++) {
	    pPort = &pAdaptor->pPorts[j];
	    pPriv = (XvPortRecPrivatePtr)pPort->devPriv.ptr;
	    if(pPriv->isOn > XV_OFF) {

		(*pPriv->AdaptorRec->StopVideo)(
			pPriv->pScrn, pPriv->DevPriv.ptr, TRUE);
		pPriv->isOn = XV_OFF;

		if(pPriv->pCompositeClip && pPriv->FreeCompositeClip)
		    REGION_DESTROY(pScreen, pPriv->pCompositeClip);

		pPriv->pCompositeClip = NULL;

		if(!pPriv->type && pPriv->pDraw) { /* still */
		    xf86XVRemovePortFromWindow((WindowPtr)pPriv->pDraw, pPriv);
		}
	    }
	}
    }

    (*ScreenPriv->LeaveVT)(index, flags);
}

static void
xf86XVAdjustFrame(int index, int x, int y, int flags)
{
  ScrnInfoPtr pScrn = xf86Screens[index];
  ScreenPtr pScreen = pScrn->pScreen;
  XvScreenPtr pxvs = GET_XV_SCREEN(pScreen);
  XF86XVScreenPtr ScreenPriv = GET_XF86XV_SCREEN(pScreen);
  WindowPtr pWin;
  XvAdaptorPtr pa;
  int c, i;

  if(ScreenPriv->AdjustFrame) {
	pScrn->AdjustFrame = ScreenPriv->AdjustFrame;
	(*pScrn->AdjustFrame)(index, x, y, flags);
	pScrn->AdjustFrame = xf86XVAdjustFrame;
  }

  for(c = pxvs->nAdaptors, pa = pxvs->pAdaptors; c > 0; c--, pa++) {
      XvPortPtr pPort = pa->pPorts;
      XvPortRecPrivatePtr pPriv;

      for(i = pa->nPorts; i > 0; i--, pPort++) {
	pPriv = (XvPortRecPrivatePtr)pPort->devPriv.ptr;

	if(!pPriv->type && (pPriv->isOn == XV_ON)) { /* overlaid still/image */

	  if(pPriv->pCompositeClip && pPriv->FreeCompositeClip)
	     REGION_DESTROY(pScreen, pPriv->pCompositeClip);

	  pPriv->pCompositeClip = NULL;

	  pWin = (WindowPtr)pPriv->pDraw;

	  if ((pPriv->AdaptorRec->ReputImage) &&
	     ((pWin->visibility == VisibilityUnobscured) ||
	      (pWin->visibility == VisibilityPartiallyObscured)))
	  {
	      xf86XVReputImage(pPriv);
	  } else {
	     (*pPriv->AdaptorRec->StopVideo)(
				 pPriv->pScrn, pPriv->DevPriv.ptr, FALSE);
	     xf86XVRemovePortFromWindow(pWin, pPriv);
	     pPriv->isOn = XV_PENDING;
	     continue;
	  }
	}
     }
  }
}


/**** XvAdaptorRec fields ****/

static int
xf86XVAllocatePort(
   unsigned long port,
   XvPortPtr pPort,
   XvPortPtr *ppPort
){
  *ppPort = pPort;
  return Success;
}



static int
xf86XVFreePort(XvPortPtr pPort)
{
  return Success;
}


static int
xf86XVPutVideo(
   ClientPtr client,
   DrawablePtr pDraw,
   XvPortPtr pPort,
   GCPtr pGC,
   INT16 vid_x, INT16 vid_y,
   CARD16 vid_w, CARD16 vid_h,
   INT16 drw_x, INT16 drw_y,
   CARD16 drw_w, CARD16 drw_h
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);
  int result;

  /* No dumping video to pixmaps... For now anyhow */
  if(pDraw->type != DRAWABLE_WINDOW) {
      pPort->pDraw = (DrawablePtr)NULL;
      return BadAlloc;
  }

  /* If we are changing windows, unregister our port in the old window */
  if(portPriv->pDraw && (portPriv->pDraw != pDraw))
     xf86XVRemovePortFromWindow((WindowPtr)(portPriv->pDraw), portPriv);

  /* Register our port with the new window */
  result =  xf86XVEnlistPortInWindow((WindowPtr)pDraw, portPriv);
  if(result != Success) return result;

  portPriv->type = XvInputMask;

  /* save a copy of these parameters */
  portPriv->vid_x = vid_x;  portPriv->vid_y = vid_y;
  portPriv->vid_w = vid_w;  portPriv->vid_h = vid_h;
  portPriv->drw_x = drw_x;  portPriv->drw_y = drw_y;
  portPriv->drw_w = drw_w;  portPriv->drw_h = drw_h;

  /* make sure we have the most recent copy of the clientClip */
  xf86XVCopyClip(portPriv, pGC);

  /* To indicate to the DI layer that we were successful */
  pPort->pDraw = pDraw;

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  return(xf86XVReputVideo(portPriv));
}

static int
xf86XVPutStill(
   ClientPtr client,
   DrawablePtr pDraw,
   XvPortPtr pPort,
   GCPtr pGC,
   INT16 vid_x, INT16 vid_y,
   CARD16 vid_w, CARD16 vid_h,
   INT16 drw_x, INT16 drw_y,
   CARD16 drw_w, CARD16 drw_h
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);
  ScreenPtr pScreen;
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  int ret = Success;
  Bool clippedAway = FALSE;

  if (pDraw->type != DRAWABLE_WINDOW)
      return BadAlloc;

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  pScreen = pDraw->pScreen;

  WinBox.x1 = pDraw->x + drw_x;
  WinBox.y1 = pDraw->y + drw_y;
  WinBox.x2 = WinBox.x1 + drw_w;
  WinBox.y2 = WinBox.y1 + drw_h;

  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, pGC->pCompositeClip);

  if(portPriv->AdaptorRec->flags & VIDEO_CLIP_TO_VIEWPORT) {
     RegionRec VPReg;
     BoxRec VPBox;

     VPBox.x1 = portPriv->pScrn->frameX0;
     VPBox.y1 = portPriv->pScrn->frameY0;
     VPBox.x2 = portPriv->pScrn->frameX1;
     VPBox.y2 = portPriv->pScrn->frameY1;

     REGION_INIT(pScreen, &VPReg, &VPBox, 1);
     REGION_INTERSECT(pScreen, &ClipRegion, &ClipRegion, &VPReg);
     REGION_UNINIT(pScreen, &VPReg);
  }

  if(portPriv->pDraw) {
     xf86XVRemovePortFromWindow((WindowPtr)(portPriv->pDraw), portPriv);
  }

  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
     clippedAway = TRUE;
     goto PUT_STILL_BAILOUT;
  }

  if(portPriv->AdaptorRec->flags & VIDEO_NO_CLIPPING) {
     BoxPtr clipBox = REGION_RECTS(&ClipRegion);
     if(  (REGION_NUM_RECTS(&ClipRegion) != 1) ||
	  (clipBox->x1 != WinBox.x1) || (clipBox->x2 != WinBox.x2) ||
	  (clipBox->y1 != WinBox.y1) || (clipBox->y2 != WinBox.y2))
     {
	  clippedAway = TRUE;
	  goto PUT_STILL_BAILOUT;
     }
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->PutStill)(portPriv->pScrn,
		vid_x, vid_y, WinBox.x1, WinBox.y1,
		vid_w, vid_h, drw_w, drw_h,
		&ClipRegion, portPriv->DevPriv.ptr,
		pDraw);

  if((ret == Success) &&
	(portPriv->AdaptorRec->flags & VIDEO_OVERLAID_STILLS)) {

     xf86XVEnlistPortInWindow((WindowPtr)pDraw, portPriv);
     portPriv->isOn = XV_ON;
     portPriv->drw_x = drw_x;  portPriv->drw_y = drw_y;
     portPriv->drw_w = drw_w;  portPriv->drw_h = drw_h;
     portPriv->type = 0;  /* no mask means it's transient and should
			     not be reput once it's removed */
     pPort->pDraw = pDraw;  /* make sure we can get stop requests */
  }

PUT_STILL_BAILOUT:

  if((clippedAway || (ret != Success)) && (portPriv->isOn == XV_ON)) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}

static int
xf86XVGetVideo(
   ClientPtr client,
   DrawablePtr pDraw,
   XvPortPtr pPort,
   GCPtr pGC,
   INT16 vid_x, INT16 vid_y,
   CARD16 vid_w, CARD16 vid_h,
   INT16 drw_x, INT16 drw_y,
   CARD16 drw_w, CARD16 drw_h
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);
  int result;

  /* No pixmaps... For now anyhow */
  if(pDraw->type != DRAWABLE_WINDOW) {
      pPort->pDraw = (DrawablePtr)NULL;
      return BadAlloc;
  }

  /* If we are changing windows, unregister our port in the old window */
  if(portPriv->pDraw && (portPriv->pDraw != pDraw))
     xf86XVRemovePortFromWindow((WindowPtr)(portPriv->pDraw), portPriv);

  /* Register our port with the new window */
  result =  xf86XVEnlistPortInWindow((WindowPtr)pDraw, portPriv);
  if(result != Success) return result;

  portPriv->type = XvOutputMask;

  /* save a copy of these parameters */
  portPriv->vid_x = vid_x;  portPriv->vid_y = vid_y;
  portPriv->vid_w = vid_w;  portPriv->vid_h = vid_h;
  portPriv->drw_x = drw_x;  portPriv->drw_y = drw_y;
  portPriv->drw_w = drw_w;  portPriv->drw_h = drw_h;

  /* make sure we have the most recent copy of the clientClip */
  xf86XVCopyClip(portPriv, pGC);

  /* To indicate to the DI layer that we were successful */
  pPort->pDraw = pDraw;

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  return(xf86XVRegetVideo(portPriv));
}

static int
xf86XVGetStill(
   ClientPtr client,
   DrawablePtr pDraw,
   XvPortPtr pPort,
   GCPtr pGC,
   INT16 vid_x, INT16 vid_y,
   CARD16 vid_w, CARD16 vid_h,
   INT16 drw_x, INT16 drw_y,
   CARD16 drw_w, CARD16 drw_h
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);
  ScreenPtr pScreen;
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  int ret = Success;
  Bool clippedAway = FALSE;

  if (pDraw->type != DRAWABLE_WINDOW)
      return BadAlloc;

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  pScreen = pDraw->pScreen;

  WinBox.x1 = pDraw->x + drw_x;
  WinBox.y1 = pDraw->y + drw_y;
  WinBox.x2 = WinBox.x1 + drw_w;
  WinBox.y2 = WinBox.y1 + drw_h;

  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, pGC->pCompositeClip);

  if(portPriv->pDraw) {
     xf86XVRemovePortFromWindow((WindowPtr)(portPriv->pDraw), portPriv);
  }

  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
     clippedAway = TRUE;
     goto GET_STILL_BAILOUT;
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->GetStill)(portPriv->pScrn,
		vid_x, vid_y, WinBox.x1, WinBox.y1,
		vid_w, vid_h, drw_w, drw_h,
		&ClipRegion, portPriv->DevPriv.ptr,
		pDraw);

GET_STILL_BAILOUT:

  if((clippedAway || (ret != Success)) && (portPriv->isOn == XV_ON)) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}



static int
xf86XVStopVideo(
   ClientPtr client,
   XvPortPtr pPort,
   DrawablePtr pDraw
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);

  if(pDraw->type != DRAWABLE_WINDOW)
      return BadAlloc;

  xf86XVRemovePortFromWindow((WindowPtr)pDraw, portPriv);

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  /* Must free resources. */

  if(portPriv->isOn > XV_OFF) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, TRUE);
	portPriv->isOn = XV_OFF;
  }

  return Success;
}

static int
xf86XVSetPortAttribute(
   ClientPtr client,
   XvPortPtr pPort,
   Atom attribute,
   INT32 value
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);

  return((*portPriv->AdaptorRec->SetPortAttribute)(portPriv->pScrn,
		attribute, value, portPriv->DevPriv.ptr));
}


static int
xf86XVGetPortAttribute(
   ClientPtr client,
   XvPortPtr pPort,
   Atom attribute,
   INT32 *p_value
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);

  return((*portPriv->AdaptorRec->GetPortAttribute)(portPriv->pScrn,
		attribute, p_value, portPriv->DevPriv.ptr));
}



static int
xf86XVQueryBestSize(
   ClientPtr client,
   XvPortPtr pPort,
   CARD8 motion,
   CARD16 vid_w, CARD16 vid_h,
   CARD16 drw_w, CARD16 drw_h,
   unsigned int *p_w, unsigned int *p_h
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);

  (*portPriv->AdaptorRec->QueryBestSize)(portPriv->pScrn,
		(Bool)motion, vid_w, vid_h, drw_w, drw_h,
		p_w, p_h, portPriv->DevPriv.ptr);

  return Success;
}


static int
xf86XVPutImage(
   ClientPtr client,
   DrawablePtr pDraw,
   XvPortPtr pPort,
   GCPtr pGC,
   INT16 src_x, INT16 src_y,
   CARD16 src_w, CARD16 src_h,
   INT16 drw_x, INT16 drw_y,
   CARD16 drw_w, CARD16 drw_h,
   XvImagePtr format,
   unsigned char* data,
   Bool sync,
   CARD16 width, CARD16 height
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);
  ScreenPtr pScreen;
  RegionRec WinRegion;
  RegionRec ClipRegion;
  BoxRec WinBox;
  int ret = Success;
  Bool clippedAway = FALSE;

  if (pDraw->type != DRAWABLE_WINDOW)
      return BadAlloc;

  if(!portPriv->pScrn->vtSema) return Success; /* Success ? */

  pScreen = pDraw->pScreen;

  WinBox.x1 = pDraw->x + drw_x;
  WinBox.y1 = pDraw->y + drw_y;
  WinBox.x2 = WinBox.x1 + drw_w;
  WinBox.y2 = WinBox.y1 + drw_h;

  REGION_INIT(pScreen, &WinRegion, &WinBox, 1);
  REGION_NULL(pScreen, &ClipRegion);
  REGION_INTERSECT(pScreen, &ClipRegion, &WinRegion, pGC->pCompositeClip);

  if(portPriv->AdaptorRec->flags & VIDEO_CLIP_TO_VIEWPORT) {
     RegionRec VPReg;
     BoxRec VPBox;

     VPBox.x1 = portPriv->pScrn->frameX0;
     VPBox.y1 = portPriv->pScrn->frameY0;
     VPBox.x2 = portPriv->pScrn->frameX1 + 1;
     VPBox.y2 = portPriv->pScrn->frameY1 + 1;

     REGION_INIT(pScreen, &VPReg, &VPBox, 1);
     REGION_INTERSECT(pScreen, &ClipRegion, &ClipRegion, &VPReg);
     REGION_UNINIT(pScreen, &VPReg);
  }

  /* If we are changing windows, unregister our port in the old window */
  if(portPriv->pDraw && (portPriv->pDraw != pDraw))
     xf86XVRemovePortFromWindow((WindowPtr)(portPriv->pDraw), portPriv);

  /* Register our port with the new window */
  ret =  xf86XVEnlistPortInWindow((WindowPtr)pDraw, portPriv);
  if(ret != Success) goto PUT_IMAGE_BAILOUT;

  if(!REGION_NOTEMPTY(pScreen, &ClipRegion)) {
     clippedAway = TRUE;
     goto PUT_IMAGE_BAILOUT;
  }

  if(portPriv->AdaptorRec->flags & VIDEO_NO_CLIPPING) {
     BoxPtr clipBox = REGION_RECTS(&ClipRegion);
     if(  (REGION_NUM_RECTS(&ClipRegion) != 1) ||
	  (clipBox->x1 != WinBox.x1) || (clipBox->x2 != WinBox.x2) ||
	  (clipBox->y1 != WinBox.y1) || (clipBox->y2 != WinBox.y2))
     {
	  clippedAway = TRUE;
	  goto PUT_IMAGE_BAILOUT;
     }
  }

  if(portPriv->AdaptorRec->flags & VIDEO_INVERT_CLIPLIST) {
     REGION_SUBTRACT(pScreen, &ClipRegion, &WinRegion, &ClipRegion);
  }

  ret = (*portPriv->AdaptorRec->PutImage)(portPriv->pScrn,
		src_x, src_y, WinBox.x1, WinBox.y1,
		src_w, src_h, drw_w, drw_h, format->id, data, width, height,
		sync, &ClipRegion, portPriv->DevPriv.ptr,
		pDraw);

  if((ret == Success) &&
	(portPriv->AdaptorRec->flags & VIDEO_OVERLAID_IMAGES)) {

     portPriv->isOn = XV_ON;
     portPriv->drw_x = drw_x;  portPriv->drw_y = drw_y;
     portPriv->drw_w = drw_w;  portPriv->drw_h = drw_h;
     portPriv->type = 0;  /* no mask means it's transient and should
			     not be reput once it's removed */
     pPort->pDraw = pDraw;  /* make sure we can get stop requests */
  }

PUT_IMAGE_BAILOUT:

  if((clippedAway || (ret != Success)) && (portPriv->isOn == XV_ON)) {
	(*portPriv->AdaptorRec->StopVideo)(
		portPriv->pScrn, portPriv->DevPriv.ptr, FALSE);
	portPriv->isOn = XV_PENDING;
  }

  REGION_UNINIT(pScreen, &WinRegion);
  REGION_UNINIT(pScreen, &ClipRegion);

  return ret;
}


static  int
xf86XVQueryImageAttributes(
   ClientPtr client,
   XvPortPtr pPort,
   XvImagePtr format,
   CARD16 *width,
   CARD16 *height,
   int *pitches,
   int *offsets
){
  XvPortRecPrivatePtr portPriv = (XvPortRecPrivatePtr)(pPort->devPriv.ptr);

  return (*portPriv->AdaptorRec->QueryImageAttributes)(portPriv->pScrn,
			format->id, width, height, pitches, offsets);
}


_X_EXPORT void
xf86XVFillKeyHelperDrawable (DrawablePtr pDraw, CARD32 key, RegionPtr clipboxes)
{
   ScreenPtr pScreen = pDraw->pScreen;
   WindowPtr pWin = (WindowPtr)pDraw;
   XF86XVWindowPtr pPriv = GET_XF86XV_WINDOW(pWin);
   GCPtr pGC = NULL;
   XID pval[2];
   BoxPtr pbox = REGION_RECTS(clipboxes);
   int i, nbox = REGION_NUM_RECTS(clipboxes);
   xRectangle *rects;

   if(!xf86Screens[pScreen->myNum]->vtSema) return;

   if(pPriv)
      pGC = pPriv->pGC;

   if(!pGC) {
       int status;
       pval[0] = key;
       pval[1] = IncludeInferiors;
       pGC = CreateGC(pDraw, GCForeground | GCSubwindowMode, pval, &status,
		      (XID)0, serverClient);
       if(!pGC) return;
       ValidateGC(pDraw, pGC);
       if (pPriv) pPriv->pGC = pGC;
   } else if (key != pGC->fgPixel){
       pval[0] = key;
       ChangeGC(pGC, GCForeground, pval);
       ValidateGC(pDraw, pGC);
   }

   REGION_TRANSLATE(pDraw->pScreen, clipboxes, -pDraw->x, -pDraw->y);

   rects = xalloc(nbox * sizeof(xRectangle));

   for(i = 0; i < nbox; i++, pbox++) {
      rects[i].x = pbox->x1;
      rects[i].y = pbox->y1;
      rects[i].width = pbox->x2 - pbox->x1;
      rects[i].height = pbox->y2 - pbox->y1;
   }

   (*pGC->ops->PolyFillRect)(pDraw, pGC, nbox, rects);

   if (!pPriv) FreeGC(pGC, 0);

   xfree(rects);
}

_X_EXPORT void
xf86XVFillKeyHelper (ScreenPtr pScreen, CARD32 key, RegionPtr clipboxes)
{
   DrawablePtr root = &WindowTable[pScreen->myNum]->drawable;
   XID pval[2];
   BoxPtr pbox = REGION_RECTS(clipboxes);
   int i, nbox = REGION_NUM_RECTS(clipboxes);
   xRectangle *rects;
   GCPtr gc;

   if(!xf86Screens[pScreen->myNum]->vtSema) return;

   gc = GetScratchGC(root->depth, pScreen);
   pval[0] = key;
   pval[1] = IncludeInferiors;
   (void) ChangeGC(gc, GCForeground|GCSubwindowMode, pval);
   ValidateGC(root, gc);

   rects = xalloc (nbox * sizeof(xRectangle));

   for(i = 0; i < nbox; i++, pbox++) 
   {
      rects[i].x = pbox->x1;
      rects[i].y = pbox->y1;
      rects[i].width = pbox->x2 - pbox->x1;
      rects[i].height = pbox->y2 - pbox->y1;
   }
   
   (*gc->ops->PolyFillRect)(root, gc, nbox, rects);
   
   xfree (rects);
   FreeScratchGC (gc);
}

/* xf86XVClipVideoHelper -

   Takes the dst box in standard X BoxRec form (top and left
   edges inclusive, bottom and right exclusive).  The new dst
   box is returned.  The source boundaries are given (x1, y1
   inclusive, x2, y2 exclusive) and returned are the new source
   boundaries in 16.16 fixed point.
*/

#define DummyScreen screenInfo.screens[0]

_X_EXPORT Bool
xf86XVClipVideoHelper(
    BoxPtr dst,
    INT32 *xa,
    INT32 *xb,
    INT32 *ya,
    INT32 *yb,
    RegionPtr reg,
    INT32 width,
    INT32 height
){
    double xsw, xdw, ysw, ydw;
    INT32 delta;
    BoxPtr extents = REGION_EXTENTS(DummyScreen, reg);
    int diff;

    xsw = (*xb - *xa) << 16;
    xdw = dst->x2 - dst->x1;
    ysw = (*yb - *ya) << 16;
    ydw = dst->y2 - dst->y1;

    *xa <<= 16; *xb <<= 16;
    *ya <<= 16; *yb <<= 16;

    diff = extents->x1 - dst->x1;
    if (diff > 0) {
	dst->x1 = extents->x1;
	*xa += (diff * xsw) / xdw;
    }
    diff = dst->x2 - extents->x2;
    if (diff > 0) {
	dst->x2 = extents->x2;
	*xb -= (diff * xsw) / xdw;
    }
    diff = extents->y1 - dst->y1;
    if (diff > 0) {
	dst->y1 = extents->y1;
	*ya += (diff * ysw) / ydw;
    }
    diff = dst->y2 - extents->y2;
    if (diff > 0) {
	dst->y2 = extents->y2;
	*yb -= (diff * ysw) / ydw;
    }

    if (*xa < 0) {
	diff = (((-*xa) * xdw) + xsw - 1) / xsw;
	dst->x1 += diff;
	*xa += (diff * xsw) / xdw;
    }
    delta = *xb - (width << 16);
    if (delta > 0) {
	diff = ((delta * xdw) + xsw - 1) / xsw;
	dst->x2 -= diff;
	*xb -= (diff * xsw) / xdw;
    }
    if (*xa >= *xb) return FALSE;

    if (*ya < 0) {
	diff = (((-*ya) * ydw) + ysw - 1) / ysw;
	dst->y1 += diff;
	*ya += (diff * ysw) / ydw;
    }
    delta = *yb - (height << 16);
    if (delta > 0) {
	diff = ((delta * ydw) + ysw - 1) / ysw;
	dst->y2 -= diff;
	*yb -= (diff * ysw) / ydw;
    }
    if (*ya >= *yb) return FALSE;

    if ((dst->x1 > extents->x1) || (dst->x2 < extents->x2) ||
	(dst->y1 > extents->y1) || (dst->y2 < extents->y2))
    {
	RegionRec clipReg;
	REGION_INIT(DummyScreen, &clipReg, dst, 1);
	REGION_INTERSECT(DummyScreen, reg, reg, &clipReg);
	REGION_UNINIT(DummyScreen, &clipReg);
    }
    return TRUE;
}

_X_EXPORT void
xf86XVCopyYUV12ToPacked(
    const void *srcy,
    const void *srcv,
    const void *srcu,
    void *dst,
    int srcPitchy,
    int srcPitchuv,
    int dstPitch,
    int h,
    int w
){
    CARD32 *Dst;
    const CARD8 *Y, *U, *V;
    int i, j;

    w >>= 1;

    for (j = 0;  j < h;  j++) {
	Dst = dst;
	Y = srcy;  V = srcv;  U = srcu;
	i = w;
	while (i >= 4) {
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
	    Dst[0] = Y[0] | (Y[1] << 16) | (U[0] << 8) | (V[0] << 24);
	    Dst[1] = Y[2] | (Y[3] << 16) | (U[1] << 8) | (V[1] << 24);
	    Dst[2] = Y[4] | (Y[5] << 16) | (U[2] << 8) | (V[2] << 24);
	    Dst[3] = Y[6] | (Y[7] << 16) | (U[3] << 8) | (V[3] << 24);
#else
	    /* This assumes a little-endian framebuffer */
	    Dst[0] = (Y[0] << 24) | (Y[1] << 8) | (U[0] << 16) | V[0];
	    Dst[1] = (Y[2] << 24) | (Y[3] << 8) | (U[1] << 16) | V[1];
	    Dst[2] = (Y[4] << 24) | (Y[5] << 8) | (U[2] << 16) | V[2];
	    Dst[3] = (Y[6] << 24) | (Y[7] << 8) | (U[3] << 16) | V[3];
#endif
	    Dst += 4;  Y += 8;  V += 4;  U += 4;
	    i -= 4;
	}

	while (i--) {
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
	    Dst[0] = Y[0] | (Y[1] << 16) | (U[0] << 8) | (V[0] << 24);
#else
	    /* This assumes a little-endian framebuffer */
	    Dst[0] = (Y[0] << 24) | (Y[1] << 8) | (U[0] << 16) | V[0];
#endif
	    Dst++;  Y += 2;  V++;  U++;
	}

	dst = (CARD8 *)dst + dstPitch;
	srcy = (const CARD8 *)srcy + srcPitchy;
	if (j & 1) {
	    srcu = (const CARD8 *)srcu + srcPitchuv;
	    srcv = (const CARD8 *)srcv + srcPitchuv;
	}
    }
}

_X_EXPORT void
xf86XVCopyPacked(
    const void *src,
    void *dst,
    int srcPitch,
    int dstPitch,
    int h,
    int w
){
    const CARD32 *Src;
    CARD32 *Dst;
    int i;

    w >>= 1;
    while (--h >= 0) {
	do {
	    Dst = dst;  Src = src;
	    i = w;
	    while (i >= 4) {
		Dst[0] = Src[0];
		Dst[1] = Src[1];
		Dst[2] = Src[2];
		Dst[3] = Src[3];
		Dst += 4;  Src += 4;  i -= 4;
	    }
	    if (!i) break;
	    Dst[0] = Src[0];
	    if (i == 1) break;
	    Dst[1] = Src[1];
	    if (i == 2) break;
	    Dst[2] = Src[2];
	} while (0);

	src = (const CARD8 *)src + srcPitch;
	dst = (CARD8 *)dst + dstPitch;
    }
}
