/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef _XGL_H_
#define _XGL_H_

#include <xgl-config.h>

#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include <glitz.h>

#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "mi.h"
#include "dix.h"
#include "damage.h"
#include "gc.h"
#include "micmap.h"
/* I'd like gc.h to provide this */
typedef struct _GCFuncs *GCFuncsPtr;

#ifdef RENDER
#include "mipict.h"
#else
#ifdef XV
#undef XV /* Xv implementation require RENDER */
#endif
#endif

#ifdef XV
#define XGL_XV_FORMAT_YUY2 0
#define XGL_XV_FORMAT_YV12 1
#define XGL_XV_FORMAT_RGB  2
#define XGL_XV_FORMAT_NUM  3
#endif

/* For the modules.  We should decide what the actual version numbering should
 * be.
 */
#define VERSION "0.0.1"

extern WindowPtr *WindowTable;

#define XGL_DEFAULT_PBO_MASK 0

typedef struct _xglSizeConstraint {
    int minWidth;
    int minHeight;
    int aboveWidth;
    int aboveHeight;
} xglSizeConstraintRec, *xglSizeConstraintPtr;

typedef struct _xglAccelInfo {
    Bool		 enabled;
    Bool		 pbuffer;
    xglSizeConstraintRec size;
} xglAccelInfoRec, *xglAccelInfoPtr;

typedef struct _xglScreenAccelInfo {
    xglAccelInfoRec pixmap;
    xglAccelInfoRec window;
    xglAccelInfoRec glx;
    xglAccelInfoRec xv;
} xglScreenAccelInfoRec, *xglScreenAccelInfoPtr;

typedef struct _xglScreenInfo {
    glitz_drawable_t	  *drawable;
    unsigned int	  depth;
    unsigned int	  width;
    unsigned int	  height;
    unsigned int	  widthMm;
    unsigned int	  heightMm;
    int			  geometryDataType;
    int			  geometryUsage;
    Bool		  yInverted;
    int			  pboMask;
    Bool		  lines;
    xglScreenAccelInfoRec accel;
} xglScreenInfoRec, *xglScreenInfoPtr;

extern xglScreenInfoRec xglScreenInfo;

typedef struct _xglPixelFormat {
    CARD8		depth, bitsPerRGB;
    glitz_pixel_masks_t masks;
} xglPixelFormatRec, *xglPixelFormatPtr;

typedef struct _xglVisual {
    struct _xglVisual *next;
    VisualID	      vid;
    xglPixelFormatPtr pPixel;
    Bool	      pbuffer;
    struct {
	glitz_drawable_format_t *drawable;
	glitz_format_t	        *surface;
    } format;
} xglVisualRec, *xglVisualPtr;

extern xglVisualPtr xglVisuals;

#define xglAreaAvailable 0
#define xglAreaDivided   1
#define xglAreaOccupied  2

typedef struct _xglRootArea *xglRootAreaPtr;

typedef struct _xglArea {
    int		    state;
    int		    level;
    int		    x, y;
    int		    width, height;
    struct _xglArea *pArea[4];
    xglRootAreaPtr  pRoot;
    pointer	    closure;
    DevUnion	    devPrivate;
} xglAreaRec, *xglAreaPtr;

typedef struct _xglAreaFuncs {
    Bool (*Create)	(xglAreaPtr pArea);

    Bool (*MoveIn)      (xglAreaPtr pArea,
			 pointer    closure);

    void (*MoveOut)     (xglAreaPtr pArea,
			 pointer    closure);

    int (*CompareScore) (xglAreaPtr pArea,
			 pointer    closure1,
			 pointer    closure2);

} xglAreaFuncsRec, *xglAreaFuncsPtr;

typedef struct _xglRootArea {
    int		    maxLevel;
    int		    width, height;
    xglAreaPtr	    pArea;
    xglAreaFuncsPtr funcs;
    int		    devPrivateSize;
    pointer	    closure;
} xglRootAreaRec;

typedef struct xglGeometry {
    glitz_buffer_t          *buffer;
    pointer	            *data;
    Bool		    broken;
    glitz_fixed16_16_t	    xOff, yOff;
    int			    dataType;
    int			    usage;
    int			    size, endOffset;
    glitz_geometry_type_t   type;
    glitz_geometry_format_t f;
    int			    first, width, count;
    glitz_multi_array_t     *array;
} xglGeometryRec, *xglGeometryPtr;

#ifdef RENDER
typedef struct _xglFBox {
    glitz_float_t x1, y1, x2, y2;
} xglFBoxRec;

typedef union _xglBox {
    BoxRec     sBox;
    xglFBoxRec fBox;
} xglBoxRec, *xglBoxPtr;

typedef struct _xglRange {
    int		 first;
    unsigned int count;
} xglRangeRec, *xglRangePtr;

typedef struct _xglGlyphTexture {
    PicturePtr		    pMask;
    glitz_pixel_format_t    pixel;
    glitz_geometry_format_t format;
    int			    geometryDataType;
} xglGlyphTextureRec, *xglGlyphTexturePtr;

typedef struct _xglGlyphArea {
    unsigned long serial;
    union {
	xglBoxRec   box;
	xglRangeRec range;
    } u;
} xglGlyphAreaRec, *xglGlyphAreaPtr;

typedef struct _xglGlyphCache {
    ScreenPtr		    pScreen;
    int			    depth;
    xglRootAreaRec	    rootArea;
    union {
	xglGlyphTextureRec texture;
	xglGeometryRec	   geometry;
    } u;
} xglGlyphCacheRec, *xglGlyphCachePtr;

