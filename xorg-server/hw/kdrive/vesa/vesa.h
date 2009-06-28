/* 
Copyright (c) 2000 by Juliusz Chroboczek
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions: 
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _VESA_H_
#define _VESA_H_

#include "kdrive.h"
#include "shadow.h"
#include "vm86.h"
#ifdef RANDR
#include "randrstr.h"
#endif

#define VESA_TEXT_SAVE	(64*1024)

#define MODE_SUPPORTED	0x01
#define MODE_COLOUR	0x08
#define MODE_GRAPHICS	0x10
#define MODE_VGA	0x20
#define MODE_LINEAR	0x80

#define MODE_DIRECT	0x1

#define MEMORY_TEXT	0
#define MEMORY_CGA	1
#define MEMORY_HERCULES	2
#define MEMORY_PLANAR	3
#define MEMORY_PSEUDO	4
#define MEMORY_NONCHAIN	5
#define MEMORY_DIRECT	6
#define MEMORY_YUV	7

typedef struct _VesaMode {
    int		mode;			/* mode number */
    int		vbe;			/* a VBE mode */
    int		ModeAttributes;		/* mode attributes */
    int		NumberOfPlanes;		/* number of memory planes */
    int		BitsPerPixel;		/* bits per pixel */
    int		MemoryModel;		/* memory model type */
    int		RedMaskSize;            /* size of direct color red mask in bits */
    int		RedFieldPosition;       /* bit position of lsb of red mask */
    int		GreenMaskSize;          /* size of direct color green mask in bits */
    int		GreenFieldPosition;     /* bit position of lsb of green mask */
    int		BlueMaskSize;           /* size of direct color blue mask in bits */
    int		BlueFieldPosition;      /* bit position of lsb of blue mask */
    int		RsvdMaskSize;           /* size of direct color reserved mask bits*/
    int		RsvdFieldPosition;      /* bit position of lsb of reserved mask */
    int		DirectColorModeInfo;    /* direct color mode attributes */
    int		XResolution;            /* horizontal resolution */
    int		YResolution;            /* vertical resolution */
    int		BytesPerScanLine;       /* bytes per scan line */
} VesaModeRec, *VesaModePtr;

#include "vbe.h"
#include "vga.h"

typedef struct _VesaCardPriv {
    int		vbe;
    VesaModePtr modes;
    int		nmode;
    Vm86InfoPtr	vi;
    int		vga_palette;
    int		old_vbe_mode;
    int		old_vga_mode;
    VbeInfoPtr	vbeInfo;
    char	text[VESA_TEXT_SAVE];
    CARD8	cmap[256*4];
} VesaCardPrivRec, *VesaCardPrivPtr;

#define VESA_LINEAR	0
#define VESA_WINDOWED	1
#define VESA_PLANAR	2
#define VESA_MONO	3

typedef struct _VesaScreenPriv {
    VesaModeRec	mode;
    Bool	shadow;
    Rotation	randr;
    int		mapping;
    int		origDepth;
    void	*fb;
    int		fb_size;
    CARD32	fb_phys;
    PixmapPtr	pShadow;
    Bool	mapped;
} VesaScreenPrivRec, *VesaScreenPrivPtr;

extern int vesa_video_mode;
extern Bool vesa_force_mode;

void
vesaReportMode (VesaModePtr mode);

VesaModePtr
vesaGetModes (Vm86InfoPtr vi, int *ret_nmode);

void
vesaTestMode (void);

void *
vesaSetWindowPlanar(ScreenPtr pScreen,
		  CARD32    row,
		  CARD32    offset,
		  int	    mode,
		  CARD32    *size);

void *
vesaSetWindowLinear (ScreenPtr	pScreen,
		     CARD32	row,
		     CARD32	offset,
		     int	mode,
		     CARD32	*size);

void *
vesaSetWindowWindowed (ScreenPtr    pScreen,
		       CARD32	    row,
		       CARD32	    offset,
		       int	    mode,
		       CARD32	    *size);

void *
vesaWindowPlanar (ScreenPtr pScreen,
		  CARD32    row,
		  CARD32    offset,
		  int	    mode,
		  CARD32    *size,
		  void	    *closure);

void *
vesaWindowLinear (ScreenPtr pScreen,
		  CARD32    row,
		  CARD32    offset,
		  int	    mode,
		  CARD32    *size,
		  void	    *closure);

void *
vesaWindowWindowed (ScreenPtr	pScreen,
		    CARD32	row,
		    CARD32	offset,
		    int		mode,
		    CARD32	*size,
		    void	*closure);

void *
vesaWindowCga (ScreenPtr    pScreen,
	       CARD32	    row,
	       CARD32	    offset,
	       int	    mode,
	       CARD32	    *size,
	       void	    *closure);

void
vesaUpdateMono (ScreenPtr	pScreen,
		shadowBufPtr	pBuf);

Bool
vesaCreateColormap16 (ColormapPtr pmap);

void
vesaSetScreenSizes (ScreenPtr pScreen);

Bool
vesaSetShadow (ScreenPtr pScreen);


void
vesaListModes(void);

Bool
vesaInitialize(KdCardInfo *card, VesaCardPrivPtr priv);

Bool
vesaCardInit(KdCardInfo *card);

int
vesaDepth (VesaModePtr mode);

Bool
vesaModeGood (KdScreenInfo  *screen,
	      VesaModePtr   a);

int
vesaSizeError (KdScreenInfo *screen,
	       VesaModePtr  a);

Bool
vesaModeBetter (KdScreenInfo	*screen,
		VesaModePtr	a,
		VesaModePtr	b);

VesaModePtr
vesaSelectMode (KdScreenInfo *screen);

Bool
vesaInitialize (KdCardInfo *card, VesaCardPrivPtr priv);

Bool
vesaScreenInitialize (KdScreenInfo *screen, VesaScreenPrivPtr pscr);

Bool
vesaScreenInit(KdScreenInfo *screen);    

PixmapPtr
vesaGetPixmap (ScreenPtr pScreen);

Bool
vesaInitScreen(ScreenPtr pScreen);

Bool
vesaFinishInitScreen(ScreenPtr pScreen);

Bool
vesaCreateResources (ScreenPtr pScreen);

Bool
vesaSetMode (ScreenPtr	    pScreen,
	     VesaModePtr    mode);

Bool
vesaEnable(ScreenPtr pScreen);

Bool
vesaDPMS (ScreenPtr pScreen, int mode);

void
vesaDisable(ScreenPtr pScreen);

void
vesaPreserve(KdCardInfo *card);

void
vesaRestore(KdCardInfo *card);

void
vesaCardFini(KdCardInfo *card);

void
vesaScreenFini(KdScreenInfo *screen);

int 
vesaSetPalette(VesaCardPrivPtr priv, int first, int number, U8 *entries);

int 
vesaGetPalette(VesaCardPrivPtr priv, int first, int number, U8 *entries);

void
vesaPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

void
vesaGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

void
vesaUseMsg (void);

int
vesaProcessArgument (int argc, char **argv, int i);

#ifdef RANDR
Bool
vesaRandRGetInfo (ScreenPtr pScreen, Rotation *rotations);

Bool
vesaRandRSetConfig (ScreenPtr		pScreen,
		    Rotation		randr,
		    int			rate,
		    RRScreenSizePtr	pSize);
Bool
vesaRandRInit (ScreenPtr pScreen);

#endif

Bool
toshibaDPMS (ScreenPtr pScreen, int mode);

#endif /* _VESA_H_ */
