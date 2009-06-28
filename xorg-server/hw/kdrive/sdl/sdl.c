/*
 * Copyright © 2004 PillowElephantBadgerBankPond 
 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of PillowElephantBadgerBankPond not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  PillowElephantBadgerBankPond makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * PillowElephantBadgerBankPond DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL PillowElephantBadgerBankPond BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * It's really not my fault - see it was the elephants!!
 * 	- jaymz
 *
 */
#ifdef HAVE_CONFIG_H
#include "kdrive-config.h"
#endif
#include "kdrive.h"
#include <SDL/SDL.h>
#include <X11/keysym.h>

static void xsdlFini(void);
static Bool sdlScreenInit(KdScreenInfo *screen);
static Bool sdlFinishInitScreen(ScreenPtr pScreen);
static Bool sdlCreateRes(ScreenPtr pScreen);

static void sdlKeyboardFini(KdKeyboardInfo *ki);
static Bool sdlKeyboardInit(KdKeyboardInfo *ki);

static Bool sdlMouseInit(KdPointerInfo *pi);
static void sdlMouseFini(KdPointerInfo *pi);

void *sdlShadowWindow (ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode, CARD32 *size, void *closure);
void sdlShadowUpdate (ScreenPtr pScreen, shadowBufPtr pBuf);

void sdlTimer(void);

KdKeyboardInfo *sdlKeyboard = NULL;
KdPointerInfo *sdlPointer = NULL;

KeySym sdlKeymap[]={
	0, 			/* 8 */
	0, 
	XK_Escape, NoSymbol, 	/* escape */
	XK_1, XK_exclam,
	XK_2, XK_at,
	XK_3, XK_numbersign,
	XK_4, XK_dollar,
	XK_5, XK_percent,
	XK_6, XK_asciicircum,
	XK_7, XK_ampersand,
	XK_8, XK_asterisk,
	XK_9, XK_parenleft,
	XK_0, XK_parenright, 
	XK_minus, XK_underscore,
	XK_equal, XK_plus, 
	XK_BackSpace, NoSymbol,		/* backspace */
	XK_Tab, NoSymbol, 
	XK_q, XK_Q, 
	XK_w, XK_W, 
	XK_e, XK_E, 
	XK_r, XK_R, 
	XK_t, XK_T, 
	XK_y, XK_Y, 
	XK_u, XK_U, 
	XK_i, XK_I, 
	XK_o, XK_O, 
	XK_p, XK_P, 
	XK_bracketleft, XK_braceleft, 		/* [, { */
	XK_bracketright, XK_braceright,		/* ]. } */ 
	XK_Return, NoSymbol,
	XK_Control_L, NoSymbol, 
	XK_a, XK_A,
	XK_s, XK_S,
	XK_d, XK_D,
	XK_f, XK_F,
	XK_g, XK_G,
	XK_h, XK_H,
	XK_j, XK_J,
	XK_k, XK_K,
	XK_l, XK_L,
	XK_semicolon, XK_colon,
	XK_apostrophe, XK_quotedbl,
	XK_grave, XK_asciitilde,
	XK_Shift_L, NoSymbol,
	XK_backslash, XK_bar, 
	XK_z, XK_z, 
	XK_x, XK_X, 
	XK_c, XK_C, 
	XK_v, XK_V, 
	XK_b, XK_B, 
	XK_n, XK_N, 
	XK_m, XK_M, 
	XK_comma, XK_less,
	XK_period, XK_greater, 
	XK_slash, XK_question, 
	XK_Shift_R, NoSymbol, 
	XK_KP_Multiply, NoSymbol,	
	XK_Meta_L, XK_Alt_L,
	XK_space, NoSymbol, 
	XK_Caps_Lock, NoSymbol, 
	XK_F1, NoSymbol,
	XK_F2, NoSymbol,
	XK_F3, NoSymbol,
	XK_F4, NoSymbol,
	XK_F5, NoSymbol,
	XK_F6, NoSymbol,
	XK_F7, NoSymbol,
	XK_F8, NoSymbol,
	XK_F9, NoSymbol,
	XK_F10, NoSymbol,
	XK_Num_Lock, NoSymbol,
	XK_Scroll_Lock, NoSymbol,
	XK_KP_Home, XK_KP_7, 
	XK_KP_Up, XK_KP_8, 
	XK_KP_Page_Up, XK_KP_9, 
	XK_KP_Subtract, NoSymbol, 
	XK_KP_Left, XK_KP_4,
	XK_KP_5, NoSymbol,
	XK_KP_Right, XK_KP_6,
	XK_KP_Add, NoSymbol,
	XK_KP_End, XK_KP_1,
	XK_KP_Down, XK_KP_2,
	XK_KP_Page_Down, XK_KP_3, 
	XK_KP_Insert, XK_KP_0, 
	XK_KP_Delete, XK_KP_Decimal, 
	NoSymbol, NoSymbol, 		/* 92 */
	NoSymbol, NoSymbol, 		/* 93 */
	NoSymbol, NoSymbol, 		/* 94 */
	XK_F11, NoSymbol, 		/* 95 */
	XK_F12, NoSymbol, 		/* 96 */
	XK_Home, NoSymbol, 		/* 97 */
	XK_Up, NoSymbol, 		/* 98 */
	XK_Page_Up, NoSymbol, 		/* 99 */
	XK_Left, NoSymbol, 		/* 100 */
	NoSymbol, NoSymbol, 		/* 101 */
	XK_Right, NoSymbol, 		/* 102 */
	NoSymbol, NoSymbol, 		/* 103 */
	XK_Down, NoSymbol, 		/* 104 */
	XK_Page_Down, NoSymbol, 		/* 105 */
	XK_Insert, NoSymbol, 		/* 106 */
	NoSymbol, NoSymbol, 		/* 107 */
	NoSymbol, NoSymbol, 		/* 108 */
	XK_Meta_R, XK_Alt_R, 		/* 109 */
	XK_Pause, XK_Break, 		/* 110 */
	XK_Sys_Req, XK_Print,		/* 111 */
	NoSymbol, NoSymbol,		/* 112 */
	XK_Control_R, NoSymbol,		/* 113 */
	NoSymbol, NoSymbol,		/* 114 */
	XK_Super_L, NoSymbol,		/* 115 */
	XK_Super_R, NoSymbol,		/* 116 */
	XK_Menu, NoSymbol,		/* 117 */
	NoSymbol, NoSymbol		/* 118 */
};

