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

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"
#include "scrnintstr.h"
#include "dixstruct.h"
#include "cursorstr.h"
#include "dixfontstr.h"
#include "opaque.h"
#include "xace.h"

typedef struct _GlyphShare {
    FontPtr font;
    unsigned short sourceChar;
    unsigned short maskChar;
    CursorBitsPtr bits;
    struct _GlyphShare *next;
} GlyphShare, *GlyphSharePtr;

static GlyphSharePtr sharedGlyphs = (GlyphSharePtr)NULL;

#ifdef XFIXES
static CARD32	cursorSerial;
#endif

static void
FreeCursorBits(CursorBitsPtr bits)
{
    if (--bits->refcnt > 0)
	return;
    xfree(bits->source);
    xfree(bits->mask);
#ifdef ARGB_CURSOR
    xfree(bits->argb);
#endif
    if (bits->refcnt == 0)
    {
	GlyphSharePtr *prev, this;

	for (prev = &sharedGlyphs;
	     (this = *prev) && (this->bits != bits);
	     prev = &this->next)
	    ;
	if (this)
	{
	    *prev = this->next;
	    CloseFont(this->font, (Font)0);
	    xfree(this);
	}
	dixFreePrivates(bits->devPrivates);
	xfree(bits);
    }
}

/**
 * To be called indirectly by DeleteResource; must use exactly two args.
 *
 *  \param value must conform to DeleteType
 */
_X_EXPORT int
FreeCursor(pointer value, XID cid)
{
    int		nscr;
    CursorPtr 	pCurs = (CursorPtr)value;

    ScreenPtr	pscr;

    if ( --pCurs->refcnt > 0)
	return(Success);

    for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
    {
	pscr = screenInfo.screens[nscr];
	(void)( *pscr->UnrealizeCursor)( pscr, pCurs);
    }
    dixFreePrivates(pCurs->devPrivates);
    FreeCursorBits(pCurs->bits);
    xfree( pCurs);
    return(Success);
}


/*
 * We check for empty cursors so that we won't have to display them
 */
static void
CheckForEmptyMask(CursorBitsPtr bits)
{
    unsigned char *msk = bits->mask;
    int n = BitmapBytePad(bits->width) * bits->height;

    bits->emptyMask = FALSE;
    while(n--) 
	if(*(msk++) != 0) return;
#ifdef ARGB_CURSOR
    if (bits->argb)
    {
	CARD32 *argb = bits->argb;
	int n = bits->width * bits->height;
	while (n--)
	    if (*argb++ & 0xff000000) return;
    }
#endif
    bits->emptyMask = TRUE;
}

/**
 * does nothing about the resource table, just creates the data structure.
 * does not copy the src and mask bits
 *
 *  \param psrcbits  server-defined padding
 *  \param pmaskbits server-defined padding
 *  \param argb      no padding
 */
int
AllocARGBCursor(unsigned char *psrcbits, unsigned char *pmaskbits,
		CARD32 *argb, CursorMetricPtr cm,
		unsigned foreRed, unsigned foreGreen, unsigned foreBlue, 
		unsigned backRed, unsigned backGreen, unsigned backBlue,
		CursorPtr *ppCurs, ClientPtr client, XID cid)
{
    CursorBitsPtr  bits;
    CursorPtr 	pCurs;
    int		rc, nscr;
    ScreenPtr 	pscr;

    *ppCurs = NULL;
    pCurs = (CursorPtr)xalloc(sizeof(CursorRec) + sizeof(CursorBits));
    if (!pCurs)
    {
	xfree(psrcbits);
	xfree(pmaskbits);
	return BadAlloc;
    }
    bits = (CursorBitsPtr)((char *)pCurs + sizeof(CursorRec));
    bits->source = psrcbits;
    bits->mask = pmaskbits;
#ifdef ARGB_CURSOR
    bits->argb = argb;
#endif
    bits->width = cm->width;
    bits->height = cm->height;
    bits->xhot = cm->xhot;
    bits->yhot = cm->yhot;
    bits->devPrivates = NULL;
    bits->refcnt = -1;
    CheckForEmptyMask(bits);
    pCurs->bits = bits;
    pCurs->refcnt = 1;		
#ifdef XFIXES
    pCurs->serialNumber = ++cursorSerial;
    pCurs->name = None;
#endif

    pCurs->foreRed = foreRed;
    pCurs->foreGreen = foreGreen;
    pCurs->foreBlue = foreBlue;

    pCurs->backRed = backRed;
    pCurs->backGreen = backGreen;
    pCurs->backBlue = backBlue;

    pCurs->id = cid;
    pCurs->devPrivates = NULL;

    /* security creation/labeling check */
    rc = XaceHook(XACE_RESOURCE_ACCESS, client, cid, RT_CURSOR,
		  pCurs, RT_NONE, NULL, DixCreateAccess);
    if (rc != Success) {
	dixFreePrivates(pCurs->devPrivates);
	FreeCursorBits(bits);
	xfree(pCurs);
	return rc;
    }
	
    /*
     * realize the cursor for every screen
     */
    for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
    {
	pscr = screenInfo.screens[nscr];
        if (!( *pscr->RealizeCursor)( pscr, pCurs))
	{
	    while (--nscr >= 0)
	    {
		pscr = screenInfo.screens[nscr];
		( *pscr->UnrealizeCursor)( pscr, pCurs);
	    }
	    dixFreePrivates(pCurs->devPrivates);
	    FreeCursorBits(bits);
	    xfree(pCurs);
	    return BadAlloc;
	}
    }
    *ppCurs = pCurs;
    return rc;
}

