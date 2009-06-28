/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclText.c
**    *
**    *  Contents:
**    *                 Character-drawing routines for the PCL DDX
**    *
**    *  Created:	10/23/95
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

#ifdef DO_TWO_BYTE_PCL
#include "iconv.h"
#endif /* DO_TWO_BYTE_PCL */
#include "gcstruct.h"
#include "windowstr.h"

#include "Pcl.h"
#include "migc.h"
#include <X11/Xatom.h>

#include "PclSFonts.h"

static PclFontHead8Ptr  makeFontHeader8 (FontPtr, PclSoftFontInfoPtr);
static PclFontHead16Ptr makeFontHeader16(FontPtr, PclSoftFontInfoPtr);
static PclInternalFontPtr makeInternalFont(FontPtr, PclSoftFontInfoPtr);
static void             fillFontDescData(FontPtr, PclFontDescPtr, unsigned int);
static PclCharDataPtr   fillCharDescData(PclCharDataPtr, CharInfoPtr);
static void             output_text(FILE *, PclContextPrivPtr, unsigned char);
static char *           getFontName(FontPtr);
static char             isInternal(FontPtr);
static void             selectInternalFont(FILE *, PclInternalFontPtr, int);
static void             selectSize(FILE *, PclContextPrivPtr, PclInternalFontPtr);
static char t[80];

#ifdef DO_TWO_BYTE_PCL
static void             code_conv(PclSoftFontInfoPtr, FontPtr, char *, char *);
#endif /* DO_TWO_BYTE_PCL */

#define ESC 0x1b
#define PER 0x25
#define ETX 0x3
#define ETX_ALT 0x2a
#define DOWNLOAD_FONT 0
#define INTERNAL_FONT 1

