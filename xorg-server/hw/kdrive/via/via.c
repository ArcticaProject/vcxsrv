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
*/

#include "via.h"
#include "viadraw.h"

/*
** viaCardInit( KdCardInfo* card )
**
** Description:
**	Create card specific structures, map chip registers and initialize the
**	VESA driver. We make the VESA driver do boring stuff for us, like set
**	up a framebuffer and program a mode.
**
** Parameters:
** 	card		Information stucture for the card we want to bring up.
**			It should be a VIA card.
**
** Return:
**	TRUE		Initialization went ok.
**	FALSE		Initialization failed.
*/
static Bool
viaCardInit( KdCardInfo* card ) {
	ViaCardInfo*	viac;

	viac = (ViaCardInfo*) xalloc( sizeof( ViaCardInfo ) );
	if( !viac ) return FALSE;
	memset( viac, '\0', sizeof( ViaCardInfo ) );


	viaMapReg( card, viac );

	if( !vesaInitialize( card, &viac->vesa ) ) {
		xfree( viac );
		return FALSE;
	}

	card->driver = viac;

	return TRUE;
}

/*
** Bool viaScreenInit( KdScreenInfo* screen )
**
** Description:
**	Initialize a single screen, described by the screen parameter.
**	This is where fairly low-level screen related things get setup,
**	such as video mode and resolution. Currently that all gets
**	handed off to the VESA driver.
**
** Parameters:
**	screen	Information structure for the screen to enable.
**
** Return:
**	TRUE	Screen was initialized successfully
**	FALSE	Screen initialization failed
*/
static Bool
viaScreenInit( KdScreenInfo* screen ) {
	ViaCardInfo*	viac = screen->card->driver;
	ViaScreenInfo*	vias;

	vias = (ViaScreenInfo*) xalloc( sizeof( ViaScreenInfo ) );
	if( !vias ) return FALSE;
	memset( vias, '\0', sizeof( ViaScreenInfo ) );

	if( !vesaScreenInitialize( screen, &vias->vesa ) ) {
		xfree( vias );
		return FALSE;
	}

	/*
	** XXX: What does this do?
	*/
	if( !viac->mapBase )
		screen->dumb = TRUE;
	if( vias->vesa.mapping != VESA_LINEAR )
		screen->dumb = TRUE;

	screen->driver = vias;
	return TRUE;
}

/*
** Bool viaInitScreen( ScreenPtr pScreen )
**
** Description:
**	High level screen initialization occurs here. We could register XV
**	adaptors, etc, here.
**
** Arguments:
**	pScreen		X screen information
**
** Return:
**	TRUE	Initialization was successful,
**	FALSE	Initialization failed.
*/
static Bool
viaInitScreen( ScreenPtr pScreen ) {
	return vesaInitScreen( pScreen );
}

/*
** Bool viaFinishInitScreen
**
** Description:
**	Finish up any high-level screen initialization. Per-Screen extension
**	initialization can be done here.
**
** Arguments:
**	pScreen		X screen information
**
** Return:
**	TRUE	Initialization was successful.
**	FALSE	Initialization failed.
*/
static Bool
viaFinishInitScreen( ScreenPtr pScreen ) {
	return vesaFinishInitScreen( pScreen );
}

/*
** Bool viaCreateResources( ScreenPtr pScreen )
**
** Description:
**	Do any screen specific configuration.
**
** Arguments:
**	pScreen		X screen information
**
** Return:
**	TRUE	configuration was successful.
**	FALSE	configuration failed.
*/
static Bool
viaCreateResources( ScreenPtr pScreen ) {
	return vesaCreateResources( pScreen );
}

/*
** void viaPreserve( KdCardInfo* card )
**
** Description:
**	Save the current state of the chip, so that it can be restored by
**	viaRestore at a later time.
**
** Arguments:
**	card	Information structure for the chip we want to preserve the
**		state of.
**
** Return:
**	None.
**
** See Also:
**	viaRestore
*/
static void
viaPreserve( KdCardInfo* card ) {
	vesaPreserve( card );
}

/*
** void viaRestore( KdCardInfo* card )
**
** Description:
**	Restore the previous state of the chip, as saved by viaPreserve
**	earlier.
**
** Arguments:
**	card	Information structure for the chip we want to restore the
**		state of.
**
** Return:
**	None.
**
** See Also:
**	viaPreserve
*/
static void viaRestore( KdCardInfo* card ) {
	ViaCardInfo* viac = card->driver;

	viaResetMMIO( card, viac );
	vesaRestore( card );
}

/*
** Bool viaEnable( ScreenPtr pScreen )
**
** Description:
**	This is where we set the card up for drawing the specified screen, e.g.:
**	set the mode and mmap the framebuffer.
**
** Arguments:
**	pScreen		X screen information
**
** Return:
**	TRUE	the screen was enabled
**	FALSE	the screen could not be enabled
*/
static Bool
viaEnable( ScreenPtr pScreen ) {
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;

	if( !vesaEnable( pScreen ) ) return FALSE;

	viaSetMMIO( pScreenPriv->card, viac );

	if( !viac->mapBase ) {
		ErrorF( "Could not map CLE266 graphics registers" );
		return FALSE;
	}

	return TRUE;
}

