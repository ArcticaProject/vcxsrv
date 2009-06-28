/*
 * Copyright © 1999 Keith Packard
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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#ifdef PSEUDO8
#include "pseudo8/pseudo8.h"
#endif
#include <mivalidate.h>
#include <dixstruct.h>
#include "privates.h"
#ifdef RANDR
#include <randrstr.h>
#endif

#ifdef XV
#include "kxv.h"
#endif

#ifdef DPMSExtension
#include "dpmsproc.h"
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include <signal.h>

typedef struct _kdDepths {
    CARD8   depth;
    CARD8   bpp;
} KdDepths;

KdDepths    kdDepths[] = {
    { 1, 1 },
    { 4, 4 },
    { 8, 8 },
    { 15, 16 },
    { 16, 16 },
    { 24, 32 },
    { 32, 32 }
};

#define NUM_KD_DEPTHS (sizeof (kdDepths) / sizeof (kdDepths[0]))

#define KD_DEFAULT_BUTTONS 5

DevPrivateKey       kdScreenPrivateKey = &kdScreenPrivateKey;
unsigned long	    kdGeneration;

Bool                kdVideoTest;
unsigned long       kdVideoTestTime;
Bool		    kdEmulateMiddleButton;
Bool		    kdRawPointerCoordinates;
Bool		    kdDisableZaphod;
Bool                kdDontZap;
Bool		    kdEnabled;
int		    kdSubpixelOrder;
int		    kdVirtualTerminal = -1;
Bool		    kdSwitchPending;
char		    *kdSwitchCmd;
DDXPointRec	    kdOrigin;
Bool		    kdHasPointer = FALSE;
Bool		    kdHasKbd = FALSE;

static Bool         kdCaughtSignal = FALSE;

/*
 * Carry arguments from InitOutput through driver initialization
 * to KdScreenInit
 */

KdOsFuncs	*kdOsFuncs;
extern WindowPtr *WindowTable;

void
KdSetRootClip (ScreenPtr pScreen, BOOL enable)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    WindowPtr	pChild;
    Bool	WasViewable;
    Bool	anyMarked = FALSE;
    RegionPtr	pOldClip = 0;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr   pLayerWin;
    BoxRec	box;

    if (!pWin)
	return;
    WasViewable = (Bool)(pWin->viewable);
    if (WasViewable)
    {
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    (void) (*pScreen->MarkOverlappedWindows)(pChild,
						     pChild,
						     &pLayerWin);
	}
	(*pScreen->MarkWindow) (pWin);
	anyMarked = TRUE;
	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr	borderVisible;

		borderVisible = REGION_CREATE(pScreen, NullBox, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				&pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    if (enable)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pScreen->width;
	box.y2 = pScreen->height;
	pWin->drawable.width = pScreen->width;
	pWin->drawable.height = pScreen->height;
	REGION_INIT (pScreen, &pWin->winSize, &box, 1);
	REGION_INIT (pScreen, &pWin->borderSize, &box, 1);
	REGION_RESET(pScreen, &pWin->borderClip, &box);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    else
    {
	REGION_EMPTY(pScreen, &pWin->borderClip);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    
    ResizeChildrenWinSize (pWin, 0, 0, 0, 0);
    
    if (WasViewable)
    {
	if (pWin->firstChild)
	{
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
							   pWin->firstChild,
							   (WindowPtr *)NULL);
	}
	else
	{
	    (*pScreen->MarkWindow) (pWin);
	    anyMarked = TRUE;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
    }

    if (WasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures)(pWin);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
}

void
KdDisableScreen (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    
    if (!pScreenPriv->enabled)
	return;
    if (!pScreenPriv->closed)
	KdSetRootClip (pScreen, FALSE);
    KdDisableColormap (pScreen);
    KdOffscreenSwapOut (pScreen);
    if (!pScreenPriv->screen->dumb && pScreenPriv->card->cfuncs->disableAccel)
	(*pScreenPriv->card->cfuncs->disableAccel) (pScreen);
    if (!pScreenPriv->screen->softCursor && pScreenPriv->card->cfuncs->disableCursor)
	(*pScreenPriv->card->cfuncs->disableCursor) (pScreen);
    if (pScreenPriv->card->cfuncs->dpms)
	(*pScreenPriv->card->cfuncs->dpms) (pScreen, KD_DPMS_NORMAL);
    pScreenPriv->enabled = FALSE;
    if(pScreenPriv->card->cfuncs->disable)
        (*pScreenPriv->card->cfuncs->disable) (pScreen);
}

static void
KdDoSwitchCmd (char *reason)
{
    if (kdSwitchCmd)
    {
	char    *command = xalloc (strlen (kdSwitchCmd) +
				   1 +
				   strlen (reason) + 
				   1);
	if (!command)
	    return;
	strcpy (command, kdSwitchCmd);
	strcat (command, " ");
	strcat (command, reason);
	system (command);
	xfree (command);
    }
}

void
KdSuspend (void)
{
    KdCardInfo	    *card;
    KdScreenInfo    *screen;

    if (kdEnabled)
    {
	for (card = kdCardInfo; card; card = card->next)
	{
	    for (screen = card->screenList; screen; screen = screen->next)
		if (screen->mynum == card->selected && screen->pScreen)
		    KdDisableScreen (screen->pScreen);
	    if (card->driver && card->cfuncs->restore)
		(*card->cfuncs->restore) (card);
	}
	KdDisableInput ();
	KdDoSwitchCmd ("suspend");
    }
}

void
KdDisableScreens (void)
{
    KdSuspend ();
    if (kdEnabled)
    {
        if (kdOsFuncs->Disable)
            (*kdOsFuncs->Disable) ();
	kdEnabled = FALSE;
    }
}

Bool
KdEnableScreen (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);

    if (pScreenPriv->enabled)
	return TRUE;
    if(pScreenPriv->card->cfuncs->enable)
	if (!(*pScreenPriv->card->cfuncs->enable) (pScreen))
	    return FALSE;
    pScreenPriv->enabled = TRUE;
    pScreenPriv->dpmsState = KD_DPMS_NORMAL;
    pScreenPriv->card->selected = pScreenPriv->screen->mynum;
    KdOffscreenSwapIn (pScreen);    
    if (!pScreenPriv->screen->softCursor && pScreenPriv->card->cfuncs->enableCursor)
	(*pScreenPriv->card->cfuncs->enableCursor) (pScreen);
    if (!pScreenPriv->screen->dumb && pScreenPriv->card->cfuncs->enableAccel)
	(*pScreenPriv->card->cfuncs->enableAccel) (pScreen);
    KdEnableColormap (pScreen);
    KdSetRootClip (pScreen, TRUE);
    if (pScreenPriv->card->cfuncs->dpms)
	(*pScreenPriv->card->cfuncs->dpms) (pScreen, pScreenPriv->dpmsState);
    return TRUE;
}

