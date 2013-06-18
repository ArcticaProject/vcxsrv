/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2010  Brian Paul   All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef TEXCOMPRESS_CPAL_H
#define TEXCOMPRESS_CPAL_H


#include "main/glheader.h"
extern void
_mesa_cpal_compressed_teximage2d(GLenum target, GLint level,
				 GLenum internalFormat,
				 GLsizei width, GLsizei height,
				 GLsizei imageSize, const void *palette);

extern unsigned
_mesa_cpal_compressed_size(int level, GLenum internalFormat,
			   unsigned width, unsigned height);


#endif /* TEXCOMPRESS_CPAL_H */