int
PclPolyText8(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int x,
     int y,
     int count,
     char *string)
{
XpContextPtr pCon;
PclContextPrivPtr pConPriv;
unsigned long n, i;
int w;
CharInfoPtr charinfo[255], *chinfo;

FILE *outFile;
PclSoftFontInfoPtr pSoftFontInfo;
PclFontHead8Ptr pfh8 = (PclFontHead8Ptr)NULL;
PclInternalFontPtr pin = (PclInternalFontPtr)NULL;
PclCharDataRec cd;
unsigned char *p;
unsigned char last_fid;
int max_ascent, max_descent;

int nbox;
BoxPtr pbox;
BoxRec box;
RegionPtr drawRegion, region;
char font_type;

    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
	return x;

    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)string,
						Linear8Bit, &n, charinfo);
    if ( n == 0 )
	return x;

    pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    pSoftFontInfo = pConPriv->pSoftFontInfo;
    font_type = isInternal(pGC->font);
    if ( font_type == DOWNLOAD_FONT ) {
	/*
 	 * Create Soft Font Header Information
 	 */
	pfh8 = makeFontHeader8(pGC->font, pSoftFontInfo);
	if (!pfh8)
	    return x;

	/*
	 * exec Soft Font Downloading
	 */
	p = (unsigned char *)string;
	for (i=0, chinfo=charinfo; i<n; i++, p++, chinfo++) {
	    if ( !pfh8->index[*p] ) {
		fillCharDescData(&cd, *chinfo);
        	PclDownloadSoftFont8(pConPriv->pJobFile, pSoftFontInfo,
					pfh8, &cd, p);
        	xfree(cd.raster_top);
	    }
	}

	/*
	 * print characters
	 */
	MACRO_START( outFile, pConPriv );
	sprintf(t, "\033%%0B;PU%d,%dPD;TD1;DT%c,1;",
                x + pDrawable->x, y + pDrawable->y + pGC->font->info.fontAscent,
		ETX);
	SAVE_PCL( outFile, pConPriv, t );
	SAVE_PCL_COUNT( outFile, pConPriv, "FI0;SS;LB", 9 );

	last_fid = 0;
	w = 0;
	max_ascent = charinfo[0]->metrics.ascent;
	max_descent = charinfo[0]->metrics.descent;
	p = (unsigned char *)string;
	for (i=0, chinfo=charinfo; i<n; i++, p++, chinfo++) {
	    if  ( last_fid != pfh8->fid ) {
		sprintf(t, "%c;FI%d;SS;LB", ETX, pfh8->fid);
		SAVE_PCL( outFile, pConPriv, t );

		last_fid = pfh8->fid;
	    }

	    output_text(outFile, pConPriv, pfh8->index[*p]);

	    w += (*chinfo)->metrics.characterWidth;
	    max_ascent = MAX(max_ascent, (*chinfo)->metrics.ascent);
	    max_descent = MAX(max_descent, (*chinfo)->metrics.descent);
	}

	sprintf(t, "%c", ETX);
	SAVE_PCL_COUNT( outFile, pConPriv, t, 1 );
	sprintf(t, "TD0;\033%%1A");
	SAVE_PCL( outFile, pConPriv, t );
	MACRO_END( outFile );

    } else {
	int fid = 0;

	pin = makeInternalFont(pGC->font, pSoftFontInfo);
	if (!pin)
	    return x;

	selectInternalFont(outFile, pin, fid);

	/*
	 * print characters
	 */
	MACRO_START( outFile, pConPriv );
	sprintf(t, "\033%%0B;PU%d,%dPD;TD1;DT%c,1;",
		x + pDrawable->x, y + pDrawable->y + pGC->font->info.fontAscent,
		ETX);
	SAVE_PCL( outFile, pConPriv, t );
	selectSize(outFile, pConPriv, pin);
	SAVE_PCL_COUNT( outFile, pConPriv, "FI0;SS;LB", 9 );

	w = 0;
	max_ascent = charinfo[0]->metrics.ascent;
	max_descent = charinfo[0]->metrics.descent;
	p = (unsigned char *)string;
	for (i=0, chinfo=charinfo; i<n; i++, p++, chinfo++) {
	    output_text(outFile, pConPriv, *p);

	    w += (*chinfo)->metrics.characterWidth;
	    max_ascent = MAX(max_ascent, (*chinfo)->metrics.ascent);
	    max_descent = MAX(max_descent, (*chinfo)->metrics.descent);
	}
	sprintf(t, "%c", ETX);
	SAVE_PCL_COUNT( outFile, pConPriv, t, 1 );
	sprintf(t, "TD0;\033%%1A");
	SAVE_PCL( outFile, pConPriv, t );
	MACRO_END( outFile );
    }

    /*
     * Convert the collection of rectangles into a proper region, then
     * intersect it with the clip region.
     */
    box.x1 = x +  pDrawable->x;
    box.y1 = y - max_ascent + pDrawable->y + pGC->font->info.fontAscent;
    box.x2 = x + w + pDrawable->x;
    box.y2 = y + max_descent + pDrawable->y + pGC->font->info.fontAscent;

    drawRegion = miRegionCreate( &box, 0 );
    region = miRegionCreate( NULL, 0 );
    miIntersect( region, drawRegion, pGC->pCompositeClip );

    /*
     * For each rectangle in the clip region, set the HP-GL/2 "input
     * window" and render the entire polyline to it.
     */
    pbox = REGION_RECTS( region );
    nbox = REGION_NUM_RECTS( region );

    PclSendData(outFile, pConPriv, pbox, nbox, 1.0);

    /*
     * Clean up the temporary regions
     */
    REGION_DESTROY( pGC->pScreen, drawRegion );
    REGION_DESTROY( pGC->pScreen, region );

    return x+w;
}