void
KdResume (void)
{
    KdCardInfo	    *card;
    KdScreenInfo    *screen;

    if (kdEnabled)
    {
	KdDoSwitchCmd ("resume");
	for (card = kdCardInfo; card; card = card->next)
	{
	    if(card->cfuncs->preserve)
		(*card->cfuncs->preserve) (card);
	    for (screen = card->screenList; screen; screen = screen->next)
		if (screen->mynum == card->selected && screen->pScreen)
		    KdEnableScreen (screen->pScreen);
	}
	KdEnableInput ();
	KdReleaseAllKeys ();
    }
}

void
KdEnableScreens (void)
{
    if (!kdEnabled)
    {
	kdEnabled = TRUE;
        if (kdOsFuncs->Enable)
            (*kdOsFuncs->Enable) ();
    }
    KdResume ();
}

void
KdProcessSwitch (void)
{
    if (kdEnabled)
	KdDisableScreens ();
    else
	KdEnableScreens ();
}

void
AbortDDX(void)
{
    KdDisableScreens ();
    if (kdOsFuncs)
    {
	if (kdEnabled && kdOsFuncs->Disable)
	    (*kdOsFuncs->Disable) ();
        if (kdOsFuncs->Fini)
            (*kdOsFuncs->Fini) ();
	KdDoSwitchCmd ("stop");
    }

    if (kdCaughtSignal)
        abort();
}

void
ddxGiveUp ()
{
    AbortDDX ();
}

Bool	kdDumbDriver;
Bool	kdSoftCursor;

char *
KdParseFindNext (char *cur, char *delim, char *save, char *last)
{
    while (*cur && !strchr (delim, *cur))
    {
	*save++ = *cur++;
    }
    *save = 0;
    *last = *cur;
    if (*cur)
	cur++;
    return cur;
}

Rotation
KdAddRotation (Rotation a, Rotation b)
{
    Rotation	rotate = (a & RR_Rotate_All) * (b & RR_Rotate_All);
    Rotation	reflect = (a & RR_Reflect_All) ^ (b & RR_Reflect_All);

    if (rotate > RR_Rotate_270)
	rotate /= (RR_Rotate_270 * RR_Rotate_90);
    return reflect | rotate;
}

Rotation
KdSubRotation (Rotation a, Rotation b)
{
    Rotation	rotate = (a & RR_Rotate_All) * 16 / (b & RR_Rotate_All);
    Rotation	reflect = (a & RR_Reflect_All) ^ (b & RR_Reflect_All);

    if (rotate > RR_Rotate_270)
	rotate /= (RR_Rotate_270 * RR_Rotate_90);
    return reflect | rotate;
}