/*
** void viaDisable( ScreenPtr pScreen )
**
** Description:
**	Shut down drawing: save some state and unmap the framebuffer.
**
** Arguments:
**	pScreen		X screen information
**
** Return:
**	None.
*/
static void
viaDisable( ScreenPtr pScreen ) {
	KdScreenPriv( pScreen );
	ViaCardInfo* viac = pScreenPriv->card->driver;

	viaResetMMIO( pScreenPriv->card, viac );
	vesaDisable( pScreen );
}

/*
** void viaScreenFini( KdScreenInfo* screen )
**
** Description:
**	Release memory and resources allocated by viaScreenInit.
**
** Arguments:
**	screen		Information structure for the screen to release.
**
** Return:
**	None.
**
** See Also:
**	viaScreenInit
*/
static void
viaScreenFini( KdScreenInfo* screen ) {
	ViaScreenInfo* vias = screen->driver;

	vesaScreenFini( screen );
	xfree( vias );
	screen->driver = 0;
}

/*
** void viaCardFini( KdCardInfo* card )
**
** Description:
**	Release memory and resources allocated by viaCardInit.
**
** Arguments:
**	card		Information structure for the chip to release.
**
** Return:
**	None.
**
** See Also:
**	viaCardInit
*/
static void
viaCardFini( KdCardInfo* card ) {
	ViaCardInfo* viac = card->driver;

	viaUnmapReg( card, viac );
	vesaCardFini( card );
	xfree( viac );
}

/*
** void viaSetMMIO( KdCardInfo* card, ViaCardInfo* viac )
**
** Description:
**	Map the card's registers, if they're not already
**	mapped.
**
** Arguments:
**	card	generic chip information
**	viac	VIA-driver specific chip information
**
** Return:
**	None.
*/
void viaSetMMIO( KdCardInfo* card, ViaCardInfo* viac ) {
	if( !viac->mapBase ) viaMapReg( card, viac );
}

/*
** void viaResetMMIO( KdCardInfo* card, ViaCardInfo* viac )
**
** Description:
**	Unmap chip's registers.
**
** Arguments:
**	card	generic chip information
**	viac	VIA-driver specific chip information
**
** Return:
**	None.
*/
void viaResetMMIO( KdCardInfo* card, ViaCardInfo* viac ) {
	viaUnmapReg( card, viac );
}

/*
** Bool viaMapReg( KdCardInfo* card, ViaCardInfo* viac )
**
** Description:
**	Map the chip's registers into our address space.
**
** Arguments:
**	card	the card information
**	viac	the VIA-driver specific card information
**
** Return:
**	TRUE	the registers were succesfully mapped
**	FALSE	the registers could not be mapped
*/
Bool
viaMapReg( KdCardInfo* card, ViaCardInfo* viac ) {
	viac->mapBase = (VOL8*) KdMapDevice( VIA_REG_BASE( card ),
					     VIA_REG_SIZE( card ) );

	if( !viac->mapBase ) {
		ErrorF( "Couldn't allocate viac->mapBase\n" );
		return FALSE;
	}

	KdSetMappedMode( VIA_REG_BASE( card ), VIA_REG_SIZE( card ),
			 KD_MAPPED_MODE_REGISTERS );

	/*
	** Enable extended IO space
	*/
	VGAOUT8( 0x3C4, 0x10 );
	VGAOUT8( 0x3C5, 0x01 );

	return TRUE;
}

/*
** void viaUnmapReg( KdCardInfo* card, ViaCardInfo* viac )
**
** Description:
**	Unmap the the chip's registers.
**
** Arguments:
**	card	the card information
**	viac	the VIA-driver specific card information
**
** Return:
**	None.
*/
void
viaUnmapReg( KdCardInfo* card, ViaCardInfo* viac ) {
	if( !viac->mapBase ) return;

	KdResetMappedMode( VIA_REG_BASE( card ), VIA_REG_SIZE( card ),
			   KD_MAPPED_MODE_REGISTERS );
	KdUnmapDevice( (void*) viac->mapBase, VIA_REG_SIZE( card ) );
	viac->mapBase = 0;
}

KdCardFuncs viaFuncs = {
	viaCardInit,		/* cardinit */
	viaScreenInit,		/* scrinit */
	viaInitScreen,		/* initScreen */
	viaFinishInitScreen,	/* finishInitScreen */
	viaCreateResources,	/* createRes */
	viaPreserve,		/* preserve */
	viaEnable,		/* enable */
	vesaDPMS,		/* dpms */
	viaDisable,		/* disable */
	viaRestore,		/* restore */
	viaScreenFini,		/* scrfini */
	viaCardFini,		/* cardfini */

	0,			/* initCursor */
	0,			/* enableCursor */
	0,			/* disableCursor */
	0,			/* finiCursor */
	0,			/* recolorCursor */

	viaDrawInit,		/* initAccel */
	viaDrawEnable,		/* enableAccel */
	viaDrawDisable,		/* disableAccel */
	viaDrawFini,		/* finiAccel */

	vesaGetColors,		/* getColors */
	vesaPutColors,		/* putColors */
};

