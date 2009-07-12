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

#include "propertyst.h"
#include "windowstr.h"


/*
 * External global variables
 */

extern HICON		g_hIconX;
extern HICON		g_hSmallIconX;


/*
 * Prototypes for local functions
 */

static void
winScaleIconToWindows (int iconSize,
		       int effBPP,
		       unsigned char *iconData,
		       unsigned short width,
		       unsigned short height,
		       int xStride,
		       int effXBPP,
		       int effXDepth,
		       unsigned char *image);

/*
 * Scale an X icon bitmap into a Windoze icon bitmap
 */

static void
winScaleXBitmapToWindows (int iconSize,
			  int effBPP,
			  PixmapPtr pixmap,
			  unsigned char *image)
{
  int effXBPP, effXDepth;
  int xStride;
  unsigned char *iconData = 0;

  effXBPP = BitsPerPixel(pixmap->drawable.depth);
  effXDepth = pixmap->drawable.depth;

  if (pixmap->drawable.bitsPerPixel == 15)
    effXBPP = 16;
  
  if (pixmap->drawable.depth == 15)
    effXDepth = 16;

  xStride = PixmapBytePad (pixmap->drawable.width, pixmap->drawable.depth);
  if (xStride == 0)
    {
      ErrorF ("winScaleXBitmapToWindows - xStride is zero.  "
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

  winScaleIconToWindows(iconSize, effBPP,
			iconData,
			pixmap->drawable.width, pixmap->drawable.height,
			xStride, effXBPP, effXDepth,
			image);

  free (iconData);
}

/*
 * Scale a drawable into a Windoze icon bitmap
 */

static void
winScaleIconToWindows (int iconSize,
		       int effBPP,
		       unsigned char *iconData,
		       unsigned short width,
		       unsigned short height,
		       int xStride,
		       int effXBPP,
		       int effXDepth,
		       unsigned char *image)
{
  int			row, column;
  unsigned char		*outPtr;
  int			stride;
  float                 factX, factY;
  int			posX, posY;
  unsigned char		*ptr;
  unsigned int		zero;
  unsigned int		color;

  winDebug("winScaleIconToWindows: scaling from %d x %d @ %d bpp (depth %d) to %d x %d @ %d bpp\n",
	   width, height, effXBPP, effXDepth, iconSize, iconSize, effBPP);

  /* Need 16-bit aligned rows for DDBitmaps */
  stride = ((iconSize * effBPP + 15) & (~15)) / 8;
  if (stride == 0)
    {
      ErrorF ("winScaleXBitmapToWindows - stride is zero.  "
	      "Bailing.\n");
      return;
    }

  /* Keep aspect ratio */
  factX = ((float)width) / ((float)iconSize);
  factY = ((float)height) / ((float)iconSize);
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
	      if (posX >= width
		  || posY >= height)
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
	      if (posX >= width
		  || posY >= height)
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
		  *(outPtr++) = (effXDepth == 32) ? *(ptr++) : 0x0; // alpha
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
	      if (posX >= width
		  || posY >= height)
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
}

static pointer
GetWindowProp(WindowPtr pWin, Atom name, long int *size_return)
{
  struct _Window	*pwin;
  struct _Property	*prop;

  if (!pWin || !name) {
    winDebug("GetWindowProp - pWin or name was NULL\n");
    return 0;
  }
  pwin = (struct _Window*) pWin;
  if (!pwin->optional) return NULL;
  for (prop = (struct _Property *) pwin->optional->userProps;
       prop;
       prop=prop->next){
    if (prop->propertyName == name) {
      *size_return=prop->size;
      return prop->data;
    }
  }
  return NULL;
}

static void
winScaleNetWMIconToWindows (int iconSize,
			    int effBPP,
			    uint32_t *icondata,
			    unsigned char *image)
{
  int height, width;
  uint32_t *pixels;

  width = icondata[0];
  height = icondata[1];
  pixels = &icondata[2];

  winScaleIconToWindows(iconSize, effBPP,
			(unsigned char *)pixels,
			width, height,
			width*4, 32, 32,
			image);
}

/*
 * Attempt to create a custom icon from the WM_HINTS bitmaps
 */

HICON
winXIconToHICON (WindowPtr pWin, int iconSize)
{
  unsigned char		*mask, *image;
  unsigned char		*dst, *src;
  PixmapPtr		iconPtr;
  PixmapPtr		maskPtr;
  int			planes, bpp, effBPP, stride, maskStride, i;
  HDC			hDC;
  ICONINFO		ii;
  WinXWMHints		hints;
  HICON			hIcon = NULL;
  Bool net_wm_icon_found = FALSE;

  static Atom _XA_NET_WM_ICON = 0;
  uint32_t *icon, *icon_data;
  long int size=0;
  
  hDC = GetDC (GetDesktopWindow ());
  planes = GetDeviceCaps (hDC, PLANES);
  bpp = GetDeviceCaps (hDC, BITSPIXEL);
  ReleaseDC (GetDesktopWindow (), hDC);
  
  /* 15 BPP is really 16BPP as far as we care */
  if (bpp == 15)
    effBPP = 16;
  else
    effBPP = bpp;
  
  /* Need 16-bit aligned rows for DDBitmaps */
  stride = ((iconSize * effBPP + 15) & (~15)) / 8;

  /* Mask is 1-bit deep */
  maskStride = ((iconSize * 1 + 15) & (~15)) / 8;

  if (!_XA_NET_WM_ICON) _XA_NET_WM_ICON = MakeAtom("_NET_WM_ICON", 12, FALSE);
  
  /* Always prefer _NET_WM_ICON icons */
  icon_data = GetWindowProp(pWin, _XA_NET_WM_ICON, &size);
  if (icon_data)
    {
      uint32_t *best_icon = 0;
      unsigned int best_size = 0;

      /*
	 For scaling to the required size, choose the smallest icon which is
	 bigger than or equal to the required size, failing that, the biggest
	 icon which is smaller than the required size
      */
      for(icon = icon_data;
 	  icon < &icon_data[size] && *icon;
 	  icon = &icon[icon[0]*icon[1]+2])
	{
	  unsigned int candidateSize = (icon[0] + icon[1])/2;
	  winDebug("winXIconToHICON: pWin%x found %lu x %lu NetIcon\n",(int)pWin,icon[0],icon[1]);

	  if (((best_size < iconSize) && ((candidateSize > best_size) || (candidateSize >= iconSize)))
	      || ((best_size > iconSize) && (candidateSize >= iconSize) && (candidateSize < best_size)))
	    {
	      best_icon = icon;
	      best_size = candidateSize;
	    }
	}

      if (best_icon)
	{
	  winDebug("winXIconToHICON: pWin%x selected %lu x %lu NetIcon for scaling to %u x %u\n",
		   (int)pWin, best_icon[0], best_icon[1], iconSize, iconSize );

	  image = malloc (stride * iconSize);

	  /* Use a completely black mask, image has alpha */
	  mask = calloc (maskStride, iconSize);

	  winScaleNetWMIconToWindows(iconSize, effBPP, best_icon, image);
	  net_wm_icon_found = TRUE;
 	}
      else
	{
	  winDebug("winXIconToHICON: pWin %x no %d x %d NetIcon\n",(int)pWin,iconSize,iconSize);
	}
    }

  if (!net_wm_icon_found)
    {
      unsigned char *imageMask;

      winMultiWindowGetWMHints (pWin, &hints);
      winDebug("winXIconToHICON: pWin 0x%x icon_pixmap hint %x\n", pWin, hints.icon_pixmap);
      if (!hints.icon_pixmap) return NULL;

      iconPtr = (PixmapPtr) LookupIDByType (hints.icon_pixmap, RT_PIXMAP);
      winDebug("winXIconToHICON: pWin 0x%x iconPtr 0x%x\n", pWin, iconPtr);

      if (!iconPtr) return NULL;

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
      free (imageMask);
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
  HICON			hIcon, hIconSmall, hiconOld;

  pWin = (WindowPtr) LookupIDByType (id, RT_WINDOW);
  if (!pWin) return;
{
  winWindowPriv(pWin);
  if (!pWinPriv->hWnd) return;

  hIcon = (HICON)winOverrideIcon ((unsigned long)pWin);

  if (!hIcon)
    hIcon = winXIconToHICON (pWin, GetSystemMetrics(SM_CXICON));

  if (hIcon)
    {
      hiconOld = (HICON) SetClassLong (pWinPriv->hWnd,
				       GCL_HICON,
				       (int) hIcon);

      /* Delete the icon if its not the default */
      if (hiconOld !=  g_hIconX)
	winDestroyIcon(hiconOld);
    }
 
  hIconSmall = (HICON)winOverrideIcon ((unsigned long)pWin);

  if (!hIconSmall)
    hIconSmall = winXIconToHICON (pWin, GetSystemMetrics(SM_CXSMICON));

  if (hIconSmall)
     {
      hiconOld = (HICON) SetClassLong (pWinPriv->hWnd,
				       GCL_HICONSM,
				       (int) hIconSmall);
      if (hiconOld != g_hSmallIconX)
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
