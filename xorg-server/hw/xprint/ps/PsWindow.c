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
**    *  File:		PsWindow.c
**    *
**    *  Contents:	Window code for PS driver.
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

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "mistruct.h"
#include "regionstr.h"
#include "windowstr.h"
#include "gcstruct.h"

#include "Ps.h"

/*
 * The following list of strings defines the properties which will be
 * placed on the screen's root window if the property was defined in
 * the start-up configuration resource database.
 */
#if 0
static char *propStrings[] = {
	DT_PRINT_JOB_HEADER,
	DT_PRINT_JOB_TRAILER,
	DT_PRINT_JOB_COMMAND,
	DT_PRINT_JOB_EXEC_COMMAND,
	DT_PRINT_JOB_EXEC_OPTIONS,
	DT_PRINT_PAGE_HEADER,
	DT_PRINT_PAGE_TRAILER,
	DT_PRINT_PAGE_COMMAND,
	(char *)NULL
};
#endif

/*
 * PsCreateWindow - watch for the creation of the root window.
 * When it's created, register the screen with the print extension,
 * and put the default command/header properties on it.
 */
/*ARGSUSED*/

Bool
PsCreateWindow(WindowPtr pWin)
{
  PsWindowPrivPtr pPriv;

#if 0
    Bool status = Success;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    PsScreenPrivPtr pScreenPriv = (PsScreenPrivPtr) 
	dixLookupPrivate(&pScreen->devPrivates, PsScreenPrivateKey);
    PsWindowPrivPtr pWinPriv = (PsWindowPrivPtr)
	dixLookupPrivate(&pWin->devPrivates, PsWindowPrivateKey);

    /*
     * Initialize this window's private struct.
     */
    pWinPriv->jobFileName = (char *)NULL;
    pWinPriv->pJobFile = (FILE *)NULL;
    pWinPriv->pageFileName = (char *)NULL;
    pWinPriv->pPageFile = (FILE *)NULL;
    
    if(pWin->parent == (WindowPtr)NULL)  /* root window? */
    {
	Atom propName; /* type = XA_STRING */
	char *propVal;
	int i;
        XrmDatabase rmdb = pScreenPriv->resDB;

        /*
         * Put the defaults spec'd in the config files in properties on this
	 * screen's root window.
         */
	for(i = 0; propStrings[i] != (char *)NULL; i++)
	{
            if((propVal = _DtPrintGetPrinterResource(pWin, rmdb, 
						     propStrings[i])) !=
	       (char *)NULL)
	    {
                propName = MakeAtom(propStrings[i], strlen(propStrings[i]),
				    TRUE);
	        dixChangeWindowProperty(serverClient, pWin, propName, XA_STRING,
					8, PropModeReplace, strlen(propVal),
					(pointer)propVal, FALSE);
	        xfree(propVal);
	    }
	}
    }

    return status;
#endif

  pPriv = (PsWindowPrivPtr)
      dixLookupPrivate(&pWin->devPrivates, PsWindowPrivateKey);
  pPriv->validContext = 0;

  return TRUE;
}


/*ARGSUSED*/
Bool PsMapWindow(WindowPtr pWindow)
{
  return TRUE;
}

/*ARGSUSED*/
Bool 
PsPositionWindow(
  WindowPtr pWin,
  int x,
  int y)
{
  return TRUE;
}

/*ARGSUSED*/
Bool 
PsUnmapWindow(WindowPtr pWindow)
{
  return TRUE;
}

/*ARGSUSED*/
void 
PsCopyWindow(
  WindowPtr   pWin,
  DDXPointRec ptOldOrg,
  RegionPtr   prgnSrc)
{
}

/*ARGSUSED*/
Bool
PsChangeWindowAttributes(
  WindowPtr     pWin,
  unsigned long mask)
{
  return TRUE;
}

/*ARGSUSED*/
Bool
PsDestroyWindow(WindowPtr pWin)
{
  return TRUE;
}
