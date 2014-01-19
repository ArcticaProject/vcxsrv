/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/


#ifndef ST_DEBUG_H
#define ST_DEBUG_H

#include "pipe/p_compiler.h"
#include "util/u_debug.h"

extern void
st_print_current(void);


#define DEBUG_MESA      0x1
#define DEBUG_TGSI      0x2
#define DEBUG_CONSTANTS 0x4
#define DEBUG_PIPE      0x8
#define DEBUG_TEX       0x10
#define DEBUG_FALLBACK  0x20
#define DEBUG_QUERY     0x40
#define DEBUG_SCREEN    0x80
#define DEBUG_DRAW      0x100
#define DEBUG_BUFFER    0x200

#ifdef DEBUG
extern int ST_DEBUG;
#define DBSTR(x) x
#else
#define ST_DEBUG 0
#define DBSTR(x) ""
#endif

void st_debug_init( void );

static INLINE void
ST_DBG( unsigned flag, const char *fmt, ... )
{
    if (ST_DEBUG & flag)
    {
        va_list args;

        va_start( args, fmt );
        debug_vprintf( fmt, args );
        va_end( args );
    }
}


#endif /* ST_DEBUG_H */