typedef struct _xglGlyph {
    xglAreaPtr pArea;
} xglGlyphRec, *xglGlyphPtr;

extern DevPrivateKey xglGlyphPrivateKey;

#define XGL_GET_GLYPH_PRIV(pScreen, pGlyph) ((xglGlyphPtr)		     \
    dixLookupPrivate(GetGlyphPrivatesForScreen (pGlyph, pScreen),	     \
    					        xglGlyphPrivateKey))

#define XGL_GLYPH_PRIV(pScreen, pGlyph)				  \
    xglGlyphPtr pGlyphPriv = XGL_GET_GLYPH_PRIV (pScreen, pGlyph)

#endif

typedef struct _xglScreen {
    xglVisualPtr		  pVisual;

#ifdef GLXEXT
    xglVisualPtr		  pGlxVisual;
#endif

#ifdef XV
    xglVisualRec		  pXvVisual[XGL_XV_FORMAT_NUM];
#endif

    xglVisualPtr		  rootVisual;
    glitz_drawable_t		  *drawable;
    glitz_surface_t		  *surface;
    PixmapPtr			  pScreenPixmap;
    unsigned long		  features;
    int				  geometryUsage;
    int				  geometryDataType;
    Bool			  yInverted;
    int				  pboMask;
    Bool			  lines;
    xglGeometryRec		  scratchGeometry;
    xglScreenAccelInfoRec	  accel;

#ifdef RENDER
    xglGlyphCacheRec		  glyphCache[33];
    PicturePtr			  pSolidAlpha;
    struct _trapInfo {
	PicturePtr		  pMask;
	glitz_geometry_format_t	  format;
    } trapInfo;
#endif

    GetImageProcPtr		  GetImage;
    GetSpansProcPtr		  GetSpans;
    CreateWindowProcPtr		  CreateWindow;
    DestroyWindowProcPtr	  DestroyWindow;
    ChangeWindowAttributesProcPtr ChangeWindowAttributes;
    CopyWindowProcPtr		  CopyWindow;
    CreateGCProcPtr		  CreateGC;
    CloseScreenProcPtr		  CloseScreen;
    SetWindowPixmapProcPtr	  SetWindowPixmap;
    BitmapToRegionProcPtr	  BitmapToRegion;

#ifdef RENDER
    CompositeProcPtr		  Composite;
    GlyphsProcPtr		  Glyphs;
    TrapezoidsProcPtr		  Trapezoids;
    AddTrapsProcPtr		  AddTraps;
    AddTrianglesProcPtr		  AddTriangles;
    ChangePictureProcPtr	  ChangePicture;
    ChangePictureTransformProcPtr ChangePictureTransform;
    ChangePictureFilterProcPtr	  ChangePictureFilter;

    RealizeGlyphProcPtr		  RealizeGlyph;
    UnrealizeGlyphProcPtr	  UnrealizeGlyph;
#endif
} xglScreenRec, *xglScreenPtr;

extern DevPrivateKey xglScreenPrivateKey;

#define XGL_GET_SCREEN_PRIV(pScreen) ((xglScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, xglScreenPrivateKey))

#define XGL_SET_SCREEN_PRIV(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, xglScreenPrivateKey, v)

#define XGL_SCREEN_PRIV(pScreen)			     \
    xglScreenPtr pScreenPriv = XGL_GET_SCREEN_PRIV (pScreen)

#define XGL_SCREEN_WRAP(field, wrapper)	 \
    pScreenPriv->field = pScreen->field; \
    pScreen->field     = wrapper

#define XGL_SCREEN_UNWRAP(field)	\
    pScreen->field = pScreenPriv->field

#ifdef RENDER
#define XGL_PICTURE_SCREEN_WRAP(field, wrapper)	   \
    pScreenPriv->field    = pPictureScreen->field; \
    pPictureScreen->field = wrapper

#define XGL_PICTURE_SCREEN_UNWRAP(field)       \
    pPictureScreen->field = pScreenPriv->field
#endif

#define xglGCSoftwareDrawableFlag (1L << 0)
#define xglGCBadFunctionFlag	  (1L << 1)
#define xglGCPlaneMaskFlag	  (1L << 2)

typedef struct _xglGC {
    glitz_surface_t   *fg;
    glitz_surface_t   *bg;
    glitz_format_id_t id;
    glitz_operator_t  op;
    unsigned long     flags;
    GCFuncsPtr	      funcs;
    GCOpsPtr	      ops;
} xglGCRec, *xglGCPtr;

extern DevPrivateKey xglGCPrivateKey;

#define XGL_GET_GC_PRIV(pGC) ((xglGCPtr) \
    dixLookupPrivate(&(pGC)->devPrivates, xglGCPrivateKey))

#define XGL_GC_PRIV(pGC)		     \
    xglGCPtr pGCPriv = XGL_GET_GC_PRIV (pGC)

#define XGL_GC_WRAP(field, wrapper) \
    pGCPriv->field = pGC->field;    \
    pGC->field     = wrapper

#define XGL_GC_UNWRAP(field)    \
    pGC->field = pGCPriv->field


#define xglPCFillMask		(1L << 0)
#define xglPCFilterMask		(1L << 1)
#define xglPCTransformMask	(1L << 2)
#define xglPCComponentAlphaMask (1L << 3)
#define xglPCDitherMask		(1L << 4)

#define xglPFFilterMask		(1L << 8)

#define xglPixmapTargetNo  0
#define xglPixmapTargetOut 1
#define xglPixmapTargetIn  2

