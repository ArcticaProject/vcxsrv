/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
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
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Earle F. Philhower, III
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include "dixevents.h"
#include "winmultiwindowclass.h"
#include "winprefs.h"


/*
 * External global variables
 */

extern HICON		g_hIconX;
extern HICON		g_hSmallIconX;


/*
 * Prototypes for local functions
 */

static void
winScaleXBitmapToWindows (int iconSize, int effBPP,
			  PixmapPtr pixmap, unsigned char *image);


/*
 * Scale an X icon bitmap into a Windoze icon bitmap
 */

static void
winScaleXBitmapToWindows (int iconSize,
			  int effBPP,
			  PixmapPtr pixmap,
			  unsigned char *image)
{
  int			row, column, effXBPP, effXDepth;
  unsigned char		*outPtr;
  unsigned char		*iconData = 0;
  int			stride, xStride;
  float			factX, factY;
  int			posX, posY;
  unsigned char		*ptr;
  unsigned int		zero;
  unsigned int		color;

  effXBPP = BitsPerPixel(pixmap->drawable.depth);
  effXDepth = pixmap->drawable.depth;

  if (pixmap->drawable.bitsPerPixel == 15)
    effXBPP = 16;
  
  if (pixmap->drawable.depth == 15)
    effXDepth = 16;

  /* Need 32-bit aligned rows */
  stride = ((iconSize * effBPP + 31) & (~31)) / 8;
  xStride = PixmapBytePad (pixmap->drawable.width, pixmap->drawable.depth);
  if (stride == 0 || xStride == 0)
    {
      ErrorF ("winScaleXBitmapToWindows - stride or xStride is zero.  "
	      "Bailing.\n");
      return;
    }

  /* Allocate memory for icon data */
  iconData = malloc (xStride * pixmap->drawable.height);
  if (!iconData)
    {
      ErrorF ("winScaleXBitmapToWindows - malloc failed for iconData.  "
	      "Bailing.\n");
      return;
    }

  /* Get icon data */
  miGetImage ((DrawablePtr) &(pixmap->drawable), 0, 0,
	      pixmap->drawable.width, pixmap->drawable.height,
	      ZPixmap, 0xffffffff, iconData);

  /* Keep aspect ratio */
  factX = ((float)pixmap->drawable.width) / ((float)iconSize);
  factY = ((float)pixmap->drawable.height) / ((float)iconSize);
  if (factX > factY)
    factY = factX;
  else
    factX = factY;
  
  /* Out-of-bounds, fill icon with zero */
  zero = 0;
 
  for (row = 0; row < iconSize; row++)
    {
      outPtr = image + stride * row;
      for (column = 0; column < iconSize; column++)
	{
	  posX = factX * column;
	  posY = factY * row;
	  
	  ptr = iconData + posY*xStride;
	  if (effXBPP == 1)
	    {
	      ptr += posX / 8;
	      
	      /* Out of X icon bounds, leave space blank */
	      if (posX >= pixmap->drawable.width
		  || posY >= pixmap->drawable.height)
		ptr = (unsigned char *) &zero;
	      
	      if ((*ptr) & (1 << (posX & 7)))
		switch (effBPP)
		  {
		  case 32:
		    *(outPtr++) = 0;
		  case 24:
		    *(outPtr++) = 0;
		  case 16:
		    *(outPtr++) = 0;
		  case 8:
		    *(outPtr++) = 0;
		    break;
		  case 1:
		    outPtr[column / 8] &= ~(1 << (7 - (column & 7)));
		    break;
		  }
	      else
		switch (effBPP)
		  {
		  case 32:
		    *(outPtr++) = 255;
		    *(outPtr++) = 255;
		    *(outPtr++) = 255;
		    *(outPtr++) = 0;
		    break;
		  case 24:
		    *(outPtr++) = 255;
		  case 16:
		    *(outPtr++) = 255;
		  case 8: 
		    *(outPtr++) = 255;
		    break;
		  case 1:
		    outPtr[column / 8] |= (1 << (7 - (column & 7)));
		    break;
		  }
	    }
	  else if (effXDepth == 24 || effXDepth == 32)
	    {
	      ptr += posX * (effXBPP / 8);

	      /* Out of X icon bounds, leave space blank */
	      if (posX >= pixmap->drawable.width
		  || posY >= pixmap->drawable.height)
		ptr = (unsigned char *) &zero;
	      color = (((*ptr) << 16)
		       + ((*(ptr + 1)) << 8)
		       + ((*(ptr + 2)) << 0));
	      switch (effBPP)
		{
		case 32:
		  *(outPtr++) = *(ptr++); // b
		  *(outPtr++) = *(ptr++); // g
		  *(outPtr++) = *(ptr++); // r
		  *(outPtr++) = 0; // resvd
		  break;
		case 24:
		  *(outPtr++) = *(ptr++);
		  *(outPtr++) = *(ptr++);
		  *(outPtr++) = *(ptr++);
		  break;
		case 16:
		  color = ((((*ptr) >> 2) << 10)
			   + (((*(ptr + 1)) >> 2) << 5)
			   + (((*(ptr + 2)) >> 2)));
		  *(outPtr++) = (color >> 8);
		  *(outPtr++) = (color & 255);
		  break;
		case 8:
		  color = (((*ptr))) + (((*(ptr + 1)))) + (((*(ptr + 2))));
		  color /= 3;
		  *(outPtr++) = color;
		  break;
		case 1:
		  if (color)
		    outPtr[column / 8] |= (1 << (7 - (column & 7)));
		  else
		    outPtr[column / 8] &= ~(1 << (7 - (column & 7)));
		}
	    }
	  else if (effXDepth == 16)
	    {
	      ptr += posX * (effXBPP / 8);
	
	      /* Out of X icon bounds, leave space blank */
	      if (posX >= pixmap->drawable.width
		  || posY >= pixmap->drawable.height)
		ptr = (unsigned char *) &zero;
	      color = ((*ptr) << 8) + (*(ptr + 1));
	      switch (effBPP)
		{
		case 32:
		  *(outPtr++) = (color & 31) << 2;
		  *(outPtr++) = ((color >> 5) & 31) << 2;
		  *(outPtr++) = ((color >> 10) & 31) << 2;
		  *(outPtr++) = 0; // resvd
		  break;
		case 24:
		  *(outPtr++) = (color & 31) << 2;
		  *(outPtr++) = ((color >> 5) & 31) << 2;
		  *(outPtr++) = ((color >> 10) & 31) << 2;
		  break;
		case 16:
		  *(outPtr++) = *(ptr++);
		  *(outPtr++) = *(ptr++);
		  break;
		case 8:
		  *(outPtr++) = (((color & 31)
				  + ((color >> 5) & 31)
				  + ((color >> 10) & 31)) / 3) << 2;
		  break;
		case 1:
		  if (color)
		    outPtr[column / 8] |= (1 << (7 - (column & 7)));
		  else
		    outPtr[column / 8] &= ~(1 << (7 - (column & 7)));
		  break;
		} /* end switch(effbpp) */
	    } /* end if effxbpp==16) */
	} /* end for column */
    } /* end for row */
  free (iconData);
}


