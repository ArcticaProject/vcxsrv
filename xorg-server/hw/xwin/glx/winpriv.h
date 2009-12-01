/*
 * Export window information for the Windows-OpenGL GLX implementation.
 *
 * Authors: Alexander Gottwald
 */

#include <X11/Xwindows.h>
#include <windowstr.h>

#define WIN_GL_WINDOW_CLASS "XWinGLTest"

typedef struct
{
    HWND    hwnd;
    HRGN    hrgn;
    RECT    rect;
} winWindowInfoRec, *winWindowInfoPtr;

void winGetWindowInfo(WindowPtr pWin, winWindowInfoPtr pWinInfo);
Bool winCheckScreenAiglxIsSupported(ScreenPtr pScreen);

