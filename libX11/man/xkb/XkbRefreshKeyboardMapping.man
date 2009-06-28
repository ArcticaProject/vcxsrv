.\" Copyright (c) 1999 - Sun Microsystems, Inc.
.\" All rights reserved.
.\" 
.\" Permission is hereby granted, free of charge, to any person obtaining a
.\" copy of this software and associated documentation files (the
.\" "Software"), to deal in the Software without restriction, including
.\" without limitation the rights to use, copy, modify, merge, publish,
.\" distribute, and/or sell copies of the Software, and to permit persons
.\" to whom the Software is furnished to do so, provided that the above
.\" copyright notice(s) and this permission notice appear in all copies of
.\" the Software and that both the above copyright notice(s) and this
.\" permission notice appear in supporting documentation.
.\" 
.\" THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
.\" OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
.\" MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
.\" OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
.\" HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
.\" INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
.\" FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
.\" NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
.\" WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
.\" 
.\" Except as contained in this notice, the name of a copyright holder
.\" shall not be used in advertising or otherwise to promote the sale, use
.\" or other dealings in this Software without prior written authorization
.\" of the copyright holder.
.\"
.TH XkbRefreshKeyboardMapping __libmansuffix__ __xorgversion__ "XKB FUNCTIONS"
.SH NAME
XkbRefreshKeyboardMapping \- Update the keyboard description that is internal to the X 
library
.SH SYNOPSIS
.B Status XkbRefreshKeyboardMapping
(
.I event
)
.br
      XkbMapNotifyEvent * \fIevent\fP\^;
.if n .ti +5n
.if t .ti +.5i
.SH ARGUMENTS
.TP
.I \- event
event initiating remapping
.SH DESCRIPTION
.LP
.I XkbRefreshKeyboardMapping 
is the Xkb equivalent of the core 
.I XRefreshKeyboardMapping 
function. It requests that the X server send the current key mapping information to 
this client. A client usually invokes 
.I XkbRefreshKeyboardMapping 
after receiving an XkbMapNotify event. 
.I XkbRefreshKeyboardMapping 
returns Success if it succeeds and BadMatch if the event is not an Xkb event.

The XkbMapNotify event can be generated when some client calls 
.I XkbSetMap, XkbChangeMap, XkbGetKeyboardByName, 
or any of the standard X library functions that change the keyboard mapping or modifier 
mapping.
.SH "RETURN VALUES"
.TP 15
Success
The 
.I XkbRefreshKeyboardMapping 
function returns Success when the request that the X server send the current key mapping information to 
this client is successful.
.SH DIAGNOSTICS
.TP 15
.B BadMatch
A compatible version of Xkb was not available in the server or an argument has correct 
type and range, but is otherwise invalid
.SH "SEE ALSO"
.BR XkbChangeMap (__libmansuffix__),
.BR XkbGetKeyboardByName (__libmansuffix__),
.BR XkbSetMap (__libmansuffix__),
.BR XRefreshKeyboardMapping (3X11)
