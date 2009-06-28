#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#if defined(XCSECURITY)
/*
 *Copyright (C) 2003-2004 Harold L Hunt II All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL HAROLD L HUNT II BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of Harold L Hunt II
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from Harold L Hunt II.
 *
 * Authors:	Harold L Hunt II
 */

#include "win.h"

/* Includes for authorization */
#include <X11/Xauth.h>
#include "securitysrv.h"
#include <X11/extensions/securstr.h>


/*
 * Constants
 */

#define AUTH_NAME	"MIT-MAGIC-COOKIE-1"


/*
 * Globals
 */

XID		g_authId = 0;
unsigned int	g_uiAuthDataLen = 0;
char		*g_pAuthData = NULL;


/*
 * Generate authorization cookie for internal server clients
 */

Bool
winGenerateAuthorization ()
{
  Bool				fFreeAuth = FALSE;
  SecurityAuthorizationPtr	pAuth = NULL;

  /* Call OS layer to generate authorization key */
  g_authId = GenerateAuthorization (strlen (AUTH_NAME),
				    AUTH_NAME,
				    0,
				    NULL,
				    &g_uiAuthDataLen,
				    &g_pAuthData);
  if ((XID) ~0L == g_authId)
    {
      ErrorF ("winGenerateAuthorization - GenerateAuthorization failed\n");
      goto auth_bailout;
    }
#if 0
  else
    {
      ErrorF ("winGenerateAuthorization - GenerateAuthorization success!\n"
	      "AuthDataLen: %d AuthData: %s\n",
	      g_uiAuthDataLen, g_pAuthData);
    }
#endif
  
  /* Allocate structure for additional auth information */
  pAuth = (SecurityAuthorizationPtr) 
    xalloc (sizeof (SecurityAuthorizationRec));
  if (!(pAuth))
    {
      ErrorF ("winGenerateAuthorization - Failed allocating "
	      "SecurityAuthorizationPtr.\n");
      goto auth_bailout;
    }
  
  /* Fill in the auth fields */
  pAuth->id = g_authId;
  pAuth->timeout = 0; /* live for x seconds after refcnt == 0 */
  pAuth->group = None;
  pAuth->trustLevel = XSecurityClientTrusted;
  pAuth->refcnt = 1; /* this auth must stick around */
  pAuth->secondsRemaining = 0;
  pAuth->timer = NULL;
  pAuth->eventClients = NULL;
  
  /* Add the authorization to the server's auth list */
  if (!AddResource (g_authId,
		    SecurityAuthorizationResType,
		    pAuth))
    {
      ErrorF ("winGenerateAuthorization - AddResource failed for auth.\n");
      fFreeAuth = TRUE;
      goto auth_bailout;
    }
  
  /* Don't free the auth data, since it is still used internally */
  pAuth = NULL;
  
  return TRUE;

 auth_bailout:
  if (fFreeAuth)
    xfree (pAuth);
  
  return FALSE;
}
#endif
