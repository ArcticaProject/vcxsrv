.\" $Xorg: XtGetSP.man,v 1.3 2000/08/17 19:42:00 cpqbld Exp $
.\"
.\" Copyright (c) 1993, 1994  X Consortium
.\" 
.\" Permission is hereby granted, free of charge, to any person obtaining a
.\" copy of this software and associated documentation files (the "Software"), 
.\" to deal in the Software without restriction, including without limitation 
.\" the rights to use, copy, modify, merge, publish, distribute, sublicense, 
.\" and/or sell copies of the Software, and to permit persons to whom the 
.\" Software furnished to do so, subject to the following conditions:
.\" 
.\" The above copyright notice and this permission notice shall be included in
.\" all copies or substantial portions of the Software.
.\" 
.\" THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
.\" IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
.\" FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
.\" THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
.\" WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
.\" OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
.\" SOFTWARE.
.\" 
.\" Except as contained in this notice, the name of the X Consortium shall not 
.\" be used in advertising or otherwise to promote the sale, use or other 
.\" dealing in this Software without prior written authorization from the 
.\" X Consortium.
.\"
.\" $XFree86: xc/doc/man/Xt/XtGetSP.man,v 1.4 2003/04/28 22:18:19 herrb Exp $
.\"
.ds tk X Toolkit
.ds xT X Toolkit Intrinsics \- C Language Interface
.ds xI Intrinsics
.ds xW X Toolkit Athena Widgets \- C Language Interface
.ds xL Xlib \- C Language X Interface
.ds xC Inter-Client Communication Conventions Manual
.ds Rn 3
.ds Vn 2.2
.hw XtSet-Selection-Parameters XtGet-Selection-Parameters wid-get
.na
.de Ds
.nf
.\\$1D \\$2 \\$1
.ft 1
.ps \\n(PS
.\".if \\n(VS>=40 .vs \\n(VSu
.\".if \\n(VS<=39 .vs \\n(VSp
..
.de De
.ce 0
.if \\n(BD .DF
.nr BD 0
.in \\n(OIu
.if \\n(TM .ls 2
.sp \\n(DDu
.fi
..
.de FD
.LP
.KS
.TA .5i 3i
.ta .5i 3i
.nf
..
.de FN
.fi
.KE
.LP
..
.de IN		\" send an index entry to the stderr
..
.de C{
.KS
.nf
.D
.\"
.\"	choose appropriate monospace font
.\"	the imagen conditional, 480,
.\"	may be changed to L if LB is too
.\"	heavy for your eyes...
.\"
.ie "\\*(.T"480" .ft L
.el .ie "\\*(.T"300" .ft L
.el .ie "\\*(.T"202" .ft PO
.el .ie "\\*(.T"aps" .ft CW
.el .ft R
.ps \\n(PS
.ie \\n(VS>40 .vs \\n(VSu
.el .vs \\n(VSp
..
.de C}
.DE
.R
..
.de Pn
.ie t \\$1\fB\^\\$2\^\fR\\$3
.el \\$1\fI\^\\$2\^\fP\\$3
..
.de ZN
.ie t \fB\^\\$1\^\fR\\$2
.el \fI\^\\$1\^\fP\\$2
..
.de NT
.ne 7
.ds NO Note
.if \\n(.$>$1 .if !'\\$2'C' .ds NO \\$2
.if \\n(.$ .if !'\\$1'C' .ds NO \\$1
.ie n .sp
.el .sp 10p
.TB
.ce
\\*(NO
.ie n .sp
.el .sp 5p
.if '\\$1'C' .ce 99
.if '\\$2'C' .ce 99
.in +5n
.ll -5n
.R
..
.		\" Note End -- doug kraft 3/85
.de NE
.ce 0
.in -5n
.ll +5n
.ie n .sp
.el .sp 10p
..
.ny0
.TH XtGetSelectionParameters __libmansuffix__ __xorgversion__ "XT FUNCTIONS"
.SH NAME
XtGetSelectionParameters \- retrieve target parameters for a selection request with a single target
.SH SYNTAX
.HP
void XtGetSelectionParameters(Widget \fIowner\fP, Atom \fIselection\fP,
XtRequestId \fIrequest_id\fP, Atom *\fItype_return\fP, XtPointer
*\fIvalue_return\fP, unsigned long *\fIlength_return\fP, int
*\fIformat_return\fP); 
.SH ARGUMENTS
.IP \fIowner\fP 1i
Specifies the widget that owns the specified selection.
.IP \fIselection\fP 1i
Specifies the selection being processed.
.IP \fIrequest_id\fP 1i
Specifies the requestor id in the case of incremental selections, or NULL
in the case of atomic transfers.
.IP \fItype_return\fP 1i
Specifies a pointer to an atom in which the property type of the
parameters will be stored.
.IP \fIvalue_return\fP 1i
Specifies a pointer into which a pointer to the parameters are to be
stored. A NULL will be stored if no parameters accompany the request.
.IP \fIlength_return\fP 1i
Specifies a pointer into which the number of data elements in 
\fIvalue_return\fP of size indicated by \fIformat_return\fP will be
stored.
.IP \fIformat_return\fP 1i
Specifies a pointer into which the size in bits of the parameter data
in the elements of \fIvalue_return\fP will be stored.
.SH DESCRIPTION
.ZN XtGetSelectionParameters
may only be called from within an
.ZN XtConvertSelectionProc
or from within the last call to an
.ZN XtConvertSelectionIncrProc
with a new request_id.
.SH "SEE ALSO"
XtSetSelectionParameters(3Xt)
.br
\fI\*(xT\fP
.br
\fI\*(xL\fP
