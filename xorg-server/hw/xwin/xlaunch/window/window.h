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
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <windows.h>
#include <string>

class CDialog;
class CWindow
{
    friend class CDialog;
    public:
        struct CBoundary
        {
            int left;
            int top;
            int width;
            int height;
            CBoundary() : 
                left(0), top(0), width(0), height(0) {};
            CBoundary(int x, int y, int w, int h) : 
                left(x), top(y), width(w), height(h) {};
            CBoundary(const RECT &r) : 
                left(r.left), top(r.top), width(r.right-r.left), height(r.bottom-r.top) {};
        };
        class CWindowClass
        {
            private:
                WNDPROC wndproc;
                ATOM atom;
                std::string classname;
            protected:
                void Register();
            public:
                CWindowClass(const char *name, WNDPROC wndproc);
                ~CWindowClass();
                const char *GetClassName() { return classname.c_str(); };
        };
    private:
        static CWindowClass windowClass;

        std::string title;
        DWORD exstyle;
        DWORD style;
        CBoundary bounds;
        HWND hwnd; 
        HWND parent;
        WNDPROC owndproc;

        BOOL showing;

    protected:

        virtual const char *GetClassName();
	virtual HWND CreateWindowHandle();
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    public:
        CWindow(const char *title);
	virtual void Create();

        virtual int ShowModal();

        void Show();
        void Hide();

        void SetWidth(int width);
        void SetHeight(int height);
        void SetLeft(int left);
        void SetTop(int top);
        int GetWidth() { return bounds.width; };
        int GetHeight() { return bounds.height; };
        int GetLeft() { return bounds.left; };
        int GetTop() { return bounds.top; };

        void SetBounds(int left, int top, int width, int height);
        void SetBounds(const RECT &rect);

        void SetStyle(DWORD style);
        DWORD GetStyle() { return style; };
        
        void SetExStyle(DWORD exstyle);
        DWORD GetExStyle() { return exstyle; };

        HWND GetHandle();
        void SetParent(CWindow *window);

        virtual LRESULT Dispatch(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