int
PclPolyText16(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int x,
     int y,
     int count,
     unsigned short *string)
{
XpContextPtr pCon;
PclContextPrivPtr pConPriv;
unsigned long n, i;
int w;
CharInfoPtr charinfo[255], *chinfo;

FILE *outFile;
PclSoftFontInfoPtr pSoftFontInfo;
PclFontHead16Ptr pfh16 = (PclFontHead16Ptr)NULL;
PclCharDataRec cd;
FontInfoPtr pfi;
unsigned char row, col;
char *p;
unsigned char last_fid;
int max_ascent, max_descent;
unsigned short def;

int nbox;
BoxPtr pbox;
BoxRec box;
RegionPtr drawRegion, region;
char font_type;

    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
	return x;

    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)string,
		(FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
		&n, charinfo);

    pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    pSoftFontInfo = pConPriv->pSoftFontInfo;

    font_type = isInternal(pGC->font);
    if ( font_type == DOWNLOAD_FONT ) {
	/*
	 * Create Soft Font Header Information
	 */
	pfh16 = makeFontHeader16(pGC->font, pSoftFontInfo);
	if (!pfh16)
	    return x;

	/*
	 * exec Soft Font Downloading
	 */
	pfi = (FontInfoRec *)&pGC->font->info;
	p = (char *)string;
	for (i=0, p=(char *)string, chinfo=charinfo; i<n; i++, p+=2, chinfo++) {
	    row = *p & 0xff;
	    col = *(p+1) & 0xff;
	    if ( (pfi->firstRow <= row) && (row <= pfi->lastRow)
		&& (pfi->firstCol <= col) && (col <= pfi->lastCol) ) {
		row = row - pfi->firstRow;
		col = col - pfi->firstCol;
	    } else {
		def = pfi->defaultCh;
		row = ((def>>8)&0xff) - pfi->firstRow;
		col = (def&0xff) - pfi->firstCol;
	    }
	    if ( !pfh16->index[row][col].fid ) {
		fillCharDescData(&cd, *chinfo);
		PclDownloadSoftFont16(pConPriv->pJobFile, pSoftFontInfo,
				pfh16, &cd, row, col);
		xfree(cd.raster_top);
	    }
	}

	/*
	 * print characters
	 */
	MACRO_START( outFile, pConPriv );
	sprintf(t, "\033%%0B;PU%d,%dPD;TD1;DT%c,1;",
		x + pDrawable->x, y + pDrawable->y + pGC->font->info.fontAscent,
		ETX);
	SAVE_PCL( outFile, pConPriv, t );
	SAVE_PCL_COUNT( outFile, pConPriv, "FI0;SS;LB", 9 );

	last_fid = 0;

	w = 0;
	max_ascent = charinfo[0]->metrics.ascent;
	max_descent = charinfo[0]->metrics.descent;
	for (i=0, p=(char *)string, chinfo=charinfo; i<n; i++, p+=2, chinfo++) {
	    row = *p & 0xff;
	    col = *(p+1) & 0xff;
	    if ( (pfi->firstRow <= row) && (row <= pfi->lastRow)
		&& (pfi->firstCol <= col) && (col <= pfi->lastCol) ) {
		row = row - pfi->firstRow;
		col = col - pfi->firstCol;
	    } else {
		def = pfi->defaultCh;
		row = ((def>>8)&0xff) - pfi->firstRow;
		col = (def&0xff) - pfi->firstCol;
	    }
	    if ( last_fid != pfh16->index[row][col].fid ) {
		sprintf(t, "%cFI%d;SS;LB",
				ETX, pfh16->index[row][col].fid);
		SAVE_PCL( outFile, pConPriv, t );
		last_fid = pfh16->index[row][col].fid;
	    }

	    output_text(outFile, pConPriv, pfh16->index[row][col].cindex);

	    w += (*chinfo)->metrics.characterWidth;
	    max_ascent = MAX(max_ascent, (*chinfo)->metrics.ascent);
	    max_descent = MAX(max_descent, (*chinfo)->metrics.descent);
	}
	sprintf(t, "%c", ETX);
	SAVE_PCL_COUNT( outFile, pConPriv, t, 1 );
	sprintf(t, "TD0;\033%%1A");
	SAVE_PCL( outFile, pConPriv, t );
	MACRO_END( outFile );

    } else {
#ifdef DO_TWO_BYTE_PCL
	PclInternalFontPtr pin;
	int fid = 0;

	pin = makeInternalFont(pGC->font, pSoftFontInfo);
	if (!pin)
	    return x;

	selectInternalFont(outFile, pin, fid);
	fprintf(outFile, "%c&t31P", ESC);

	/*
	 * print characters
	 */
	MACRO_START( outFile, pConPriv );
	sprintf(t, "\033%%0B;PU%d,%dPD;TD1;DT%c,1;",
		x + pDrawable->x, y + pDrawable->y + pGC->font->info.fontAscent,
		ETX);
	SAVE_PCL( outFile, pConPriv, t );
	sprintf(t, "TD0;\033%%1A");
	SAVE_PCL( outFile, pConPriv, t );

	w = 0;
	last_fid = 0;
	max_ascent = charinfo[0]->metrics.ascent;
	max_descent = charinfo[0]->metrics.descent;
	for (i=0, p=(char *)string, chinfo=charinfo; i<n; i++, p+=2, chinfo++) {
	    char tobuf[3];
	    code_conv(pSoftFontInfo, pGC->font, (char *)p, tobuf);
	    fprintf(outFile, "%c%c", tobuf[0], tobuf[1]);

	    w += (*chinfo)->metrics.characterWidth;
	    max_ascent = MAX(max_ascent, (*chinfo)->metrics.ascent);
	    max_descent = MAX(max_descent, (*chinfo)->metrics.descent);
	}
	MACRO_END( outFile );
#else
	return x;
#endif /* DO_TWO_BYTE_PCL */
    }

    /*
     * Convert the collection of rectangles into a proper region, then
     * intersect it with the clip region.
     */
    box.x1 = x + pDrawable->x;
    box.y1 = y - max_ascent + pDrawable->y + pGC->font->info.fontAscent;
    box.x2 = x + w + pDrawable->x;
    box.y2 = y + max_descent + pDrawable->y + pGC->font->info.fontAscent;

    drawRegion = miRegionCreate( &box, 0 );
    region = miRegionCreate( NULL, 0 );
    miIntersect( region, drawRegion, pGC->pCompositeClip );

    /*
     * For each rectangle in the clip region, set the HP-GL/2 "input
     * window" and render the entire polyline to it.
     */
    pbox = REGION_RECTS( region );
    nbox = REGION_NUM_RECTS( region );

    PclSendData(outFile, pConPriv, pbox, nbox, 1.0);

    /*
     * Clean up the temporary regions
     */
    REGION_DESTROY( pGC->pScreen, drawRegion );
    REGION_DESTROY( pGC->pScreen, region );

    return x+w;
}

