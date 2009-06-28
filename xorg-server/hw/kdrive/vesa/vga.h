/*
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _VGA_H_
#define _VGA_H_

int
VgaGetNmode (Vm86InfoPtr vi);

int
VgaGetModes (Vm86InfoPtr vi, VesaModePtr mode, int nmode);

int
VgaSetMode(Vm86InfoPtr vi, int mode);

int
VgaGetMode (Vm86InfoPtr vi, int *mode);

void
VgaSetWritePlaneMask(Vm86InfoPtr vi, int mask);

void
VgaSetReadPlaneMap(Vm86InfoPtr vi, int map);

int 
VgaSetPalette(Vm86InfoPtr vi, int first, int number, U8 *entries);
        
int 
VgaGetPalette(Vm86InfoPtr vi, int first, int number, U8 *entries);
        
void *
VgaSetWindow (Vm86InfoPtr vi, int vmode, int bytes, int mode, int *size);
    
void *
VgaMapFramebuffer (Vm86InfoPtr vi, int vmode, int *size, CARD32 *phys);

void
VgaUnmapFramebuffer (Vm86InfoPtr vi);

#endif /* _VGA_H_ */