KdKeyboardDriver sdlKeyboardDriver = {
    .name = "keyboard",
    .Init = sdlKeyboardInit,
    .Fini = sdlKeyboardFini,
};

KdPointerDriver sdlMouseDriver = {
    .name = "mouse",
    .Init = sdlMouseInit,
    .Fini = sdlMouseFini,
};


KdCardFuncs sdlFuncs = {
    .scrinit = sdlScreenInit,	/* scrinit */
    .finishInitScreen = sdlFinishInitScreen, /* finishInitScreen */
    .createRes = sdlCreateRes,	/* createRes */
};

int mouseState=0;

struct SdlDriver
{
	SDL_Surface *screen;
};



static Bool sdlScreenInit(KdScreenInfo *screen)
{
	struct SdlDriver *sdlDriver=calloc(1, sizeof(struct SdlDriver));
#ifdef DEBUG
	printf("sdlScreenInit()\n");
#endif
	if (!screen->width || !screen->height)
	{
		screen->width = 640;
		screen->height = 480;
	}
	if (!screen->fb[0].depth)
		screen->fb[0].depth = 4;
#ifdef DEBUG
	printf("Attempting for %dx%d/%dbpp mode\n", screen->width, screen->height, screen->fb[0].depth);
#endif
	sdlDriver->screen=SDL_SetVideoMode(screen->width, screen->height, screen->fb[0].depth, 0);
	if(sdlDriver->screen==NULL)
		return FALSE;
#ifdef DEBUG
	printf("Set %dx%d/%dbpp mode\n", sdlDriver->screen->w, sdlDriver->screen->h, sdlDriver->screen->format->BitsPerPixel);
#endif
	screen->width=sdlDriver->screen->w;
	screen->height=sdlDriver->screen->h;
	screen->fb[0].depth=sdlDriver->screen->format->BitsPerPixel;
	screen->fb[0].visuals=(1<<TrueColor);
	screen->fb[0].redMask=sdlDriver->screen->format->Rmask;
	screen->fb[0].greenMask=sdlDriver->screen->format->Gmask;
	screen->fb[0].blueMask=sdlDriver->screen->format->Bmask;
	screen->fb[0].bitsPerPixel=sdlDriver->screen->format->BitsPerPixel;
	screen->rate=60;
	screen->memory_base=(CARD8 *)sdlDriver->screen->pixels;
	screen->memory_size=0;
	screen->off_screen_base=0;
	screen->driver=sdlDriver;
	screen->fb[0].byteStride=(sdlDriver->screen->w*sdlDriver->screen->format->BitsPerPixel)/8;
	screen->fb[0].pixelStride=sdlDriver->screen->w;
	screen->fb[0].frameBuffer=(CARD8 *)sdlDriver->screen->pixels;
	SDL_WM_SetCaption("Freedesktop.org X server (SDL)", NULL);
	return TRUE;
}

void sdlShadowUpdate (ScreenPtr pScreen, shadowBufPtr pBuf)
{
	KdScreenPriv(pScreen);
	KdScreenInfo *screen = pScreenPriv->screen;
	struct SdlDriver *sdlDriver=screen->driver;
#ifdef DEBUG
	printf("Shadow update()\n");
#endif
	if(SDL_MUSTLOCK(sdlDriver->screen))
	{
		if(SDL_LockSurface(sdlDriver->screen)<0)
		{
#ifdef DEBUG
			printf("Couldn't lock SDL surface - d'oh!\n");
#endif
			return;
		}
	}
	
	if(SDL_MUSTLOCK(sdlDriver->screen))
		SDL_UnlockSurface(sdlDriver->screen);
	SDL_UpdateRect(sdlDriver->screen, 0, 0, sdlDriver->screen->w, sdlDriver->screen->h);
}


