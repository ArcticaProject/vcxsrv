/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

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

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include <X11/X.h>
#include "scrnintstr.h"
#include "windowstr.h"
#include "cfb.h"
#include "mistruct.h"
#include "regionstr.h"
#include "cfbmskbits.h"

Bool
cfbCreateWindow(WindowPtr pWin)
{
#ifdef PIXMAP_PER_WINDOW
    /* Setup pointer to Screen pixmap */
    dixSetPrivate(&pWin->devPrivates, frameWindowPrivateKey,
		  cfbGetScreenPixmap(pWin->drawable.pScreen));
#endif

    return TRUE;
}

Bool
cfbDestroyWindow(WindowPtr pWin)
{
    return(TRUE);
}

/*ARGSUSED*/
Bool
cfbMapWindow(pWindow)
    WindowPtr pWindow;
{
    return(TRUE);
}

/*ARGSUSED*/
Bool
cfbPositionWindow(WindowPtr pWin, int x, int y)
{
    return (TRUE);
}

/*ARGSUSED*/
Bool
cfbUnmapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   cfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
cfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionRec rgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    REGION_NULL(pWin->drawable.pScreen, &rgnDst);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
    REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if(!nbox || !(pptSrc = (DDXPointPtr )xalloc(nbox * sizeof(DDXPointRec))))
    {
	REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
	return;
    }
    ppt = pptSrc;

    for (i = nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    cfbDoBitbltCopy((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		GXcopy, &rgnDst, pptSrc, ~0L);
    xfree(pptSrc);
    REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

Bool
cfbChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{
    return (TRUE);
}

