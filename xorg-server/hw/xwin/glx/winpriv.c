/*
 * Export window information for the Windows-OpenGL GLX implementation.
 *
 * Authors: Alexander Gottwald
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include "winpriv.h"
#include "winwindow.h"

void
winCreateWindowsWindow (WindowPtr pWin);
/**
 * Return size and handles of a window.
 * If pWin is NULL, then the information for the root window is requested.
 */
void winGetWindowInfo(WindowPtr pWin, winWindowInfoPtr pWinInfo)
{
    /* Sanity check */
    if (pWinInfo == NULL)
        return;

    winDebug("%s:%d pWin=%p\n", __FUNCTION__, __LINE__, pWin);

    /* a real window was requested */
    if (pWin != NULL)
    {
        /* Get the window and screen privates */
        ScreenPtr pScreen = pWin->drawable.pScreen;
        winPrivScreenPtr pWinScreen = winGetScreenPriv(pScreen);
        winScreenInfoPtr pScreenInfo = NULL;

        if (pWinScreen == NULL)
        {
            ErrorF("winGetWindowInfo: screen has no privates\n");
            return;
        }

        pWinInfo->hwnd = pWinScreen->hwndScreen;

        pScreenInfo = pWinScreen->pScreenInfo;
#ifdef XWIN_MULTIWINDOW
        /* check for multiwindow mode */
        if (pScreenInfo->fMultiWindow)
        {
            winWindowPriv(pWin);

            if (pWinPriv == NULL)
            {
                ErrorF("winGetWindowInfo: window has no privates\n");
                return;
            }

            if (pWinPriv->hWnd == NULL)
            {
              winCreateWindowsWindow(pWin);
              ErrorF("winGetWindowInfo: forcing window to exist...\n");
            }

            if (pWinPriv->hWnd != NULL)
              {
                /* copy window handle */
                pWinInfo->hwnd = pWinPriv->hWnd;
              }

            return;
        }
#endif
#ifdef XWIN_MULTIWINDOWEXTWM
        /* check for multiwindow external wm mode */
        if (pScreenInfo->fMWExtWM)
        {
            win32RootlessWindowPtr pRLWinPriv
                = (win32RootlessWindowPtr) RootlessFrameForWindow (pWin, FALSE);

            if (pRLWinPriv == NULL) {
                ErrorF("winGetWindowInfo: window has no privates\n");
                return;
            }

            if (pRLWinPriv->hWnd != NULL)
            {
                /* copy window handle */
                pWinInfo->hwnd = pRLWinPriv->hWnd;
            }
            return;
        }
#endif
    }
    else
    {
        ScreenPtr pScreen = g_ScreenInfo[0].pScreen;
        winPrivScreenPtr pWinScreen = winGetScreenPriv(pScreen);

        pWinInfo->hwnd = NULL;

        if (pWinScreen == NULL)
        {
            ErrorF("winGetWindowInfo: screen has no privates\n");
            return;
        }

        ErrorF("winGetWindowInfo: returning root window\n");

        pWinInfo->hwnd = pWinScreen->hwndScreen;
    }
    return;
}

Bool
winCheckScreenAiglxIsSupported(ScreenPtr pScreen)
{
  winPrivScreenPtr pWinScreen = winGetScreenPriv(pScreen);
  winScreenInfoPtr pScreenInfo = pWinScreen->pScreenInfo;

#ifdef XWIN_MULTIWINDOW
  if (pScreenInfo->fMultiWindow)
    return TRUE;
#endif

#ifdef XWIN_MULTIWINDOWEXTWM
  if (pScreenInfo->fMWExtWM)
    return TRUE;
#endif

  return FALSE;
}
