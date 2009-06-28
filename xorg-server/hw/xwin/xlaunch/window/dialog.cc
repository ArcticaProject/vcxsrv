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
#include <stdio.h>
#include "dialog.h"
#include "util.h"

CBaseDialog::CBaseDialog() : CWindow(""), result(0)
{
}

CDialog::CDialog(const char *res) : CBaseDialog(), resourcename(res) 
{
}

HWND CDialog::CreateWindowHandle()
{
    HWND ret = CreateDialog(
            GetModuleHandle(NULL),
            resourcename.c_str(),
            NULL,
            DialogProc);
    if (ret == NULL)
        throw win32_error("CreateDialog failed");
    return ret;
}

INT_PTR CALLBACK CBaseDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MessageDebug::debug(hwndDlg, uMsg, wParam, lParam, __FUNCTION__);
	CBaseDialog* dialog = (CDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dialog != NULL)
	    return dialog->DlgDispatch(hwndDlg, uMsg, wParam, lParam);
    return FALSE;
}

INT_PTR CBaseDialog::DlgDispatch(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    result = wParam;
                    EndDialog(hwndDlg, wParam);
                    DestroyWindow(hwndDlg);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

INT_PTR CDialog::DlgDispatch(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return CBaseDialog::DlgDispatch(hwndDlg, uMsg, wParam, lParam);
}

int CBaseDialog::Execute()
{
    return CWindow::ShowModal();
}
