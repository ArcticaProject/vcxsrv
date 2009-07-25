/******************************************************************************
 * 
 * Copyright (c) 1994, 1995  Hewlett-Packard Company
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Hewlett-Packard
 * Company shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Hewlett-Packard Company.
 * 
 *     Header file for DIX-related DBE
 *
 *****************************************************************************/

#ifndef DBE_STRUCT_H
#define DBE_STRUCT_H


/* INCLUDES */

#define NEED_DBE_PROTOCOL
#include <X11/extensions/Xdbeproto.h>
#include "windowstr.h"
#include "privates.h"


/* DEFINES */

#define DBE_SCREEN_PRIV(pScreen) ((DbeScreenPrivPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, dbeScreenPrivKey))

#define DBE_SCREEN_PRIV_FROM_DRAWABLE(pDrawable) \
    DBE_SCREEN_PRIV((pDrawable)->pScreen)

#define DBE_SCREEN_PRIV_FROM_WINDOW_PRIV(pDbeWindowPriv) \
    DBE_SCREEN_PRIV((pDbeWindowPriv)->pWindow->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_WINDOW(pWindow) \
    DBE_SCREEN_PRIV((pWindow)->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_PIXMAP(pPixmap) \
    DBE_SCREEN_PRIV((pPixmap)->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_GC(pGC)\
    DBE_SCREEN_PRIV((pGC)->pScreen)

#define DBE_WINDOW_PRIV(pWin) ((DbeWindowPrivPtr) \
    dixLookupPrivate(&(pWin)->devPrivates, dbeWindowPrivKey))

/* Initial size of the buffer ID array in the window priv. */
#define DBE_INIT_MAX_IDS	2

/* Reallocation increment for the buffer ID array. */
#define DBE_INCR_MAX_IDS	4

/* Marker for free elements in the buffer ID array. */
#define DBE_FREE_ID_ELEMENT	0

extern void DbeExtensionInit (void);

/* TYPEDEFS */

/* Record used to pass swap information between DIX and DDX swapping
 * procedures.
 */
typedef struct _DbeSwapInfoRec
{
    WindowPtr		pWindow;
    unsigned char	swapAction;

} DbeSwapInfoRec, *DbeSwapInfoPtr;

/*
 ******************************************************************************
 ** Per-window data
 ******************************************************************************
 */

typedef struct _DbeWindowPrivRec
{
    /* A pointer to the window with which the DBE window private (buffer) is
     * associated.
     */
    WindowPtr		pWindow;

    /* Last known swap action for this buffer.  Legal values for this field
     * are XdbeUndefined, XdbeBackground, XdbeUntouched, and XdbeCopied.
     */
    unsigned char	swapAction;

    /* Last known buffer size.
     */
    unsigned short	width, height;

    /* Coordinates used for static gravity when the window is positioned.
     */
    short		x, y;

    /* Number of XIDs associated with this buffer.
     */
    int			nBufferIDs;

    /* Capacity of the current buffer ID array, IDs. */
    int			maxAvailableIDs;

    /* Pointer to the array of buffer IDs.  This initially points to initIDs.
     * When the static limit of the initIDs array is reached, the array is
     * reallocated and this pointer is set to the new array instead of initIDs.
     */
    XID			*IDs;

    /* Initial array of buffer IDs.  We are defining the XID array within the
     * window priv to optimize for data locality.  In most cases, only one
     * buffer will be associated with a window.  Having the array declared
     * here can prevent us from accessing the data in another memory page,
     * possibly resulting in a page swap and loss of performance.  Initially we
     * will use this array to store buffer IDs.  For situations where we have
     * more IDs than can fit in this static array, we will allocate a larger
     * array to use, possibly suffering a performance loss. 
     */
    XID			initIDs[DBE_INIT_MAX_IDS];

    /* Device-specific private information.
     */
    PrivateRec		*devPrivates;

} DbeWindowPrivRec, *DbeWindowPrivPtr;


/*
 ******************************************************************************
 ** Per-screen data
 ******************************************************************************
 */

typedef struct _DbeScreenPrivRec
{
    /* Resources created by DIX to be used by DDX */
    RESTYPE	dbeDrawableResType;
    RESTYPE	dbeWindowPrivResType;

    /* Private indices created by DIX to be used by DDX */
    DevPrivateKey dbeScreenPrivKey;
    DevPrivateKey dbeWindowPrivKey;

    /* Wrapped functions
     * It is the responsibilty of the DDX layer to wrap PositionWindow().
     * DbeExtensionInit wraps DestroyWindow().
     */
    PositionWindowProcPtr PositionWindow;
    DestroyWindowProcPtr  DestroyWindow;

    /* Per-screen DIX routines */
    Bool	(*SetupBackgroundPainter)(
		WindowPtr /*pWin*/,
		GCPtr /*pGC*/
);

    /* Per-screen DDX routines */
    Bool	(*GetVisualInfo)(
		ScreenPtr /*pScreen*/,
		XdbeScreenVisualInfo * /*pVisInfo*/
);
    int		(*AllocBackBufferName)(
		WindowPtr /*pWin*/,
		XID /*bufId*/,
		int /*swapAction*/
);
    int		(*SwapBuffers)(
		ClientPtr /*client*/,
		int * /*pNumWindows*/,
		DbeSwapInfoPtr /*swapInfo*/
);
    void	(*BeginIdiom)(
		ClientPtr /*client*/
);
    void	(*EndIdiom)(
		ClientPtr /*client*/
);
    void	(*WinPrivDelete)(
		DbeWindowPrivPtr /*pDbeWindowPriv*/,
		XID /*bufId*/
);
    void	(*ResetProc)(
		ScreenPtr /*pScreen*/
);

    /* Device-specific private information.
     */
    PrivateRec	*devPrivates;

} DbeScreenPrivRec, *DbeScreenPrivPtr;

#endif /* DBE_STRUCT_H */