void
KdParseScreen (KdScreenInfo *screen,
	       char	    *arg)
{
    char    delim;
    char    save[1024];
    int	    fb;
    int	    i;
    int	    pixels, mm;
    
    screen->dumb = kdDumbDriver;
    screen->softCursor = kdSoftCursor;
    screen->origin = kdOrigin;
    screen->randr = RR_Rotate_0;
    screen->width = 0;
    screen->height = 0;
    screen->width_mm = 0;
    screen->height_mm = 0;
    screen->subpixel_order = kdSubpixelOrder;
    screen->rate = 0;
    for (fb = 0; fb < KD_MAX_FB; fb++)
	screen->fb[fb].depth = 0;
    if (!arg)
	return;
    if (strlen (arg) >= sizeof (save))
	return;
    
    for (i = 0; i < 2; i++)
    {
	arg = KdParseFindNext (arg, "x/@XY", save, &delim);
	if (!save[0])
	    return;
	
	pixels = atoi(save);
	mm = 0;
	
	if (delim == '/')
	{
	    arg = KdParseFindNext (arg, "x@XY", save, &delim);
	    if (!save[0])
		return;
	    mm = atoi(save);
	}
	
	if (i == 0)
	{
	    screen->width = pixels;
	    screen->width_mm = mm;
	}
	else
	{
	    screen->height = pixels;
	    screen->height_mm = mm;
	}
	if (delim != 'x' && delim != '@' && delim != 'X' && delim != 'Y')
	    return;
    }

    kdOrigin.x += screen->width;
    kdOrigin.y = 0;
    kdDumbDriver = FALSE;
    kdSoftCursor = FALSE;
    kdSubpixelOrder = SubPixelUnknown;

    if (delim == '@')
    {
	arg = KdParseFindNext (arg, "xXY", save, &delim);
	if (save[0])
	{
	    int	    rotate = atoi (save);
	    if (rotate < 45)
		screen->randr = RR_Rotate_0;
	    else if (rotate < 135)
		screen->randr = RR_Rotate_90;
	    else if (rotate < 225)
		screen->randr = RR_Rotate_180;
	    else if (rotate < 315)
		screen->randr = RR_Rotate_270;
	    else
		screen->randr = RR_Rotate_0;
	}
    }
    if (delim == 'X')
    {
	arg = KdParseFindNext (arg, "xY", save, &delim);
	screen->randr |= RR_Reflect_X;
    }

    if (delim == 'Y')
    {
	arg = KdParseFindNext (arg, "xY", save, &delim);
	screen->randr |= RR_Reflect_Y;
    }
    
    fb = 0;
    while (fb < KD_MAX_FB)
    {
	arg = KdParseFindNext (arg, "x/,", save, &delim);
	if (!save[0])
	    break;
	screen->fb[fb].depth = atoi(save);
	if (delim == '/')
	{
	    arg = KdParseFindNext (arg, "x,", save, &delim);
	    if (!save[0])
		break;
	    screen->fb[fb].bitsPerPixel = atoi (save);
	}
	else
	    screen->fb[fb].bitsPerPixel = 0;
	if (delim != ',')
	    break;
	fb++;
    }

    if (delim == 'x')
    {
	arg = KdParseFindNext (arg, "x", save, &delim);
	if (save[0])
	    screen->rate = atoi(save);
    }
}

/*
 * Mouse argument syntax:
 *
 *  device,protocol,options...
 *
 *  Options are any of:
 *	1-5	    n button mouse
 *	2button	    emulate middle button
 *	{NMO}	    Reorder buttons
 */

char *
KdSaveString (char *str)
{
    char    *n = (char *) xalloc (strlen (str) + 1);

    if (!n)
	return 0;
    strcpy (n, str);
    return n;
}

void
KdParseRgba (char *rgba)
{
    if (!strcmp (rgba, "rgb"))
	kdSubpixelOrder = SubPixelHorizontalRGB;
    else if (!strcmp (rgba, "bgr"))
	kdSubpixelOrder = SubPixelHorizontalBGR;
    else if (!strcmp (rgba, "vrgb"))
	kdSubpixelOrder = SubPixelVerticalRGB;
    else if (!strcmp (rgba, "vbgr"))
	kdSubpixelOrder = SubPixelVerticalBGR;
    else if (!strcmp (rgba, "none"))
	kdSubpixelOrder = SubPixelNone;
    else
	kdSubpixelOrder = SubPixelUnknown;
}

