/*
 * Copyright © 2004 Ralph Thomas
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Ralph Thomas not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Ralph Thomas makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * RALPH THOMAS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL RALPH THOMAS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
** VIA CLE266 driver
** Copyright 2004 (C) Ralph Thomas <ralpht@gmail.com>
**
** http://www.viatech.com.tw/
**
** This code is for accelerated drawing of solids and accelerated
** copies. Note that there is currently no software FIFO implemented,
** and no documentation on any hardware FIFO.
*/

#include "via.h"
#include "viadraw.h"
#include "via_regs.h"
#include <sched.h>
#include "kdrive.h"
#include "kaa.h"

/*
** A global to contain card information between calls into this file.
** XXX: This is totally brain-damaged. Why don't I get the information
**      I want in viaSolid/viaCopy/viaDoneXXX?
*/
static ViaCardInfo* card;

/*
** Translation table from GC raster operation values into ROP3 values
** that the VIA chip understands.
**
** viaPatternRop is used by viaPrepareSolid.
** viaCopyRop is used by viaPrepareCopy.
*/
CARD8 viaPatternRop[16] = {
	/* GXclear      */	0x00,	/* ROP_0	0 */
	/* GXand        */	0xA0,	/* ROP_DPa	src AND dst */
	/* GXandReverse */	0x50,	/* ROP_PDna	src AND NOT dst */
	/* GXcopy       */	0xF0,	/* ROP_P	src */
	/* GXandInverted*/	0x0A,	/* ROP_DPna	NOT src AND dst */
	/* GXnoop       */	0xAA,	/* ROP_D	dst */
	/* GXxor        */	0x5A,	/* ROP_DPx	src XOR dst */
	/* GXor         */	0xFA,	/* ROP_DPo	src OR dst */
	/* GXnor        */	0x05,	/* ROP_DPon	NOT src AND NOT dst */
	/* GXequiv      */	0xA5,	/* ROP_PDxn	NOT src XOR dst */
	/* GXinvert     */	0x55,	/* ROP_Dn	NOT dst */
	/* GXorReverse  */	0xF5,	/* ROP_PDno	src OR NOT dst */
	/* GXcopyInverted*/	0x0F, 	/* ROP_Pn	NOT src */
	/* GXorInverted */	0xAF,	/* ROP_DPno	NOT src OR dst */
	/* GXnand       */	0x5F,	/* ROP_DPan	NOT src OR NOT dst */
	/* GXset        */	0xFF,	/* ROP_1	1 */
};

CARD8 viaCopyRop[16] = {
	/* GXclear	*/	0x00,	/* ROP_0	0 */
	/* GXand	*/	0x88,	/* ROP_DSa	src AND dst */
	/* GXandReverse	*/	0x44,	/* ROP_SDna	src AND NOT dst */
	/* GXcopy	*/	0xCC,	/* ROP_S	src */
	/* GXandInverted*/	0x22,	/* ROP_DSna	NOT src AND dst */
	/* GXnoop	*/	0xAA,	/* ROP_D	dst */
	/* GXxor	*/	0x66,	/* ROP_DSx	src XOR dst */
	/* GXor		*/	0xEE,	/* ROP_DSo	src OR dst */
	/* GXnor	*/	0x11,	/* ROP_DSon	NOT src AND NOT dst */
	/* GXequiv	*/	0x99,	/* ROP_DSxn	NOT src XOR dst */
	/* GXinvert	*/	0x55,	/* ROP_Dn	NOT dst */
	/* GXorReverse	*/	0xDD,	/* ROP_SDno	src OR NOT dst */
	/* GXcopyInverted*/	0x33,	/* ROP_Sn	NOT src */
	/* GXorInverted	*/	0xBB,	/* ROP_DSno	NOT src OR dst */
	/* GXnand	*/	0x77,	/* ROP_DSan	NOT src OR NOT dst */
	/* GXset	*/	0xFF,	/* ROP_1	1 */
};