void
PclImageText8(
     DrawablePtr pDrawable,
     GCPtr pGC, 
     int x, int y,
     int count,
     char *string)
{
}

void
PclImageText16(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int x,
     int y,
     int count,
     unsigned short *string)
{
}

void
PclImageGlyphBlt(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int x, int y,
     unsigned int nGlyphs,
     CharInfoPtr *pCharInfo,
     pointer pGlyphBase)
{
}

void
PclPolyGlyphBlt(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int x, int y,
     unsigned int nGlyphs,
     CharInfoPtr *pCharInfo,
     pointer pGlyphBase)
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PclFontHead8Ptr
makeFontHeader8(FontPtr pfont, PclSoftFontInfoPtr pSoftFontInfo)
{
PclFontHead8Ptr phead8 = pSoftFontInfo->phead8;
PclFontHead8Ptr pfh8 = phead8;
PclFontHead8Ptr prev = (PclFontHead8Ptr)NULL;
FontInfoPtr pfi;
char *fontname;
unsigned char nindex;
int i;
unsigned long n;
CharInfoPtr charinfo[1];
unsigned int space_width;

    if (pSoftFontInfo == (PclSoftFontInfoPtr) NULL)
	return (PclFontHead8Ptr)NULL;

    /*
     * Verify it has already been created, if so, return it.
     */
    if ( (fontname = getFontName(pfont)) == (char *)NULL)
	return (PclFontHead8Ptr)NULL;

    while (pfh8 != (PclFontHead8Ptr) NULL) {
	if (!strcmp(pfh8->fontname, fontname))
	    return pfh8;
	prev = pfh8;
	pfh8 = pfh8->next;
    }

    /*
     * Create Font Header Information
     */
    pfh8 = (PclFontHead8Ptr)xalloc(sizeof(PclFontHead8Rec));
    if (pfh8 == (PclFontHead8Ptr)NULL)
	return (PclFontHead8Ptr)NULL;

    pfi = (FontInfoRec *)&pfont->info;
    GetGlyphs(pfont, 1, (unsigned char *)&pfi->defaultCh,
						Linear8Bit, &n, charinfo);
    if ( n )
	space_width = charinfo[0]->metrics.characterWidth;
    else
	space_width = FONTMAXBOUNDS(pfont,characterWidth);

    fillFontDescData(pfont, &(pfh8->fd), space_width);
    pfh8->fid = 0;
    pfh8->fontname = (char *)xalloc(strlen(fontname) + 1);
    if (pfh8->fontname == (char *)NULL) {
	xfree(pfh8);
	return (PclFontHead8Ptr) NULL;
    }
    strcpy(pfh8->fontname, fontname);

    nindex = 0xff;
    pfh8->index = (unsigned char *)xalloc(nindex);
    if ( pfh8->index == (unsigned char *) NULL ) {
	xfree(pfh8->fontname);
	xfree(pfh8);
	return (PclFontHead8Ptr) NULL;
    }

    for (i=0; i<=nindex; i++)
        pfh8->index[i] = 0x0;

    pfh8->next = (PclFontHead8Ptr)NULL;

    if ( prev == (PclFontHead8Ptr) NULL)
	pSoftFontInfo->phead8 = pfh8;
    else
	prev->next = pfh8;

    return pfh8;
}

