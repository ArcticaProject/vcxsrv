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
_X_EXPORT WindowPtr
SecurityLookupWindow(XID id, ClientPtr client, Mask access_mode)
{
    WindowPtr pWin;
    int i = dixLookupWindow(&pWin, id, client, access_mode);
    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: LookupWindow()/SecurityLookupWindow() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupWindow().\n");
    return (i == Success) ? pWin : NULL;
}

/* replaced by dixLookupWindow */
_X_EXPORT WindowPtr
LookupWindow(XID id, ClientPtr client)
{
    return SecurityLookupWindow(id, client, DixUnknownAccess);
}

/* replaced by dixLookupDrawable */
_X_EXPORT pointer
SecurityLookupDrawable(XID id, ClientPtr client, Mask access_mode)
{
    DrawablePtr pDraw;
    int i = dixLookupDrawable(&pDraw, id, client, M_DRAWABLE, access_mode);
    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: LookupDrawable()/SecurityLookupDrawable() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupDrawable().\n");
    return (i == Success) ? pDraw : NULL;
}

/* replaced by dixLookupDrawable */
_X_EXPORT pointer
LookupDrawable(XID id, ClientPtr client)
{
    return SecurityLookupDrawable(id, client, DixUnknownAccess);
}

/* replaced by dixLookupClient */
_X_EXPORT ClientPtr
LookupClient(XID id, ClientPtr client)
{
    ClientPtr pClient;
    int i = dixLookupClient(&pClient, id, client, DixUnknownAccess);
    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: LookupClient() is deprecated.  Please convert your "
	       "driver/module to use dixLookupClient().\n");
    return (i == Success) ? pClient : NULL;
}

/* replaced by dixLookupResourceByType */
_X_EXPORT pointer
SecurityLookupIDByType(ClientPtr client, XID id, RESTYPE rtype,
		       Mask access_mode)
{
    pointer retval;
    int i = dixLookupResourceByType(&retval, id, rtype, client, access_mode);
    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: LookupIDByType()/SecurityLookupIDByType() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupResourceByType().\n");
    return (i == Success) ? retval : NULL;
}

/* replaced by dixLookupResourceByClass */
_X_EXPORT pointer
SecurityLookupIDByClass(ClientPtr client, XID id, RESTYPE classes,
			Mask access_mode)
{
    pointer retval;
    int i = dixLookupResourceByClass(&retval, id, classes, client, access_mode);
    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: LookupIDByClass()/SecurityLookupIDByClass() "
	       "are deprecated.  Please convert your driver/module "
	       "to use dixLookupResourceByClass().\n");
    return (i == Success) ? retval : NULL;
}

/* replaced by dixLookupResourceByType */
_X_EXPORT pointer
LookupIDByType(XID id, RESTYPE rtype)
{
    return SecurityLookupIDByType(NullClient, id, rtype, DixUnknownAccess);
}

/* replaced by dixLookupResourceByClass */
_X_EXPORT pointer
LookupIDByClass(XID id, RESTYPE classes)
{
    return SecurityLookupIDByClass(NullClient, id, classes, DixUnknownAccess);
}

/* replaced by dixLookupResourceBy{Type,Class} */
_X_EXPORT int
dixLookupResource (pointer *result, XID id, RESTYPE rtype,
		   ClientPtr client, Mask mode)
{
    Bool istype = ((rtype & TypeMask) && (rtype != RC_ANY)) || (rtype == RT_NONE);

    static int warn = 1;
    if (warn > 0 && --warn)
	ErrorF("Warning: dixLookupResource() "
	       "is deprecated.  Please convert your driver/module "
	       "to use dixLookupResourceByType/dixLookupResourceByClass().\n");
    if (istype)
	return dixLookupResourceByType (result, id, rtype, client, mode);
    else
	return dixLookupResourceByClass (result, id, rtype, client, mode);
}

/* end deprecated functions */