#ifdef XV

typedef struct _xglXvPort {
    PixmapPtr  pPixmap;
    PicturePtr pSrc;
    PicturePtr pDst;
} xglXvPortRec, *xglXvPortPtr;

#endif

typedef struct _xglPixmap {
    xglVisualPtr     pVisual;
    glitz_surface_t  *surface;
    glitz_drawable_t *drawable;
    glitz_buffer_t   *buffer;
    int		     target;
    Bool	     acceleratedTile;
    pointer	     bits;
    int		     stride;
    DamagePtr	     pDamage;
    BoxRec	     damageBox;
    RegionRec	     bitRegion;
    Bool	     allBits;
    unsigned long    pictureMask;
    xglGeometryPtr   pGeometry;

#ifdef XV
    xglXvPortPtr     pPortPriv;
#endif

} xglPixmapRec, *xglPixmapPtr;

extern DevPrivateKey xglPixmapPrivateKey;

#define XGL_GET_PIXMAP_PRIV(pPixmap) ((xglPixmapPtr) \
    dixLookupPrivate(&(pPixmap)->devPrivates, xglPixmapPrivateKey))

#define XGL_PIXMAP_PRIV(pPixmap)			     \
    xglPixmapPtr pPixmapPriv = XGL_GET_PIXMAP_PRIV (pPixmap)

#define XGL_PICTURE_CHANGES(pictureMask)  (pictureMask & 0x0000ffff)
#define XGL_PICTURE_FAILURES(pictureMask) (pictureMask & 0xffff0000)

typedef struct _xglWin {
    PixmapPtr    pPixmap;
} xglWinRec, *xglWinPtr;

extern DevPrivateKey xglWinPrivateKey;

#define XGL_GET_WINDOW_PRIV(pWin) ((xglWinPtr) \
    dixLookupPrivate(&(pWin)->devPrivates, xglWinPrivateKey))

#define XGL_WINDOW_PRIV(pWin)			    \
    xglWinPtr pWinPriv = XGL_GET_WINDOW_PRIV (pWin)

#define XGL_GET_WINDOW_PIXMAP(pWin)		       \
    (XGL_GET_WINDOW_PRIV((WindowPtr) (pWin))->pPixmap)


#define XGL_GET_DRAWABLE_PIXMAP(pDrawable)   \
    (((pDrawable)->type == DRAWABLE_WINDOW)? \
     XGL_GET_WINDOW_PIXMAP (pDrawable):	     \
     (PixmapPtr) (pDrawable))

#define XGL_DRAWABLE_PIXMAP(pDrawable)			    \
    PixmapPtr pPixmap = XGL_GET_DRAWABLE_PIXMAP (pDrawable)

#define XGL_GET_DRAWABLE_PIXMAP_PRIV(pDrawable)		      \
    XGL_GET_PIXMAP_PRIV (XGL_GET_DRAWABLE_PIXMAP (pDrawable))

#define XGL_DRAWABLE_PIXMAP_PRIV(pDrawable)			        \
    xglPixmapPtr pPixmapPriv = XGL_GET_DRAWABLE_PIXMAP_PRIV (pDrawable)

#ifdef COMPOSITE
#define __XGL_OFF_X_WIN(pPix) (-(pPix)->screen_x)
#define __XGL_OFF_Y_WIN(pPix) (-(pPix)->screen_y)
#else
#define __XGL_OFF_X_WIN(pPix) (0)
#define __XGL_OFF_Y_WIN(pPix) (0)
#endif

#define XGL_GET_DRAWABLE(pDrawable, pSurface, xOff, yOff)  \
    {							   \
	PixmapPtr _pPix;				   \
	if ((pDrawable)->type != DRAWABLE_PIXMAP) {	   \
	    _pPix = XGL_GET_WINDOW_PIXMAP (pDrawable);	   \
	    (xOff) = __XGL_OFF_X_WIN (_pPix);		   \
	    (yOff) = __XGL_OFF_Y_WIN (_pPix);		   \
	} else {					   \
	    _pPix = (PixmapPtr) (pDrawable);		   \
	    (yOff) = (xOff) = 0;			   \
	}						   \
	(pSurface) = XGL_GET_PIXMAP_PRIV (_pPix)->surface; \
    }

#define XGL_DEFAULT_DPI 96

#define XGL_SW_FAILURE_STRING "software fall-back failure"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define POWER_OF_TWO(v) ((v & (v - 1)) == 0)

#define MOD(a,b) ((a) < 0 ? ((b) - ((-(a) - 1) % (b))) - 1 : (a) % (b))

#define FIXED_TO_FLOAT(f) (((glitz_float_t) (f)) / 65536)
#define FLOAT_TO_FIXED(f) ((int) ((f) * 65536))

#define BOX_NOTEMPTY(pBox)	      \
    (((pBox)->x2 - (pBox)->x1) > 0 && \
     ((pBox)->y2 - (pBox)->y1) > 0)


/* xglinput.c */

int
xglMouseProc (DeviceIntPtr pDevice,
	      int	   onoff);

int
xglKeybdProc (DeviceIntPtr pDevice,
	      int	   onoff);

void
xglBell (int	      volume,
	 DeviceIntPtr pDev,
	 pointer      ctrl,
	 int	      something);

void
xglKbdCtrl (DeviceIntPtr pDevice,
	    KeybdCtrl	 *ctrl);

void
xglInitInput (int argc, char **argv);


/* xgloutput.c */

