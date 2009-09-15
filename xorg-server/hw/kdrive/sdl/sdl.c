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


