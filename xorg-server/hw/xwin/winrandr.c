/*
 *Copyright (C) 2001-2004 Harold L Hunt II All Rights Reserved.
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

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"


/*
 * Local prototypes
 */

static Bool
winRandRGetInfo (ScreenPtr pScreen, Rotation *pRotations);

static Bool
winRandRSetConfig (ScreenPtr		pScreen,
		   Rotation		rotateKind,
		   int			rate,
		   RRScreenSizePtr	pSize);

Bool
winRandRInit (ScreenPtr pScreen);


/*
 * Answer queries about the RandR features supported.
 */

static Bool
winRandRGetInfo (ScreenPtr pScreen, Rotation *pRotations)
{
  winScreenPriv(pScreen);
  winScreenInfo			*pScreenInfo = pScreenPriv->pScreenInfo;
  int				n;
  Rotation			rotateKind;
  RRScreenSizePtr		pSize;

  winDebug ("winRandRGetInfo ()\n");

  /* Don't support rotations, yet */
  *pRotations = RR_Rotate_0;

  /* Bail if no depth has a visual associated with it */
  for (n = 0; n < pScreen->numDepths; n++)
    if (pScreen->allowedDepths[n].numVids)
      break;
  if (n == pScreen->numDepths)
    return FALSE;

  /* Only one allowed rotation for now */
  rotateKind = RR_Rotate_0;

  /*
   * Register supported sizes.  This can be called many times, but
   * we only support one size for now.
   */
  pSize = RRRegisterSize (pScreen,
			  pScreenInfo->dwWidth,
			  pScreenInfo->dwHeight,
			  pScreenInfo->dwWidth_mm,
			  pScreenInfo->dwHeight_mm);

  /* Tell RandR what the current config is */
  RRSetCurrentConfig (pScreen,
		      rotateKind,
		      0, /* refresh rate, not needed */
		      pSize);
  
  return TRUE;
}


/*
 * Respond to resize/rotate request from either X Server or X client app
 */

static Bool
winRandRSetConfig (ScreenPtr		pScreen,
		   Rotation		rotateKind,
		   int			rate,
		   RRScreenSizePtr	pSize)
{
  winDebug ("winRandRSetConfig ()\n");

  return TRUE;
}


/*
 * Initialize the RandR layer.
 */

Bool
winRandRInit (ScreenPtr pScreen)
{
  rrScrPrivPtr		pRRScrPriv;

  winDebug ("winRandRInit ()\n");

  if (!RRScreenInit (pScreen))
    {
      ErrorF ("winRandRInit () - RRScreenInit () failed\n");
      return FALSE;
    }

  /* Set some RandR function pointers */
  pRRScrPriv = rrGetScrPriv (pScreen);
  pRRScrPriv->rrGetInfo = winRandRGetInfo;
  pRRScrPriv->rrSetConfig = winRandRSetConfig;

  return TRUE;
}