void
xglSetPixmapFormats (ScreenInfo *pScreenInfo);

void
xglSetRootClip (ScreenPtr pScreen,
		Bool	  enable);


/* xglcmap.c */

void
xglSetVisualTypes (int depth,
		   int visuals,
		   int redSize,
		   int greenSize,
		   int blueSize);

Bool
xglHasVisualTypes (xglVisualPtr pVisual,
		   int		depth);

glitz_format_t *
xglFindBestSurfaceFormat (ScreenPtr         pScreen,
			  xglPixelFormatPtr pPixel);

void
xglInitVisuals (ScreenPtr pScreen);

xglVisualPtr
xglFindVisualWithDepth (ScreenPtr pScreen,
			int       depth);

xglVisualPtr
xglFindVisualWithId (ScreenPtr pScreen,
		     int       vid);

void
xglClearVisualTypes (void);


/* xglparse.c */

char *
xglParseFindNext (char *cur,
		  char *delim,
		  char *save,
		  char *last);

void
xglParseScreen (char *arg);

void
xglUseMsg (void);

int
xglProcessArgument (int	 argc,
		    char **argv,
		    int	 i);


/* xglscreen.c */

Bool
xglScreenInit (ScreenPtr pScreen);

Bool
xglFinishScreenInit (ScreenPtr pScreen);

Bool
xglCloseScreen (int	  index,
		ScreenPtr pScreen);

void
xglCreateSolidAlphaPicture (ScreenPtr pScreen);


/* xglarea.c */

Bool
xglRootAreaInit (xglRootAreaPtr	    pRoot,
		 int		    maxLevel,
		 int		    width,
		 int		    height,
		 int		    devPrivateSize,
		 xglAreaFuncsPtr    funcs,
		 pointer	    closure);

void
xglRootAreaFini (xglRootAreaPtr pRoot);

void
xglLeaveArea (xglAreaPtr pArea);

void
xglWithdrawArea (xglAreaPtr pArea);

Bool
xglFindArea (xglAreaPtr pArea,
	     int	width,
	     int	height,
	     Bool	kickOut,
	     pointer	closure);


/* xglgeometry.c */

#define GEOMETRY_DATA_TYPE_SHORT 0
#define GEOMETRY_DATA_TYPE_FLOAT 1

typedef struct _xglDataTypeInfo {
    glitz_data_type_t type;
    int		      size;
} xglDataTypeInfoRec, *xglDataTypeInfoPtr;

extern xglDataTypeInfoRec xglGeometryDataTypes[2];

#define DEFAULT_GEOMETRY_DATA_TYPE GEOMETRY_DATA_TYPE_FLOAT

#define GEOMETRY_USAGE_STREAM  0
#define GEOMETRY_USAGE_STATIC  1
#define GEOMETRY_USAGE_DYNAMIC 2
#define GEOMETRY_USAGE_SYSMEM  3

#define DEFAULT_GEOMETRY_USAGE GEOMETRY_USAGE_SYSMEM

#define GEOMETRY_INIT(pScreen, pGeometry, _type, _usage, _size)		  \
    {									  \
	(pGeometry)->type      = _type;					  \
	(pGeometry)->usage     = _usage;				  \
	(pGeometry)->dataType  = DEFAULT_GEOMETRY_DATA_TYPE;		  \
	(pGeometry)->usage     = _usage;				  \
	(pGeometry)->size      = 0;					  \
	(pGeometry)->endOffset = 0;					  \
	(pGeometry)->data      = (pointer) 0;				  \
	(pGeometry)->buffer    = NULL;					  \
	(pGeometry)->broken    = FALSE;					  \
	(pGeometry)->xOff      = 0;					  \
	(pGeometry)->yOff      = 0;					  \
	(pGeometry)->array     = NULL;					  \
	(pGeometry)->first     = 0;					  \
	(pGeometry)->count     = 0;					  \
	if (_type == GLITZ_GEOMETRY_TYPE_VERTEX)			  \
	{								  \
	    (pGeometry)->width = 2;					  \
	    (pGeometry)->f.vertex.type =				  \
		xglGeometryDataTypes[(pGeometry)->dataType].type;	  \
	    (pGeometry)->f.vertex.bytes_per_vertex = (pGeometry)->width * \
		xglGeometryDataTypes[(pGeometry)->dataType].size;	  \
	    (pGeometry)->f.vertex.primitive = GLITZ_PRIMITIVE_QUADS;	  \
	    (pGeometry)->f.vertex.attributes = 0;			  \
	    (pGeometry)->f.vertex.src.type = GLITZ_DATA_TYPE_FLOAT;	  \
	    (pGeometry)->f.vertex.src.size = GLITZ_COORDINATE_SIZE_X;	  \
	    (pGeometry)->f.vertex.src.offset = 0;			  \
	    (pGeometry)->f.vertex.mask.type = GLITZ_DATA_TYPE_FLOAT;	  \
	    (pGeometry)->f.vertex.mask.size = GLITZ_COORDINATE_SIZE_X;	  \
	    (pGeometry)->f.vertex.mask.offset = 0;			  \
	}								  \
	else								  \
	{								  \
	    (pGeometry)->width = 0;					  \
	    (pGeometry)->f.bitmap.scanline_order =			  \
		GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;			  \
	    (pGeometry)->f.bitmap.bytes_per_line = 0;			  \
	    (pGeometry)->f.bitmap.pad = GLYPHPADBYTES;			  \
	}								  \
	if (_size)							  \
	    xglGeometryResize (pScreen, pGeometry, _size);		  \
    }