int
AllocGlyphCursor(Font source, unsigned sourceChar, Font mask, unsigned maskChar,
                unsigned foreRed, unsigned foreGreen, unsigned foreBlue, 
                unsigned backRed, unsigned backGreen, unsigned backBlue,
		CursorPtr *ppCurs, ClientPtr client, XID cid)
{
    FontPtr  sourcefont, maskfont;
    unsigned char   *srcbits;
    unsigned char   *mskbits;
    CursorMetricRec cm;
    int rc;
    CursorBitsPtr  bits;
    CursorPtr 	pCurs;
    int		nscr;
    ScreenPtr 	pscr;
    GlyphSharePtr pShare;

    rc = dixLookupResource((pointer *)&sourcefont, source, RT_FONT, client,
			   DixUseAccess);
    if (rc != Success)
    {
	client->errorValue = source;
	return (rc == BadValue) ? BadFont : rc;
    }
    rc = dixLookupResource((pointer *)&maskfont, mask, RT_FONT, client,
			   DixUseAccess);
    if (rc != Success && mask != None)
    {
	client->errorValue = mask;
	return (rc == BadValue) ? BadFont : rc;
    }
    if (sourcefont != maskfont)
	pShare = (GlyphSharePtr)NULL;
    else
    {
	for (pShare = sharedGlyphs;
	     pShare &&
	     ((pShare->font != sourcefont) ||
	      (pShare->sourceChar != sourceChar) ||
	      (pShare->maskChar != maskChar));
	     pShare = pShare->next)
	    ;
    }
    if (pShare)
    {
	pCurs = (CursorPtr)xalloc(sizeof(CursorRec));
	if (!pCurs)
	    return BadAlloc;
	bits = pShare->bits;
	bits->refcnt++;
    }
    else
    {
	if (!CursorMetricsFromGlyph(sourcefont, sourceChar, &cm))
	{
	    client->errorValue = sourceChar;
	    return BadValue;
	}
	if (!maskfont)
	{
	    long n;
	    unsigned char *mskptr;

	    n = BitmapBytePad(cm.width)*(long)cm.height;
	    mskptr = mskbits = (unsigned char *)xalloc(n);
	    if (!mskptr)
		return BadAlloc;
	    while (--n >= 0)
		*mskptr++ = ~0;
	}
	else
	{
	    if (!CursorMetricsFromGlyph(maskfont, maskChar, &cm))
	    {
		client->errorValue = maskChar;
		return BadValue;
	    }
	    if ((rc = ServerBitsFromGlyph(maskfont, maskChar, &cm, &mskbits)))
		return rc;
	}
	if ((rc = ServerBitsFromGlyph(sourcefont, sourceChar, &cm, &srcbits)))
	{
	    xfree(mskbits);
	    return rc;
	}
	if (sourcefont != maskfont)
	{
	    pCurs = (CursorPtr)xalloc(sizeof(CursorRec) + sizeof(CursorBits));
	    if (pCurs)
		bits = (CursorBitsPtr)((char *)pCurs + sizeof(CursorRec));
	    else
		bits = (CursorBitsPtr)NULL;
	}
	else
	{
	    pCurs = (CursorPtr)xalloc(sizeof(CursorRec));
	    if (pCurs)
		bits = (CursorBitsPtr)xalloc(sizeof(CursorBits));
	    else
		bits = (CursorBitsPtr)NULL;
	}
	if (!bits)
	{
	    xfree(pCurs);
	    xfree(mskbits);
	    xfree(srcbits);
	    return BadAlloc;
	}
	bits->source = srcbits;
	bits->mask = mskbits;
#ifdef ARGB_CURSOR
	bits->argb = 0;
#endif
	bits->width = cm.width;
	bits->height = cm.height;
	bits->xhot = cm.xhot;
	bits->yhot = cm.yhot;
	bits->devPrivates = NULL;
	if (sourcefont != maskfont)
	    bits->refcnt = -1;
	else
	{
	    bits->refcnt = 1;
	    pShare = (GlyphSharePtr)xalloc(sizeof(GlyphShare));
	    if (!pShare)
	    {
		FreeCursorBits(bits);
		return BadAlloc;
	    }
	    pShare->font = sourcefont;
	    sourcefont->refcnt++;
	    pShare->sourceChar = sourceChar;
	    pShare->maskChar = maskChar;
	    pShare->bits = bits;
	    pShare->next = sharedGlyphs;
	    sharedGlyphs = pShare;
	}
    }
    CheckForEmptyMask(bits);
    pCurs->bits = bits;
    pCurs->refcnt = 1;
#ifdef XFIXES
    pCurs->serialNumber = ++cursorSerial;
    pCurs->name = None;
#endif

    pCurs->foreRed = foreRed;
    pCurs->foreGreen = foreGreen;
    pCurs->foreBlue = foreBlue;

    pCurs->backRed = backRed;
    pCurs->backGreen = backGreen;
    pCurs->backBlue = backBlue;

    pCurs->id = cid;
    pCurs->devPrivates = NULL;

    /* security creation/labeling check */
    rc = XaceHook(XACE_RESOURCE_ACCESS, client, cid, RT_CURSOR,
		  pCurs, RT_NONE, NULL, DixCreateAccess);
    if (rc != Success) {
	dixFreePrivates(pCurs->devPrivates);
	FreeCursorBits(bits);
	xfree(pCurs);
	return rc;
    }
	
    /*
     * realize the cursor for every screen
     */
    for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
    {
	pscr = screenInfo.screens[nscr];
        if (!( *pscr->RealizeCursor)( pscr, pCurs))
	{
	    while (--nscr >= 0)
	    {
		pscr = screenInfo.screens[nscr];
		( *pscr->UnrealizeCursor)( pscr, pCurs);
	    }
	    dixFreePrivates(pCurs->devPrivates);
	    FreeCursorBits(pCurs->bits);
	    xfree(pCurs);
	    return BadAlloc;
	}
    }
    *ppCurs = pCurs;
    return Success;
}

