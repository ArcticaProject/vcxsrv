/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *  ABSTRACT:
 *
 *      This module is the platform-specific but conditionally independent
 *      code for the XTrap extension (usually I/O or platform setup).
 *      This is shared code and is subject to change only by team approval.
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <errno.h>
#include <X11/Xos.h>
#ifdef PC
# include "fcntl.h"
# include "io.h"
# define O_NDELAY 0L
#endif

#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>        /* From library include environment */
#include "inputstr.h"    /* From server include env. (must be before Xlib.h!) */
#ifdef PC
# include "scrintst.h"          /* Screen struct */
# include "extnsist.h"
#else
# include "extnsionst.h"        /* Server ExtensionEntry definitions */
# include "scrnintstr.h"        /* Screen struct */
#endif

#include <X11/extensions/xtrapdi.h>
#include <X11/extensions/xtrapddmi.h>
#include <X11/extensions/xtrapproto.h>

extern int XETrapErrorBase;
extern xXTrapGetAvailReply XETrap_avail;
extern DevicePtr XETrapKbdDev;
extern DevicePtr XETrapPtrDev;

/*
 *  DESCRIPTION:
 *
 *      This function performs the platform specific setup for server
 *      extension implementations.
 */
void XETrapPlatformSetup()
{
}


#if !defined _XINPUT
/*
 *  DESCRIPTION:
 *
 *  This routine processes the simulation of some input event.
 *
 */
int XETrapSimulateXEvent(register xXTrapInputReq *request,
    register ClientPtr client)
{
    ScreenPtr pScr = NULL;
    int status = Success;
    xEvent xev;
    register int x = request->input.x;
    register int y = request->input.y;
    DevicePtr keydev = (DevicePtr)inputInfo.keyboard;
    DevicePtr ptrdev = (DevicePtr)inputInfo.pointer;

    if (request->input.screen < screenInfo.numScreens)
    {
        pScr = screenInfo.screens[request->input.screen];
    }
    else
    {   /* Trying to play bogus events to this WS! */
#ifdef VERBOSE
        ErrorF("%s:  Trying to send events to screen %d!\n", XTrapExtName,
            request->input.screen);
#endif
        status = XETrapErrorBase + BadScreen;
    }
    /* Fill in the event structure with the information
     * Note:  root, event, child, eventX, eventY, state, and sameScreen
     *        are all updated by FixUpEventFromWindow() when the events
     *        are delivered via DeliverDeviceEvents() or whatever.  XTrap
     *        needs to only concern itself with type, detail, time, rootX, 
     *        and rootY.
     */
    if (status == Success)
    {
        xev.u.u.type   = request->input.type;
        xev.u.u.detail = request->input.detail;
        xev.u.keyButtonPointer.time   = GetTimeInMillis();
        xev.u.keyButtonPointer.rootX = x;
        xev.u.keyButtonPointer.rootY = y;

        if (request->input.type == MotionNotify)
        {   /* Set new cursor position on screen */
            XETrap_avail.data.cur_x = x;
            XETrap_avail.data.cur_y = y;
          NewCurrentScreen (pScr, x, y); /* fix from amnonc@mercury.co.il */
            if (!(*pScr->SetCursorPosition)(pScr, x, y, xFalse))
            {
                status = BadImplementation;
            }
        }
    }
    if (status == Success)
    {
        switch(request->input.type)
        {   /* Now process the event appropriately */
            case KeyPress:
            case KeyRelease:
                (*XETrapKbdDev->realInputProc)(&xev,(DeviceIntPtr)keydev, 1L);
                break;
            case MotionNotify:
            case ButtonPress:
            case ButtonRelease:
                (*XETrapPtrDev->realInputProc)(&xev,(DeviceIntPtr)ptrdev, 1L);
                break;
            default:
                status = BadValue;
                break;
        }
    }
    return(status);
}
#endif /* _XINPUT */
