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
** This is the stub code which links the VIA drawing code into the kdrive
** framework.
*/

#include "via.h"
#include <klinux.h>

/*
** void InitCard( char* name )
**
** Description:
**	Initialize the graphics chip. We find the chip on the PCI bus, 
**	register the functions to access the chip.
**
** Arguments:
**	name	XXX: unknown.
**
** Return:
**	None.
*/
void
InitCard( char* name ) {
	KdCardAttr attr;

	if( LinuxFindPci( 0x1106, 0x3122, 0, &attr ) )
		KdCardInfoAdd( &viaFuncs, &attr, 0 );
}

/*
** void InitOutput( ScreenInfo* pScreenInfo, int argc, char** argv )
**
** Description:
**	Initialize I/O, or something. XXX: Or what?
**
** Arguments:
**	pScreenInfo	XXX
**	argc		command line argument count
**	argv		command line arguments
**
** Return:
**	None.
*/
void
InitOutput( ScreenInfo* pScreenInfo, int argc, char** argv ) {
	KdInitOutput( pScreenInfo, argc, argv );
}

/*
** void InitInput( int argc, char** argv )
**
** Description:
**	Initialize keyboard and mouse input.
**
** Arguments:
**	argc		command line argument count
**	argv		command line arguments
**
** Return:
**	None.
*/
void
InitInput( int argc, char** argv ) {
        KdOsAddInputDrivers();
	KdInitInput();
}

/*
** void ddxUseMsg()
**
** Description:
**	Print the usage message for Xvia.
**
** Arguments:
**	None.
**
** Return:
**	None.
*/
void
ddxUseMsg() {
	KdUseMsg();
	vesaUseMsg();
}


/*
** int ddxProcessArgument( int argc, char** argv, int i )
**
** Description:
**	Process a single command line argument.
**
** Arguments:
**	argc		command line argument count
**	argv		command line arguments
**	i		number of argument to process
**
** Return:
**	some values.
*/
int
ddxProcessArgument( int argc, char** argv, int i ) {
	int ret;

	if( !( ret = vesaProcessArgument( argc, argv, i ) ))
		ret = KdProcessArgument( argc, argv, i );
	return ret;
}