/*
 * Attempt to create a custom icon from the WM_HINTS bitmaps
 */

HICON
winXIconToHICON (WindowPtr pWin, int iconSize)
{
  unsigned char		*mask, *image, *imageMask;
  unsigned char		*dst, *src;
  PixmapPtr		iconPtr;
  PixmapPtr		maskPtr;
  int			planes, bpp, effBPP, stride, maskStride, i;
  HDC			hDC;
  ICONINFO		ii;
  WinXWMHints		hints;
  HICON			hIcon;

  winMultiWindowGetWMHints (pWin, &hints);
  if (!hints.icon_pixmap) return NULL;

  iconPtr = (PixmapPtr) LookupIDByType (hints.icon_pixmap, RT_PIXMAP);
  
  if (!iconPtr) return NULL;
  
  hDC = GetDC (GetDesktopWindow ());
  planes = GetDeviceCaps (hDC, PLANES);
  bpp = GetDeviceCaps (hDC, BITSPIXEL);
  ReleaseDC (GetDesktopWindow (), hDC);
  
  /* 15 BPP is really 16BPP as far as we care */
  if (bpp == 15)
    effBPP = 16;
  else
    effBPP = bpp;
  
  /* Need 32-bit aligned rows */
  stride = ((iconSize * effBPP + 31) & (~31)) / 8;

  /* Mask is 1-bit deep */
  maskStride = ((iconSize * 1 + 31) & (~31)) / 8; 

  image = malloc (stride * iconSize);
  imageMask = malloc (stride * iconSize);
  /* Default to a completely black mask */
  mask = calloc (maskStride, iconSize);
  
  winScaleXBitmapToWindows (iconSize, effBPP, iconPtr, image);
  maskPtr = (PixmapPtr) LookupIDByType (hints.icon_mask, RT_PIXMAP);

  if (maskPtr) 
    {
      winScaleXBitmapToWindows (iconSize, 1, maskPtr, mask);
      
      winScaleXBitmapToWindows (iconSize, effBPP, maskPtr, imageMask);
      
      /* Now we need to set all bits of the icon which are not masked */
      /* on to 0 because Color is really an XOR, not an OR function */
      dst = image;
      src = imageMask;

      for (i = 0; i < (stride * iconSize); i++)
	if ((*(src++)))
	  *(dst++) = 0;
	else
	  dst++;
    }
  
  ii.fIcon = TRUE;
  ii.xHotspot = 0; /* ignored */
  ii.yHotspot = 0; /* ignored */
  
  /* Create Win32 mask from pixmap shape */
  ii.hbmMask = CreateBitmap (iconSize, iconSize, planes, 1, mask);

  /* Create Win32 bitmap from pixmap */
  ii.hbmColor = CreateBitmap (iconSize, iconSize, planes, bpp, image);

  /* Merge Win32 mask and bitmap into icon */
  hIcon = CreateIconIndirect (&ii);

  /* Release Win32 mask and bitmap */
  DeleteObject (ii.hbmMask);
  DeleteObject (ii.hbmColor);

  /* Free X mask and bitmap */
  free (mask);
  free (image);
  free (imageMask);

  return hIcon;
}



