'\" t
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
.TH XkbGetDeviceButtonActions __libmansuffix__ __xorgversion__ "XKB FUNCTIONS"
.SH NAME
XkbGetDeviceButtonActions \- Query the button actions associated with an X Input 
Extension device
.SH SYNOPSIS
.B Status XkbGetDeviceButtonActions
(
.I dpy,
.I device_info,
.I all_buttons,
.I first_button,
.I num_buttons
)
.br
      Display * \fIdpy\fP\^;
.br
      XkbDeviceInfoPtr \fIdevice_info\fP\^;
.br
      Bool \fIall_buttons\fP\^;
.br
      unsigned int \fIfirst_button\fP\^;
.br
      unsigned int \fInum_buttons\fP\^;
.if n .ti +5n
.if t .ti +.5i
.SH ARGUMENTS
.TP
.I \- dpy
connection to X server
.TP
.I \- device_info
structure to update with results
.TP
.I \- all_buttons
True => get information for all buttons
.TP
.I \- first_button
number of first button for which info is desired
.TP
.I \- num_buttons
number of buttons for which info is desired
.SH DESCRIPTION
.LP
.I XkbGetDeviceButtonActions 
queries the server for the desired button information for the device indicated 
by the 
.I device_spec 
field of 
.I device_info 
and waits for a reply. If successful, 
.I XkbGetDeviceButtonActions 
backfills the button actions 
.I (btn_acts 
field of 
.I device_info) 
for only the requested buttons, updates the 
.I name, type, supported, 
and 
.I unsupported 
fields, and returns Success.

.I all_buttons, first_button 
and 
.I num_buttons 
specify the device buttons for which actions should be returned. Setting
.I all_buttons 
to True requests actions for all device buttons; if 
.I all_buttons 
is False, 
.I first_button 
and 
.I num_buttons 
specify a range of buttons for which actions are requested.

If a compatible version of Xkb is not available in the server or the Xkb 
extension has not been properly initialized, 
.I XkbGetDeviceButtonActions 
returns BadAccess. If allocation errors occur, a BadAlloc status is returned. If 
the specified device 
.I (device_info->device_spec) 
is invalid, a BadKeyboard status is returned. If the device has no buttons, a 
BadMatch status is returned. If 
.I first_button 
and 
.I num_buttons 
specify illegal buttons, a BadValue status is returned. 
.SH DIAGNOSTICS
.TP 15
.B BadAccess
The Xkb extension has not been properly initialized
.TP 15
.B BadAlloc
Unable to allocate storage
.TP 15
.B BadKeyboard
The device specified was not a valid core or input extension device
.TP 15
.B BadMatch
A compatible version of Xkb was not available in the server or an argument has 
correct type and range, but is otherwise invalid
.TP 15
.B BadValue
An argument is out of range
