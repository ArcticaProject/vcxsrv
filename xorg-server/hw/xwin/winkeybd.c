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
 * Authors:	Dakshinamurthy Karra
 *		Suhaib M Siddiqi
 *		Peter Busch
 *		Harold L Hunt II
 */


#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include "winkeybd.h"
#include "winconfig.h"
#include "winmsg.h"

#include "xkbsrv.h"

static Bool g_winKeyState[NUM_KEYCODES];

/*
 * Local prototypes
 */

static void
winGetKeyMappings (KeySymsPtr pKeySyms, CARD8 *pModMap);

static void
winKeybdBell (int iPercent, DeviceIntPtr pDeviceInt,
	      pointer pCtrl, int iClass);

static void
winKeybdCtrl (DeviceIntPtr pDevice, KeybdCtrl *pCtrl);


/* 
 * Translate a Windows WM_[SYS]KEY(UP/DOWN) message
 * into an ASCII scan code.
 *
 * We do this ourselves, rather than letting Windows handle it,
 * because Windows tends to munge the handling of special keys,
 * like AltGr on European keyboards.
 */

void
winTranslateKey (WPARAM wParam, LPARAM lParam, int *piScanCode)
{
  int		iKeyFixup = g_iKeyMap[wParam * WIN_KEYMAP_COLS + 1];
  int		iKeyFixupEx = g_iKeyMap[wParam * WIN_KEYMAP_COLS + 2];
  int		iParam = HIWORD (lParam);
  int		iParamScanCode = LOBYTE (iParam);

/* WM_ key messages faked by Vista speech recognition (WSR) don't have a
 * scan code.
 *
 * Vocola 3 (Rick Mohr's supplement to WSR) uses
 * System.Windows.Forms.SendKeys.SendWait(), which appears always to give a
 * scan code of 1
 */
  if (iParamScanCode <= 1)
    {
      if (VK_PRIOR <= wParam && wParam <= VK_DOWN)
        /* Trigger special case table to translate to extended
         * keycode, otherwise if num_lock is on, we can get keypad
         * numbers instead of navigation keys. */
        iParam |= KF_EXTENDED;
      else
        iParamScanCode = MapVirtualKeyEx(wParam,
                         /*MAPVK_VK_TO_VSC*/0,
                         GetKeyboardLayout(0));
    }

  /* Branch on special extended, special non-extended, or normal key */
  if ((iParam & KF_EXTENDED) && iKeyFixupEx)
    *piScanCode = iKeyFixupEx;
  else if (iKeyFixup)
    *piScanCode = iKeyFixup;
  else if (wParam == 0 && iParamScanCode == 0x70)
    *piScanCode = KEY_HKTG;
  else
    switch (iParamScanCode)
    {
      case 0x70:
        *piScanCode = KEY_HKTG;
        break;
      case 0x73:
        *piScanCode = KEY_BSlash2;
        break;
      default: 
        *piScanCode = iParamScanCode;
        break;
    }
}


/*
 * We call this function from winKeybdProc when we are
 * initializing the keyboard.
 */

static void
winGetKeyMappings (KeySymsPtr pKeySyms, CARD8 *pModMap)
{
  int			i;
  KeySym		*pMap = map;
  KeySym		*pKeySym;

  /*
   * Initialize all key states to up... which may not be true
   * but it is close enough.
   */
  ZeroMemory (g_winKeyState, sizeof (g_winKeyState[0]) * NUM_KEYCODES);

  /* MAP_LENGTH is defined in Xserver/include/input.h to be 256 */
  for (i = 0; i < MAP_LENGTH; i++)
    pModMap[i] = NoSymbol;  /* make sure it is restored */

  /* Loop through all valid entries in the key symbol table */
  for (pKeySym = pMap, i = MIN_KEYCODE;
       i < (MIN_KEYCODE + NUM_KEYCODES);
       i++, pKeySym += GLYPHS_PER_KEY)
    {
      switch (*pKeySym)
	{
	case XK_Shift_L:
	case XK_Shift_R:
	  pModMap[i] = ShiftMask;
	  break;

	case XK_Control_L:
	case XK_Control_R:
	  pModMap[i] = ControlMask;
	  break;

	case XK_Caps_Lock:
	  pModMap[i] = LockMask;
	  break;

	case XK_Alt_L:
	case XK_Alt_R:
	  pModMap[i] = AltMask;
	  break;

	case XK_Num_Lock:
	  pModMap[i] = NumLockMask;
	  break;

	case XK_Scroll_Lock:
	  pModMap[i] = ScrollLockMask;
	  break;

#if 0
	case XK_Super_L:
	case XK_Super_R:
	  pModMap[i] = Mod4Mask;
	  break;
#else
	/* Hirigana/Katakana toggle */
	case XK_Kana_Lock:
	case XK_Kana_Shift:
	  pModMap[i] = KanaMask;
	  break;
#endif

	/* alternate toggle for multinational support */
	case XK_Mode_switch:
	  pModMap[i] = AltLangMask;
	  break;
	}
    }

  pKeySyms->map        = (KeySym *) pMap;
  pKeySyms->mapWidth   = GLYPHS_PER_KEY;
  pKeySyms->minKeyCode = MIN_KEYCODE;
  pKeySyms->maxKeyCode = MAX_KEYCODE;
}