static PclFontHead16Ptr
makeFontHeader16(FontPtr pfont, PclSoftFontInfoPtr pSoftFontInfo)
{
PclFontHead16Ptr phead16 = pSoftFontInfo->phead16;
PclFontHead16Ptr pfh16 = phead16;
PclFontHead16Ptr prev = (PclFontHead16Ptr)NULL;
PclFontMapRec ** index;
FontInfoPtr pfi;
char *fontname;
unsigned char nindex_row, nindex_col;
int i, j;
unsigned long n;
CharInfoPtr charinfo[1];
unsigned int space_width;

    if (pSoftFontInfo == (PclSoftFontInfoPtr) NULL)
	return (PclFontHead16Ptr)NULL;

    /*
     * Verify it has already been created, if so, return it.
     */
    if ( (fontname = getFontName(pfont)) == (char *)NULL)
	return (PclFontHead16Ptr)NULL;

    while (pfh16 != (PclFontHead16Ptr) NULL) {
	if (!strcmp(pfh16->fontname, fontname))
	    return pfh16;
	prev = pfh16;
	pfh16 = pfh16->next;
    }

    /*
     * Create Font Header Information
     */
    pfh16 = (PclFontHead16Ptr)xalloc(sizeof(PclFontHead16Rec));
    if (pfh16 == (PclFontHead16Ptr)NULL)
	return (PclFontHead16Ptr)NULL;

    pfi = (FontInfoRec *)&pfont->info;
    GetGlyphs(pfont, 1, (unsigned char *)&pfi->defaultCh,
		(FONTLASTROW(pfont) == 0) ? Linear16Bit : TwoD16Bit,
		&n, charinfo);

    if ( n )
	space_width = charinfo[0]->metrics.characterWidth;
    else
	space_width = FONTMAXBOUNDS(pfont,characterWidth);

    fillFontDescData(pfont, &(pfh16->fd), space_width);
    pfh16->cur_fid = 0;
    pfh16->cur_cindex = 0;
    pfh16->fontname = (char *)xalloc(strlen(fontname) + 1);
    if (pfh16->fontname == (char *)NULL) {
	xfree(pfh16);
	return (PclFontHead16Ptr) NULL;
    }
    strcpy(pfh16->fontname, fontname);

    pfi = (FontInfoRec *)&pfont->info;
    nindex_col = pfi->lastCol - pfi->firstCol + 1;
    nindex_row = pfi->lastRow - pfi->firstRow + 1;
    index = (PclFontMapRec **)xalloc(sizeof(PclFontMapRec *)*nindex_row);
    if (index == (PclFontMapRec **)NULL) {
	xfree(pfh16->fontname);
	xfree(pfh16);
	return (PclFontHead16Ptr) NULL;
    }
    for (i=0; i<nindex_row; i++) {
	index[i] = (PclFontMapRec *)xalloc(sizeof(PclFontMapRec)*nindex_col);
	if (index[i] == (PclFontMapRec *)NULL) {
	    for(j=0; j<i; j++)
		xfree(index[j]);
	    xfree(pfh16->fontname);
	    xfree(pfh16);
	    return (PclFontHead16Ptr) NULL;
	}
        for (j=0; j<=nindex_col; j++)
            index[i][j].fid = 0x0;
    }

    pfh16->index = index;
    pfh16->firstCol = pfi->firstCol;
    pfh16->lastCol = pfi->lastCol;
    pfh16->firstRow = pfi->firstRow;
    pfh16->lastRow = pfi->lastRow;
    pfh16->next = (PclFontHead16Ptr)NULL;

    if ( prev == (PclFontHead16Ptr) NULL)
	pSoftFontInfo->phead16 = pfh16;
    else
	prev->next = pfh16;

    return pfh16;
}