#define GEOMETRY_UNINIT(pGeometry)			    \
    {							    \
	if ((pGeometry)->array)				    \
	    glitz_multi_array_destroy ((pGeometry)->array); \
	if ((pGeometry)->buffer)			    \
	    glitz_buffer_destroy ((pGeometry)->buffer);     \
	if ((pGeometry)->data)				    \
	    xfree ((pGeometry)->data);			    \
    }

#define GEOMETRY_SET_BUFFER(pGeometry, _buffer)		\
    {							\
	glitz_buffer_reference (_buffer);		\
	if ((pGeometry)->buffer)			\
	    glitz_buffer_destroy ((pGeometry)->buffer); \
	(pGeometry)->buffer = _buffer;			\
    }

#define GEOMETRY_SET_MULTI_ARRAY(pGeometry, _array)	    \
    {							    \
	glitz_multi_array_reference (_array);		    \
	if ((pGeometry)->array)				    \
	    glitz_multi_array_destroy ((pGeometry)->array); \
	(pGeometry)->array = _array;			    \
    }

#define GEOMETRY_RESIZE(pScreen, pGeometry, size) \
    xglGeometryResize (pScreen, pGeometry, size)

#define GEOMETRY_SET_TRANSLATE(pGeometry, _x, _y) \
    {						  \
	(pGeometry)->xOff = (_x) << 16;		  \
	(pGeometry)->yOff = (_y) << 16;		  \
    }

#define GEOMETRY_TRANSLATE(pGeometry, tx, ty) \
    {				              \
	(pGeometry)->xOff += (tx) << 16;      \
	(pGeometry)->yOff += (ty) << 16;      \
    }

#define GEOMETRY_TRANSLATE_FIXED(pGeometry, ftx, fty) \
    {						      \
	(pGeometry)->xOff += (ftx);		      \
	(pGeometry)->yOff += (fty);		      \
    }

#define GEOMETRY_SET_VERTEX_PRIMITIVE(pGeometry, _primitive) \
    (pGeometry)->f.vertex.primitive = _primitive

#define GEOMETRY_SET_VERTEX_DATA_TYPE(pGeometry, _type)		       \
    {								       \
	(pGeometry)->dataType = _type;				       \
	(pGeometry)->f.vertex.type = xglGeometryDataTypes[_type].type; \
	(pGeometry)->f.vertex.bytes_per_vertex = (pGeometry)->width *  \
	    xglGeometryDataTypes[_type].size;			       \
    }

#define GEOMETRY_ADD_BOX(pScreen, pGeometry, pBox, nBox) \
    xglGeometryAddBox (pScreen, pGeometry, pBox, nBox,	 \
		       (pGeometry)->endOffset)

#define GEOMETRY_ADD_REGION_AT(pScreen, pGeometry, pRegion, offset) \
     xglGeometryAddBox (pScreen, pGeometry,			    \
			REGION_RECTS (pRegion),			    \
			REGION_NUM_RECTS (pRegion),		    \
			offset)

#define GEOMETRY_ADD_REGION(pScreen, pGeometry, pRegion) \
    xglGeometryAddBox (pScreen, pGeometry,		 \
		       REGION_RECTS (pRegion),		 \
		       REGION_NUM_RECTS (pRegion),	 \
		       (pGeometry)->endOffset)

#define GEOMETRY_ADD_SPAN(pScreen, pGeometry, ppt, pwidth, n) \
    xglGeometryAddSpan (pScreen, pGeometry, ppt, pwidth, n,   \
			(pGeometry)->endOffset)

#define GEOMETRY_ADD_LINE(pScreen, pGeometry, loop, mode, npt, ppt) \
    xglGeometryAddLine (pScreen, pGeometry, loop, mode, npt, ppt,   \
			(pGeometry)->endOffset)

#define GEOMETRY_ADD_SEGMENT(pScreen, pGeometry, nsegInit, pSegInit) \
    xglGeometryAddSegment (pScreen, pGeometry, nsegInit, pSegInit,   \
			   (pGeometry)->endOffset)

#define GEOMETRY_FOR_GLYPH(pScreen, pGeometry, nGlyph, ppciInit, pglyphBase) \
    xglGeometryForGlyph (pScreen, pGeometry, nGlyph, ppciInit, pglyphBase);

#define GEOMETRY_ADD_TRAPEZOID(pScreen, pGeometry, pTrap, nTrap) \
    xglGeometryAddTrapezoid (pScreen, pGeometry, pTrap, nTrap,	 \
			     (pGeometry)->endOffset)

#define GEOMETRY_ADD_TRAP(pScreen, pGeometry, pTrap, nTrap) \
    xglGeometryAddTrap (pScreen, pGeometry, pTrap, nTrap,   \
			(pGeometry)->endOffset)

#define GEOMETRY_GET_FORMAT(pGeometry, format) \
    xglGeometryGetFormat (pGeometry, format)

#define GEOMETRY_ENABLE(pGeometry, surface) \
    xglSetGeometry (pGeometry, surface)

#define GEOMETRY_DISABLE(surface)				       \
    glitz_set_geometry (surface, GLITZ_GEOMETRY_TYPE_NONE, NULL, NULL)

void
xglGeometryResize (ScreenPtr	  pScreen,
		   xglGeometryPtr pGeometry,
		   int		  size);

void
xglGeometryAddBox (ScreenPtr	  pScreen,
		   xglGeometryPtr pGeometry,
		   BoxPtr	  pBox,
		   int		  nBox,
		   int		  offset);