/** CreateRootCursor
 *
 * look up the name of a font
 * open the font
 * add the font to the resource table
 * make a cursor from the glyphs
 * add the cursor to the resource table
 *************************************************************/

CursorPtr 
CreateRootCursor(char *unused1, unsigned int unused2)
{
    CursorPtr 	curs;
#ifdef NULL_ROOT_CURSOR
    CursorMetricRec cm;
#else
    FontPtr 	cursorfont;
    int	err;
    XID		fontID;
#endif

#ifdef NULL_ROOT_CURSOR
    cm.width = 0;
    cm.height = 0;
    cm.xhot = 0;
    cm.yhot = 0;

    AllocARGBCursor(NULL, NULL, NULL, &cm, 0, 0, 0, 0, 0, 0,
		    &curs, serverClient, (XID)0);

    if (curs == NullCursor)
        return NullCursor;
#else
    fontID = FakeClientID(0);
    err = OpenFont(serverClient, fontID, FontLoadAll | FontOpenSync,
	(unsigned)strlen(defaultCursorFont), defaultCursorFont);
    if (err != Success)
	return NullCursor;

    cursorfont = (FontPtr)LookupIDByType(fontID, RT_FONT);
    if (!cursorfont)
	return NullCursor;
    if (AllocGlyphCursor(fontID, 0, fontID, 1, 0, 0, 0, ~0, ~0, ~0,
			 &curs, serverClient, (XID)0) != Success)
	return NullCursor;
#endif

    if (!AddResource(FakeClientID(0), RT_CURSOR, (pointer)curs))
	return NullCursor;

    return curs;
}
