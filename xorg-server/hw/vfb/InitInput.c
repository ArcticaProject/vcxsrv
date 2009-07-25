/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_EVENTS
#include "mi.h"
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "inputstr.h"
#include <X11/Xos.h>
#include "mibstore.h"
#include "mipointer.h"
#include "lk201kbd.h"
#include <X11/keysym.h>

Bool
LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
    return TRUE;
}

void
ProcessInputEvents()
{
    mieqProcessInputEvents();
}

void DDXRingBell(int volume, int pitch, int duration)
{
}

#define VFB_MIN_KEY 8
#define VFB_MAX_KEY 255
KeySym  map[MAP_LENGTH * LK201_GLYPHS_PER_KEY];

/* The only reason for using the LK201 mappings here was that they were
 * easy to lift.
 */
static Bool
GetLK201Mappings(KeySymsPtr pKeySyms, CARD8 *pModMap)
{
#define INDEX(in) ((in - VFB_MIN_KEY) * LK201_GLYPHS_PER_KEY)
    int i;

    for (i = 0; i < MAP_LENGTH; i++)
	pModMap[i] = NoSymbol;	/* make sure it is restored */
    pModMap[ KEY_LOCK ] = LockMask;
    pModMap[ KEY_SHIFT ] = ShiftMask;
    pModMap[ KEY_CTRL ] = ControlMask;
    pModMap[ KEY_COMPOSE ] = Mod1Mask;

    pKeySyms->minKeyCode = VFB_MIN_KEY;
    pKeySyms->maxKeyCode = VFB_MAX_KEY;
    pKeySyms->mapWidth = LK201_GLYPHS_PER_KEY;
    pKeySyms->map = map;

    for (i = 0; i < (MAP_LENGTH * LK201_GLYPHS_PER_KEY); i++)
	map[i] = NoSymbol;	/* make sure it is restored */

    map[INDEX(KEY_F1)] = XK_F1;
    map[INDEX(KEY_F2)] = XK_F2;
    map[INDEX(KEY_F3)] = XK_F3;
    map[INDEX(KEY_F4)] = XK_F4;
    map[INDEX(KEY_F5)] = XK_F5;
    map[INDEX(KEY_F6)] = XK_F6;
    map[INDEX(KEY_F7)] = XK_F7;
    map[INDEX(KEY_F8)] = XK_F8;
    map[INDEX(KEY_F9)] = XK_F9;
    map[INDEX(KEY_F10)] = XK_F10;
    map[INDEX(KEY_F11)] = XK_F11;
    map[INDEX(KEY_F12)] = XK_F12;
    map[INDEX(KEY_F13)] = XK_F13;
    map[INDEX(KEY_F14)] = XK_F14;

    map[INDEX(KEY_HELP)] = XK_Help;
    map[INDEX(KEY_MENU)] = XK_Menu;

    map[INDEX(KEY_F17)] = XK_F17;
    map[INDEX(KEY_F18)] = XK_F18;
    map[INDEX(KEY_F19)] = XK_F19;
    map[INDEX(KEY_F20)] = XK_F20;

    map[INDEX(KEY_FIND)] = XK_Find;
    map[INDEX(KEY_INSERT_HERE)] = XK_Insert;
    map[INDEX(KEY_REMOVE)] = XK_Delete;
    map[INDEX(KEY_SELECT)] = XK_Select;
    map[INDEX(KEY_PREV_SCREEN)] = XK_Prior;
    map[INDEX(KEY_NEXT_SCREEN)] = XK_Next;

    map[INDEX(KEY_KP_0)] = XK_KP_0;
    map[INDEX(KEY_KP_PERIOD)] = XK_KP_Decimal;
    map[INDEX(KEY_KP_ENTER)] = XK_KP_Enter;
    map[INDEX(KEY_KP_1)] = XK_KP_1;
    map[INDEX(KEY_KP_2)] = XK_KP_2;
    map[INDEX(KEY_KP_3)] = XK_KP_3;
    map[INDEX(KEY_KP_4)] = XK_KP_4;
    map[INDEX(KEY_KP_5)] = XK_KP_5;
    map[INDEX(KEY_KP_6)] = XK_KP_6;
    map[INDEX(KEY_KP_COMMA)] = XK_KP_Separator;
    map[INDEX(KEY_KP_7)] = XK_KP_7;
    map[INDEX(KEY_KP_8)] = XK_KP_8;
    map[INDEX(KEY_KP_9)] = XK_KP_9;
    map[INDEX(KEY_KP_HYPHEN)] = XK_KP_Subtract;
    map[INDEX(KEY_KP_PF1)] = XK_KP_F1;
    map[INDEX(KEY_KP_PF2)] = XK_KP_F2;
    map[INDEX(KEY_KP_PF3)] = XK_KP_F3;
    map[INDEX(KEY_KP_PF4)] = XK_KP_F4;

    map[INDEX(KEY_LEFT)] = XK_Left;
    map[INDEX(KEY_RIGHT)] = XK_Right;
    map[INDEX(KEY_DOWN)] = XK_Down;
    map[INDEX(KEY_UP)] = XK_Up;

    map[INDEX(KEY_SHIFT)] = XK_Shift_L;
    map[INDEX(KEY_CTRL)] = XK_Control_L;
    map[INDEX(KEY_LOCK)] = XK_Caps_Lock;
    map[INDEX(KEY_COMPOSE)] = XK_Multi_key;
    map[INDEX(KEY_COMPOSE)+1] = XK_Meta_L;
    map[INDEX(KEY_DELETE)] = XK_Delete;
    map[INDEX(KEY_RETURN)] = XK_Return;
    map[INDEX(KEY_TAB)] = XK_Tab;

    map[INDEX(KEY_TILDE)] = XK_quoteleft;
    map[INDEX(KEY_TILDE)+1] = XK_asciitilde;

    map[INDEX(KEY_TR_1)] = XK_1;                 
    map[INDEX(KEY_TR_1)+1] = XK_exclam;
    map[INDEX(KEY_Q)] = XK_Q;
    map[INDEX(KEY_A)] = XK_A;
    map[INDEX(KEY_Z)] = XK_Z;

    map[INDEX(KEY_TR_2)] = XK_2;
    map[INDEX(KEY_TR_2)+1] = XK_at;

    map[INDEX(KEY_W)] = XK_W;
    map[INDEX(KEY_S)] = XK_S;
    map[INDEX(KEY_X)] = XK_X;

    map[INDEX(KEY_LANGLE_RANGLE)] = XK_less;
    map[INDEX(KEY_LANGLE_RANGLE)+1] = XK_greater;

    map[INDEX(KEY_TR_3)] = XK_3;
    map[INDEX(KEY_TR_3)+1] = XK_numbersign;

    map[INDEX(KEY_E)] = XK_E;
    map[INDEX(KEY_D)] = XK_D;
    map[INDEX(KEY_C)] = XK_C;

    map[INDEX(KEY_TR_4)] = XK_4;
    map[INDEX(KEY_TR_4)+1] = XK_dollar;

    map[INDEX(KEY_R)] = XK_R;
    map[INDEX(KEY_F)] = XK_F;
    map[INDEX(KEY_V)] = XK_V;
    map[INDEX(KEY_SPACE)] = XK_space;

    map[INDEX(KEY_TR_5)] = XK_5;
    map[INDEX(KEY_TR_5)+1] = XK_percent;

    map[INDEX(KEY_T)] = XK_T;
    map[INDEX(KEY_G)] = XK_G;
    map[INDEX(KEY_B)] = XK_B;

    map[INDEX(KEY_TR_6)] = XK_6;
    map[INDEX(KEY_TR_6)+1] = XK_asciicircum;

    map[INDEX(KEY_Y)] = XK_Y;
    map[INDEX(KEY_H)] = XK_H;
    map[INDEX(KEY_N)] = XK_N;

    map[INDEX(KEY_TR_7)] = XK_7;
    map[INDEX(KEY_TR_7)+1] = XK_ampersand;

    map[INDEX(KEY_U)] = XK_U;
    map[INDEX(KEY_J)] = XK_J;
    map[INDEX(KEY_M)] = XK_M;

    map[INDEX(KEY_TR_8)] = XK_8;
    map[INDEX(KEY_TR_8)+1] = XK_asterisk;

    map[INDEX(KEY_I)] = XK_I;
    map[INDEX(KEY_K)] = XK_K;

    map[INDEX(KEY_COMMA)] = XK_comma;
    map[INDEX(KEY_COMMA)+1] = XK_less;

    map[INDEX(KEY_TR_9)] = XK_9;
    map[INDEX(KEY_TR_9)+1] = XK_parenleft;

    map[INDEX(KEY_O)] = XK_O;
    map[INDEX(KEY_L)] = XK_L;

    map[INDEX(KEY_PERIOD)] = XK_period;
    map[INDEX(KEY_PERIOD)+1] = XK_greater;

    map[INDEX(KEY_TR_0)] = XK_0;
    map[INDEX(KEY_TR_0)+1] = XK_parenright;

    map[INDEX(KEY_P)] = XK_P;

    map[INDEX(KEY_SEMICOLON)] = XK_semicolon;
    map[INDEX(KEY_SEMICOLON)+1] = XK_colon;

    map[INDEX(KEY_QMARK)] = XK_slash;   
    map[INDEX(KEY_QMARK)+1] = XK_question;

    map[INDEX(KEY_PLUS)] = XK_equal;
    map[INDEX(KEY_PLUS)+1] = XK_plus;

    map[INDEX(KEY_RBRACE)] = XK_bracketright;
    map[INDEX(KEY_RBRACE)+1] = XK_braceright;

    map[INDEX(KEY_VBAR)] = XK_backslash;
    map[INDEX(KEY_VBAR)+1] = XK_bar;

    map[INDEX(KEY_UBAR)] = XK_minus;
    map[INDEX(KEY_UBAR)+1] = XK_underscore;

    map[INDEX(KEY_LBRACE)] = XK_bracketleft;
    map[INDEX(KEY_LBRACE)+1] = XK_braceleft;

    map[INDEX(KEY_QUOTE)] = XK_quoteright;
    map[INDEX(KEY_QUOTE)+1] = XK_quotedbl;

    map[INDEX(KEY_F11)] = XK_Escape;

    return TRUE;
#undef INDEX
}

