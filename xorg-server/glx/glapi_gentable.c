/* DO NOT EDIT - This file generated automatically by gl_gen_table.py (from Mesa) script */

/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * (C) Copyright IBM Corporation 2004, 2005
 * (C) Copyright Apple Inc 2011
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL, IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "glapi.h"
#include "glapitable.h"

#include "os.h"

static void
__glapi_gentable_NoOp(void) {
    const char *fstr = "Unknown";

#ifdef HAVE_BACKTRACE
    void *frames[2];

    if(backtrace(frames, 2) == 2) {
        Dl_info info;
        dladdr(frames[1], &info);
        if(info.dli_sname)
            fstr = info.dli_sname;
    }
#endif

    LogMessage(X_ERROR, "GLX: Call to unimplemented API: %s\n", fstr);
}

static void
__glapi_gentable_set_remaining_noop(struct _glapi_table *disp) {
    GLuint entries = _glapi_get_dispatch_table_size();
    void **dispatch = (void **) disp;
    int i;

    /* ISO C is annoying sometimes */
    union {_glapi_proc p; void *v;} p;
    p.p = __glapi_gentable_NoOp;

    for(i=0; i < entries; i++)
        if(dispatch[i] == NULL)
            dispatch[i] = p.v;
}

struct _glapi_table *
_glapi_create_table_from_handle(void *handle, const char *symbol_prefix) {
    struct _glapi_table *disp = calloc(1, sizeof(struct _glapi_table));
    char symboln[512];
    void ** procp;

    if(!disp)
        return NULL;

    if(symbol_prefix == NULL)
        symbol_prefix = "";


    if(!disp->NewList) {
        snprintf(symboln, sizeof(symboln), "%sNewList", symbol_prefix);
        procp = (void **) &disp->NewList;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EndList) {
        snprintf(symboln, sizeof(symboln), "%sEndList", symbol_prefix);
        procp = (void **) &disp->EndList;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CallList) {
        snprintf(symboln, sizeof(symboln), "%sCallList", symbol_prefix);
        procp = (void **) &disp->CallList;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CallLists) {
        snprintf(symboln, sizeof(symboln), "%sCallLists", symbol_prefix);
        procp = (void **) &disp->CallLists;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteLists) {
        snprintf(symboln, sizeof(symboln), "%sDeleteLists", symbol_prefix);
        procp = (void **) &disp->DeleteLists;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenLists) {
        snprintf(symboln, sizeof(symboln), "%sGenLists", symbol_prefix);
        procp = (void **) &disp->GenLists;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ListBase) {
        snprintf(symboln, sizeof(symboln), "%sListBase", symbol_prefix);
        procp = (void **) &disp->ListBase;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Begin) {
        snprintf(symboln, sizeof(symboln), "%sBegin", symbol_prefix);
        procp = (void **) &disp->Begin;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Bitmap) {
        snprintf(symboln, sizeof(symboln), "%sBitmap", symbol_prefix);
        procp = (void **) &disp->Bitmap;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3b) {
        snprintf(symboln, sizeof(symboln), "%sColor3b", symbol_prefix);
        procp = (void **) &disp->Color3b;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3bv) {
        snprintf(symboln, sizeof(symboln), "%sColor3bv", symbol_prefix);
        procp = (void **) &disp->Color3bv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3d) {
        snprintf(symboln, sizeof(symboln), "%sColor3d", symbol_prefix);
        procp = (void **) &disp->Color3d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3dv) {
        snprintf(symboln, sizeof(symboln), "%sColor3dv", symbol_prefix);
        procp = (void **) &disp->Color3dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3f) {
        snprintf(symboln, sizeof(symboln), "%sColor3f", symbol_prefix);
        procp = (void **) &disp->Color3f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3fv) {
        snprintf(symboln, sizeof(symboln), "%sColor3fv", symbol_prefix);
        procp = (void **) &disp->Color3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3i) {
        snprintf(symboln, sizeof(symboln), "%sColor3i", symbol_prefix);
        procp = (void **) &disp->Color3i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3iv) {
        snprintf(symboln, sizeof(symboln), "%sColor3iv", symbol_prefix);
        procp = (void **) &disp->Color3iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3s) {
        snprintf(symboln, sizeof(symboln), "%sColor3s", symbol_prefix);
        procp = (void **) &disp->Color3s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3sv) {
        snprintf(symboln, sizeof(symboln), "%sColor3sv", symbol_prefix);
        procp = (void **) &disp->Color3sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3ub) {
        snprintf(symboln, sizeof(symboln), "%sColor3ub", symbol_prefix);
        procp = (void **) &disp->Color3ub;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3ubv) {
        snprintf(symboln, sizeof(symboln), "%sColor3ubv", symbol_prefix);
        procp = (void **) &disp->Color3ubv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3ui) {
        snprintf(symboln, sizeof(symboln), "%sColor3ui", symbol_prefix);
        procp = (void **) &disp->Color3ui;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3uiv) {
        snprintf(symboln, sizeof(symboln), "%sColor3uiv", symbol_prefix);
        procp = (void **) &disp->Color3uiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3us) {
        snprintf(symboln, sizeof(symboln), "%sColor3us", symbol_prefix);
        procp = (void **) &disp->Color3us;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color3usv) {
        snprintf(symboln, sizeof(symboln), "%sColor3usv", symbol_prefix);
        procp = (void **) &disp->Color3usv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4b) {
        snprintf(symboln, sizeof(symboln), "%sColor4b", symbol_prefix);
        procp = (void **) &disp->Color4b;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4bv) {
        snprintf(symboln, sizeof(symboln), "%sColor4bv", symbol_prefix);
        procp = (void **) &disp->Color4bv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4d) {
        snprintf(symboln, sizeof(symboln), "%sColor4d", symbol_prefix);
        procp = (void **) &disp->Color4d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4dv) {
        snprintf(symboln, sizeof(symboln), "%sColor4dv", symbol_prefix);
        procp = (void **) &disp->Color4dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4f) {
        snprintf(symboln, sizeof(symboln), "%sColor4f", symbol_prefix);
        procp = (void **) &disp->Color4f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4fv) {
        snprintf(symboln, sizeof(symboln), "%sColor4fv", symbol_prefix);
        procp = (void **) &disp->Color4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4i) {
        snprintf(symboln, sizeof(symboln), "%sColor4i", symbol_prefix);
        procp = (void **) &disp->Color4i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4iv) {
        snprintf(symboln, sizeof(symboln), "%sColor4iv", symbol_prefix);
        procp = (void **) &disp->Color4iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4s) {
        snprintf(symboln, sizeof(symboln), "%sColor4s", symbol_prefix);
        procp = (void **) &disp->Color4s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4sv) {
        snprintf(symboln, sizeof(symboln), "%sColor4sv", symbol_prefix);
        procp = (void **) &disp->Color4sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4ub) {
        snprintf(symboln, sizeof(symboln), "%sColor4ub", symbol_prefix);
        procp = (void **) &disp->Color4ub;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4ubv) {
        snprintf(symboln, sizeof(symboln), "%sColor4ubv", symbol_prefix);
        procp = (void **) &disp->Color4ubv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4ui) {
        snprintf(symboln, sizeof(symboln), "%sColor4ui", symbol_prefix);
        procp = (void **) &disp->Color4ui;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4uiv) {
        snprintf(symboln, sizeof(symboln), "%sColor4uiv", symbol_prefix);
        procp = (void **) &disp->Color4uiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4us) {
        snprintf(symboln, sizeof(symboln), "%sColor4us", symbol_prefix);
        procp = (void **) &disp->Color4us;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Color4usv) {
        snprintf(symboln, sizeof(symboln), "%sColor4usv", symbol_prefix);
        procp = (void **) &disp->Color4usv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EdgeFlag) {
        snprintf(symboln, sizeof(symboln), "%sEdgeFlag", symbol_prefix);
        procp = (void **) &disp->EdgeFlag;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EdgeFlagv) {
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagv", symbol_prefix);
        procp = (void **) &disp->EdgeFlagv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->End) {
        snprintf(symboln, sizeof(symboln), "%sEnd", symbol_prefix);
        procp = (void **) &disp->End;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexd) {
        snprintf(symboln, sizeof(symboln), "%sIndexd", symbol_prefix);
        procp = (void **) &disp->Indexd;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexdv) {
        snprintf(symboln, sizeof(symboln), "%sIndexdv", symbol_prefix);
        procp = (void **) &disp->Indexdv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexf) {
        snprintf(symboln, sizeof(symboln), "%sIndexf", symbol_prefix);
        procp = (void **) &disp->Indexf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexfv) {
        snprintf(symboln, sizeof(symboln), "%sIndexfv", symbol_prefix);
        procp = (void **) &disp->Indexfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexi) {
        snprintf(symboln, sizeof(symboln), "%sIndexi", symbol_prefix);
        procp = (void **) &disp->Indexi;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexiv) {
        snprintf(symboln, sizeof(symboln), "%sIndexiv", symbol_prefix);
        procp = (void **) &disp->Indexiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexs) {
        snprintf(symboln, sizeof(symboln), "%sIndexs", symbol_prefix);
        procp = (void **) &disp->Indexs;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexsv) {
        snprintf(symboln, sizeof(symboln), "%sIndexsv", symbol_prefix);
        procp = (void **) &disp->Indexsv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3b) {
        snprintf(symboln, sizeof(symboln), "%sNormal3b", symbol_prefix);
        procp = (void **) &disp->Normal3b;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3bv) {
        snprintf(symboln, sizeof(symboln), "%sNormal3bv", symbol_prefix);
        procp = (void **) &disp->Normal3bv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3d) {
        snprintf(symboln, sizeof(symboln), "%sNormal3d", symbol_prefix);
        procp = (void **) &disp->Normal3d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3dv) {
        snprintf(symboln, sizeof(symboln), "%sNormal3dv", symbol_prefix);
        procp = (void **) &disp->Normal3dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3f) {
        snprintf(symboln, sizeof(symboln), "%sNormal3f", symbol_prefix);
        procp = (void **) &disp->Normal3f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3fv) {
        snprintf(symboln, sizeof(symboln), "%sNormal3fv", symbol_prefix);
        procp = (void **) &disp->Normal3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3i) {
        snprintf(symboln, sizeof(symboln), "%sNormal3i", symbol_prefix);
        procp = (void **) &disp->Normal3i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3iv) {
        snprintf(symboln, sizeof(symboln), "%sNormal3iv", symbol_prefix);
        procp = (void **) &disp->Normal3iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3s) {
        snprintf(symboln, sizeof(symboln), "%sNormal3s", symbol_prefix);
        procp = (void **) &disp->Normal3s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Normal3sv) {
        snprintf(symboln, sizeof(symboln), "%sNormal3sv", symbol_prefix);
        procp = (void **) &disp->Normal3sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2d) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2d", symbol_prefix);
        procp = (void **) &disp->RasterPos2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2dv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2dv", symbol_prefix);
        procp = (void **) &disp->RasterPos2dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2f) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2f", symbol_prefix);
        procp = (void **) &disp->RasterPos2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2fv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2fv", symbol_prefix);
        procp = (void **) &disp->RasterPos2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2i) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2i", symbol_prefix);
        procp = (void **) &disp->RasterPos2i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2iv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2iv", symbol_prefix);
        procp = (void **) &disp->RasterPos2iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2s) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2s", symbol_prefix);
        procp = (void **) &disp->RasterPos2s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos2sv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos2sv", symbol_prefix);
        procp = (void **) &disp->RasterPos2sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3d) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3d", symbol_prefix);
        procp = (void **) &disp->RasterPos3d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3dv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3dv", symbol_prefix);
        procp = (void **) &disp->RasterPos3dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3f) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3f", symbol_prefix);
        procp = (void **) &disp->RasterPos3f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3fv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3fv", symbol_prefix);
        procp = (void **) &disp->RasterPos3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3i) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3i", symbol_prefix);
        procp = (void **) &disp->RasterPos3i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3iv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3iv", symbol_prefix);
        procp = (void **) &disp->RasterPos3iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3s) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3s", symbol_prefix);
        procp = (void **) &disp->RasterPos3s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos3sv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos3sv", symbol_prefix);
        procp = (void **) &disp->RasterPos3sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4d) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4d", symbol_prefix);
        procp = (void **) &disp->RasterPos4d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4dv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4dv", symbol_prefix);
        procp = (void **) &disp->RasterPos4dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4f) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4f", symbol_prefix);
        procp = (void **) &disp->RasterPos4f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4fv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4fv", symbol_prefix);
        procp = (void **) &disp->RasterPos4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4i) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4i", symbol_prefix);
        procp = (void **) &disp->RasterPos4i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4iv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4iv", symbol_prefix);
        procp = (void **) &disp->RasterPos4iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4s) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4s", symbol_prefix);
        procp = (void **) &disp->RasterPos4s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RasterPos4sv) {
        snprintf(symboln, sizeof(symboln), "%sRasterPos4sv", symbol_prefix);
        procp = (void **) &disp->RasterPos4sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectd) {
        snprintf(symboln, sizeof(symboln), "%sRectd", symbol_prefix);
        procp = (void **) &disp->Rectd;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectdv) {
        snprintf(symboln, sizeof(symboln), "%sRectdv", symbol_prefix);
        procp = (void **) &disp->Rectdv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectf) {
        snprintf(symboln, sizeof(symboln), "%sRectf", symbol_prefix);
        procp = (void **) &disp->Rectf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectfv) {
        snprintf(symboln, sizeof(symboln), "%sRectfv", symbol_prefix);
        procp = (void **) &disp->Rectfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Recti) {
        snprintf(symboln, sizeof(symboln), "%sRecti", symbol_prefix);
        procp = (void **) &disp->Recti;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectiv) {
        snprintf(symboln, sizeof(symboln), "%sRectiv", symbol_prefix);
        procp = (void **) &disp->Rectiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rects) {
        snprintf(symboln, sizeof(symboln), "%sRects", symbol_prefix);
        procp = (void **) &disp->Rects;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rectsv) {
        snprintf(symboln, sizeof(symboln), "%sRectsv", symbol_prefix);
        procp = (void **) &disp->Rectsv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1d) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1d", symbol_prefix);
        procp = (void **) &disp->TexCoord1d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1dv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1dv", symbol_prefix);
        procp = (void **) &disp->TexCoord1dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1f) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1f", symbol_prefix);
        procp = (void **) &disp->TexCoord1f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1fv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1fv", symbol_prefix);
        procp = (void **) &disp->TexCoord1fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1i) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1i", symbol_prefix);
        procp = (void **) &disp->TexCoord1i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1iv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1iv", symbol_prefix);
        procp = (void **) &disp->TexCoord1iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1s) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1s", symbol_prefix);
        procp = (void **) &disp->TexCoord1s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord1sv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord1sv", symbol_prefix);
        procp = (void **) &disp->TexCoord1sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2d) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2d", symbol_prefix);
        procp = (void **) &disp->TexCoord2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2dv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2dv", symbol_prefix);
        procp = (void **) &disp->TexCoord2dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2f) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2f", symbol_prefix);
        procp = (void **) &disp->TexCoord2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2fv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2fv", symbol_prefix);
        procp = (void **) &disp->TexCoord2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2i) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2i", symbol_prefix);
        procp = (void **) &disp->TexCoord2i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2iv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2iv", symbol_prefix);
        procp = (void **) &disp->TexCoord2iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2s) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2s", symbol_prefix);
        procp = (void **) &disp->TexCoord2s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord2sv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord2sv", symbol_prefix);
        procp = (void **) &disp->TexCoord2sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3d) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3d", symbol_prefix);
        procp = (void **) &disp->TexCoord3d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3dv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3dv", symbol_prefix);
        procp = (void **) &disp->TexCoord3dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3f) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3f", symbol_prefix);
        procp = (void **) &disp->TexCoord3f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3fv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3fv", symbol_prefix);
        procp = (void **) &disp->TexCoord3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3i) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3i", symbol_prefix);
        procp = (void **) &disp->TexCoord3i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3iv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3iv", symbol_prefix);
        procp = (void **) &disp->TexCoord3iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3s) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3s", symbol_prefix);
        procp = (void **) &disp->TexCoord3s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord3sv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord3sv", symbol_prefix);
        procp = (void **) &disp->TexCoord3sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4d) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4d", symbol_prefix);
        procp = (void **) &disp->TexCoord4d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4dv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4dv", symbol_prefix);
        procp = (void **) &disp->TexCoord4dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4f) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4f", symbol_prefix);
        procp = (void **) &disp->TexCoord4f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4fv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4fv", symbol_prefix);
        procp = (void **) &disp->TexCoord4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4i) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4i", symbol_prefix);
        procp = (void **) &disp->TexCoord4i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4iv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4iv", symbol_prefix);
        procp = (void **) &disp->TexCoord4iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4s) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4s", symbol_prefix);
        procp = (void **) &disp->TexCoord4s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoord4sv) {
        snprintf(symboln, sizeof(symboln), "%sTexCoord4sv", symbol_prefix);
        procp = (void **) &disp->TexCoord4sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2d) {
        snprintf(symboln, sizeof(symboln), "%sVertex2d", symbol_prefix);
        procp = (void **) &disp->Vertex2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2dv) {
        snprintf(symboln, sizeof(symboln), "%sVertex2dv", symbol_prefix);
        procp = (void **) &disp->Vertex2dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2f) {
        snprintf(symboln, sizeof(symboln), "%sVertex2f", symbol_prefix);
        procp = (void **) &disp->Vertex2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2fv) {
        snprintf(symboln, sizeof(symboln), "%sVertex2fv", symbol_prefix);
        procp = (void **) &disp->Vertex2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2i) {
        snprintf(symboln, sizeof(symboln), "%sVertex2i", symbol_prefix);
        procp = (void **) &disp->Vertex2i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2iv) {
        snprintf(symboln, sizeof(symboln), "%sVertex2iv", symbol_prefix);
        procp = (void **) &disp->Vertex2iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2s) {
        snprintf(symboln, sizeof(symboln), "%sVertex2s", symbol_prefix);
        procp = (void **) &disp->Vertex2s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex2sv) {
        snprintf(symboln, sizeof(symboln), "%sVertex2sv", symbol_prefix);
        procp = (void **) &disp->Vertex2sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3d) {
        snprintf(symboln, sizeof(symboln), "%sVertex3d", symbol_prefix);
        procp = (void **) &disp->Vertex3d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3dv) {
        snprintf(symboln, sizeof(symboln), "%sVertex3dv", symbol_prefix);
        procp = (void **) &disp->Vertex3dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3f) {
        snprintf(symboln, sizeof(symboln), "%sVertex3f", symbol_prefix);
        procp = (void **) &disp->Vertex3f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3fv) {
        snprintf(symboln, sizeof(symboln), "%sVertex3fv", symbol_prefix);
        procp = (void **) &disp->Vertex3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3i) {
        snprintf(symboln, sizeof(symboln), "%sVertex3i", symbol_prefix);
        procp = (void **) &disp->Vertex3i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3iv) {
        snprintf(symboln, sizeof(symboln), "%sVertex3iv", symbol_prefix);
        procp = (void **) &disp->Vertex3iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3s) {
        snprintf(symboln, sizeof(symboln), "%sVertex3s", symbol_prefix);
        procp = (void **) &disp->Vertex3s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex3sv) {
        snprintf(symboln, sizeof(symboln), "%sVertex3sv", symbol_prefix);
        procp = (void **) &disp->Vertex3sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4d) {
        snprintf(symboln, sizeof(symboln), "%sVertex4d", symbol_prefix);
        procp = (void **) &disp->Vertex4d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4dv) {
        snprintf(symboln, sizeof(symboln), "%sVertex4dv", symbol_prefix);
        procp = (void **) &disp->Vertex4dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4f) {
        snprintf(symboln, sizeof(symboln), "%sVertex4f", symbol_prefix);
        procp = (void **) &disp->Vertex4f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4fv) {
        snprintf(symboln, sizeof(symboln), "%sVertex4fv", symbol_prefix);
        procp = (void **) &disp->Vertex4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4i) {
        snprintf(symboln, sizeof(symboln), "%sVertex4i", symbol_prefix);
        procp = (void **) &disp->Vertex4i;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4iv) {
        snprintf(symboln, sizeof(symboln), "%sVertex4iv", symbol_prefix);
        procp = (void **) &disp->Vertex4iv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4s) {
        snprintf(symboln, sizeof(symboln), "%sVertex4s", symbol_prefix);
        procp = (void **) &disp->Vertex4s;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Vertex4sv) {
        snprintf(symboln, sizeof(symboln), "%sVertex4sv", symbol_prefix);
        procp = (void **) &disp->Vertex4sv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClipPlane) {
        snprintf(symboln, sizeof(symboln), "%sClipPlane", symbol_prefix);
        procp = (void **) &disp->ClipPlane;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorMaterial) {
        snprintf(symboln, sizeof(symboln), "%sColorMaterial", symbol_prefix);
        procp = (void **) &disp->ColorMaterial;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CullFace) {
        snprintf(symboln, sizeof(symboln), "%sCullFace", symbol_prefix);
        procp = (void **) &disp->CullFace;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Fogf) {
        snprintf(symboln, sizeof(symboln), "%sFogf", symbol_prefix);
        procp = (void **) &disp->Fogf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Fogfv) {
        snprintf(symboln, sizeof(symboln), "%sFogfv", symbol_prefix);
        procp = (void **) &disp->Fogfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Fogi) {
        snprintf(symboln, sizeof(symboln), "%sFogi", symbol_prefix);
        procp = (void **) &disp->Fogi;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Fogiv) {
        snprintf(symboln, sizeof(symboln), "%sFogiv", symbol_prefix);
        procp = (void **) &disp->Fogiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FrontFace) {
        snprintf(symboln, sizeof(symboln), "%sFrontFace", symbol_prefix);
        procp = (void **) &disp->FrontFace;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Hint) {
        snprintf(symboln, sizeof(symboln), "%sHint", symbol_prefix);
        procp = (void **) &disp->Hint;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Lightf) {
        snprintf(symboln, sizeof(symboln), "%sLightf", symbol_prefix);
        procp = (void **) &disp->Lightf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Lightfv) {
        snprintf(symboln, sizeof(symboln), "%sLightfv", symbol_prefix);
        procp = (void **) &disp->Lightfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Lighti) {
        snprintf(symboln, sizeof(symboln), "%sLighti", symbol_prefix);
        procp = (void **) &disp->Lighti;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Lightiv) {
        snprintf(symboln, sizeof(symboln), "%sLightiv", symbol_prefix);
        procp = (void **) &disp->Lightiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LightModelf) {
        snprintf(symboln, sizeof(symboln), "%sLightModelf", symbol_prefix);
        procp = (void **) &disp->LightModelf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LightModelfv) {
        snprintf(symboln, sizeof(symboln), "%sLightModelfv", symbol_prefix);
        procp = (void **) &disp->LightModelfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LightModeli) {
        snprintf(symboln, sizeof(symboln), "%sLightModeli", symbol_prefix);
        procp = (void **) &disp->LightModeli;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LightModeliv) {
        snprintf(symboln, sizeof(symboln), "%sLightModeliv", symbol_prefix);
        procp = (void **) &disp->LightModeliv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LineStipple) {
        snprintf(symboln, sizeof(symboln), "%sLineStipple", symbol_prefix);
        procp = (void **) &disp->LineStipple;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LineWidth) {
        snprintf(symboln, sizeof(symboln), "%sLineWidth", symbol_prefix);
        procp = (void **) &disp->LineWidth;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Materialf) {
        snprintf(symboln, sizeof(symboln), "%sMaterialf", symbol_prefix);
        procp = (void **) &disp->Materialf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Materialfv) {
        snprintf(symboln, sizeof(symboln), "%sMaterialfv", symbol_prefix);
        procp = (void **) &disp->Materialfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Materiali) {
        snprintf(symboln, sizeof(symboln), "%sMateriali", symbol_prefix);
        procp = (void **) &disp->Materiali;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Materialiv) {
        snprintf(symboln, sizeof(symboln), "%sMaterialiv", symbol_prefix);
        procp = (void **) &disp->Materialiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointSize) {
        snprintf(symboln, sizeof(symboln), "%sPointSize", symbol_prefix);
        procp = (void **) &disp->PointSize;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PolygonMode) {
        snprintf(symboln, sizeof(symboln), "%sPolygonMode", symbol_prefix);
        procp = (void **) &disp->PolygonMode;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PolygonStipple) {
        snprintf(symboln, sizeof(symboln), "%sPolygonStipple", symbol_prefix);
        procp = (void **) &disp->PolygonStipple;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Scissor) {
        snprintf(symboln, sizeof(symboln), "%sScissor", symbol_prefix);
        procp = (void **) &disp->Scissor;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ShadeModel) {
        snprintf(symboln, sizeof(symboln), "%sShadeModel", symbol_prefix);
        procp = (void **) &disp->ShadeModel;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexParameterf) {
        snprintf(symboln, sizeof(symboln), "%sTexParameterf", symbol_prefix);
        procp = (void **) &disp->TexParameterf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sTexParameterfv", symbol_prefix);
        procp = (void **) &disp->TexParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexParameteri) {
        snprintf(symboln, sizeof(symboln), "%sTexParameteri", symbol_prefix);
        procp = (void **) &disp->TexParameteri;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sTexParameteriv", symbol_prefix);
        procp = (void **) &disp->TexParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexImage1D) {
        snprintf(symboln, sizeof(symboln), "%sTexImage1D", symbol_prefix);
        procp = (void **) &disp->TexImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexImage2D) {
        snprintf(symboln, sizeof(symboln), "%sTexImage2D", symbol_prefix);
        procp = (void **) &disp->TexImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexEnvf) {
        snprintf(symboln, sizeof(symboln), "%sTexEnvf", symbol_prefix);
        procp = (void **) &disp->TexEnvf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexEnvfv) {
        snprintf(symboln, sizeof(symboln), "%sTexEnvfv", symbol_prefix);
        procp = (void **) &disp->TexEnvfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexEnvi) {
        snprintf(symboln, sizeof(symboln), "%sTexEnvi", symbol_prefix);
        procp = (void **) &disp->TexEnvi;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexEnviv) {
        snprintf(symboln, sizeof(symboln), "%sTexEnviv", symbol_prefix);
        procp = (void **) &disp->TexEnviv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGend) {
        snprintf(symboln, sizeof(symboln), "%sTexGend", symbol_prefix);
        procp = (void **) &disp->TexGend;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGendv) {
        snprintf(symboln, sizeof(symboln), "%sTexGendv", symbol_prefix);
        procp = (void **) &disp->TexGendv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGenf) {
        snprintf(symboln, sizeof(symboln), "%sTexGenf", symbol_prefix);
        procp = (void **) &disp->TexGenf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGenfv) {
        snprintf(symboln, sizeof(symboln), "%sTexGenfv", symbol_prefix);
        procp = (void **) &disp->TexGenfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGeni) {
        snprintf(symboln, sizeof(symboln), "%sTexGeni", symbol_prefix);
        procp = (void **) &disp->TexGeni;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexGeniv) {
        snprintf(symboln, sizeof(symboln), "%sTexGeniv", symbol_prefix);
        procp = (void **) &disp->TexGeniv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FeedbackBuffer) {
        snprintf(symboln, sizeof(symboln), "%sFeedbackBuffer", symbol_prefix);
        procp = (void **) &disp->FeedbackBuffer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SelectBuffer) {
        snprintf(symboln, sizeof(symboln), "%sSelectBuffer", symbol_prefix);
        procp = (void **) &disp->SelectBuffer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RenderMode) {
        snprintf(symboln, sizeof(symboln), "%sRenderMode", symbol_prefix);
        procp = (void **) &disp->RenderMode;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->InitNames) {
        snprintf(symboln, sizeof(symboln), "%sInitNames", symbol_prefix);
        procp = (void **) &disp->InitNames;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadName) {
        snprintf(symboln, sizeof(symboln), "%sLoadName", symbol_prefix);
        procp = (void **) &disp->LoadName;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PassThrough) {
        snprintf(symboln, sizeof(symboln), "%sPassThrough", symbol_prefix);
        procp = (void **) &disp->PassThrough;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PopName) {
        snprintf(symboln, sizeof(symboln), "%sPopName", symbol_prefix);
        procp = (void **) &disp->PopName;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PushName) {
        snprintf(symboln, sizeof(symboln), "%sPushName", symbol_prefix);
        procp = (void **) &disp->PushName;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawBuffer) {
        snprintf(symboln, sizeof(symboln), "%sDrawBuffer", symbol_prefix);
        procp = (void **) &disp->DrawBuffer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Clear) {
        snprintf(symboln, sizeof(symboln), "%sClear", symbol_prefix);
        procp = (void **) &disp->Clear;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClearAccum) {
        snprintf(symboln, sizeof(symboln), "%sClearAccum", symbol_prefix);
        procp = (void **) &disp->ClearAccum;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClearIndex) {
        snprintf(symboln, sizeof(symboln), "%sClearIndex", symbol_prefix);
        procp = (void **) &disp->ClearIndex;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClearColor) {
        snprintf(symboln, sizeof(symboln), "%sClearColor", symbol_prefix);
        procp = (void **) &disp->ClearColor;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClearStencil) {
        snprintf(symboln, sizeof(symboln), "%sClearStencil", symbol_prefix);
        procp = (void **) &disp->ClearStencil;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClearDepth) {
        snprintf(symboln, sizeof(symboln), "%sClearDepth", symbol_prefix);
        procp = (void **) &disp->ClearDepth;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilMask) {
        snprintf(symboln, sizeof(symboln), "%sStencilMask", symbol_prefix);
        procp = (void **) &disp->StencilMask;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorMask) {
        snprintf(symboln, sizeof(symboln), "%sColorMask", symbol_prefix);
        procp = (void **) &disp->ColorMask;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DepthMask) {
        snprintf(symboln, sizeof(symboln), "%sDepthMask", symbol_prefix);
        procp = (void **) &disp->DepthMask;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IndexMask) {
        snprintf(symboln, sizeof(symboln), "%sIndexMask", symbol_prefix);
        procp = (void **) &disp->IndexMask;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Accum) {
        snprintf(symboln, sizeof(symboln), "%sAccum", symbol_prefix);
        procp = (void **) &disp->Accum;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Disable) {
        snprintf(symboln, sizeof(symboln), "%sDisable", symbol_prefix);
        procp = (void **) &disp->Disable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Enable) {
        snprintf(symboln, sizeof(symboln), "%sEnable", symbol_prefix);
        procp = (void **) &disp->Enable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Finish) {
        snprintf(symboln, sizeof(symboln), "%sFinish", symbol_prefix);
        procp = (void **) &disp->Finish;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Flush) {
        snprintf(symboln, sizeof(symboln), "%sFlush", symbol_prefix);
        procp = (void **) &disp->Flush;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PopAttrib) {
        snprintf(symboln, sizeof(symboln), "%sPopAttrib", symbol_prefix);
        procp = (void **) &disp->PopAttrib;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PushAttrib) {
        snprintf(symboln, sizeof(symboln), "%sPushAttrib", symbol_prefix);
        procp = (void **) &disp->PushAttrib;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Map1d) {
        snprintf(symboln, sizeof(symboln), "%sMap1d", symbol_prefix);
        procp = (void **) &disp->Map1d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Map1f) {
        snprintf(symboln, sizeof(symboln), "%sMap1f", symbol_prefix);
        procp = (void **) &disp->Map1f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Map2d) {
        snprintf(symboln, sizeof(symboln), "%sMap2d", symbol_prefix);
        procp = (void **) &disp->Map2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Map2f) {
        snprintf(symboln, sizeof(symboln), "%sMap2f", symbol_prefix);
        procp = (void **) &disp->Map2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapGrid1d) {
        snprintf(symboln, sizeof(symboln), "%sMapGrid1d", symbol_prefix);
        procp = (void **) &disp->MapGrid1d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapGrid1f) {
        snprintf(symboln, sizeof(symboln), "%sMapGrid1f", symbol_prefix);
        procp = (void **) &disp->MapGrid1f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapGrid2d) {
        snprintf(symboln, sizeof(symboln), "%sMapGrid2d", symbol_prefix);
        procp = (void **) &disp->MapGrid2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapGrid2f) {
        snprintf(symboln, sizeof(symboln), "%sMapGrid2f", symbol_prefix);
        procp = (void **) &disp->MapGrid2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord1d) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1d", symbol_prefix);
        procp = (void **) &disp->EvalCoord1d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord1dv) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1dv", symbol_prefix);
        procp = (void **) &disp->EvalCoord1dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord1f) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1f", symbol_prefix);
        procp = (void **) &disp->EvalCoord1f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord1fv) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1fv", symbol_prefix);
        procp = (void **) &disp->EvalCoord1fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord2d) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2d", symbol_prefix);
        procp = (void **) &disp->EvalCoord2d;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord2dv) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2dv", symbol_prefix);
        procp = (void **) &disp->EvalCoord2dv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord2f) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2f", symbol_prefix);
        procp = (void **) &disp->EvalCoord2f;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalCoord2fv) {
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2fv", symbol_prefix);
        procp = (void **) &disp->EvalCoord2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalMesh1) {
        snprintf(symboln, sizeof(symboln), "%sEvalMesh1", symbol_prefix);
        procp = (void **) &disp->EvalMesh1;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalPoint1) {
        snprintf(symboln, sizeof(symboln), "%sEvalPoint1", symbol_prefix);
        procp = (void **) &disp->EvalPoint1;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalMesh2) {
        snprintf(symboln, sizeof(symboln), "%sEvalMesh2", symbol_prefix);
        procp = (void **) &disp->EvalMesh2;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EvalPoint2) {
        snprintf(symboln, sizeof(symboln), "%sEvalPoint2", symbol_prefix);
        procp = (void **) &disp->EvalPoint2;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AlphaFunc) {
        snprintf(symboln, sizeof(symboln), "%sAlphaFunc", symbol_prefix);
        procp = (void **) &disp->AlphaFunc;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendFunc) {
        snprintf(symboln, sizeof(symboln), "%sBlendFunc", symbol_prefix);
        procp = (void **) &disp->BlendFunc;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LogicOp) {
        snprintf(symboln, sizeof(symboln), "%sLogicOp", symbol_prefix);
        procp = (void **) &disp->LogicOp;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilFunc) {
        snprintf(symboln, sizeof(symboln), "%sStencilFunc", symbol_prefix);
        procp = (void **) &disp->StencilFunc;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilOp) {
        snprintf(symboln, sizeof(symboln), "%sStencilOp", symbol_prefix);
        procp = (void **) &disp->StencilOp;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DepthFunc) {
        snprintf(symboln, sizeof(symboln), "%sDepthFunc", symbol_prefix);
        procp = (void **) &disp->DepthFunc;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelZoom) {
        snprintf(symboln, sizeof(symboln), "%sPixelZoom", symbol_prefix);
        procp = (void **) &disp->PixelZoom;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTransferf) {
        snprintf(symboln, sizeof(symboln), "%sPixelTransferf", symbol_prefix);
        procp = (void **) &disp->PixelTransferf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTransferi) {
        snprintf(symboln, sizeof(symboln), "%sPixelTransferi", symbol_prefix);
        procp = (void **) &disp->PixelTransferi;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelStoref) {
        snprintf(symboln, sizeof(symboln), "%sPixelStoref", symbol_prefix);
        procp = (void **) &disp->PixelStoref;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelStorei) {
        snprintf(symboln, sizeof(symboln), "%sPixelStorei", symbol_prefix);
        procp = (void **) &disp->PixelStorei;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelMapfv) {
        snprintf(symboln, sizeof(symboln), "%sPixelMapfv", symbol_prefix);
        procp = (void **) &disp->PixelMapfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelMapuiv) {
        snprintf(symboln, sizeof(symboln), "%sPixelMapuiv", symbol_prefix);
        procp = (void **) &disp->PixelMapuiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelMapusv) {
        snprintf(symboln, sizeof(symboln), "%sPixelMapusv", symbol_prefix);
        procp = (void **) &disp->PixelMapusv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ReadBuffer) {
        snprintf(symboln, sizeof(symboln), "%sReadBuffer", symbol_prefix);
        procp = (void **) &disp->ReadBuffer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyPixels) {
        snprintf(symboln, sizeof(symboln), "%sCopyPixels", symbol_prefix);
        procp = (void **) &disp->CopyPixels;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ReadPixels) {
        snprintf(symboln, sizeof(symboln), "%sReadPixels", symbol_prefix);
        procp = (void **) &disp->ReadPixels;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawPixels) {
        snprintf(symboln, sizeof(symboln), "%sDrawPixels", symbol_prefix);
        procp = (void **) &disp->DrawPixels;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBooleanv) {
        snprintf(symboln, sizeof(symboln), "%sGetBooleanv", symbol_prefix);
        procp = (void **) &disp->GetBooleanv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetClipPlane) {
        snprintf(symboln, sizeof(symboln), "%sGetClipPlane", symbol_prefix);
        procp = (void **) &disp->GetClipPlane;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetDoublev) {
        snprintf(symboln, sizeof(symboln), "%sGetDoublev", symbol_prefix);
        procp = (void **) &disp->GetDoublev;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetError) {
        snprintf(symboln, sizeof(symboln), "%sGetError", symbol_prefix);
        procp = (void **) &disp->GetError;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFloatv) {
        snprintf(symboln, sizeof(symboln), "%sGetFloatv", symbol_prefix);
        procp = (void **) &disp->GetFloatv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetIntegerv) {
        snprintf(symboln, sizeof(symboln), "%sGetIntegerv", symbol_prefix);
        procp = (void **) &disp->GetIntegerv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetLightfv) {
        snprintf(symboln, sizeof(symboln), "%sGetLightfv", symbol_prefix);
        procp = (void **) &disp->GetLightfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetLightiv) {
        snprintf(symboln, sizeof(symboln), "%sGetLightiv", symbol_prefix);
        procp = (void **) &disp->GetLightiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMapdv) {
        snprintf(symboln, sizeof(symboln), "%sGetMapdv", symbol_prefix);
        procp = (void **) &disp->GetMapdv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMapfv) {
        snprintf(symboln, sizeof(symboln), "%sGetMapfv", symbol_prefix);
        procp = (void **) &disp->GetMapfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMapiv) {
        snprintf(symboln, sizeof(symboln), "%sGetMapiv", symbol_prefix);
        procp = (void **) &disp->GetMapiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMaterialfv) {
        snprintf(symboln, sizeof(symboln), "%sGetMaterialfv", symbol_prefix);
        procp = (void **) &disp->GetMaterialfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMaterialiv) {
        snprintf(symboln, sizeof(symboln), "%sGetMaterialiv", symbol_prefix);
        procp = (void **) &disp->GetMaterialiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPixelMapfv) {
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapfv", symbol_prefix);
        procp = (void **) &disp->GetPixelMapfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPixelMapuiv) {
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapuiv", symbol_prefix);
        procp = (void **) &disp->GetPixelMapuiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPixelMapusv) {
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapusv", symbol_prefix);
        procp = (void **) &disp->GetPixelMapusv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPolygonStipple) {
        snprintf(symboln, sizeof(symboln), "%sGetPolygonStipple", symbol_prefix);
        procp = (void **) &disp->GetPolygonStipple;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetString) {
        snprintf(symboln, sizeof(symboln), "%sGetString", symbol_prefix);
        procp = (void **) &disp->GetString;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexEnvfv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexEnvfv", symbol_prefix);
        procp = (void **) &disp->GetTexEnvfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexEnviv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexEnviv", symbol_prefix);
        procp = (void **) &disp->GetTexEnviv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexGendv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexGendv", symbol_prefix);
        procp = (void **) &disp->GetTexGendv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexGenfv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexGenfv", symbol_prefix);
        procp = (void **) &disp->GetTexGenfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexGeniv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexGeniv", symbol_prefix);
        procp = (void **) &disp->GetTexGeniv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexImage) {
        snprintf(symboln, sizeof(symboln), "%sGetTexImage", symbol_prefix);
        procp = (void **) &disp->GetTexImage;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterfv", symbol_prefix);
        procp = (void **) &disp->GetTexParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexParameteriv", symbol_prefix);
        procp = (void **) &disp->GetTexParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexLevelParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexLevelParameterfv", symbol_prefix);
        procp = (void **) &disp->GetTexLevelParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTexLevelParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetTexLevelParameteriv", symbol_prefix);
        procp = (void **) &disp->GetTexLevelParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsEnabled) {
        snprintf(symboln, sizeof(symboln), "%sIsEnabled", symbol_prefix);
        procp = (void **) &disp->IsEnabled;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsList) {
        snprintf(symboln, sizeof(symboln), "%sIsList", symbol_prefix);
        procp = (void **) &disp->IsList;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DepthRange) {
        snprintf(symboln, sizeof(symboln), "%sDepthRange", symbol_prefix);
        procp = (void **) &disp->DepthRange;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Frustum) {
        snprintf(symboln, sizeof(symboln), "%sFrustum", symbol_prefix);
        procp = (void **) &disp->Frustum;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadIdentity) {
        snprintf(symboln, sizeof(symboln), "%sLoadIdentity", symbol_prefix);
        procp = (void **) &disp->LoadIdentity;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadMatrixf) {
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixf", symbol_prefix);
        procp = (void **) &disp->LoadMatrixf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadMatrixd) {
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixd", symbol_prefix);
        procp = (void **) &disp->LoadMatrixd;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MatrixMode) {
        snprintf(symboln, sizeof(symboln), "%sMatrixMode", symbol_prefix);
        procp = (void **) &disp->MatrixMode;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultMatrixf) {
        snprintf(symboln, sizeof(symboln), "%sMultMatrixf", symbol_prefix);
        procp = (void **) &disp->MultMatrixf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultMatrixd) {
        snprintf(symboln, sizeof(symboln), "%sMultMatrixd", symbol_prefix);
        procp = (void **) &disp->MultMatrixd;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Ortho) {
        snprintf(symboln, sizeof(symboln), "%sOrtho", symbol_prefix);
        procp = (void **) &disp->Ortho;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PopMatrix) {
        snprintf(symboln, sizeof(symboln), "%sPopMatrix", symbol_prefix);
        procp = (void **) &disp->PopMatrix;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PushMatrix) {
        snprintf(symboln, sizeof(symboln), "%sPushMatrix", symbol_prefix);
        procp = (void **) &disp->PushMatrix;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rotated) {
        snprintf(symboln, sizeof(symboln), "%sRotated", symbol_prefix);
        procp = (void **) &disp->Rotated;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Rotatef) {
        snprintf(symboln, sizeof(symboln), "%sRotatef", symbol_prefix);
        procp = (void **) &disp->Rotatef;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Scaled) {
        snprintf(symboln, sizeof(symboln), "%sScaled", symbol_prefix);
        procp = (void **) &disp->Scaled;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Scalef) {
        snprintf(symboln, sizeof(symboln), "%sScalef", symbol_prefix);
        procp = (void **) &disp->Scalef;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Translated) {
        snprintf(symboln, sizeof(symboln), "%sTranslated", symbol_prefix);
        procp = (void **) &disp->Translated;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Translatef) {
        snprintf(symboln, sizeof(symboln), "%sTranslatef", symbol_prefix);
        procp = (void **) &disp->Translatef;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Viewport) {
        snprintf(symboln, sizeof(symboln), "%sViewport", symbol_prefix);
        procp = (void **) &disp->Viewport;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ArrayElement) {
        snprintf(symboln, sizeof(symboln), "%sArrayElement", symbol_prefix);
        procp = (void **) &disp->ArrayElement;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ArrayElement) {
        snprintf(symboln, sizeof(symboln), "%sArrayElementEXT", symbol_prefix);
        procp = (void **) &disp->ArrayElement;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindTexture) {
        snprintf(symboln, sizeof(symboln), "%sBindTexture", symbol_prefix);
        procp = (void **) &disp->BindTexture;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindTexture) {
        snprintf(symboln, sizeof(symboln), "%sBindTextureEXT", symbol_prefix);
        procp = (void **) &disp->BindTexture;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorPointer) {
        snprintf(symboln, sizeof(symboln), "%sColorPointer", symbol_prefix);
        procp = (void **) &disp->ColorPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DisableClientState) {
        snprintf(symboln, sizeof(symboln), "%sDisableClientState", symbol_prefix);
        procp = (void **) &disp->DisableClientState;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawArrays) {
        snprintf(symboln, sizeof(symboln), "%sDrawArrays", symbol_prefix);
        procp = (void **) &disp->DrawArrays;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawArrays) {
        snprintf(symboln, sizeof(symboln), "%sDrawArraysEXT", symbol_prefix);
        procp = (void **) &disp->DrawArrays;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawElements) {
        snprintf(symboln, sizeof(symboln), "%sDrawElements", symbol_prefix);
        procp = (void **) &disp->DrawElements;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EdgeFlagPointer) {
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagPointer", symbol_prefix);
        procp = (void **) &disp->EdgeFlagPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EnableClientState) {
        snprintf(symboln, sizeof(symboln), "%sEnableClientState", symbol_prefix);
        procp = (void **) &disp->EnableClientState;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IndexPointer) {
        snprintf(symboln, sizeof(symboln), "%sIndexPointer", symbol_prefix);
        procp = (void **) &disp->IndexPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexub) {
        snprintf(symboln, sizeof(symboln), "%sIndexub", symbol_prefix);
        procp = (void **) &disp->Indexub;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Indexubv) {
        snprintf(symboln, sizeof(symboln), "%sIndexubv", symbol_prefix);
        procp = (void **) &disp->Indexubv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->InterleavedArrays) {
        snprintf(symboln, sizeof(symboln), "%sInterleavedArrays", symbol_prefix);
        procp = (void **) &disp->InterleavedArrays;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->NormalPointer) {
        snprintf(symboln, sizeof(symboln), "%sNormalPointer", symbol_prefix);
        procp = (void **) &disp->NormalPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PolygonOffset) {
        snprintf(symboln, sizeof(symboln), "%sPolygonOffset", symbol_prefix);
        procp = (void **) &disp->PolygonOffset;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoordPointer) {
        snprintf(symboln, sizeof(symboln), "%sTexCoordPointer", symbol_prefix);
        procp = (void **) &disp->TexCoordPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexPointer) {
        snprintf(symboln, sizeof(symboln), "%sVertexPointer", symbol_prefix);
        procp = (void **) &disp->VertexPointer;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AreTexturesResident) {
        snprintf(symboln, sizeof(symboln), "%sAreTexturesResident", symbol_prefix);
        procp = (void **) &disp->AreTexturesResident;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AreTexturesResident) {
        snprintf(symboln, sizeof(symboln), "%sAreTexturesResidentEXT", symbol_prefix);
        procp = (void **) &disp->AreTexturesResident;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexImage1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage1D", symbol_prefix);
        procp = (void **) &disp->CopyTexImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexImage1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage1DEXT", symbol_prefix);
        procp = (void **) &disp->CopyTexImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexImage2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage2D", symbol_prefix);
        procp = (void **) &disp->CopyTexImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexImage2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage2DEXT", symbol_prefix);
        procp = (void **) &disp->CopyTexImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage1D", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage1DEXT", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage2D", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage2DEXT", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteTextures) {
        snprintf(symboln, sizeof(symboln), "%sDeleteTextures", symbol_prefix);
        procp = (void **) &disp->DeleteTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteTextures) {
        snprintf(symboln, sizeof(symboln), "%sDeleteTexturesEXT", symbol_prefix);
        procp = (void **) &disp->DeleteTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenTextures) {
        snprintf(symboln, sizeof(symboln), "%sGenTextures", symbol_prefix);
        procp = (void **) &disp->GenTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenTextures) {
        snprintf(symboln, sizeof(symboln), "%sGenTexturesEXT", symbol_prefix);
        procp = (void **) &disp->GenTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPointerv) {
        snprintf(symboln, sizeof(symboln), "%sGetPointerv", symbol_prefix);
        procp = (void **) &disp->GetPointerv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPointerv) {
        snprintf(symboln, sizeof(symboln), "%sGetPointervEXT", symbol_prefix);
        procp = (void **) &disp->GetPointerv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsTexture) {
        snprintf(symboln, sizeof(symboln), "%sIsTexture", symbol_prefix);
        procp = (void **) &disp->IsTexture;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsTexture) {
        snprintf(symboln, sizeof(symboln), "%sIsTextureEXT", symbol_prefix);
        procp = (void **) &disp->IsTexture;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PrioritizeTextures) {
        snprintf(symboln, sizeof(symboln), "%sPrioritizeTextures", symbol_prefix);
        procp = (void **) &disp->PrioritizeTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PrioritizeTextures) {
        snprintf(symboln, sizeof(symboln), "%sPrioritizeTexturesEXT", symbol_prefix);
        procp = (void **) &disp->PrioritizeTextures;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage1D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage1D", symbol_prefix);
        procp = (void **) &disp->TexSubImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage1D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage1DEXT", symbol_prefix);
        procp = (void **) &disp->TexSubImage1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage2D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage2D", symbol_prefix);
        procp = (void **) &disp->TexSubImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage2D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage2DEXT", symbol_prefix);
        procp = (void **) &disp->TexSubImage2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PopClientAttrib) {
        snprintf(symboln, sizeof(symboln), "%sPopClientAttrib", symbol_prefix);
        procp = (void **) &disp->PopClientAttrib;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PushClientAttrib) {
        snprintf(symboln, sizeof(symboln), "%sPushClientAttrib", symbol_prefix);
        procp = (void **) &disp->PushClientAttrib;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendColor) {
        snprintf(symboln, sizeof(symboln), "%sBlendColor", symbol_prefix);
        procp = (void **) &disp->BlendColor;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendColor) {
        snprintf(symboln, sizeof(symboln), "%sBlendColorEXT", symbol_prefix);
        procp = (void **) &disp->BlendColor;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendEquation) {
        snprintf(symboln, sizeof(symboln), "%sBlendEquation", symbol_prefix);
        procp = (void **) &disp->BlendEquation;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendEquation) {
        snprintf(symboln, sizeof(symboln), "%sBlendEquationEXT", symbol_prefix);
        procp = (void **) &disp->BlendEquation;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawRangeElements) {
        snprintf(symboln, sizeof(symboln), "%sDrawRangeElements", symbol_prefix);
        procp = (void **) &disp->DrawRangeElements;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawRangeElements) {
        snprintf(symboln, sizeof(symboln), "%sDrawRangeElementsEXT", symbol_prefix);
        procp = (void **) &disp->DrawRangeElements;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTable) {
        snprintf(symboln, sizeof(symboln), "%sColorTable", symbol_prefix);
        procp = (void **) &disp->ColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTable) {
        snprintf(symboln, sizeof(symboln), "%sColorTableSGI", symbol_prefix);
        procp = (void **) &disp->ColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTable) {
        snprintf(symboln, sizeof(symboln), "%sColorTableEXT", symbol_prefix);
        procp = (void **) &disp->ColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTableParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterfv", symbol_prefix);
        procp = (void **) &disp->ColorTableParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTableParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterfvSGI", symbol_prefix);
        procp = (void **) &disp->ColorTableParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTableParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sColorTableParameteriv", symbol_prefix);
        procp = (void **) &disp->ColorTableParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorTableParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterivSGI", symbol_prefix);
        procp = (void **) &disp->ColorTableParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyColorTable) {
        snprintf(symboln, sizeof(symboln), "%sCopyColorTable", symbol_prefix);
        procp = (void **) &disp->CopyColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyColorTable) {
        snprintf(symboln, sizeof(symboln), "%sCopyColorTableSGI", symbol_prefix);
        procp = (void **) &disp->CopyColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTable) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTable", symbol_prefix);
        procp = (void **) &disp->GetColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTable) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableSGI", symbol_prefix);
        procp = (void **) &disp->GetColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTable) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableEXT", symbol_prefix);
        procp = (void **) &disp->GetColorTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfv", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfvSGI", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameteriv", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterivSGI", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetColorTableParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetColorTableParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorSubTable) {
        snprintf(symboln, sizeof(symboln), "%sColorSubTable", symbol_prefix);
        procp = (void **) &disp->ColorSubTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorSubTable) {
        snprintf(symboln, sizeof(symboln), "%sColorSubTableEXT", symbol_prefix);
        procp = (void **) &disp->ColorSubTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyColorSubTable) {
        snprintf(symboln, sizeof(symboln), "%sCopyColorSubTable", symbol_prefix);
        procp = (void **) &disp->CopyColorSubTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyColorSubTable) {
        snprintf(symboln, sizeof(symboln), "%sCopyColorSubTableEXT", symbol_prefix);
        procp = (void **) &disp->CopyColorSubTable;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionFilter1D) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter1D", symbol_prefix);
        procp = (void **) &disp->ConvolutionFilter1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionFilter1D) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter1DEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionFilter1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter2D", symbol_prefix);
        procp = (void **) &disp->ConvolutionFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter2DEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameterf) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterf", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameterf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameterf) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameterf;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfv", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameteri) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteri", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameteri;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameteri) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteriEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameteri;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteriv", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ConvolutionParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterivEXT", symbol_prefix);
        procp = (void **) &disp->ConvolutionParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyConvolutionFilter1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter1D", symbol_prefix);
        procp = (void **) &disp->CopyConvolutionFilter1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyConvolutionFilter1D) {
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter1DEXT", symbol_prefix);
        procp = (void **) &disp->CopyConvolutionFilter1D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyConvolutionFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter2D", symbol_prefix);
        procp = (void **) &disp->CopyConvolutionFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyConvolutionFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter2DEXT", symbol_prefix);
        procp = (void **) &disp->CopyConvolutionFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionFilter) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionFilter", symbol_prefix);
        procp = (void **) &disp->GetConvolutionFilter;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionFilter) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionFilterEXT", symbol_prefix);
        procp = (void **) &disp->GetConvolutionFilter;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterfv", symbol_prefix);
        procp = (void **) &disp->GetConvolutionParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->GetConvolutionParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameteriv", symbol_prefix);
        procp = (void **) &disp->GetConvolutionParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetConvolutionParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetConvolutionParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetSeparableFilter) {
        snprintf(symboln, sizeof(symboln), "%sGetSeparableFilter", symbol_prefix);
        procp = (void **) &disp->GetSeparableFilter;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetSeparableFilter) {
        snprintf(symboln, sizeof(symboln), "%sGetSeparableFilterEXT", symbol_prefix);
        procp = (void **) &disp->GetSeparableFilter;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SeparableFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sSeparableFilter2D", symbol_prefix);
        procp = (void **) &disp->SeparableFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SeparableFilter2D) {
        snprintf(symboln, sizeof(symboln), "%sSeparableFilter2DEXT", symbol_prefix);
        procp = (void **) &disp->SeparableFilter2D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogram) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogram", symbol_prefix);
        procp = (void **) &disp->GetHistogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogram) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogramEXT", symbol_prefix);
        procp = (void **) &disp->GetHistogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogramParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterfv", symbol_prefix);
        procp = (void **) &disp->GetHistogramParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogramParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->GetHistogramParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogramParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameteriv", symbol_prefix);
        procp = (void **) &disp->GetHistogramParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHistogramParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetHistogramParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmax) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmax", symbol_prefix);
        procp = (void **) &disp->GetMinmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmax) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxEXT", symbol_prefix);
        procp = (void **) &disp->GetMinmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmaxParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterfv", symbol_prefix);
        procp = (void **) &disp->GetMinmaxParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmaxParameterfv) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->GetMinmaxParameterfv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmaxParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameteriv", symbol_prefix);
        procp = (void **) &disp->GetMinmaxParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetMinmaxParameteriv) {
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetMinmaxParameteriv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Histogram) {
        snprintf(symboln, sizeof(symboln), "%sHistogram", symbol_prefix);
        procp = (void **) &disp->Histogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Histogram) {
        snprintf(symboln, sizeof(symboln), "%sHistogramEXT", symbol_prefix);
        procp = (void **) &disp->Histogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Minmax) {
        snprintf(symboln, sizeof(symboln), "%sMinmax", symbol_prefix);
        procp = (void **) &disp->Minmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Minmax) {
        snprintf(symboln, sizeof(symboln), "%sMinmaxEXT", symbol_prefix);
        procp = (void **) &disp->Minmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ResetHistogram) {
        snprintf(symboln, sizeof(symboln), "%sResetHistogram", symbol_prefix);
        procp = (void **) &disp->ResetHistogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ResetHistogram) {
        snprintf(symboln, sizeof(symboln), "%sResetHistogramEXT", symbol_prefix);
        procp = (void **) &disp->ResetHistogram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ResetMinmax) {
        snprintf(symboln, sizeof(symboln), "%sResetMinmax", symbol_prefix);
        procp = (void **) &disp->ResetMinmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ResetMinmax) {
        snprintf(symboln, sizeof(symboln), "%sResetMinmaxEXT", symbol_prefix);
        procp = (void **) &disp->ResetMinmax;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexImage3D) {
        snprintf(symboln, sizeof(symboln), "%sTexImage3D", symbol_prefix);
        procp = (void **) &disp->TexImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexImage3D) {
        snprintf(symboln, sizeof(symboln), "%sTexImage3DEXT", symbol_prefix);
        procp = (void **) &disp->TexImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage3D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage3D", symbol_prefix);
        procp = (void **) &disp->TexSubImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexSubImage3D) {
        snprintf(symboln, sizeof(symboln), "%sTexSubImage3DEXT", symbol_prefix);
        procp = (void **) &disp->TexSubImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage3D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage3D", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CopyTexSubImage3D) {
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage3DEXT", symbol_prefix);
        procp = (void **) &disp->CopyTexSubImage3D;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ActiveTextureARB) {
        snprintf(symboln, sizeof(symboln), "%sActiveTexture", symbol_prefix);
        procp = (void **) &disp->ActiveTextureARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ActiveTextureARB) {
        snprintf(symboln, sizeof(symboln), "%sActiveTextureARB", symbol_prefix);
        procp = (void **) &disp->ActiveTextureARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClientActiveTextureARB) {
        snprintf(symboln, sizeof(symboln), "%sClientActiveTexture", symbol_prefix);
        procp = (void **) &disp->ClientActiveTextureARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ClientActiveTextureARB) {
        snprintf(symboln, sizeof(symboln), "%sClientActiveTextureARB", symbol_prefix);
        procp = (void **) &disp->ClientActiveTextureARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1d", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1f", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1i", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1iARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1iv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1ivARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1s", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1sARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1sv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord1svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1svARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord1svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2d", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2f", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2i", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2iARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2iv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2ivARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2s", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2sARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2sv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord2svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2svARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord2svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3d", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3f", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3i", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3iARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3iv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3ivARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3s", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3sARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3sv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord3svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3svARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord3svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4d", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4dARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4f", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4fARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fvARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4i", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4iARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4iARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4iv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4ivARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4s", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4sARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4sARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4sv", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiTexCoord4svARB) {
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4svARB", symbol_prefix);
        procp = (void **) &disp->MultiTexCoord4svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AttachShader) {
        snprintf(symboln, sizeof(symboln), "%sAttachShader", symbol_prefix);
        procp = (void **) &disp->AttachShader;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CreateProgram) {
        snprintf(symboln, sizeof(symboln), "%sCreateProgram", symbol_prefix);
        procp = (void **) &disp->CreateProgram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CreateShader) {
        snprintf(symboln, sizeof(symboln), "%sCreateShader", symbol_prefix);
        procp = (void **) &disp->CreateShader;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteProgram) {
        snprintf(symboln, sizeof(symboln), "%sDeleteProgram", symbol_prefix);
        procp = (void **) &disp->DeleteProgram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteShader) {
        snprintf(symboln, sizeof(symboln), "%sDeleteShader", symbol_prefix);
        procp = (void **) &disp->DeleteShader;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DetachShader) {
        snprintf(symboln, sizeof(symboln), "%sDetachShader", symbol_prefix);
        procp = (void **) &disp->DetachShader;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetAttachedShaders) {
        snprintf(symboln, sizeof(symboln), "%sGetAttachedShaders", symbol_prefix);
        procp = (void **) &disp->GetAttachedShaders;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramInfoLog) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramInfoLog", symbol_prefix);
        procp = (void **) &disp->GetProgramInfoLog;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramiv) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramiv", symbol_prefix);
        procp = (void **) &disp->GetProgramiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetShaderInfoLog) {
        snprintf(symboln, sizeof(symboln), "%sGetShaderInfoLog", symbol_prefix);
        procp = (void **) &disp->GetShaderInfoLog;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetShaderiv) {
        snprintf(symboln, sizeof(symboln), "%sGetShaderiv", symbol_prefix);
        procp = (void **) &disp->GetShaderiv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsProgram) {
        snprintf(symboln, sizeof(symboln), "%sIsProgram", symbol_prefix);
        procp = (void **) &disp->IsProgram;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsShader) {
        snprintf(symboln, sizeof(symboln), "%sIsShader", symbol_prefix);
        procp = (void **) &disp->IsShader;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilFuncSeparate) {
        snprintf(symboln, sizeof(symboln), "%sStencilFuncSeparate", symbol_prefix);
        procp = (void **) &disp->StencilFuncSeparate;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilMaskSeparate) {
        snprintf(symboln, sizeof(symboln), "%sStencilMaskSeparate", symbol_prefix);
        procp = (void **) &disp->StencilMaskSeparate;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilOpSeparate) {
        snprintf(symboln, sizeof(symboln), "%sStencilOpSeparate", symbol_prefix);
        procp = (void **) &disp->StencilOpSeparate;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilOpSeparate) {
        snprintf(symboln, sizeof(symboln), "%sStencilOpSeparateATI", symbol_prefix);
        procp = (void **) &disp->StencilOpSeparate;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix2x3fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x3fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix2x3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix2x4fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x4fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix2x4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix3x2fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x2fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix3x2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix3x4fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x4fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix3x4fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix4x2fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x2fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix4x2fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix4x3fv) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x3fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix4x3fv;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadTransposeMatrixdARB) {
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixd", symbol_prefix);
        procp = (void **) &disp->LoadTransposeMatrixdARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadTransposeMatrixdARB) {
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixdARB", symbol_prefix);
        procp = (void **) &disp->LoadTransposeMatrixdARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadTransposeMatrixfARB) {
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixf", symbol_prefix);
        procp = (void **) &disp->LoadTransposeMatrixfARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadTransposeMatrixfARB) {
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixfARB", symbol_prefix);
        procp = (void **) &disp->LoadTransposeMatrixfARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultTransposeMatrixdARB) {
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixd", symbol_prefix);
        procp = (void **) &disp->MultTransposeMatrixdARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultTransposeMatrixdARB) {
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixdARB", symbol_prefix);
        procp = (void **) &disp->MultTransposeMatrixdARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultTransposeMatrixfARB) {
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixf", symbol_prefix);
        procp = (void **) &disp->MultTransposeMatrixfARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultTransposeMatrixfARB) {
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixfARB", symbol_prefix);
        procp = (void **) &disp->MultTransposeMatrixfARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SampleCoverageARB) {
        snprintf(symboln, sizeof(symboln), "%sSampleCoverage", symbol_prefix);
        procp = (void **) &disp->SampleCoverageARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SampleCoverageARB) {
        snprintf(symboln, sizeof(symboln), "%sSampleCoverageARB", symbol_prefix);
        procp = (void **) &disp->SampleCoverageARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage1DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage1D", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage1DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage1DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage1DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage1DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage2DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage2D", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage2DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage2DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage2DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage2DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage3DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage3D", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage3DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexImage3DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage3DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexImage3DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage1DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage1D", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage1DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage1DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage1DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage1DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage2DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage2D", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage2DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage2DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage2DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage2DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage3DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage3D", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage3DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompressedTexSubImage3DARB) {
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage3DARB", symbol_prefix);
        procp = (void **) &disp->CompressedTexSubImage3DARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCompressedTexImageARB) {
        snprintf(symboln, sizeof(symboln), "%sGetCompressedTexImage", symbol_prefix);
        procp = (void **) &disp->GetCompressedTexImageARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCompressedTexImageARB) {
        snprintf(symboln, sizeof(symboln), "%sGetCompressedTexImageARB", symbol_prefix);
        procp = (void **) &disp->GetCompressedTexImageARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DisableVertexAttribArrayARB) {
        snprintf(symboln, sizeof(symboln), "%sDisableVertexAttribArray", symbol_prefix);
        procp = (void **) &disp->DisableVertexAttribArrayARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DisableVertexAttribArrayARB) {
        snprintf(symboln, sizeof(symboln), "%sDisableVertexAttribArrayARB", symbol_prefix);
        procp = (void **) &disp->DisableVertexAttribArrayARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EnableVertexAttribArrayARB) {
        snprintf(symboln, sizeof(symboln), "%sEnableVertexAttribArray", symbol_prefix);
        procp = (void **) &disp->EnableVertexAttribArrayARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EnableVertexAttribArrayARB) {
        snprintf(symboln, sizeof(symboln), "%sEnableVertexAttribArrayARB", symbol_prefix);
        procp = (void **) &disp->EnableVertexAttribArrayARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramEnvParameterdvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramEnvParameterdvARB", symbol_prefix);
        procp = (void **) &disp->GetProgramEnvParameterdvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramEnvParameterfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramEnvParameterfvARB", symbol_prefix);
        procp = (void **) &disp->GetProgramEnvParameterfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramLocalParameterdvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramLocalParameterdvARB", symbol_prefix);
        procp = (void **) &disp->GetProgramLocalParameterdvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramLocalParameterfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramLocalParameterfvARB", symbol_prefix);
        procp = (void **) &disp->GetProgramLocalParameterfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramStringARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramStringARB", symbol_prefix);
        procp = (void **) &disp->GetProgramStringARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramivARB", symbol_prefix);
        procp = (void **) &disp->GetProgramivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribdvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdv", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribdvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribdvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdvARB", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribdvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfv", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfvARB", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribiv", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribivARB", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4dARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4dARB", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4dARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4dNV", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4dvARB", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4dvNV", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4fARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4fARB", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4fARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4fNV", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4fvARB", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameter4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4fvNV", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameter4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramLocalParameter4dARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4dARB", symbol_prefix);
        procp = (void **) &disp->ProgramLocalParameter4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramLocalParameter4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4dvARB", symbol_prefix);
        procp = (void **) &disp->ProgramLocalParameter4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramLocalParameter4fARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4fARB", symbol_prefix);
        procp = (void **) &disp->ProgramLocalParameter4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramLocalParameter4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4fvARB", symbol_prefix);
        procp = (void **) &disp->ProgramLocalParameter4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramStringARB) {
        snprintf(symboln, sizeof(symboln), "%sProgramStringARB", symbol_prefix);
        procp = (void **) &disp->ProgramStringARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1d", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1f", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1s", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1svARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2d", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2f", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2s", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2svARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3d", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3f", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3s", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3svARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NbvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nbv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NbvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NbvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NbvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NbvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Niv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NivARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NsvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nsv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NsvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NsvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NsvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NsvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NubARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nub", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NubARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NubARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NubARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NubARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NubvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nubv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NubvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NubvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NubvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NubvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NuivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nuiv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NuivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NuivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NuivARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NuivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NusvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nusv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NusvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4NusvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NusvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4NusvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4bvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4bv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4bvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4bvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4bvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4bvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4d", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4f", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4iv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ivARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4s", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4sARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4sARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4svARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4svARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4svARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ubvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ubvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ubvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ubvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4uivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4uiv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4uivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4uivARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4uivARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4uivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4usvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4usv", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4usvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4usvARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4usvARB", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4usvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribPointerARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointer", symbol_prefix);
        procp = (void **) &disp->VertexAttribPointerARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribPointerARB) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointerARB", symbol_prefix);
        procp = (void **) &disp->VertexAttribPointerARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sBindBuffer", symbol_prefix);
        procp = (void **) &disp->BindBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sBindBufferARB", symbol_prefix);
        procp = (void **) &disp->BindBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BufferDataARB) {
        snprintf(symboln, sizeof(symboln), "%sBufferData", symbol_prefix);
        procp = (void **) &disp->BufferDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BufferDataARB) {
        snprintf(symboln, sizeof(symboln), "%sBufferDataARB", symbol_prefix);
        procp = (void **) &disp->BufferDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BufferSubDataARB) {
        snprintf(symboln, sizeof(symboln), "%sBufferSubData", symbol_prefix);
        procp = (void **) &disp->BufferSubDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BufferSubDataARB) {
        snprintf(symboln, sizeof(symboln), "%sBufferSubDataARB", symbol_prefix);
        procp = (void **) &disp->BufferSubDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sDeleteBuffers", symbol_prefix);
        procp = (void **) &disp->DeleteBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sDeleteBuffersARB", symbol_prefix);
        procp = (void **) &disp->DeleteBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sGenBuffers", symbol_prefix);
        procp = (void **) &disp->GenBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sGenBuffersARB", symbol_prefix);
        procp = (void **) &disp->GenBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferParameterivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferParameteriv", symbol_prefix);
        procp = (void **) &disp->GetBufferParameterivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferParameterivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferParameterivARB", symbol_prefix);
        procp = (void **) &disp->GetBufferParameterivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferPointervARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferPointerv", symbol_prefix);
        procp = (void **) &disp->GetBufferPointervARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferPointervARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferPointervARB", symbol_prefix);
        procp = (void **) &disp->GetBufferPointervARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferSubDataARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferSubData", symbol_prefix);
        procp = (void **) &disp->GetBufferSubDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetBufferSubDataARB) {
        snprintf(symboln, sizeof(symboln), "%sGetBufferSubDataARB", symbol_prefix);
        procp = (void **) &disp->GetBufferSubDataARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sIsBuffer", symbol_prefix);
        procp = (void **) &disp->IsBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sIsBufferARB", symbol_prefix);
        procp = (void **) &disp->IsBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sMapBuffer", symbol_prefix);
        procp = (void **) &disp->MapBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MapBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sMapBufferARB", symbol_prefix);
        procp = (void **) &disp->MapBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UnmapBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sUnmapBuffer", symbol_prefix);
        procp = (void **) &disp->UnmapBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UnmapBufferARB) {
        snprintf(symboln, sizeof(symboln), "%sUnmapBufferARB", symbol_prefix);
        procp = (void **) &disp->UnmapBufferARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BeginQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sBeginQuery", symbol_prefix);
        procp = (void **) &disp->BeginQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BeginQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sBeginQueryARB", symbol_prefix);
        procp = (void **) &disp->BeginQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteQueriesARB) {
        snprintf(symboln, sizeof(symboln), "%sDeleteQueries", symbol_prefix);
        procp = (void **) &disp->DeleteQueriesARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteQueriesARB) {
        snprintf(symboln, sizeof(symboln), "%sDeleteQueriesARB", symbol_prefix);
        procp = (void **) &disp->DeleteQueriesARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EndQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sEndQuery", symbol_prefix);
        procp = (void **) &disp->EndQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EndQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sEndQueryARB", symbol_prefix);
        procp = (void **) &disp->EndQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenQueriesARB) {
        snprintf(symboln, sizeof(symboln), "%sGenQueries", symbol_prefix);
        procp = (void **) &disp->GenQueriesARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenQueriesARB) {
        snprintf(symboln, sizeof(symboln), "%sGenQueriesARB", symbol_prefix);
        procp = (void **) &disp->GenQueriesARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjectivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectiv", symbol_prefix);
        procp = (void **) &disp->GetQueryObjectivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjectivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectivARB", symbol_prefix);
        procp = (void **) &disp->GetQueryObjectivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjectuivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectuiv", symbol_prefix);
        procp = (void **) &disp->GetQueryObjectuivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjectuivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectuivARB", symbol_prefix);
        procp = (void **) &disp->GetQueryObjectuivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryiv", symbol_prefix);
        procp = (void **) &disp->GetQueryivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryivARB", symbol_prefix);
        procp = (void **) &disp->GetQueryivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sIsQuery", symbol_prefix);
        procp = (void **) &disp->IsQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsQueryARB) {
        snprintf(symboln, sizeof(symboln), "%sIsQueryARB", symbol_prefix);
        procp = (void **) &disp->IsQueryARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AttachObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sAttachObjectARB", symbol_prefix);
        procp = (void **) &disp->AttachObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompileShaderARB) {
        snprintf(symboln, sizeof(symboln), "%sCompileShader", symbol_prefix);
        procp = (void **) &disp->CompileShaderARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CompileShaderARB) {
        snprintf(symboln, sizeof(symboln), "%sCompileShaderARB", symbol_prefix);
        procp = (void **) &disp->CompileShaderARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CreateProgramObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sCreateProgramObjectARB", symbol_prefix);
        procp = (void **) &disp->CreateProgramObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CreateShaderObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sCreateShaderObjectARB", symbol_prefix);
        procp = (void **) &disp->CreateShaderObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sDeleteObjectARB", symbol_prefix);
        procp = (void **) &disp->DeleteObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DetachObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sDetachObjectARB", symbol_prefix);
        procp = (void **) &disp->DetachObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetActiveUniformARB) {
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniform", symbol_prefix);
        procp = (void **) &disp->GetActiveUniformARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetActiveUniformARB) {
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformARB", symbol_prefix);
        procp = (void **) &disp->GetActiveUniformARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetAttachedObjectsARB) {
        snprintf(symboln, sizeof(symboln), "%sGetAttachedObjectsARB", symbol_prefix);
        procp = (void **) &disp->GetAttachedObjectsARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetHandleARB) {
        snprintf(symboln, sizeof(symboln), "%sGetHandleARB", symbol_prefix);
        procp = (void **) &disp->GetHandleARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetInfoLogARB) {
        snprintf(symboln, sizeof(symboln), "%sGetInfoLogARB", symbol_prefix);
        procp = (void **) &disp->GetInfoLogARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetObjectParameterfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetObjectParameterfvARB", symbol_prefix);
        procp = (void **) &disp->GetObjectParameterfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetObjectParameterivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetObjectParameterivARB", symbol_prefix);
        procp = (void **) &disp->GetObjectParameterivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetShaderSourceARB) {
        snprintf(symboln, sizeof(symboln), "%sGetShaderSource", symbol_prefix);
        procp = (void **) &disp->GetShaderSourceARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetShaderSourceARB) {
        snprintf(symboln, sizeof(symboln), "%sGetShaderSourceARB", symbol_prefix);
        procp = (void **) &disp->GetShaderSourceARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformLocation", symbol_prefix);
        procp = (void **) &disp->GetUniformLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformLocationARB", symbol_prefix);
        procp = (void **) &disp->GetUniformLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformfv", symbol_prefix);
        procp = (void **) &disp->GetUniformfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformfvARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformfvARB", symbol_prefix);
        procp = (void **) &disp->GetUniformfvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformiv", symbol_prefix);
        procp = (void **) &disp->GetUniformivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetUniformivARB) {
        snprintf(symboln, sizeof(symboln), "%sGetUniformivARB", symbol_prefix);
        procp = (void **) &disp->GetUniformivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LinkProgramARB) {
        snprintf(symboln, sizeof(symboln), "%sLinkProgram", symbol_prefix);
        procp = (void **) &disp->LinkProgramARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LinkProgramARB) {
        snprintf(symboln, sizeof(symboln), "%sLinkProgramARB", symbol_prefix);
        procp = (void **) &disp->LinkProgramARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ShaderSourceARB) {
        snprintf(symboln, sizeof(symboln), "%sShaderSource", symbol_prefix);
        procp = (void **) &disp->ShaderSourceARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ShaderSourceARB) {
        snprintf(symboln, sizeof(symboln), "%sShaderSourceARB", symbol_prefix);
        procp = (void **) &disp->ShaderSourceARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1f", symbol_prefix);
        procp = (void **) &disp->Uniform1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1fARB", symbol_prefix);
        procp = (void **) &disp->Uniform1fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1fv", symbol_prefix);
        procp = (void **) &disp->Uniform1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1fvARB", symbol_prefix);
        procp = (void **) &disp->Uniform1fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1i", symbol_prefix);
        procp = (void **) &disp->Uniform1iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1iARB", symbol_prefix);
        procp = (void **) &disp->Uniform1iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1iv", symbol_prefix);
        procp = (void **) &disp->Uniform1ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform1ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform1ivARB", symbol_prefix);
        procp = (void **) &disp->Uniform1ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2f", symbol_prefix);
        procp = (void **) &disp->Uniform2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2fARB", symbol_prefix);
        procp = (void **) &disp->Uniform2fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2fv", symbol_prefix);
        procp = (void **) &disp->Uniform2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2fvARB", symbol_prefix);
        procp = (void **) &disp->Uniform2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2i", symbol_prefix);
        procp = (void **) &disp->Uniform2iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2iARB", symbol_prefix);
        procp = (void **) &disp->Uniform2iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2iv", symbol_prefix);
        procp = (void **) &disp->Uniform2ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform2ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform2ivARB", symbol_prefix);
        procp = (void **) &disp->Uniform2ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3f", symbol_prefix);
        procp = (void **) &disp->Uniform3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3fARB", symbol_prefix);
        procp = (void **) &disp->Uniform3fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3fv", symbol_prefix);
        procp = (void **) &disp->Uniform3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3fvARB", symbol_prefix);
        procp = (void **) &disp->Uniform3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3i", symbol_prefix);
        procp = (void **) &disp->Uniform3iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3iARB", symbol_prefix);
        procp = (void **) &disp->Uniform3iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3iv", symbol_prefix);
        procp = (void **) &disp->Uniform3ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform3ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform3ivARB", symbol_prefix);
        procp = (void **) &disp->Uniform3ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4f", symbol_prefix);
        procp = (void **) &disp->Uniform4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4fARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4fARB", symbol_prefix);
        procp = (void **) &disp->Uniform4fARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4fv", symbol_prefix);
        procp = (void **) &disp->Uniform4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4fvARB", symbol_prefix);
        procp = (void **) &disp->Uniform4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4i", symbol_prefix);
        procp = (void **) &disp->Uniform4iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4iARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4iARB", symbol_prefix);
        procp = (void **) &disp->Uniform4iARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4iv", symbol_prefix);
        procp = (void **) &disp->Uniform4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->Uniform4ivARB) {
        snprintf(symboln, sizeof(symboln), "%sUniform4ivARB", symbol_prefix);
        procp = (void **) &disp->Uniform4ivARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix2fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2fvARB", symbol_prefix);
        procp = (void **) &disp->UniformMatrix2fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix3fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3fvARB", symbol_prefix);
        procp = (void **) &disp->UniformMatrix3fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4fv", symbol_prefix);
        procp = (void **) &disp->UniformMatrix4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UniformMatrix4fvARB) {
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4fvARB", symbol_prefix);
        procp = (void **) &disp->UniformMatrix4fvARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UseProgramObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sUseProgram", symbol_prefix);
        procp = (void **) &disp->UseProgramObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UseProgramObjectARB) {
        snprintf(symboln, sizeof(symboln), "%sUseProgramObjectARB", symbol_prefix);
        procp = (void **) &disp->UseProgramObjectARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ValidateProgramARB) {
        snprintf(symboln, sizeof(symboln), "%sValidateProgram", symbol_prefix);
        procp = (void **) &disp->ValidateProgramARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ValidateProgramARB) {
        snprintf(symboln, sizeof(symboln), "%sValidateProgramARB", symbol_prefix);
        procp = (void **) &disp->ValidateProgramARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindAttribLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sBindAttribLocation", symbol_prefix);
        procp = (void **) &disp->BindAttribLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindAttribLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sBindAttribLocationARB", symbol_prefix);
        procp = (void **) &disp->BindAttribLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetActiveAttribARB) {
        snprintf(symboln, sizeof(symboln), "%sGetActiveAttrib", symbol_prefix);
        procp = (void **) &disp->GetActiveAttribARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetActiveAttribARB) {
        snprintf(symboln, sizeof(symboln), "%sGetActiveAttribARB", symbol_prefix);
        procp = (void **) &disp->GetActiveAttribARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetAttribLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sGetAttribLocation", symbol_prefix);
        procp = (void **) &disp->GetAttribLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetAttribLocationARB) {
        snprintf(symboln, sizeof(symboln), "%sGetAttribLocationARB", symbol_prefix);
        procp = (void **) &disp->GetAttribLocationARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sDrawBuffers", symbol_prefix);
        procp = (void **) &disp->DrawBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersARB", symbol_prefix);
        procp = (void **) &disp->DrawBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DrawBuffersARB) {
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersATI", symbol_prefix);
        procp = (void **) &disp->DrawBuffersARB;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PolygonOffsetEXT) {
        snprintf(symboln, sizeof(symboln), "%sPolygonOffsetEXT", symbol_prefix);
        procp = (void **) &disp->PolygonOffsetEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPixelTexGenParameterfvSGIS) {
        snprintf(symboln, sizeof(symboln), "%sGetPixelTexGenParameterfvSGIS", symbol_prefix);
        procp = (void **) &disp->GetPixelTexGenParameterfvSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetPixelTexGenParameterivSGIS) {
        snprintf(symboln, sizeof(symboln), "%sGetPixelTexGenParameterivSGIS", symbol_prefix);
        procp = (void **) &disp->GetPixelTexGenParameterivSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTexGenParameterfSGIS) {
        snprintf(symboln, sizeof(symboln), "%sPixelTexGenParameterfSGIS", symbol_prefix);
        procp = (void **) &disp->PixelTexGenParameterfSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTexGenParameterfvSGIS) {
        snprintf(symboln, sizeof(symboln), "%sPixelTexGenParameterfvSGIS", symbol_prefix);
        procp = (void **) &disp->PixelTexGenParameterfvSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTexGenParameteriSGIS) {
        snprintf(symboln, sizeof(symboln), "%sPixelTexGenParameteriSGIS", symbol_prefix);
        procp = (void **) &disp->PixelTexGenParameteriSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTexGenParameterivSGIS) {
        snprintf(symboln, sizeof(symboln), "%sPixelTexGenParameterivSGIS", symbol_prefix);
        procp = (void **) &disp->PixelTexGenParameterivSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SampleMaskSGIS) {
        snprintf(symboln, sizeof(symboln), "%sSampleMaskSGIS", symbol_prefix);
        procp = (void **) &disp->SampleMaskSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SampleMaskSGIS) {
        snprintf(symboln, sizeof(symboln), "%sSampleMaskEXT", symbol_prefix);
        procp = (void **) &disp->SampleMaskSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SamplePatternSGIS) {
        snprintf(symboln, sizeof(symboln), "%sSamplePatternSGIS", symbol_prefix);
        procp = (void **) &disp->SamplePatternSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SamplePatternSGIS) {
        snprintf(symboln, sizeof(symboln), "%sSamplePatternEXT", symbol_prefix);
        procp = (void **) &disp->SamplePatternSGIS;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sColorPointerEXT", symbol_prefix);
        procp = (void **) &disp->ColorPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EdgeFlagPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagPointerEXT", symbol_prefix);
        procp = (void **) &disp->EdgeFlagPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IndexPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sIndexPointerEXT", symbol_prefix);
        procp = (void **) &disp->IndexPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->NormalPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sNormalPointerEXT", symbol_prefix);
        procp = (void **) &disp->NormalPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TexCoordPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sTexCoordPointerEXT", symbol_prefix);
        procp = (void **) &disp->TexCoordPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sVertexPointerEXT", symbol_prefix);
        procp = (void **) &disp->VertexPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterf", symbol_prefix);
        procp = (void **) &disp->PointParameterfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfARB", symbol_prefix);
        procp = (void **) &disp->PointParameterfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfEXT", symbol_prefix);
        procp = (void **) &disp->PointParameterfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfSGIS", symbol_prefix);
        procp = (void **) &disp->PointParameterfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfv", symbol_prefix);
        procp = (void **) &disp->PointParameterfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvARB", symbol_prefix);
        procp = (void **) &disp->PointParameterfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->PointParameterfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvSGIS", symbol_prefix);
        procp = (void **) &disp->PointParameterfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LockArraysEXT) {
        snprintf(symboln, sizeof(symboln), "%sLockArraysEXT", symbol_prefix);
        procp = (void **) &disp->LockArraysEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->UnlockArraysEXT) {
        snprintf(symboln, sizeof(symboln), "%sUnlockArraysEXT", symbol_prefix);
        procp = (void **) &disp->UnlockArraysEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3bEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3b", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3bEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3bEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3bEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3bvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3bvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3bvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bvEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3bvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3dEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3d", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3dEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3dEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3dEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3dvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3dvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3dvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dvEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3dvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3fEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3f", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3fEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3fEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3fEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3fvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3fvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3fvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fvEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3fvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3iEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3i", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3iEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3iEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3iEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3iEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ivEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3iv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ivEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ivEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3sEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3s", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3sEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3sEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3sEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3sEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3svEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3sv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3svEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3svEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3svEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3svEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ubEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ub", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ubEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ubEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ubEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ubvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ubvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3ubvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubvEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3ubvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3uiEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ui", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3uiEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3uiEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uiEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3uiEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3uivEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uiv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3uivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3uivEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uivEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3uivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3usEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3us", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3usEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3usEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3usEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3usvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usv", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3usvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColor3usvEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usvEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColor3usvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColorPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorPointer", symbol_prefix);
        procp = (void **) &disp->SecondaryColorPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SecondaryColorPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorPointerEXT", symbol_prefix);
        procp = (void **) &disp->SecondaryColorPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiDrawArraysEXT) {
        snprintf(symboln, sizeof(symboln), "%sMultiDrawArrays", symbol_prefix);
        procp = (void **) &disp->MultiDrawArraysEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiDrawArraysEXT) {
        snprintf(symboln, sizeof(symboln), "%sMultiDrawArraysEXT", symbol_prefix);
        procp = (void **) &disp->MultiDrawArraysEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiDrawElementsEXT) {
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElements", symbol_prefix);
        procp = (void **) &disp->MultiDrawElementsEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiDrawElementsEXT) {
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElementsEXT", symbol_prefix);
        procp = (void **) &disp->MultiDrawElementsEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordPointer", symbol_prefix);
        procp = (void **) &disp->FogCoordPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordPointerEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordPointerEXT", symbol_prefix);
        procp = (void **) &disp->FogCoordPointerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoorddEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordd", symbol_prefix);
        procp = (void **) &disp->FogCoorddEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoorddEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoorddEXT", symbol_prefix);
        procp = (void **) &disp->FogCoorddEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoorddvEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoorddv", symbol_prefix);
        procp = (void **) &disp->FogCoorddvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoorddvEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoorddvEXT", symbol_prefix);
        procp = (void **) &disp->FogCoorddvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordfEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordf", symbol_prefix);
        procp = (void **) &disp->FogCoordfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordfEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordfEXT", symbol_prefix);
        procp = (void **) &disp->FogCoordfEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordfv", symbol_prefix);
        procp = (void **) &disp->FogCoordfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FogCoordfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sFogCoordfvEXT", symbol_prefix);
        procp = (void **) &disp->FogCoordfvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PixelTexGenSGIX) {
        snprintf(symboln, sizeof(symboln), "%sPixelTexGenSGIX", symbol_prefix);
        procp = (void **) &disp->PixelTexGenSGIX;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendFuncSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparate", symbol_prefix);
        procp = (void **) &disp->BlendFuncSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendFuncSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateEXT", symbol_prefix);
        procp = (void **) &disp->BlendFuncSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendFuncSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateINGR", symbol_prefix);
        procp = (void **) &disp->BlendFuncSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FlushVertexArrayRangeNV) {
        snprintf(symboln, sizeof(symboln), "%sFlushVertexArrayRangeNV", symbol_prefix);
        procp = (void **) &disp->FlushVertexArrayRangeNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexArrayRangeNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexArrayRangeNV", symbol_prefix);
        procp = (void **) &disp->VertexArrayRangeNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerInputNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerInputNV", symbol_prefix);
        procp = (void **) &disp->CombinerInputNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerOutputNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerOutputNV", symbol_prefix);
        procp = (void **) &disp->CombinerOutputNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerParameterfNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerParameterfNV", symbol_prefix);
        procp = (void **) &disp->CombinerParameterfNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerParameterfvNV", symbol_prefix);
        procp = (void **) &disp->CombinerParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerParameteriNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerParameteriNV", symbol_prefix);
        procp = (void **) &disp->CombinerParameteriNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CombinerParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sCombinerParameterivNV", symbol_prefix);
        procp = (void **) &disp->CombinerParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FinalCombinerInputNV) {
        snprintf(symboln, sizeof(symboln), "%sFinalCombinerInputNV", symbol_prefix);
        procp = (void **) &disp->FinalCombinerInputNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCombinerInputParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetCombinerInputParameterfvNV", symbol_prefix);
        procp = (void **) &disp->GetCombinerInputParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCombinerInputParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetCombinerInputParameterivNV", symbol_prefix);
        procp = (void **) &disp->GetCombinerInputParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCombinerOutputParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetCombinerOutputParameterfvNV", symbol_prefix);
        procp = (void **) &disp->GetCombinerOutputParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetCombinerOutputParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetCombinerOutputParameterivNV", symbol_prefix);
        procp = (void **) &disp->GetCombinerOutputParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFinalCombinerInputParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetFinalCombinerInputParameterfvNV", symbol_prefix);
        procp = (void **) &disp->GetFinalCombinerInputParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFinalCombinerInputParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetFinalCombinerInputParameterivNV", symbol_prefix);
        procp = (void **) &disp->GetFinalCombinerInputParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ResizeBuffersMESA) {
        snprintf(symboln, sizeof(symboln), "%sResizeBuffersMESA", symbol_prefix);
        procp = (void **) &disp->ResizeBuffersMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2d", symbol_prefix);
        procp = (void **) &disp->WindowPos2dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dv", symbol_prefix);
        procp = (void **) &disp->WindowPos2dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dvARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2f", symbol_prefix);
        procp = (void **) &disp->WindowPos2fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fv", symbol_prefix);
        procp = (void **) &disp->WindowPos2fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fvARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2i", symbol_prefix);
        procp = (void **) &disp->WindowPos2iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iv", symbol_prefix);
        procp = (void **) &disp->WindowPos2ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2ivARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2ivMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2s", symbol_prefix);
        procp = (void **) &disp->WindowPos2sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sv", symbol_prefix);
        procp = (void **) &disp->WindowPos2svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2svARB", symbol_prefix);
        procp = (void **) &disp->WindowPos2svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos2svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos2svMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos2svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3d", symbol_prefix);
        procp = (void **) &disp->WindowPos3dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dv", symbol_prefix);
        procp = (void **) &disp->WindowPos3dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dvARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3f", symbol_prefix);
        procp = (void **) &disp->WindowPos3fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fv", symbol_prefix);
        procp = (void **) &disp->WindowPos3fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fvARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3i", symbol_prefix);
        procp = (void **) &disp->WindowPos3iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iv", symbol_prefix);
        procp = (void **) &disp->WindowPos3ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3ivARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3ivMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3s", symbol_prefix);
        procp = (void **) &disp->WindowPos3sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sv", symbol_prefix);
        procp = (void **) &disp->WindowPos3svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3svARB", symbol_prefix);
        procp = (void **) &disp->WindowPos3svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos3svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos3svMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos3svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4dMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4dMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4dMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4dvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4dvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4dvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4fMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4fMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4fMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4fvMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4fvMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4fvMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4iMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4iMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4iMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4ivMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4ivMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4ivMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4sMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4sMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4sMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->WindowPos4svMESA) {
        snprintf(symboln, sizeof(symboln), "%sWindowPos4svMESA", symbol_prefix);
        procp = (void **) &disp->WindowPos4svMESA;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiModeDrawArraysIBM) {
        snprintf(symboln, sizeof(symboln), "%sMultiModeDrawArraysIBM", symbol_prefix);
        procp = (void **) &disp->MultiModeDrawArraysIBM;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->MultiModeDrawElementsIBM) {
        snprintf(symboln, sizeof(symboln), "%sMultiModeDrawElementsIBM", symbol_prefix);
        procp = (void **) &disp->MultiModeDrawElementsIBM;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteFencesNV) {
        snprintf(symboln, sizeof(symboln), "%sDeleteFencesNV", symbol_prefix);
        procp = (void **) &disp->DeleteFencesNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FinishFenceNV) {
        snprintf(symboln, sizeof(symboln), "%sFinishFenceNV", symbol_prefix);
        procp = (void **) &disp->FinishFenceNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenFencesNV) {
        snprintf(symboln, sizeof(symboln), "%sGenFencesNV", symbol_prefix);
        procp = (void **) &disp->GenFencesNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFenceivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetFenceivNV", symbol_prefix);
        procp = (void **) &disp->GetFenceivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsFenceNV) {
        snprintf(symboln, sizeof(symboln), "%sIsFenceNV", symbol_prefix);
        procp = (void **) &disp->IsFenceNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SetFenceNV) {
        snprintf(symboln, sizeof(symboln), "%sSetFenceNV", symbol_prefix);
        procp = (void **) &disp->SetFenceNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TestFenceNV) {
        snprintf(symboln, sizeof(symboln), "%sTestFenceNV", symbol_prefix);
        procp = (void **) &disp->TestFenceNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AreProgramsResidentNV) {
        snprintf(symboln, sizeof(symboln), "%sAreProgramsResidentNV", symbol_prefix);
        procp = (void **) &disp->AreProgramsResidentNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sBindProgramARB", symbol_prefix);
        procp = (void **) &disp->BindProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sBindProgramNV", symbol_prefix);
        procp = (void **) &disp->BindProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteProgramsNV) {
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramsARB", symbol_prefix);
        procp = (void **) &disp->DeleteProgramsNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteProgramsNV) {
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramsNV", symbol_prefix);
        procp = (void **) &disp->DeleteProgramsNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ExecuteProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sExecuteProgramNV", symbol_prefix);
        procp = (void **) &disp->ExecuteProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenProgramsNV) {
        snprintf(symboln, sizeof(symboln), "%sGenProgramsARB", symbol_prefix);
        procp = (void **) &disp->GenProgramsNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenProgramsNV) {
        snprintf(symboln, sizeof(symboln), "%sGenProgramsNV", symbol_prefix);
        procp = (void **) &disp->GenProgramsNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramParameterdvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramParameterdvNV", symbol_prefix);
        procp = (void **) &disp->GetProgramParameterdvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramParameterfvNV", symbol_prefix);
        procp = (void **) &disp->GetProgramParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramStringNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramStringNV", symbol_prefix);
        procp = (void **) &disp->GetProgramStringNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramivNV", symbol_prefix);
        procp = (void **) &disp->GetProgramivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetTrackMatrixivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetTrackMatrixivNV", symbol_prefix);
        procp = (void **) &disp->GetTrackMatrixivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribPointervNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointerv", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribPointervNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribPointervNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointervARB", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribPointervNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribPointervNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointervNV", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribPointervNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribdvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdvNV", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribdvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfvNV", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetVertexAttribivNV) {
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribivNV", symbol_prefix);
        procp = (void **) &disp->GetVertexAttribivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sIsProgramARB", symbol_prefix);
        procp = (void **) &disp->IsProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sIsProgramNV", symbol_prefix);
        procp = (void **) &disp->IsProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->LoadProgramNV) {
        snprintf(symboln, sizeof(symboln), "%sLoadProgramNV", symbol_prefix);
        procp = (void **) &disp->LoadProgramNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramParameters4dvNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameters4dvNV", symbol_prefix);
        procp = (void **) &disp->ProgramParameters4dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramParameters4fvNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramParameters4fvNV", symbol_prefix);
        procp = (void **) &disp->ProgramParameters4fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RequestResidentProgramsNV) {
        snprintf(symboln, sizeof(symboln), "%sRequestResidentProgramsNV", symbol_prefix);
        procp = (void **) &disp->RequestResidentProgramsNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->TrackMatrixNV) {
        snprintf(symboln, sizeof(symboln), "%sTrackMatrixNV", symbol_prefix);
        procp = (void **) &disp->TrackMatrixNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1sNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1sNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib1svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib1svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2sNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2sNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib2svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib2svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3sNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3sNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib3svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib3svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4sNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4sNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ubNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ubNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttrib4ubvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttrib4ubvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribPointerNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointerNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribPointerNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs1dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs1dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs1fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs1fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs1svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs1svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs2dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs2dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs2fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs2fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs2svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs2svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs3dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs3dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs3fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs3fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs3svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs3svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs4dvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4dvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs4dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs4fvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4fvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs4fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs4svNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4svNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs4svNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->VertexAttribs4ubvNV) {
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4ubvNV", symbol_prefix);
        procp = (void **) &disp->VertexAttribs4ubvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AlphaFragmentOp1ATI) {
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp1ATI", symbol_prefix);
        procp = (void **) &disp->AlphaFragmentOp1ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AlphaFragmentOp2ATI) {
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp2ATI", symbol_prefix);
        procp = (void **) &disp->AlphaFragmentOp2ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->AlphaFragmentOp3ATI) {
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp3ATI", symbol_prefix);
        procp = (void **) &disp->AlphaFragmentOp3ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BeginFragmentShaderATI) {
        snprintf(symboln, sizeof(symboln), "%sBeginFragmentShaderATI", symbol_prefix);
        procp = (void **) &disp->BeginFragmentShaderATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindFragmentShaderATI) {
        snprintf(symboln, sizeof(symboln), "%sBindFragmentShaderATI", symbol_prefix);
        procp = (void **) &disp->BindFragmentShaderATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorFragmentOp1ATI) {
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp1ATI", symbol_prefix);
        procp = (void **) &disp->ColorFragmentOp1ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorFragmentOp2ATI) {
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp2ATI", symbol_prefix);
        procp = (void **) &disp->ColorFragmentOp2ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ColorFragmentOp3ATI) {
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp3ATI", symbol_prefix);
        procp = (void **) &disp->ColorFragmentOp3ATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteFragmentShaderATI) {
        snprintf(symboln, sizeof(symboln), "%sDeleteFragmentShaderATI", symbol_prefix);
        procp = (void **) &disp->DeleteFragmentShaderATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->EndFragmentShaderATI) {
        snprintf(symboln, sizeof(symboln), "%sEndFragmentShaderATI", symbol_prefix);
        procp = (void **) &disp->EndFragmentShaderATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenFragmentShadersATI) {
        snprintf(symboln, sizeof(symboln), "%sGenFragmentShadersATI", symbol_prefix);
        procp = (void **) &disp->GenFragmentShadersATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PassTexCoordATI) {
        snprintf(symboln, sizeof(symboln), "%sPassTexCoordATI", symbol_prefix);
        procp = (void **) &disp->PassTexCoordATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SampleMapATI) {
        snprintf(symboln, sizeof(symboln), "%sSampleMapATI", symbol_prefix);
        procp = (void **) &disp->SampleMapATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->SetFragmentShaderConstantATI) {
        snprintf(symboln, sizeof(symboln), "%sSetFragmentShaderConstantATI", symbol_prefix);
        procp = (void **) &disp->SetFragmentShaderConstantATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameteriNV) {
        snprintf(symboln, sizeof(symboln), "%sPointParameteri", symbol_prefix);
        procp = (void **) &disp->PointParameteriNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameteriNV) {
        snprintf(symboln, sizeof(symboln), "%sPointParameteriNV", symbol_prefix);
        procp = (void **) &disp->PointParameteriNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sPointParameteriv", symbol_prefix);
        procp = (void **) &disp->PointParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->PointParameterivNV) {
        snprintf(symboln, sizeof(symboln), "%sPointParameterivNV", symbol_prefix);
        procp = (void **) &disp->PointParameterivNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ActiveStencilFaceEXT) {
        snprintf(symboln, sizeof(symboln), "%sActiveStencilFaceEXT", symbol_prefix);
        procp = (void **) &disp->ActiveStencilFaceEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindVertexArrayAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sBindVertexArrayAPPLE", symbol_prefix);
        procp = (void **) &disp->BindVertexArrayAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteVertexArraysAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sDeleteVertexArrays", symbol_prefix);
        procp = (void **) &disp->DeleteVertexArraysAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteVertexArraysAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sDeleteVertexArraysAPPLE", symbol_prefix);
        procp = (void **) &disp->DeleteVertexArraysAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenVertexArraysAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sGenVertexArraysAPPLE", symbol_prefix);
        procp = (void **) &disp->GenVertexArraysAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsVertexArrayAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sIsVertexArray", symbol_prefix);
        procp = (void **) &disp->IsVertexArrayAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsVertexArrayAPPLE) {
        snprintf(symboln, sizeof(symboln), "%sIsVertexArrayAPPLE", symbol_prefix);
        procp = (void **) &disp->IsVertexArrayAPPLE;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramNamedParameterdvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramNamedParameterdvNV", symbol_prefix);
        procp = (void **) &disp->GetProgramNamedParameterdvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetProgramNamedParameterfvNV) {
        snprintf(symboln, sizeof(symboln), "%sGetProgramNamedParameterfvNV", symbol_prefix);
        procp = (void **) &disp->GetProgramNamedParameterfvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramNamedParameter4dNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4dNV", symbol_prefix);
        procp = (void **) &disp->ProgramNamedParameter4dNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramNamedParameter4dvNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4dvNV", symbol_prefix);
        procp = (void **) &disp->ProgramNamedParameter4dvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramNamedParameter4fNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4fNV", symbol_prefix);
        procp = (void **) &disp->ProgramNamedParameter4fNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramNamedParameter4fvNV) {
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4fvNV", symbol_prefix);
        procp = (void **) &disp->ProgramNamedParameter4fvNV;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DepthBoundsEXT) {
        snprintf(symboln, sizeof(symboln), "%sDepthBoundsEXT", symbol_prefix);
        procp = (void **) &disp->DepthBoundsEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendEquationSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparate", symbol_prefix);
        procp = (void **) &disp->BlendEquationSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendEquationSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateEXT", symbol_prefix);
        procp = (void **) &disp->BlendEquationSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlendEquationSeparateEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateATI", symbol_prefix);
        procp = (void **) &disp->BlendEquationSeparateEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBindFramebuffer", symbol_prefix);
        procp = (void **) &disp->BindFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBindFramebufferEXT", symbol_prefix);
        procp = (void **) &disp->BindFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBindRenderbuffer", symbol_prefix);
        procp = (void **) &disp->BindRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BindRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBindRenderbufferEXT", symbol_prefix);
        procp = (void **) &disp->BindRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CheckFramebufferStatusEXT) {
        snprintf(symboln, sizeof(symboln), "%sCheckFramebufferStatus", symbol_prefix);
        procp = (void **) &disp->CheckFramebufferStatusEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CheckFramebufferStatusEXT) {
        snprintf(symboln, sizeof(symboln), "%sCheckFramebufferStatusEXT", symbol_prefix);
        procp = (void **) &disp->CheckFramebufferStatusEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteFramebuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sDeleteFramebuffers", symbol_prefix);
        procp = (void **) &disp->DeleteFramebuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteFramebuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sDeleteFramebuffersEXT", symbol_prefix);
        procp = (void **) &disp->DeleteFramebuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteRenderbuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sDeleteRenderbuffers", symbol_prefix);
        procp = (void **) &disp->DeleteRenderbuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->DeleteRenderbuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sDeleteRenderbuffersEXT", symbol_prefix);
        procp = (void **) &disp->DeleteRenderbuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferRenderbuffer", symbol_prefix);
        procp = (void **) &disp->FramebufferRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferRenderbufferEXT", symbol_prefix);
        procp = (void **) &disp->FramebufferRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture1DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture1D", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture1DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture1DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture1DEXT", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture1DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture2DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture2D", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture2DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture2DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture2DEXT", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture2DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture3DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture3D", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture3DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTexture3DEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture3DEXT", symbol_prefix);
        procp = (void **) &disp->FramebufferTexture3DEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenFramebuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenFramebuffers", symbol_prefix);
        procp = (void **) &disp->GenFramebuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenFramebuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenFramebuffersEXT", symbol_prefix);
        procp = (void **) &disp->GenFramebuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenRenderbuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenRenderbuffers", symbol_prefix);
        procp = (void **) &disp->GenRenderbuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenRenderbuffersEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenRenderbuffersEXT", symbol_prefix);
        procp = (void **) &disp->GenRenderbuffersEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenerateMipmapEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenerateMipmap", symbol_prefix);
        procp = (void **) &disp->GenerateMipmapEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GenerateMipmapEXT) {
        snprintf(symboln, sizeof(symboln), "%sGenerateMipmapEXT", symbol_prefix);
        procp = (void **) &disp->GenerateMipmapEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFramebufferAttachmentParameterivEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetFramebufferAttachmentParameteriv", symbol_prefix);
        procp = (void **) &disp->GetFramebufferAttachmentParameterivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetFramebufferAttachmentParameterivEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetFramebufferAttachmentParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetFramebufferAttachmentParameterivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetRenderbufferParameterivEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetRenderbufferParameteriv", symbol_prefix);
        procp = (void **) &disp->GetRenderbufferParameterivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetRenderbufferParameterivEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetRenderbufferParameterivEXT", symbol_prefix);
        procp = (void **) &disp->GetRenderbufferParameterivEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sIsFramebuffer", symbol_prefix);
        procp = (void **) &disp->IsFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sIsFramebufferEXT", symbol_prefix);
        procp = (void **) &disp->IsFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sIsRenderbuffer", symbol_prefix);
        procp = (void **) &disp->IsRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->IsRenderbufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sIsRenderbufferEXT", symbol_prefix);
        procp = (void **) &disp->IsRenderbufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RenderbufferStorageEXT) {
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorage", symbol_prefix);
        procp = (void **) &disp->RenderbufferStorageEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->RenderbufferStorageEXT) {
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorageEXT", symbol_prefix);
        procp = (void **) &disp->RenderbufferStorageEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlitFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlitFramebuffer", symbol_prefix);
        procp = (void **) &disp->BlitFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->BlitFramebufferEXT) {
        snprintf(symboln, sizeof(symboln), "%sBlitFramebufferEXT", symbol_prefix);
        procp = (void **) &disp->BlitFramebufferEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTextureLayerEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureLayer", symbol_prefix);
        procp = (void **) &disp->FramebufferTextureLayerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->FramebufferTextureLayerEXT) {
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureLayerEXT", symbol_prefix);
        procp = (void **) &disp->FramebufferTextureLayerEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->StencilFuncSeparateATI) {
        snprintf(symboln, sizeof(symboln), "%sStencilFuncSeparateATI", symbol_prefix);
        procp = (void **) &disp->StencilFuncSeparateATI;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramEnvParameters4fvEXT) {
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameters4fvEXT", symbol_prefix);
        procp = (void **) &disp->ProgramEnvParameters4fvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->ProgramLocalParameters4fvEXT) {
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameters4fvEXT", symbol_prefix);
        procp = (void **) &disp->ProgramLocalParameters4fvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjecti64vEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjecti64vEXT", symbol_prefix);
        procp = (void **) &disp->GetQueryObjecti64vEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->GetQueryObjectui64vEXT) {
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectui64vEXT", symbol_prefix);
        procp = (void **) &disp->GetQueryObjectui64vEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CullParameterdvEXT) {
        snprintf(symboln, sizeof(symboln), "%sCullParameterdvEXT", symbol_prefix);
        procp = (void **) &disp->CullParameterdvEXT;
        *procp = dlsym(handle, symboln);
    }


    if(!disp->CullParameterfvEXT) {
        snprintf(symboln, sizeof(symboln), "%sCullParameterfvEXT", symbol_prefix);
        procp = (void **) &disp->CullParameterfvEXT;
        *procp = dlsym(handle, symboln);
    }


    __glapi_gentable_set_remaining_noop(disp);

    return disp;
}