void
xglGeometryAddSpan (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    DDXPointPtr	   ppt,
		    int		   *pwidth,
		    int		   n,
		    int		   offset);

void
xglGeometryAddLine (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    int		   loop,
		    int		   mode,
		    int		   npt,
		    DDXPointPtr    ppt,
		    int		   offset);

void
xglGeometryAddSegment (ScreenPtr      pScreen,
		       xglGeometryPtr pGeometry,
		       int	      nsegInit,
		       xSegment       *pSegInit,
		       int	      offset);

void
xglGeometryForGlyph (ScreenPtr	    pScreen,
		     xglGeometryPtr pGeometry,
		     unsigned int   nGlyph,
		     CharInfoPtr    *ppciInit,
		     pointer	    pglyphBase);

void
xglGeometryAddTrapezoid (ScreenPtr	pScreen,
			 xglGeometryPtr pGeometry,
			 xTrapezoid	*pTrap,
			 int		nTrap,
			 int		offset);

void
xglGeometryAddTrap (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    xTrap	   *pTrap,
		    int		   nTrap,
		    int		   offset);

xglGeometryPtr
xglGetScratchGeometryWithSize (ScreenPtr pScreen,
			       int	 size);

xglGeometryPtr
xglGetScratchVertexGeometryWithType (ScreenPtr pScreen,
				     int       type,
				     int       count);

xglGeometryPtr
xglGetScratchVertexGeometry (ScreenPtr pScreen,
			     int       count);

Bool
xglSetGeometry (xglGeometryPtr	pGeometry,
		glitz_surface_t *surface);


/* xglpixmap.c */

#define XGL_PIXMAP_USAGE_HINT_STREAM_DRAW  1
#define XGL_PIXMAP_USAGE_HINT_STREAM_READ  2
#define XGL_PIXMAP_USAGE_HINT_STREAM_COPY  3
#define XGL_PIXMAP_USAGE_HINT_STATIC_DRAW  4
#define XGL_PIXMAP_USAGE_HINT_STATIC_READ  5
#define XGL_PIXMAP_USAGE_HINT_STATIC_COPY  6
#define XGL_PIXMAP_USAGE_HINT_DYNAMIC_DRAW 7
#define XGL_PIXMAP_USAGE_HINT_DYNAMIC_READ 8
#define XGL_PIXMAP_USAGE_HINT_DYNAMIC_COPY 9

#define XGL_PIXMAP_USAGE_HINT_DEFAULT XGL_PIXMAP_USAGE_HINT_STREAM_DRAW

PixmapPtr
xglCreatePixmap (ScreenPtr  pScreen,
		 int	    width,
		 int	    height,
		 int	    depth,
		 unsigned   usage_hint);

void
xglFiniPixmap (PixmapPtr pPixmap);

Bool
xglDestroyPixmap (PixmapPtr pPixmap);

Bool
xglModifyPixmapHeader (PixmapPtr pPixmap,
		       int	 width,
		       int	 height,
		       int	 depth,
		       int	 bitsPerPixel,
		       int	 devKind,
		       pointer	 pPixData);

void
xglSetPixmapVisual (PixmapPtr    pPixmap,
		    xglVisualPtr pVisual);

RegionPtr
xglPixmapToRegion (PixmapPtr pPixmap);

xglGeometryPtr
xglPixmapToGeometry (PixmapPtr pPixmap,
		     int       xOff,
		     int       yOff);

Bool
xglCreatePixmapSurface (PixmapPtr pPixmap);

Bool
xglAllocatePixmapBits (PixmapPtr pPixmap, int hint);

Bool
xglMapPixmapBits (PixmapPtr pPixmap);

Bool
xglUnmapPixmapBits (PixmapPtr pPixmap);

Bool
xglCheckPixmapSize (PixmapPtr		 pPixmap,
		    xglSizeConstraintPtr pSize);

void
xglEnablePixmapAccel (PixmapPtr	      pPixmap,
		      xglAccelInfoPtr pAccel);


/* xglsync.c */

Bool
xglSyncBits (DrawablePtr pDrawable,
	     BoxPtr	 pExtents);

void
xglSyncDamageBoxBits (DrawablePtr pDrawable);

Bool
xglSyncSurface (DrawablePtr pDrawable);

Bool
xglPrepareTarget (DrawablePtr pDrawable);

void
xglAddSurfaceDamage (DrawablePtr pDrawable,
		     RegionPtr   pRegion);

void
xglAddCurrentSurfaceDamage (DrawablePtr pDrawable);

void
xglAddBitDamage (DrawablePtr pDrawable,
		 RegionPtr   pRegion);

void
xglAddCurrentBitDamage (DrawablePtr pDrawable);


/* xglsolid.c */

Bool
xglSolid (DrawablePtr	   pDrawable,
	  glitz_operator_t op,
	  glitz_surface_t  *solid,
	  xglGeometryPtr   pGeometry,
	  int		   x,
	  int		   y,
	  int		   width,
	  int		   height,
	  BoxPtr	   pBox,
	  int		   nBox);

Bool
xglSolidGlyph (DrawablePtr  pDrawable,
	       GCPtr	    pGC,
	       int	    x,
	       int	    y,
	       unsigned int nGlyph,
	       CharInfoPtr  *ppci,
	       pointer      pglyphBase);


/* xgltile.c */

xglGeometryPtr
xglTiledBoxGeometry (PixmapPtr pTile,
		     int       tileX,
		     int       tileY,
		     BoxPtr    pBox,
		     int       nBox);

