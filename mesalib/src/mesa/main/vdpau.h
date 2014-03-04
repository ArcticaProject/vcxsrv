/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#ifndef VDPAU_H
#define VDPAU_H

extern void GLAPIENTRY
_mesa_VDPAUInitNV(const GLvoid *vdpDevice, const GLvoid *getProcAddress);

extern void GLAPIENTRY
_mesa_VDPAUFiniNV(void);

extern GLintptr GLAPIENTRY
_mesa_VDPAURegisterVideoSurfaceNV(const GLvoid *vdpSurface, GLenum target,
                                  GLsizei numTextureNames,
                                  const GLuint *textureNames);

extern GLintptr GLAPIENTRY
_mesa_VDPAURegisterOutputSurfaceNV(const GLvoid *vdpSurface, GLenum target,
                                   GLsizei numTextureNames,
                                   const GLuint *textureNames);

extern GLboolean GLAPIENTRY
_mesa_VDPAUIsSurfaceNV(GLintptr surface);

extern void GLAPIENTRY
_mesa_VDPAUUnregisterSurfaceNV(GLintptr surface);

extern void GLAPIENTRY
_mesa_VDPAUGetSurfaceivNV(GLintptr surface, GLenum pname, GLsizei bufSize,
                          GLsizei *length, GLint *values);

extern void GLAPIENTRY
_mesa_VDPAUSurfaceAccessNV(GLintptr surface, GLenum access);

extern void GLAPIENTRY
_mesa_VDPAUMapSurfacesNV(GLsizei numSurfaces, const GLintptr *surfaces);

extern void GLAPIENTRY
_mesa_VDPAUUnmapSurfacesNV(GLsizei numSurfaces, const GLintptr *surfaces);

#endif /* VDPAU_H */