/*
 * Change the Windows window icon 
 */

#ifdef XWIN_MULTIWINDOW
void
winUpdateIcon (Window id)
{
  WindowPtr		pWin;
  HICON			hIcon, hiconOld;

  pWin = (WindowPtr) LookupIDByType (id, RT_WINDOW);
  if (!pWin) return;
  hIcon = (HICON)winOverrideIcon ((unsigned long)pWin);

  if (!hIcon)
    hIcon = winXIconToHICON (pWin, GetSystemMetrics(SM_CXICON));

  if (hIcon)
    {
      winWindowPriv(pWin);

      if (pWinPriv->hWnd)
	{
	  hiconOld = (HICON) SetClassLong (pWinPriv->hWnd,
					   GCL_HICON,
					   (int) hIcon);
	  
	  /* Delete the icon if its not the default */
	  winDestroyIcon(hiconOld);
	}
    }
 
  hIcon = winXIconToHICON (pWin, GetSystemMetrics(SM_CXSMICON));
  if (hIcon)
    {
      winWindowPriv(pWin);

      if (pWinPriv->hWnd)
	{
	  hiconOld = (HICON) SetClassLong (pWinPriv->hWnd,
					   GCL_HICONSM,
					   (int) hIcon);
	  winDestroyIcon (hiconOld);
	}
    }
}

void winInitGlobalIcons (void)
{
  int sm_cx = GetSystemMetrics(SM_CXICON);
  int sm_cxsm = GetSystemMetrics(SM_CXSMICON);
  /* Load default X icon in case it's not ready yet */
  if (!g_hIconX) 
    {  
      g_hIconX = (HICON)winOverrideDefaultIcon(sm_cx);
      g_hSmallIconX = (HICON)winOverrideDefaultIcon(sm_cxsm);
    }
  
  if (!g_hIconX)
    {   
      g_hIconX = (HICON)LoadImage (g_hInstance,
	      MAKEINTRESOURCE(IDI_XWIN),
	      IMAGE_ICON,
	      GetSystemMetrics(SM_CXICON),
	      GetSystemMetrics(SM_CYICON),
	      0);
      g_hSmallIconX = (HICON)LoadImage (g_hInstance,
	      MAKEINTRESOURCE(IDI_XWIN),
	      IMAGE_ICON,
	      GetSystemMetrics(SM_CXSMICON),
	      GetSystemMetrics(SM_CYSMICON),
	      LR_DEFAULTSIZE);
    }
}

void winSelectIcons(WindowPtr pWin, HICON *pIcon, HICON *pSmallIcon)
{
  HICON hIcon, hSmallIcon;
  
  winInitGlobalIcons();  
  
  /* Try and get the icon from WM_HINTS */
  hIcon = winXIconToHICON (pWin, GetSystemMetrics(SM_CXICON));
  hSmallIcon = winXIconToHICON (pWin, GetSystemMetrics(SM_CXSMICON));

  /* If we got the small, but not the large one swap them */
  if (!hIcon && hSmallIcon) 
  {
      hIcon = hSmallIcon;
      hSmallIcon = NULL;
  }
  
  /* Use default X icon if no icon loaded from WM_HINTS */
  if (!hIcon) {
    hIcon = g_hIconX;
    hSmallIcon = g_hSmallIconX;
  }

  if (pIcon)
    *pIcon = hIcon;
  else
    winDestroyIcon(hIcon);
  if (pSmallIcon)
    *pSmallIcon = hSmallIcon;
  else
    winDestroyIcon(hSmallIcon);
}

void winDestroyIcon(HICON hIcon)
{
  /* Delete the icon if its not the default */
  if (hIcon &&
      hIcon != g_hIconX &&
      hIcon != g_hSmallIconX &&
      !winIconIsOverride((unsigned long)hIcon))
    DestroyIcon (hIcon);
}
#endif