void *sdlShadowWindow (ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode, CARD32 *size, void *closure)
{
	KdScreenPriv(pScreen);
	KdScreenInfo *screen = pScreenPriv->screen;
	struct SdlDriver *sdlDriver=screen->driver;
	*size=(sdlDriver->screen->w*sdlDriver->screen->format->BitsPerPixel)/8;
#ifdef DEBUG
	printf("Shadow window()\n");
#endif
	return (void *)((CARD8 *)sdlDriver->screen->pixels + row * (*size) + offset);
}


static Bool sdlCreateRes(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	KdScreenInfo *screen = pScreenPriv->screen;
	KdShadowFbAlloc(screen, 0, FALSE);
	KdShadowSet(pScreen, RR_Rotate_0, sdlShadowUpdate, sdlShadowWindow);
	return TRUE;
}

static Bool sdlFinishInitScreen(ScreenPtr pScreen)
{
	if (!shadowSetup (pScreen))
		return FALSE;
		
/*
#ifdef RANDR
	if (!sdlRandRInit (pScreen))
		return FALSE;
#endif
*/
	return TRUE;
}

static void sdlKeyboardFini(KdKeyboardInfo *ki)
{
        sdlKeyboard = NULL;
}

static Bool sdlKeyboardInit(KdKeyboardInfo *ki)
{
        ki->minScanCode = 8;
        ki->maxScanCode = 255;
        ki->keySyms.minKeyCode = 8;
        ki->keySyms.maxKeyCode = 255;
        ki->keySyms.mapWidth = 2;
        memcpy(ki->keySyms.map, sdlKeymap, sizeof(sdlKeymap));

	sdlKeyboard = ki;

        return TRUE;
}

static Bool sdlMouseInit (KdPointerInfo *pi)
{
        sdlPointer = pi;
	return TRUE;
}

static void sdlMouseFini(KdPointerInfo *pi)
{
        sdlPointer = NULL;
}


void InitCard(char *name)
{
	KdCardAttr attr;
        KdCardInfoAdd (&sdlFuncs, &attr, 0);
#ifdef DEBUG
	printf("InitCard: %s\n", name);
#endif
}

void InitOutput(ScreenInfo *pScreenInfo, int argc, char **argv)
{
	KdInitOutput(pScreenInfo, argc, argv);
#ifdef DEBUG
	printf("InitOutput()\n");
#endif
}

void InitInput(int argc, char **argv)
{
        KdPointerInfo *pi;
        KdKeyboardInfo *ki;

        KdAddKeyboardDriver(&sdlKeyboardDriver);
        KdAddPointerDriver(&sdlMouseDriver);
        
        ki = KdParseKeyboard("keyboard");
        KdAddKeyboard(ki);
        pi = KdParsePointer("mouse");
        KdAddPointer(pi);

        KdInitInput();
}

void ddxUseMsg(void)
{
	KdUseMsg();
}

int ddxProcessArgument(int argc, char **argv, int i)
{
	return KdProcessArgument(argc, argv, i);
}

void sdlTimer(void)
{
	static int buttonState=0;
	SDL_Event event;
	SDL_ShowCursor(FALSE);
	/* get the mouse state */
	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
			case SDL_MOUSEMOTION:
				KdEnqueuePointerEvent(sdlPointer, mouseState, event.motion.x, event.motion.y, 0);
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch(event.button.button)
				{
					case 1:
						buttonState=KD_BUTTON_1;
						break;
					case 2:
						buttonState=KD_BUTTON_2;
						break;
					case 3:
						buttonState=KD_BUTTON_3;
						break;
				}
				mouseState|=buttonState;
				KdEnqueuePointerEvent(sdlPointer, mouseState|KD_MOUSE_DELTA, 0, 0, 0);
				break;
			case SDL_MOUSEBUTTONUP:
				switch(event.button.button)
				{
					case 1:
						buttonState=KD_BUTTON_1;
						break;
					case 2:
						buttonState=KD_BUTTON_2;
						break;
					case 3:
						buttonState=KD_BUTTON_3;
						break;
				}
				mouseState &= ~buttonState;
				KdEnqueuePointerEvent(sdlPointer, mouseState|KD_MOUSE_DELTA, 0, 0, 0);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
#ifdef DEBUG
				printf("Keycode: %d\n", event.key.keysym.scancode);
#endif
			        KdEnqueueKeyboardEvent (sdlKeyboard, event.key.keysym.scancode, event.type==SDL_KEYUP);
				break;

			case SDL_QUIT:
				/* this should never happen */
				SDL_Quit();
		}
	}
}

static int xsdlInit(void)
{
#ifdef DEBUG
	printf("Calling SDL_Init()\n");
#endif
	return SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
}


static void xsdlFini(void)
{
	SDL_Quit();
}

KdOsFuncs sdlOsFuncs={
	.Init = xsdlInit,
	.Fini = xsdlFini,
	.pollEvents = sdlTimer,
};

void OsVendorInit (void)
{
    KdOsInit (&sdlOsFuncs);
}


