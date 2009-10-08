/*
 * Mesa 3-D graphics library
 * Version:  6.3
 *
 * Copyright (C) 2005  Tungsten Graphics   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * TUNGSTEN GRAPHICS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef T_VP_BUILD_H
#define T_VP_BUILD_H

#include "main/mtypes.h"

#define TNL_FIXED_FUNCTION_STATE_FLAGS (_NEW_PROGRAM |		\
					_NEW_LIGHT |		\
					_NEW_TEXTURE |		\
					_NEW_TEXTURE_MATRIX |		\
					_NEW_TRANSFORM |	\
					_NEW_FOG |		\
					_NEW_POINT)

extern void _tnl_UpdateFixedFunctionProgram( GLcontext *ctx );

#endif
