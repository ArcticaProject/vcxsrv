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

#include    <X11/X.h>
#include    "scrnintstr.h"
#include    "validate.h"
#include    "windowstr.h"
#include    "mi.h"
#include    "regionstr.h"
#include    "mivalidate.h"

/*
 * XpValidateTree - a validateTree routine which ignores overlapping
 * top-level windows when computing the clip lists for such windows.
 * This can be used by any driver which maintains a separate memory
 * store for each top-level window (with its respective children).
 * If the pParent is not the root window, then miValidateTree 
 * is used unmodified.
 *
 * The strategy if pParent is the root is to save off the 
 * current values of pParent->firstChild and pParent->lastChild,
 * replacing both with the single child of interest. We save off
 * pChild->prevSib and pChild->nextSib, and replace them with NullWindow.
 * We save off pParent->clipList, and replace it with 
 * pParent->winSize - pChild->winSize.  We then call miValidateTree
 * to do the needed ComputeClips on the pChild's heirarchy.
 * pParent's clipList is then recomputed based on the sizes
 * of its children, and the saved pointers are restored.
 * The trees associated with the siblings of pChild must be descended
 * and cleaned of any marks (i.e. the valdata pointer freed, and set to NULL),
 * and pParent' AfterValidate structure's exposed field must be updated.
 */
/*ARGSUSED*/
int
XpValidateTree (pParent, pChild, kind)
    WindowPtr	  	pParent;    /* Parent to validate */
    WindowPtr	  	pChild;     /* First child of pParent that was
				     * affected */
    VTKind    	  	kind;       /* What kind of configuration caused call */
{
    RegionRec	  	origPrntClip;  /* orig clipList for parent */
    RegionRec	  	childClip;  /* The new borderClip for the current
				     * child */
    RegionRec		tmpPrntClip; /* parent clipList - child borderClip */
    RegionRec		exposed;    /* For intermediate calculations */
    register ScreenPtr	pScreen;
    register WindowPtr	pWin;
    Bool		overlap;
    int			viewvals;
    Bool		forward;

    WindowPtr	origFirstChild, origLastChild, origPrevSib, origNextSib;

    /*
     * If we're validating something other than a top-level window,
     * then just invoke miValidateTree.
     */
    if(pParent->parent != NullWindow)
	return miValidateTree(pParent, pChild, kind);

    /*
     * If it's a stack change of top levels then it's a no-op for
     * this scheme, so we just clean up any marks on windows and return.
     */
    if(kind == VTStack)
    {
	CleanMarks(pParent);
	return 1;
    }

    pScreen = pParent->drawable.pScreen;
    if (pChild == NullWindow)
	pChild = pParent->firstChild;
    
    /* Save off the existing window heirarchy */
    origFirstChild = pParent->firstChild;
    origLastChild = pParent->lastChild;
    origPrevSib = pChild->prevSib;
    origNextSib = pChild->nextSib;
    pParent->firstChild = pChild;
    pParent->lastChild = pChild;
    pChild->prevSib = NullWindow;
    pChild->nextSib = NullWindow;

    /*
     * Set pParent's clipList to be its winSize minus the pChild's
     * borderSize.
     */
    origPrntClip = pParent->clipList;
    REGION_NULL(pScreen, &tmpPrntClip);
    REGION_SUBRACT(pScreen, &tmpPrntClip, &pParent->winSize,
		   &pChild->borderSize);
    pParent->clipList = tmpPrntClip;

    /*
     * Call miValidateTree on the pruned tree.
     */
    (void) miValidateTree(pParent, pChild, kind);

    /* Restore the saved heirarchy */
    pChild->prevSib = origPrevSib;
    pChild->nextSib = origNextSib;
    pParent->firstChild = origFirstChild;
    pParent->lastChild = origLastChild;

    /*
     * Compute pParent's clipList by taking its winSize and subracting
     * the borderSize of each of its children.
     */
    for(pWin = pParent->firstChild, 
	REGION_COPY(pScreen, &pParent->clipList, &pParent->winSize); 
	pWin != NullWindow; 
	pWin = pWin->nextSib)
    {
	REGION_SUBTRACT(pScreen, &pParent->clipList, &pParent->clipList, 
			&pWin->borderSize);
    }

    /*
     * Compute pParent's AfterValidate structure by subracting the original
     * clipList from the newly computed clipList.
     */
    REGION_NULL(pScreen, &pParent->valdata->after.exposed);
    REGION_SUBTRACT(pScreen, &pParent->valdata->after.exposed, 
		    &pParent->clipList, &origPrntClip);

    /*
     * Remove the marks from all but pParent and pChild's heirarchy.
     * i.e. from all of pChild's siblings and their children.
     */
    for(pWin = pParent->firstChild; pWin != NullWindow; pWin = pWin->nextSib)
    {
	WindowPtr pCurChild = pWin;

	if(pCurChild == pChild)
	    continue;

        while (1)
        {
	    if(pCurChild->valdata)
	    {
		xfree(pCurChild->valdata);
		pCurChild->valdata = (ValidatePtr)NULL;
	    }

            if (pCurChild->firstChild)
            {
                pCurChild = pCurChild->firstChild;
                continue;
            }
            while (!pCurChild->nextSib && (pCurChild != pWin))
                pCurChild = pCurChild->parent;
            if (pCurChild == pWin)
                break;
            pCurChild = pCurChild->nextSib;
        }
    }
}