/*
** void viaWaitIdle( ViaCardInfo* viac )
**
** Description:
**	Block up the CPU while waiting for the last command sent to
**	the chip to complete. As an experiment I'm going to try to
**	yield my process to others instead of just tight looping.
**
** Arguments:
**	viac	VIA-driver specific chip information
**
** Return:
**	None.
*/
void
viaWaitIdle( ViaCardInfo* viac ) {
	while( INREG32( VIA_REG_STATUS ) & VIA_BUSY )
		sched_yield();
}

/*
** void viaDrawSync( ScreenPtr pScreen, int marker )
**
** Description:
**	Block until the graphics chip has finished all outstanding drawing
**	operations and the framebuffer contents is static.
**
** Arguments:
**	pScreen		Pointer to screen strucutre for the screen we're
**			waiting for drawing to end on.
**
** Return:
**	None.
*/
static void
viaWaitMarker( ScreenPtr pScreen, int marker ) {
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;

	viaWaitIdle( viac );
}


/*
** Bool viaPrepareSolid( PixmapPtr pPixmap, int alu, Pixel planemask, Pixel fg )
**
** Description:
**	Decide if the specified solid fill operation can be accelerated or not,
**	and if the fill can be accelerated, prepare the hardware for doing it.
**
** Arguments:
**	pPixmap		Pixmap to draw solid into.
**	alu		Raster operation to draw using, these are the same
**			values which get set by XSetFunction(3X). See the
**			Xlib PM p. 140 for a list of raster operations as
**			well as descriptions.
**	planemask	This is the GC plane mask. We only copy bits which
**			match the plane mask.
**	fg		The foreground pixel of the GC, the pixel to draw in.
**
** Return:
**	TRUE	This operation can be accelerated, call viaSolid to actually
**		have it drawn.
**	FALSE	This operation cannot be accelerated, fall back to software.
**
** See Also:
**	viaSolid - the function which actually draws the solid.
*/
static Bool
viaPrepareSolid( PixmapPtr pPixmap, int alu, Pixel planeMask, Pixel fg ) {
	ScreenPtr pScreen = pPixmap->drawable.pScreen;
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;

	/*
	** We don't accelerate when the plane mask is not everything.
	*/
	if( ~planeMask & FbFullMask( pPixmap->drawable.depth ) ) return FALSE;

	/*
	** Compose the command, then store the composed command and color
	** in the viac structure so that when viaSolid gets called we can
	** write them out.
	*/
	viac->savedCommand = VIA_GEC_BLT | VIA_GEC_FIXCOLOR_PAT |
			     (viaPatternRop[alu]  << 24); 
	viac->savedFgColor = fg;

	/*
	** Store pointer to chip information, due to brain-damaged KAA.
	*/
	card = viac;

	return TRUE;
}

/*
** void viaSolid( int x1, int y1, int x2, int y2 )
**
** Description:
**	Perform a solid fill, using the data that was stored by viaPrepareSolid.
**
** Arguments:
**	x1	x-coordinate of fill origin
**	y1	y-coordinate of fill origin
**	x2	x-coordinate of fill end point
**	y2	y-coordinate of fill end point
**
** Return:
**	None.
**
** See Also:
**	viaPrepareSolid - the function that composes the GE command and saves
**			  the color for us.
*/
static void
viaSolid( int x1, int y1, int x2, int y2 ) {
	ViaCardInfo* viac = card;
	int w = x2 - x1; int h = y2 - y1;

	if( !viac ) return;
	if( !w || !h ) return;

	/*
	** Wait for the card to finish the current draw.
	*/
	viaWaitIdle( viac );

	/*
	** Do the draw.
	*/
	OUTREG32( VIA_REG_DSTPOS, ((y1 << 16) | x1) );
	OUTREG32( VIA_REG_DIMENSION, (((h - 1) << 16) | (w - 1)) );
	OUTREG32( VIA_REG_FGCOLOR, viac->savedFgColor );
	OUTREG32( VIA_REG_GECMD, viac->savedCommand );
}

/*
** void viaDoneSolid
**
** Description:
**	Finish up drawing of the solid.
**
** Arguments:
**	None.
**
** Return:
**	None.
*/
static void
viaDoneSolid(void) {
}

