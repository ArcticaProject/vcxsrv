.\" To print this out, type tbl macros.t ThisFile | troff -ms
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
\&
.TL
\s+3\fBX11R6 Sample Implementation Frame Work\fP\s-3
.sp 2
.AU
Katsuhisa Yano
.AI
TOSHIBA Corporation
.AU
Yoshio Horiuchi
.AI
IBM Japan
.LP
.bp
.br
\&
.sp 15
.ps 9
.nr PS 9
.LP
Copyright \(co 1994 by TOSHIBA Corporation
.br
Copyright \(co 1994 by IBM Corporation
.LP
Permission to use, copy, modify, and distribute this documentation
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission notice appear 
in all copies.
TOSHIBA Corporation and IBM Corporation make no representations about 
the suitability for any purpose of the information in this document.
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
.EH '\fBSample Implementation Frame Work\fP''\fB\*(xV\fP'
.OH '\fBSample Implementation Frame Work\fP''\fB\*(xV\fP'
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Preface 
.XS \*(SN Preface 
.XE 
.LP 
This document proposes to define the structures, methods and their 
signatures that are expected to be common to all locale dependent 
functions within the Xlib sample implementation.  The following 
illustration (Fig.1) is proposed to outline the separating of 
the components within the sample implementation.  
.LP
.\" figure start
.in +1c
\^... 0.237 5.796 5.24 10.14
\^... 0.000i 4.344i 5.003i 0.000i
.nr 00 \n(.u
.nf
.PS 4.344i 5.003i 
.br
.ps 11
\h'1.753i'\v'2.130i'\v'-.13m'\L'-1.000i\(br'\v'.13m'
.sp -1
\h'1.753i'\v'1.130i'\l'1.500i'
.sp -1
\h'3.253i'\v'1.130i'\v'-.13m'\L'1.000i\(br'\v'.13m'
.sp -1
\h'3.253i'\v'2.130i'\l'-1.500i'
.sp -1
\h'1.751i'\v'1.628i'\l'1.499i'
.sp -1
\h'2.500i'\v'1.128i'\v'-.13m'\L'0.500i\(br'\v'.13m'
.sp -1
\h'1.875i'\v'1.344i'\h'-0.0m'\v'0.2m'\s12\fRInput\fP
.sp -1
\h'1.875i'\v'1.532i'\h'-0.0m'\v'0.2m'\s12\fRMethod\fP
.sp -1
\h'2.625i'\v'1.344i'\h'-0.0m'\v'0.2m'\s12\fROutput\fP
.sp -1
\h'2.625i'\v'1.532i'\h'-0.0m'\v'0.2m'\s12\fRMethod\fP
.sp -1
\h'1.938i'\v'1.844i'\h'-0.0m'\v'0.2m'\s12\fR<Locl. Serv. API>\fP
.sp -1
\h'2.000i'\v'2.032i'\h'-0.0m'\v'0.2m'\s12\fRX Locale Object\fP
.sp -1
\h'3.503i'\v'1.630i'\v'-.13m'\L'-0.500i\(br'\v'.13m'
.sp -1
\h'3.503i'\v'1.130i'\l'1.500i'
.sp -1
\h'5.003i'\v'1.130i'\v'-.13m'\L'0.500i\(br'\v'.13m'
.sp -1
\h'5.003i'\v'1.630i'\l'-1.500i'
.sp -1
\h'3.625i'\v'1.344i'\h'-0.0m'\v'0.2m'\s12\fRC Library\fP
.sp -1
\h'4.250i'\v'1.532i'\h'-0.0m'\v'0.2m'\s12\fRANSI impl.\fP
.sp -1
\h'0.003i'\v'1.630i'\v'-.13m'\L'-0.500i\(br'\v'.13m'
.sp -1
\h'0.003i'\v'1.130i'\l'1.500i'
.sp -1
\h'1.503i'\v'1.130i'\v'-.13m'\L'0.500i\(br'\v'.13m'
.sp -1
\h'1.503i'\v'1.630i'\l'-1.500i'
.sp -1
\h'0.125i'\v'1.344i'\h'-0.0m'\v'0.2m'\s12\fRLocale Library\fP
.sp -1
\h'0.438i'\v'1.507i'\h'-0.0m'\v'0.2m'\s12\fRnon-AnSI impl.\fP
.sp -1
\h'3.500i'\v'0.782i'\h'-0.0m'\v'0.2m'\s12\fR<<  ANSI/MSE API >>\fP
.sp -1
\h'4.250i'\v'0.969i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fR(X Contrib)\fP'u/2u'\s12\fR(X Contrib)\fP\h'-\w'\s12\fR(X Contrib)\fP'u/2u'
.sp -1
\h'0.125i'\v'3.094i'\h'-0.0m'\v'0.2m'\s12\fRXLC_XLOCALE\fP
.sp -1
\h'0.125i'\v'3.282i'\h'-0.0m'\v'0.2m'\s12\fR- MB_CUR_MAX\fP
.sp -1
\h'0.125i'\v'3.444i'\h'-0.0m'\v'0.2m'\s12\fR- codeset info\fP
.sp -1
\h'0.125i'\v'3.607i'\h'-0.0m'\v'0.2m'\s12\fRo char/charset\fP
.sp -1
\h'0.125i'\v'3.769i'\h'-0.0m'\v'0.2m'\s12\fRo conv/charset\fP
.sp -1
\h'0.003i'\v'3.880i'\v'-.13m'\L'-1.000i\(br'\v'.13m'
.sp -1
\h'0.003i'\v'2.880i'\l'1.500i'
.sp -1
\h'1.503i'\v'2.880i'\v'-.13m'\L'1.000i\(br'\v'.13m'
.sp -1
\h'1.503i'\v'3.880i'\l'-1.500i'
.sp -1
\h'1.875i'\v'3.094i'\h'-0.0m'\v'0.2m'\s12\fRXLC_FONTSET\fP
.sp -1
\h'1.875i'\v'3.282i'\h'-0.0m'\v'0.2m'\s12\fR- fonset info\fP
.sp -1
\h'1.875i'\v'3.444i'\h'-0.0m'\v'0.2m'\s12\fR- charset info\fP
.sp -1
\h'1.875i'\v'3.607i'\h'-0.0m'\v'0.2m'\s12\fR- font/charset\fP
.sp -1
\h'1.875i'\v'3.769i'\h'-0.0m'\v'0.2m'\s12\fR- XLFD, GL/GR\fP
.sp -1
\h'1.753i'\v'3.880i'\v'-.13m'\L'-1.000i\(br'\v'.13m'
.sp -1
\h'1.753i'\v'2.880i'\l'1.500i'
.sp -1
\h'3.253i'\v'2.880i'\v'-.13m'\L'1.000i\(br'\v'.13m'
.sp -1
\h'3.253i'\v'3.880i'\l'-1.500i'
.sp -1
\h'3.625i'\v'3.444i'\h'-0.0m'\v'0.2m'\s12\fR- codeset info\fP
.sp -1
\h'3.625i'\v'3.607i'\h'-0.0m'\v'0.2m'\s12\fRo char/charset\fP
.sp -1
\h'3.625i'\v'3.769i'\h'-0.0m'\v'0.2m'\s12\fRo conv/charset\fP
.sp -1
\h'3.625i'\v'3.282i'\h'-0.0m'\v'0.2m'\s12\fR- MB_CUR_MAX\fP
.sp -1
\h'3.625i'\v'3.094i'\h'-0.0m'\v'0.2m'\s12\fRlocaledef DB\fP
.sp -1
\h'3.503i'\v'3.880i'\v'-.13m'\L'-1.000i\(br'\v'.13m'
.sp -1
\h'3.503i'\v'2.880i'\l'1.500i'
.sp -1
\h'5.003i'\v'2.880i'\v'-.13m'\L'1.000i\(br'\v'.13m'
.sp -1
\h'5.003i'\v'3.880i'\l'-1.500i'
.sp -1
\h'0.753i'\v'0.250i'\D'l0.000i -0.250i'
.sp -1
\h'0.753i'\l'3.500i'
.sp -1
\h'4.253i'\D'l0.000i 0.250i'
.sp -1
\h'4.253i'\v'0.250i'\l'-3.500i'
.sp -1
\h'2.500i'\v'0.157i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fRApplication\fP'u/2u'\s12\fRApplication\fP\h'-\w'\s12\fRApplication\fP'u/2u'
.sp -1
\v'0.782i'\h'-0.0m'\v'0.2m'\s12\fR<<  ANSI/MSE API >>\fP
.sp -1
\h'0.751i'\v'0.969i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fR(X Contrib)\fP'u/2u'\s12\fR(X Contrib)\fP\h'-\w'\s12\fR(X Contrib)\fP'u/2u'
.sp -1
\h'2.500i'\v'2.128i'\v'-.13m'\L'0.749i\(br'\v'.13m'
.sp -1
\h'2.475i'\v'2.777i'\D'l0.025i 0.100i'
.sp -1
\h'2.525i'\v'2.777i'\D'l-0.025i 0.100i'
.sp -1
\h'2.500i'\v'2.315i'\D'l-0.250i 0.187i'
.sp -1
\h'2.250i'\v'2.502i'\l'-1.124i'
.sp -1
\h'1.126i'\v'2.502i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'1.101i'\v'2.777i'\D'l0.025i 0.100i'
.sp -1
\h'1.151i'\v'2.777i'\D'l-0.025i 0.100i'
.sp -1
\h'2.500i'\v'2.315i'\D'l0.250i 0.187i'
.sp -1
\h'2.750i'\v'2.502i'\l'1.125i'
.sp -1
\h'3.875i'\v'2.502i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'3.850i'\v'2.777i'\D'l0.025i 0.100i'
.sp -1
\h'3.900i'\v'2.777i'\D'l-0.025i 0.100i'
.sp -1
\h'0.376i'\v'1.628i'\v'-.13m'\L'1.249i\(br'\v'.13m'
.sp -1
\h'0.351i'\v'2.777i'\D'l0.025i 0.100i'
.sp -1
\h'0.401i'\v'2.777i'\D'l-0.025i 0.100i'
.sp -1
\h'4.625i'\v'1.628i'\v'-.13m'\L'1.249i\(br'\v'.13m'
.sp -1
\h'4.600i'\v'2.777i'\D'l0.025i 0.100i'
.sp -1
\h'4.650i'\v'2.777i'\D'l-0.025i 0.100i'
.sp -1
\h'2.125i'\v'0.253i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'2.100i'\v'0.528i'\D'l0.025i 0.100i'
.sp -1
\h'2.150i'\v'0.528i'\D'l-0.025i 0.100i'
.sp -1
\h'2.875i'\v'0.253i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'2.850i'\v'0.528i'\D'l0.025i 0.100i'
.sp -1
\h'2.900i'\v'0.528i'\D'l-0.025i 0.100i'
.sp -1
\h'1.126i'\v'0.253i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'1.101i'\v'0.528i'\D'l0.025i 0.100i'
.sp -1
\h'1.151i'\v'0.528i'\D'l-0.025i 0.100i'
.sp -1
\h'3.875i'\v'0.253i'\v'-.13m'\L'0.375i\(br'\v'.13m'
.sp -1
\h'3.850i'\v'0.528i'\D'l0.025i 0.100i'
.sp -1
\h'3.900i'\v'0.528i'\D'l-0.025i 0.100i'
.sp -1
\v'4.002i'\D'l0.125i 0.125i'
.sp -1
\h'0.125i'\v'4.127i'\l'3.000i'
.sp -1
\h'3.125i'\v'4.127i'\D'l0.125i -0.125i'
.sp -1
\h'3.500i'\v'4.002i'\D'l0.125i 0.125i'
.sp -1
\h'3.625i'\v'4.127i'\l'1.250i'
.sp -1
\h'4.875i'\v'4.127i'\D'l0.125i -0.125i'
.sp -1
\h'1.626i'\v'4.344i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fRXLocale Source (X Core)\fP'u/2u'\s12\fRXLocale Source (X Core)\fP\h'-\w'\s12\fRXLocale Source (X Core)\fP'u/2u'
.sp -1
\h'4.250i'\v'4.344i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fRSystem LOcale Source\fP'u/2u'\s12\fRSystem LOcale Source\fP\h'-\w'\s12\fRSystem LOcale Source\fP'u/2u'
.sp -1
\h'2.500i'\v'0.782i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fRXLib API\fP'u/2u'\s12\fRXLib API\fP\h'-\w'\s12\fRXLib API\fP'u/2u'
.sp -1
\h'2.500i'\v'0.969i'\h'-0.0m'\v'0.2m'\h'-\w'\s12\fR(X Core)\fP'u/2u'\s12\fR(X Core)\fP\h'-\w'\s12\fR(X Core)\fP'u/2u'
.sp -1
\h'1.751i'\v'0.782i'\h'-0.0m'\v'0.2m'\s12\fR<<\fP
.sp -1
\h'3.063i'\v'0.782i'\h'-0.0m'\v'0.2m'\s12\fR>>\fP
.sp -1
.sp 1+4.344i
.in -1c
.PE
.if \n(00 .fi
.\" figure end
.LP
.ce
.sp 6p
Fig.1 : Frame Work of Locale Service API Proposal
.LP
Generally speaking, the internationalized portion of Xlib (Locale
Dependent X, LDX) consists of three objects;
locale (LC) , input method (IM) and output method (OM).
The LC provides a set of information that depends on user's language
environment.  The IM manages text inputing, and the OM manages text
drawing.  Both IM and OM highly depend on LC data.
.LP
In X11R5, there are two sample implementations, Ximp and Xsi, for
Xlib internationalization.  But in both implementations, IM and OM
actually refer the private extension of LC.  It breaks coexistence 
of these two sample implementations.  For example, if a user creates 
a new OM for special purpose as a part of Ximp, it will not work with 
Xsi.
.LP
As a solution of this problem, we propose to define the standard
APIs between these three objects, and define the structure that are
common to these objects.
.LP
.NH 1
Objective
.XS \*(SN Objective
.XE 
.LP 
.IP \(bu
Explain the current X11R6 sample implementation
.IP \(bu
Document the common set of locale dependent interfaces
.IP \(bu
Provide more flexible pluggable layer
.LP
.NH 1
Locale Object Binding Functions
.XS \*(SN Locale Object Binding Functions
.XE 
.LP 
This chapter describes functions related locale object binding for
implementing the pluggable layer.
.LP
A locale loader is an entry point for locale object, which
instantiates XLCd object and binds locale methods with specified
locale name. The behavior of loader is implementation dependent.
And, what kind of loaders are available is also implementation
dependent.
.LP
The loader is called in 
.PN _XOpenLC, 
but caller of 
.PN _XOpenLC 
does not need to care about its inside.  For example, if the loader is
implemented with dynamic load functions, and the dynamic module is
expected to be unloaded when the corresponding XLCd is freed,
close methods of XLCdMethods should handle unloading.
.LP
.sp
\fBInitializing a locale loader list\fP
.LP
.FD 0
void _XlcInitLoader()
.FN
The 
.PN _XlcInitLoader
function initializes the locale loader list with vendor specific 
manner.  Each loader is registered with calling
.PN _XlcAddLoader.
The number of loaders and their order in the loader list is
implementation dependent.
.sp
.LP
\fBAdd a loader\fP
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef XLCd (*XLCdLoadProc)(\fIname\fP);
      char \fI*name\fP;

typedef int XlcPosition;
.De
.TS
lw(.5i) lw(2i) lw(2i).
T{
#define
T}	T{
XlcHead
T}	T{
 0
T}
T{
#define
T}	T{
XlcTail
T}	T{
-1
T}
.TE
.LP
.FD 0
Bool _XlcAddLoader(\fIproc, position\fP)
.br
      XLCdLoadProc \fIproc\fP;
.br
      XlcPosition \fIposition\fP;
.FN
.LP
The 
.PN _XlcAddLoader
function registers the specified locale loader ``\fIproc\fP'' to the 
internal loader list.  The position specifies that the loader 
``\fIproc\fP'' should be placed in the top of the loader list(XlcHead) 
or last(XlcTail).
.LP
The object loader is called from the top of the loader list in order,
when calling time.
.sp
.LP
\fBRemove a loader\fP
.LP
.FD 0
void _XlcRemoveLoader(\fIproc\fP)
.br
      XLCdLoadProc \fIproc\fP;
.FN
.LP
The 
.PN _XlcRemoveLoader
function removes the locale loader specified by ``\fIproc\fP'' from the 
loader list.
.LP
Current implementation provides following locale loaders;
.DS
.PN _XlcDefaultLoader
.PN _XlcGenericLoader
.PN _XlcEucLoader
.PN _XlcSjisLoader
.PN _XlcUtfLoader
.PN _XaixOsDynamicLoad
.DE
.LP
.NH 1
Locale Method Interface
.XS \*(SN Locale Method Interface
.XE 
.LP 
This chapter describes the locale method API, which is a set of 
accessible functions from both IM and OM parts.
The locale method API provides the functionalities;  obtaining locale
dependent information, handling charset, converting text, etc.
.LP
As a result of using these APIs instead of accessing vender private
extension of the locale object, we can keep locale, IM and OM
independently each other.
.LP
.NH 1
Locale Method Functions
.XS \*(SN Locale Method Functions
.XE 
.LP 
\fBOpen a Locale Method\fP
.LP
.FD 0
XLCd _XOpenLC(\fIname\fP)
.br
      char \fI*name\fP;
.FN
.LP
The 
.PN _XOpenLC
function opens a locale method which corresponds to the
specified locale name.  
.PN _XOpenLC
calls a locale object loader, which is registered via 
.PN _XlcAddLoader into the internal loader list.  If the called loader 
is valid and successfully opens a locale, 
.PN _XOpenLC
returns the XLCd.  If the loader is invalid or failed to open a locale,
.PN _XOpenLC
calls the next loader.  If all registered loaders cannot open a locale, 
.PN _XOpenLC
returns NULL.
.LP
.FD 0
XLCd _XlcCurrentLC()
.FN
.LP
The 
.PN _XlcCurrentLC
function returns an XLCd that are bound to current locale.
.sp
.LP
\fBClose a Locale Method\fP
.LP
.FD 0
void _XCloseLC(\fIlcd\fP)
.br
      XLCd \fIlcd\fP;
.FN
.LP
The 
.PN _XCloseLC
function close a locale method the specified lcd.
.sp
.LP
\fBObtain Locale Method values\fP
.LP
.FD 0
char * _XGetLCValues(\fIlcd\fP, ...)
.br
      XLCd \fIlcd\fP;
.FN
.LP
The 
.PN _XGetLCValues
function returns NULL if no error occurred; otherwise, it returns the 
name of the first argument that could not be obtained.
The following values are defined as standard arguments. Other values
are implementation dependent.
.LP
.TS H
tab(:);
l l l.
_
.sp 6p
.B
Name:Type:Description
.sp 6p
_
.sp 6p
.TH
.R
XlcNCodeset:char*:codeset part of locale name
XlcNDefaultString:char*:XDefaultString()
XlcNEncodingName:char*:encoding name
XlcNLanguage:char*:language part of locale name
XlcNMbCurMax:int:ANSI C MB_CUR_MAX
XlcNStateDependentEncoding:Bool:is state-dependent encoding or not
XlcNTerritory:char*:territory part of locale name
.sp 6p
_
.TE
.LP
.NH 1
Charset functions
.XS \*(SN 
Charset functions
.XE 
.LP 
The XlcCharSet is an identifier which represents a subset of characters
(character set) in the locale object. 
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef enum {
      XlcUnknown, XlcC0, XlcGL, XlcC1, XlcGR, XlcGLGR, XlcOther
} XlcSide;

typedef struct _XlcCharSetRec *XlcCharSet;

typedef struct {
      char *name;
      XPointer value;
} XlcArg, *XlcArgList;

typedef char* (*XlcGetCSValuesProc)(\fIcharset\fP, \fIargs\fP, \fInum_args\fP);
      XlcCharSet \fIcharset\fP;
      XlcArgList \fIargs\fP;
      int \fInum_args\fP;

typedef struct _XlcCharSetRec {
      char *name;
      XrmQuark xrm_name;
      char *encoding_name;
      XrmQuark xrm_encoding_name;
      XlcSide side;
      int char_size;
      int set_size;
      char *ct_sequence; 
      XlcGetCSValuesProc get_values;
} XlcCharSetRec;
.De
.sp
.LP
\fBGet an XlcCharSet\fP
.LP
.FD 0
XlcCharSet _XlcGetCharSet(\fIname\fP)
.br
      char \fI*name\fP;
.FN
.LP
The 
.PN _XlcGetCharSet
function gets an XlcCharSet which corresponds to the charset name 
specified by ``\fIname\fP''.  
.PN _XlcGetCharSet 
returns NULL, if no XlcCharSet bound to specified ``\fIname\fP''.
.LP
The following character sets are pre-registered.
.LP
.TS H
tab(@);
l l.
_
.sp 6p
.B
Name@Description
.sp 6p
_
.sp 6p
.TH
.R
ISO8859-1:GL@7-bit ASCII graphics (ANSI X3.4-1968),
@Left half of ISO 8859 sets
JISX0201.1976-0:GL@Left half of JIS X0201-1976 (reaffirmed 1984),
@8-Bit Alphanumeric-Katakana Code
.sp
ISO8859-1:GR@Right half of ISO 8859-1, Latin alphabet No. 1
ISO8859-2:GR@Right half of ISO 8859-2, Latin alphabet No. 2
ISO8859-3:GR@Right half of ISO 8859-3, Latin alphabet No. 3
ISO8859-4:GR@Right half of ISO 8859-4, Latin alphabet No. 4
ISO8859-7:GR@Right half of ISO 8859-7, Latin/Greek alphabet
ISO8859-6:GR@Right half of ISO 8859-6, Latin/Arabic alphabet
ISO8859-8:GR@Right half of ISO 8859-8, Latin/Hebrew alphabet
ISO8859-5:GR@Right half of ISO 8859-5, Latin/Cyrillic alphabet
ISO8859-9:GR@Right half of ISO 8859-9, Latin alphabet No. 5
JISX0201.1976-0:GR@Right half of JIS X0201-1976 (reaffirmed 1984),
@8-Bit Alphanumeric-Katakana Code
.sp
GB2312.1980-0:GL@GB2312-1980, China (PRC) Hanzi defined as GL
GB2312.1980-0:GR@GB2312-1980, China (PRC) Hanzi defined as GR
JISX0208.1983-0:GL@JIS X0208-1983, Japanese Graphic Character Set
@defined as GL
JISX0208.1983-0:GR@JIS X0208-1983, Japanese Graphic Character Set
@defined as GR
KSC5601.1987-0:GL@KS C5601-1987, Korean Graphic Character Set
@defined as GL
KSC5601.1987-0:GR@KS C5601-1987, Korean Graphic Character Set
@defined as GR
JISX0212.1990-0:GL@JIS X0212-1990, Japanese Graphic Character Set
@defined as GL
JISX0212.1990-0:GR@JIS X0212-1990, Japanese Graphic Character Set
@defined as GR
.\" CNS11643.1986-0:GL
.\" CNS11643.1986-1:GL
.\" TIS620-0:GR
.sp 6p
_
.TE
.LP
.sp
\fBAdd an XlcCharSet\fP
.LP
.FD 0
Bool _XlcAddCharSet(\fIcharset\fP)
      XlcCharSet \fIcharset\fP;
.FN
.LP
The 
.PN _XlcAddCharSet
function registers XlcCharSet specified by ``\fIcharset\fP''.
.LP
.sp
\fBObtain Character Set values\fP
.LP
.FD 0
char * _XlcGetCSValues(\fIcharset\fP, ...)
.br
      XlcCharSet \fIcharset\fP;
.FN
.LP
The 
.PN _XlcGetCSValues
function returns NULL if no error occurred; 
otherwise, it returns the name of the first argument that could not 
be obtained.  The following values are defined as standard arguments. 
Other values are implementation dependent.
.LP
.TS H
tab(:);
l l l.
_
.sp 6p
.B
Name:Type:Description
.sp 6p
_
.sp 6p
.TH
.R
XlcNName:char*:charset name
XlcNEncodingName:char*:XLFD CharSet Registry and Encoding
XlcNSide:XlcSide:charset side (GL, GR, ...)
XlcNCharSize:int:number of octets per character
XlcNSetSize:int:number of character sets
XlcNControlSequence:char*:control sequence of Compound Text
.sp 6p
_
.TE
.LP
.NH 1
Converter Functions
.XS \*(SN Converter Functions
.XE 
.LP 
We provide a set of the common converter APIs, that are independent 
from both of source and destination text type.
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef struct _XlcConvRec *XlcConv;

typedef void (*XlcCloseConverterProc)(\fIconv\fP);
      XlcConv \fIconv\fP;

typedef int (*XlcConvertProc)(\fIconv\fP, \fIfrom\fP, \fIfrom_left\fP, \fIto\fP, \fIto_left\fP, \fIargs\fP, \fInum_args\fP);
      XlcConv \fIconv\fP;
      XPointer \fI*from\fP;
      int \fI*from_left\fP;
      XPointer \fI*to\fP;
      int \fI*to_left\fP;
      XPointer \fI*args\fP;
      int \fInum_args\fP;

typedef void (*XlcResetConverterProc)(\fIconv\fP);
      XlcConv \fIconv\fP;

typedef struct _XlcConvMethodsRec {
      XlcCloseConverterProc close;
      XlcConvertProc convert;
      XlcResetConverterProc reset;
} XlcConvMethodsRec, *XlcConvMethods;

typedef struct _XlcConvRec {
    XlcConvMethods methods;
    XPointer state;
} XlcConvRec;
.De
.LP
.sp
\fBOpen a converter\fP
.LP
.FD 0
XlcConv _XlcOpenConverter(\fIfrom_lcd\fP, \fIfrom_type\fP, \fIto_lcd\fP, \fIto_type\fP)
.br
      XLCd \fIfrom_lcd\fP;
.br
      char \fI*from_type\fP;
.br
      XLCd \fIto_lcd\fP;
.br
      char \fI*to_type\fP;
.FN
.LP
.PN _XlcOpenConverter 
function opens the converter which converts a text from specified 
``\fIfrom_type\fP'' to specified ``\fIto_type\fP'' encoding.  If the 
function cannot find proper converter or cannot open a corresponding 
converter, it returns NULL.  Otherwise, it returns the conversion 
descriptor.
.LP
The following types are pre-defined. Other types are implementation
dependent.
.LP
.TS H
tab(:);
l l l l.
_
.sp 6p
.B
Name:Type:Description:Arguments
.sp 6p
_
.sp 6p
.TH
.R
XlcNMultiByte:char *:multibyte:-
XlcNWideChar:wchar_t *:wide character:-
XlcNCompoundText:char *:COMPOUND_TEXT:-
XlcNString:char *:STRING:-
XlcNCharSet:char *:per charset:XlcCharSet
XlcNChar:char *:per character:XlcCharSet
.sp 6p
_
.TE
.LP
.sp
\fBClose a converter\fP
.LP
.FD 0
void _XlcCloseConverter(\fIconv\fP)
.br
      XlcConv \fIconv\fP;
.FN
.LP
The 
.PN _XlcCloseConverter
function closes the specified converter ``\fIconv\fP''.
.LP
.sp
\fBCode conversion\fP
.LP
.FD 0
int _XlcConvert(\fIconv\fP, \fIfrom\fP, \fIfrom_left\fP, \fIto\fP, \fIto_left\fP, \fIargs\fP, \fInum_args\fP)
.br
      XlcConv \fIconv\fP;
.br
      XPointer \fI*from\fP;
.br
      int \fI*from_left\fP;
.br
      XPointer \fI*to\fP;
.br
      int \fI*to_left\fP;
.br
      XPointer \fI*args\fP;
.br
      int \fInum_args\fP;
.FN
.LP
The 
.PN _XlcConvert
function converts a sequence of characters from one type, in the array 
specified by ``\fIfrom\fP'', into a sequence of corresponding characters 
in another type, in the array specified by ``\fIto\fP''.  The types are 
those specified in the 
.PN _XlcOpenConverter() 
call that returned the conversion descriptor, ``\fIconv\fP''.
The arguments ``\fIfrom\fP'', ``\fIfrom_left\fP'', ``\fIto\fP'' and 
``\fIto_left\fP'' have the same specification of XPG4 iconv function.
.LP
For state-dependent encodings, the conversion descriptor ``\fIconv\fP''
is placed into its initial shift state by a call for which ``\fIfrom\fP'' 
is a NULL pointer, or for which ``\fIfrom\fP'' points to a null pointer.
.LP
The following 2 converters prepared by locale returns appropriate 
charset (XlcCharSet) in an area pointed by args[0].
.LP
.TS
tab(:);
l l l.
_
.sp 6p
.B
From:To:Description
.sp 6p
_
.sp 6p
.R
XlcNMultiByte:XlcNCharSet:Segmentation (Decomposing)
XlcNWideChar:XlcNCharSet:Segmentation (Decomposing)
.sp 6p
_
.TE
.LP
The conversion, from XlcNMultiByte/XlcNWideChar to XlcNCharSet,
extracts a segment which has same charset encoding characters.
More than one segment cannot be converted in a call.
.LP
.sp
\fBReset a converter\fP
.LP
.FD 0
void _XlcResetConverter(\fIconv\fP)
.br
      XlcConv \fIconv\fP;
.FN
.LP
The 
.PN _XlcResetConverter 
function reset the specified converter ``\fIconv\fP''.
.LP
.sp
\fBRegister a converter\fP
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef XlcConv (*XlcOpenConverterProc)(\fIfrom_lcd\fP, \fIfrom_type\fP, \fIto_lcd\fP, \fIto_type\fP);
      XLCd \fIfrom_lcd\fP;
      char \fI*from_type\fP;
      XLCd \fIto_lcd\fP;
      char \fI*to_type\fP;
.De
.LP
.FD 0
Bool _XlcSetConverter(\fIfrom_lcd\fP, \fIfrom\fP, \fIto_lcd\fP, \fIto\fP, \fIconverter\fP)
.br
      XLCd \fIfrom_lcd\fP;
.br
      char \fI*from\fP;
.br
      XLCd \fIto_lcd\fP;
.br
      char \fI*to\fP;
.br
      XlcOpenConverterProc \fIconverter\fP;
.FN
.LP
The \fBXlcSetConverter\fP function registers a converter which convert 
from ``\fIfrom_type\fP'' to ``\fIto_type\fP'' into the converter list 
(in the specified XLCd).
.LP
.NH 1
X Locale Database functions
.XS \*(SN X Locale Database functions
.XE 
.LP 
X Locale Database contains the subset of user's environment that
depends on language.  The following APIs are provided for accessing
X Locale Database and other locale relative files.
.LP
For more detail about  X Locale Database, please refer
X Locale Database Definition document.
.LP
.sp
\fBGet a resource from database\fP
.LP
.FD 0
void _XlcGetResource(\fIlcd\fP, \fIcategory\fP, \fIclass\fP, \fIvalue\fP, \fIcount\fP)
.br
      XLCd \fIlcd\fP;
.br
      char \fI*category\fP;
.br
      char \fI*class\fP;
.br
      char \fI***value\fP;
.br
      int \fI*count\fP;
.FN
.LP
The 
.PN _XlcGetResource
function obtains a locale dependent data which is associated with the 
locale of specified ``\fIlcd\fP''.
The locale data is provided by system locale or by X Locale Database 
file, and what kind of data is available is implementation dependent.
.LP
The specified ``\fIcategory\fP'' and ``\fIclass\fP'' are used for 
finding out the objective locale data.
.LP
The returned  value is returned in value argument in string list form,
and the returned count shows the number of strings in the value.
.LP
The returned value is owned by locale method, and should not be modified 
or freed by caller.
.LP
.sp
\fBGet a locale relative file name\fP
.LP
.FD 0
char * _XlcFileName(\fIlcd\fP, \fIcategory\fP)
.br
      XLCd \fIlcd\fP;
.br
      char \fI*category\fP;
.FN
.LP
The 
.PN _XlcFileName 
functions returns a file name which is bound to the specified ``\fIlcd\fP'' 
and ``\fIcategory\fP'', as a null-terminated string.  If no file name can 
be found, or there is no readable file for the found file name, 
.PN _XlcFileName
returns NULL.  The returned file name should be freed by caller.
.LP
The rule for searching a file name is implementation dependent.
In current implementation, 
.PN _XlcFileName 
uses ``{category}.dir'' file as mapping table, which has pairs of 
strings, a full locale name and a corresponding file name.
.LP
.NH 1
Utility Functions
.XS \*(SN Utility Functions
.XE 
.LP 
\fBCompare Latin-1 strings\fP
.LP
.FD 0
int _XlcCompareISOLatin1(\fIstr1\fP, \fIstr2\fP)
.br
      char \fI*str1\fP, \fI*str2\fP;
.FN
.FD 0
int _XlcNCompareISOLatin1(\fIstr1\fP, \fIstr2\fP, \fIlen\fP)
.br
      char \fI*str1\fP, \fI*str2\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _XlcCompareIsoLatin1 
function to compares two ISO-8859-1 strings.  Bytes representing ASCII lower 
case letters are converted to upper case before making the comparison.  
The value returned is an integer less than, equal to, or greater than 
zero, depending on whether ``\fIstr1\fP'' is lexicographicly less than, 
equal to, or greater than ``\fIstr2\fP''.
.LP 
The 
.PN _XlcNCompareIsoLatin1
function is identical to 
.PN _XlcCompareISOLatin1,
except that at most ``\fIlen\fP'' bytes are compared.
.LP
.sp
\fBResource Utility\fP
.LP
.FD 0
int XlcNumber(\fIarray\fP)
      ArrayType \fIarray\fP;
.FN
.LP
Similar to XtNumber.
.LP
.FD 0
void _XlcCopyFromArg(\fIsrc\fP, \fIdst\fP, \fIsize\fP)
.br
      char \fI*src\fP;
.br
      char \fI*dst\fP;
.br
      int \fIsize\fP;
.FN
.FD 0
void _XlcCopyToArg(\fIsrc\fP, \fIdst\fP, \fIsize\fP)
.br
      char \fI*src\fP;
.br
      char \fI**dst\fP;
.br
      int \fIsize\fP;
.FN
.LP
Similar to 
.PN _XtCopyFromArg 
and 
.PN _XtCopyToArg.
.LP
.FD 0
void _XlcCountVaList(\fIvar\fP, \fIcount_ret\fP)
.br
      va_list \fIvar\fP;
.br
      int \fI*count_ret\fP;
.FN
.LP
Similar to 
.PN _XtCountVaList.
.LP
.FD 0
void _XlcVaToArgList(\fIvar\fP, \fIcount\fP, \fIargs_ret\fP)
.br
      va_list \fIvar\fP;
.br
      int \fIcount\fP;
.br
      XlcArgList \fI*args_ret\fP;
.FN
.LP
Similar to 
.PN _XtVaToArgList.
.LP
.Ds 0
.TA .5i 2.5i
.ta .5i 2.5i
typedef struct _XlcResource {
      char *name;
      XrmQuark xrm_name;
      int size;
      int offset;
      unsigned long mask;
} XlcResource, *XlcResourceList;
.De
.LP
.TS
lw(.5i) lw(2i) lw(2i).
T{
#define
T}	T{
XlcCreateMask
T}	T{
(1L<<0)
T}
T{
#define
T}	T{
XlcDefaultMask
T}	T{
(1L<<1)
T}
T{
#define
T}	T{
XlcGetMask
T}	T{
(1L<<2)
T}
T{
#define
T}	T{
XlcSetMask
T}	T{
(1L<<3)
T}
T{
#define
T}	T{
XlcIgnoreMask
T}	T{
(1L<<4)
T}
.TE
.LP
.FD 0
void _XlcCompileResourceList(\fIresources\fP, \fInum_resources\fP)
.br
      XlcResourceList \fIresources\fP;
.br
      int \fInum_resources\fP;
.FN
.LP
Similar to 
.PN _XtCompileResourceList.
.LP
.FD 0
char * _XlcGetValues(\fIbase\fP, \fIresources\fP, \fInum_resources\fP, \fIargs\fP, \fInum_args\fP, \fImask\fP)
.br
      XPointer \fIbase\fP;
.br
      XlcResourceList \fIresources\fP;
.br
      int \fInum_resources\fP;
.br
      XlcArgList \fIargs\fP;
.br
      int \fInum_args\fP;
.br
      unsigned long \fImask\fP;
.FN
.LP
Similar to XtGetSubvalues.
.LP
.FD 0
char * _XlcSetValues(\fIbase\fP, \fIresources\fP, \fInum_resources\fP, \fIargs\fP, \fInum_args\fP, \fImask\fP)
.br
      XPointer \fIbase\fP;
.br
      XlcResourceList \fIresources\fP;
.br
      int \fInum_resources\fP;
.br
      XlcArgList \fIargs\fP;
.br
      int \fInum_args\fP;
.br
      unsigned long \fImask\fP;
.FN
.LP
Similar to XtSetSubvalues.
.LP
.sp
\fBANSI C Compatible Functions\fP
.LP
The following are ANSI C/MSE Compatible Functions for non-ANSI C environment.
.LP
.FD 0
int _Xmblen(\fIstr\fP, \fIlen\fP)
.br
      char \fI*str\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xmblen 
function returns the number of characters pointed to by ``\fIstr\fP''.  
Only ``\fIlen\fP'' bytes in ``\fIstr\fP'' are used in determining the 
character count returned.  ``\fIStr\fP'' may point at characters from 
any valid codeset in the current locale.
.LP 
The call 
.PN _Xmblen
is equivalent to
.RS
_Xmbtowc(_Xmbtowc((\fIwchar_t*\fP)NULL, \fIstr\fP, \fIlen\fP))
.RE
.LP
.FD 0
int _Xmbtowc(\fIwstr\fP, \fIstr\fP, \fIlen\fP)
.br
      wchar_t \fI*wstr\fP;
.br
      char \fI*str\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xmbtowc
function converts the character(s) pointed to by ``\fIstr\fP'' 
to their wide character representation(s) pointed to by ``\fIwstr\fP''.  
``\fILen\fP'' is the number of bytes in ``\fIstr\fP'' to be converted.  
The return value is the number of characters converted.
.LP 
The call 
.PN _Xmbtowc
is equivalent to
.RS
_Xlcmbtowc((XLCd)NULL, \fIwstr\fP, \fIstr\fP, \fIlen\fP)
.RE
.LP
.FD 0
int _Xlcmbtowc(\fIlcd\fP, \fIwstr\fP, \fIstr\fP, \fIlen\fP)
.br
      XLCd \fIlcd\fP;
.br
      wchar_t \fI*wstr\fP;
.br
      char \fI*str\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xlcmbtowc
function is identical to 
.PN _Xmbtowc, 
except that it requires the ``\fIlcd\fP'' argument.  If ``\fIlcd\fP'' 
is (XLCd) NULL, 
.PN _Xlcmbtowc, 
calls 
.PN _XlcCurrentLC 
to determine the current locale.
.LP 
.FD 0
int _Xwctomb(\fIstr\fP, \fIwc\fP)
.br
      char \fI*str\fP;
.br
      wchar_t \fIwc\fP;
.FN
.LP
The 
.PN _Xwctomb 
function converts a single wide character pointed to by ``\fIwc\fP'' to 
its multibyte representation pointed to by ``\fIstr\fP''.  
On success, the return value is 1.
.LP 
The call 
.PN _Xwctomb
is equivalent to
.RS
_Xlcwctomb((XLCd)NULL, \fIstr\fP, \fIwstr\fP)
.RE
.LP
.FD 0
int _Xlcwctomb(\fIlcd\fP, \fIstr\fP, \fIwc\fP)
.br
      XLCd \fIlcd\fP;
.br
      char \fI*str\fP;
.br
      wchar_t \fIwc\fP;
.FN
.LP
The 
.PN _Xlcwctomb
function is identical to _Xwctomb, except that it requires the 
``\fIlcd\fP'' argument.  If ``\fIlcd\fP'' is (XLCd) NULL, 
.PN _Xlcwctomb, 
calls 
.PN _XlcCurrentLC 
to determine the current locale.
.LP
.FD 0
int _Xmbstowcs(\fIwstr\fP, \fIstr\fP, \fIlen\fP)
.br
      wchar_t \fI*wstr\fP;
.br
      char \fI*str\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xmbstowcs
function converts the NULL-terminated string pointed to by ``\fIstr\fP'' 
to its wide character string representation pointed to by ``\fIwstr\fP''.
``\fILen\fP'' is the number of characters in ``\fIstr\fP'' to be converted.
.LP
The call 
.PN _Xmbstowcs
is equivalent to
.RS
_Xlcmbstowcs((XLCd)NULL, \fIwstr\fP, \fIstr\fP, \fIlen\fP)
.RE
.LP
.FD 0
int _Xlcmbstowcs(\fIlcd\fP, \fIwstr\fP, \fIstr\fP, \fIlen\fP)
.br
      XLCd \fIlcd\fP;
.br
      wchar_t \fI*wstr\fP;
.br
      char \fI*str\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xlcmbstowcs 
function is identical to _Xmbstowcs, except that it requires the 
``\fIlcd\fP'' argument.  If ``\fIlcd\fP'' is (XLCd) NULL, 
.PN _Xlcmbstowcs, 
calls 
.PN _XlcCurrentLC
to determine the current locale.
.LP
.FD 0
int _Xwcstombs(\fIstr\fP, \fIwstr\fP, \fIlen\fP)
.br
      char \fI*str\fP;
.br
      wchar_t \fI*wstr\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xwcstombs 
function converts the (wchar_t) NULL terminated wide character string 
pointed to by ``\fIwstr\fP'' to the NULL terminated multibyte string 
pointed to by ``\fIstr\fP''.
.LP 
The call 
.PN _Xwcstombs 
is equivalent to
.RS
_Xlcwcstombs((XLCd)NULL, \fIstr\fP, \fIwstr\fP, \fIlen\fP)
.RE
.LP
.FD 0
int _Xlcwcstombs(\fIlcd\fP, \fIstr\fP, \fIwstr\fP, \fIlen\fP)
.br
      XLCd \fIlcd\fP;
.br
      char \fI*str\fP;
.br
      wchar_t \fI*wstr\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xlcwcstombs 
function is identical to _Xwcstombs, except that it requires the 
``\fIlcd\fP'' argument.  If ``\fIlcd\fP'' is (XLCd) NULL, 
.PN _Xlcwcstombs, 
calls 
.PN _XlcCurrentLC 
to determine the current locale.
.LP
.FD 0
int _Xwcslen(\fIwstr\fP)
.br
      wchar_t \fI*wstr\fP;
.FN
.LP
The 
.PN _Xwcslen 
function returns the count of wide characters in the (wchar_t) NULL 
terminated wide character string pointed to by ``\fIwstr\fP''.
.LP
.FD 0
wchar_t * _Xwcscpy(\fIwstr1\fP, \fIwstr2\fP)
.br
      wchar_t \fI*wstr1\fP, \fI*wstr2\fP;
.FN
.FD 0
wchar_t * _Xwcsncpy(\fIwstr1\fP, \fIwstr2\fP, \fIlen\fP)
.br
      wchar_t \fI*wstr1\fP, \fI*wstr2\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xwcscpy 
function copies the (wchar_t) NULL terminated wide character string 
pointed to by ``\fIwstr2\fP'' to the object pointed at by ``\fIwstr1\fP''.
``\fIWstr1\fP'' is (wchar_t) NULL terminated.  The return value is a 
pointer to ``\fIwstr1\fP''.
.LP
The 
.PN _Xwcsncpy
function is identical to 
.PN _Xwcscpy, 
except that it copies ``\fIlen\fP'' wide characters from the object 
pointed to by ``\fIwstr2\fP'' to the object pointed to ``\fIwstr1\fP''.
.LP
.FD 0
int _Xwcscmp(\fIwstr1\fP, \fIwstr2\fP)
.br
      wchar_t \fI*wstr1\fP, \fI*wstr2\fP;
.FN
.FD 0
int _Xwcsncmp(\fIwstr1\fP, \fIwstr2\fP, \fIlen\fP)
.br
      wchar_t \fI*wstr1\fP, \fI*wstr2\fP;
.br
      int \fIlen\fP;
.FN
.LP
The 
.PN _Xwcscmp 
function  compares two (wchar_t) NULL terminated wide character strings.  
The value returned is an integer less than, equal to, or greater than zero, 
depending on whether ``\fIwstr1\fP'' is lexicographicly less then, equal to,
or greater than ``\fIstr2\fP''.
.LP 
The 
.PN _Xwcsncmp 
function is identical to 
.PN _XlcCompareISOLatin1, 
except that at most ``\fIlen\fP'' wide characters are compared.
.sp
.\" --------------------------------------------------------------------
.\" .LP
.\" \fBLocale Method Internal Functions\fP
.\" .LP
.\" .FD 0
.\" XlcCharSet _XlcCreateDefaultCharSet(\fIname\fP, \fIct_sequence\fP)
.\" .br
.\"       char \fI*name\fP;
.\" .br
.\"       char \fI*ct_sequence\fP;
.\" .FN
.\" .FD 0
.\" Bool _XlcParseCharSet(\fIcharset\fP)
.\" .br
.\"       XlcCharSet \fIcharset\fP;
.\" .FN
.\" .FD 0
.\" void _XlcGetLocaleDataBase(\fIlcd\fP, \fIcategory\fP, \fIname\fP, \fIvalue\fP, \fIcount\fP)
.\" .br
.\"       XLCd \fIlcd\fP;
.\" .br
.\"       char \fI*category\fP;
.\" .br
.\"       char \fI*name\fP;
.\" .br
.\"       char \fI***value\fP;
.\" .br
.\"       int \fI*count\fP;
.\" .FN
.\" .FD 0
.\" void _XlcDestroyLocaleDataBase(\fIlcd\fP)
.\" .br
.\"       XLCd \fIlcd\fP;
.\" .FN
.\" .FD 0
.\" XPointer _XlcCreateLocaleDataBase(\fIlcd\fP)
.\" .br
.\"       XLCd \fIlcd\fP;
.\" .FN
.\" .LP
.\" .sp
.\" \fBObtain an locale database path\fP
.\" .LP
.\" .FD 0
.\" int _XlcResolveI18NPath(\fIdir\fP)
.\" .br
.\"       char \fI*dir\fP;
.\" .FN
.\" .LP
.\" The 
.\" .PN _XlcResolveI18NPath 
.\" function returns path name list that is related to X Locale Database.
.\" The obtained path is stored into the array which is pointed by
.\" specified ``\fIdir\fP''.  The path consists of directory paths which 
.\" are separated with colon.
.\" If the environment variable XLOCALEDIR is specified, the path
.\" contains its contents.
.\" .LP
.\" The default path of X Locale Database is implementation dependent.
.\" In current implementation, it's determined in build time.
.\" .LP
.\" .PN _XlcResolveI18NPath 
.\" does not check overflow of the array to which the ``\fIdir\fP'' 
.\" parameter points.  Caller should provide enough buffer to store this 
.\" string.
.\" .LP
.\" .sp
.\" \fBObtain a full locale name\fP
.\" .LP
.\" .FD 0
.\" int _XlcResolveLocaleName(\fIlc_name\fP, \fIfull_name\fP, \fIlanguage\fP, \fIterritory\fP, \fIcodeset\fP)
.\" .br
.\"       char \fI*lc_name\fP;
.\" .br
.\"       char \fI*full_name\fP;
.\" .br
.\"       char \fI*language\fP;
.\" .br
.\"       char \fI*territory\fP;
.\" .br
.\"       char \fI*codeset\fP;
.\" .FN
.\" .LP
.\" The 
.\" .PN _XlcResolveLocaleName 
.\" function returns a full locale name.
.\" The obtained full locale name is stored into the array which is
.\" pointed by specified ``\fIfull_name\fP''.
.\" The language, territory and codeset part of the full locale name
.\" are copied to the return arguments, ``\fIlanguage\fP'', 
.\" ``\fIterritory\fP'' and ``\fIcodeset\fP'', respectively.
.\" NULL can be specified for these arguments.
.\" .LP
.\" The rule for mapping from locale name to full locale name is
.\" implementation dependent.
.\" .LP
.\" .PN _XlcResolveLocaleName 
.\" does not check overflow of the array to which
.\" ``\fIfull_name\fP'', ``\fIlanguage\fP'', ``\fIterritory\fP'' and 
.\" ``\fIcodeset\fP'' parameter point.
.\" Caller should provide enough buffer to store those string.
.\" .LP
.\" In current implementation, 
.\" .PN _XlcResolveLocaleName 
.\" uses locale.alias file as mapping table, which has pairs of strings, 
.\" a locale name and a full locale name.
.\" .LP
.\" .FD 0
.\" int _XlcResolveDBName(\fIlc_name\fP, \fIfile_name\fP)
.\" .br
.\"       char \fI*lc_name\fP;
.\" .br
.\"       char \fI*file_name\fP;
.\" .FN
.\" .FD 0 
.\" XLCd _XlcCreateLC(\fIname\fP, \fImethods\fP)
.\" .br
.\"       char \fI*name\fP;
.\" .br
.\"       XLCdMethods \fImethods\fP;
.\" .FN
.\" .FD 0
.\" void _XlcDestroyLC(\fIlcd\fP)
.\" .br
.\"       XLCd \fIlcd\fP;
.\" .FN
.\" .LP
.\" 

