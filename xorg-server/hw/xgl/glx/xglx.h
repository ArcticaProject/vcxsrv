/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef _XGLX_H_
#define _XGLX_H_

#include "xgl.h"

#ifdef _XSERVER64
#define _XSERVER64_tmp
#undef _XSERVER64
typedef unsigned long XID64;
typedef unsigned long Mask64;
typedef unsigned long Atom64;
typedef unsigned long VisualID64;
typedef unsigned long Time64;
#define XID      XID64
#define Mask     Mask64
#define Atom     Atom64
#define VisualID VisualID64
#define Time     Time64
typedef XID Window64;
typedef XID Drawable64;
typedef XID Font64;
typedef XID Pixmap64;
typedef XID Cursor64;
typedef XID Colormap64;
typedef XID GContext64;
typedef XID KeySym64;
#define Window   Window64
#define Drawable Drawable64
#define Font     Font64
#define Pixmap   Pixmap64
#define Cursor   Cursor64
#define Colormap Colormap64
#define GContext GContext64
#define KeySym   KeySym64
#endif

#define GC XlibGC
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#undef GC

#ifdef _XSERVER64_tmp
#ifndef _XSERVER64
#define _XSERVER64
#endif
#undef _XSERVER64_tmp
#undef XID
#undef Mask
#undef Atom
#undef VisualID
#undef Time
#undef Window
#undef Drawable
#undef Font
#undef Pixmap
#undef Cursor
#undef Colormap
#undef GContext
#undef KeySym
#endif

void
xglxInitOutput (ScreenInfo *pScreenInfo,
		int	   argc,
		char	   **argv);

Bool
xglxLegalModifier (unsigned int key,
		   DeviceIntPtr pDev);

void
xglxProcessInputEvents (void);

void
xglxInitInput (int  argc,
	       char **argv);

void
xglxUseMsg (void);

int
xglxProcessArgument (int  argc,
		     char **argv,
		     int  i);

void
xglxAbort (void);

void
xglxGiveUp (void);

void
xglxOsVendorInit (void);

#ifndef NXGLXORG

void
xglxUseXorgMsg (void);

int
xglxProcessXorgArgument (int  argc,
			 char **argv,
			 int  i);

void
xglxAbortXorg (void);

char *
xglxInitXorg (void);

#endif

#endif /* _XGLX_H_ */
