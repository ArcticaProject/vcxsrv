/*

Copyright 1993 by Davor Matic

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.  Davor Matic makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

*/

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "screenint.h"
#include "input.h"
#include "misc.h"
#include "cursor.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"

#include "Xnest.h"

#include "Display.h"
#include "Screen.h"
#include "XNCursor.h"
#include "Visual.h"
#include "Keyboard.h"
#include "Args.h"

Bool
xnestRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor)
{
  XImage *ximage;
  Pixmap source, mask;
  XColor fg_color, bg_color;
  unsigned long valuemask;
  XGCValues values;

  valuemask = GCFunction | 
              GCPlaneMask | 
	      GCForeground |
	      GCBackground |
	      GCClipMask;

  values.function = GXcopy;
  values.plane_mask = AllPlanes;
  values.foreground = 1L;
  values.background = 0L;
  values.clip_mask = None;

  XChangeGC(xnestDisplay, xnestBitmapGC, valuemask, &values);

  source = XCreatePixmap(xnestDisplay, 
			 xnestDefaultWindows[pScreen->myNum],
			 pCursor->bits->width,
			 pCursor->bits->height,
			 1);
  
  mask   = XCreatePixmap(xnestDisplay, 
			 xnestDefaultWindows[pScreen->myNum],
			 pCursor->bits->width,
			 pCursor->bits->height,
			 1);
  
  ximage = XCreateImage(xnestDisplay, 
			xnestDefaultVisual(pScreen),
			1, XYBitmap, 0, 
			(char *)pCursor->bits->source,
			pCursor->bits->width,
			pCursor->bits->height,
			BitmapPad(xnestDisplay), 0);
  
  XPutImage(xnestDisplay, source, xnestBitmapGC, ximage,
	    0, 0, 0, 0, pCursor->bits->width, pCursor->bits->height);
  
  XFree(ximage);
  
  ximage = XCreateImage(xnestDisplay, 
			xnestDefaultVisual(pScreen),
			1, XYBitmap, 0, 
			(char *)pCursor->bits->mask,
			pCursor->bits->width,
			pCursor->bits->height,
			BitmapPad(xnestDisplay), 0);
  
  XPutImage(xnestDisplay, mask, xnestBitmapGC, ximage,
	    0, 0, 0, 0, pCursor->bits->width, pCursor->bits->height);
  
  XFree(ximage);
  
  fg_color.red = pCursor->foreRed;
  fg_color.green = pCursor->foreGreen;
  fg_color.blue = pCursor->foreBlue;
  
  bg_color.red = pCursor->backRed;
  bg_color.green = pCursor->backGreen;
  bg_color.blue = pCursor->backBlue;

  xnestSetCursorPriv(pCursor, pScreen, xalloc(sizeof(xnestPrivCursor)));
  xnestCursor(pCursor, pScreen) = 
    XCreatePixmapCursor(xnestDisplay, source, mask, &fg_color, &bg_color,
			pCursor->bits->xhot, pCursor->bits->yhot);
  
  XFreePixmap(xnestDisplay, source);
  XFreePixmap(xnestDisplay, mask);
  
  return True;
}

Bool
xnestUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor)
{
  XFreeCursor(xnestDisplay, xnestCursor(pCursor, pScreen));
  xfree(xnestGetCursorPriv(pCursor, pScreen));
  return True;
}

void
xnestRecolorCursor(ScreenPtr pScreen, CursorPtr pCursor, Bool displayed)
{
  XColor fg_color, bg_color;
  
  fg_color.red = pCursor->foreRed;
  fg_color.green = pCursor->foreGreen;
  fg_color.blue = pCursor->foreBlue;
  
  bg_color.red = pCursor->backRed;
  bg_color.green = pCursor->backGreen;
  bg_color.blue = pCursor->backBlue;
  
  XRecolorCursor(xnestDisplay, 
		 xnestCursor(pCursor, pScreen),
		 &fg_color, &bg_color);
}

void xnestSetCursor (ScreenPtr pScreen, CursorPtr pCursor, int x, int y)
{
    if (pCursor)
    {
	XDefineCursor(xnestDisplay, 
		      xnestDefaultWindows[pScreen->myNum], 
		      xnestCursor(pCursor, pScreen));
    }
}

void
xnestMoveCursor (ScreenPtr pScreen, int x, int y)
{
}