static PclInternalFontPtr
makeInternalFont(FontPtr pfont, PclSoftFontInfoPtr pSoftFontInfo)
{
PclInternalFontPtr pinfont = pSoftFontInfo->pinfont;
PclInternalFontPtr pin = pinfont;
PclInternalFontPtr prev = (PclInternalFontPtr)NULL;
FontPropPtr props;
FontInfoPtr pfi;
char *fontname;
Atom xa_pcl_font_name, xa_res, xa_ave_width, xa_spacing;
int width = 1;
int mask;
int i;

    if (pSoftFontInfo == (PclSoftFontInfoPtr) NULL)
	return (PclInternalFontPtr)NULL;

    /*
     * Verify it has already been created, if so, return it.
     */
    if ( (fontname = getFontName(pfont)) == (char *)NULL)
	return (PclInternalFontPtr)NULL;

    while (pin != (PclInternalFontPtr) NULL) {
	if (!strcmp(pin->fontname, fontname))
	    return pin;
	prev = pin;
	pin = pin->next;
    }

    /*
     * Create Internal Font Information
     */
    pin = (PclInternalFontPtr)xalloc(sizeof(PclInternalFontRec));
    if (pin == (PclInternalFontPtr)NULL)
	return (PclInternalFontPtr)NULL;

    pin->fontname = (char *)xalloc(strlen(fontname) + 1);
    if (pin->fontname == (char *)NULL) {
	xfree(pin);
	return (PclInternalFontPtr) NULL;
    }
    strcpy(pin->fontname, fontname);

    xa_pcl_font_name = MakeAtom("PCL_FONT_NAME", strlen("PCL_FONT_NAME"), TRUE);
    xa_res = MakeAtom("RESOLUTION_X", strlen("RESOLUTION_X"), TRUE);
    xa_ave_width = MakeAtom("AVERAGE_WIDTH", strlen("AVERAGE_WIDTH"), TRUE);
    xa_spacing = MakeAtom("SPACING", strlen("SPACING"), TRUE);
    pfi = (FontInfoRec *)&pfont->info;
    props = pfi->props;

    mask = 0;
    for (i=0; i<pfi->nprops; i++, props++) {
	if ( (Atom) props->name == xa_pcl_font_name ) {
	    pin->pcl_font_name = NameForAtom(props->value);
	    mask |= 0x1;
	} else if ( props->name == XA_POINT_SIZE ) {
	    pin->height = (float) props->value / 10.0;
	    mask |= 0x2;
	} else if ( (Atom) props->name == xa_res ) {
	    mask |= 0x4;
	} else if ( (Atom) props->name == xa_ave_width ) {
	    width = (int) props->value / 10;
	    mask |= 0x8;
	} else if ( (Atom) props->name == xa_spacing ) {
	    pin->spacing = NameForAtom(props->value);
	    mask |= 0x10;
	}
    }
    if ( mask != 0x1f ) {
	xfree(pin->fontname);
	xfree(pin);
	return (PclInternalFontPtr) NULL;
    }

    if ( *pin->spacing != 'P' || *pin->spacing != 'p' ) {
	if (width == 0)
	    width = 1;
	pin->pitch = (float) 300.0 / width;  /* Hard-Code: Resolution is 300 */
    }

    pin->next = (PclInternalFontPtr)NULL;
    if ( prev == (PclInternalFontPtr) NULL)
	pSoftFontInfo->pinfont = pin;
    else
	prev->next = pin;

    return pin;
}

