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
extern void winGetWindowInfo(WindowPtr pWin, winWindowInfoPtr pWinInfo)
{
    /* Sanity check */
    if (pWinInfo == NULL)
        return;

    winDebug("%s:%d pWin=%p\n", __FUNCTION__, __LINE__, pWin);

    /* a real window was requested */
    if (pWin != NULL) 
    {
        /* Initialize the size information */
        RECT rect = {
            pWin->drawable.x,
            pWin->drawable.y,
            pWin->drawable.x + pWin->drawable.width,
            pWin->drawable.y + pWin->drawable.height
        }, rect_extends;
        /* Get the window and screen privates */
        ScreenPtr pScreen = pWin->drawable.pScreen;
        winPrivScreenPtr pWinScreen = winGetScreenPriv(pScreen);
        winScreenInfoPtr pScreenInfo = NULL;

        rect_extends = rect;
        OffsetRect(&rect_extends, -pWin->drawable.x, -pWin->drawable.y);

        if (pWinScreen == NULL) 
        {
            ErrorF("winGetWindowInfo: screen has no privates\n");
            return;
        }
        
        pWinInfo->hwnd = pWinScreen->hwndScreen;
        pWinInfo->hrgn = NULL;
        pWinInfo->rect = rect;
    

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
            }
            if (pWinPriv->hWnd != NULL) { 
                
                /* copy size and window handle */
                pWinInfo->rect = rect_extends;
                pWinInfo->hwnd = pWinPriv->hWnd;

                /* Copy window region */
                if (pWinInfo->hrgn)
                    DeleteObject(pWinInfo->hrgn);
                pWinInfo->hrgn = CreateRectRgn(0,0,0,0);
                CombineRgn(pWinInfo->hrgn, pWinPriv->hRgn, pWinPriv->hRgn, 
                        RGN_COPY);
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
                /* copy size and window handle */
                pWinInfo->rect = rect_extends;
                pWinInfo->hwnd = pRLWinPriv->hWnd;
            }
            return;
        }
#endif
    } 
    else 
    {
        RECT rect = {0, 0, 0, 0};
        ScreenPtr pScreen = g_ScreenInfo[0].pScreen;
        winPrivScreenPtr pWinScreen = winGetScreenPriv(pScreen);

        pWinInfo->hwnd = NULL;
        pWinInfo->hrgn = NULL;
        pWinInfo->rect = rect;
        
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
