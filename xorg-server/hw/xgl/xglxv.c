/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: David Reveman <davidr@novell.com>
 *          Matthias Hopf <mhopf@suse.de>
 */

#include "xgl.h"

#ifdef XV

#include "xvdix.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>

static DevPrivateKey xglXvScreenKey;
static unsigned long portResource = 0;

#define XGL_GET_XV_SCREEN(pScreen) ((XvScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, xglXvScreenKey))

#define XGL_XV_SCREEN(pScreen)				\
    XvScreenPtr pXvScreen = XGL_GET_XV_SCREEN (pScreen)

#define XGL_GET_XV_SCREEN_PRIV(pScreen)			      \
    ((xglXvScreenPtr) (GET_XV_SCREEN (pScreen)->devPriv.ptr))

#define XGL_XV_SCREEN_PRIV(pScreen)				    \
    xglXvScreenPtr pXvScreenPriv = XGL_GET_XV_SCREEN_PRIV (pScreen)

#define XGL_GET_XV_PORT_PRIV(pPort)	    \
    ((xglXvPortPtr) ((pPort)->devPriv.ptr))

#define XGL_XV_PORT_PRIV(pPort)				  \
    xglXvPortPtr pPortPriv = XGL_GET_XV_PORT_PRIV (pPort)

#define XGL_XV_NUM_PORTS 32

#define XGL_XV_IMAGE_MAX_WIDTH  2048
#define XGL_XV_IMAGE_MAX_HEIGHT 2048

