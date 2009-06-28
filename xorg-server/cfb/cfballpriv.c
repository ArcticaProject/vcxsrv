/*
 *
Copyright 1991, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

#if 1 || PSZ==8
DevPrivateKey cfbGCPrivateKey = &cfbGCPrivateKey;
#endif
#ifdef CFB_NEED_SCREEN_PRIVATE
DevPrivateKey cfbScreenPrivateKey = &cfbScreenPrivateKey;
#endif


Bool
cfbAllocatePrivates(ScreenPtr pScreen, DevPrivateKey *gc_key)
{
    if (!gc_key || !*gc_key)
    {
    	if (!mfbAllocatePrivates(pScreen, &cfbGCPrivateKey))
	    return FALSE;
    	if (gc_key)
	    *gc_key = cfbGCPrivateKey;
    }
    else
    {
	cfbGCPrivateKey = *gc_key;
    }
    return dixRequestPrivate(cfbGCPrivateKey, sizeof(cfbPrivGC));
}
