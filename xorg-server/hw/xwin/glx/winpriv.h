/*
 * Export window information for the Windows-OpenGL GLX implementation.
 *
 * Authors: Alexander Gottwald
 */

#include <X11/Xwindows.h>
#include <windowstr.h>

typedef struct
{
    HWND    hwnd;
} winWindowInfoRec, *winWindowInfoPtr;

void winGetWindowInfo(WindowPtr pWin, winWindowInfoPtr pWinInfo);
Bool winCheckScreenAiglxIsSupported(ScreenPtr pScreen);

