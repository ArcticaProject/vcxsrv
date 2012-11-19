/*

Copyright 1986, 1998  The Open Group

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

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"

int
XQueryColor(
    register Display *dpy,
    Colormap cmap,
    XColor *def)	/* RETURN */
{
    xrgb color;
    xQueryColorsReply rep;
    register xQueryColorsReq *req;
    unsigned long val = def->pixel;	/* needed for macro below */

    LockDisplay(dpy);
    GetReqExtra(QueryColors, 4, req); /* a pixel (CARD32) is 4 bytes */
    req->cmap = cmap;

    OneDataCard32 (dpy, NEXTPTR(req,xQueryColorsReq), val);

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) != 0) {

	    _XRead(dpy, (char *)&color, (long) SIZEOF(xrgb));

	    def->red = color.red;
	    def->blue = color.blue;
	    def->green = color.green;
	    def->flags = DoRed | DoGreen | DoBlue;
        }
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}