/* Ring the keyboard bell (system speaker on PCs) */
static void
winKeybdBell (int iPercent, DeviceIntPtr pDeviceInt,
	      pointer pCtrl, int iClass)
{
  /*
   * We can't use Beep () here because it uses the PC speaker
   * on NT/2000.  MessageBeep (MB_OK) will play the default system
   * sound on systems with a sound card or it will beep the PC speaker
   * on systems that do not have a sound card.
   */
  MessageBeep (MB_OK);
}


/* Change some keyboard configuration parameters */
static void
winKeybdCtrl (DeviceIntPtr pDevice, KeybdCtrl *pCtrl)
{
}


/* 
 * See Porting Layer Definition - p. 18
 * winKeybdProc is known as a DeviceProc.
 */

int
winKeybdProc (DeviceIntPtr pDeviceInt, int iState)
{
  DevicePtr		pDevice = (DevicePtr) pDeviceInt;
  XkbSrvInfoPtr       xkbi;
  XkbControlsPtr      ctrl;

  switch (iState)
    {
    case DEVICE_INIT:
      winConfigKeyboard (pDeviceInt);

      /* FIXME: Maybe we should use winGetKbdLeds () here? */
      defaultKeyboardControl.leds = g_winInfo.keyboard.leds;

      winErrorFVerb(2, "Rules = \"%s\" Model = \"%s\" Layout = \"%s\""
                    " Variant = \"%s\" Options = \"%s\"\n",
                    g_winInfo.xkb.rules ? g_winInfo.xkb.rules : "none",
                    g_winInfo.xkb.model ? g_winInfo.xkb.model : "none",
                    g_winInfo.xkb.layout ? g_winInfo.xkb.layout : "none",
                    g_winInfo.xkb.variant ? g_winInfo.xkb.variant : "none",
                    g_winInfo.xkb.options ? g_winInfo.xkb.options : "none");

      InitKeyboardDeviceStruct (pDeviceInt,
                                &g_winInfo.xkb,
                                winKeybdBell,
                                winKeybdCtrl);

      xkbi = pDeviceInt->key->xkbInfo;
      if ((xkbi != NULL) && (xkbi->desc != NULL))
        {
          ctrl = xkbi->desc->ctrls;
          ctrl->repeat_delay = g_winInfo.keyboard.delay;
          ctrl->repeat_interval = 1000/g_winInfo.keyboard.rate;
        }
      else
        {
          winErrorFVerb (1, "winKeybdProc - Error initializing keyboard AutoRepeat\n");
        }

      break;
      
    case DEVICE_ON: 
      pDevice->on = TRUE;

      // immediately copy the state of this keyboard device to the VCK
      // (which otherwise happens lazily after the first keypress)
      CopyKeyClass(pDeviceInt, inputInfo.keyboard);
      break;

    case DEVICE_CLOSE:
    case DEVICE_OFF: 
      pDevice->on = FALSE;
      break;
    }

  return Success;
}


/*
 * Detect current mode key states upon server startup.
 *
 * Simulate a press and release of any key that is currently
 * toggled.
 */

void
winInitializeModeKeyStates (void)
{
  /* Restore NumLock */
  if (GetKeyState (VK_NUMLOCK) & 0x0001)
    {
      winSendKeyEvent (KEY_NumLock, TRUE);
      winSendKeyEvent (KEY_NumLock, FALSE);
    }

  /* Restore CapsLock */
  if (GetKeyState (VK_CAPITAL) & 0x0001)
    {
      winSendKeyEvent (KEY_CapsLock, TRUE);
      winSendKeyEvent (KEY_CapsLock, FALSE);
    }

  /* Restore ScrollLock */
  if (GetKeyState (VK_SCROLL) & 0x0001)
    {
      winSendKeyEvent (KEY_ScrollLock, TRUE);
      winSendKeyEvent (KEY_ScrollLock, FALSE);
    }

  /* Restore KanaLock */
  if (GetKeyState (VK_KANA) & 0x0001)
    {
      winSendKeyEvent (KEY_HKTG, TRUE);
      winSendKeyEvent (KEY_HKTG, FALSE);
    }
}


/*
 * Upon regaining the keyboard focus we must
 * resynchronize our internal mode key states
 * with the actual state of the keys.
 */

