/*
 * Copyright (C) 1989-95 GROUPE BULL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * GROUPE BULL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of GROUPE BULL shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from GROUPE BULL.
 */

/*****************************************************************************\
*  Image.c:                                                                   *
*                                                                             *
*  XPM library                                                                *
*  Functions to init and free the XpmImage structure.                         *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "XpmI.h"

/*
 * Init returned data to free safely later on
 */
void
xpmInitXpmImage(XpmImage *image)
{
    image->ncolors = 0;
    image->colorTable = NULL;
    image->data = NULL;
}

/*
 * Free the XpmImage data which have been allocated
 */
void
XpmFreeXpmImage(XpmImage *image)
{
    if (image->colorTable)
	xpmFreeColorTable(image->colorTable, image->ncolors);
    if (image->data)
	XpmFree(image->data);
    image->data = NULL;
}