void
KdUseMsg (void)
{
    ErrorF("\nTinyX Device Dependent Usage:\n");
    ErrorF("-card pcmcia     Use PCMCIA card as additional screen\n");
    ErrorF("-screen WIDTH[/WIDTHMM]xHEIGHT[/HEIGHTMM][@ROTATION][X][Y][xDEPTH/BPP{,DEPTH/BPP}[xFREQ]]  Specify screen characteristics\n");
    ErrorF("-rgba rgb/bgr/vrgb/vbgr/none   Specify subpixel ordering for LCD panels\n");
    ErrorF("-mouse driver [,n,,options]    Specify the pointer driver and its options (n is the number of buttons)\n");
    ErrorF("-keybd driver [,,options]      Specify the keyboard driver and its options\n");
    ErrorF("-zaphod          Disable cursor screen switching\n");
    ErrorF("-2button         Emulate 3 button mouse\n");
    ErrorF("-3button         Disable 3 button mouse emulation\n");
    ErrorF("-rawcoord        Don't transform pointer coordinates on rotation\n");
    ErrorF("-dumb            Disable hardware acceleration\n");
    ErrorF("-softCursor      Force software cursor\n");
    ErrorF("-videoTest       Start the server, pause momentarily and exit\n");
    ErrorF("-origin X,Y      Locates the next screen in the the virtual screen (Xinerama)\n");
    ErrorF("-switchCmd       Command to execute on vt switch\n");
    ErrorF("-nozap           Don't terminate server on Ctrl+Alt+Backspace\n");
    ErrorF("vtxx             Use virtual terminal xx instead of the next available\n");
#ifdef PSEUDO8
    p8UseMsg ();
#endif
}