Bool
xglTile (DrawablePtr	  pDrawable,
	 glitz_operator_t op,
	 PixmapPtr	  pTile,
	 int		  tileX,
	 int		  tileY,
	 xglGeometryPtr	  pGeometry,
	 int		  x,
	 int		  y,
	 int		  width,
	 int		  height,
	 BoxPtr		  pBox,
	 int		  nBox);


/* xglcopy.c */

Bool
xglCopy (DrawablePtr pSrc,
	 DrawablePtr pDst,
	 int	     dx,
	 int	     dy,
	 BoxPtr	     pBox,
	 int	     nBox);

void
xglCopyProc (DrawablePtr pSrc,
	     DrawablePtr pDst,
	     GCPtr	 pGC,
	     BoxPtr	 pBox,
	     int	 nBox,
	     int	 dx,
	     int	 dy,
	     Bool	 reverse,
	     Bool	 upsidedown,
	     Pixel	 bitplane,
	     void	 *closure);


/* xglfill.c */

Bool
xglFill (DrawablePtr	pDrawable,
	 GCPtr		pGC,
	 xglGeometryPtr pGeometry,
	 int		x,
	 int		y,
	 int		width,
	 int		height,
	 BoxPtr		pBox,
	 int		nBox);

void
xglFillSpan (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 n,
	     DDXPointPtr ppt,
	     int	 *pwidth);

void
xglFillRect (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 nrect,
	     xRectangle  *prect);

Bool
xglFillLine (DrawablePtr pDrawable,
	     GCPtr       pGC,
	     int	 mode,
	     int	 npt,
	     DDXPointPtr ppt);

Bool
xglFillSegment (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    nsegInit,
		xSegment    *pSegInit);

Bool
xglFillGlyph (DrawablePtr  pDrawable,
	      GCPtr	   pGC,
	      int	   x,
	      int	   y,
	      unsigned int nglyph,
	      CharInfoPtr  *ppciInit,
	      pointer      pglyphBase);


/* xglwindow.c */

Bool
xglCreateWindow (WindowPtr pWin);

Bool
xglDestroyWindow (WindowPtr pWin);

Bool
xglChangeWindowAttributes (WindowPtr	 pWin,
			   unsigned long mask);

void
xglCopyWindow (WindowPtr   pWin,
	       DDXPointRec ptOldOrg,
	       RegionPtr   prgnSrc);

PixmapPtr
xglGetWindowPixmap (WindowPtr pWin);

void
xglSetWindowPixmap (WindowPtr pWin,
		    PixmapPtr pPixmap);


/* xglget.c */

void
xglGetImage (DrawablePtr   pDrawable,
	     int	   x,
	     int	   y,
	     int	   w,
	     int	   h,
	     unsigned int  format,
	     unsigned long planeMask,
	     char	   *d);

void
xglGetSpans (DrawablePtr pDrawable,
	     int	 wMax,
	     DDXPointPtr ppt,
	     int	 *pwidth,
	     int	 nspans,
	     char	 *pchardstStart);


/* xglgc.c */

Bool
xglCreateGC (GCPtr pGC);

void
xglDestroyGC (GCPtr pGC);

void
xglValidateGC (GCPtr	     pGC,
	       unsigned long changes,
	       DrawablePtr   pDrawable);

void
xglFillSpans  (DrawablePtr pDrawable,
	       GCPtr	   pGC,
	       int	   nspans,
	       DDXPointPtr ppt,
	       int	   *pwidth,
	       int	   fSorted);

void
xglSetSpans (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     char	 *psrc,
	     DDXPointPtr ppt,
	     int	 *pwidth,
	     int	 nspans,
	     int	 fSorted);

void
xglPutImage (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 depth,
	     int	 x,
	     int	 y,
	     int	 w,
	     int	 h,
	     int	 leftPad,
	     int	 format,
	     char	 *bits);

RegionPtr
xglCopyArea (DrawablePtr pSrc,
	     DrawablePtr pDst,
	     GCPtr	 pGC,
	     int	 srcX,
	     int	 srcY,
	     int	 w,
	     int	 h,
	     int	 dstX,
	     int	 dstY);

RegionPtr
xglCopyPlane (DrawablePtr   pSrc,
	      DrawablePtr   pDst,
	      GCPtr	    pGC,
	      int	    srcX,
	      int	    srcY,
	      int	    w,
	      int	    h,
	      int	    dstX,
	      int	    dstY,
	      unsigned long bitPlane);

void
xglPolyPoint (DrawablePtr pDrawable,
	      GCPtr       pGC,
	      int	  mode,
	      int	  npt,
	      DDXPointPtr pptInit);

void
xglPolylines (DrawablePtr pDrawable,
	      GCPtr       pGC,
	      int	  mode,
	      int	  npt,
	      DDXPointPtr ppt);

void
xglPolySegment (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    nsegInit,
		xSegment    *pSegInit);

void
xglPolyArc (DrawablePtr pDrawable,
	    GCPtr	pGC,
	    int		narcs,
	    xArc	*pArcs);

void
xglPolyFillRect (DrawablePtr pDrawable,
		 GCPtr	     pGC,
		 int	     nrect,
		 xRectangle  *prect);

void
xglPolyFillArc (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    narcs,
		xArc	    *pArcs);

void
xglImageGlyphBlt (DrawablePtr  pDrawable,
		  GCPtr	       pGC,
		  int	       x,
		  int	       y,
		  unsigned int nglyph,
		  CharInfoPtr  *ppci,
		  pointer      pglyphBase);

