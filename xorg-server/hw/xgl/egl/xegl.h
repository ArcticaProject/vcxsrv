/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef _XEGL_H_
#define _XEGL_H_

#include "xgl.h"

#include "randrstr.h"

#define KD_BUTTON_1     0x01
#define KD_BUTTON_2     0x02
#define KD_BUTTON_3     0x04
#define KD_BUTTON_4     0x08
#define KD_BUTTON_5     0x10
#define KD_MOUSE_DELTA  0x80000000

typedef struct _KdMouseFuncs {
    Bool (*Init) (void);
    void (*Fini) (void);
} KdMouseFuncs;

typedef struct _KdKeyboardFuncs {
    void (*Load) (void);
    int  (*Init) (void);
    void (*Leds) (int);
    void (*Bell) (int, int, int);
    void (*Fini) (void);
    int  LockLed;
} KdKeyboardFuncs;

typedef struct _KdOsFuncs {
    int  (*Init)       (void);
    void (*Enable)     (void);
    Bool (*SpecialKey) (KeySym);
    void (*Disable)    (void);
    void (*Fini)       (void);
    void (*pollEvents) (void);
} KdOsFuncs;

typedef struct _KdMouseMatrix {
    int matrix[2][3];
} KdMouseMatrix;

typedef enum _KdMouseState {
    start,
    button_1_pend,
    button_1_down,
    button_2_down,
    button_3_pend,
    button_3_down,
    synth_2_down_13,
    synth_2_down_3,
    synth_2_down_1,
    num_input_states
} KdMouseState;

#define KD_MAX_BUTTON  7

typedef struct _KdMouseInfo {
    struct _KdMouseInfo *next;
    void		*driver;
    void		*closure;
    char		*name;
    char		*prot;
    char		map[KD_MAX_BUTTON];
    int			nbutton;
    Bool		emulateMiddleButton;
    unsigned long	emulationTimeout;
    Bool		timeoutPending;
    KdMouseState	mouseState;
    Bool		eventHeld;
    xEvent		heldEvent;
    unsigned char	buttonState;
    int			emulationDx, emulationDy;
    int			inputType;
    Bool		transformCoordinates;
} KdMouseInfo;

typedef struct _xeglScreen {
    CloseScreenProcPtr CloseScreen;
    ScreenPtr	       pScreen;
    DDXPointRec	       origin;
} xeglScreenRec, *xeglScreenPtr;

extern KdMouseInfo     *kdMouseInfo;
extern KdOsFuncs       *kdOsFuncs;
extern Bool	       kdDontZap;
extern Bool	       kdDisableZaphod;
extern DevPrivateKey   xeglScreenPrivateKey;
extern KdMouseFuncs    LinuxEvdevMouseFuncs;
extern KdKeyboardFuncs LinuxEvdevKeyboardFuncs;

#define RR_Rotate_All						 \
    (RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270)
#define RR_Reflect_All (RR_Reflect_X | RR_Reflect_Y)

#define KdGetScreenPriv(pScreen) ((xeglScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, xeglScreenPrivateKey))
#define KdScreenPriv(pScreen)				  \
    xeglScreenPtr pScreenPriv = KdGetScreenPriv (pScreen)

void
eglInitInput (KdMouseFuncs    *pMouseFuncs,
	      KdKeyboardFuncs *pKeyboardFuncs);

void
KdParseMouse (char *arg);

KdMouseInfo *
KdMouseInfoAdd (void);

void
KdMouseInfoDispose (KdMouseInfo *mi);

int
KdAllocInputType (void);

char *
KdSaveString (char *str);

Bool
KdRegisterFd (int  type,
	      int  fd,
	      void (*read) (int fd, void *closure),
	      void *closure);

void
KdUnregisterFds (int  type,
		 Bool do_close);

void
KdEnqueueKeyboardEvent (unsigned char scan_code,
			unsigned char is_up);

void
KdEnqueueMouseEvent (KdMouseInfo   *mi,
		     unsigned long flags,
		     int	   rx,
		     int	   ry);

void
KdRegisterFdEnableDisable (int  fd,
			   int  (*enable)  (int fd, void *closure),
			   void (*disable) (int fd, void *closure));

void
KdWakeupHandler (pointer data,
		 int	 result,
		 pointer readmask);

Bool
KdLegalModifier (unsigned int key, 
		 DeviceIntPtr pDev);

void
KdProcessInputEvents (void);

void
xeglInitOutput (ScreenInfo *pScreenInfo,
		int	   argc,
		char       **argv);

Bool
xeglLegalModifier (unsigned int key,
		   DevicePtr    pDev);

void
xeglProcessInputEvents (void);

void
xeglInitInput (int  argc,
	       char **argv);

void
xeglUseMsg (void);

int
xeglProcessArgument (int  argc,
		     char **argv,
		     int  i);

void
xeglAbort (void);

void
xeglGiveUp (void);

void
xeglOsVendorInit (void);

#endif /* _XEGL_H_ */