void
winRestoreModeKeyStates (void)
{
  DWORD			dwKeyState;
  BOOL			processEvents = TRUE;
  unsigned short	internalKeyStates;

  /* X server is being initialized */
  if (!inputInfo.keyboard)
    return;

  /* Only process events if the rootwindow is mapped. The keyboard events
   * will cause segfaults otherwise */
  if (WindowTable && WindowTable[0] && WindowTable[0]->mapped == FALSE)
    processEvents = FALSE;    
  
  /* Force to process all pending events in the mi event queue */
  if (processEvents)
    mieqProcessInputEvents ();
  
  /* Read the mode key states of our X server */
  /* (stored in the virtual core keyboard) */
  internalKeyStates = XkbStateFieldFromRec(&inputInfo.keyboard->key->xkbInfo->state);
  winDebug("winRestoreModeKeyStates: state %d\n", internalKeyStates);

  /* 
   * NOTE: The C XOR operator, ^, will not work here because it is
   * a bitwise operator, not a logical operator.  C does not
   * have a logical XOR operator, so we use a macro instead.
   */

  /* Has the key state changed? */
  dwKeyState = GetKeyState (VK_NUMLOCK) & 0x0001;
  if (WIN_XOR (internalKeyStates & NumLockMask, dwKeyState))
    {
      winSendKeyEvent (KEY_NumLock, TRUE);
      winSendKeyEvent (KEY_NumLock, FALSE);
    }

  /* Has the key state changed? */
  dwKeyState = GetKeyState (VK_CAPITAL) & 0x0001;
  if (WIN_XOR (internalKeyStates & LockMask, dwKeyState))
    {
      winSendKeyEvent (KEY_CapsLock, TRUE);
      winSendKeyEvent (KEY_CapsLock, FALSE);
    }

  /* Has the key state changed? */
  dwKeyState = GetKeyState (VK_SCROLL) & 0x0001;
  if (WIN_XOR (internalKeyStates & ScrollLockMask, dwKeyState))
    {
      winSendKeyEvent (KEY_ScrollLock, TRUE);
      winSendKeyEvent (KEY_ScrollLock, FALSE);
    }

  /* Has the key state changed? */
  dwKeyState = GetKeyState (VK_KANA) & 0x0001;
  if (WIN_XOR (internalKeyStates & KanaMask, dwKeyState))
    {
      winSendKeyEvent (KEY_HKTG, TRUE);
      winSendKeyEvent (KEY_HKTG, FALSE);
    }
}


/*
 * Look for the lovely fake Control_L press/release generated by Windows
 * when AltGr is pressed/released on a non-U.S. keyboard.
 */

