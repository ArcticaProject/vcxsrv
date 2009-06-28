/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include "windowstr.h"
#include "servermd.h"
#include "DiPrint.h"

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	If this is built as a print-only server, then we must supply
 *      an InitOutput routine.  If a normal server's real ddx InitOutput
 *      is used, then it should call PrinterInitOutput if it so desires.
 *      The ddx-level hook is needed to allow the printer stuff to 
 *      create additional screens.  An extension can't reliably do
 *      this for two reasons:
 *
 *          1) If InitOutput doesn't create any screens, then main()
 *             exits before calling InitExtensions().
 *
 *          2) Other extensions may rely on knowing about all screens
 *             when they initialize, and we can't guarantee the order
 *             of extension initialization.
 *
 * Results:
 *	ScreenInfo filled in, and PrinterInitOutput is called to create
 *      the screens associated with printers.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

void 
InitOutput(
    ScreenInfo   *pScreenInfo,
    int          argc,
    char         **argv)

{
    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = 0; /* get them in PrinterInitOutput */
    screenInfo.numVideoScreens = 0;
    
#ifdef PRINT_ONLY_SERVER
    PrinterInitOutput(pScreenInfo, argc, argv);
#endif

}

void
DDXRingBell(int volume, int pitch, int duration)
{
   /* dummy func; link fails without */
}

static void
BellProc(
    int volume,
    DeviceIntPtr pDev)
{
    return;
}

static void
KeyControlProc(
    DeviceIntPtr pDev,
    KeybdCtrl *ctrl)
{
    return;
}

static KeySym printKeyMap[256];
static CARD8 printModMap[256];

static int
KeyboardProc(
    DevicePtr pKbd,
    int what,
    int argc,
    char *argv[])
{
    KeySymsRec keySyms;

    keySyms.minKeyCode = 8;
    keySyms.maxKeyCode = 8;
    keySyms.mapWidth = 1;
    keySyms.map = printKeyMap;

    switch(what)
    {
	case DEVICE_INIT:
	    InitKeyboardDeviceStruct(pKbd, &keySyms, printModMap, 
				     (BellProcPtr)BellProc,
				     KeyControlProc);
	    break;
	case DEVICE_ON:
	    break;
	case DEVICE_OFF:
	    break;
	case DEVICE_CLOSE:
	    break;
    }
    return Success;
}

#include "../mi/mipointer.h"
static int
PointerProc(
     DevicePtr pPtr,
     int what,
     int argc,
     char *argv[])
{
#define NUM_BUTTONS 1
    CARD8 map[NUM_BUTTONS];

    switch(what)
      {
        case DEVICE_INIT:
	  {
	      map[0] = 0;
	      InitPointerDeviceStruct(pPtr, map, NUM_BUTTONS, 
				      GetMotionHistory,
				      (PtrCtrlProcPtr)_XpVoidNoop,
				      GetMotionHistorySize(), 2);
	      break;
	  }
        case DEVICE_ON:
	  break;
        case DEVICE_OFF:
	  break;
        case DEVICE_CLOSE:
	  break;
      }
    return Success;
}

void
InitInput(
     int       argc,
     char **argv)
{
    DeviceIntPtr ptr, kbd;

    ptr = AddInputDevice((DeviceProc)PointerProc, TRUE);
    kbd = AddInputDevice((DeviceProc)KeyboardProc, TRUE);
    RegisterPointerDevice(ptr);
    RegisterKeyboardDevice(kbd);
    return;
}


Bool
LegalModifier(
     unsigned int key,
     DeviceIntPtr dev)
{
    return TRUE;
}

void
ProcessInputEvents(void)
{
}

#ifdef __APPLE__
#include "micmap.h"

void GlxExtensionInit(void);
void GlxWrapInitVisuals(miInitVisualsProcPtr *procPtr);

void
DarwinHandleGUI(int argc, char *argv[])
{
}

void DarwinGlxExtensionInit(void)
{
    GlxExtensionInit();
}

void DarwinGlxWrapInitVisuals(
    miInitVisualsProcPtr *procPtr)
{
    GlxWrapInitVisuals(procPtr);
}
#endif

#ifdef DDXOSINIT
void
OsVendorInit(void)
{
}
#endif

#ifdef DDXOSFATALERROR
void
OsVendorFatalError(void)
{
}
#endif

#ifdef DDXBEFORERESET
void
ddxBeforeReset(void)
{
    return;
}
#endif

/* ddxInitGlobals - called by |InitGlobals| from os/util.c */
void ddxInitGlobals(void)
{
    PrinterInitGlobals();
}

/****************************************
* ddxUseMsg()
*
* Called my usemsg from os/utils/c
*
*****************************************/

void ddxUseMsg(void)
{
}

void AbortDDX (void)
{
}

void ddxGiveUp(void)	/* Called by GiveUp() */
{
}

int
ddxProcessArgument (
    int argc,
    char *argv[],
    int i)
{
    return(0);
}

#ifdef XINPUT

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "XIstubs.h"
#include "exglobals.h"

/* Place dummy config functions here instead of config/config.c, 
   since Xprint does not use D-BUS */
void config_init() { }
void config_fini() { }


int
ChangePointerDevice (
    DeviceIntPtr       old_dev,
    DeviceIntPtr       new_dev,
    unsigned char      x,
    unsigned char      y)
{
        return (BadDevice);
}

int
ChangeDeviceControl (
    register    ClientPtr       client,
    DeviceIntPtr dev,
    xDeviceCtl  *control)
{
    return BadMatch;
}

int
NewInputDeviceRequest(InputOption *options, DeviceIntPtr *pdev)
{
    return BadValue;
}

void
DeleteInputDeviceRequest(DeviceIntPtr dev)
{
}

void
OpenInputDevice (
    DeviceIntPtr dev,
    ClientPtr client,
    int *status)
{
    return;
}

void
AddOtherInputDevices (void)
{
    return;
}

void
CloseInputDevice (
    DeviceIntPtr        dev,
    ClientPtr           client)
{
    return;
}

int
ChangeKeyboardDevice (
    DeviceIntPtr        old_dev,
    DeviceIntPtr        new_dev)
{
    return (Success);
}

int
SetDeviceMode (
    register    ClientPtr       client,
    DeviceIntPtr dev,
    int         mode)
{
    return BadMatch;
}

int
SetDeviceValuators (
    register    ClientPtr       client,
    DeviceIntPtr dev,
    int         *valuators,
    int         first_valuator,
    int         num_valuators)
{
    return BadMatch;
}


#endif /* XINPUT */

#ifdef AIXV3
/*
 * This is just to get the server to link on AIX, where some bits
 * that should be in os/ are instead in hw/ibm.
 */
int SelectWaitTime = 10000; /* usec */
#endif
