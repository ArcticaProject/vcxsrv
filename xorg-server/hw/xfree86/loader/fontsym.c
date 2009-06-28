/*
 * Copyright (c) 1998-2002 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/fonts/font.h>
#include "sym.h"
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontenc.h>
#ifdef FONTENC_COMPATIBILITY
#include <X11/fonts/fontencc.h>
#endif
#include <X11/fonts/fntfilio.h>
#include <X11/fonts/fntfil.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/fontxlfd.h>
#ifdef FONTCACHE
#define _FONTCACHE_SERVER_
#include "fontcache.h"
#endif

_X_HIDDEN void *fontLookupTab[] = {

    SYMFUNC(TwoByteSwap)
    SYMFUNC(FourByteSwap)
    SYMFUNC(FontCouldBeTerminal)
    SYMFUNC(BufFileRead)
    SYMFUNC(BufFileWrite)
    SYMFUNC(CheckFSFormat)
    SYMFUNC(FontFileOpen)
    SYMFUNC(FontFilePriorityRegisterRenderer)
    SYMFUNC(FontFileRegisterRenderer)
    SYMFUNC(FontParseXLFDName)
    SYMFUNC(FontFileCloseFont)
    SYMFUNC(FontFileOpenBitmap)
    SYMFUNC(FontFileCompleteXLFD)
    SYMFUNC(FontFileCountDashes)
    SYMFUNC(FontFileFindNameInDir)
    SYMFUNC(FontFileClose)
    SYMFUNC(FontComputeInfoAccelerators)
    SYMFUNC(FontDefaultFormat)
    SYMFUNC(NameForAtom)
    SYMFUNC(BitOrderInvert)
    SYMFUNC(FontFileMatchRenderer)
    SYMFUNC(RepadBitmap)
    SYMFUNC(FontEncName)
    SYMFUNC(FontEncRecode)
    SYMFUNC(FontEncFind)
    SYMFUNC(FontMapFind)
    SYMFUNC(FontEncMapFind)
    SYMFUNC(FontEncFromXLFD)
    SYMFUNC(FontEncDirectory)
    SYMFUNC(FontMapReverse)
    SYMFUNC(FontMapReverseFree)
    SYMFUNC(CreateFontRec)
    SYMFUNC(DestroyFontRec)
    SYMFUNC(GetGlyphs)
    SYMFUNC(QueryGlyphExtents)

    SYMVAR(FontFileBitmapSources)

#ifdef FONTENC_COMPATIBILITY
    /* Obsolete backwards compatibility symbols -- fontencc.c */
    SYMFUNC(font_encoding_from_xlfd)
    SYMFUNC(font_encoding_find)
    SYMFUNC(font_encoding_recode)
    SYMFUNC(font_encoding_name)
    SYMFUNC(identifyEncodingFile)
#endif

#ifdef FONTCACHE
    /* fontcache.c */
    SYMFUNC(FontCacheGetSettings)
    SYMFUNC(FontCacheGetStatistics)
    SYMFUNC(FontCacheChangeSettings)
    SYMFUNC(FontCacheOpenCache)
    SYMFUNC(FontCacheCloseCache)
    SYMFUNC(FontCacheSearchEntry)
    SYMFUNC(FontCacheGetEntry)
    SYMFUNC(FontCacheInsertEntry)
    SYMFUNC(FontCacheGetBitmap)
#endif
};