int
KdProcessArgument (int argc, char **argv, int i)
{
    KdCardInfo	    *card;
    KdScreenInfo    *screen;

    if (!strcmp (argv[i], "-card"))
    {
	if ((i+1) < argc)
	    InitCard (argv[i+1]);
	else
	    UseMsg ();
	return 2;
    }
    if (!strcmp (argv[i], "-screen"))
    {
	if ((i+1) < argc)
	{
	    card = KdCardInfoLast ();
	    if (!card)
	    {
		InitCard (0);
		card = KdCardInfoLast ();
	    }
	    if (card) {
		screen = KdScreenInfoAdd (card);
		KdParseScreen (screen, argv[i+1]);
	    } else
		ErrorF("No matching card found!\n");
	}
	else
	    UseMsg ();
	return 2;
    }
    if (!strcmp (argv[i], "-zaphod"))
    {
	kdDisableZaphod = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-nozap"))
    {
	kdDontZap = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-3button"))
    {
	kdEmulateMiddleButton = FALSE;
	return 1;
    }
    if (!strcmp (argv[i], "-2button"))
    {
	kdEmulateMiddleButton = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-rawcoord"))
    {
	kdRawPointerCoordinates = 1;
	return 1;
    }
    if (!strcmp (argv[i], "-dumb"))
    {
	kdDumbDriver = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-softCursor"))
    {
	kdSoftCursor = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-videoTest"))
    {
	kdVideoTest = TRUE;
	return 1;
    }
    if (!strcmp (argv[i], "-origin"))
    {
	if ((i+1) < argc)
	{
	    char    *x = argv[i+1];
	    char    *y = strchr (x, ',');
	    if (x)
		kdOrigin.x = atoi (x);
	    else
		kdOrigin.x = 0;
	    if (y)
		kdOrigin.y = atoi(y+1);
	    else
		kdOrigin.y = 0;
	}
	else
	    UseMsg ();
	return 2;
    }
    if (!strcmp (argv[i], "-rgba"))
    {
	if ((i+1) < argc)
	    KdParseRgba (argv[i+1]);
	else
	    UseMsg ();
	return 2;
    }
    if (!strcmp (argv[i], "-switchCmd"))
    {
	if ((i+1) < argc)
	    kdSwitchCmd = argv[i+1];
	else
	    UseMsg ();
	return 2;
    }
    if (!strncmp (argv[i], "vt", 2) &&
	sscanf (argv[i], "vt%2d", &kdVirtualTerminal) == 1)
    {
	return 1;
    }
    if (!strcmp (argv[i], "-mouse") ||
        !strcmp (argv[i], "-pointer")) {
        if (i + 1 >= argc)
            UseMsg();
        KdAddConfigPointer(argv[i + 1]);
	kdHasPointer = TRUE;
        return 2;
    }
    if (!strcmp (argv[i], "-keybd")) {
        if (i + 1 >= argc)
            UseMsg();
        KdAddConfigKeyboard(argv[i + 1]);
	kdHasKbd = TRUE;
        return 2;
    }

#ifdef PSEUDO8
    return p8ProcessArgument (argc, argv, i);
#else
    return 0;
#endif
}

/*
 * These are getting tossed in here until I can think of where
 * they really belong
 */

void
KdOsInit (KdOsFuncs *pOsFuncs)
{
    kdOsFuncs = pOsFuncs;
    if (pOsFuncs)
    {
	if (serverGeneration == 1) 
	{
	    KdDoSwitchCmd ("start");
            if (pOsFuncs->Init)
                (*pOsFuncs->Init) ();
	}
    }
}

Bool
KdAllocatePrivates (ScreenPtr pScreen)
{
    KdPrivScreenPtr	pScreenPriv;
    
    if (kdGeneration != serverGeneration)
	kdGeneration = serverGeneration;

    pScreenPriv = (KdPrivScreenPtr) xalloc(sizeof (*pScreenPriv));
    if (!pScreenPriv)
	return FALSE;
    memset (pScreenPriv, '\0', sizeof (KdPrivScreenRec));
    KdSetScreenPriv (pScreen, pScreenPriv);
    return TRUE;
}

Bool
KdCreateScreenResources (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdCardInfo	    *card = pScreenPriv->card;
    Bool ret;

    pScreen->CreateScreenResources = pScreenPriv->CreateScreenResources;
    if(pScreen->CreateScreenResources)
	ret = (*pScreen->CreateScreenResources) (pScreen);
    else
	ret= -1;
    pScreenPriv->CreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = KdCreateScreenResources;
    if (ret && card->cfuncs->createRes)
	ret = (*card->cfuncs->createRes) (pScreen);
    return ret;
}

Bool
KdCloseScreen (int index, ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    KdCardInfo	    *card = pScreenPriv->card;
    Bool	    ret;
    
    pScreenPriv->closed = TRUE;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    if(pScreen->CloseScreen)
        ret = (*pScreen->CloseScreen) (index, pScreen);
    else
	ret = TRUE;
    
    if (screen->off_screen_base < screen->memory_size)
	KdOffscreenFini (pScreen);
    
    if (pScreenPriv->dpmsState != KD_DPMS_NORMAL)
	(*card->cfuncs->dpms) (pScreen, KD_DPMS_NORMAL);
    
    if (screen->mynum == card->selected)
	KdDisableScreen (pScreen);
    
    /*
     * Restore video hardware when last screen is closed
     */
    if (screen == card->screenList)
    {
	if (kdEnabled && card->cfuncs->restore)
	    (*card->cfuncs->restore) (card);
    }
	
    if (!pScreenPriv->screen->dumb && card->cfuncs->finiAccel)
	(*card->cfuncs->finiAccel) (pScreen);

    if (!pScreenPriv->screen->softCursor && card->cfuncs->finiCursor)
	(*card->cfuncs->finiCursor) (pScreen);

    if(card->cfuncs->scrfini)
        (*card->cfuncs->scrfini) (screen);

    /*
     * Clean up card when last screen is closed, DIX closes them in
     * reverse order, thus we check for when the first in the list is closed
     */
    if (screen == card->screenList)
    {
	if(card->cfuncs->cardfini)
	    (*card->cfuncs->cardfini) (card);
	/*
	 * Clean up OS when last card is closed
	 */
	if (card == kdCardInfo)
	{
	    if (kdEnabled)
	    {
		kdEnabled = FALSE;
		if(kdOsFuncs->Disable)
		    (*kdOsFuncs->Disable) ();
	    }
	}
    }
    
    pScreenPriv->screen->pScreen = 0;
    
    xfree ((pointer) pScreenPriv);
    return ret;
}

Bool
KdSaveScreen (ScreenPtr pScreen, int on)
{
    KdScreenPriv(pScreen);
    int	    dpmsState;
    
    if (!pScreenPriv->card->cfuncs->dpms)
	return FALSE;
    
    dpmsState = pScreenPriv->dpmsState;
    switch (on) {
    case SCREEN_SAVER_OFF:
	dpmsState = KD_DPMS_NORMAL;
	break;
    case SCREEN_SAVER_ON:
	if (dpmsState == KD_DPMS_NORMAL)
	    dpmsState = KD_DPMS_NORMAL+1;
	break;
    case SCREEN_SAVER_CYCLE:
	if (dpmsState < KD_DPMS_MAX)
	    dpmsState++;
	break;
    case SCREEN_SAVER_FORCER:
	break;
    }
    if (dpmsState != pScreenPriv->dpmsState)
    {
	if (pScreenPriv->enabled)
	    (*pScreenPriv->card->cfuncs->dpms) (pScreen, dpmsState);
	pScreenPriv->dpmsState = dpmsState;
    }
    return TRUE;
}

static Bool
KdCreateWindow (WindowPtr pWin)
{
#ifndef PHOENIX
    if (!pWin->parent)
    {
	KdScreenPriv(pWin->drawable.pScreen);

	if (!pScreenPriv->enabled)
	{
	    REGION_EMPTY (pWin->drawable.pScreen, &pWin->borderClip);
	    REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
	}
    }
#endif
    return fbCreateWindow (pWin);
}

void
KdSetSubpixelOrder (ScreenPtr pScreen, Rotation randr)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    int			subpixel_order = screen->subpixel_order;
    Rotation		subpixel_dir;
    int			i;
    
    static struct {
	int	    subpixel_order;
	Rotation    direction;
    } orders[] = {
	{ SubPixelHorizontalRGB, 	RR_Rotate_0 },
	{ SubPixelHorizontalBGR,	RR_Rotate_180 },
	{ SubPixelVerticalRGB,		RR_Rotate_270 },
	{ SubPixelVerticalBGR,		RR_Rotate_90 },
    };

    static struct {
	int	bit;
	int	normal; 
	int	reflect;
    } reflects[] = {
	{ RR_Reflect_X, SubPixelHorizontalRGB,	SubPixelHorizontalBGR },
	{ RR_Reflect_X, SubPixelHorizontalBGR,	SubPixelHorizontalRGB },
	{ RR_Reflect_Y, SubPixelVerticalRGB,	SubPixelVerticalBGR },
	{ RR_Reflect_Y, SubPixelVerticalRGB,	SubPixelVerticalRGB },
    };
    
    /* map subpixel to direction */
    for (i = 0; i < 4; i++)
	if (orders[i].subpixel_order == subpixel_order)
	    break;
    if (i < 4)
    {
	subpixel_dir = KdAddRotation (randr & RR_Rotate_All, orders[i].direction);
	
	/* map back to subpixel order */
	for (i = 0; i < 4; i++)
	    if (orders[i].direction & subpixel_dir)
	    {
		subpixel_order = orders[i].subpixel_order;
		break;
	    }
	/* reflect */
	for (i = 0; i < 4; i++)
	    if ((randr & reflects[i].bit) &&
		reflects[i].normal == subpixel_order)
	    {
		subpixel_order = reflects[i].reflect;
		break;
	    }
    }
    PictureSetSubpixelOrder (pScreen, subpixel_order);
}

/* Pass through AddScreen, which doesn't take any closure */
static KdScreenInfo *kdCurrentScreen;

Bool
KdScreenInit(int index, ScreenPtr pScreen, int argc, char **argv)
{
    KdScreenInfo	*screen = kdCurrentScreen;
    KdCardInfo		*card = screen->card;
    KdPrivScreenPtr	pScreenPriv;
    int			fb;
    /*
     * note that screen->fb is set up for the nominal orientation
     * of the screen; that means if randr is rotated, the values
     * there should reflect a rotated frame buffer (or shadow).
     */
    Bool		rotated = (screen->randr & (RR_Rotate_90|RR_Rotate_270)) != 0;
    int			width, height, *width_mmp, *height_mmp;

    KdAllocatePrivates (pScreen);

    pScreenPriv = KdGetScreenPriv(pScreen);
    
    if (!rotated)
    {
	width = screen->width;
	height = screen->height;
	width_mmp = &screen->width_mm;
	height_mmp = &screen->height_mm;
    }
    else
    {
	width = screen->height;
	height = screen->width;
	width_mmp = &screen->height_mm;
	height_mmp = &screen->width_mm;
    }
    screen->pScreen = pScreen;
    pScreenPriv->screen = screen;
    pScreenPriv->card = card;
    for (fb = 0; fb < KD_MAX_FB && screen->fb[fb].depth; fb++)
	pScreenPriv->bytesPerPixel[fb] = screen->fb[fb].bitsPerPixel >> 3;
    pScreenPriv->dpmsState = KD_DPMS_NORMAL;
#ifdef PANORAMIX
    dixScreenOrigins[pScreen->myNum] = screen->origin;
#endif

    if (!monitorResolution)
	monitorResolution = 75;
    /*
     * This is done in this order so that backing store wraps
     * our GC functions; fbFinishScreenInit initializes MI
     * backing store
     */
    if (!fbSetupScreen (pScreen, 
			screen->fb[0].frameBuffer, 
			width, height, 
			monitorResolution, monitorResolution, 
			screen->fb[0].pixelStride,
			screen->fb[0].bitsPerPixel))
    {
	return FALSE;
    }

    /*
     * Set colormap functions
     */
    pScreen->InstallColormap	= KdInstallColormap;
    pScreen->UninstallColormap	= KdUninstallColormap;
    pScreen->ListInstalledColormaps = KdListInstalledColormaps;
    pScreen->StoreColors	= KdStoreColors;
     
    pScreen->SaveScreen		= KdSaveScreen;
    pScreen->CreateWindow	= KdCreateWindow;

#if KD_MAX_FB > 1
    if (screen->fb[1].depth)
    {
	if (!fbOverlayFinishScreenInit (pScreen, 
					screen->fb[0].frameBuffer, 
					screen->fb[1].frameBuffer, 
					width, height, 
					monitorResolution, monitorResolution,
					screen->fb[0].pixelStride,
					screen->fb[1].pixelStride,
					screen->fb[0].bitsPerPixel,
					screen->fb[1].bitsPerPixel,
					screen->fb[0].depth,
					screen->fb[1].depth))
	{
	    return FALSE;
	}
    }
    else
#endif
    {
	if (!fbFinishScreenInit (pScreen, 
				 screen->fb[0].frameBuffer, 
				 width, height,
				 monitorResolution, monitorResolution,
				 screen->fb[0].pixelStride,
				 screen->fb[0].bitsPerPixel))
	{
	    return FALSE;
	}
    }
    
    /*
     * Fix screen sizes; for some reason mi takes dpi instead of mm.
     * Rounding errors are annoying
     */
    if (*width_mmp)
	pScreen->mmWidth = *width_mmp;
    else
	*width_mmp = pScreen->mmWidth;
    if (*height_mmp)
	pScreen->mmHeight = *height_mmp;
    else
	*height_mmp = pScreen->mmHeight;
    
    /*
     * Plug in our own block/wakeup handlers.
     * miScreenInit installs NoopDDA in both places
     */
    pScreen->BlockHandler	= KdBlockHandler;
    pScreen->WakeupHandler	= KdWakeupHandler;
    
#ifdef RENDER
    if (!fbPictureInit (pScreen, 0, 0))
	return FALSE;
#endif
    if (card->cfuncs->initScreen)
	if (!(*card->cfuncs->initScreen) (pScreen))
	    return FALSE;
	    
    if (!screen->dumb && card->cfuncs->initAccel)
	if (!(*card->cfuncs->initAccel) (pScreen))
	    screen->dumb = TRUE;

    if (screen->off_screen_base < screen->memory_size)
	KdOffscreenInit (pScreen);
    
#ifdef PSEUDO8
    (void) p8Init (pScreen, PSEUDO8_USE_DEFAULT);
#endif
    
    if (card->cfuncs->finishInitScreen)
	if (!(*card->cfuncs->finishInitScreen) (pScreen))
	    return FALSE;
	    
#if 0
    fbInitValidateTree (pScreen);
#endif
    
#if 0
    pScreen->backingStoreSupport = Always;
    miInitializeBackingStore (pScreen);
#endif


    /* 
     * Wrap CloseScreen, the order now is:
     *	KdCloseScreen
     *	miBSCloseScreen
     *	fbCloseScreen
     */
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = KdCloseScreen;

    pScreenPriv->CreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = KdCreateScreenResources;
    
    if (screen->softCursor ||
	!card->cfuncs->initCursor || 
	!(*card->cfuncs->initCursor) (pScreen))
    {
	/* Use MI for cursor display and event queueing. */
	screen->softCursor = TRUE;
	miDCInitialize(pScreen, &kdPointerScreenFuncs);
    }

    
    if (!fbCreateDefColormap (pScreen))
    {
	return FALSE;
    }

    KdSetSubpixelOrder (pScreen, screen->randr);

    /*
     * Enable the hardware
     */
    if (!kdEnabled)
    {
	kdEnabled = TRUE;
	if(kdOsFuncs->Enable)
	    (*kdOsFuncs->Enable) ();
    }
    
    if (screen->mynum == card->selected)
    {
	if(card->cfuncs->preserve)
	    (*card->cfuncs->preserve) (card);
	if(card->cfuncs->enable)
	    if (!(*card->cfuncs->enable) (pScreen))
		return FALSE;
	pScreenPriv->enabled = TRUE;
	if (!screen->softCursor && card->cfuncs->enableCursor)
	    (*card->cfuncs->enableCursor) (pScreen);
	KdEnableColormap (pScreen);
	if (!screen->dumb && card->cfuncs->enableAccel)
	    (*card->cfuncs->enableAccel) (pScreen);
    }
    
    return TRUE;
}

void
KdInitScreen (ScreenInfo    *pScreenInfo,
	      KdScreenInfo  *screen,
	      int	    argc,
	      char	    **argv)
{
    KdCardInfo	*card = screen->card;
    
    (*card->cfuncs->scrinit) (screen);
    
    if (!card->cfuncs->initAccel)
	screen->dumb = TRUE;
    if (!card->cfuncs->initCursor)
	screen->softCursor = TRUE;
}

static Bool
KdSetPixmapFormats (ScreenInfo	*pScreenInfo)
{
    CARD8	    depthToBpp[33];	/* depth -> bpp map */
    KdCardInfo	    *card;
    KdScreenInfo    *screen;
    int		    i;
    int		    bpp;
    int		    fb;
    PixmapFormatRec *format;

    for (i = 1; i <= 32; i++)
	depthToBpp[i] = 0;

    /*
     * Generate mappings between bitsPerPixel and depth,
     * also ensure that all screens comply with protocol
     * restrictions on equivalent formats for the same
     * depth on different screens
     */
    for (card = kdCardInfo; card; card = card->next)
    {
	for (screen = card->screenList; screen; screen = screen->next)
	{
	    for (fb = 0; fb < KD_MAX_FB && screen->fb[fb].depth; fb++)
	    {
		bpp = screen->fb[fb].bitsPerPixel;
		if (bpp == 24)
		    bpp = 32;
		if (!depthToBpp[screen->fb[fb].depth])
		    depthToBpp[screen->fb[fb].depth] = bpp;
		else if (depthToBpp[screen->fb[fb].depth] != bpp) 
		    return FALSE;
	    }
	}
    }
    
    /*
     * Fill in additional formats
     */
    for (i = 0; i < NUM_KD_DEPTHS; i++)
	if (!depthToBpp[kdDepths[i].depth])
	    depthToBpp[kdDepths[i].depth] = kdDepths[i].bpp;
	
    pScreenInfo->imageByteOrder     = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad  = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder     = BITMAP_BIT_ORDER;
    
    pScreenInfo->numPixmapFormats = 0;
    
    for (i = 1; i <= 32; i++)
    {
	if (depthToBpp[i])
	{
	    format = &pScreenInfo->formats[pScreenInfo->numPixmapFormats++];
	    format->depth = i;
	    format->bitsPerPixel = depthToBpp[i];
	    format->scanlinePad = BITMAP_SCANLINE_PAD;
	}
    }
    
    return TRUE;
}

static void
KdAddScreen (ScreenInfo	    *pScreenInfo,
	     KdScreenInfo   *screen,
	     int	    argc,
	     char	    **argv)
{
    int	    i;
    /*
     * Fill in fb visual type masks for this screen
     */
    for (i = 0; i < pScreenInfo->numPixmapFormats; i++)
    {
	unsigned long	visuals;
	Pixel		rm, gm, bm;
	int		fb;
	
	visuals = 0;
	rm = gm = bm = 0;
	for (fb = 0; fb < KD_MAX_FB && screen->fb[fb].depth; fb++)
	{
	    if (pScreenInfo->formats[i].depth == screen->fb[fb].depth)
	    {
		visuals = screen->fb[fb].visuals;
		rm = screen->fb[fb].redMask;
		gm = screen->fb[fb].greenMask;
		bm = screen->fb[fb].blueMask;
		break;
	    }
	}
	fbSetVisualTypesAndMasks (pScreenInfo->formats[i].depth,
				  visuals,
				  8,
				  rm, gm, bm);
    }

    kdCurrentScreen = screen;
    
    AddScreen (KdScreenInit, argc, argv);
}

#if 0 /* This function is not used currently */

int
KdDepthToFb (ScreenPtr	pScreen, int depth)
{
    KdScreenPriv(pScreen);
    int	    fb;

    for (fb = 0; fb <= KD_MAX_FB && pScreenPriv->screen->fb[fb].frameBuffer; fb++)
	if (pScreenPriv->screen->fb[fb].depth == depth)
	    return fb;
}

#endif

#ifdef HAVE_BACKTRACE
/* shamelessly ripped from xf86Events.c */
void
KdBacktrace (int signum)
{
    void *array[32]; /* more than 32 and you have bigger problems */
    size_t size, i;
    char **strings;

    signal(signum, SIG_IGN);

    size = backtrace (array, 32);
    fprintf (stderr, "\nBacktrace (%d deep):\n", size);
    strings = backtrace_symbols (array, size);
    for (i = 0; i < size; i++)
        fprintf (stderr, "%d: %s\n", i, strings[i]);
    free (strings);
    
    kdCaughtSignal = TRUE;    
    if (signum == SIGSEGV)
        FatalError("Segmentation fault caught\n");
    else if (signum > 0)
        FatalError("Signal %d caught\n", signum);
}
#else
void
KdBacktrace (int signum)
{
    kdCaughtSignal = TRUE;
    FatalError("Segmentation fault caught\n");
}
#endif

void
KdInitOutput (ScreenInfo    *pScreenInfo,
	      int	    argc,
	      char	    **argv)
{
    KdCardInfo	    *card;
    KdScreenInfo    *screen;

    if (!kdCardInfo)
    {
	InitCard (0);
	if (!(card = KdCardInfoLast ()))
	    FatalError("No matching cards found!\n");
	screen = KdScreenInfoAdd (card);
	KdParseScreen (screen, 0);
    }
    /*
     * Initialize all of the screens for all of the cards
     */
    for (card = kdCardInfo; card; card = card->next)
    {
	int ret=1;
	if(card->cfuncs->cardinit)
		ret=(*card->cfuncs->cardinit) (card);
	if (ret)
	{
	    for (screen = card->screenList; screen; screen = screen->next)
		KdInitScreen (pScreenInfo, screen, argc, argv);
	}
    }
    
    /*
     * Merge the various pixmap formats together, this can fail
     * when two screens share depth but not bitsPerPixel
     */
    if (!KdSetPixmapFormats (pScreenInfo))
	return;
    
    /*
     * Add all of the screens
     */
    for (card = kdCardInfo; card; card = card->next)
	for (screen = card->screenList; screen; screen = screen->next)
	    KdAddScreen (pScreenInfo, screen, argc, argv);

    signal(SIGSEGV, KdBacktrace);
}

#ifdef DPMSExtension
int
DPMSSet(ClientPtr client, int level)
{
}

int
DPMSGet (int *level)
{
    return -1;
}

Bool
DPMSSupported (void)
{
    return FALSE;
}
#endif

void ddxInitGlobals(void) { /* THANK YOU XPRINT */ }