static void
fillFontDescData(FontPtr pfont, PclFontDescPtr pfd, unsigned int space)
{
FontInfoPtr pfi;

    pfi = (FontInfoRec *)&pfont->info;

    if ( (pfi->maxbounds.leftSideBearing == pfi->minbounds.leftSideBearing)
	&& (pfi->maxbounds.rightSideBearing == pfi->minbounds.rightSideBearing)
	&& (pfi->maxbounds.characterWidth == pfi->minbounds.characterWidth)
	&& (pfi->maxbounds.ascent == pfi->minbounds.ascent)
	&& (pfi->maxbounds.descent == pfi->minbounds.descent)
    )
	pfd->spacing = MONOSPACE;
    else
	pfd->spacing = PROPSPACE;

    pfd->pitch      = space;
    pfd->cellheight = FONTMAXBOUNDS(pfont,ascent)
				+ FONTMAXBOUNDS(pfont,descent);
    pfd->cellwidth  = FONTMAXBOUNDS(pfont,rightSideBearing)
				- FONTMINBOUNDS(pfont,leftSideBearing);
    pfd->ascent     = FONTMAXBOUNDS(pfont,ascent);   /*FONTASCENT(pfont);*/
    pfd->descent    = FONTMAXBOUNDS(pfont,descent); /*FONTDESCENT(pfont);*/
}

static PclCharDataPtr
fillCharDescData(PclCharDataPtr pcd, CharInfoPtr pci)
{
unsigned int byte_width;
unsigned char *p;
register int nbyGlyphWidth;
unsigned char *pglyph, *pg;
unsigned int i, j;

    pcd->h_offset   = pci->metrics.leftSideBearing;
    pcd->v_offset   = pci->metrics.ascent;
    pcd->width      = pci->metrics.rightSideBearing
				- pci->metrics.leftSideBearing;
    pcd->height     = pci->metrics.ascent + pci->metrics.descent;
    pcd->font_pitch = pci->metrics.characterWidth;

    byte_width = (pcd->width + 7)/8;
    pcd->raster_top = (unsigned char *)xalloc(byte_width * pcd->height);
    if (pcd->raster_top == (unsigned char *)NULL)
	return (PclCharDataPtr)NULL;

    p = pcd->raster_top;
    nbyGlyphWidth = GLYPHWIDTHBYTESPADDED(pci);
    pglyph = FONTGLYPHBITS(pglyphBase, pci);
    for (i=0; i<pcd->height; i++) {
	pg = pglyph + nbyGlyphWidth * i;
	for (j=0; j<byte_width; j++) 
	    *p++ = *pg++;
    }
    return pcd;
}

