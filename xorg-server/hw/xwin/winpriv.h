/*
 * Export window information for the Windows-OpenGL GLX implementation.
 *
 * Authors: Alexander Gottwald
 */
#include <windows.h>

typedef struct
{
    HWND    hwnd;
    HRGN    hrgn;
    RECT    rect;
} winWindowInfoRec, *winWindowInfoPtr;

extern void winGetWindowInfo(WindowPtr pWin, winWindowInfoPtr pWinInfo);
