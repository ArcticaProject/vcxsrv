/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

@@@ 
@@@ Each unique machine architecture needs an entry in this file
@@@ If fc-arch runs and doesn't find a matching entry, it will print
@@@ out the archtecture signature in the error message. Take that
@@@ signature and place it in this file along with a suitable architecture
@@@ name. Architecture names are used to construct file names, so
@@@ use something reasonable and don't include any spaces
@@@
@@@ So far the differences boil down to: endianness, 32 vs 64 bit pointers,
@@@ and on @@@ 32bit ones, whether double is aligned to one word or two words.
@@@ Those result in the 6 formats listed below.
@@@
@@@ ,name (endianness,pointer-size,double-alignment)
@@@ |   ,endian
@@@ |   |    ,FcAlign
@@@ |   |    |  ,char
@@@ |   |    |  |  ,char*
@@@ |   |    |  |  |  ,int
@@@ |   |    |  |  |  |  ,intptr_t
@@@ |   |    |  |  |  |  |  ,Pattern
@@@ |   |    |  |  |  |  |  |  ,EltPtr
@@@ |   |    |  |  |  |  |  |  |  ,Elt *
@@@ |   |    |  |  |  |  |  |  |  |  ,Elt
@@@ |   |    |  |  |  |  |  |  |  |  |  ,ObjPtr
@@@ |   |    |  |  |  |  |  |  |  |  |  |  ,ValueListPtr
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  ,Value
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  ,ValueBinding
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  ,ValueList *
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,StrSet *
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,CharSet
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,CharLeaf **
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,Char16 *
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,Char16
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,CharLeaf
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,Char32
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ,Cache
@@@ |   |    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
le32d4	4321_08_01_04_04_04_10_04_04_08_04_04_0c_04_04_04_10_04_04_02_20_04_20
le32d8	4321_08_01_04_04_04_10_04_04_08_04_04_10_04_04_04_10_04_04_02_20_04_20
le64	4321_08_01_08_04_08_18_08_08_10_04_08_10_04_08_08_18_08_08_02_20_04_38
be32d4	1234_08_01_04_04_04_10_04_04_08_04_04_0c_04_04_04_10_04_04_02_20_04_20
be32d8	1234_08_01_04_04_04_10_04_04_08_04_04_10_04_04_04_10_04_04_02_20_04_20
be64	1234_08_01_08_04_08_18_08_08_10_04_08_10_04_08_08_18_08_08_02_20_04_38