Bool
winIsFakeCtrl_L (UINT message, WPARAM wParam, LPARAM lParam)
{
  MSG		msgNext;
  LONG		lTime;
  Bool		fReturn;

  /*
   * Fake Ctrl_L presses will be followed by an Alt_R keypress
   * with the same timestamp as the Ctrl_L press.
   */
  if ((message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
      && wParam == VK_CONTROL
      && (HIWORD (lParam) & KF_EXTENDED) == 0)
    {
      /* Got a Ctrl_L press */

      /* Get time of current message */
      lTime = GetMessageTime ();

      /* Look for fake Ctrl_L preceeding an Alt_R press. */
      fReturn = PeekMessage (&msgNext, NULL,
			     WM_KEYDOWN, WM_SYSKEYDOWN,
			     PM_NOREMOVE);

      /*
       * Try again if the first call fails.
       * NOTE: This usually happens when TweakUI is enabled.
       */
      if (!fReturn)
	{
	  /* Voodoo to make sure that the Alt_R message has posted */
	  Sleep (0);

	  /* Look for fake Ctrl_L preceeding an Alt_R press. */
	  fReturn = PeekMessage (&msgNext, NULL,
				 WM_KEYDOWN, WM_SYSKEYDOWN,
				 PM_NOREMOVE);
	}
      if (msgNext.message != WM_KEYDOWN && msgNext.message != WM_SYSKEYDOWN)
          fReturn = 0;

      /* Is next press an Alt_R with the same timestamp? */
      if (fReturn && msgNext.wParam == VK_MENU
	  && msgNext.time == lTime
	  && (HIWORD (msgNext.lParam) & KF_EXTENDED))
	{
	  /* 
	   * Next key press is Alt_R with same timestamp as current
	   * Ctrl_L message.  Therefore, this Ctrl_L press is a fake
	   * event, so discard it.
	   */
	  return TRUE;
	}
    }

  /* 
   * Fake Ctrl_L releases will be followed by an Alt_R release
   * with the same timestamp as the Ctrl_L release.
   */
  if ((message == WM_KEYUP || message == WM_SYSKEYUP)
      && wParam == VK_CONTROL
      && (HIWORD (lParam) & KF_EXTENDED) == 0)
    {
      /* Got a Ctrl_L release */

      /* Get time of current message */
      lTime = GetMessageTime ();

      /* Look for fake Ctrl_L release preceeding an Alt_R release. */
      fReturn = PeekMessage (&msgNext, NULL,
			     WM_KEYUP, WM_SYSKEYUP, 
			     PM_NOREMOVE);

      /*
       * Try again if the first call fails.
       * NOTE: This usually happens when TweakUI is enabled.
       */
      if (!fReturn)
	{
	  /* Voodoo to make sure that the Alt_R message has posted */
	  Sleep (0);

	  /* Look for fake Ctrl_L release preceeding an Alt_R release. */
	  fReturn = PeekMessage (&msgNext, NULL,
				 WM_KEYUP, WM_SYSKEYUP, 
				 PM_NOREMOVE);
	}

      if (msgNext.message != WM_KEYUP && msgNext.message != WM_SYSKEYUP)
          fReturn = 0;
      
      /* Is next press an Alt_R with the same timestamp? */
      if (fReturn
	  && (msgNext.message == WM_KEYUP
	      || msgNext.message == WM_SYSKEYUP)
	  && msgNext.wParam == VK_MENU
	  && msgNext.time == lTime
	  && (HIWORD (msgNext.lParam) & KF_EXTENDED))
	{
	  /*
	   * Next key release is Alt_R with same timestamp as current
	   * Ctrl_L message. Therefore, this Ctrl_L release is a fake
	   * event, so discard it.
	   */
	  return TRUE;
	}
    }
  
  /* Not a fake control left press/release */
  return FALSE;
}


/*
 * Lift any modifier keys that are pressed
 */

void
winKeybdReleaseKeys (void)
{
  int				i;

#ifdef HAS_DEVWINDOWS
  /* Verify that the mi input system has been initialized */
  if (g_fdMessageQueue == WIN_FD_INVALID)
    return;
#endif

  /* Loop through all keys */
  for (i = 0; i < NUM_KEYCODES; ++i)
    {
      /* Pop key if pressed */
      if (g_winKeyState[i])
	winSendKeyEvent (i, FALSE);

      /* Reset pressed flag for keys */
      g_winKeyState[i] = FALSE;
    }
}


/*
 * Take a raw X key code and send an up or down event for it.
 *
 * Thanks to VNC for inspiration, though it is a simple function.
 */

void
winSendKeyEvent (DWORD dwKey, Bool fDown)
{
  EventListPtr events;
  int i, nevents;

  /*
   * When alt-tabing between screens we can get phantom key up messages
   * Here we only pass them through it we think we should!
   */
  if (g_winKeyState[dwKey] == FALSE && fDown == FALSE) return;

  /* Update the keyState map */
  g_winKeyState[dwKey] = fDown;

  GetEventList(&events);
  nevents = GetKeyboardEvents(events, g_pwinKeyboard, fDown ? KeyPress : KeyRelease, dwKey + MIN_KEYCODE);

  for (i = 0; i < nevents; i++)
    mieqEnqueue(g_pwinKeyboard, events[i].event);

#if CYGDEBUG
  ErrorF("winSendKeyEvent: dwKey: %d, fDown: %d, nEvents %d\n",
          dwKey, fDown, nevents);
#endif
}

BOOL winCheckKeyPressed(WPARAM wParam, LPARAM lParam)
{
  switch (wParam)
  {
    case VK_CONTROL:
      if ((lParam & 0x1ff0000) == 0x11d0000 && g_winKeyState[KEY_RCtrl])
        return TRUE;
      if ((lParam & 0x1ff0000) == 0x01d0000 && g_winKeyState[KEY_LCtrl])
        return TRUE;
      break;
    case VK_SHIFT:
      if ((lParam & 0x1ff0000) == 0x0360000 && g_winKeyState[KEY_ShiftR])
        return TRUE;
      if ((lParam & 0x1ff0000) == 0x02a0000 && g_winKeyState[KEY_ShiftL])
        return TRUE;
      break;
    default:
      return TRUE;
  }
  return FALSE;
}

/* Only on shift release message is sent even if both are pressed.
 * Fix this here 
 */
void winFixShiftKeys (int iScanCode)
{
  if (GetKeyState (VK_SHIFT) & 0x8000)
    return;

  if (iScanCode == KEY_ShiftL && g_winKeyState[KEY_ShiftR])
    winSendKeyEvent (KEY_ShiftR, FALSE);
  if (iScanCode == KEY_ShiftR && g_winKeyState[KEY_ShiftL])
    winSendKeyEvent (KEY_ShiftL, FALSE);
}