/*
** Bool viaPrepareCopy( PixmapPtr pSrcPixmap, PixmapPtr pDestPixmap, int dx,
**			int dy, int alu, Pixel planeMask )
**
** Description:
**	Set up the VIA chip for a BitBlt.
**
** Arguments:
**	pSrcPixmap	the source pixmap to copy from
**	pDestPixmap	the destination pixmap to copy to
**	dx		direction of copy in x
**	dy		direction of copy in y
**	alu		Raster operation to draw using, these are the same
**			values which get set by XSetFunction(3X). See the
**			Xlib PM p. 140 for a list of raster operations as
**			well as descriptions.
**	planeMask	This is the GC plane mask. We only copy bits which
**			match the plane mask.
**
** Return:
**	TRUE	the requested copy operation can be accelerated using hardware,
**		call viaCopy next.
**	FALSE	the requested copy operation cannot be accelerated using
**		hardware - fallback to software.
**
** See Also:
**	viaCopy	- the function which does the actual copy.
*/
static Bool
viaPrepareCopy( PixmapPtr pSrcPixmap, PixmapPtr pDestPixmap, int dx, int dy,
		int alu, Pixel planeMask ) {
	ScreenPtr pScreen = pDestPixmap->drawable.pScreen;
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;

	/*
	** Don't accelerate when the plane mask is set.
	*/
	if( ~planeMask & FbFullMask( pDestPixmap->drawable.depth ) ) return FALSE;

	viac->savedCommand = VIA_GEC_BLT | (viaCopyRop[alu] << 24);

	if( dx < 0 ) viac->savedCommand |= VIA_GEC_DECX;
	if( dy < 0 ) viac->savedCommand |= VIA_GEC_DECY;

	/*
	** Store pointer to chip structure, due to brain-damaged KAA.
	*/
	card = viac;

	return TRUE;
}

/*
** void viaCopy( int srcX, int srcY, int dstX, int dstY, int w, int h )
**
** Description:
**	Perform a BitBlt from one screen area to another.
**
** Arguments:
**	srcX	source x-coordinate
**	srcY	source y-coordinate
**	dstX	destination x-coordinate
**	dstY	destination y-coordinate
**	w	width of area to copy (pixels)
**	h	height of area to copy (pixels)
**
** Return:
**	None.
**
** See Also:
**	viaPrepareCopy	- the function which sets up for the copy.
*/
static void
viaCopy( int srcX, int srcY, int dstX, int dstY, int w, int h ) {
	ViaCardInfo* viac = card;

	if( !viac ) return;
	if( !w | !h ) return;

	/*
	** XXX: Check these two "if"s out.
	*/
	if( viac->savedCommand & VIA_GEC_DECX ) {
		srcX += ( w - 1 );
		dstX += ( w - 1 );
	}

	if( viac->savedCommand & VIA_GEC_DECY ) {
		srcY += ( h - 1 );
		dstY += ( h - 1 );
	}

	OUTREG32( VIA_REG_SRCPOS, ((srcY << 16) | srcX) );
	OUTREG32( VIA_REG_DSTPOS, ((dstY << 16) | dstX) );
	OUTREG32( VIA_REG_DIMENSION, (((h - 1) << 16) | (w - 1)) );
	OUTREG32( VIA_REG_GECMD, viac->savedCommand );
}

/*
** void viaDoneCopy()
**
** Description:
**	Finish up the copy.
**
** Arguments:
**	None.
**
** Return:
**	None.
*/
static void
viaDoneCopy(void) {
}


