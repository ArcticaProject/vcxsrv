
/*
 * Copyright (c) 2000-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/time.h>
#include <unistd.h> 
#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "compiler.h"

CARD8  xf86PeekFb8(CARD8  *p) { return *p; }
CARD16 xf86PeekFb16(CARD16 *p) { return *p; }
CARD32 xf86PeekFb32(CARD32 *p) { return *p; }
void xf86PokeFb8(CARD8  *p, CARD8  v) { *p = v; }
void xf86PokeFb16(CARD16 *p, CARD16 v) { *p = v; }
void xf86PokeFb32(CARD16 *p, CARD32 v) { *p = v; }

CARD8  xf86PeekMmio8(pointer Base, unsigned long Offset)
{
    return MMIO_IN8(Base,Offset);
}

CARD16 xf86PeekMmio16(pointer Base, unsigned long Offset)
{
    return MMIO_IN16(Base,Offset);
}

CARD32 xf86PeekMmio32(pointer Base, unsigned long Offset)
{
    return MMIO_IN32(Base,Offset);
}

void xf86PokeMmio8(pointer Base, unsigned long Offset, CARD8  v)
{
    MMIO_OUT8(Base,Offset,v);
}

void xf86PokeMmio16(pointer Base, unsigned long Offset, CARD16 v)
{
    MMIO_OUT16(Base,Offset,v);
}

void xf86PokeMmio32(pointer Base, unsigned long Offset, CARD32 v)
{
    MMIO_OUT32(Base,Offset,v);
}
