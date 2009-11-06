/* $XFree86: xc/programs/xload/xload.h,v 1.1 2001/08/27 23:35:14 dawes Exp $ */

#ifndef _XLOAD_H_
#define _XLOAD_H_

#include <X11/Intrinsic.h>

extern void InitLoadPoint(void);
extern void GetLoadPoint(Widget w, XtPointer closure, XtPointer call_data);
extern void GetRLoadPoint(Widget w, XtPointer closure, XtPointer call_data);

#endif