/*
** Bool viaDrawInit( ScreenPtr pScreen )
**
** Description:
**	Initialize the 2D acceleration hardware and register the KAA
**	acceleration layer with the VIA acceleration functions (above).
**
** Arguments:
**	pScreen		Pointer to screen structure for the screen we're
**			enabling acceleration on.
**
** Return:
**	TRUE	initialization and setup of KAA acceleration was successful.
**	FALSE	initialization and setup of KAA acceleration failed.
*/
Bool
viaDrawInit( ScreenPtr pScreen ) {
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;
	ViaScreenInfo* vias = pScreenPriv->card->driver;
	CARD32 geMode = 0;

	if( !viac ) return FALSE;
	DebugF( "viac->mapBase = 0x%x\n", viac->mapBase );

	/*
	** We reset the 2D engine to a known state by setting all of it's
	** registers to zero.
	*/
	OUTREG32( VIA_REG_GEMODE, 0x0 );
	OUTREG32( VIA_REG_SRCPOS, 0x0 );
	OUTREG32( VIA_REG_DSTPOS, 0x0 );
	OUTREG32( VIA_REG_DIMENSION, 0x0 );
	OUTREG32( VIA_REG_PATADDR, 0x0 );
	OUTREG32( VIA_REG_FGCOLOR, 0x0 );
	OUTREG32( VIA_REG_BGCOLOR, 0x0 );
	OUTREG32( VIA_REG_CLIPTL, 0x0 );
	OUTREG32( VIA_REG_CLIPBR, 0x0 );
	OUTREG32( VIA_REG_OFFSET, 0x0 );
	OUTREG32( VIA_REG_KEYCONTROL, 0x0 );
	OUTREG32( VIA_REG_SRCBASE, 0x0 );
	OUTREG32( VIA_REG_DSTBASE, 0x0 );
	OUTREG32( VIA_REG_PITCH, 0x0 );
	OUTREG32( VIA_REG_MONOPAT0, 0x0 );
	OUTREG32( VIA_REG_MONOPAT1, 0x0 );

	/*
	** Set the GE mode up.
	** XXX: What happens in 24bpp mode?
	*/
	switch( pScreenPriv->screen->fb[0].bitsPerPixel ) {
	  case 16:
		geMode = VIA_GEM_16bpp;
		break;
	  case 32:
		geMode = VIA_GEM_32bpp;
		break;
	  default:
		geMode = VIA_GEM_8bpp;
		break;
	}

	OUTREG32( VIA_REG_GEMODE, geMode );

	/*
	** Set the source and destination base addresses, and set pitch.
	*/
	OUTREG32( VIA_REG_SRCBASE, 0x0 );
	OUTREG32( VIA_REG_DSTBASE, 0x0 );
	OUTREG32( VIA_REG_PITCH, VIA_PITCH_ENABLE |
	       ((pScreen->width * pScreenPriv->screen->fb[0].bitsPerPixel >> 3) >> 3) |
               (((pScreen->width * pScreenPriv->screen->fb[0].bitsPerPixel >> 3) >> 3) << 16));

	DebugF( "Initialized 2D engine!\n" );

	memset(&vias->kaa, 0, sizeof(KaaScreenInfoRec));
	vias->kaa.waitMarker	= viaWaitMarker;
	vias->kaa.PrepareSolid	= viaPrepareSolid;
	vias->kaa.Solid		= viaSolid;
	vias->kaa.DoneSolid	= viaDoneSolid;
	vias->kaa.PrepareCopy	= viaPrepareCopy;
	vias->kaa.Copy		= viaCopy;
	vias->kaa.DoneCopy	= viaDoneCopy;

	return kaaDrawInit( pScreen, &vias->kaa );
}

/*
** void viaDrawEnable( ScreenPtr pScreen )
**
** Description:
**	Enable accelerated drawing on the specified screen.
**
** Arguments:
**	pScreen		Pointer to screen structure for the screen we're
**			enabling acceleration on.
**
** Return:
**	None.
*/
void
viaDrawEnable( ScreenPtr pScreen ) {
	kaaMarkSync( pScreen );
}

/*
** void viaDrawDisable( ScreenPtr pScreen )
**
** Description:
**	Disable accelerated drawing to the specified screen.
**
** Arguments:
**	pScreen		Pointer to screen structure for the screen we're
**			disabling acceleration on.
**
** Return:
**	None
*/
void
viaDrawDisable( ScreenPtr pScreen ) {
}

/*
** void viaDrawFini( ScreenPtr pScreen )
**
** Description:
**	Shutdown accelerated drawing and free associated strucures and
**	resources.
**
** Arguments:
**	pScreen		Pointer to screen structure for the screen we're
**			disabling acceleration on.
**
** Return:
**	None.
*/
void
viaDrawFini( ScreenPtr pScreen ) {
}
