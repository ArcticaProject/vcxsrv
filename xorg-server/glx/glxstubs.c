/*
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Adam Jackson <ajax@redhat.com>
 */

/*
 * Redirection stubs for things that we call by name but that aren't exported
 * from libGL by name.  Strictly speaking this list should be a lot longer,
 * but this is enough to get us linking against contemporary Mesa.
 */

#include <inttypes.h>
#include "glxserver.h"

#define thunk(name, type, call_args, ...) \
    _X_HIDDEN void name(__VA_ARGS__) { \
	static type proc; \
	if (!proc) proc = __glGetProcAddress(#name); \
	proc call_args; \
    }

thunk(glSampleMaskSGIS, PFNGLSAMPLEMASKSGISPROC,
      (value, invert), GLclampf value, GLboolean invert)

thunk(glSamplePatternSGIS, PFNGLSAMPLEPATTERNSGISPROC,
      (pattern), GLenum pattern)

thunk(glActiveStencilFaceEXT, PFNGLACTIVESTENCILFACEEXTPROC,
      (face), GLenum face)
