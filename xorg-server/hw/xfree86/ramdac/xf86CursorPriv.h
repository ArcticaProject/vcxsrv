
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifndef _XF86CURSORPRIV_H
#define _XF86CURSORPRIV_H

#include "xf86Cursor.h"
#include "mipointrst.h"

typedef struct {
    Bool			SWCursor;
    Bool			isUp;
    Bool			showTransparent;
    short			HotX;
    short			HotY;
    short			x;
    short			y;
    CursorPtr			CurrentCursor, CursorToRestore;
    xf86CursorInfoPtr		CursorInfoPtr;
    CloseScreenProcPtr          CloseScreen;
    RecolorCursorProcPtr	RecolorCursor;
    InstallColormapProcPtr	InstallColormap;
    QueryBestSizeProcPtr	QueryBestSize;
    miPointerSpriteFuncPtr	spriteFuncs;
    Bool			PalettedCursor;
    ColormapPtr			pInstalledMap;
    Bool                	(*SwitchMode)(int, DisplayModePtr,int);
    xf86EnableDisableFBAccessProc *EnableDisableFBAccess;
    CursorPtr                   SavedCursor;

    /* Number of requests to force HW cursor */
    int				ForceHWCursorCount;
    Bool			HWCursorForced;

    pointer			transparentData;
} xf86CursorScreenRec, *xf86CursorScreenPtr;

void xf86SetCursor(ScreenPtr pScreen, CursorPtr pCurs, int x, int y);
void xf86SetTransparentCursor(ScreenPtr pScreen);
void xf86MoveCursor(ScreenPtr pScreen, int x, int y);
void xf86RecolorCursor(ScreenPtr pScreen, CursorPtr pCurs, Bool displayed);
Bool xf86InitHardwareCursor(ScreenPtr pScreen, xf86CursorInfoPtr infoPtr);

CARD32 xf86ReverseBitOrder(CARD32 data);

extern _X_EXPORT DevPrivateKey xf86CursorScreenKey;

#endif /* _XF86CURSORPRIV_H */
