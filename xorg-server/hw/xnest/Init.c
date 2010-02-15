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
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "mi.h"
#include <X11/fonts/fontstruct.h>

#include "Xnest.h"

#include "Display.h"
#include "Screen.h"
#include "Pointer.h"
#include "Keyboard.h"
#include "Handlers.h"
#include "Init.h"
#include "Args.h"
#include "Drawable.h"
#include "XNGC.h"
#include "XNFont.h"
#ifdef DPMSExtension
#include "dpmsproc.h"
#endif

Bool xnestDoFullGeneration = True;

EventList *xnestEvents = NULL;

void
InitOutput(ScreenInfo *screenInfo, int argc, char *argv[])
{
  int i, j;

  xnestOpenDisplay(argc, argv);
  
  screenInfo->imageByteOrder = ImageByteOrder(xnestDisplay);
  screenInfo->bitmapScanlineUnit = BitmapUnit(xnestDisplay);
  screenInfo->bitmapScanlinePad = BitmapPad(xnestDisplay);
  screenInfo->bitmapBitOrder = BitmapBitOrder(xnestDisplay);
  
  screenInfo->numPixmapFormats = 0;
  for (i = 0; i < xnestNumPixmapFormats; i++) 
    for (j = 0; j < xnestNumDepths; j++)
      if ((xnestPixmapFormats[i].depth == 1) ||
          (xnestPixmapFormats[i].depth == xnestDepths[j])) {
	screenInfo->formats[screenInfo->numPixmapFormats].depth = 
	  xnestPixmapFormats[i].depth;
	screenInfo->formats[screenInfo->numPixmapFormats].bitsPerPixel = 
	  xnestPixmapFormats[i].bits_per_pixel;
	screenInfo->formats[screenInfo->numPixmapFormats].scanlinePad = 
	  xnestPixmapFormats[i].scanline_pad;
	screenInfo->numPixmapFormats++;
	break;
      }
  
  xnestFontPrivateIndex = AllocateFontPrivateIndex();
  
  if (!xnestNumScreens) xnestNumScreens = 1;

  for (i = 0; i < xnestNumScreens; i++)
    AddScreen(xnestOpenScreen, argc, argv);

  xnestNumScreens = screenInfo->numScreens;

  xnestDoFullGeneration = xnestFullGeneration;
}

void
InitInput(int argc, char *argv[])
{
  int rc;
  rc = AllocDevicePair(serverClient, "Xnest",
                       &xnestPointerDevice,
                       &xnestKeyboardDevice,
                       xnestPointerProc,
                       xnestKeyboardProc,
                       FALSE);

  if (rc != Success)
      FatalError("Failed to init Xnest default devices.\n");

  GetEventList(&xnestEvents);

  mieqInit();

  AddEnabledDevice(XConnectionNumber(xnestDisplay));

  RegisterBlockAndWakeupHandlers(xnestBlockHandler, xnestWakeupHandler, NULL);
}

void
CloseInput(void)
{
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void AbortDDX(void)
{
  xnestDoFullGeneration = True;
  xnestCloseDisplay();
}

/* Called by GiveUp(). */
void ddxGiveUp(void)
{
  AbortDDX();
}

#ifdef __APPLE__
void
DarwinHandleGUI(int argc, char *argv[])
{
}
#endif

void OsVendorInit(void)
{
    return;
}

void OsVendorFatalError(void)
{
    return;
}

#if defined(DDXBEFORERESET)
void ddxBeforeReset(void)
{
    return;
}
#endif
