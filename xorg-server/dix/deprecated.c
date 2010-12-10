/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "dix.h"
#include "misc.h"
#include "dixstruct.h"

/*
 * These are deprecated compatibility functions and will be marked as such
 * and removed soon!
 *
 * Please use the noted replacements instead.
 */

/* replaced by dixLookupWindow */
WindowPtr
SecurityLookupWindow(XID id, ClientPtr client, Mask access_mode)
{
    WindowPtr pWin;
    static int warn = 1;
    dixLookupWindow(&pWin, id, client, access_mode);
    if (warn > 0 && warn--)
	ErrorF("Warning: LookupWindow()/SecurityLookupWindow() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupWindow().\n");
    return pWin;
}

/* replaced by dixLookupWindow */
WindowPtr
LookupWindow(XID id, ClientPtr client)
{
    return SecurityLookupWindow(id, client, DixUnknownAccess);
}

/* replaced by dixLookupDrawable */
pointer
SecurityLookupDrawable(XID id, ClientPtr client, Mask access_mode)
{
    DrawablePtr pDraw;
    static int warn = 1;
    dixLookupDrawable(&pDraw, id, client, M_DRAWABLE, access_mode);
    if (warn > 0 && warn--)
	ErrorF("Warning: LookupDrawable()/SecurityLookupDrawable() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupDrawable().\n");
    return pDraw;
}

/* replaced by dixLookupDrawable */
pointer
LookupDrawable(XID id, ClientPtr client)
{
    return SecurityLookupDrawable(id, client, DixUnknownAccess);
}

/* replaced by dixLookupClient */
ClientPtr
LookupClient(XID id, ClientPtr client)
{
    ClientPtr pClient;
    static int warn = 1;
    dixLookupClient(&pClient, id, client, DixUnknownAccess);
    if (warn > 0 && warn--)
	ErrorF("Warning: LookupClient() is deprecated.  Please convert your "
	       "driver/module to use dixLookupClient().\n");
    return pClient;
}

/* replaced by dixLookupResourceByType */
pointer
SecurityLookupIDByType(ClientPtr client, XID id, RESTYPE rtype,
		       Mask access_mode)
{
    pointer retval;
    static int warn = 1;
    dixLookupResourceByType(&retval, id, rtype, client, access_mode);
    if (warn > 0 && warn--)
	ErrorF("Warning: LookupIDByType()/SecurityLookupIDByType() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupResourceByType().\n");
    return retval;
}

pointer
SecurityLookupIDByClass(ClientPtr client, XID id, RESTYPE classes,
			Mask access_mode)
{
    pointer retval;
    static int warn = 1;
    dixLookupResourceByClass(&retval, id, classes, client, access_mode);
    if (warn > 0 && warn--)
	ErrorF("Warning: LookupIDByClass()/SecurityLookupIDByClass() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupResourceByClass().\n");
    return retval;
}

/* replaced by dixLookupResourceByType */
pointer
LookupIDByType(XID id, RESTYPE rtype)
{
    pointer val;
    dixLookupResourceByType(&val, id, rtype, NullClient, DixUnknownAccess);
    return val;
}

/* replaced by dixLookupResourceByClass */
pointer
LookupIDByClass(XID id, RESTYPE classes)
{
    pointer val;
    dixLookupResourceByClass(&val, id, classes, NullClient, DixUnknownAccess);
    return val;
}

/* end deprecated functions */