static XvImageRec xvImages[] = {
    {
	GLITZ_FOURCC_YUY2, XvYUV, BITMAP_BIT_ORDER,
	{
	    'Y','U','Y','2',
	    0x00, 0x00, 0x00, 0x10, 0x80, 0x00,
	    0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
	},
	16, XvPacked, 1,
	0, 0, 0, 0,
	8, 8, 8,  1, 2, 2,  1, 1, 1,
	{
	    'Y', 'U', 'Y', 'V',
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	XvTopToBottom
    }, {
	GLITZ_FOURCC_YV12, XvYUV, BITMAP_BIT_ORDER,
	{
	    'Y', 'V', '1', '2',
	    0x00, 0x00, 0x00, 0x10, 0x80, 0x00,
	    0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
	},
	12, XvPlanar, 3,
	0, 0, 0, 0,
	8, 8, 8,  1, 2, 2,  1, 2, 2,
	{
	    'Y', 'V', 'U', 0,
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	XvTopToBottom
    }, {
	GLITZ_FOURCC_RGB, XvRGB, BITMAP_BIT_ORDER,
	{
	    0x03, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x10, 0x80, 0x00,
	    0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
	},
	32, XvPacked, 1,
	24, 0xff0000, 0xff00, 0xff,
	0, 0, 0,  0, 0, 0,  0, 0, 0,
	{
	    0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	XvTopToBottom
    }
};

static struct _xglXvFormat {
    CARD32	      format;
    glitz_fourcc_t    fourcc;
    xglPixelFormatRec pixel;
} xglXvFormat[XGL_XV_FORMAT_NUM] = {
    {
	PICT_yuy2,
	GLITZ_FOURCC_YUY2,
	{
	    16, 6,
	    {
		16,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
	    }
	}
    }, {
	PICT_yv12,
	GLITZ_FOURCC_YV12,
	{
	    12, 4,
	    {
		12,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
	    }
	}
    }, {
	PICT_x8r8g8b8,
	GLITZ_FOURCC_RGB,
	{
	    24, 8,
	    {
		32,
		0x00000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
	    }
	}
    }
};

static int
xglXvQueryAdaptors (ScreenPtr	 pScreen,
		    XvAdaptorPtr *pAdaptors,
		    int		 *nAdaptors)
{
    XGL_XV_SCREEN (pScreen);

    *nAdaptors = pXvScreen->nAdaptors;
    *pAdaptors = pXvScreen->pAdaptors;

    return Success;
}

static int
xglXvAllocatePort (unsigned long port,
		   XvPortPtr	 pPort,
		   XvPortPtr	 *ppPort)
{
    *ppPort = pPort;

    return Success;
}

static int
xglXvFreePort (XvPortPtr pPort)
{
    XGL_XV_PORT_PRIV (pPort);

    if (pPortPriv->pDst)
    {
	FreePicture ((pointer) pPortPriv->pDst, 0);
	pPortPriv->pDst = (PicturePtr) 0;
    }

    if (pPortPriv->pSrc)
    {
	FreePicture ((pointer) pPortPriv->pSrc, 0);
	pPortPriv->pSrc = (PicturePtr) 0;
    }

    if (pPortPriv->pPixmap)
    {
	ScreenPtr pScreen;

	pScreen = pPortPriv->pPixmap->drawable.pScreen;
	(*pScreen->DestroyPixmap) (pPortPriv->pPixmap);
	pPortPriv->pPixmap = (PixmapPtr) 0;
    }

    return Success;
}

static int
xglXvQueryBestSize (ClientPtr	 client,
		    XvPortPtr	 pPort,
		    CARD8	 motion,
		    CARD16	 srcWidth,
		    CARD16	 srcHeight,
		    CARD16	 dstWidth,
		    CARD16	 dstHeight,
		    unsigned int *pWidth,
		    unsigned int *pHeight)
{
    *pWidth  = dstWidth;
    *pHeight = dstHeight;

    return Success;
}

static int
xglXvStopVideo (ClientPtr   client,
		XvPortPtr   pPort,
		DrawablePtr pDrawable)
{
    xglXvFreePort (pPort);

    return Success;
}

static int
xglXvPutImage (ClientPtr     client,
	       DrawablePtr   pDrawable,
	       XvPortPtr     pPort,
	       GCPtr	     pGC,
	       INT16	     srcX,
	       INT16	     srcY,
	       CARD16	     srcWidth,
	       CARD16	     srcHeight,
	       INT16	     dstX,
	       INT16	     dstY,
	       CARD16	     dstWidth,
	       CARD16	     dstHeight,
	       XvImagePtr    pImage,
	       unsigned char *data,
	       Bool	     sync,
	       CARD16	     width,
	       CARD16	     height)
{
    ScreenPtr	  pScreen = pDrawable->pScreen;
    PictTransform transform;
    int		  depth, bpp;
    CARD32	  format;

    XGL_SCREEN_PRIV (pScreen);
    XGL_XV_PORT_PRIV (pPort);
    XGL_DRAWABLE_PIXMAP (pDrawable);
    XGL_PIXMAP_PRIV (pPixmap);

    switch (pImage->id) {
    case GLITZ_FOURCC_YUY2:
	bpp = depth = 16;
	format = PICT_yuy2;
	break;
    case GLITZ_FOURCC_YV12:
	depth = bpp = 12;
	format = PICT_yv12;
	break;
    case GLITZ_FOURCC_RGB:
	depth = 24;
	bpp = 32;
	format = PICT_x8r8g8b8;
	break;
    default:
	return BadImplementation;
    }

    pPort->pDraw = pDrawable;

    if (!pPortPriv->pPixmap)
    {
	pPortPriv->pPixmap = (*pScreen->CreatePixmap) (pScreen, 0, 0, depth, 0);
	if (!pPortPriv->pPixmap)
	    return BadAlloc;
    }

    (*pScreen->ModifyPixmapHeader) (pPortPriv->pPixmap,
				    srcWidth, srcHeight,
				    depth, bpp, -1, (pointer) data);

    XGL_GET_PIXMAP_PRIV (pPortPriv->pPixmap)->stride = -srcWidth;

    pPortPriv->pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    if (!pPortPriv->pSrc || pPortPriv->pSrc->format != format)
    {
	PictFormatPtr pFormat;
	int	      error;
	static XID    value = RepeatPad;

	pFormat = PictureMatchFormat (pScreen, depth, format);
	if (!pFormat)
	    return BadImplementation;

	if (pPortPriv->pSrc)
	    FreePicture ((pointer) pPortPriv->pSrc, 0);

	pPortPriv->pSrc = CreatePicture (0, &pPortPriv->pPixmap->drawable,
					 pFormat, CPRepeat, &value,
					 serverClient, &error);
	if (!pPortPriv->pSrc)
	{
	    xglXvFreePort (pPort);
	    return error;
	}

	SetPictureFilter (pPortPriv->pSrc,
			  FilterBilinear, strlen (FilterBilinear),
			  0, 0);
    }

    if (!pPortPriv->pDst || pPortPriv->pDst->pDrawable != pDrawable)
    {
	PictFormatPtr pFormat = 0;
	int	      i, error;

	for (i = 0; i < pScreen->numVisuals; i++)
	{
	    if (pScreen->visuals[i].nplanes == pDrawable->depth)
	    {
		pFormat = PictureMatchVisual (pScreen, pDrawable->depth,
					      &pScreen->visuals[i]);
		break;
	    }
	}

	if (!pFormat)
	    return BadImplementation;

	if (pPortPriv->pDst)
	    FreePicture ((pointer) pPortPriv->pDst, 0);

	pPortPriv->pDst = CreatePicture (0, pDrawable,
					 pFormat, 0, 0, serverClient,
					 &error);
	if (!pPortPriv->pDst)
	{
	    xglXvFreePort (pPort);
	    return error;
	}
    }

    transform.matrix[0][0] = ((srcWidth << 16) + (dstWidth >> 1))
			     / dstWidth;
    transform.matrix[0][1] = 0;
    transform.matrix[0][2] = 0;

    /* flip Y */
    transform.matrix[1][0] = 0;
    transform.matrix[1][1] = -((srcHeight << 16) + (dstHeight >> 1))
			     / dstHeight;
    transform.matrix[1][2] = (srcHeight << 16);

    transform.matrix[2][0] = 0;
    transform.matrix[2][1] = 0;
    transform.matrix[2][2] = 1 << 16;

    SetPictureTransform (pPortPriv->pSrc, &transform);

    if (pPixmap != pScreenPriv->pScreenPixmap && !pPixmapPriv->target)
	xglEnablePixmapAccel (pPixmap, &pScreenPriv->accel.xv);

    CompositePicture (PictOpSrc,
		      pPortPriv->pSrc,
		      (PicturePtr) 0,
		      pPortPriv->pDst,
		      srcX, srcY,
		      0, 0,
		      dstX, dstY,
		      dstWidth, dstHeight);

    return Success;
}

static int
xglXvQueryImageAttributes (ClientPtr  client,
			   XvPortPtr  pPort,
			   XvImagePtr pImage,
			   CARD16     *width,
			   CARD16     *height,
			   int	      *pitches,
			   int	      *offsets)
{
    if (*width > XGL_XV_IMAGE_MAX_WIDTH)
	*width = XGL_XV_IMAGE_MAX_WIDTH;

    if (*height > XGL_XV_IMAGE_MAX_HEIGHT)
	*height = XGL_XV_IMAGE_MAX_HEIGHT;

    *width = (*width + 7) & ~7;

    switch (pImage->id) {
    case GLITZ_FOURCC_YUY2:
	if (offsets)
	    offsets[0] = 0;

	if (pitches)
	    pitches[0] = *width * 2;

	return *width * *height * 2;
    case GLITZ_FOURCC_YV12:
	*height = (*height + 1) & ~1;

	if (offsets)
	{
	    offsets[0] = 0;
	    offsets[1] = *width * *height;
	    offsets[2] = *width * *height + (*width >> 1) * (*height >> 1);
	}

	if (pitches)
	{
	    pitches[0] = *width;
	    pitches[1] = pitches[2] = *width >> 1;
	}

	return *width * *height + (*width >> 1) * *height;
    case GLITZ_FOURCC_RGB:
	if (offsets)
	    offsets[0] = 0;

	if (pitches)
	    pitches[0] = *width * 4;

	return *width * *height * 4;
    default:
	return 0;
    }
}

static void
xglXvFreeAdaptor (XvAdaptorPtr pAdaptor)
{
    xfree (pAdaptor->pEncodings);
    xfree (pAdaptor->pFormats);

    if (pAdaptor->pPorts)
	xfree (pAdaptor->pPorts);
}

static Bool
xglXvInitAdaptors (ScreenPtr pScreen)
{
    XvAdaptorPtr  pAdaptor;
    xglXvPortPtr  pPortPriv;
    XvPortPtr     pPort;
    XvFormatPtr   pFormat;
    XvEncodingPtr pEncoding;
    int		  i;

    XGL_XV_SCREEN (pScreen);

    pXvScreen->nAdaptors = 0;
    pXvScreen->pAdaptors = NULL;

    pAdaptor = xcalloc (1, sizeof (XvAdaptorRec));
    if (!pAdaptor)
	return FALSE;

    pAdaptor->type    = XvInputMask | XvImageMask;
    pAdaptor->pScreen = pScreen;

    pAdaptor->ddAllocatePort	     = xglXvAllocatePort;
    pAdaptor->ddFreePort	     = xglXvFreePort;
    pAdaptor->ddStopVideo	     = xglXvStopVideo;
    pAdaptor->ddPutImage	     = xglXvPutImage;
    pAdaptor->ddQueryBestSize	     = xglXvQueryBestSize;
    pAdaptor->ddQueryImageAttributes = xglXvQueryImageAttributes;

    pAdaptor->name = "Xgl Generic Texture Video";

    pEncoding = xcalloc (1, sizeof (XvEncodingRec));
    if (!pEncoding)
	return FALSE;

    pEncoding->id      = 0;
    pEncoding->pScreen = pScreen;
    pEncoding->name    = "XV_IMAGE";

    pEncoding->width  = XGL_XV_IMAGE_MAX_WIDTH;
    pEncoding->height = XGL_XV_IMAGE_MAX_HEIGHT;

    pEncoding->rate.numerator	= 1;
    pEncoding->rate.denominator = 1;

    pAdaptor->nEncodings = 1;
    pAdaptor->pEncodings = pEncoding;

    pAdaptor->nImages = sizeof (xvImages) / sizeof (XvImageRec);
    pAdaptor->pImages = xvImages;

    /* TODO: Currently no attributes */
    pAdaptor->nAttributes = 0;
    pAdaptor->pAttributes = 0;

    pFormat = xcalloc (pScreen->numVisuals, sizeof (XvFormatRec));
    if (!pFormat)
	return FALSE;

    for (i = 0; i < pScreen->numVisuals; i++)
    {
	pFormat[i].depth  = pScreen->visuals[i].nplanes;
	pFormat[i].visual = pScreen->visuals[i].vid;
    }

    /* All visuals allowed */
    pAdaptor->nFormats = pScreen->numVisuals;
    pAdaptor->pFormats = pFormat;

    pPort = xcalloc (XGL_XV_NUM_PORTS,
		     sizeof (XvPortRec) + sizeof (xglXvPortRec));
    pPortPriv = (xglXvPortPtr) (pPort + XGL_XV_NUM_PORTS);
    if (!pPort)
	return FALSE;

    for (i = 0; i < XGL_XV_NUM_PORTS; i++)
    {
	pPort[i].id = FakeClientID (0);
	if (!pPort[i].id)
	    return FALSE;

	if (!AddResource (pPort[i].id, portResource, &pPort[i]))
	    return FALSE;

	pPort[i].pAdaptor    = pAdaptor;
	pPort[i].pNotify     = (XvPortNotifyPtr) 0;
	pPort[i].pDraw	     = (DrawablePtr) 0;
	pPort[i].client      = (ClientPtr) 0;
	pPort[i].grab.client = (ClientPtr) 0;
	pPort[i].time	     = currentTime;
	pPort[i].devPriv.ptr = pPortPriv + i;
    }

    pAdaptor->nPorts  = XGL_XV_NUM_PORTS;
    pAdaptor->pPorts  = pPort;
    pAdaptor->base_id = pPort->id;

    pXvScreen->pAdaptors = pAdaptor;
    pXvScreen->nAdaptors = 1;

    return TRUE;
}

static Bool
xglXvCloseScreen (int i, ScreenPtr pScreen)
{
    int	j;

    XGL_XV_SCREEN (pScreen);

    for (j = 0; j < pXvScreen->nAdaptors; j++)
	xglXvFreeAdaptor (&pXvScreen->pAdaptors[j]);

    if (pXvScreen->pAdaptors)
	xfree (pXvScreen->pAdaptors);

    return TRUE;
}

Bool
xglXvScreenInit (ScreenPtr pScreen)
{
    XvScreenPtr  pXvScreen;
    xglVisualPtr v;
    int		 i, status, vid = 0;

    XGL_SCREEN_PRIV (pScreen);

    status = XvScreenInit (pScreen);
    if (status != Success)
	return FALSE;

    xglXvScreenKey = XvGetScreenKey ();
    portResource = XvGetRTPort ();

    pXvScreen = XGL_GET_XV_SCREEN (pScreen);

    /* Anyone initializing the Xv layer must provide these two.
       The Xv di layer calls them without even checking if they exist! */
    pXvScreen->ddCloseScreen   = xglXvCloseScreen;
    pXvScreen->ddQueryAdaptors = xglXvQueryAdaptors;

    pXvScreen->devPriv.ptr = (pointer) 0;

    if (!xglXvInitAdaptors (pScreen))
	return FALSE;

    for (v = pScreenPriv->pVisual; v; v = v->next)
    {
	if (v->vid > vid)
	    vid = v->vid;
    }

    memset (pScreenPriv->pXvVisual, 0, sizeof (pScreenPriv->pXvVisual));

    for (i = 0; i < XGL_XV_FORMAT_NUM; i++)
    {
	glitz_format_t templ;

	templ.color.fourcc = xglXvFormat[i].fourcc;

	pScreenPriv->pXvVisual[i].vid = ++vid;
	pScreenPriv->pXvVisual[i].pPixel = &xglXvFormat[i].pixel;
	pScreenPriv->pXvVisual[i].format.surface =
	    glitz_find_format (pScreenPriv->drawable,
			       GLITZ_FORMAT_FOURCC_MASK,
			       &templ, 0);
    }

    return TRUE;
}

#endif
