/* $Xorg: Pointer.c,v 1.4 2001/02/09 02:03:56 xorgcvs Exp $ */

/********************************************************

Copyright 1988 by Hewlett-Packard Company
Copyright 1987, 1988, 1989 by Digital Equipment Corporation, Maynard

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that copyright notice and this permission
notice appear in supporting documentation, and that the names of
Hewlett-Packard or Digital not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/*

Copyright 1987, 1988, 1989, 1998  The Open Group

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "IntrinsicI.h"
#include "PassivGraI.h"


#define AllButtonsMask (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)

Widget _XtProcessPointerEvent(
    XButtonEvent  	*event,
    Widget 		widget,
    XtPerDisplayInput 	pdi)
{
    XtDevice		device = &pdi->pointer;
    XtServerGrabPtr	newGrab = NULL, devGrab = &device->grab;
    Widget		dspWidget = NULL;
    Boolean		deactivateGrab = FALSE;

    switch (event->type)
      {
	case ButtonPress:
	  {
	      if (!IsServerGrab(device->grabType))
		{
		    Cardinal		i;

		    for (i = pdi->traceDepth;
			 i > 0 && !newGrab;
			 i--)
		      newGrab = _XtCheckServerGrabsOnWidget((XEvent*)event,
							    pdi->trace[i-1],
							    POINTER);
		}
	      if (newGrab)
		{
		    /* Activate the grab */
		    device->grab = *newGrab;
		    device->grabType = XtPassiveServerGrab;
		}
	  }
	  break;

	case ButtonRelease:
	  {
	      if ((device->grabType == XtPassiveServerGrab) &&
		  !(event->state & ~(Button1Mask << (event->button - 1)) &
		    AllButtonsMask))
		deactivateGrab = TRUE;
	  }
	  break;
      }

    if (IsServerGrab(device->grabType) && !(devGrab)->ownerEvents)
      dspWidget = (devGrab)->widget;
    else
      dspWidget = widget;

    if (deactivateGrab)
      device->grabType = XtNoServerGrab;

    return dspWidget;
}
