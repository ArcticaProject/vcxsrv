/*
  Copyright (c) 2002 by Juliusz Chroboczek

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
/* $XFree86: xc/programs/mkfontscale/data.h,v 1.3 2003/04/30 20:39:43 herrb Exp $ */

#ifndef _MKS_DATA_H_
#define _MKS_DATA_H_ 1

/* Order is significant.  For example, some B&H fonts are hinted by
   URW++, and both strings appear in the notice. */

char *notice_foundries[][2] =
    {{"Bigelow", "b&h"},
     {"Adobe", "adobe"},
     {"Bitstream", "bitstream"},
     {"Monotype", "monotype"},
     {"Linotype", "linotype"},
     {"LINOTYPE-HELL", "linotype"},
     {"IBM", "ibm"},
     {"URW", "urw"},
     {"International Typeface Corporation", "itc"},
     {"Tiro Typeworks", "tiro"},
     {"XFree86", "xfree86"},
     {"Xorg", "xorg"},
     {"Microsoft", "microsoft"},
     {"Omega", "omega"},
     {"Font21", "hwan"},
     {"HanYang System", "hanyang"}};

/* This table is partly taken from ttmkfdir by Joerg Pommnitz. */

/* It should not contain useless entries (such as UNKN) nor duplicate
   entries for padding both with spaces and NULs. */

char *vendor_foundries[][2] =
    {{"ADBE", "adobe"},
     {"AGFA", "agfa"},
     {"ALTS", "altsys"},
     {"APPL", "apple"},
     {"ARPH", "arphic"},
     {"ATEC", "alltype"},
     {"B&H", "b&h"},
     {"BITS", "bitstream"},
     {"CANO", "cannon"},
     {"DYNA", "dynalab"},
     {"EPSN", "epson"},
     {"FJ",  "fujitsu"},
     {"IBM", "ibm"},
     {"ITC", "itc"},
     {"IMPR", "impress"},
     {"LARA", "larabiefonts"},
     {"LEAF", "interleaf"},
     {"LETR", "letraset"},
     {"LINO", "linotype"},
     {"MACR", "macromedia"},
     {"MONO", "monotype"},
     {"MS", "microsoft"},
     {"MT", "monotype"},
     {"NEC", "nec"},
     {"PARA", "paratype"},
     {"QMSI", "qms"},
     {"RICO", "ricoh"},
     {"URW", "urw"},
     {"Y&Y", "y&y"}};

#endif /* _MKS_DATA_H_ */
