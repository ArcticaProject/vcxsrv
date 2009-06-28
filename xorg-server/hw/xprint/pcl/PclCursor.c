/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclCursor.c
**    *
**    *  Contents:
**    *                 Cursor-handling code for the PCL DDX driver
**    *
**    *  Created:	1/18/96
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

#include <stdio.h>

#include "Pcl.h"
#include "gcstruct.h"
#include "windowstr.h"

void
PclConstrainCursor(
     ScreenPtr pScreen,
     BoxPtr pBox)
{
}

void
PclCursorLimits(
     ScreenPtr pScreen,
     CursorPtr pCursor,
     BoxPtr pHotBox,
     BoxPtr pTopLeftBox)
{
}

Bool
PclDisplayCursor(
     ScreenPtr pScreen,
     CursorPtr pCursor)
{
    return True;
}

Bool
PclRealizeCursor(
     ScreenPtr pScreen,
     CursorPtr pCursor)
{
    return True;
}

Bool
PclUnrealizeCursor(
     ScreenPtr pScreen,
     CursorPtr pCursor)
{
    return True;
}

void
PclRecolorCursor(
     ScreenPtr pScreen,
     CursorPtr pCursor,
     Bool displayed)
{
}

Bool
PclSetCursorPosition(
     ScreenPtr pScreen,
     int x,
     int y,
     Bool generateEvent)
{
    return True;
}
