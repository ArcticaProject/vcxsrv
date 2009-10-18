.\" $Xorg: Trans.ms,v 1.3 2000/08/17 19:42:49 cpqbld Exp $
.\" $XdotOrg: xc/doc/specs/i18n/Trans.ms,v 1.2 2004/04/23 18:42:19 eich Exp $
.\" To print this out, type tbl macros.t This File | troff -ms
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
.\" .nr PD 1v
.\" .nr DD 1v
\&
.sp 8
.TL
\s+3\fBThe XIM Transport Specification\s-3\fP
.sp
.sp
\fBRevision 0.1\fP
.sp
\fBX Version 11, Release 7\fP
.sp
\fB\*(xV\fP
.sp 3
.AU
Takashi Fujiwara
.AI
FUJITSU LIMITED
.sp 3
.AB
.LP
This specification describes the transport layer interfaces between
Xlib and IM Server, which makes various channels usable such as X
protocol or, TCP/IP, DECnet and etc.
.AE
.ce 0
.br
.LP
.bp
\&
.ps 9
.nr PS 9
.sp 8
.LP
Copyright \(co 1994 by FUJITSU LIMITED
.LP
Permission to use, copy, modify, and distribute this documentation
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission
notice appear in all copies.
Fujitsu makes no representations about the suitability 
for any purpose of the information in this document.
This documentation is provided as is without express or implied warranty.
.sp 5
Copyright \(co 1994 X Consortium
.LP
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the ``Software''), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
.LP
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
.LP
THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
.LP
Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
.sp 3
\fIX Window System\fP is a trademark of The Open Group.
.ps 11
.nr PS 11
.bp 1
.EH '\fBXIM Transport Specification\fP''\fB\*(xV\fP'
.OH '\fBXIM Transport Specification\fP''\fB\*(xV\fP'
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Introduction
.XS
\*(SN Introduction
.XE
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
This specification describes the interface layer and the transport 
layer, which makes various communication channels usable such as 
X protocol or, TCP/IP, DECnet, STREAM, etc., and provides
the information needed for adding another new transport layer.
In addition, sample implementations for the transporter using the 
X connection is described in section 4.
.NH 1
Initialization
.XS
\*(SN Initialization
.XE
.NH 2
Registering structure to initialize
.XS
\*(SN Registering structure to initialize
.XE
.LP
The structure typed as TransportSW contains the list of the transport
layer the specific implementations supports.
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef struct {
.br
      char *transport_name;
.br
      Bool (*config);
} TransportSW;
.De
.LP
.IP "\fItransport_name\fP" 15
name of transport(*1)
.FS 
(*1) Refer to "The Input Method Protocol: Appendix B" 
.FE
.IP "\fIconfig\fP" 15
initial configuration function
.LP
A sample entry for the Xlib supporting transporters is shown below:
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
TransportSW _XimTransportRec[] = {
.sp 3p
/*	char \fI*:
\ *	transport_name\fP,	Bool \fI(*config)()\fP
\ */
	``X'',	_XimXConf,
	``tcp'',	_XimTransConf,
	``local'',	_XimTransConf,
	``decnet'',	_XimTransConf,
	``streams'',	_XimTransConf,
	(char *)NULL,	(Bool (*)())NULL,
};
.De
.LP
.NH 2
Initialization function
.XS
\*(SN Initialization function
.XE
.LP
The following function will be called once when Xlib configures the
transporter functions.
.sp 6p 
.FD 0
Bool (*config)(\fIim\fP, \fItransport_data\fP)
.br
      XIM \fIim\fP;
.br
      char \fI*transport_data\fP;
.br
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fItransport_data\fP 1i
Specifies the data specific to the transporter, in IM Server address. (*1)
.FS 
(*1) Refer to "The Input Method Protocol: Appendix B" 
.FE
.sp 6p
.LP
This function must setup the transporter function pointers. 
.LP
The actual \fIconfig\fP function will be chosen by IM Server at the
pre-connection time, matching by the \fItransport_name\fP specified 
in the \fB_XimTransportRec\fP array; The specific members of XimProto 
structure listed below must be initialized so that point they 
appropriate transporter functions.
.LP
If the specified transporter has been configured successfully, this
function returns True. There is no Alternative Entry for config
function itself.
.LP
The structure XimProto contains the following function pointers:
.DS
.TA .5i 2.5i
.ta .5i 2.5i
Bool (*connect)();			/* Open connection */
Bool (*shutdown)();			/* Close connection */
Bool (*write)();				/* Write data */
Bool (*read)();				/* Read data */
Bool (*flush)();				/* Flush data buffer */
Bool (*register_dispatcher)();	/* Register asynchronous data handler */
Bool (*call_dispatcher)();	/* Call dispatcher */
.DE
These functions are called when Xlib needs to communicate the
IM Server. These functions must process the appropriate procedure
described below.
.LP
.NH 1
The interface/transport layer functions
.XS
\*(SN The interface/transport layer functions
.XE
.LP  
Following functions are used for the transport interface.
.LP
.ce
Table 3-1; The Transport Layer Functions.
.SM
.TS
tab(:) center box;
cw(4c) | cw(4c) | c
c | c | c
l | l | c.
.B
Alternative Entry:XimProto member:Section 
(Interface Layer):(Transport Layer):\^
=
.R
\fB_XimConnect\fP:connect:3.1
_
\fB_XimShutdown\fP:shutdown:3.2
_
\fB_XimWrite\fP:write:3.3
_
\fB_XimRead\fP:read:3.4
_
\fB_XimFlush\fP:flush:3.5
_
\fB_XimRegisterDispatcher\fP:register_dispatcher:3.6
_
\fB_XimCallDispatcher\fP:call_dispatcher:3.7
.TE
.NL
.LP
The Protocol layer calls the above functions using the Alternative
Entry in the left column. The transport implementation defines
XimProto member function in the right column. The Alternative Entry is
provided so as to make easier to implement the Protocol Layer.
.LP
.NH 2
Opening connection
.XS
\*(SN Opening connection
.XE
.LP
When \fBXOpenIM\fP is called, the following function is called to connect
with the IM Server.
.sp 6p
.FD 0
Bool (*connect)(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.sp 6p
.LP
This function must establishes the connection to the IM Server. If the
connection is established successfully, this function returns True.
The Alternative Entry for this function is:
.sp 6p
.FD 0
Bool _XimConnect(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.LP
.NH 2
Closing connection
.XS
\*(SN Closing connection
.XE
.LP
When \fBXCloseIM\fP is called, the following function is called to
disconnect the connection with the IM Server. The Alternative Entry
for this function is:
.sp 6p
.FD 0
Bool (*shutdown)(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.sp 6p
.LP
This function must close connection with the IM Server. If the
connection is closed successfully, this function returns True. The
Alternative Entry for this function is:
.sp 6p
.FD 0
Bool _XimShutdown(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP
Specifies XIM structure address.
.LP
.NH 2
Writing data
.XS
\*(SN Writing data
.XE
.LP
The following function is called, when Xlib needs to write data to the
IM Server.
.sp 6p
.FD 0
Bool (*write)(\fIim\fP, \fIlen\fP, \fIdata\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIdata\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIlen\fP 1i
Specifies the length of writing data.
.IP \fIdata\fP 1i
Specifies the writing data.
.sp 6p
.LP
This function writes the \fIdata\fP to the IM Server, regardless
of the contents.  The number of bytes is passed to \fIlen\fP. The
writing data is passed to \fIdata\fP. If data is sent successfully,
the function returns True. Refer to "The Input Method Protocol" for
the contents of the writing data. The Alternative Entry for this
function is:
.sp 6p
.FD 0
Bool _XimWrite(\fIim\fP, \fIlen\fP, \fIdata\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIdata\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIlen\fP 1i
Specifies the length of writing data.
.IP \fIdata\fP 1i
Specifies the writing data.
.LP
.NH 2
Reading data
.XS
\*(SN Reading data
.XE
.LP
The following function is called when Xlib waits for response from IM
server synchronously.
.sp 6p
.FD 0
Bool (*read)(\fIim\fP, \fIread_buf\fP, \fIbuf_len\fP, \fIret_len\fP)
.br
      XIM \fIim\fP;
.br
      XPointer \fIread_buf\fP;
.br
      int \fIbuf_len\fP;
.br
      int \fI*ret_len\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIread_buf\fP 1i
Specifies the buffer to store data.
.IP \fIbuf_len\fP 1i
Specifies the size of the \fIbuffer\fP
.IP \fIret_len\fP
Specifies the length of stored data.
.sp 6p
.LP
This function stores the read data in \fIread_buf\fP, which size is
specified as \fIbuf_len\fP. The size of data is set to \fIret_len\fP. 
This function return True, if the data is read normally or reading
data is completed.
.LP
The Alternative Entry for this function is:
.sp 6p
.FD 0
Bool _XimRead(\fIim\fP, \fIret_len\fP, \fIbuf\fP, \fIbuf_len\fP, \fIpredicate\fP, \fIpredicate_arg\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fI*ret_len\fP;
.br
      XPointer \fIbuf\fP;
.br
      int \fIbuf_len\fP;
.br
      Bool \fI(*predicate)()\fP;
.br
      XPointer \fIpredicate_arg\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIret_len\fP 1i
Specifies the size of the \fIdata\fP buffer.
.IP \fIbuf\fP 1i
Specifies the buffer to store data.
.IP \fIbuf_len\fP 1i
Specifies the length of \fIbuffer\fP.
.IP \fIpredicate\fP 1i
Specifies the predicate for the XIM data.
.IP \fIpredicate_arg\fP 1i
Specifies the predicate specific data.
.sp 6p
.LP
The predicate procedure indicates whether the \fIdata\fP is for the
XIM or not. \fIlen\fP
This function stores the read data in \fIbuf\fP, which size is specified 
as \fIbuf_len\fP. The size of data is set to \fIret_len\fP.
If \fIpreedicate()\fP returns True, this function returns True.
If not, it calls the registered callback function. 
.LP
The procedure and its arguments are:
.LP
.sp 6p
.FD 0
Bool (*predicate)(\fIim\fP, \fIlen\fP, \fIdata\fP, \fIpredicate_arg\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIdata\fP;
.br
      XPointer \fIpredicate_arg\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIlen\fP 1i
Specifies the size of the \fIdata\fP buffer.
.IP \fIdata\fP 1i
Specifies the buffer to store data.
.IP \fIpredicate_arg\fP 1i
Specifies the predicate specific data.
.LP
.NH 2
Flushing buffer
.XS
\*(SN Flushing buffer
.XE
.LP
The following function is called when Xlib needs to flush the data.
.sp 6p
.FD 0
void (*flush)(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.sp 6p
.LP
This function must flush the data stored in internal buffer on the
transport layer. If data transfer is completed, the function returns
True.  The Alternative Entry for this function is:
.sp 6p
.FD 0
void _XimFlush(\fIim\fP)
.br
      XIM \fIim\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.LP
.NH 2
Registering asynchronous data handler
.XS
\*(SN Registering asynchronous data handler
.XE
.LP  
Xlib needs to handle asynchronous response from IM Server. This is
because some of the XIM data occur asynchronously to X events.
.LP
Those data will be handled in the \fIFilter\fP, and the \fIFilter\fP
will call asynchronous data handler in the protocol layer. Then it
calls dispatchers in the transport layer. The dispatchers are
implemented by the protocol layer. This function must store the
information and prepare for later call of the dispatchers using
\fB_XimCallDispatcher\fP.
.LP
When multiple dispatchers are registered, they will be called
sequentially in order of registration, on arrival of asynchronous
data. The register_dispatcher is declared as following:
.sp 6p
.FD 0
Bool (*register_dispatcher)(\fIim\fP, \fIdispatcher\fP, \fIcall_data\fP)
.br
      XIM \fIim\fP;
.br
      Bool \fI(*dispatcher)()\fP;
.br
      XPointer \fIcall_data\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIdispatcher\fP 1i
Specifies the dispatcher function to register.
.IP \fIcall_data\fP 1i
Specifies a parameter for the \fIdispatcher\fP.
.LP
The dispatcher is a function of the following type:
.sp 6p
.FD 0
Bool (*dispatcher)(\fIim\fP, \fIlen\fP, \fIdata\fP, \fIcall_data\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIdata\fP;
.br
      XPointer \fIcall_data\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIlen\fP 1i
Specifies the size of the \fIdata\fP buffer.
.IP \fIdata\fP 1i
Specifies the buffer to store data.
.IP \fIcall_data\fP 1i
Specifies a parameter passed to the register_dispatcher.
.sp 6p
.LP
The dispatcher is provided by the protocol layer. They are called once
for every asynchronous data, in order of registration. If the data is
used, it must return True. otherwise, it must return False.
.LP
If the dispatcher function returns True, the Transport Layer assume
that the data has been processed by the upper layer.  The Alternative
Entry for this function is:
.sp 6p
.FD 0
Bool _XimRegisterDispatcher(\fIim\fP, \fIdispatcher\fP, \fIcall_data\fP)
.br
      XIM \fIim\fP;
.br
      Bool \fI(*dispatcher)()\fP;
.br
      XPointer \fIcall_data\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIdispatcher\fP 1i
Specifies the dispatcher function to register.
.IP \fIcall_data\fP 1i
Specifies a parameter for the \fIdispatcher\fP.
.LP
.NH 2
Calling dispatcher
.XS
\*(SN Calling dispatcher
.XE
.LP
The following function is used to call the registered dispatcher
function, when the asynchronous response from IM Server has arrived.
.sp 6p
.FD 0
Bool (*call_dispatcher)(\fIim\fP, \fIlen\fP, \fIdata\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIdata\fP;
.FN
.IP \fIim\fP 1i
Specifies XIM structure address.
.IP \fIlen\fP 1i
Specifies the size of \fIdata\fP buffer.
.IP \fIdata\fP 1i
Specifies the buffer to store data.
.LP
The call_dispatcher must call the dispatcher function, in order of 
their registration. \fIlen\fP and \fIdata\fP are the data passed to 
register_dispatcher.
.LP
The return values are checked at each invocation, and if it finds
True, it immediately return with true for its return value.
.LP
It is depend on the upper layer whether the read data is XIM
Protocol packet unit or not.
The Alternative Entry for this function is:
.sp 6p
.FD 0
Bool _XimCallDispatcher(\fIim\fP, \fIlen\fP, \fIdata\fP)
.br
      XIM \fIim\fP;
.br
      INT16 \fIlen\fP;
.br
      XPointer \fIcall_data\fP;
.FN
.LP
.bp
.NH 1
Sample implementations for the Transport Layer
.XS
\*(SN Sample implementations for the Transport Layer
.XE
.LP
Sample implementations for the transporter using the X connection is
described here.
.LP
.NH 2
X Transport
.XS
\*(SN X Transport 
.XE
.LP
At the beginning of the X Transport connection for the XIM transport
mechanism, two different windows must be created either in an Xlib XIM
or in an IM Server, with which the Xlib and the IM Server exchange the
XIM transports by using the ClientMessage events and Window Properties.
In the following, the window created by the Xlib is referred as the
"client communication window", and on the other hand, the window created
by the IM Server is referred as the "IMS communication window".
.LP
.NH 3
Connection
.XS
\*(SN X Connection
.XE
.LP
In order to establish a connection, a communication window is created.
A ClientMessage in the following event's format is sent to the owner 
window of XIM_SERVER selection, which the IM Server has created.
.LP
Refer to "The Input Method Protocol" for the XIM_SERVER atom.
.LP
.ce
Table 4-1; The ClientMessage sent to the IMS window.
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
.ce
Table 4-2; The ClientMessage sent by IM Server.
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
Table 4-3; The read/write method and the major/minor-transport-version
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
The client must support all methods in Table 4-3.
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
.LP
.NH 3
read/write  
.XS
\*(SN read/write  
.XE
.LP  
The data is transferred via either ClientMessage or Window Property in
the X Window System.
.LP
.NH 4
Format for the data from the Client to the IM Server
.XS
\*(SN Format for the data from the Client to the IM Server
.XE
.LP
.B
ClientMessage
.LP
.RS
If data is sent via ClientMessage event, the format is as follows:
.LP
.ce
Table 4-4; The ClientMessage event's format (first or middle)
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
Atom;message_type;XInternAtom(display, ``_XIM_MOREDATA'', False)
int;format;8  
char;data.b[20];(read/write DATA : 20 byte)  
.sp 6p
_
.TE
.LP
.ce
Table 4-5; The ClientMessage event's format (only or last)
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
.B
Property
.LP
.RS
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
.ce
Table 4-6; The XChangeProperty event's format
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
Table 4-7; The ClientMessage event's format to send Atom of property
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
.NH 4
Format for the data from the IM Server to the Client
.XS
\*(SN Format for the data from the Client to the Client
.XE
.LP
.B
ClientMessage
.LP
.RS
The format of the ClientMessage is as follows:
.LP
.ce
Table 4-8; The ClientMessage event's format (first or middle)
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
.ce
Table 4-9; The ClientMessage event's format (only or last)
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
.RE
.LP
.B
Property
.LP
.RS
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
Table 4-10; The XChangeProperty event's format
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
.ce
Table 4-11; The ClientMessage event's format to send Atom of property 
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
.LP
.NH 3
Closing Connection
.XS
\*(SN Closing Connection
.XE
.LP
If the client disconnect with the IM Server, shutdown function should 
free the communication window properties and etc..
.LP
.NH 1
References
.XS
\*(SN References
.XE
.LP
[1] Masahiko Narita and Hideki Hiura, \fI``The Input Method Protocol''\fP
.LP

