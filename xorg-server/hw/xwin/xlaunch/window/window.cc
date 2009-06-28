/*
 * Copyright (c) 2005 Alexander Gottwald
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#include "window.h"
#include "util.h"
#include <stdio.h>
#include <stdexcept>

CWindow::CWindowClass CWindow::windowClass("CWINDOWCLASS", DefWindowProc);

CWindow::CWindowClass::CWindowClass(const char *_name, WNDPROC _wndproc) : 
    wndproc(_wndproc), atom(0), classname(_name)
{
    Register();
}

CWindow::CWindowClass::~CWindowClass()
{
    UnregisterClass(classname.c_str(), GetModuleHandle(NULL));
}

void CWindow::CWindowClass::Register()
{
    WNDCLASSEX wndclass;
    memset(&wndclass, 0, sizeof(wndclass));
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = 0;
    wndclass.lpfnWndProc = wndproc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = GetModuleHandle(NULL); 
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = classname.c_str();
    wndclass.hIconSm = NULL;
    atom = RegisterClassEx(&wndclass);
    if (atom == 0)
        throw win32_error("RegisterClassEx failed");
}

CWindow::CWindow(const char *_title) : title(_title), hwnd(NULL), parent(NULL), bounds(), owndproc(NULL), showing(FALSE)
{
    style = WS_CHILD;
    exstyle = 0;
}

HWND CWindow::CreateWindowHandle()
{
    HWND ret = CreateWindowEx(
            exstyle,
            GetClassName(),
            title.c_str(),
            style,
            bounds.left,
            bounds.top,
            bounds.width,
            bounds.height,
            parent,
            NULL,
            GetModuleHandle(NULL),
            0
            );
    if (ret == NULL)
        throw win32_error("CreateWindowEx failed");
    return ret;
}

void CWindow::Create()
{
    if (hwnd != NULL)
        return;
    hwnd = CreateWindowHandle();
    if (hwnd == NULL)
        throw win32_error("Could not create window");

    // Reset the error code
    DWORD err = 0;
    SetLastError(err);
    
    // Attach the object reference to the window handle
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    err = GetLastError();
    if (err != 0)
        throw win32_error("SetWindowLongPtr failed",err);
    
    // Set the window proc
    owndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    err = GetLastError();
    if (err != 0)
        throw win32_error("SetWindowLongPtr failed",err);
}

const char *CWindow::GetClassName()
{
    return windowClass.GetClassName();
}

LRESULT CALLBACK CWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MessageDebug::debug(hwnd, uMsg, wParam, lParam, __FUNCTION__);
    CWindow* window = (CWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (window != NULL)
        return window->Dispatch(hwnd, uMsg, wParam, lParam);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CWindow::Dispatch(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SIZE:
            bounds.width = LOWORD(lParam);
            bounds.height = LOWORD(lParam);
            break;
        case WM_MOVE:
            bounds.left = LOWORD(lParam);
            bounds.top = LOWORD(lParam);
            break;
        case WM_DESTROY:
            showing = FALSE;
            break;
    }
    if (owndproc)
        return CallWindowProc(owndproc, hwnd, uMsg, wParam, lParam);
    else
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CWindow::Show()
{
    if (hwnd == NULL)
        Create();
    ShowWindow(hwnd, SW_SHOWNORMAL);
}

int CWindow::ShowModal()
{
    MSG msg;
    BOOL bRet;
    showing = TRUE;
    Show();

    while( showing && (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    return 0; 
}

void CWindow::SetLeft(int left)
{
    bounds.left = left;
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL, 
                bounds.left, bounds.top,
                0, 0, 
                SWP_NOZORDER |  SWP_NOSIZE))
            throw win32_error("SetWindowPos failed");
}

void CWindow::SetTop(int top)
{
    bounds.top = top;
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL, 
                bounds.left, bounds.top,
                0, 0, 
                SWP_NOZORDER |  SWP_NOSIZE))
            throw win32_error("SetWindowPos failed");
}

void CWindow::SetWidth(int width)
{
    bounds.width = width;
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL,
                0, 0, 
                bounds.width, bounds.height, 
                SWP_NOZORDER |  SWP_NOMOVE))
            throw win32_error("SetWindowPos failed");
}
void CWindow::SetHeight(int height)
{
    bounds.height = height;
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL,
                0, 0, 
                bounds.width, bounds.height,
                SWP_NOZORDER |  SWP_NOMOVE))
            throw win32_error("SetWindowPos failed");
}

void CWindow::SetBounds(int left, int top, int width, int height)
{
    bounds = CBoundary(left, top, width, height);
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL, 
                bounds.left, bounds.top, 
                bounds.width, bounds.height, 
                SWP_NOZORDER))
            throw win32_error("SetWindowPos failed");
}

void CWindow::SetBounds(const RECT &rect)
{
    bounds = rect;
    if (hwnd)
        if (!SetWindowPos(hwnd, NULL, 
                bounds.left, bounds.top, 
                bounds.width, bounds.height, 
                SWP_NOZORDER))
            throw win32_error("SetWindowPos failed");
}

HWND CWindow::GetHandle()
{
    if (hwnd == NULL)
        Create();
    return hwnd;
}

void CWindow::SetParent(CWindow *window)
{
    parent = window->GetHandle();
    if (hwnd != NULL)
        if (::SetParent(hwnd, parent) == NULL)
            throw win32_error("SetParent failed");
        
}

void CWindow::SetStyle(DWORD style)
{
    this->style = style;
    SetLastError(0);
    if (hwnd)
        SetWindowLong(hwnd, GWL_STYLE, style);
    int err = GetLastError();
    if (err != 0)
        throw win32_error("SetWindowLong failed", err);
}

void CWindow::SetExStyle(DWORD exstyle)
{
    this->exstyle = exstyle;
    SetLastError(0);
    if (hwnd)
        SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
    int err = GetLastError();
    if (err != 0)
        throw win32_error("SetWindowWLong failed", err);
}