void
xglPolyGlyphBlt (DrawablePtr  pDrawable,
		 GCPtr	      pGC,
		 int	      x,
		 int	      y,
		 unsigned int nglyph,
		 CharInfoPtr  *ppci,
		 pointer      pglyphBase);
void
xglPushPixels (GCPtr	   pGC,
	       PixmapPtr   pBitmap,
	       DrawablePtr pDrawable,
	       int	   w,
	       int	   h,
	       int	   x,
	       int	   y);


#ifdef MITSHM

/* xglshm.c */

void
xglShmPutImage (DrawablePtr  pDrawable,
		GCPtr	     pGC,
		int	     depth,
		unsigned int format,
		int	     w,
		int	     h,
		int	     sx,
		int	     sy,
		int	     sw,
		int	     sh,
		int	     dx,
		int	     dy,
		char	     *data);

#endif


#ifdef RENDER

/* xglpict.c */

void
xglComposite (CARD8	 op,
	      PicturePtr pSrc,
	      PicturePtr pMask,
	      PicturePtr pDst,
	      INT16	 xSrc,
	      INT16	 ySrc,
	      INT16	 xMask,
	      INT16	 yMask,
	      INT16	 xDst,
	      INT16	 yDst,
	      CARD16	 width,
	      CARD16	 height);

void
xglAddTriangles (PicturePtr pDst,
		 INT16	    xOff,
		 INT16	    yOff,
		 int	    ntri,
		 xTriangle  *tris);

void
xglChangePicture (PicturePtr pPicture,
		  Mask	     mask);

int
xglChangePictureTransform (PicturePtr    pPicture,
			   PictTransform *transform);

int
xglChangePictureFilter (PicturePtr pPicture,
			int	   filter,
			xFixed	   *params,
			int	   nparams);

PicturePtr
xglCreateDevicePicture (pointer data);

Bool
xglSyncPicture (ScreenPtr  pScreen,
		PicturePtr pPicture,
		INT16	   x,
		INT16	   y,
		CARD16	   width,
		CARD16	   height,
		INT16	   *xOff,
		INT16	   *yOff);

Bool
xglPictureInit (ScreenPtr pScreen);

void
xglPictureClipExtents (PicturePtr pPicture,
		       BoxPtr     extents);


/* xglcompose.c */

Bool
xglCompositeGeneral (CARD8	     op,
		     PicturePtr	     pSrc,
		     PicturePtr	     pMask,
		     PicturePtr	     pDst,
		     xglGeometryPtr  pGeometry,
		     INT16	     xSrc,
		     INT16	     ySrc,
		     INT16	     xMask,
		     INT16	     yMask,
		     INT16	     xDst,
		     INT16	     yDst,
		     CARD16	     width,
		     CARD16	     height);


/* xglglyph.c */

Bool
xglRealizeGlyph (ScreenPtr pScreen,
		 GlyphPtr  pGlyph);

void
xglUnrealizeGlyph (ScreenPtr pScreen,
		   GlyphPtr  pGlyph);

Bool
xglInitGlyphCache (xglGlyphCachePtr pCache,
		   ScreenPtr	    pScreen,
		   PictFormatPtr    format);

void
xglFiniGlyphCache (xglGlyphCachePtr pCache);

void
xglGlyphs (CARD8	 op,
	   PicturePtr	 pSrc,
	   PicturePtr	 pDst,
	   PictFormatPtr maskFormat,
	   INT16	 xSrc,
	   INT16	 ySrc,
	   int		 nlist,
	   GlyphListPtr	 list,
	   GlyphPtr	 *glyphs);


/* xgltrap.c */

void
xglTrapezoids (CARD8	     op,
	       PicturePtr    pSrc,
	       PicturePtr    pDst,
	       PictFormatPtr maskFormat,
	       INT16	     xSrc,
	       INT16	     ySrc,
	       int	     nTrap,
	       xTrapezoid    *traps);

void
xglAddTraps (PicturePtr pDst,
	     INT16	xOff,
	     INT16	yOff,
	     int	nTrap,
	     xTrap	*traps);

#endif

#ifdef XGL_MODULAR

/* xglloader.c */

typedef struct _xglSymbol {
    void       **ptr;
    const char *name;
} xglSymbolRec, *xglSymbolPtr;

void *
xglLoadModule (const char *name,
	       int	  flag);

void
xglUnloadModule (void *handle);

Bool
xglLookupSymbols (void         *handle,
		  xglSymbolPtr sym,
		  int	       nSym);

#endif


/* xglxv.c */

#ifdef XV

Bool
xglXvScreenInit (ScreenPtr pScreen);

#endif


/* xglhash.c */

typedef struct _xglHashTable *xglHashTablePtr;

Bool
xglLoadHashFuncs (void *handle);

xglHashTablePtr
xglNewHashTable (void);

void
xglDeleteHashTable (xglHashTablePtr pTable);

void *
xglHashLookup (const xglHashTablePtr pTable,
	       unsigned int	     key);

void
xglHashInsert (xglHashTablePtr pTable,
	       unsigned int    key,
	       void	       *data);

void
xglHashRemove (xglHashTablePtr pTable,
	       unsigned int    key);

unsigned int
xglHashFirstEntry (xglHashTablePtr pTable);

unsigned int
xglHashNextEntry (const xglHashTablePtr pTable,
		  unsigned int		key);

unsigned int
xglHashFindFreeKeyBlock (xglHashTablePtr pTable,
			 unsigned int	 numKeys);

#endif /* _XGL_H_ */
