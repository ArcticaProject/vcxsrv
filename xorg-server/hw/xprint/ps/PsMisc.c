/*

Copyright 1996, 1998  The Open Group

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
/*
 * (c) Copyright 1996 Hewlett-Packard Company
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc.
 * (c) Copyright 1996 Digital Equipment Corp.
 * (c) Copyright 1996 Fujitsu Limited
 * (c) Copyright 1996 Hitachi, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the names of the copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from said copyright holders.
 */

/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PsMisc.c
**    *
**    *  Contents:	Miscellaneous code for Ps driver.
**    *
**    *  Created By:	Roger Helmendach (Liberty Systems)
**    *
**    *  Copyright:	Copyright 1996 The Open Group, Inc.
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xos.h>	/* for SIGCLD on pre-POSIX systems */
#include <stdio.h>
#include "Ps.h"

#include "cursor.h"
#include "resource.h"

#include "windowstr.h"
#include "propertyst.h"


/*ARGSUSED*/
void
PsQueryBestSize(
  int type,
  short *pwidth,
  short *pheight,
  ScreenPtr pScreen)
{
  unsigned width, highBit;

  switch(type)
  {
    case CursorShape:
      *pwidth  = 0;
      *pheight = 0;
      break;
    case TileShape:
    case StippleShape:
      width = *pwidth;
      if (!width) break;
      /* Return the nearest power of two >= what they gave us */
      highBit = 0x80000000;
      /* Find the highest 1 bit in the given width */
      while(!(highBit & width))
        highBit >>= 1;
      /* If greater than that then return the next power of two */
      if((highBit - 1) & width)
        highBit <<= 1;
      *pwidth = highBit;
      /* height is a don't-care */
        break;
  }
}

/*
 * PsGetMediumDimensions is installed in the GetMediumDimensions field
 * of each Ps-initialized context.
 */
int
PsGetMediumDimensions(XpContextPtr pCon, CARD16 *width, CARD16 *height)
{
    XpGetMediumDimensions(pCon, width, height);
    return Success;
}

/*
 * PsGetReproducibleArea is installed in the GetReproducibleArea field
 * of each Ps-initialized context.
 */
int
PsGetReproducibleArea(XpContextPtr pCon, xRectangle *pRect)
{
    XpGetReproductionArea(pCon, pRect);
    return Success;
}

/*
 * PsSetImageResolution is installed in the SetImageResolution field
 * of each Ps-initialized context.
 */
int
PsSetImageResolution(XpContextPtr pCon, int imageRes, Bool *status)
{
    pCon->imageRes = imageRes;
    *status = True;
    return Success;
}

/*
 * GetPropString searches the window heirarchy from pWin up looking for
 * a property by the name of propName.  If found, returns the property's
 * value. If not, it returns NULL.
 */
/*
char *
GetPropString(
    WindowPtr pWin,
    char *propName)
{
    Atom atom;
    PropertyPtr pProp = (PropertyPtr)NULL;
    char *retVal;

    atom = MakeAtom(propName, strlen(propName), FALSE);
    if(atom != BAD_RESOURCE)
    {
        WindowPtr pPropWin;
	int rc, n;
*/

	/*
	 * The atom has been defined, but it might only exist as a
	 * property on an unrelated window.
	 */
/*
        for(pPropWin = pWin; pPropWin != (WindowPtr)NULL; 
	    pPropWin = pPropWin->parent)
        {
	    rc = dixLookupProperty(&pProp, pPropWin, atom,
				   serverClient, DixReadAccess);
	    if (rc == Success)
		break;
	    else
		pProp = NULL;
        }
	if(pProp == (PropertyPtr)NULL)
	    return (char *)NULL;

	n = (pProp->format/8) * pProp->size; *//* size (bytes) of prop */
/*
	retVal = (char *)xalloc(n + 1);
	(void)memcpy((void *)retVal, (void *)pProp->data, n);
	retVal[n] = '\0';

	return retVal;
    }

    return (char *)NULL;
}

#include <signal.h>

*/
/* ARGSUSED */
/*
static void SigchldHndlr (int dummy)
{
    int   status, w;
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SigchldHndlr;

    w = wait (&status);

*/
    /*
     * Is this really necessary?
     */
/*
    sigaction(SIGCHLD, &act, (struct sigaction *)NULL);
}
*/

/*
 * SystemCmd provides a wrapper for the 'system' library call.  The call
 * appears to be sensitive to the handling of SIGCHLD, so this wrapper
 * sets the status to SIG_DFL, and then resets the established handler
 * after system returns.
 */
/*
int
SystemCmd(char *cmdStr)
{
    int status;
    struct sigaction newAct, oldAct;
    sigfillset(&newAct.sa_mask);
    newAct.sa_flags = 0;
    newAct.sa_handler = SIG_DFL;
    sigfillset(&oldAct.sa_mask);
    oldAct.sa_flags = 0;
    oldAct.sa_handler = SigchldHndlr;

*/
    /*
     * get the old handler, and set the action to IGN
     */
/*
    sigaction(SIGCHLD, &newAct, &oldAct);

    status = system (cmdStr);

    sigaction(SIGCHLD, &oldAct, (struct sigaction *)NULL);
    return status;
}
*/

Bool
PsCloseScreen(
  int       index,
  ScreenPtr pScreen)
{
  return TRUE;
}

void
PsLineAttrs(
  PsOutPtr    psOut,
  GCPtr       pGC,
  ColormapPtr cMap)
{
  int        i;
  int        nDsh;
  int        dshOff;
  int       *dsh;
  PsCapEnum  cap;
  PsJoinEnum join;

  switch(pGC->capStyle) {
    case CapButt:       cap = PsCButt;   break;
    case CapRound:      cap = PsCRound;  break;
    case CapProjecting: cap = PsCSquare; break;
    default:            cap = PsCButt;   break; }
  switch(pGC->joinStyle) {
    case JoinMiter:   join = PsJMiter; break;
    case JoinRound:   join = PsJRound; break;
    case JoinBevel:   join = PsJBevel; break;
    default:          join = PsJBevel; break; }
  if( pGC->lineStyle==LineSolid ) { nDsh = dshOff = 0; dsh = (int *)0; }
  else
  {
    nDsh   = pGC->numInDashList;
    dshOff = pGC->dashOffset;
    if( !nDsh ) dsh = (int *)0;
    else
    {
      dsh = (int *)xalloc(sizeof(int)*nDsh);
      for( i=0 ; i<nDsh ; i++ ) dsh[i] = (int)pGC->dash[i]&0xFF;
    }
  }

  if( pGC->lineStyle!=LineDoubleDash )
    PsOut_LineAttrs(psOut, (int)pGC->lineWidth,
                    cap, join, nDsh, dsh, dshOff, -1);
  else
    PsOut_LineAttrs(psOut, (int)pGC->lineWidth,
                    cap, join, nDsh, dsh, dshOff,
                    PsGetPixelColor(cMap, pGC->bgPixel));
  if( nDsh && dsh ) xfree(dsh);
}