static void
output_text(FILE *outFile,
	PclContextPrivPtr pConPriv,
	unsigned char index)
{
    if ( index == ETX ) {
	sprintf(t, "%c;DT%c,1;LB%c%c;DT%c,1;LB",
				ETX, ETX_ALT, ETX, ETX_ALT, ETX);
	SAVE_PCL( outFile, pConPriv, t );
    } else {
	sprintf(t, "%c", index);
	SAVE_PCL_COUNT( outFile, pConPriv, t, 1 );
    }
}

static char *
getFontName(FontPtr pfont)
{
int i;
FontInfoPtr pfi;
FontPropPtr props;
char *fontname;

    pfi = (FontInfoRec *)&pfont->info;
    props = pfi->props;
    fontname = (char *) NULL;
    for (i=0; i<pfi->nprops; i++, props++) {
        if ( props->name == XA_FONT ) {
            fontname = (char *)NameForAtom(props->value);
            break;
        }
    }
    return fontname;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Internal Font Selection                                               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static char
isInternal(FontPtr pfont)
{
int i;
FontInfoPtr pfi;
FontPropPtr props;
Atom dest;

    dest = MakeAtom("PRINTER_RESIDENT_FONT", strlen("PRINTER_RESIDENT_FONT"), TRUE);

    pfi = (FontInfoRec *)&pfont->info;
    props = pfi->props;
    for (i=0; i<pfi->nprops; i++, props++) {
        if ( (Atom) props->name == dest && props->value == 2 )
		return INTERNAL_FONT;
    }
    return DOWNLOAD_FONT;
}

static void
selectInternalFont(FILE *outFile, PclInternalFontPtr pin, int fid)
{
    fprintf(outFile, "%c*c%dD", ESC, fid);
    if ( *pin->spacing == 'P' || *pin->spacing == 'p' )
	fprintf(outFile, pin->pcl_font_name, pin->height);
    else
	fprintf(outFile, pin->pcl_font_name, pin->pitch);
    fprintf(outFile, "%c*c6F", ESC);
}

static void
selectSize(FILE *outFile,
	PclContextPrivPtr pConPriv,
	PclInternalFontPtr pin)
{
    if ( *pin->spacing == 'P' || *pin->spacing == 'p' ) {
	sprintf(t, "SD4,%f;", pin->height);
	SAVE_PCL( outFile, pConPriv, t );
    } else {
	sprintf(t, "SD3,%f;", pin->pitch);
	SAVE_PCL( outFile, pConPriv, t );
    }
    return;
}

#ifdef DO_TWO_BYTE_PCL
static void
code_conv(
    PclSoftFontInfoPtr pSoftFontInfo,
    FontPtr pfont,
    char *from,
    char *to
)
{
iconv_t cd;
char frombuf[9], *fromptr;
size_t inbyte = 5, outbyte=2;

    fromptr = frombuf;
    frombuf[0] = 0x1b; /* Esc */
    frombuf[1] = 0x24; /* $ */
    frombuf[2] = 0x42; /* B */
    frombuf[3] = *from;
    frombuf[4] = *(from+1);
    frombuf[5] = 0x1b; /* Esc */
    frombuf[6] = 0x28; /* ( */
    frombuf[7] = 0x4a; /* J */
    frombuf[8] = 0x0;
    if ((cd = iconv_open("sjis", "jis")) == (iconv_t)(-1)) {
	*to = (unsigned char)NULL;
	return;
    }

    if ( iconv(cd, &fromptr, &inbyte, &to, &outbyte) == -1 )
	*to = (unsigned char)NULL;

    iconv_close(cd);
    return;
}
#endif /* DO_TWO_BYTE_PCL */
