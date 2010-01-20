.\" To print this out, type tbl macros.t ThisFile | troff -ms
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
\&
.sp 8
.TL
\s+3\fBThe Input Method Protocol\fP\s-3
.sp
\fBVersion 1.0\fP
.sp
\fBX Consortium Standard\fP
.sp
\fBX Version 11, Release 7\fP
.sp
\fB\*(xV\fP
.sp 3
.AU
Masahiko Narita
.AI
FUJITSU Limited.
.AU
Hideki Hiura
.AI
SunSoft, Inc.
.sp 3
.AB
.LP
This specifies a protocol between IM library and IM (Input Method) 
Server for internationalized text input, which is independent from  
any specific language, any specific input method and the transport layer 
used in communication between the IM library and the IM Server, and uses 
a client-server model. 
This protocol allows user to use his/her favorite input method for all 
applications within the stand-alone distributed environment.
.AE
.ce 0
.br
\&
.LP
.ps 11
.nr PS 11
.bp
\&
.ps 9
.nr PS 9
.sp 8
.LP
.DS C
X Window System is a trademark of X Consortium, Inc.
.sp
Copyright \(co 1993, 1994 by X Consortium, Inc.
.DE
.sp 2
.LP
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
.LP
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
.LP
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
.LP
Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
.sp 3
.DS C
Copyright \(co 1993, 1994 by FUJITSU LIMITED
.DE
.sp 2
.LP 
Permission to use, copy, modify, and distribute this documentation 
for any purpose and without fee is hereby granted, provided 
that the above copyright notice and this permission 
notice appear in all copies.
Fujitsu and Sun Microsystems make no representations 
about the suitability for any purpose of the information in this document. 
This documentation is provided as is without express or implied warranty. 
.sp 3
.DS C
Copyright \(co 1993, 1994 by Sun Microsystems, Inc.
.DE
.sp 2
.LP 
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:
.LP
The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.
.LP
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
.ps 11
.nr PS 11
.bp 1
.EH '\fBX Input Method Protocol\fP''\fB\*(xV\fP'
.OH '\fBX Input Method Protocol\fP''\fB\*(xV\fP'
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Introduction
.XS
\*(SN Introduction
.XE
.NH 2
Scope
.XS
\*(SN Scope
.XE
.LP
The internationalization in the
X Window System
Version 11, Release 5 (X11R5) provides a common API which application 
developers can use to create portable internationalized programs and to 
adapt them to the requirements of different native languages, local customs,
and character string encodings (this is called ``localization'').  
As one of its internationalization mechanisms X11R5 has defined a functional 
interface for internationalized text input, called XIM (X Input Method).  
.LP
When a client-server model is used with an IM (Input Method) implementation,
a protocol must be established between the client and the server. 
However, the protocol used to interface Input Method Servers (IM Servers) 
with the Input Method libraries (IM libraries) to which applications are 
linked was not addressed in X11R5. 
This led application developers to depend on vendor-specific input methods, 
decreased the user's choice of available input methods, and made it more 
difficult for developers to create portable applications. This paper describes 
the Input Method Protocol developed for X11R6 to resolve the above problems 
and to address the requirements of existing and future input methods.
.LP
The Input Method Protocol is independent from the transport layer used in 
communication between the IM library and the IM Server. 
Thus, the input method protocol can be built on any inter-process 
communication mechanism, such as TCP/IP or the X protocol.
.LP
In addition, the protocol provides for future extensions such as differing 
input model types.
.LP
.NH 2
Background
.XS
\*(SN Background
.XE
.LP
Text input is much more simple for some languages than
others.  English, for instance, uses an alphabet of a manageable size,
and input consists of pressing the corresponding key on a keyboard,
perhaps in combination with a shift key for capital letters or special
characters.
.LP
Some languages have larger alphabets, or modifiers such as accents,
which require the addition of special key combinations in order to enter
text.  These input methods may require ``dead-keys'' or ``compose-keys''
which, when followed by different combinations of key strokes,
generate different characters.
.LP
Text input for ideographic languages is much less simple.  In these
languages, characters represent actual objects rather than phonetic 
sounds used in pronouncing a word, and the number of characters
in these languages may continue to grow.  In Japanese, for instance, most
text input methods involve entering characters in a phonetic alphabet,
after which the input method searches a dictionary for possible
ideographic equivalents (of which there may be many).  The input method then
presents the candidate characters for the user to choose from.
.LP
In Japanese, either Kana (phonetic symbols) or Roman letters are
typed and then a region is selected for conversion to Kanji. Several
Kanji characters may have the same phonetic representation. If that
is the case with the string entered, a menu of characters is presented
and the user must choose the appropriate one. If no choice is necessary
or a preference has been established, the input method does the
substitution directly.
.LP
These complicated input methods must present state information (Status Area), 
text entry and edit space (Preedit Area), and menu/choice presentations 
(Auxiliary Area).  Much of the protocol between the IM library and the IM
Server involves managing these IM areas.
Because of the size and complexity of these input methods, and because
of how widely they vary from one language or locale to another, they are
usually implemented as separate processes which can serve many client
processes on the same computer or network.
.LP
.NH 2
Input Method Styles
.XS
\*(SN Input Method Styles
.XE
.LP
X11 internationalization support includes the following four types of
input method:
.RS
.IP "- on-the-spot:" 20
The client application is directed by the IM Server to display all
pre-edit data at the site of text insertion.  The client registers
callbacks invoked by the input method during pre-editing.
.IP "- off-the-spot:" 20
The client application provides display windows for the pre-edit data
to the input method which displays into them directly.
.IP "- over-the-spot:" 20
The input method displays pre-edit data in a window which it brings up
directly over the text insertion position.
.IP "- root-window:" 20
The input method displays all pre-edit data in a separate area of the
screen in a window specific to the input method.
.RE
.LP
Client applications must choose from the available input methods
supported by the IM Server and provide the display areas and callbacks
required by the input method.
.LP
.NH 1
Architecture
.XS
\*(SN Architecture
.XE
.NH 2
Implementation Model
.XS
\*(SN Implementation Model
.XE
.LP
Within the X Window System environment, the following two typical
architectural models can be used as an input method's implementation
model.
.RS
.IP "- Client/Server model:" 20
A separate process, the IM Server, processes input and handles preediting, 
converting, and committing.  The IM library within the application, acting 
as client to the IM Server, simply receives the committed string from the 
IM Server.
.IP "- Library model:" 20
All input is handled by the IM library within the application.  The
event process is closed within the IM library and a separate IM Server
process may not be required.
.RE
.LP
Most languages which need complex preediting, such as Asian languages,
are implemented using the Client/Server IM model.  Other languages
which need only dead key or compose key processing, such as European
languages, are implemented using the Library model.
.LP
In this paper, we discuss mainly the Client/Server IM model and the
protocol used in communication between the IM library (client) and the IM 
Server.
.LP
.NH 2
Structure of IM
.XS
\*(SN Structure of IM
.XE
.LP
When the client connects or disconnects to the IM Server, an open or close
operation occurs between the client and the IM Server.
.LP
The IM can be specified at the time of XOpenIM() by setting the locale 
of the client and a locale modifier. Since the IM remembers 
the locale at the time of creation XOpenIM() can be called
multiple times (with the 
setting for the locale and the locale modifier changed) to support 
multiple languages.
.LP
In addition, the supported IM type can be obtained using XGetIMValues().
.LP
The client usually holds multiple input (text) fields. Xlib provides a
value type called the ``Input Context'' (IC) to manage each individual 
input field.  An IC can be created by specifying XIM using XCreateIC(), 
and it can be destroyed using XDestroyIC().
.LP
The IC can specify the type of IM which is supported by XIM for each 
input field, so each input field can handle a different type of IM.
.LP
Most importantly information such as the committed string sent from
the IM Server to the client, is exchanged based on each IC.
.LP
Since each IC corresponds to an input field, the focused input field 
should be announced to the IM Server using XSetICFocus(). (XUnsetICFocus() 
can also be used to change the focus.)
.LP
.NH 2
Event Handling Model
.XS
\*(SN Event Handling Model
.XE
.LP
Existing input methods support either the FrontEnd method, the BackEnd method, 
or both.  This protocol specifically supports the BackEnd method as 
the default method, but also supports the FrontEnd method as an optional 
IM Server extension.
.LP
The difference between the FrontEnd and BackEnd methods is in how
events are delivered to the IM Server.  (Fig. 1)
.LP
.NH 3
BackEnd Method
.XS
\*(SN BackEnd Method
.XE
.LP
In the BackEnd method, client window input events are always delivered
to the IM library, which then passes them to the IM Server.  Events are
handled serially in the order delivered, and therefore there is no
synchronization problem between the IM library and the IM Server.
.LP
Using this method, the IM library forwards all KeyPress and KeyRelease
events to the IM Server (as required by the Event Flow Control model 
described in section 2.4. ``Event Flow Control''), and synchronizes 
with the IM Server (as described in section 4.16.  ``Filtering Events'').
.LP
.NH 3
FrontEnd Method
.XS
\*(SN FrontEnd Method
.XE
.LP
In the FrontEnd method, client window input events are delivered by the
X server directly to both the IM Server and the IM library.  Therefore this
method provides much better interactive performance while preediting
(particularly in cases such as when the IM Server is running locally on
the user's workstation and the client application is running on another
workstation over a relatively slow network).
.LP
However, the FrontEnd model may have synchronization problems between
the key events handled in the IM Server and other events handled in the
client, and these problems could possibly cause the loss or duplication
of key events.  For this reason, the BackEnd method is the core method
supported, and the FrontEnd method is made available as an extension for
performance purposes. (Refer to Appendix A for more information.)
.LP
.LP
.bp
\^... 0.05 6.513 4.737 10.45
\^... 0.000i 3.937i 4.687i 0.000i
.nr 00 \n(.u
.nf
.PS 3.937i 4.687i 
.br
.ps
.ps 10
\h'3.687i'\v'3.437i'\v'-.13m'\L'-0.500i\(br'\v'.13m'
.sp -1
\h'3.712i'\v'3.037i'\D'l-0.025i -0.100i'
.sp -1
\h'3.687i'\v'2.937i'\D'l-0.025i 0.100i'
.sp -1
\h'2.187i'\v'1.938i'\v'-.13m'\L'-0.750i\(br'\v'.13m'
.sp -1
\h'2.187i'\v'1.188i'\l'0.750i'
.sp -1
\h'2.937i'\v'1.188i'\v'-.13m'\L'1.250i\(br'\v'.13m'
.sp -1
\h'2.912i'\v'2.338i'\D'l0.025i 0.100i'
.sp -1
\h'2.937i'\v'2.438i'\D'l0.025i -0.100i'
.sp -1
\h'2.187i'\v'3.437i'\v'-.13m'\L'-1.499i\(br'\v'.13m'
.sp -1
\h'2.212i'\v'2.038i'\D'l-0.025i -0.100i'
.sp -1
\h'2.187i'\v'1.938i'\D'l-0.025i 0.100i'
.sp -1
\h'1.938i'\v'3.437i'\l'1.999i'
.sp -1
\h'3.937i'\v'3.437i'\v'-.13m'\L'0.500i\(br'\v'.13m'
.sp -1
\h'3.937i'\v'3.937i'\l'-1.999i'
.sp -1
\h'1.938i'\v'3.937i'\v'-.13m'\L'-0.500i\(br'\v'.13m'
.sp -1
\h'2.562i'\v'2.438i'\l'2.125i'
.sp -1
\h'4.687i'\v'2.438i'\v'-.13m'\L'0.499i\(br'\v'.13m'
.sp -1
\h'4.687i'\v'2.937i'\l'-2.125i'
.sp -1
\h'2.562i'\v'2.937i'\v'-.13m'\L'-0.499i\(br'\v'.13m'
.sp -1
\h'2.562i'\v'1.438i'\l'1.313i'
.sp -1
\h'3.875i'\v'1.438i'\v'-.13m'\L'0.437i\(br'\v'.13m'
.sp -1
\h'3.875i'\v'1.875i'\l'-1.313i'
.sp -1
\h'2.562i'\v'1.875i'\v'-.13m'\L'-0.437i\(br'\v'.13m'
.sp -1
\h'1.938i'\v'0.438i'\l'1.999i'
.sp -1
\h'3.937i'\v'0.438i'\v'-.13m'\L'1.500i\(br'\v'.13m'
.sp -1
\h'3.937i'\v'1.938i'\l'-1.999i'
.sp -1
\h'1.938i'\v'1.938i'\v'-.13m'\L'-1.500i\(br'\v'.13m'
.sp -1
\D'l0.000i 0.000i'
.sp -1
.ps
.ps 12
\h'3.812i'\v'3.217i'\h'-0.0m'\v'0.2m'FrontEnd Method (Extension)
.sp -1
\h'0.813i'\v'3.217i'\h'-0.0m'\v'0.2m'BackEnd Method (Core)
.sp -1
\h'2.562i'\v'3.779i'\h'-0.0m'\v'0.2m'X Server
.sp -1
\h'3.062i'\v'2.779i'\h'-0.0m'\v'0.2m'IM Server
.sp -1
\h'3.062i'\v'1.717i'\h'-0.0m'\v'0.2m'Library
.sp -1
\h'2.187i'\v'0.904i'\h'-0.0m'\v'0.2m'Application
.sp -1
.ps
.ft
.sp 1+3.937i
.PE
.if \n(00 .fi
.ce
.sp
Fig.1 The Flow of Events
.LP
.NH 2
Event Flow Control
.XS
\*(SN Event Flow Control
.XE
.LP
This protocol supports two event flow models for communication between the 
IM library and the IM Server (Static and Dynamic).  
.LP
Static Event Flow requires that input events always be sent to the IM
Server from the client.
.LP
Dynamic Event Flow, however, requires only that those input events which
need to be processed (converted) be sent to the IM Server from the client.
.LP
For instance, in the case of inputing a combination of ASCII characters
and Chinese characters, ASCII characters do not need to be processed in
the IM Server, so their key events do not have to be sent to the IM
Server.  On the other hand, key events necessary for composing Chinese
characters must be sent to the IM Server.
.LP
Thus, by adopting the Dynamic Event Flow, the number of requests among the
X Server, the client, and the IM Server is significantly reduced, and the
number of context switches is also reduced, resulting in improved performance.
The IM Server can send 
.PN XIM_REGISTER_TRIGGERKEYS 
message in order to switch the event flow in the Dynamic Event Flow.
.LP
The protocol for this process is described in section 4.5. ``Event Flow
Control''.
.LP
.NH 1
Default Preconnection Convention
.XS
\*(SN Default Preconnection Convention 
.XE
.LP
IM Servers are strongly encouraged to register their symbolic
names as the ATOM names into the IM Server directory property, 
.PN XIM_SERVERS,
on the root window of the screen_number 0.
This property can contain a list of ATOMs, and the each ATOM represents
each possible IM Server.
IM Server names are restricted to POSIX Portable Filename Character Set.
To discover if the IM Server is active, see if there is an owner for
the selection with that atom name.  To learn the address of that IM Server, 
convert the selection target
.PN TRANSPORT,
which will return a string form of the transport address(es).
To learn the supported locales of that IM Server, convert the selection target 
.PN LOCALES,
which will return a set of names of the supported locales in the syntax 
X/Open defines.
.LP
The basic semantics to determine the IM Server if there are
multiple ATOMs are found in 
.PN XIM_SERVERS
property, is first fit if the IM Server name is not given as
a X modifier's category
.PN im.
.LP
The address information retrievable from the 
.PN TRANSPORT
target is a transport-specific name. 
The preregistered formats for transport-specific names are listed in Appendix B.
Additional transport-specific names may be registered with X Consortium.
.LP
For environments that lack X connections, or for IM Servers which
do not use the X Window System, the preconnection convention with IM Server 
may be given outside the X Window system (e.g. using a Name Service).
.LP
.NH 1
Protocol
.XS
\*(SN Protocol
.XE
.LP
The protocol described below uses the bi-directional
synchronous/asynchronous request/reply/error model and is specified
using the same conventions outlined in Section 2 of the core X Window
System protocol [1]:
.LP
.NH 2
Basic Requests Packet Format
.XS
\*(SN Basic Requests Packet Format
.XE
.LP
This section describes the requests that may be exchanged between the client 
and the IM Server.
.LP
The basic request packet header format is as follows.
.RS
.DS
	major-opcode:			CARD8
	minor-opcode:			CARD8
	length:				CARD16
.DE
.RE
The MAJOR-OPCODE specifies which core request or extension package this 
packet represents.  If the MAJOR-OPCODE corresponds to a core request, 
the MINOR-OPCODE contains 8 bits of request-specific data.  
(If the MINOR-OPCODE is not used, it is 0.)
Otherwise, the MAJOR-OPCODE and the MINOR-OPCODE are specified by
.PN XIM_QUERY_EXTENSION
message.  (Refer to 4.7. Query the supported extension protocol list.)
The LENGTH field specifies the number of 4 bytes elements following the 
header.  If no additional data is followed by the header, the LENGTH field 
will be 0.
.LP
.NH 2
Data Types
.XS
\*(SN Data Types
.XE
.LP
The following data types are used in the core X IM Server protocol:
.LP
.nf
.ta .2i .5i 2.0i
BITMASK16
	CARD16
.sp
BITMASK32
	CARD32
.sp
PADDING FORMAT
	Where N is some expression, and Pad(N) is the number of bytes needed to round N up to a 
	multiple of four.
		Pad(N) = (4 - (N mod 4)) mod 4
.sp
LPCE
	1		A character from the4 X Portable Character Set in Latin Portable 
			Character Encoding
.bp
STRING
	2	n	length of string in bytes
	n	LISTofLPCE	string
	p		unused, p=Pad(2+n)
.sp
STR
	1	n	length of name in bytes
	n	STRING8	name
.sp
XIMATTR
	2	CARD16	attribute ID (*1)
	2	CARD16	type of the value (*2)
	2	n	length of im-attribute
	n	STRING8	im-attribute
	p		unused, p = Pad(2+n)
.sp
The im-attribute argument specifies XIM values such as XNQueryInputStyle.
.sp
XICATTR
	2	CARD16	attribute ID (*1)
	2	CARD16	type of the value (*2) 
	2	n	length of ic-attribute
	n	STRING8	ic-attribute
	p		unused, p = Pad(2+n) 
.LP
.IP (*1)
XIMATTR and XICATTR are used during the setup stage and XIMATTRIBUTE and 
XICATTRIBUTE are used after each attribute ID has been recognized by
the IM Server and the IM library.
.sp
.IP (*2)
The value types are defined as follows:
.TS H
tab(:);
l l l s s
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l l l
l l l s s
l l l s s
l l l s s
l l l s s
l l l s s
l l l l l.
_
.sp 6p
.B
values:data:format
.sp 6p
_
.sp 6p
.TH
.R
#0:Separator of NestedList:----- (*3)
#1:byte data:CARD8
#2:word data:CARD16
#3:long data:CARD32
#4:char data:STRING8
#5:Window:CARD32
#10:XIMStyles:2:n:number of XIMStyle list
::2::unused
::n:CARD32:XIMStyle list
#11:XRectangle:2:INT16:X
::2:INT16:Y
::2:CARD16:width
::2:CARD16:height
#12:XPoint:2:INT16:X
::2:INT16:Y
#13:XFontSet:2:n:length of Base font name
::n:STRING8:Base font name list
::p::unused, p = Pad(2+n)
#15:XIMHotKeyTriggers:4:n:T{
number of XIMTRIGGERKEY list (*4)
T}
::n:XIMTRIGGERKEY:XIMHotkeyTrigger list
#16:XIMHotKeyState::XIMHOTKEYSTATE:T{
HotKey processing state
T}
#17:XIMStringConversion:XIMSTRCONVTEXT
#18:XIMPreeditState:XIMPREEDITSTATE
#19:XIMResetState:XIMRESETSTATE
#x7fff:NestedList:-----	
.sp 6p
_
.TE
.LP
.IP (*3)
The IC value for the separator of NestedList is defined as follows,
.br
	#define   XNSeparatorofNestedList   ``separatorofNestedList''
.br
, which is registered in X Consortium and cannot be used for any 
other purpose.
.sp
.IP (*4)
LISTofFOO
.RS
A Type name of the form LISTof FOO means a counted list of elements of
type FOO.
The size of the length field may vary (it is not necessarily the same
size as a FOO), and in some cases, it may be implicit.
.RE
.sp
.LP
.nf
.ta .2i .5i 2.0i
XIMTRIGGERKEY
	4	CARD32	keysym
	4	CARD32	modifier
	4	CARD32	modifier mask
.sp
ENCODINGINFO
	2	n	length of encoding info
	n	STRING8	encoding info
	p		unused, p=Pad(2+n)
.sp
EXT
	1	CARD8	extension major-opcode
	1	CARD8	extension minor-opcode
	2	n	length of extension name
	n	STRING8	extension name
	p		unused, p = Pad(n)
.sp
XIMATTRIBUTE
	2	CARD16	attribute ID
	2	n	value length
	n		value
	p		unused, p = Pad(n)
.sp
XICATTRIBUTE
	2	CARD16	attribute ID
	2	n	value length
	n		value
	p		unused, p = Pad(n)
.sp
.bp
.ta .2i .5i 3.0i
XIMSTRCONVTEXT
	2	CARD16	XIMStringConversionFeedback
		#x0000001	XIMStringConversionLeftEdge
		#x0000002	XIMStringConversionRightEdge
		#x0000004	XIMStringConversionTopEdge
		#x0000008	XIMStringConversionBottomEdge
		#x0000010	XIMStringConversionConvealed
		#x0000020	XIMStringConversionWrapped
	2	n	byte length of the retrieved string
	n	STRING8	retrieved string
	p		unused, p = Pad(n)
	2	m	byte length of feedback array
	2		unused
	m	LISTofXIMSTRCONVFEEDBACK	feedback array(*1)
.IP (*1)
This field is reserved for future use.
.sp
.LP
.nf
.ta .2i .5i 2.0i
XIMFEEDBACK
	4	CARD32	XIMFeedback
		#x000001	XIMReverse
		#x000002	XIMUnderline
		#x000004	XIMHighlight
		#x000008	XIMPrimary
		#x000010	XIMSecondary
		#x000020	XIMTertiary
		#x000040	XIMVisibleToForward
		#x000080	XIMVisibleToBackward
		#x000100	XIMVisibleCenter
.sp
XIMHOTKEYSTATE
	4	CARD32	XIMHotKeyState
		#x0000001	XIMHotKeyStateON
		#x0000002	XIMHotKeyStateOFF
.sp
XIMPREEDITSTATE
	4	CARD32	XIMPreeditState
		#x0000001	XIMPreeditEnable
		#x0000002	XIMPreeditDisable
.sp
XIMRESETSTATE
	4	CARD32	XIMResetState
		#x0000001	XIMInitialState
		#x0000002	XIMPreserveState
.LP
.NH 2
Error Notification
.XS
\*(SN Error Notification
.XE
.LP
Both the IM Server and the IM library return 
.PN XIM_ERROR
messages instead of the corresponding reply messages if any errors occur 
during data processing.
.LP
At most one error is generated per request. If more than one error condition
is encountered in processing a request, the choice of which error is returned
is implementation-dependent.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_ERROR (IM Server \(<-\(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:BITMASK16:flag (*1)
::#0000:Both Input-Method-ID and Input-Context-ID are invalid
::#0001:Input-Method-ID is valid
::#0002:Input-Context-ID is valid
:2:CARD16:Error Code
::#1:BadAlloc
::#2:BadStyle
::#3:BadClientWindow
::#4:BadFocusWindow
::#5:BadArea
::#6:BadSpotLocation
::#7:BadColormap
::#8:BadAtom
::#9:BadPixel
::#10:BadPixmap
::#11:BadName
::#12:BadCursor
::#13:BadProtocol
::#14:BadForeground
::#15:BadBackground
::#16:LocaleNotSupported
::#999:BadSomething (*2)
:2:n:byte length of error detail.
:2:CARD16:type of error detail (*3)
:n:STRING8:error detail (*4)
:p::unused, p = Pad(n)
.TE
.LP
.IP (*1)
Before an IM is created, both Input-Method-ID and 
Input-Context-ID are invalid.
Before an IC is created, only Input-Method-ID is valid. 
After that, both of Input-Method-ID and Input-Context-ID are valid.
.IP (*2) 
Unspecific error, for example ``language engine died''
.IP (*3)
This field is reserved for future use.
.IP (*4)
Vendor defined detail error message
.RE
.LP
.NH 2
Connection Establishment
.XS
\*(SN Connection Establishment
.XE
.LP
.PN XIM_CONNECT
message requests to establish a connection over a mutually-understood virtual 
stream.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CONNECT (IM library \(-> IM Server)
.sp 6p
:1::byte order
::#x42 MSB first
::#x6c LSB first
:1::unused
:2:CARD16:client-major-protocol-version (*1)
:2:CARD16:client-minor-protocol-version (*1)
:2:CARD16:number of client-auth-protocol-names
:n:LISTofSTRING:client-auth-protocol-names
.TE
.LP
.IP (*1)
Specify the version of IM Protocol that the client supports.
.RE
.sp
.LP
A client must send 
.PN XIM_CONNECT
message as the first message on the connection. 
The list specifies the names of authentication protocols the sending 
IM Server is willing to perform.
(If the client need not authenticate, the list may be omited.)
.LP
.PN XIM_AUTH_REQUIRED 
message is used to send the authentication protocol name and protocol-specific 
data.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_AUTH_REQUIRED (IM library \(<-\(-> IM Server)
.sp 6p
:1:CARD8:auth-protocol-index
:3::unused
:2:n:length of authentication data
:2::unused
:n:<varies>:data 
:p::unused, p = Pad(n)
.TE
.RE
.LP
The auth-protocol is specified by an index into the list of names 
given in the 
.PN XIM_CONNECT
or 
.PN XIM_AUTH_SETUP
message. Any protocol-specific data that might be required is also sent.
.LP
The IM library sends
.PN XIM_AUTH_REPLY
message as the reply to 
.PN XIM_AUTH_REQUIRED
message, if the IM Server is authenticated. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_AUTH_REPLY (IM library \(-> IM Server)
.sp 6p
:2:n:length of authentication data
:2::unused
:2:n:length of authentication data
:2::unused
:n:<varies>:data
:p::unused, p = Pad(n)
.TE
.RE
.LP
The auth data is specific to the authentication protocol in use.
.LP
.PN XIM_AUTH_NEXT 
message requests to send more auth data.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_AUTH_NEXT (IM library \(<-\(-> IM Server)
.sp 6p
:2:n:length of authentication data
:2::unused
:n:<varies>:data
:p::unused, p = Pad(n)
.TE
.RE
.LP	
The auth data is specific to the authentication protocol in use.
.LP
The IM Server sends
.PN XIM_AUTH_SETUP
message to authenticate the client. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_AUTH_SETUP (IM Server \(-> IM library)
.sp 6p
:2:CARD16:number of client-auth-protocol-names
:2::unused
:n:LISTofSTRING:server-auth-protocol-names
.TE
.RE
.LP
The list specifies the names of authentication protocols the
client is willing to perform.
.LP
.PN XIM_AUTH_NG
message requests to give up the connection.  
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_AUTH_NG (IM library \(<-\(-> IM Server)
.TE
.RE
.LP
The IM Server sends
.PN XIM_CONNECT_REPLY
message as the reply to
.PN XIM_CONNECT
or
.PN XIM_AUTH_REQUIRED
message.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CONNECT_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:server-major-protocol-version (*1)
:2:CARD16:server-minor-protocol-version (*1)
.TE
.LP
.IP (*1)
Specify the version of IM Protocol that the IM Server supports.
This document specifies major version one, minor version zero.
.RE
.sp
.LP	
Here are the state diagrams for the client and the IM Server.
.sp
.B
State transitions for the client
.R
.RS
.LP
\fIinit_status\fP:
.RS
Use authorization function \(-> \fIclient_ask\fP
.br
Not use authorization function \(-> \fIclient_no_check\fP
.RE
.sp
.LP
\fIstart\fP:
.RS
Send 
.PN XIM_CONNECT
.RS
If \fIclient_ask\fP \(-> \fIclient_wait1\fP
.br
If \fIclient_no_check\fP, client-auth-protocol-names may be omited \(-> \fIclient_wait2\fP
.RE
.RE
.sp
.LP
\fIclient_wait1\fP:
.RS
Receive 
.PN XIM_AUTH_REQUIRED
\(-> \fIclient_check\fP
.br
Receive <other> \(-> \fIclient_NG\fP
.RE
.sp
.LP
\fIclient_check\fP:
.RS
If no more auth needed, send 
.PN XIM_AUTH_REPLY
\(-> \fIclient_wait2\fP
.br
If good auth data, send 
.PN XIM_AUTH_NEXT
\(-> \fIclient_wait1\fP
.br
If bad auth data, send 
.PN XIM_AUTH_NG
\(-> give up on this protocol
.RE
.sp
.LP
\fIclient_wait2\fP:
.RS
Receive 
.PN XIM_CONNECT_REPLY
\(-> connect
.br
Receive 
.PN XIM_AUTH_SETUP 
\(-> \fIclient_more\fP
.br
Receive 
.PN XIM_AUTH_NEXT
\(-> \fIclient_more\fP
.br
Receive 
.PN XIM_AUTH_NG
\(-> give up on this protocol
.br
Receive <other> \(-> \fIclient_NG\fP
.RE
.sp
.LP
\fIclient_more\fP:
.RS
Send 
.PN XIM_AUTH_REQUIRED
\(-> \fIclient_wait2\fP
.RE
.sp
.LP
\fIclient_NG\fP:
.RS
Send 
.PN XIM_AUTH_NG
\(-> give up on this protocol
.RE
.RE
.sp
.LP
.B
State transitions for the IM Server
.R
.RS
.LP
\fIinit-status\fP:
.RS
Use authorization function \(-> \fIserver_ask\fP
.br
Not use authorization function \(-> \fIserver_no_check\fP
.RE
.sp
.LP
\fIstart\fP:
.RS
Receive 
.PN XIM_CONNECT
\(-> \fIstart2\fP
.br
Receive <other> \(-> \fIserver_NG\fP
.RE
.sp
.LP
\fIstart2\fP:
.RS
If \fIclient_ask\fP, send 
.PN XIM_AUTH_REQUIRED
\(-> \fIserver_wait1\fP
.br
If \fIclient_no_check\fP and \fIserver_ask\fP, send 
.PN XIM_AUTH_SETUP
\(-> \fIserver_wait2\fP
.br
If \fIclient_no_check\fP and \fIserver_no_check\fP, send 
.PN XIM_CONNECT_REPLY
\(-> connect
.RE
.sp
.LP
\fIserver_wait1\fP:
.RS
Receive 
.PN XIM_AUTH_REPLY
\(-> \fIserver2\fP
.br
Receive 
.PN XIM_AUTH_NEXT
\(-> \fIserver_more\fP
.br
Receive <other> \(-> \fIserver_NG\fP
.RE
.sp
.LP
\fIserver_more\fP
.RS
Send 
.PN XIM_AUTH_REQUIRED
\(-> \fIserver_wait1\fP
.RE
.sp
.LP
\fIserver2\fP
.RS
If \fIserver_ask\fP, send 
.PN XIM_AUTH_SETUP
\(-> \fIserver_wait2\fP
.br
If \fIserver_no_check\fP, send 
.PN XIM_CONNECT_REPLY 
\(-> connect
.RE
.sp
.LP
\fIserver_wait2\fP
.RS
Receive 
.PN XIM_AUTH_REQUIRED
\(-> \fIserver_check\fP
.br
Receive <other> \(-> \fIserver_NG\fP
.RE
.sp
.LP
\fIserver_check\fP
.RS
If no more auth data, send 
.PN XIM_CONNECT_REPLY
\(-> connect
.br
If bad auth data, send 
.PN XIM_AUTH_NG
\(-> give up on this protocol
.br
If good auth data, send 
.PN XIM_AUTH_NEXT
\(-> \fIserver_wait2\fP
.RE
.sp
.LP
\fIserver_NG\fP
.RS
Send 
.PN XIM_AUTH_NG
\(-> give up on this protocol
.RE
.RE
.sp
.LP
.PN XIM_DISCONNECT 
message requests to shutdown the connection over a mutually-understood 
virtual stream.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_DISCONNECT (IM library \(-> IM Server)
.TE
.RE
.LP
.PN XIM_DISCONNECT
is a synchronous request.  The IM library should wait until it receives 
either an 
.PN XIM_DISCONNECT_REPLY
packet or an 
.PN XIM_ERROR
packet.  
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_DISCONNECT_REPLY (IM Server \(-> IM library)
.TE
.RE
.LP
.PN XIM_OPEN
requests to establish a logical connection between the IM library and the IM 
Server. 
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_OPEN (IM library \(-> IM Server)
.sp 6p
:n:STR:locale name
:p::unused, p = Pad(n)
.TE
.RE
.LP
.PN XIM_OPEN
is a synchronous request.  The IM library should wait until receiving 
either an 
.PN XIM_OPEN_REPLY
packet or an 
.PN XIM_ERROR 
packet. 
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_OPEN_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of IM attributes supported
:n:LISTofXIMATTR:IM attributes supported
:2:m:byte length of IC attributes supported
:2:CARD16:unused
:m:LISTofXICATTR: IC attributes supported
.TE
.RE
.LP
.PN XIM_OPEN_REPLY
message returns all supported IM and IC attributes in LISTofXIMATTR and 
LISTofXICATTR.  These IM and IC attribute IDs are used to reduce the amount
of data which must be transferred via the network. In addition, this
indicates to the IM library what kinds of IM/IC attributes can be used
in this session, and what types of data will be exchanged. This allows 
the IM Server provider and application writer to support IM system 
enhancements with new IM/IC attributes, without modifying Xlib.
The IC value for the separator of NestedList must be included in the
LISTofXICATTR.
.LP
.PN XIM_CLOSE 
message requests to shutdown the logical connection between the IM library
and the IM Server. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CLOSE (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2::unused
.TE
.RE
.LP
.PN XIM_CLOSE
is a synchronous request.  The IM library should wait until receiving 
either an 
.PN XIM_CLOSE_REPLY
packet or an 
.PN XIM_ERROR
packet. 
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CLOSE_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2::unused
.TE
.RE
.LP
.NH 2
Event Flow Control 
.XS
\*(SN Event Flow Control
.XE
.LP
An IM Server must send 
.PN XIM_SET_EVENT_MASK 
message to the IM library in order for events to be forwarded to the IM 
Server, since the IM library initially doesn't forward any events to the 
IM Server. In the protocol, the IM Server will specify masks of X events 
to be forwarded and which need to be synchronized by the IM library.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_EVENT_MASK (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:EVENTMASK:forward-event-mask (*1)
:4:EVENTMASK:synchronous-event-mask (*2)
.TE
.LP
.IP (*1)
Specify all the events to be forwarded to the IM Server by the IM library.
.IP (*2)
Specify the events to be forwarded with synchronous flag on by the IM library.
.RE
.sp
.LP
.PN XIM_SET_EVENT_MASK 
is an asynchronous request.  The event masks are valid immediately after 
they are set until changed by another 
.PN XIM_SET_EVENT_MASK
message.  If input-context-ID is set to zero, the default value of the 
input-method-ID will be changed to the event masks specified in the request. 
That value will be used for the IC's which have no individual values.
.LP
Using the Dynamic Event Flow model, an IM Server sends 
.PN XIM_REGISTER_TRIGGERKEYS 
message to the IM library before sending
.PN XIM_OPEN_REPLY
message.  
Or the IM library may suppose that the IM Server uses the Static Event Flow 
model.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_REGISTER_TRIGGERKEYS (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2::unused
:4:n:byte length of on-keys
:n:LISTofXIMTRIGGERKEY:on-keys list
:4:m:byte length of off-keys
:m:LISTofXIMTRIGGERKEY:off-keys list
.TE
.RE
.LP
.PN XIM_REGISTER_TRIGGERKEYS 
is an asynchronous request.  
The IM Server notifys the IM library of on-keys and off-keys lists with 
this message.
.LP
The IM library notifys the IM Server with 
.PN XIM_TRIGGER_NOTIFY 
message that a key event matching either on-keys or off-keys has been occurred.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_TRIGGER_NOTIFY (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:CARD32:flag
::#0:on-keys list
::#1:off-keys list
:4:CARD32:index of keys list
:4:EVENTMASK:client-select-event-mask (*1)
.TE
.LP
.IP (*1) 
Specify the events currently selected by the IM library with XSelectInput.
.RE
.sp
.LP
.PN XIM_TRIGGER_NOTIFY 
is a synchronous request.  The IM library should wait until receiving 
either an 
.PN XIM_TRIGGER_NOTIFY_REPLY
packet or an 
.PN XIM_ERROR
packet. 
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_TRIGGER_NOTIFY_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.NH 2
Encoding Negotiation
.XS
\*(SN Encoding Negotiation
.XE
.LP
.PN XIM_ENCODING_NEGOTIATION
message requests to decide which encoding to be sent across the wire.
When the negotiation fails, the fallback default encoding is Portable 
Character Encoding.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_ENCODING_NEGOTIATION (IM library \(-> IM Server).sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of encodings listed by name
:n:LISTofSTR:list of encodings supported in the IM library.
:p::unused, p = Pad(n)
:2:m:byte length of encodings listed by detailed data
:2::unused
:m:LISTofENCODINGINFO:list of encordings supported in the IM library
.TE
.RE
.LP
The IM Server must choose one encoding from the list sent by the IM library.
If index of the encording determined is -1 to indicate that the negotiation
is failed, the fallback default encoding is used. 
The message must be issued after sending 
.PN XIM_OPEN
message via XOpenIM().
The name of encoding may be registered with X Consortium.
.LP
.PN XIM_ENCODING_NEGOTIATION
is a synchronous request.  The IM library should wait until receiving 
either an 
.PN XIM_ENCODING_NEGOTIATION_REPLY
packet or an 
.PN XIM_ERROR
packet. 
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_ENCODING_NEGOTIATION_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:category of the encoding determined.
::#0:name
::#1:detailed data
:2:INT16:index of the encoding determinated.
:2::unused
.TE
.RE
.LP
.NH 2
Query the supported extension protocol list
.XS
\*(SN Query the supported extension protocol list
.XE
.LP
.PN XIM_QUERY_EXTENSION
message requests to query the IM extensions supported by the IM Server to 
which the client is being connected. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_QUERY_EXTENSION (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:n:T{
byte length of extensions supported by the IM library
T}
:n:LISTofSTR:extensions supported by the IM library
:p::unused, p = Pad(n)
.TE
.RE
.LP
An example of a supported extension is FrontEnd.
The message must be issued after sending 
.PN XIM_OPEN 
message via XOpenIM().
.LP
If n is 0, the IM library queries the IM Server for all extensions.
.LP
If n is not 0, the IM library queries whether the IM Server supports the 
contents specified in the list.
.LP
If a client uses an extension request without previously having issued a
.PN XIM_QUERY_EXTENSION
message for that extension, the IM Server responds with a
.PN BadProtocol
error.  If the IM Server encounters a request with an unknown MAJOR-OPCODE 
or MINOR-OPCODE, it responds with a
.PN BadProtocol
error.
.LP
.PN XIM_QUERY_EXTENSION
is a synchronous request.  The IM library should wait until receiving 
either an 
.PN XIM_QUERY_EXTENSION_REPLY
packet or an 
.PN XIM_ERROR
packet.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_QUERY_EXTENSION_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:n:T{
byte length of extensions supported by both the IM library and the IM Server
T}
:n:LISTofEXT:T{
list of extensions supported by both the IM library and the IM Server
T}
.TE
.RE
.LP
.PN XIM_QUERY_EXTENSION_REPLY
message returns the list of extensions supported by both the IM library and 
the IM Server. If the list passed in 
.PN XIM_QUERY_EXTENSION
message is NULL, the IM Server returns the full list of extensions supported 
by the IM Server.  If the list is not NULL, the IM Server returns the 
extensions in the list that are supported by the IM Server.
.LP
A zero-length string is not a valid extension name.  The IM library should 
disregard any zero-length strings that are returned in the extension list.  
The IM library does not use the requests which are not supported by the IM 
Server.
.LP
.NH 2
Setting IM Values
.XS
\*(SN Setting IM Values
.XE
.LP
.PN XIM_SET_IM_VALUES 
requests to set attributes to the IM.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_IM_VALUES (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of im-attribute
:n:LISTofXIMATTRIBUTE:im-attributes
.TE
.RE
.LP
The im-attributes in 
.PN XIM_SET_IM_VALUES
message are specified as a LISTofXIMATTRIBUTE, specifying the attributes 
to be set. Attributes other than the ones returned by 
.PN XIM_OPEN_REPLY
message should not be specified.  
.LP
.PN XIM_SET_IM_VALUES 
is a synchronous request. The IM library should wait until receiving 
either an 
.PN XIM_SET_IM_VALUES_REPLY
packet or an 
.PN XIM_ERROR
packet, because it must receive the error attribute if 
.PN XIM_ERROR
message is returned.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_IM_VALUES_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2::unused
.TE
.RE
.LP
.PN XIM_SET_IM_VALUES_REPLY
message returns the input-method-ID to distinguish replies from multiple IMs.
.LP
.NH 2
Getting IM Values
.XS
\*(SN getting IM Values
.XE
.LP
.PN XIM_GET_IM_VALUES 
requests to query IM values supported by the IM Server currently being 
connected.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_GET_IM_VALUES (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of im-attribute-id
:n:LISTofCARD16:im-attribute-id
:p::unused, p=Pad(n)
.TE
.RE
.LP
.PN XIM_GET_IM_VALUES
is a synchronous request.  The IM library should wait until it receives 
either an 
.PN XIM_GET_IM_VALUES_REPLY
packet or an 
.PN XIM_ERROR
packet.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_GET_IM_VALUES_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of im-attributes returned
:n:LISTofXIMATTRIBUTE:im-attributes returned
.TE
.RE
.LP
The IM Server returns IM values with 
.PN XIM_GET_IM_VALUES_REPLY
message.  The order of the returned im-attribute values corresponds directly
to that of the list passed with the 
.PN XIM_GET_IM_VALUES
message.
.LP
.NH 2
Creating an IC
.XS
\*(SN Creating an IC
.XE
.LP
.PN XIM_CREATE_IC
message requests to create an IC.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CREATE_IC (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:n:byte length of ic-attributes
:n:LISTofXICATTRIBUTE:ic-attributes
.TE
.RE
.LP
The input-context-id is specified by the IM Server to identify the client
(IC).  (It is not specified by the client in 
.PN XIM_CREATE_IC
message.), and it should not be set to zero.
.LP
.PN XIM_CREATE_IC
is a synchronous request which returns the input-context-ID. 
The IM library should wait until it receives either an 
.PN XIM_CREATE_IC_REPLY
packet or an 
.PN XIM_ERROR
packet. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_CREATE_IC_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.NH 2
Destroying the IC
.XS
\*(SN Destroying the IC
.XE
.LP
.PN XIM_DESTROY_IC
message requests to destroy the IC.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_DESTROY_IC (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.PN XIM_DESTROY_IC 
is a synchronous request. The IM library should not free its resources 
until it receives an 
.PN XIM_DESTROY_IC_REPLY
message because
.PN XIM_DESTROY_IC
message may result in Callback packets such as 
.PN XIM_PREEDIT_DRAW
and
.PN XIM_PREEDIT_DONE.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_DESTROY_IC_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.NH 2
Setting IC Values
.XS
\*(SN Setting IC Values
.XE
.LP
.PN XIM_SET_IC_VALUES
messages requests to set attributes to the IC.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_IC_VALUES (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:n:byte length of ic-attributes
:2::unused
:n:LISTofXICATTRIBUTE:ic-attributes
.TE
.RE
.LP
The ic-attributes in 
.PN XIM_SET_IC_VALUES
message are specified as a LISTofXICATTRIBUTE, specifying the attributes 
to be set. Attributes other than the ones returned by 
.PN XIM_OPEN_REPLY
message should not be specified. 
.LP
.PN XIM_SET_IC_VALUES 
is a synchronous request. The IM library should wait until receiving 
either an 
.PN XIM_SET_IC_VALUES_REPLY 
packet or an 
.PN XIM_ERROR
packet, because it must receive the error attribute if 
.PN XIM_ERROR
message is returned. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_IC_VALUES_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.NH 2
Getting IC Values
.XS
\*(SN Getting IC Values
.XE
.LP
.PN XIM_GET_IC_VALUES
message requests to query IC values supported by the IM Server currently
being connected.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_GET_IC_VALUES (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:n:byte length of ic-attribute-id
:n:LISTofCARD16:ic-attribute-id
:p::unused, p=Pad(2+n)
.TE
.RE
.LP
In LISTofCARD16, the appearance of the ic-attribute-id for the separator 
of NestedList shows the end of the heading nested list.
.LP
.PN XIM_GET_IC_VALUES
is a synchronous request and returns each attribute with its values to 
show the correspondence.  The IM library should wait until receiving 
either an 
.PN XIM_GET_IC_VALUES_REPLY
packet or an 
.PN XIM_ERROR
packet.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_GET_IC_VALUES_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:n:byte length of ic-attribute 
:2::unused
:n:LISTofXICATTRIBUTE:ic-attribute
.TE
.RE
.LP
.NH 2
Setting IC Focus
.XS
\*(SN Setting IC Focus
.XE
.LP
.PN XIM_SET_IC_FOCUS
message requests to set the focus to the IC.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SET_IC_FOCUS (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.PN XIM_SET_IC_FOCUS
is an asynchronous request.
.LP
.NH 2
Unsetting IC Focus
.XS
\*(SN Unsetting IC Focus
.XE
.LP
.PN XIM_UNSET_IC_FOCUS
message requests to unset the focus to the focused IC.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_UNSET_IC_FOCUS (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.PN XIM_UNSET_IC_FOCUS
is an asynchronous request.
.LP
.NH 2
Filtering Events
.XS
\*(SN Filtering Events
.XE
.LP
Event filtering is mainly provided for BackEnd method to allow input method
to capture X events transparently to clients.
.LP
X Events are forwarded by 
.PN XIM_FORWARD_EVENT
message.
This message can be operated both synchronously and asynchronously. 
If the requester sets the synchronous flag, the receiver must send 
.PN XIM_SYNC_REPLY
message back to the requester when all the data processing is done.
.sp
.B
Protocol flow of BackEnd model
.R
.LP
.LP
With BackEnd method, the protocol flow can be classified into two
methods in terms of synchronization, depending on the synchronous-eventmask
of 
.PN XIM_SET_EVENT_MASK
message.  One can be called on-demand-synchronous method and another
can be called as full-synchronous method.
.LP
In on-demand-synchronous method, the IM library always receives
.PN XIM_FORWARD_EVENT
or
.PN XIM_COMMIT
message as a synchronous request. Also, the IM Server needs to synchronously 
process the correspondent reply from the IM library and the following 
.PN XIM_FORWARD_EVENT
message sent from the IM library when any of the event causes the IM Server 
to send 
.PN XIM_FORWARD_EVENT
or
.PN XIM_COMMIT
message to the IM library, so that the input service is consistent.  If the 
IM library gets the control back from the application after receiving the 
synchronous request, the IM library replies for the synchronous request before 
processing any of the events. In this time, the IM Server blocks 
.PN XIM_FORWARD_EVENT
message which is sent by the IM library, and handles it after receiving the 
reply. However, the IM Server handles the other protocols at any time.
.LP
In full-synchronous method, the IM library always sends 
.PN XIM_FORWARD_EVENT
message to the IM Server as a synchronous request. Therefore, the reply to it 
from the IM Server will be put between the 
.PN XIM_FORWARD_EVENT
message and its 
.PN XIM_SYNC_REPLY
message.
In case of sending 
.PN XIM_FORWARD_EVENT
or
.PN XIM_COMMIT
message, the IM Server should set the synchronous flag off. Because the 
synchronization can be done by the following 
.PN XIM_SYNC_REPLY
message.
.sp
.LP
.B
Sample Protocol flow chart 1
.R
.LP
Following chart shows one of the simplest protocol flow which only
deals with keyevents for preediting operation.
.LP
.\"====================== event flow figure start =====================
\^... 0.425 6.888 6.3 10.296
\^... 0.000i 3.408i 5.875i 0.000i
.nr 00 \n(.u
.nf
.PS 3.408i 5.875i 
.br
.ps 11
\h'3.125i'\v'0.496i'\D'l1.625i 0.250i'
.sp -1
\h'4.647i'\v'0.756i'\D'l0.103i -0.010i'
.sp -1
\h'4.655i'\v'0.706i'\D'l0.095i 0.040i'
.sp -1
\h'3.125i'\v'1.221i'\D'l1.687i 0.188i'
.sp -1
\h'4.710i'\v'1.423i'\D'l0.102i -0.014i'
.sp -1
\h'4.715i'\v'1.373i'\D'l0.097i 0.036i'
.sp -1
\h'4.750i'\v'0.971i'\D'l-1.625i 0.438i'
.sp -1
\h'3.215i'\v'1.359i'\D'l-0.090i 0.050i'
.sp -1
\h'3.228i'\v'1.407i'\D'l-0.103i 0.002i'
.sp -1
\h'2.000i'\v'0.409i'\D'l1.000i 0.062i'
.sp -1
\h'2.899i'\v'0.490i'\D'l0.101i -0.019i'
.sp -1
\h'2.902i'\v'0.440i'\D'l0.098i 0.031i'
.sp -1
\h'2.000i'\v'1.034i'\D'l1.000i 0.125i'
.sp -1
\h'2.898i'\v'1.171i'\D'l0.102i -0.012i'
.sp -1
\h'2.904i'\v'1.122i'\D'l0.096i 0.037i'
.sp -1
\h'3.000i'\v'1.409i'\D'l-1.000i 0.062i'
.sp -1
\h'2.098i'\v'1.440i'\D'l-0.098i 0.031i'
.sp -1
\h'2.101i'\v'1.490i'\D'l-0.101i -0.019i'
.sp -1
\h'1.125i'\v'1.846i'\l'-0.500i'
.sp -1
\h'0.725i'\v'1.821i'\D'l-0.100i 0.025i'
.sp -1
\h'0.725i'\v'1.871i'\D'l-0.100i -0.025i'
.sp -1
\h'0.688i'\v'0.159i'\l'0.437i'
.sp -1
\h'1.025i'\v'0.184i'\D'l0.100i -0.025i'
.sp -1
\h'1.025i'\v'0.134i'\D'l0.100i 0.025i'
.sp -1
\h'0.688i'\v'0.846i'\l'0.437i'
.sp -1
\h'1.025i'\v'0.871i'\D'l0.100i -0.025i'
.sp -1
\h'1.025i'\v'0.821i'\D'l0.100i 0.025i'
.sp -1
\h'5.562i'\v'1.409i'\l'0.313i'
.sp -1
\h'5.875i'\v'1.409i'\v'-.13m'\L'1.937i\(br'\v'.13m'
.sp -1
\h'5.875i'\v'3.346i'\D'l-0.250i 0.000i'
.sp -1
\h'5.725i'\v'3.321i'\D'l-0.100i 0.025i'
.sp -1
\h'5.725i'\v'3.371i'\D'l-0.100i -0.025i'
.sp -1
\h'2.062i'\v'2.096i'\l'0.875i'
.sp -1
\h'2.837i'\v'2.121i'\D'l0.100i -0.025i'
.sp -1
\h'2.837i'\v'2.071i'\D'l0.100i 0.025i'
.sp -1
\h'3.000i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'4.875i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'2.013i'\v'2.871i'\D'l0.937i 0.250i'
.sp -1
\h'2.847i'\v'3.119i'\D'l0.103i 0.002i'
.sp -1
\h'2.860i'\v'3.071i'\D'l0.090i 0.050i'
.sp -1
\h'3.062i'\v'3.134i'\D'l1.688i 0.187i'
.sp -1
\h'4.648i'\v'3.335i'\D'l0.102i -0.014i'
.sp -1
\h'4.653i'\v'3.285i'\D'l0.097i 0.036i'
.sp -1
\h'3.062i'\v'2.533i'\D'l1.750i 0.213i'
.sp -1
\h'4.710i'\v'2.759i'\D'l0.102i -0.013i'
.sp -1
\h'4.716i'\v'2.709i'\D'l0.096i 0.037i'
.sp -1
\h'3.062i'\v'2.096i'\l'1.750i'
.sp -1
\h'4.712i'\v'2.121i'\D'l0.100i -0.025i'
.sp -1
\h'4.712i'\v'2.071i'\D'l0.100i 0.025i'
.sp -1
\h'4.812i'\v'2.284i'\l'-1.750i'
.sp -1
\h'3.162i'\v'2.259i'\D'l-0.100i 0.025i'
.sp -1
\h'3.162i'\v'2.309i'\D'l-0.100i -0.025i'
.sp -1
\h'1.250i'\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'0.381i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent\fP
.sp -1
\h'1.250i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'1.068i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent\fP
.sp -1
\h'1.250i'\v'1.506i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'1.881i'\h'-0.0m'\v'0.2m'\s10\fRXmbLookupString\fP
.sp -1
\h'4.875i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'2.437i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\h'1.250i'\v'1.693i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent (returns False)   \fP
.sp -1
\v'2.168i'\h'-0.0m'\v'0.2m'\s10\fRthe focus\fP
.sp -1
\h'1.250i'\h'-0.0m'\v'0.2m'\s12\fRXlib API\fP
.sp -1
\v'2.006i'\h'-0.0m'\v'0.2m'\s10\fRApplication moves\fP
.sp -1
\h'3.187i'\v'0.443i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'3.187i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'3.187i'\v'1.631i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'3.187i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRor XIM_COMMIT\fP
.sp -1
\h'5.000i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRsynchronous \fP
.sp -1
\h'5.000i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRrequest\fP
.sp -1
\h'0.062i'\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRKey event\fP
.sp -1
\h'0.062i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRKey event\fP
.sp -1
\h'3.187i'\v'1.131i'\h'-0.0m'\v'0.2m'\s10\fR(synchronous)   \fP
.sp -1
\h'5.000i'\v'1.443i'\h'-0.0m'\v'0.2m'\s10\fRPending\fP
.sp -1
\h'5.000i'\v'2.381i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'5.000i'\v'2.506i'\h'-0.0m'\v'0.2m'\s10\fR(The focused\fP
.sp -1
\h'5.000i'\v'2.631i'\h'-0.0m'\v'0.2m'\s10\fRIC is changed)  \fP
.sp -1
\h'5.000i'\v'2.881i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'1.250i'\v'2.131i'\h'-0.0m'\v'0.2m'\s10\fRXSetICFocus\fP
.sp -1
\h'3.125i'\v'2.881i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC_REPLY as a reply\fP
.sp -1
\h'3.125i'\v'3.043i'\h'-0.0m'\v'0.2m'\s10\fRof the XIM_FORWARD_EVENT\fP
.sp -1
\h'1.250i'\v'2.881i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'3.312i'\v'2.506i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SET_IC_FOCUS\fP
.sp -1
\h'3.312i'\v'2.006i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC\fP
.sp -1
\h'3.312i'\v'2.193i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC_REPLY\fP
.sp -1
\h'5.000i'\v'3.381i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
.sp 1+3.408i
.PE
.if \n(00 .fi

.\"====================== event flow figure end =======================
.ce
.sp
Fig.2 Sample Protocol Flow
.sp
.LP
.B
Sample Protocol flow chart 2
.R
.LP
Following chart shows one of the complex protocol flow, which deals
with multiple focus windows and button press event as well as keyevent,
and the focus is moved by the application triggered by both of keyevent
and button press event.
.LP
.bp
.\"====================== event2 flow figure start =====================
\^... 0.425 5.575 6.3 10.296
\^... 0.000i 4.721i 5.875i 0.000i
.nr 00 \n(.u
.nf
.PS 4.721i 5.875i 
.br
.ps 11
\h'3.125i'\v'0.496i'\D'l1.625i 0.163i'
.sp -1
\h'4.648i'\v'0.674i'\D'l0.102i -0.015i'
.sp -1
\h'4.653i'\v'0.624i'\D'l0.097i 0.035i'
.sp -1
\h'2.000i'\v'0.409i'\D'l1.000i 0.062i'
.sp -1
\h'2.899i'\v'0.490i'\D'l0.101i -0.019i'
.sp -1
\h'2.902i'\v'0.440i'\D'l0.098i 0.031i'
.sp -1
\h'0.688i'\v'0.159i'\l'0.437i'
.sp -1
\h'1.025i'\v'0.184i'\D'l0.100i -0.025i'
.sp -1
\h'1.025i'\v'0.134i'\D'l0.100i 0.025i'
.sp -1
\h'1.250i'\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'0.381i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent\fP
.sp -1
\h'3.187i'\v'0.443i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'0.062i'\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRKey event\fP
.sp -1
\h'3.125i'\v'1.221i'\D'l1.687i 0.125i'
.sp -1
\h'4.710i'\v'1.364i'\D'l0.102i -0.018i'
.sp -1
\h'4.714i'\v'1.314i'\D'l0.098i 0.032i'
.sp -1
\h'4.750i'\v'0.971i'\D'l-1.625i 0.750i'
.sp -1
\h'3.205i'\v'1.656i'\D'l-0.080i 0.065i'
.sp -1
\h'3.226i'\v'1.702i'\D'l-0.101i 0.019i'
.sp -1
\h'2.000i'\v'1.034i'\D'l1.000i 0.125i'
.sp -1
\h'2.898i'\v'1.171i'\D'l0.102i -0.012i'
.sp -1
\h'2.904i'\v'1.122i'\D'l0.096i 0.037i'
.sp -1
\h'0.688i'\v'0.846i'\l'0.437i'
.sp -1
\h'1.025i'\v'0.871i'\D'l0.100i -0.025i'
.sp -1
\h'1.025i'\v'0.821i'\D'l0.100i 0.025i'
.sp -1
\h'3.000i'\v'0.034i'\v'-.13m'\L'4.687i\(br'\v'.13m'
.sp -1
\h'0.750i'\v'1.346i'\l'0.313i'
.sp -1
\h'0.963i'\v'1.371i'\D'l0.100i -0.025i'
.sp -1
\h'0.963i'\v'1.321i'\D'l0.100i 0.025i'
.sp -1
\h'3.125i'\v'1.509i'\D'l1.687i 0.125i'
.sp -1
\h'4.710i'\v'1.652i'\D'l0.102i -0.018i'
.sp -1
\h'4.714i'\v'1.602i'\D'l0.098i 0.032i'
.sp -1
\h'4.812i'\v'1.721i'\D'l-1.687i 0.188i'
.sp -1
\h'3.222i'\v'1.873i'\D'l-0.097i 0.036i'
.sp -1
\h'3.227i'\v'1.923i'\D'l-0.102i -0.014i'
.sp -1
\h'2.937i'\v'1.971i'\D'l-0.937i 0.188i'
.sp -1
\h'2.093i'\v'2.115i'\D'l-0.093i 0.044i'
.sp -1
\h'2.103i'\v'2.164i'\D'l-0.103i -0.005i'
.sp -1
\h'1.125i'\v'2.533i'\l'-0.500i'
.sp -1
\h'0.725i'\v'2.508i'\D'l-0.100i 0.025i'
.sp -1
\h'0.725i'\v'2.558i'\D'l-0.100i -0.025i'
.sp -1
\h'5.562i'\v'1.346i'\l'0.313i'
.sp -1
\h'5.875i'\v'1.346i'\v'-.13m'\L'2.687i\(br'\v'.13m'
.sp -1
\h'5.875i'\v'4.033i'\D'l-0.250i 0.000i'
.sp -1
\h'5.725i'\v'4.008i'\D'l-0.100i 0.025i'
.sp -1
\h'5.725i'\v'4.058i'\D'l-0.100i -0.025i'
.sp -1
\h'2.013i'\v'3.559i'\D'l0.937i 0.250i'
.sp -1
\h'2.847i'\v'3.807i'\D'l0.103i 0.002i'
.sp -1
\h'2.860i'\v'3.759i'\D'l0.090i 0.050i'
.sp -1
\h'3.062i'\v'3.821i'\D'l1.688i 0.188i'
.sp -1
\h'4.648i'\v'4.023i'\D'l0.102i -0.014i'
.sp -1
\h'4.653i'\v'3.973i'\D'l0.097i 0.036i'
.sp -1
\h'2.000i'\v'1.358i'\D'l1.000i 0.126i'
.sp -1
\h'2.898i'\v'1.496i'\D'l0.102i -0.012i'
.sp -1
\h'2.904i'\v'1.447i'\D'l0.096i 0.037i'
.sp -1
\h'3.062i'\v'2.159i'\D'l-0.250i 0.000i'
.sp -1
\h'2.812i'\v'2.159i'\v'-.13m'\L'1.812i\(br'\v'.13m'
.sp -1
\h'2.812i'\v'3.971i'\D'l0.125i 0.125i'
.sp -1
\h'2.849i'\v'4.043i'\D'l0.088i 0.053i'
.sp -1
\h'2.884i'\v'4.008i'\D'l0.053i 0.088i'
.sp -1
\h'2.062i'\v'2.783i'\l'0.875i'
.sp -1
\h'2.837i'\v'2.808i'\D'l0.100i -0.025i'
.sp -1
\h'2.837i'\v'2.758i'\D'l0.100i 0.025i'
.sp -1
\h'2.062i'\v'3.783i'\D'l0.813i 0.438i'
.sp -1
\h'2.775i'\v'4.196i'\D'l0.100i 0.025i'
.sp -1
\h'2.799i'\v'4.152i'\D'l0.076i 0.069i'
.sp -1
\h'0.625i'\v'3.533i'\l'0.438i'
.sp -1
\h'0.963i'\v'3.558i'\D'l0.100i -0.025i'
.sp -1
\h'0.963i'\v'3.508i'\D'l0.100i 0.025i'
.sp -1
\h'3.062i'\v'4.346i'\D'l1.625i 0.163i'
.sp -1
\h'4.585i'\v'4.524i'\D'l0.102i -0.015i'
.sp -1
\h'4.590i'\v'4.474i'\D'l0.097i 0.035i'
.sp -1
\h'4.875i'\v'0.034i'\v'-.13m'\L'4.687i\(br'\v'.13m'
.sp -1
\h'3.062i'\v'4.146i'\D'l1.688i 0.187i'
.sp -1
\h'4.648i'\v'4.347i'\D'l0.102i -0.014i'
.sp -1
\h'4.653i'\v'4.297i'\D'l0.097i 0.036i'
.sp -1
\h'3.062i'\v'2.871i'\D'l1.750i 0.212i'
.sp -1
\h'4.710i'\v'3.096i'\D'l0.102i -0.013i'
.sp -1
\h'4.716i'\v'3.046i'\D'l0.096i 0.037i'
.sp -1
\h'1.250i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'1.068i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent\fP
.sp -1
\h'4.875i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'2.437i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\h'1.250i'\h'-0.0m'\v'0.2m'\s12\fRXlib API\fP
.sp -1
\h'3.187i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'5.000i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRsynchronous \fP
.sp -1
\h'5.000i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRrequest\fP
.sp -1
\h'0.062i'\v'0.881i'\h'-0.0m'\v'0.2m'\s10\fRKey event\fP
.sp -1
\h'3.187i'\v'1.131i'\h'-0.0m'\v'0.2m'\s10\fR(synchronous)   \fP
.sp -1
\h'0.062i'\v'1.256i'\h'-0.0m'\v'0.2m'\s10\fRButton press causes\fP
.sp -1
\h'0.062i'\v'1.381i'\h'-0.0m'\v'0.2m'\s10\fRfocus change\fP
.sp -1
\h'1.250i'\v'1.381i'\h'-0.0m'\v'0.2m'\s10\fRXSetICFocus\fP
.sp -1
\h'3.250i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRor XIM_COMMIT\fP
.sp -1
\h'3.187i'\v'1.443i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'3.687i'\v'1.693i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC\fP
.sp -1
\h'3.375i'\v'2.006i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC_REPLY\fP
.sp -1
\h'1.250i'\v'2.193i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'1.250i'\v'2.568i'\h'-0.0m'\v'0.2m'\s10\fRXmbLookupString\fP
.sp -1
\h'1.250i'\v'2.381i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent (returns False)   \fP
.sp -1
\v'2.856i'\h'-0.0m'\v'0.2m'\s10\fRthe focus\fP
.sp -1
\v'2.693i'\h'-0.0m'\v'0.2m'\s10\fRApplication moves\fP
.sp -1
\h'5.000i'\v'3.068i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'5.000i'\v'3.193i'\h'-0.0m'\v'0.2m'\s10\fR(The focused\fP
.sp -1
\h'5.000i'\v'3.318i'\h'-0.0m'\v'0.2m'\s10\fRIC is changed)  \fP
.sp -1
\h'5.000i'\v'3.568i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'3.125i'\v'3.568i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SYNC_REPLY as a reply\fP
.sp -1
\h'3.125i'\v'3.731i'\h'-0.0m'\v'0.2m'\s10\fRof the XIM_FORWARD_EVENT\fP
.sp -1
\h'1.250i'\v'3.568i'\h'-0.0m'\v'0.2m'\s10\fRXNextEvent\fP
.sp -1
\h'5.000i'\v'4.068i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'5.000i'\v'1.381i'\h'-0.0m'\v'0.2m'\s10\fRPending\fP
.sp -1
\h'5.000i'\v'4.256i'\h'-0.0m'\v'0.2m'\s10\fRprocessed\fP
.sp -1
\h'1.250i'\v'2.818i'\h'-0.0m'\v'0.2m'\s10\fRXSetICFocus\fP
.sp -1
\h'3.125i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRis started by XIM_COMMIT\fP
.sp -1
\h'3.125i'\v'2.193i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SET_IC_FOCUS is\fP
.sp -1
\h'3.125i'\v'2.318i'\h'-0.0m'\v'0.2m'\s10\fRpend because another sync cycle\fP
.sp -1
\h'2.062i'\v'1.693i'\h'-0.0m'\v'0.2m'\s10\fRsync cycle is done\fP
.sp -1
\h'2.062i'\v'1.568i'\h'-0.0m'\v'0.2m'\s10\fRPending until\fP
.sp -1
\v'3.568i'\h'-0.0m'\v'0.2m'\s10\fRKey event\fP
.sp -1
\h'1.250i'\v'3.756i'\h'-0.0m'\v'0.2m'\s10\fRXFilterEvent\fP
.sp -1
\h'3.125i'\v'4.631i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
\h'3.375i'\v'4.131i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SET_IC_FOCUS\fP
.sp -1
\h'3.250i'\v'2.818i'\h'-0.0m'\v'0.2m'\s10\fRXIM_SET_IC_FOCUS\fP
.sp -1
.sp 1+4.721i
.PE
.if \n(00 .fi

.\"====================== event2 flow figure end =======================
.ce
.sp
Fig.3 Sample Protocol Flow chart
.LP
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_FORWARD_EVENT (IM library \(<-\(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:BITMASK16:flag
::#0001:synchronous
::#0002:request filtering (*1)
::#0004:request lookupstring (*2)
:2:CARD16:serial number
::XEVENT:X event
.TE
.LP
.IP (*1)
Indicate the receiver should filter events and possible preedit may be invoked.
.IP (*2)
Indicate the receiver should only do lookup string. The IM Server is expected
to just do a conversion of the key event to the best candidate. This bit may
affect the state of the preedit state (e.g. compose of dead key sequences).
.RE
.LP
XEVENT format is same as the X Protocol event format(xEvent).
As the value of xEvent's sequenceNumber is the bottom of 16 bit of XEvent's 
xany.serial, the top of 16 bit is sent by serial number(INT16).
.LP
.PN XIM_FORWARD_EVENT
message is used for forwarding the events from the IM library to the IM Server 
in order for IM to be able to filter the event. On the other hand, this 
message is also used for forwarding the events from the IM Server to the IM 
library if the event forwarded from the IM library is not filtered. 
The IM Server, which receives 
.PN XIM_FORWARD_EVENT
message without synchronous bit, should set synchronous bit.
If both ``request event filtering'' and ``request lookupstring'' flag are
set, then both filtering and lookup should be done for the same event.
.LP
.NH 2
Synchronizing with the IM Server
.XS
\*(SN Synchronizing with the IM Server
.XE
.LP
.PN XIM_SYNC
message requests to synchronize the IM library and the IM Server. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SYNC (IM library \(<-\(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
This synchronization can be started either on the IM library side or on the 
IM Server side.  The side which receives 
.PN XIM_SYNC
message should process all XIM requests before replying. The input-context-ID 
is necessary to distinguish the IC with which the IM library and the IM 
Server are synchronized.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_SYNC_REPLY (IM Server \(<-\(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
The side which receives 
.PN XIM_FORWARD_EVENT, 
.PN XIM_COMMIT
or any other message with synchronous bit, should process all XIM request 
before replying, and send 
.PN XIM_SYNC_REPLY
message as the reply to the previous message. 
.LP
.NH 2
Sending a committed string
.XS
\*(SN Sending a committed string
.XE
.LP
When the IM Server commits a string, the IM Server sends either the committed 
string or list of KeySym, or both, by 
.PN XIM_COMMIT
message.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_COMMIT (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:BITMASK16:flag
::#0001:synchronous
::#0002:XLookupChars
::#0004:XLookupKeySym
::#0006: XLookupBoth = XLookupChars | XLookupKeySym
.TE
.LP
If flag is XLookupKeySym, the arguments continue as follows:
.TS
tab(:);
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
:2::unused
:4:KEYSYM:KeySym
.TE
.LP
If flag is XLookupChars, the arguments continue as follows:
.TS
tab(:);
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
:2:m:byte length of committed string
:m:LISTofBYTE:committed string
:p::unused, p = Pad(m)
.TE
.LP
If flag is XLookupBoth, the arguments continue as follows:
.TS
tab(:);
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
:2::unused
:4:KEYSYM:KeySym
:2:n:byte length of committed string
:n:LISTofBYTE:committed string
:p::unused, p = Pad(2+n)
.TE
.RE
.LP
The IM Server which receives 
.PN XIM_COMMIT
message without synchronous bit should set synchronous bit.
.LP
.NH 2
Reset IC
.XS
\*(SN Reset IC
.XE
.LP
.PN XIM_RESET_IC
message requests to reset the status of IC in the IM Server. 
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_RESET_IC (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.PN XIM_RESET_IC
is a synchronous request. The IM library should wait until receiving either an 
.PN XIM_RESET_IC_REPLY
packet or an 
.PN XIM_ERROR
packet.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_RESET_IC_REPLY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:n:byte length of preedit string
:n:LISTofBYTE:preedit string
:p::unused, p = Pad(2+n)
.TE
.RE
.LP
.PN XIM_RESET_IC_REPLY 
message returns the input-context-ID to distinguish replies from multiple ICs.
.LP
.\"============================== Callbacks ===============================
.NH 2
Callbacks
.XS
\*(SN Callbacks
.XE
.LP
If XIMStyle has XIMPreeditArea or XIMStatusArea set, XIMGeometryCallback 
may be used, and if XIMPreeditCallback and/or XIMStatusCallback are set,
corresponding callbacks may be used.
.LP
Any callback request may be sent from an IM Server to an IM client 
asynchronously in response to any request previously sent by the IM client 
to the IM Server.
.LP
When an IM Server needs to send a callback request synchronously with
the request previously sent by an IM client, the IM Server sends it
before replying to the previous request.
.LP
.NH 3
Negotiating geometry
.XS
\*(SN Negotiating geometry
.XE
.LP
The IM Server sends
.PN XIM_GEOMETRY 
message to start geometry negotiation, if XIMStyle has XIMPreeditArea or 
XIMStatusArea set.  
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_GEOMETRY (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
There is always a single Focus Window, even if some input fields have only 
one IC.
.LP
.NH 3
Converting a string
.XS
\*(SN Converting a string
.XE
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_STR_CONVERSION (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:CARD16:XIMStringConversionPosition
:2::unused
:4:CARD32:XIMCaretDirection
::#0:XIMForwardChar
::#1:XIMBackwardChar
::#2:XIMForwardWord
::#3:XIMBackwardWord
::#4:XIMCaretUp
::#5:XIMCaretDown
::#6:XIMNextLine
::#7:XIMCPreviousLine
::#8:XIMLineStart
::#9:XIMLineEnd
::#10:XIMAbsolutePosition
::#11:XIMDontChange
:2:CARD16:factor
:2:CARD16:XIMStringConversionOperation
::#0001:XIMStringConversionSubstitution
::#0002:XIMStringConversionRetrieval
:2:INT16:T{
byte length to multiply the XIMStringConversionType
T}
.TE
.RE
.sp
.LP
.PN XIM_STR_CONVERSION 
message may be used to start the string conversion from the IM 
Server.
.LP
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_STR_CONVERSION_REPLY (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:CARD32:XIMStringConversionFeedback
::XIMSTRCONVTEXT:XIMStringConversionText
.sp
.TE
.RE
.LP
.PN XIM_STR_CONVERSION_REPLY 
message returns the string to be converted and the feedback information array.
.LP
.NH 3
Preedit Callbacks
.XS
\*(SN Preedit Callbacks
.XE
.LP
The IM Server sends
.PN XIM_PREEDIT_START 
message to call the XIMPreeditStartCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_START (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
The reply to this message must be sent synchronously. The reply forwards 
the return value from the callback function to the IM Server.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_START_REPLY (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:INT32:return value
.TE
.RE
.LP
.PN XIM_PREEDIT_START_REPLY
message returns the input-context-ID to distinguish replies from multiple 
IC's.  The return value contains the return value of the function 
XIMPreeditStartCallback.
.LP
The IM Server sends
.PN XIM_PREEDIT_DRAW 
message to call the XIMPreeditDrawCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_DRAW (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:INT32:caret
:4:INT32:chg_first
:4:INT32:chg_length
:4:BITMASK32:status
::#x0000001:no string
::#x0000002:no feedback
:2:n:length of preedit string
:n:STRING8:preedit string
:p::unused, p = Pad(2+n)
:2:m:byte length of feedback array
:2::unused
:m:LISTofXIMFEEDBACK:feedback array
.TE
.RE
.LP
The fields ``caret'', ``chg_first'' and ``chg_length'' correspond to the 
fields of XIMPreeditDrawCallbackStruct.
When the ``no string'' bit of the status field is set, the text field of 
XIMPreeditDrawCallbackStruct is NULL.
When the ``no feedback'' bit of the status field is set, the text feedback 
field of XIMPreeditDrawCallbackStruct is NULL.
When the above bits are not set, ``preedit string'' contains the preedit 
string to be displayed, and the feedback array contains feedback information.
.LP
The IM Server sends
.PN XIM_PREEDIT_CARET 
message to call the PreeditCaretCallback function.  
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_CARET (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:INT32:position
:4:CARD32:direction
::#0:XIMForwardChar
::#1:XIMBackwardChar
::#2:XIMForwardWord
::#3:XIMBackwardWord
::#4:XIMCaretUp
::#5:XIMCaretDown
::#6:XIMNextLine
::#7:XIMCPreviousLine
::#8:XIMLineStart
::#9:XIMLineEnd
::#10:XIMAbsolutePosition
::#11:XIMDontChange
:4:CARD32:style
::#0:XIMInvisible
::#1:XIMCPrimary
::#2:XIMSecondary
.TE
.RE
.LP
Each entry corresponds to a field of XIMPreeditCaretCallbackStruct. 
Since this callback sets the caret position, its reply must be sent 
synchronously.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_CARET_REPLY (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:CARD32:position
.TE
.RE
.LP
The position is the value returned by the callback function after it
has been called.
.LP
The IM Server sends
.PN XIM_PREEDIT_DONE 
message to call the XIMPreeditDoneCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDIT_DONE (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.NH 3
Preedit state notify
.XS
\*(SN Preedit state notify
.XE
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_PREEDITSTATE (IM Server \(-> IM Library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:BITMASK32:XIMPreeditState
::#x0000000:XIMPreeditUnknown
::#x0000001:XIMPreeditEnable
::#x0000002:XIMPreeditDisable
.TE
.sp
.TE
.RE
.LP
.PN XIM_PREEDITSTATE
message is used to call the XIMPreeditStateNotifyCallback function.
.LP
.NH 3
Status Callbacks
.XS
\*(SN Status Callbacks
.XE
.LP
The IM Server sends
.PN XIM_STATUS_START 
message to call the XIMStatusStartCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_STATUS_START (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
The IM Server sends
.PN XIM_STATUS_DRAW 
message to call the XIMStatusDrawCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_STATUS_DRAW (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:CARD32:type
::#0:XIMTextType
::#1:XIMBitmapType
.TE
.LP
If type is XIMTextType, the arguments continue as follows.
.TS
tab(:);
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
:4:BITMASK32:status
::#x0000001:no string
::#x0000002:no feedback
:2:n:length of status string
:n:STRING8:status string
:p::unused, p = Pad(2+n)
:2:m:byte length of feedback array
:2::unused
:m:LISTofXIMFEEDBACK:feedback array
.TE
.LP
If type is XIMBitmapType, the arguments continue as follows.
.TS
tab(:);
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
:4:PIXMAP:pixmap data
.TE
.RE
.LP
The field ``type'' corresponds to the field in XIMStatusDrawCallbackStruct.
.LP
The IM Server sends
.PN XIM_STATUS_DONE 
message to call the XIMStatusDoneCallback function.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_STATUS_DONE (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
.TE
.RE
.LP
.bp
.NH 1
Acknowledgements
.XS
\*(SN Acknowledgements
.XE
.LP
This document represents the culmination of several years of debate and
experiments done under the auspices of the MIT X Consortium i18n working 
group.  Although this was a group effort, the author remains responsible 
for any errors or omissions.
.LP
We would like to thank to all members of this group.
And we would like to make special thanks to the following people 
(in alphabetical order) for their participation in the IM Protocol 
design,
Hector Chan, Takashi Fujiwara, Yoshio Horiuchi, Makoto Inada, 
Hiromu Inukai, Mickael Kung, Seiji Kuwari, Franky Ling, Hiroyuki Machida, 
Hiroyuki Miyamoto, Frank Rojas, Bob Scheifler, Makiko Shimamura, 
Shoji Sugiyama, Hidetoshi Tajima, Masaki Takeuchi, Makoto Wakamatsu, 
Masaki Wakao, Nobuyuki Tanaka, Shigeru Yamada, Katsuhisa Yano, Jinsoo Yoon.
.LP
.NH 1
References
.XS
\*(SN References
.XE
.LP
All of the following documents are X Consortium standards available from MIT:
.LP
[1] Scheifler, Robert W., \fI``X Window System Protocol Version 11''\fP
.LP
[2] Scheifler, Robert W. etc., \fI``Xlib \- C Language X Interface''\fP
.LP
.bp
.XS
Appendix A \- Common Extensions
.XE
.ce 10
.sp 5
\s+2\fBAppendix A\fP\s-2
.sp
\s+1\fBCommon Extensions\fP\s-1
.ce 0
.sp
.LP
Extension opcodes and packet names (e.g. 
.PN XIM_EXT_SET_EVENT_MASK
) for additional extensions may be registered with X Consortium.
The following is a commonly well-known extended packet.
.LP
.LP
.IP \fB(1)
Extension to manipulate the event handling\fP 
.LP
.PN XIM_EXT_SET_EVENT_MASK 
message specifies the set of event masks that the IM library should manipulate.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_EXT_SET_EVENT_MASK (IM Server \(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:4:EVENTMASK:filter-event-mask (*1)
:4:EVENTMASK:intercept-event-mask (*2)
:4:EVENTMASK:select-event-mask (*3)
:4:EVENTMASK:forward-event-mask (*4)
:4:EVENTMASK:synchronous-event-mask (*5)
.TE
.IP (*1)
Specify the events to be neglected by the IM library via XFilterEvent.
.IP (*2)
Specify the events to be deselected by the IM library with XSelectInput.
.IP (*3)
Specify the events to be selected by the IM library with XSelectInput.
.IP (*4)
Specify all the events to be forwarded to the IM Server by the IM library.
.IP (*5)
Specify the events to be forwarded with synchronous flag on by the IM library.
.RE
.LP
The IM library must reply 
.PN XIM_SYNC_REPLY
message to the IM Server. This request is valid after the ic is created.
.LP
.sp
.IP \fB(2)
Extension for improvement of performance\fR
.LP
The following requests may be used for improvement of performance.
.LP
.PN XIM_EXT_FORWARD_KEYEVENT
message may be used instead of 
.PN XIM_FORWARD_EVENT
message.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_EXT_FORWARD_KEYEVENT (IM Server \(<-\(-> IM library)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:BITMASK16:flag
::#0001:synchronous
:2:CARD16:sequence number
:1:BYTE:xEvent.u.u.type
:1:BYTE:keycode
:2:CARD16:state
:4:CARD32:time
:4:CARD32:window
.TE
.RE
.LP
.bp
.PN XIM_EXT_MOVE
message may be used to change the spot location instead of
.PN
XIM_SET_IC_VALUES
message.
It is effective only if the client specified XIMPreeditPosition.
.RS
.TS
tab(:);
lfB s s s
lw(.25i) lw(.25i) lw(1.75i) lw(3.5i).
XIM_EXT_MOVE (IM library \(-> IM Server)
.sp 6p
:2:CARD16:input-method-ID
:2:CARD16:input-context-ID
:2:INT16:X
:2:INT16:Y
.TE
.RE
.LP
.PN XIM_EXT_MOVE
message is a asynchronous request.  
.LP
.bp
.XS
Appendix B \- The list of transport specific IM Server names registered
.XE
.ce 10
.sp 5
\s+2\fBAppendix B\fP\s-2
.sp
\s+1\fBThe list of transport specific IM Server address format registered\fP\s-1
.ce 0
.sp
.LP
The following format represents the ATOM contained in
.PN XIM_SERVERS
property and the string returned from the request converting 
selection target LOCALES and TRANSPORT.
.DS 
	``{@\^\fIcategory\fP\^=[\^\fIvalue\fP,...]}...''
.DE
.LP
The following categories are currently registered.
.RS
.TS
tab(;);
l l.
\fBserver\fP;: IM Server name (used for XIM_SERVERS)
\fBlocale\fP;: XPG4 locale name (LOCALES)
\fBtransport\fP;: transport-specific name (TRANSPORT)
.TE
.RE
.LP
The preregistered formats for transport-specific names are as follows:
.RS
.LP
\fBTCP/IP Names\fP
.LP
.RS
The following syntax should be used for system internal domain names:
.DS
<\fIlocal name\fP>  ::= ``local/''<\fIhostname\fP>``:''<\fIpathname\fP>
.DE
.LP
Where <\fIpathname\fP> is a path name of socket address.
.LP
IM Server's name should be set to <\fIpathname\fP> to run multiple IM Server 
at the same time
.LP
The following syntax should be used for Internet domain names:
.DS
<\fITCP name\fP>  ::=  ``tcp/''<\fIhostname\fP>``:''<\fIipportnumber\fP>
.DE
where <\fIhostname\fP> is either symbolic (such as expo.lcs.mit.edu) or 
numeric decimal (such as 18.30.0.212).  The <\fIipportnumber\fP> is the 
port on which the IM Server is listening for connections.  
For example:
.DS
tcp/expo.lcs.mit.edu:8012
tcp/18.30.0.212:7890
.DE
.RE
.LP
\fBDECnet Names\fP
.LP
.RS
The following syntax should be used for DECnet names:
.DS
<\fIDECnet name\fP>  ::=  ``decnet/''<\fInodename\fP>``::IMSERVER$''<\fIobjname\fP>
.DE
where <\fInodename\fP> is either symbolic (such as SRVNOD) or the numeric 
decimal form of the DECnet address (such as 44.70).  The <\fIobjname\fP> 
is normal, case-insensitive DECnet object name. For example:
.DS
DECNET/SRVNOD::IMSERVER$DEFAULT
decnet/44.70::IMSERVER$other
.DE
.RE
.LP
\fBX Names\fP
.LP
.RS
The following syntax should be used for X names:
.DS
<\fIX name\fP>  ::=  ``X/''
.DE
.RE
.RE
.LP
If a given category has multiple values, the value is evaluated in order of
setting.
.bp
.XS
Appendix C \- Protocol number
.XE
.ce 10
.sp 5
\s+2\fBAppendix C\fP\s-2
.sp
\s+1\fBProtocol number\fP\s-1
.ce 0
.sp
.LP
\fBMajor Protocol number\fP
.TS
center, tab(:);
lw(9c) l.
XIM_CONNECT:#001
XIM_CONNECT_REPLY:#002
XIM_DISCONNECT:#003
XIM_DISCONNECT_REPLY:#004

XIM_AUTH_REQUIRED:#010
XIM_AUTH_REPLY:#011
XIM_AUTH_NEXT:#012
XIM_AUTH_SETUP:#013
XIM_AUTH_NG:#014

XIM_ERROR:#020

XIM_OPEN:#030
XIM_OPEN_REPLY:#031
XIM_CLOSE:#032
XIM_CLOSE_REPLY:#033
XIM_REGISTER_TRIGGERKEYS:#034
XIM_TRIGGER_NOTIFY:#035
XIM_TRIGGER_NOTIFY_REPLY:#036
XIM_SET_EVENT_MASK:#037
XIM_ENCODING_NEGOTIATION:#038
XIM_ENCODING_NEGOTIATION_REPLY:#039
XIM_QUERY_EXTENSION:#040
XIM_QUERY_EXTENSION_REPLY:#041
XIM_SET_IM_VALUES:#042
XIM_SET_IM_VALUES_REPLY:#043
XIM_GET_IM_VALUES:#044
XIM_GET_IM_VALUES_REPLY:#045

XIM_CREATE_IC:#050
XIM_CREATE_IC_REPLY:#051
XIM_DESTROY_IC:#052
XIM_DESTROY_IC_REPLY:#053
XIM_SET_IC_VALUES:#054
XIM_SET_IC_VALUES_REPLY:#055
XIM_GET_IC_VALUES:#056
XIM_GET_IC_VALUES_REPLY:#057
XIM_SET_IC_FOCUS:#058
XIM_UNSET_IC_FOCUS:#059
XIM_FORWARD_EVENT:#060
XIM_SYNC:#061
XIM_SYNC_REPLY:#062
XIM_COMMIT:#063
XIM_RESET_IC:#064
XIM_RESET_IC_REPLY:#065

XIM_GEOMETRY:#070
XIM_STR_CONVERSION:#071
XIM_STR_CONVERSION_REPLY:#072
XIM_PREEDIT_START:#073
XIM_PREEDIT_START_REPLY:#074
XIM_PREEDIT_DRAW:#075
XIM_PREEDIT_CARET:#076
XIM_PREEDIT_CARET_REPLY:#077
XIM_PREEDIT_DONE:#078
XIM_STATUS_START:#079
XIM_STATUS_DRAW:#080
XIM_STATUS_DONE:#081
XIM_PREEDITSTATE:#082
.TE
.sp
(*) The IM Server's extension protocol number should be more than #128.
.bp
.XS
Appendix D \- Implementation Tips
.XE
.ce 10
.sp 5
\s+2\fBAppendix D\fP\s-2
.sp
\s+1\fBImplementation Tips\fP\s-1
.ce 0
.sp
.LP
.B
.IP \fB(1)
FrontEnd Method\fP
.LP
FrontEnd method is recognized as a performance acceleration by the
trade off of the variety of the reliability.
.LP
In order to use the FrontEnd method, the IM library must query the IM
Server to see if the FrontEnd extension is available.  The query is
made by using the 
.PN XIM_QUERY_EXTENSION
message. The IM Server may send 
.PN XIM_EXT_SET_EVENT_MASK
message with intercept-event-mask, forward-event-mask, and 
synchronous-event-mask values set after replying 
.PN XIM_QUERY_EXTENSION_REPLY
message.  
.LP
FrontEnd method can be implemented in a couple of ways depending on
how the IM Server utilize 
.PN XIM_EXT_SET_EVENT_MASK
message.
.LP
One approach is to update both of the input mask and the filter-event-mask 
depending on the preeidting state. The sample protocol sequence using the 
static event flow is as follows:
.LP
.\"===================================================================
.sp
\^... 1.675 6.888 6.237 10.296
\^... 0.000i 3.408i 4.562i 0.000i
.nr 00 \n(.u
.nf
.PS 3.408i 4.562i 
.br
.ps 11
\h'3.750i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'3.912i'\v'1.384i'\D'l-0.100i 0.025i'
.sp -1
\h'3.912i'\v'1.434i'\D'l-0.100i -0.025i'
.sp -1
\h'3.812i'\v'1.409i'\l'0.750i'
.sp -1
\h'3.750i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'3.812i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'3.812i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRto select the event\fP
.sp -1
\h'3.875i'\v'2.381i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'3.875i'\v'2.568i'\h'-0.0m'\v'0.2m'\s10\fRto deselect the event\fP
.sp -1
\h'3.875i'\v'1.631i'\h'-0.0m'\v'0.2m'\s10\fRX events directly come\fP
.sp -1
\h'3.875i'\v'1.756i'\h'-0.0m'\v'0.2m'\s10\fRto the IM Server.\fP
.sp -1
\h'3.875i'\v'2.006i'\h'-0.0m'\v'0.2m'\s10\fRwhen preediting is turning off\fP
.sp -1
\h'0.625i'\v'0.284i'\l'0.875i'
.sp -1
\h'1.400i'\v'0.309i'\D'l0.100i -0.025i'
.sp -1
\h'1.400i'\v'0.259i'\D'l0.100i 0.025i'
.sp -1
\h'1.750i'\v'0.346i'\l'1.687i'
.sp -1
\h'3.337i'\v'0.371i'\D'l0.100i -0.025i'
.sp -1
\h'3.337i'\v'0.321i'\D'l0.100i 0.025i'
.sp -1
\h'1.850i'\v'2.134i'\D'l-0.100i 0.025i'
.sp -1
\h'1.850i'\v'2.184i'\D'l-0.100i -0.025i'
.sp -1
\h'1.750i'\v'2.159i'\l'1.687i'
.sp -1
\h'1.562i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'1.850i'\v'0.446i'\D'l-0.100i 0.025i'
.sp -1
\h'1.850i'\v'0.496i'\D'l-0.100i -0.025i'
.sp -1
\h'1.750i'\v'0.471i'\l'1.687i'
.sp -1
\h'1.687i'\v'0.631i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'1.875i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRintercept-event-mask is set\fP
.sp -1
\h'1.687i'\v'2.318i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'1.875i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRselect-event-mask is set\fP
.sp -1
\h'0.937i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'0.250i'\v'2.318i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'0.250i'\v'2.506i'\h'-0.0m'\v'0.2m'\s10\fRto select the event\fP
.sp -1
\h'0.250i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRto deselect the event\fP
.sp -1
\h'0.250i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'1.812i'\v'0.256i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
.sp 1+3.408i
.PE
.if \n(00 .fi
.sp
.\"===================================================================
.LP
To pursuit a maximum performance regardless of the preediting mode,
the IM Server may use the dynamic event flow with the following 
sample protocol sequence.
.bp
.LP
.\"===================================================================
\^... 1.675 6.888 6.237 10.296
\^... 0.000i 3.408i 4.562i 0.000i
.nr 00 \n(.u
.nf
.PS 3.408i 4.562i 
.br
.ps 11
\h'3.750i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'3.912i'\v'1.384i'\D'l-0.100i 0.025i'
.sp -1
\h'3.912i'\v'1.434i'\D'l-0.100i -0.025i'
.sp -1
\h'3.812i'\v'1.409i'\l'0.750i'
.sp -1
\h'3.750i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'3.812i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'3.812i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRto select the event\fP
.sp -1
\h'3.875i'\v'2.381i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'3.875i'\v'2.568i'\h'-0.0m'\v'0.2m'\s10\fRto deselect the event\fP
.sp -1
\h'3.875i'\v'1.631i'\h'-0.0m'\v'0.2m'\s10\fRX events directly come\fP
.sp -1
\h'3.875i'\v'1.756i'\h'-0.0m'\v'0.2m'\s10\fRto the IM Server.\fP
.sp -1
\h'3.875i'\v'2.006i'\h'-0.0m'\v'0.2m'\s10\fRwhen preediting is turning off\fP
.sp -1
\h'0.625i'\v'0.284i'\l'0.875i'
.sp -1
\h'1.400i'\v'0.309i'\D'l0.100i -0.025i'
.sp -1
\h'1.400i'\v'0.259i'\D'l0.100i 0.025i'
.sp -1
\h'1.750i'\v'0.346i'\l'1.687i'
.sp -1
\h'3.337i'\v'0.371i'\D'l0.100i -0.025i'
.sp -1
\h'3.337i'\v'0.321i'\D'l0.100i 0.025i'
.sp -1
\h'1.850i'\v'1.196i'\D'l-0.100i 0.025i'
.sp -1
\h'1.850i'\v'1.246i'\D'l-0.100i -0.025i'
.sp -1
\h'1.750i'\v'1.221i'\l'1.687i'
.sp -1
\h'1.850i'\v'2.134i'\D'l-0.100i 0.025i'
.sp -1
\h'1.850i'\v'2.184i'\D'l-0.100i -0.025i'
.sp -1
\h'1.750i'\v'2.159i'\l'1.687i'
.sp -1
\h'1.562i'\v'0.034i'\v'-.13m'\L'3.374i\(br'\v'.13m'
.sp -1
\h'1.850i'\v'0.446i'\D'l-0.100i 0.025i'
.sp -1
\h'1.850i'\v'0.496i'\D'l-0.100i -0.025i'
.sp -1
\h'1.750i'\v'0.471i'\l'1.687i'
.sp -1
\h'1.812i'\v'0.256i'\h'-0.0m'\v'0.2m'\s10\fRXIM_TRIGGER_NOTIFY\fP
.sp -1
\h'1.687i'\v'1.068i'\h'-0.0m'\v'0.2m'\s10\fRXIM_TRIGGER_NOTIFY_REPLY\fP
.sp -1
\h'1.687i'\v'0.631i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'1.875i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRintercept-event-mask is set\fP
.sp -1
\h'1.687i'\v'2.318i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'1.875i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRselect-event-mask is set\fP
.sp -1
\h'0.937i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\v'0.193i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'0.250i'\v'2.318i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
\h'0.250i'\v'2.506i'\h'-0.0m'\v'0.2m'\s10\fRto select the event\fP
.sp -1
\h'0.250i'\v'1.006i'\h'-0.0m'\v'0.2m'\s10\fRto deselect the event\fP
.sp -1
\h'0.250i'\v'0.818i'\h'-0.0m'\v'0.2m'\s10\fRevent mask is changed\fP
.sp -1
.sp 1+3.408i
.PE
.if \n(00 .fi
.\"===================================================================
.LP
This method can reduce the XIM protocol traffic dramatically
by updating intercept-event-mask and select-event-mask accordingly.
The tradeoff of this performance improvement is that the key
events may be lost or disordered in some particular situation, such as
when the user types the keyboard in following sequence really fast:
.sp 6p 
.RS
<preediting on key>``some strings''<preediting off key>``another string''
.RE
.sp  6p
Since this method requires the input mask updates to the both the IM Server 
and Xlib when turning on and off the preediting, and there is a time lag 
till the requests take effect when two client issues the input mask updates
simultaneously.
.LP
Another approach of the FrontEnd method is to update the filter-event-mask
depending on the preediting state and not to update the input mask.
The IM Server must register both of the preediting on key list and off key 
list by 
.PN XIM_REGISTER_TRIGGERKEYS
message.
In this method, Both the IM Server and the IM client select the same
events on the same client's window, so that the events are delivered
to both of the IM Server and the client. The preediting on and off 
states are expressed by whether the key events are filtered or not.
The sample protocol sequence are as follows:
.LP
.bp
<<Using static event flow>>
.LP
.\"====================================================================
.sp
\^... 1.488 7.325 6.487 10.358
\^... 0.000i 3.033i 4.999i 0.000i
.nr 00 \n(.u
.nf
.PS 3.033i 4.999i 
.br
.ps 11
\h'4.099i'\v'0.383i'\D'l-0.100i 0.025i'
.sp -1
\h'4.099i'\v'0.433i'\D'l-0.100i -0.025i'
.sp -1
\h'3.999i'\v'0.408i'\l'1.000i'
.sp -1
\h'4.099i'\v'1.696i'\D'l-0.100i 0.025i'
.sp -1
\h'4.099i'\v'1.746i'\D'l-0.100i -0.025i'
.sp -1
\h'3.999i'\v'1.721i'\l'1.000i'
.sp -1
\h'3.937i'\v'0.096i'\v'-.13m'\L'2.937i\(br'\v'.13m'
.sp -1
\h'3.937i'\v'0.062i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'4.062i'\v'0.755i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'4.062i'\v'0.943i'\h'-0.0m'\v'0.2m'\s10\fRare being processed\fP
.sp -1
\h'4.062i'\v'2.255i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'4.062i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRare being discarded\fP
.sp -1
\h'4.249i'\v'1.568i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the off-key-list\fP
.sp -1
\h'4.187i'\v'0.318i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'0.812i'\v'0.346i'\l'0.875i'
.sp -1
\h'1.587i'\v'0.371i'\D'l0.100i -0.025i'
.sp -1
\h'1.587i'\v'0.321i'\D'l0.100i 0.025i'
.sp -1
\h'1.937i'\v'0.408i'\l'1.687i'
.sp -1
\h'3.524i'\v'0.433i'\D'l0.100i -0.025i'
.sp -1
\h'3.524i'\v'0.383i'\D'l0.100i 0.025i'
.sp -1
\h'1.749i'\v'0.096i'\v'-.13m'\L'2.937i\(br'\v'.13m'
.sp -1
\h'2.037i'\v'0.508i'\D'l-0.100i 0.025i'
.sp -1
\h'2.037i'\v'0.558i'\D'l-0.100i -0.025i'
.sp -1
\h'1.937i'\v'0.533i'\l'1.687i'
.sp -1
\h'0.812i'\v'1.721i'\l'0.875i'
.sp -1
\h'1.587i'\v'1.746i'\D'l0.100i -0.025i'
.sp -1
\h'1.587i'\v'1.696i'\D'l0.100i 0.025i'
.sp -1
\h'2.099i'\v'1.758i'\D'l-0.100i 0.025i'
.sp -1
\h'2.099i'\v'1.808i'\D'l-0.100i -0.025i'
.sp -1
\h'1.999i'\v'1.783i'\l'1.688i'
.sp -1
\h'1.874i'\v'0.693i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\v'0.255i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'2.062i'\v'0.880i'\h'-0.0m'\v'0.2m'\s10\fRfilter-event-mask is set\fP
.sp -1
\h'0.624i'\v'0.755i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'0.624i'\v'0.943i'\h'-0.0m'\v'0.2m'\s10\fRare being filtered\fP
.sp -1
\h'1.937i'\v'1.943i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'2.124i'\v'2.068i'\h'-0.0m'\v'0.2m'\s10\fRfilter-event-mask is set\fP
.sp -1
\h'0.062i'\v'1.568i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the off-key-list\fP
.sp -1
\h'1.062i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\h'0.624i'\v'2.255i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'0.624i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRare being processed\fP
.sp -1
\h'1.999i'\v'0.318i'\h'-0.0m'\v'0.2m'\s10\fRXIM_FORWARD_EVENT\fP
.sp -1
.sp 1+3.033i
.PE
.if \n(00 .fi
.\"====================================================================
.LP
<<Using the dynamic event flow>>
.LP
.\"====================================================================
\^... 1.488 7.325 6.487 10.358
\^... 0.000i 3.033i 4.999i 0.000i
.nr 00 \n(.u
.nf
.PS 3.033i 4.999i 
.br
.ps 11
\h'4.099i'\v'0.383i'\D'l-0.100i 0.025i'
.sp -1
\h'4.099i'\v'0.433i'\D'l-0.100i -0.025i'
.sp -1
\h'3.999i'\v'0.408i'\l'1.000i'
.sp -1
\h'4.099i'\v'1.696i'\D'l-0.100i 0.025i'
.sp -1
\h'4.099i'\v'1.746i'\D'l-0.100i -0.025i'
.sp -1
\h'3.999i'\v'1.721i'\l'1.000i'
.sp -1
\h'3.937i'\v'0.096i'\v'-.13m'\L'2.937i\(br'\v'.13m'
.sp -1
\h'3.937i'\v'0.062i'\h'-0.0m'\v'0.2m'\s12\fRIM Server\fP
.sp -1
\h'4.062i'\v'0.755i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'4.062i'\v'0.943i'\h'-0.0m'\v'0.2m'\s10\fRare being processed\fP
.sp -1
\h'4.062i'\v'2.255i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'4.062i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRare being discarded\fP
.sp -1
\h'4.249i'\v'1.568i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the off-key-list\fP
.sp -1
\h'4.187i'\v'0.318i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'0.812i'\v'0.346i'\l'0.875i'
.sp -1
\h'1.587i'\v'0.371i'\D'l0.100i -0.025i'
.sp -1
\h'1.587i'\v'0.321i'\D'l0.100i 0.025i'
.sp -1
\h'1.937i'\v'0.408i'\l'1.687i'
.sp -1
\h'3.524i'\v'0.433i'\D'l0.100i -0.025i'
.sp -1
\h'3.524i'\v'0.383i'\D'l0.100i 0.025i'
.sp -1
\h'2.037i'\v'1.258i'\D'l-0.100i 0.025i'
.sp -1
\h'2.037i'\v'1.308i'\D'l-0.100i -0.025i'
.sp -1
\h'1.937i'\v'1.283i'\l'1.687i'
.sp -1
\h'1.749i'\v'0.096i'\v'-.13m'\L'2.937i\(br'\v'.13m'
.sp -1
\h'2.037i'\v'0.508i'\D'l-0.100i 0.025i'
.sp -1
\h'2.037i'\v'0.558i'\D'l-0.100i -0.025i'
.sp -1
\h'1.937i'\v'0.533i'\l'1.687i'
.sp -1
\h'0.812i'\v'1.721i'\l'0.875i'
.sp -1
\h'1.587i'\v'1.746i'\D'l0.100i -0.025i'
.sp -1
\h'1.587i'\v'1.696i'\D'l0.100i 0.025i'
.sp -1
\h'2.099i'\v'1.758i'\D'l-0.100i 0.025i'
.sp -1
\h'2.099i'\v'1.808i'\D'l-0.100i -0.025i'
.sp -1
\h'1.999i'\v'1.783i'\l'1.688i'
.sp -1
\h'1.999i'\v'0.318i'\h'-0.0m'\v'0.2m'\s10\fRXIM_TRIGGER_NOTIFY\fP
.sp -1
\h'1.874i'\v'1.130i'\h'-0.0m'\v'0.2m'\s10\fRXIM_TRIGGER_NOTIFY_REPLY\fP
.sp -1
\h'1.874i'\v'0.693i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\v'0.255i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the on-key-list\fP
.sp -1
\h'2.062i'\v'0.880i'\h'-0.0m'\v'0.2m'\s10\fRfilter-event-mask is set\fP
.sp -1
\h'0.624i'\v'0.755i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'0.624i'\v'0.943i'\h'-0.0m'\v'0.2m'\s10\fRare being filtered\fP
.sp -1
\h'1.937i'\v'1.943i'\h'-0.0m'\v'0.2m'\s10\fRXIM_EXT_SET_EVENT_MASK\fP
.sp -1
\h'2.124i'\v'2.068i'\h'-0.0m'\v'0.2m'\s10\fRfilter-event-mask is set\fP
.sp -1
\h'0.062i'\v'1.568i'\h'-0.0m'\v'0.2m'\s10\fRKeys in the off-key-list\fP
.sp -1
\h'1.062i'\h'-0.0m'\v'0.2m'\s12\fRIM library\fP
.sp -1
\h'0.624i'\v'2.255i'\h'-0.0m'\v'0.2m'\s10\fRthe specified events\fP
.sp -1
\h'0.624i'\v'2.443i'\h'-0.0m'\v'0.2m'\s10\fRare being processed\fP
.sp -1
.sp 1+3.033i
.PE
.if \n(00 .fi

.\"====================================================================
.LP
This method does not have the problem of the time lag when going across
the preediting on and off mode, however, the amount of the performance
acceleration is not as good as the method described above.
.LP
In general, the FrontEnd method requires some synchronization to some
of the X protocols, such as the ChangeWindowAttribute protocol for the 
event mask change or the GrabKey protocol, since it relies on the X's
principal event dispatching mechanism. Any X protocol bindings do not
consider the synchronization might cause some mis-synchronization 
between the IM clients and the IM Server.
.LP
.bp
.IP \fB(2) 
Transport Layer\fP 
.LP
The Xlib XIM implementation is layered into three functions, a protocol
layer, an interface layer and a transport layer. The purpose of this
layering is to make the protocol independent of transport implementation.
Each function of these layers are:
.RS 3
.IP "\fIThe protocol layer\fP"
.br
implements overall function of XIM and calls the interface layer
functions when it needs to communicate to IM Server. 
.IP "\fIThe interface layer\fP"
.br
separates the implementation of the transport layer from the protocol
layer, in other words, it provides implementation independent hook for
the transport layer functions.
.IP "\fIThe transport layer\fP"
.br
handles actual data communication with IM Server. It is done by a set
of several functions named transporters.
.RE
.LP
The interface layer and the transport layer make various communication
channels usable such as X Protocol, TCP/IP, DECnet or STREAM. 
The following is a sample implementation for the transporter using 
the X connection.
Refer to "xtrans" for the transporter using Socket Transport.
.LP
At the beginning of the X Transport connection for the XIM transport
mechanism, two different windows must be created either in an Xlib XIM
or in an IM Server, with which the Xlib and the IM Server exchange the
XIM transports by using the ClientMessage events and Window Properties.
In the following, the window created by the Xlib is referred as the
"client communication window", and on the other hand, the window created
by the IM Server is referred as the "IMS communication window".
.LP
.B
Connection
.LP
.RS
In order to establish a connection, a communication window is created.
A ClientMessage in the following event's format is sent to the owner 
window of XIM_SERVER selection, which the IM Server has created.
.LP
Refer to "The Input Method Protocol" for the XIM_SERVER atom.
.LP
.ce
Table D-1; The ClientMessage sent to the IMS window.
.TS H
tab(:);
l s|l
l l|l.
_
.sp 6p
.B
Structure Member:Contents
.sp 6p
_
.sp 6p
.TH
.R
int:type:ClientMessage  
u_long:serial:Set by the X Window System  
Bool:send_event:Set by the X Window System  
Display:*display:The display to which connects
Window:window:IMS Window ID
Atom:message_type:XInternAtom(display, ``_XIM_XCONNECT'', False)
int:format:32  
long:data.l[0]:client communication window ID  
long:data.l[1]:client-major-transport-version (*1)
long:data.l[2]:client-major-transport-version (*1)
.sp 6p
_
.TE
.LP
In order to establish the connection (to notify the IM Server communication
window), the IM Server sends a ClientMessage in the following event's
format to the client communication window.
.LP
.bp
.ce
Table D-2; The ClientMessage sent by IM Server.
.TS H
tab(:);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member:Contents
.sp 6p
_
.sp 6p
.TH
.R
int:type:ClientMessage  
u_long:serial:Set by the X Window System  
Bool:send_event:Set by the X Window System  
Display:*display:The display to which connects
Window:window:client communication window ID  
Atom:message_type:XInternAtom(display, ``_XIM_XCONNECT'', False)
int:format:32  
long:data.l[0]:IMS communication window ID  
long:data.l[1]:server-major-transport-version (*1)
long:data.l[2]:server-minor-transport-version (*1)
long:data.l[3]:dividing size between ClientMessage and Property (*2)
.sp 6p
_
.TE
.LP
.IP (*1) 
major/minor-transport-version
.RS
The read/write method is decided by the combination of 
major/minor-transport-version, as follows:
.LP
.ce
Table D-3; The read/write method and the major/minor-transport-version
.TS
center, tab(:);
| c s | l |
| c | c | l |.
_
.sp 6p
.B
Transport-version:read/write 
.sp 6p
_
.sp 6p
major:minor:
.sp 6p
_
.sp 6p
.R
0:0:only-CM & Property-with-CM
:1:only-CM & multi-CM
:2:only-CM & multi-CM & Property-with-CM
.sp 6p
_
.sp 6p
1:0:PropertyNotify
.sp 6p
_
.sp 6p
2:0:only-CM & PropertyNotify
:1:only-CM & multi-CM & PropertyNotify
.sp 6p
_
.TE
.LP
.RS
.TS
center, tab(;);
l n l.
only-CM;:;data is sent via a ClientMessage
multi-CM;:;data is sent via multiple ClientMessages 
Property-with-CM;:;T{
data is written in Property, and its Atom is send via ClientMessage
T}
PropertyNotify;:;T{
data is written in Property, and its Atom is send via PropertyNotify
T}
.TE
.RE
.LP
The method to decide major/minor-transport-version is as follows:
.LP
.IP (1)
The client sends 0 as major/minor-transport-version to the IM Server.
The client must support all methods in Table D-3.
The client may send another number as major/minor-transport-version to
use other method than the above in the future.
.IP (2)
The IM Server sends its major/minor-transport-version number to
the client. The client sends data using the method specified by the 
IM Server.
.IP (3)
If major/minor-transport-version number is not available, it is regarded
as 0.
.RE
.LP
.IP (*2) 
dividing size between ClientMessage and Property
.RS
If data is sent via both of multi-CM and Property, specify the dividing
size between ClientMessage and Property. The data, which is smaller than
this size, is sent via multi-CM (or only-CM), and the data, which is 
lager than this size, is sent via Property. 
.RE
.RE
.LP
.sp
.LP
.B
read/write
.LP  
.RS
The data is transferred via either ClientMessage or Window Property in
the X Window System.
.LP
.B
Format for the data from the Client to the IM Server
.LP
.RS
.B
ClientMessage
.LP
If data is sent via ClientMessage event, the format is as follows:
.LP
.ce
Table D-4; The ClientMessage event's format (first or middle)
.TS
tab(;);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member;Contents
.sp 6p
_
.sp 6p
.R
int;type;ClientMessage  
u_long;serial;Set by the X Window System  
Bool;send_event;Set by the X Window System  
Display;*display;The display to which connects  
Window;window;IMS communication window ID  
Atom;message_type;XInternAtom(display, ``_XIM_MOREDATA'', False)
int;format;8  
char;data.b[20];(read/write DATA : 20 byte)  
.sp 6p
_
.TE
.LP
.ce
Table D-5; The ClientMessage event's format (only or last)
.TS H
tab(;);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member;Contents
.sp 6p
_
.sp 6p
.TH
.R
int;type;ClientMessage  
u_long;serial;Set by the X Window System  
Bool;send_event;Set by the X Window System  
Display;*display;The display to which connects  
Window;window;IMS communication window ID  
Atom;message_type;XInternAtom(display, ``_XIM_PROTOCOL'', False)
int;format;8  
char;data.b[20];(read/write DATA : MAX 20 byte)  (*1)
.sp 6p
_
.TE
.IP (*1)
If the data is smaller than 20 byte, all data other than available data
must be 0.
.RE
.LP
.RS
.B
Property
.LP
In the case of large data, data will be sent via the Window Property 
for the efficiency.  There are the following two methods to notify 
Property, and transport-version is decided which method is used.
.LP
.IP (1)
The XChangeProperty function is used to store data in the client 
communication window, and Atom of the stored data is notified to the 
IM Server via ClientMessage event.
.IP (2)
The XChangeProperty function is used to store data in the client 
communication window, and Atom of the stored data is notified to the 
IM Server via PropertyNotify event.
.LP
The arguments of the XChangeProperty are as follows:
.LP
.bp
.ce
Table D-6; The XChangeProperty event's format
.TS H
tab(:);
l s | l
l l | l.
_
.sp 6p
.B
Argument:Contents
.sp 6p
_
.sp 6p
.TH
.R
Display:*display:The display to which connects
Window:window:IMS communication window ID  
Atom:property:read/write property Atom (*1) 
Atom:type:XA_STRING  
int:format:8  
int:mode:PropModeAppend  
u_char:*data:read/write DATA 
int:nelements:length of DATA
.sp 6p
_
.TE
.LP
.IP (*1)
The read/write property ATOM allocates the following strings by
\fBXInternAtom\fP.
.RS
``_clientXXX''
.RE
.LP
The client changes the property with the mode of PropModeAppend and
the IM Server will read it with the delete mode i.e. (delete = True).
.LP
If Atom is notified via ClientMessage event, the format of the ClientMessage 
is as follows:
.LP
.ce 
Table D-7; The ClientMessage event's format to send Atom of property
.TS H
tab(:);
l s | l 
l l | l.
_
.sp 6p
.B
Structure Member:Contents
.sp 6p
_
.sp 6p
.TH
.R
int:type:ClientMessage  
u_long:serial:Set by the X Window System  
Bool:send_event:Set by the X Window System  
Display:*display:The display to which connects
Window:window:IMS communication window ID  
Atom:message_type:XInternAtom(display, ``_XIM_PROTOCOL'', False)
int:format:32  
long:data.l[0]:length of read/write property Atom  
long:data.l[1]:read/write property Atom
.sp 6p
_
.TE
.RE
.LP
.B
Format for the data from the IM Server to the Client
.LP
.RS
.B
ClientMessage
.LP
The format of the ClientMessage is as follows:
.LP
.ce
Table D-8; The ClientMessage event's format (first or middle)
.TS H
tab(;);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member;Contents
.sp 6p
_
.sp 6p
.TH
.R
int;type;ClientMessage  
u_long;serial;Set by the X Window System  
Bool;send_event ;Set by the X Window System  
Display;*display;The display to which connects
Window;window;client communication window ID  
Atom;message_type;XInternAtom(display, ``_XIM_MOREDATA'', False)
int;format;8  
char;data.b[20];(read/write DATA : 20 byte)  
.sp 6p
_
.TE
.LP
.bp
.ce
Table D-9; The ClientMessage event's format (only or last)
.TS
tab(;);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member;Contents
.sp 6p
_
.sp 6p
.R
int;type;ClientMessage  
u_long;serial;Set by the X Window System  
Bool;send_event ;Set by the X Window System  
Display;*display;The display to which connects
Window;window;client communication window ID  
Atom;message_type;XInternAtom(display, ``_XIM_PROTOCOL'', False)
int;format;8  
char;data.b[20];(read/write DATA : MAX 20 byte) (*1) 
.sp 6p
_
.TE
.LP
.IP (*1)
If the data size is smaller than 20 bytes, all data other than available 
data must be 0.
.LP
.B
Property
.LP
In the case of large data, data will be sent via the Window Property
for the efficiency. There are the following two methods to notify 
Property, and transport-version is decided which method is used.
.LP
.IP (1)
The XChangeProperty function is used to store data in the IMS 
communication window, and Atom of the property is sent via the 
ClientMessage event.
.IP (2)
The XChangeProperty function is used to store data in the IMS 
communication window, and Atom of the property is sent via 
PropertyNotify event.
.LP
The arguments of the XChangeProperty are as follows:
.LP
.ce
Table D-10; The XChangeProperty event's format
.TS H
tab(:);
l s | l
l l | l.
_
.sp 6p
.B
Argument:Contents
.sp 6p
_
.sp 6p
.TH
.R
Display:*display:The display which to connects
Window:window:client communication window ID  
Atom:property:read/write property Atom (*1)
Atom:type:XA_STRING  
int:format:8  
int:mode:PropModeAppend  
u_char:*data:read/write DATA  
int:nelements:length of DATA  
.sp 6p
_
.TE
.LP  
.IP (*1)
The read/write property ATOM allocates some strings, which are not
allocated by the client, by \fBXInternAtom\fP.
.LP
The IM Server changes the property with the mode of PropModeAppend and
the client reads it with the delete mode, i.e. (delete = True).
.LP
If Atom is notified via ClientMessage event, the format of the ClientMessage 
is as follows:
.LP
.bp
.ce
Table D-11; The ClientMessage event's format to send Atom of property 
.TS H
tab(:);
l s | l
l l | l.
_
.sp 6p
.B
Structure Member:Contents
.sp 6p
_
.sp 6p
.TH
.R
int:type:ClientMessage  
u_long:serial:Set by the X Window System  
Bool:send_event:Set by the X Window System  
Display:*display:The display to which connects  
Window:window:client communication window ID  
Atom:message_type:XInternAtom(display, ``_XIM_PROTOCOL'', False)
int:format:32  
long:data.l[0]:length of read/write property ATOM  
long:data.l[1]:read/write property ATOM  
.sp 6p
_
.TE
.RE
.RE
.LP
.B
Closing Connection
.RS
.LP
If the client disconnect with the IM Server, shutdown function should 
free the communication window properties and etc..
.RE
.LP
.bp
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.TC