static int
vfbKeybdProc(DeviceIntPtr pDevice, int onoff)
{
    KeySymsRec		keySyms;
    CARD8 		modMap[MAP_LENGTH];
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT: 
	GetLK201Mappings(&keySyms, modMap);
	InitKeyboardDeviceStruct(pDev, &keySyms, modMap,
			(BellProcPtr)NoopDDA, (KbdCtrlProcPtr)NoopDDA);
	    break;
    case DEVICE_ON: 
	pDev->on = TRUE;
	break;
    case DEVICE_OFF: 
	pDev->on = FALSE;
	break;
    case DEVICE_CLOSE:
	break;
    }
    return Success;
}

static int
vfbMouseProc(DeviceIntPtr pDevice, int onoff)
{
    BYTE map[4];
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT:
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(pDev, map, 3,
		(PtrCtrlProcPtr)NoopDDA, GetMotionHistorySize(), 2);
	    break;

    case DEVICE_ON:
	pDev->on = TRUE;
        break;

    case DEVICE_OFF:
	pDev->on = FALSE;
	break;

    case DEVICE_CLOSE:
	break;
    }
    return Success;
}

void
InitInput(int argc, char *argv[])
{
    DeviceIntPtr p, k;
    p = AddInputDevice(serverClient, vfbMouseProc, TRUE);
    k = AddInputDevice(serverClient, vfbKeybdProc, TRUE);
    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    (void)mieqInit();
}
