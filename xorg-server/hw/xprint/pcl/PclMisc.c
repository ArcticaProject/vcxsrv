/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclMisc.c
**    *
**    *  Contents:
**    *                 Miscellaneous code for Pcl driver.
**    *
**    *  Created:	2/01/95
**    *
**    *********************************************************
** 
********************************************************************/
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

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/Xos.h>	/* for SIGCLD on pre-POSIX systems */
#include "Pcl.h"

#include "cursor.h"
#include "resource.h"

#include "windowstr.h"
#include "propertyst.h"
#include "attributes.h"


/*ARGSUSED*/
void
PclQueryBestSize(
    int type,
    short *pwidth,
    short *pheight,
    ScreenPtr pScreen)
{
    unsigned width, highBit;

    switch(type)
    {
      case CursorShape:
	*pwidth = 0;
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
 * GetPropString searches the window heirarchy from pWin up looking for
 * a property by the name of propName.  If found, returns the property's
 * value. If not, it returns NULL.
 */
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

	/*
	 * The atom has been defined, but it might only exist as a
	 * property on an unrelated window.
	 */
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

	n = (pProp->format/8) * pProp->size; /* size (bytes) of prop */
	retVal = (char *)xalloc(n + 1);
	(void)memcpy((void *)retVal, (void *)pProp->data, n);
	retVal[n] = '\0';

	return retVal;
    }

    return (char *)NULL;
}

#include <signal.h>
#include <errno.h>

/* ARGSUSED */
static void SigchldHndlr (
    int dummy)
{
    int   status;
    int olderrno = errno;
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SigchldHndlr;

    (void) wait (&status);

    /*
     * Is this really necessary?
     */
    sigaction(SIGCHLD, &act, (struct sigaction *)NULL);
    errno = olderrno;
}

/*
 * SystemCmd provides a wrapper for the 'system' library call.  The call
 * appears to be sensitive to the handling of SIGCHLD, so this wrapper
 * sets the status to SIG_DFL, and then resets the established handler
 * after system returns.
 */
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

    /*
     * get the old handler, and set the action to IGN
     */
    sigaction(SIGCHLD, &newAct, &oldAct);

    status = system (cmdStr);

    sigaction(SIGCHLD, &oldAct, (struct sigaction *)NULL);
    return status;
}


/*
 * PclGetMediumDimensions is installed in the GetMediumDimensions field
 * of each Pcl-initialized context.
 */
int
PclGetMediumDimensions(XpContextPtr pCon,
                       CARD16 *width,
                       CARD16 *height)
{
    XpGetMediumDimensions(pCon, width, height);
    return Success;
}

/*
 * PclGetReproducibleArea is installed in the GetReproducibleArea field
 * of each Pcl-initialized context.
 */
int
PclGetReproducibleArea(XpContextPtr pCon,
                       xRectangle *pRect)
{
    XpGetReproductionArea(pCon, pRect);
    return Success;
}

#ifdef XP_PCL_LJ3
/*
 * PclSpoolFigs spooled the rendering PCL/HP-GL2 commands into the
 * temporary buffer pointed by figures pointer in pcl private context.
 * LaserJet IIIs printers don't support the macro function which
 * includes some HP-GL/2 commands.
 */
void
PclSpoolFigs(PclContextPrivPtr pConPriv, char *t, int n)
{
char *ptr;

    ptr = pConPriv->figures;
    while ( ( pConPriv->fcount + n) > pConPriv->fcount_max ) {
	ptr = (char *)xrealloc(ptr, 1024 + pConPriv->fcount_max);
	if ( !ptr )
	    return;
	pConPriv->figures = ptr;
	pConPriv->fcount_max += 1024;
    }
    ptr += pConPriv->fcount;
    pConPriv->fcount += n;
    memcpy(ptr, t, n);
}
#endif /* XP_PCL_LJ3 */

/*
 * PclSendData:
 * For XP-PCL-COLOR/XP-PCL-MONO, it executes the macro stored before
 * in the clipped area.
 * For XP-PCL-LJ3, it draws the spooled figures in the clipped area.
 */
void
PclSendData(
	FILE *outFile,
	PclContextPrivPtr pConPriv,
	BoxPtr pbox,
	int nbox,
	double ratio
)
{
char *ptr;
int n;
char t[80];

#ifdef XP_PCL_LJ3
    ptr = pConPriv->figures;
    n = pConPriv->fcount;
#else
    ptr = "\033&f3X";
    n = 5;
#endif /* XP_PCL_LJ3 */

    while( nbox )
      {
	  /*
	   * Set the HP-GL/2 input window to the current
	   * rectangle in the clip region, then send the code to
	   * execute the macro defined above.
	   */
	  if (ratio == 1.0)
	    sprintf( t, "\033%%0BIW%d,%d,%d,%d;\033%%0A",
			pbox->x1, pbox->y1,
			pbox->x2, pbox->y2 );
	  else
	    sprintf( t, "\033%%0BIW%g,%d,%g,%d;\033%%0A",
			ratio * pbox->x1, pbox->y1,
			ratio * pbox->x2, pbox->y2 );

	  SEND_PCL( outFile, t );
	  SEND_PCL_COUNT( outFile, ptr, n);

	  nbox--;
	  pbox++;
      }
}
