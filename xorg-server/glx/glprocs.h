/* DO NOT EDIT - This file generated automatically by gl_procs.py (from Mesa) script */

/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * (C) Copyright IBM Corporation 2004, 2006
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


/* This file is only included by glapi.c and is used for
 * the GetProcAddress() function
 */

typedef struct {
    GLint Name_offset;
#if defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING)
    _glapi_proc Address;
#endif
    GLuint Offset;
} glprocs_table_t;

#if   !defined(NEED_FUNCTION_POINTER) && !defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , o }
#elif  defined(NEED_FUNCTION_POINTER) && !defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f1 , o }
#elif  defined(NEED_FUNCTION_POINTER) &&  defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f2 , o }
#elif !defined(NEED_FUNCTION_POINTER) &&  defined(GLX_INDIRECT_RENDERING)
#  define NAME_FUNC_OFFSET(n,f1,f2,f3,o) { n , (_glapi_proc) f3 , o }
#endif



static const char gl_string_table[] =
    "glNewList\0"
    "glEndList\0"
    "glCallList\0"
    "glCallLists\0"
    "glDeleteLists\0"
    "glGenLists\0"
    "glListBase\0"
    "glBegin\0"
    "glBitmap\0"
    "glColor3b\0"
    "glColor3bv\0"
    "glColor3d\0"
    "glColor3dv\0"
    "glColor3f\0"
    "glColor3fv\0"
    "glColor3i\0"
    "glColor3iv\0"
    "glColor3s\0"
    "glColor3sv\0"
    "glColor3ub\0"
    "glColor3ubv\0"
    "glColor3ui\0"
    "glColor3uiv\0"
    "glColor3us\0"
    "glColor3usv\0"
    "glColor4b\0"
    "glColor4bv\0"
    "glColor4d\0"
    "glColor4dv\0"
    "glColor4f\0"
    "glColor4fv\0"
    "glColor4i\0"
    "glColor4iv\0"
    "glColor4s\0"
    "glColor4sv\0"
    "glColor4ub\0"
    "glColor4ubv\0"
    "glColor4ui\0"
    "glColor4uiv\0"
    "glColor4us\0"
    "glColor4usv\0"
    "glEdgeFlag\0"
    "glEdgeFlagv\0"
    "glEnd\0"
    "glIndexd\0"
    "glIndexdv\0"
    "glIndexf\0"
    "glIndexfv\0"
    "glIndexi\0"
    "glIndexiv\0"
    "glIndexs\0"
    "glIndexsv\0"
    "glNormal3b\0"
    "glNormal3bv\0"
    "glNormal3d\0"
    "glNormal3dv\0"
    "glNormal3f\0"
    "glNormal3fv\0"
    "glNormal3i\0"
    "glNormal3iv\0"
    "glNormal3s\0"
    "glNormal3sv\0"
    "glRasterPos2d\0"
    "glRasterPos2dv\0"
    "glRasterPos2f\0"
    "glRasterPos2fv\0"
    "glRasterPos2i\0"
    "glRasterPos2iv\0"
    "glRasterPos2s\0"
    "glRasterPos2sv\0"
    "glRasterPos3d\0"
    "glRasterPos3dv\0"
    "glRasterPos3f\0"
    "glRasterPos3fv\0"
    "glRasterPos3i\0"
    "glRasterPos3iv\0"
    "glRasterPos3s\0"
    "glRasterPos3sv\0"
    "glRasterPos4d\0"
    "glRasterPos4dv\0"
    "glRasterPos4f\0"
    "glRasterPos4fv\0"
    "glRasterPos4i\0"
    "glRasterPos4iv\0"
    "glRasterPos4s\0"
    "glRasterPos4sv\0"
    "glRectd\0"
    "glRectdv\0"
    "glRectf\0"
    "glRectfv\0"
    "glRecti\0"
    "glRectiv\0"
    "glRects\0"
    "glRectsv\0"
    "glTexCoord1d\0"
    "glTexCoord1dv\0"
    "glTexCoord1f\0"
    "glTexCoord1fv\0"
    "glTexCoord1i\0"
    "glTexCoord1iv\0"
    "glTexCoord1s\0"
    "glTexCoord1sv\0"
    "glTexCoord2d\0"
    "glTexCoord2dv\0"
    "glTexCoord2f\0"
    "glTexCoord2fv\0"
    "glTexCoord2i\0"
    "glTexCoord2iv\0"
    "glTexCoord2s\0"
    "glTexCoord2sv\0"
    "glTexCoord3d\0"
    "glTexCoord3dv\0"
    "glTexCoord3f\0"
    "glTexCoord3fv\0"
    "glTexCoord3i\0"
    "glTexCoord3iv\0"
    "glTexCoord3s\0"
    "glTexCoord3sv\0"
    "glTexCoord4d\0"
    "glTexCoord4dv\0"
    "glTexCoord4f\0"
    "glTexCoord4fv\0"
    "glTexCoord4i\0"
    "glTexCoord4iv\0"
    "glTexCoord4s\0"
    "glTexCoord4sv\0"
    "glVertex2d\0"
    "glVertex2dv\0"
    "glVertex2f\0"
    "glVertex2fv\0"
    "glVertex2i\0"
    "glVertex2iv\0"
    "glVertex2s\0"
    "glVertex2sv\0"
    "glVertex3d\0"
    "glVertex3dv\0"
    "glVertex3f\0"
    "glVertex3fv\0"
    "glVertex3i\0"
    "glVertex3iv\0"
    "glVertex3s\0"
    "glVertex3sv\0"
    "glVertex4d\0"
    "glVertex4dv\0"
    "glVertex4f\0"
    "glVertex4fv\0"
    "glVertex4i\0"
    "glVertex4iv\0"
    "glVertex4s\0"
    "glVertex4sv\0"
    "glClipPlane\0"
    "glColorMaterial\0"
    "glCullFace\0"
    "glFogf\0"
    "glFogfv\0"
    "glFogi\0"
    "glFogiv\0"
    "glFrontFace\0"
    "glHint\0"
    "glLightf\0"
    "glLightfv\0"
    "glLighti\0"
    "glLightiv\0"
    "glLightModelf\0"
    "glLightModelfv\0"
    "glLightModeli\0"
    "glLightModeliv\0"
    "glLineStipple\0"
    "glLineWidth\0"
    "glMaterialf\0"
    "glMaterialfv\0"
    "glMateriali\0"
    "glMaterialiv\0"
    "glPointSize\0"
    "glPolygonMode\0"
    "glPolygonStipple\0"
    "glScissor\0"
    "glShadeModel\0"
    "glTexParameterf\0"
    "glTexParameterfv\0"
    "glTexParameteri\0"
    "glTexParameteriv\0"
    "glTexImage1D\0"
    "glTexImage2D\0"
    "glTexEnvf\0"
    "glTexEnvfv\0"
    "glTexEnvi\0"
    "glTexEnviv\0"
    "glTexGend\0"
    "glTexGendv\0"
    "glTexGenf\0"
    "glTexGenfv\0"
    "glTexGeni\0"
    "glTexGeniv\0"
    "glFeedbackBuffer\0"
    "glSelectBuffer\0"
    "glRenderMode\0"
    "glInitNames\0"
    "glLoadName\0"
    "glPassThrough\0"
    "glPopName\0"
    "glPushName\0"
    "glDrawBuffer\0"
    "glClear\0"
    "glClearAccum\0"
    "glClearIndex\0"
    "glClearColor\0"
    "glClearStencil\0"
    "glClearDepth\0"
    "glStencilMask\0"
    "glColorMask\0"
    "glDepthMask\0"
    "glIndexMask\0"
    "glAccum\0"
    "glDisable\0"
    "glEnable\0"
    "glFinish\0"
    "glFlush\0"
    "glPopAttrib\0"
    "glPushAttrib\0"
    "glMap1d\0"
    "glMap1f\0"
    "glMap2d\0"
    "glMap2f\0"
    "glMapGrid1d\0"
    "glMapGrid1f\0"
    "glMapGrid2d\0"
    "glMapGrid2f\0"
    "glEvalCoord1d\0"
    "glEvalCoord1dv\0"
    "glEvalCoord1f\0"
    "glEvalCoord1fv\0"
    "glEvalCoord2d\0"
    "glEvalCoord2dv\0"
    "glEvalCoord2f\0"
    "glEvalCoord2fv\0"
    "glEvalMesh1\0"
    "glEvalPoint1\0"
    "glEvalMesh2\0"
    "glEvalPoint2\0"
    "glAlphaFunc\0"
    "glBlendFunc\0"
    "glLogicOp\0"
    "glStencilFunc\0"
    "glStencilOp\0"
    "glDepthFunc\0"
    "glPixelZoom\0"
    "glPixelTransferf\0"
    "glPixelTransferi\0"
    "glPixelStoref\0"
    "glPixelStorei\0"
    "glPixelMapfv\0"
    "glPixelMapuiv\0"
    "glPixelMapusv\0"
    "glReadBuffer\0"
    "glCopyPixels\0"
    "glReadPixels\0"
    "glDrawPixels\0"
    "glGetBooleanv\0"
    "glGetClipPlane\0"
    "glGetDoublev\0"
    "glGetError\0"
    "glGetFloatv\0"
    "glGetIntegerv\0"
    "glGetLightfv\0"
    "glGetLightiv\0"
    "glGetMapdv\0"
    "glGetMapfv\0"
    "glGetMapiv\0"
    "glGetMaterialfv\0"
    "glGetMaterialiv\0"
    "glGetPixelMapfv\0"
    "glGetPixelMapuiv\0"
    "glGetPixelMapusv\0"
    "glGetPolygonStipple\0"
    "glGetString\0"
    "glGetTexEnvfv\0"
    "glGetTexEnviv\0"
    "glGetTexGendv\0"
    "glGetTexGenfv\0"
    "glGetTexGeniv\0"
    "glGetTexImage\0"
    "glGetTexParameterfv\0"
    "glGetTexParameteriv\0"
    "glGetTexLevelParameterfv\0"
    "glGetTexLevelParameteriv\0"
    "glIsEnabled\0"
    "glIsList\0"
    "glDepthRange\0"
    "glFrustum\0"
    "glLoadIdentity\0"
    "glLoadMatrixf\0"
    "glLoadMatrixd\0"
    "glMatrixMode\0"
    "glMultMatrixf\0"
    "glMultMatrixd\0"
    "glOrtho\0"
    "glPopMatrix\0"
    "glPushMatrix\0"
    "glRotated\0"
    "glRotatef\0"
    "glScaled\0"
    "glScalef\0"
    "glTranslated\0"
    "glTranslatef\0"
    "glViewport\0"
    "glArrayElement\0"
    "glBindTexture\0"
    "glColorPointer\0"
    "glDisableClientState\0"
    "glDrawArrays\0"
    "glDrawElements\0"
    "glEdgeFlagPointer\0"
    "glEnableClientState\0"
    "glIndexPointer\0"
    "glIndexub\0"
    "glIndexubv\0"
    "glInterleavedArrays\0"
    "glNormalPointer\0"
    "glPolygonOffset\0"
    "glTexCoordPointer\0"
    "glVertexPointer\0"
    "glAreTexturesResident\0"
    "glCopyTexImage1D\0"
    "glCopyTexImage2D\0"
    "glCopyTexSubImage1D\0"
    "glCopyTexSubImage2D\0"
    "glDeleteTextures\0"
    "glGenTextures\0"
    "glGetPointerv\0"
    "glIsTexture\0"
    "glPrioritizeTextures\0"
    "glTexSubImage1D\0"
    "glTexSubImage2D\0"
    "glPopClientAttrib\0"
    "glPushClientAttrib\0"
    "glBlendColor\0"
    "glBlendEquation\0"
    "glDrawRangeElements\0"
    "glColorTable\0"
    "glColorTableParameterfv\0"
    "glColorTableParameteriv\0"
    "glCopyColorTable\0"
    "glGetColorTable\0"
    "glGetColorTableParameterfv\0"
    "glGetColorTableParameteriv\0"
    "glColorSubTable\0"
    "glCopyColorSubTable\0"
    "glConvolutionFilter1D\0"
    "glConvolutionFilter2D\0"
    "glConvolutionParameterf\0"
    "glConvolutionParameterfv\0"
    "glConvolutionParameteri\0"
    "glConvolutionParameteriv\0"
    "glCopyConvolutionFilter1D\0"
    "glCopyConvolutionFilter2D\0"
    "glGetConvolutionFilter\0"
    "glGetConvolutionParameterfv\0"
    "glGetConvolutionParameteriv\0"
    "glGetSeparableFilter\0"
    "glSeparableFilter2D\0"
    "glGetHistogram\0"
    "glGetHistogramParameterfv\0"
    "glGetHistogramParameteriv\0"
    "glGetMinmax\0"
    "glGetMinmaxParameterfv\0"
    "glGetMinmaxParameteriv\0"
    "glHistogram\0"
    "glMinmax\0"
    "glResetHistogram\0"
    "glResetMinmax\0"
    "glTexImage3D\0"
    "glTexSubImage3D\0"
    "glCopyTexSubImage3D\0"
    "glActiveTexture\0"
    "glClientActiveTexture\0"
    "glMultiTexCoord1d\0"
    "glMultiTexCoord1dv\0"
    "glMultiTexCoord1fARB\0"
    "glMultiTexCoord1fvARB\0"
    "glMultiTexCoord1i\0"
    "glMultiTexCoord1iv\0"
    "glMultiTexCoord1s\0"
    "glMultiTexCoord1sv\0"
    "glMultiTexCoord2d\0"
    "glMultiTexCoord2dv\0"
    "glMultiTexCoord2fARB\0"
    "glMultiTexCoord2fvARB\0"
    "glMultiTexCoord2i\0"
    "glMultiTexCoord2iv\0"
    "glMultiTexCoord2s\0"
    "glMultiTexCoord2sv\0"
    "glMultiTexCoord3d\0"
    "glMultiTexCoord3dv\0"
    "glMultiTexCoord3fARB\0"
    "glMultiTexCoord3fvARB\0"
    "glMultiTexCoord3i\0"
    "glMultiTexCoord3iv\0"
    "glMultiTexCoord3s\0"
    "glMultiTexCoord3sv\0"
    "glMultiTexCoord4d\0"
    "glMultiTexCoord4dv\0"
    "glMultiTexCoord4fARB\0"
    "glMultiTexCoord4fvARB\0"
    "glMultiTexCoord4i\0"
    "glMultiTexCoord4iv\0"
    "glMultiTexCoord4s\0"
    "glMultiTexCoord4sv\0"
    "glCompressedTexImage1D\0"
    "glCompressedTexImage2D\0"
    "glCompressedTexImage3D\0"
    "glCompressedTexSubImage1D\0"
    "glCompressedTexSubImage2D\0"
    "glCompressedTexSubImage3D\0"
    "glGetCompressedTexImage\0"
    "glLoadTransposeMatrixd\0"
    "glLoadTransposeMatrixf\0"
    "glMultTransposeMatrixd\0"
    "glMultTransposeMatrixf\0"
    "glSampleCoverage\0"
    "glBlendFuncSeparate\0"
    "glFogCoordPointer\0"
    "glFogCoordd\0"
    "glFogCoorddv\0"
    "glMultiDrawArrays\0"
    "glPointParameterf\0"
    "glPointParameterfv\0"
    "glPointParameteri\0"
    "glPointParameteriv\0"
    "glSecondaryColor3b\0"
    "glSecondaryColor3bv\0"
    "glSecondaryColor3d\0"
    "glSecondaryColor3dv\0"
    "glSecondaryColor3i\0"
    "glSecondaryColor3iv\0"
    "glSecondaryColor3s\0"
    "glSecondaryColor3sv\0"
    "glSecondaryColor3ub\0"
    "glSecondaryColor3ubv\0"
    "glSecondaryColor3ui\0"
    "glSecondaryColor3uiv\0"
    "glSecondaryColor3us\0"
    "glSecondaryColor3usv\0"
    "glSecondaryColorPointer\0"
    "glWindowPos2d\0"
    "glWindowPos2dv\0"
    "glWindowPos2f\0"
    "glWindowPos2fv\0"
    "glWindowPos2i\0"
    "glWindowPos2iv\0"
    "glWindowPos2s\0"
    "glWindowPos2sv\0"
    "glWindowPos3d\0"
    "glWindowPos3dv\0"
    "glWindowPos3f\0"
    "glWindowPos3fv\0"
    "glWindowPos3i\0"
    "glWindowPos3iv\0"
    "glWindowPos3s\0"
    "glWindowPos3sv\0"
    "glBeginQuery\0"
    "glBindBuffer\0"
    "glBufferData\0"
    "glBufferSubData\0"
    "glDeleteBuffers\0"
    "glDeleteQueries\0"
    "glEndQuery\0"
    "glGenBuffers\0"
    "glGenQueries\0"
    "glGetBufferParameteriv\0"
    "glGetBufferPointerv\0"
    "glGetBufferSubData\0"
    "glGetQueryObjectiv\0"
    "glGetQueryObjectuiv\0"
    "glGetQueryiv\0"
    "glIsBuffer\0"
    "glIsQuery\0"
    "glMapBuffer\0"
    "glUnmapBuffer\0"
    "glAttachShader\0"
    "glBindAttribLocation\0"
    "glBlendEquationSeparate\0"
    "glCompileShader\0"
    "glCreateProgram\0"
    "glCreateShader\0"
    "glDeleteProgram\0"
    "glDeleteShader\0"
    "glDetachShader\0"
    "glDisableVertexAttribArray\0"
    "glDrawBuffers\0"
    "glEnableVertexAttribArray\0"
    "glGetActiveAttrib\0"
    "glGetActiveUniform\0"
    "glGetAttachedShaders\0"
    "glGetAttribLocation\0"
    "glGetProgramInfoLog\0"
    "glGetProgramiv\0"
    "glGetShaderInfoLog\0"
    "glGetShaderSource\0"
    "glGetShaderiv\0"
    "glGetUniformLocation\0"
    "glGetUniformfv\0"
    "glGetUniformiv\0"
    "glGetVertexAttribPointerv\0"
    "glGetVertexAttribdv\0"
    "glGetVertexAttribfv\0"
    "glGetVertexAttribiv\0"
    "glIsProgram\0"
    "glIsShader\0"
    "glLinkProgram\0"
    "glShaderSource\0"
    "glStencilFuncSeparate\0"
    "glStencilMaskSeparate\0"
    "glStencilOpSeparate\0"
    "glUniform1f\0"
    "glUniform1fv\0"
    "glUniform1i\0"
    "glUniform1iv\0"
    "glUniform2f\0"
    "glUniform2fv\0"
    "glUniform2i\0"
    "glUniform2iv\0"
    "glUniform3f\0"
    "glUniform3fv\0"
    "glUniform3i\0"
    "glUniform3iv\0"
    "glUniform4f\0"
    "glUniform4fv\0"
    "glUniform4i\0"
    "glUniform4iv\0"
    "glUniformMatrix2fv\0"
    "glUniformMatrix3fv\0"
    "glUniformMatrix4fv\0"
    "glUseProgram\0"
    "glValidateProgram\0"
    "glVertexAttrib1d\0"
    "glVertexAttrib1dv\0"
    "glVertexAttrib1s\0"
    "glVertexAttrib1sv\0"
    "glVertexAttrib2d\0"
    "glVertexAttrib2dv\0"
    "glVertexAttrib2s\0"
    "glVertexAttrib2sv\0"
    "glVertexAttrib3d\0"
    "glVertexAttrib3dv\0"
    "glVertexAttrib3s\0"
    "glVertexAttrib3sv\0"
    "glVertexAttrib4Nbv\0"
    "glVertexAttrib4Niv\0"
    "glVertexAttrib4Nsv\0"
    "glVertexAttrib4Nub\0"
    "glVertexAttrib4Nubv\0"
    "glVertexAttrib4Nuiv\0"
    "glVertexAttrib4Nusv\0"
    "glVertexAttrib4bv\0"
    "glVertexAttrib4d\0"
    "glVertexAttrib4dv\0"
    "glVertexAttrib4iv\0"
    "glVertexAttrib4s\0"
    "glVertexAttrib4sv\0"
    "glVertexAttrib4ubv\0"
    "glVertexAttrib4uiv\0"
    "glVertexAttrib4usv\0"
    "glVertexAttribPointer\0"
    "glUniformMatrix2x3fv\0"
    "glUniformMatrix2x4fv\0"
    "glUniformMatrix3x2fv\0"
    "glUniformMatrix3x4fv\0"
    "glUniformMatrix4x2fv\0"
    "glUniformMatrix4x3fv\0"
    "glBeginConditionalRender\0"
    "glBeginTransformFeedback\0"
    "glBindBufferBase\0"
    "glBindBufferRange\0"
    "glBindFragDataLocation\0"
    "glClampColor\0"
    "glClearBufferfi\0"
    "glClearBufferfv\0"
    "glClearBufferiv\0"
    "glClearBufferuiv\0"
    "glColorMaski\0"
    "glDisablei\0"
    "glEnablei\0"
    "glEndConditionalRender\0"
    "glEndTransformFeedback\0"
    "glGetBooleani_v\0"
    "glGetFragDataLocation\0"
    "glGetIntegeri_v\0"
    "glGetStringi\0"
    "glGetTexParameterIiv\0"
    "glGetTexParameterIuiv\0"
    "glGetTransformFeedbackVarying\0"
    "glGetUniformuiv\0"
    "glGetVertexAttribIiv\0"
    "glGetVertexAttribIuiv\0"
    "glIsEnabledi\0"
    "glTexParameterIiv\0"
    "glTexParameterIuiv\0"
    "glTransformFeedbackVaryings\0"
    "glUniform1ui\0"
    "glUniform1uiv\0"
    "glUniform2ui\0"
    "glUniform2uiv\0"
    "glUniform3ui\0"
    "glUniform3uiv\0"
    "glUniform4ui\0"
    "glUniform4uiv\0"
    "glVertexAttribI1iv\0"
    "glVertexAttribI1uiv\0"
    "glVertexAttribI4bv\0"
    "glVertexAttribI4sv\0"
    "glVertexAttribI4ubv\0"
    "glVertexAttribI4usv\0"
    "glVertexAttribIPointer\0"
    "glPrimitiveRestartIndex\0"
    "glTexBuffer\0"
    "glFramebufferTexture\0"
    "glGetBufferParameteri64v\0"
    "glGetInteger64i_v\0"
    "glVertexAttribDivisor\0"
    "glBindProgramARB\0"
    "glDeleteProgramsARB\0"
    "glGenProgramsARB\0"
    "glGetProgramEnvParameterdvARB\0"
    "glGetProgramEnvParameterfvARB\0"
    "glGetProgramLocalParameterdvARB\0"
    "glGetProgramLocalParameterfvARB\0"
    "glGetProgramStringARB\0"
    "glGetProgramivARB\0"
    "glIsProgramARB\0"
    "glProgramEnvParameter4dARB\0"
    "glProgramEnvParameter4dvARB\0"
    "glProgramEnvParameter4fARB\0"
    "glProgramEnvParameter4fvARB\0"
    "glProgramLocalParameter4dARB\0"
    "glProgramLocalParameter4dvARB\0"
    "glProgramLocalParameter4fARB\0"
    "glProgramLocalParameter4fvARB\0"
    "glProgramStringARB\0"
    "glVertexAttrib1fARB\0"
    "glVertexAttrib1fvARB\0"
    "glVertexAttrib2fARB\0"
    "glVertexAttrib2fvARB\0"
    "glVertexAttrib3fARB\0"
    "glVertexAttrib3fvARB\0"
    "glVertexAttrib4fARB\0"
    "glVertexAttrib4fvARB\0"
    "glAttachObjectARB\0"
    "glCreateProgramObjectARB\0"
    "glCreateShaderObjectARB\0"
    "glDeleteObjectARB\0"
    "glDetachObjectARB\0"
    "glGetAttachedObjectsARB\0"
    "glGetHandleARB\0"
    "glGetInfoLogARB\0"
    "glGetObjectParameterfvARB\0"
    "glGetObjectParameterivARB\0"
    "glDrawArraysInstancedARB\0"
    "glDrawElementsInstancedARB\0"
    "glBindFramebuffer\0"
    "glBindRenderbuffer\0"
    "glBlitFramebuffer\0"
    "glCheckFramebufferStatus\0"
    "glDeleteFramebuffers\0"
    "glDeleteRenderbuffers\0"
    "glFramebufferRenderbuffer\0"
    "glFramebufferTexture1D\0"
    "glFramebufferTexture2D\0"
    "glFramebufferTexture3D\0"
    "glFramebufferTextureLayer\0"
    "glGenFramebuffers\0"
    "glGenRenderbuffers\0"
    "glGenerateMipmap\0"
    "glGetFramebufferAttachmentParameteriv\0"
    "glGetRenderbufferParameteriv\0"
    "glIsFramebuffer\0"
    "glIsRenderbuffer\0"
    "glRenderbufferStorage\0"
    "glRenderbufferStorageMultisample\0"
    "glFramebufferTextureFaceARB\0"
    "glFlushMappedBufferRange\0"
    "glMapBufferRange\0"
    "glBindVertexArray\0"
    "glDeleteVertexArrays\0"
    "glGenVertexArrays\0"
    "glIsVertexArray\0"
    "glGetActiveUniformBlockName\0"
    "glGetActiveUniformBlockiv\0"
    "glGetActiveUniformName\0"
    "glGetActiveUniformsiv\0"
    "glGetUniformBlockIndex\0"
    "glGetUniformIndices\0"
    "glUniformBlockBinding\0"
    "glCopyBufferSubData\0"
    "glClientWaitSync\0"
    "glDeleteSync\0"
    "glFenceSync\0"
    "glGetInteger64v\0"
    "glGetSynciv\0"
    "glIsSync\0"
    "glWaitSync\0"
    "glDrawElementsBaseVertex\0"
    "glDrawElementsInstancedBaseVertex\0"
    "glDrawRangeElementsBaseVertex\0"
    "glMultiDrawElementsBaseVertex\0"
    "glProvokingVertex\0"
    "glBlendEquationSeparateiARB\0"
    "glBlendEquationiARB\0"
    "glBlendFuncSeparateiARB\0"
    "glBlendFunciARB\0"
    "glBindFragDataLocationIndexed\0"
    "glGetFragDataIndex\0"
    "glBindSampler\0"
    "glDeleteSamplers\0"
    "glGenSamplers\0"
    "glGetSamplerParameterIiv\0"
    "glGetSamplerParameterIuiv\0"
    "glGetSamplerParameterfv\0"
    "glGetSamplerParameteriv\0"
    "glIsSampler\0"
    "glSamplerParameterIiv\0"
    "glSamplerParameterIuiv\0"
    "glSamplerParameterf\0"
    "glSamplerParameterfv\0"
    "glSamplerParameteri\0"
    "glSamplerParameteriv\0"
    "glGetQueryObjecti64v\0"
    "glGetQueryObjectui64v\0"
    "glQueryCounter\0"
    "glColorP3ui\0"
    "glColorP3uiv\0"
    "glColorP4ui\0"
    "glColorP4uiv\0"
    "glMultiTexCoordP1ui\0"
    "glMultiTexCoordP1uiv\0"
    "glMultiTexCoordP2ui\0"
    "glMultiTexCoordP2uiv\0"
    "glMultiTexCoordP3ui\0"
    "glMultiTexCoordP3uiv\0"
    "glMultiTexCoordP4ui\0"
    "glMultiTexCoordP4uiv\0"
    "glNormalP3ui\0"
    "glNormalP3uiv\0"
    "glSecondaryColorP3ui\0"
    "glSecondaryColorP3uiv\0"
    "glTexCoordP1ui\0"
    "glTexCoordP1uiv\0"
    "glTexCoordP2ui\0"
    "glTexCoordP2uiv\0"
    "glTexCoordP3ui\0"
    "glTexCoordP3uiv\0"
    "glTexCoordP4ui\0"
    "glTexCoordP4uiv\0"
    "glVertexAttribP1ui\0"
    "glVertexAttribP1uiv\0"
    "glVertexAttribP2ui\0"
    "glVertexAttribP2uiv\0"
    "glVertexAttribP3ui\0"
    "glVertexAttribP3uiv\0"
    "glVertexAttribP4ui\0"
    "glVertexAttribP4uiv\0"
    "glVertexP2ui\0"
    "glVertexP2uiv\0"
    "glVertexP3ui\0"
    "glVertexP3uiv\0"
    "glVertexP4ui\0"
    "glVertexP4uiv\0"
    "glBindTransformFeedback\0"
    "glDeleteTransformFeedbacks\0"
    "glDrawTransformFeedback\0"
    "glGenTransformFeedbacks\0"
    "glIsTransformFeedback\0"
    "glPauseTransformFeedback\0"
    "glResumeTransformFeedback\0"
    "glBeginQueryIndexed\0"
    "glDrawTransformFeedbackStream\0"
    "glEndQueryIndexed\0"
    "glGetQueryIndexediv\0"
    "glClearDepthf\0"
    "glDepthRangef\0"
    "glGetShaderPrecisionFormat\0"
    "glReleaseShaderCompiler\0"
    "glShaderBinary\0"
    "glGetProgramBinary\0"
    "glProgramBinary\0"
    "glProgramParameteri\0"
    "glDebugMessageCallbackARB\0"
    "glDebugMessageControlARB\0"
    "glDebugMessageInsertARB\0"
    "glGetDebugMessageLogARB\0"
    "glGetGraphicsResetStatusARB\0"
    "glGetnColorTableARB\0"
    "glGetnCompressedTexImageARB\0"
    "glGetnConvolutionFilterARB\0"
    "glGetnHistogramARB\0"
    "glGetnMapdvARB\0"
    "glGetnMapfvARB\0"
    "glGetnMapivARB\0"
    "glGetnMinmaxARB\0"
    "glGetnPixelMapfvARB\0"
    "glGetnPixelMapuivARB\0"
    "glGetnPixelMapusvARB\0"
    "glGetnPolygonStippleARB\0"
    "glGetnSeparableFilterARB\0"
    "glGetnTexImageARB\0"
    "glGetnUniformdvARB\0"
    "glGetnUniformfvARB\0"
    "glGetnUniformivARB\0"
    "glGetnUniformuivARB\0"
    "glReadnPixelsARB\0"
    "glDrawArraysInstancedBaseInstance\0"
    "glDrawElementsInstancedBaseInstance\0"
    "glDrawElementsInstancedBaseVertexBaseInstance\0"
    "glDrawTransformFeedbackInstanced\0"
    "glDrawTransformFeedbackStreamInstanced\0"
    "glGetInternalformativ\0"
    "glTexStorage1D\0"
    "glTexStorage2D\0"
    "glTexStorage3D\0"
    "glTextureStorage1DEXT\0"
    "glTextureStorage2DEXT\0"
    "glTextureStorage3DEXT\0"
    "glInvalidateBufferData\0"
    "glInvalidateBufferSubData\0"
    "glInvalidateFramebuffer\0"
    "glInvalidateSubFramebuffer\0"
    "glInvalidateTexImage\0"
    "glInvalidateTexSubImage\0"
    "glPolygonOffsetEXT\0"
    "glDrawTexfOES\0"
    "glDrawTexfvOES\0"
    "glDrawTexiOES\0"
    "glDrawTexivOES\0"
    "glDrawTexsOES\0"
    "glDrawTexsvOES\0"
    "glDrawTexxOES\0"
    "glDrawTexxvOES\0"
    "glPointSizePointerOES\0"
    "glQueryMatrixxOES\0"
    "glSampleMaskSGIS\0"
    "glSamplePatternSGIS\0"
    "glColorPointerEXT\0"
    "glEdgeFlagPointerEXT\0"
    "glIndexPointerEXT\0"
    "glNormalPointerEXT\0"
    "glTexCoordPointerEXT\0"
    "glVertexPointerEXT\0"
    "glLockArraysEXT\0"
    "glUnlockArraysEXT\0"
    "glSecondaryColor3fEXT\0"
    "glSecondaryColor3fvEXT\0"
    "glMultiDrawElementsEXT\0"
    "glFogCoordfEXT\0"
    "glFogCoordfvEXT\0"
    "glResizeBuffersMESA\0"
    "glWindowPos4dMESA\0"
    "glWindowPos4dvMESA\0"
    "glWindowPos4fMESA\0"
    "glWindowPos4fvMESA\0"
    "glWindowPos4iMESA\0"
    "glWindowPos4ivMESA\0"
    "glWindowPos4sMESA\0"
    "glWindowPos4svMESA\0"
    "glMultiModeDrawArraysIBM\0"
    "glMultiModeDrawElementsIBM\0"
    "glAreProgramsResidentNV\0"
    "glExecuteProgramNV\0"
    "glGetProgramParameterdvNV\0"
    "glGetProgramParameterfvNV\0"
    "glGetProgramStringNV\0"
    "glGetProgramivNV\0"
    "glGetTrackMatrixivNV\0"
    "glGetVertexAttribdvNV\0"
    "glGetVertexAttribfvNV\0"
    "glGetVertexAttribivNV\0"
    "glLoadProgramNV\0"
    "glProgramParameters4dvNV\0"
    "glProgramParameters4fvNV\0"
    "glRequestResidentProgramsNV\0"
    "glTrackMatrixNV\0"
    "glVertexAttrib1dNV\0"
    "glVertexAttrib1dvNV\0"
    "glVertexAttrib1fNV\0"
    "glVertexAttrib1fvNV\0"
    "glVertexAttrib1sNV\0"
    "glVertexAttrib1svNV\0"
    "glVertexAttrib2dNV\0"
    "glVertexAttrib2dvNV\0"
    "glVertexAttrib2fNV\0"
    "glVertexAttrib2fvNV\0"
    "glVertexAttrib2sNV\0"
    "glVertexAttrib2svNV\0"
    "glVertexAttrib3dNV\0"
    "glVertexAttrib3dvNV\0"
    "glVertexAttrib3fNV\0"
    "glVertexAttrib3fvNV\0"
    "glVertexAttrib3sNV\0"
    "glVertexAttrib3svNV\0"
    "glVertexAttrib4dNV\0"
    "glVertexAttrib4dvNV\0"
    "glVertexAttrib4fNV\0"
    "glVertexAttrib4fvNV\0"
    "glVertexAttrib4sNV\0"
    "glVertexAttrib4svNV\0"
    "glVertexAttrib4ubNV\0"
    "glVertexAttrib4ubvNV\0"
    "glVertexAttribPointerNV\0"
    "glVertexAttribs1dvNV\0"
    "glVertexAttribs1fvNV\0"
    "glVertexAttribs1svNV\0"
    "glVertexAttribs2dvNV\0"
    "glVertexAttribs2fvNV\0"
    "glVertexAttribs2svNV\0"
    "glVertexAttribs3dvNV\0"
    "glVertexAttribs3fvNV\0"
    "glVertexAttribs3svNV\0"
    "glVertexAttribs4dvNV\0"
    "glVertexAttribs4fvNV\0"
    "glVertexAttribs4svNV\0"
    "glVertexAttribs4ubvNV\0"
    "glGetTexBumpParameterfvATI\0"
    "glGetTexBumpParameterivATI\0"
    "glTexBumpParameterfvATI\0"
    "glTexBumpParameterivATI\0"
    "glAlphaFragmentOp1ATI\0"
    "glAlphaFragmentOp2ATI\0"
    "glAlphaFragmentOp3ATI\0"
    "glBeginFragmentShaderATI\0"
    "glBindFragmentShaderATI\0"
    "glColorFragmentOp1ATI\0"
    "glColorFragmentOp2ATI\0"
    "glColorFragmentOp3ATI\0"
    "glDeleteFragmentShaderATI\0"
    "glEndFragmentShaderATI\0"
    "glGenFragmentShadersATI\0"
    "glPassTexCoordATI\0"
    "glSampleMapATI\0"
    "glSetFragmentShaderConstantATI\0"
    "glActiveStencilFaceEXT\0"
    "glBindVertexArrayAPPLE\0"
    "glGenVertexArraysAPPLE\0"
    "glGetProgramNamedParameterdvNV\0"
    "glGetProgramNamedParameterfvNV\0"
    "glProgramNamedParameter4dNV\0"
    "glProgramNamedParameter4dvNV\0"
    "glProgramNamedParameter4fNV\0"
    "glProgramNamedParameter4fvNV\0"
    "glPrimitiveRestartNV\0"
    "glGetTexGenxvOES\0"
    "glTexGenxOES\0"
    "glTexGenxvOES\0"
    "glDepthBoundsEXT\0"
    "glBufferParameteriAPPLE\0"
    "glFlushMappedBufferRangeAPPLE\0"
    "glVertexAttribI1iEXT\0"
    "glVertexAttribI1uiEXT\0"
    "glVertexAttribI2iEXT\0"
    "glVertexAttribI2ivEXT\0"
    "glVertexAttribI2uiEXT\0"
    "glVertexAttribI2uivEXT\0"
    "glVertexAttribI3iEXT\0"
    "glVertexAttribI3ivEXT\0"
    "glVertexAttribI3uiEXT\0"
    "glVertexAttribI3uivEXT\0"
    "glVertexAttribI4iEXT\0"
    "glVertexAttribI4ivEXT\0"
    "glVertexAttribI4uiEXT\0"
    "glVertexAttribI4uivEXT\0"
    "glClearColorIiEXT\0"
    "glClearColorIuiEXT\0"
    "glBindBufferOffsetEXT\0"
    "glGetObjectParameterivAPPLE\0"
    "glObjectPurgeableAPPLE\0"
    "glObjectUnpurgeableAPPLE\0"
    "glActiveProgramEXT\0"
    "glCreateShaderProgramEXT\0"
    "glUseShaderProgramEXT\0"
    "glTextureBarrierNV\0"
    "glStencilFuncSeparateATI\0"
    "glProgramEnvParameters4fvEXT\0"
    "glProgramLocalParameters4fvEXT\0"
    "glEGLImageTargetRenderbufferStorageOES\0"
    "glEGLImageTargetTexture2DOES\0"
    "glAlphaFuncx\0"
    "glClearColorx\0"
    "glClearDepthx\0"
    "glColor4x\0"
    "glDepthRangex\0"
    "glFogx\0"
    "glFogxv\0"
    "glFrustumf\0"
    "glFrustumx\0"
    "glLightModelx\0"
    "glLightModelxv\0"
    "glLightx\0"
    "glLightxv\0"
    "glLineWidthx\0"
    "glLoadMatrixx\0"
    "glMaterialx\0"
    "glMaterialxv\0"
    "glMultMatrixx\0"
    "glMultiTexCoord4x\0"
    "glNormal3x\0"
    "glOrthof\0"
    "glOrthox\0"
    "glPointSizex\0"
    "glPolygonOffsetx\0"
    "glRotatex\0"
    "glSampleCoveragex\0"
    "glScalex\0"
    "glTexEnvx\0"
    "glTexEnvxv\0"
    "glTexParameterx\0"
    "glTranslatex\0"
    "glClipPlanef\0"
    "glClipPlanex\0"
    "glGetClipPlanef\0"
    "glGetClipPlanex\0"
    "glGetFixedv\0"
    "glGetLightxv\0"
    "glGetMaterialxv\0"
    "glGetTexEnvxv\0"
    "glGetTexParameterxv\0"
    "glPointParameterx\0"
    "glPointParameterxv\0"
    "glTexParameterxv\0"
    "glTexGenfOES\0"
    "glTexGenfvOES\0"
    "glTexGeniOES\0"
    "glTexGenivOES\0"
    "glReadBufferNV\0"
    "glGetTexGenfvOES\0"
    "glGetTexGenivOES\0"
    "glArrayElementEXT\0"
    "glBindTextureEXT\0"
    "glDrawArraysEXT\0"
    "glAreTexturesResidentEXT\0"
    "glCopyTexImage1DEXT\0"
    "glCopyTexImage2DEXT\0"
    "glCopyTexSubImage1DEXT\0"
    "glCopyTexSubImage2DEXT\0"
    "glDeleteTexturesEXT\0"
    "glGenTexturesEXT\0"
    "glGetPointervEXT\0"
    "glIsTextureEXT\0"
    "glPrioritizeTexturesEXT\0"
    "glTexSubImage1DEXT\0"
    "glTexSubImage2DEXT\0"
    "glBlendColorEXT\0"
    "glBlendEquationEXT\0"
    "glBlendEquationOES\0"
    "glDrawRangeElementsEXT\0"
    "glColorTableSGI\0"
    "glColorTableEXT\0"
    "glColorTableParameterfvSGI\0"
    "glColorTableParameterivSGI\0"
    "glCopyColorTableSGI\0"
    "glGetColorTableSGI\0"
    "glGetColorTableEXT\0"
    "glGetColorTableParameterfvSGI\0"
    "glGetColorTableParameterfvEXT\0"
    "glGetColorTableParameterivSGI\0"
    "glGetColorTableParameterivEXT\0"
    "glColorSubTableEXT\0"
    "glCopyColorSubTableEXT\0"
    "glConvolutionFilter1DEXT\0"
    "glConvolutionFilter2DEXT\0"
    "glConvolutionParameterfEXT\0"
    "glConvolutionParameterfvEXT\0"
    "glConvolutionParameteriEXT\0"
    "glConvolutionParameterivEXT\0"
    "glCopyConvolutionFilter1DEXT\0"
    "glCopyConvolutionFilter2DEXT\0"
    "glGetConvolutionFilterEXT\0"
    "glGetConvolutionParameterfvEXT\0"
    "glGetConvolutionParameterivEXT\0"
    "glGetSeparableFilterEXT\0"
    "glSeparableFilter2DEXT\0"
    "glGetHistogramEXT\0"
    "glGetHistogramParameterfvEXT\0"
    "glGetHistogramParameterivEXT\0"
    "glGetMinmaxEXT\0"
    "glGetMinmaxParameterfvEXT\0"
    "glGetMinmaxParameterivEXT\0"
    "glHistogramEXT\0"
    "glMinmaxEXT\0"
    "glResetHistogramEXT\0"
    "glResetMinmaxEXT\0"
    "glTexImage3DEXT\0"
    "glTexImage3DOES\0"
    "glTexSubImage3DEXT\0"
    "glTexSubImage3DOES\0"
    "glCopyTexSubImage3DEXT\0"
    "glCopyTexSubImage3DOES\0"
    "glActiveTextureARB\0"
    "glClientActiveTextureARB\0"
    "glMultiTexCoord1dARB\0"
    "glMultiTexCoord1dvARB\0"
    "glMultiTexCoord1f\0"
    "glMultiTexCoord1fv\0"
    "glMultiTexCoord1iARB\0"
    "glMultiTexCoord1ivARB\0"
    "glMultiTexCoord1sARB\0"
    "glMultiTexCoord1svARB\0"
    "glMultiTexCoord2dARB\0"
    "glMultiTexCoord2dvARB\0"
    "glMultiTexCoord2f\0"
    "glMultiTexCoord2fv\0"
    "glMultiTexCoord2iARB\0"
    "glMultiTexCoord2ivARB\0"
    "glMultiTexCoord2sARB\0"
    "glMultiTexCoord2svARB\0"
    "glMultiTexCoord3dARB\0"
    "glMultiTexCoord3dvARB\0"
    "glMultiTexCoord3f\0"
    "glMultiTexCoord3fv\0"
    "glMultiTexCoord3iARB\0"
    "glMultiTexCoord3ivARB\0"
    "glMultiTexCoord3sARB\0"
    "glMultiTexCoord3svARB\0"
    "glMultiTexCoord4dARB\0"
    "glMultiTexCoord4dvARB\0"
    "glMultiTexCoord4f\0"
    "glMultiTexCoord4fv\0"
    "glMultiTexCoord4iARB\0"
    "glMultiTexCoord4ivARB\0"
    "glMultiTexCoord4sARB\0"
    "glMultiTexCoord4svARB\0"
    "glCompressedTexImage1DARB\0"
    "glCompressedTexImage2DARB\0"
    "glCompressedTexImage3DARB\0"
    "glCompressedTexImage3DOES\0"
    "glCompressedTexSubImage1DARB\0"
    "glCompressedTexSubImage2DARB\0"
    "glCompressedTexSubImage3DARB\0"
    "glCompressedTexSubImage3DOES\0"
    "glGetCompressedTexImageARB\0"
    "glLoadTransposeMatrixdARB\0"
    "glLoadTransposeMatrixfARB\0"
    "glMultTransposeMatrixdARB\0"
    "glMultTransposeMatrixfARB\0"
    "glSampleCoverageARB\0"
    "glBlendFuncSeparateEXT\0"
    "glBlendFuncSeparateINGR\0"
    "glBlendFuncSeparateOES\0"
    "glFogCoordPointerEXT\0"
    "glFogCoorddEXT\0"
    "glFogCoorddvEXT\0"
    "glMultiDrawArraysEXT\0"
    "glPointParameterfARB\0"
    "glPointParameterfEXT\0"
    "glPointParameterfSGIS\0"
    "glPointParameterfvARB\0"
    "glPointParameterfvEXT\0"
    "glPointParameterfvSGIS\0"
    "glPointParameteriNV\0"
    "glPointParameterivNV\0"
    "glSecondaryColor3bEXT\0"
    "glSecondaryColor3bvEXT\0"
    "glSecondaryColor3dEXT\0"
    "glSecondaryColor3dvEXT\0"
    "glSecondaryColor3iEXT\0"
    "glSecondaryColor3ivEXT\0"
    "glSecondaryColor3sEXT\0"
    "glSecondaryColor3svEXT\0"
    "glSecondaryColor3ubEXT\0"
    "glSecondaryColor3ubvEXT\0"
    "glSecondaryColor3uiEXT\0"
    "glSecondaryColor3uivEXT\0"
    "glSecondaryColor3usEXT\0"
    "glSecondaryColor3usvEXT\0"
    "glSecondaryColorPointerEXT\0"
    "glWindowPos2dARB\0"
    "glWindowPos2dMESA\0"
    "glWindowPos2dvARB\0"
    "glWindowPos2dvMESA\0"
    "glWindowPos2fARB\0"
    "glWindowPos2fMESA\0"
    "glWindowPos2fvARB\0"
    "glWindowPos2fvMESA\0"
    "glWindowPos2iARB\0"
    "glWindowPos2iMESA\0"
    "glWindowPos2ivARB\0"
    "glWindowPos2ivMESA\0"
    "glWindowPos2sARB\0"
    "glWindowPos2sMESA\0"
    "glWindowPos2svARB\0"
    "glWindowPos2svMESA\0"
    "glWindowPos3dARB\0"
    "glWindowPos3dMESA\0"
    "glWindowPos3dvARB\0"
    "glWindowPos3dvMESA\0"
    "glWindowPos3fARB\0"
    "glWindowPos3fMESA\0"
    "glWindowPos3fvARB\0"
    "glWindowPos3fvMESA\0"
    "glWindowPos3iARB\0"
    "glWindowPos3iMESA\0"
    "glWindowPos3ivARB\0"
    "glWindowPos3ivMESA\0"
    "glWindowPos3sARB\0"
    "glWindowPos3sMESA\0"
    "glWindowPos3svARB\0"
    "glWindowPos3svMESA\0"
    "glBeginQueryARB\0"
    "glBindBufferARB\0"
    "glBufferDataARB\0"
    "glBufferSubDataARB\0"
    "glDeleteBuffersARB\0"
    "glDeleteQueriesARB\0"
    "glEndQueryARB\0"
    "glGenBuffersARB\0"
    "glGenQueriesARB\0"
    "glGetBufferParameterivARB\0"
    "glGetBufferPointervARB\0"
    "glGetBufferPointervOES\0"
    "glGetBufferSubDataARB\0"
    "glGetQueryObjectivARB\0"
    "glGetQueryObjectuivARB\0"
    "glGetQueryivARB\0"
    "glIsBufferARB\0"
    "glIsQueryARB\0"
    "glMapBufferARB\0"
    "glMapBufferOES\0"
    "glUnmapBufferARB\0"
    "glUnmapBufferOES\0"
    "glBindAttribLocationARB\0"
    "glBlendEquationSeparateEXT\0"
    "glBlendEquationSeparateATI\0"
    "glBlendEquationSeparateOES\0"
    "glCompileShaderARB\0"
    "glDisableVertexAttribArrayARB\0"
    "glDrawBuffersARB\0"
    "glDrawBuffersATI\0"
    "glDrawBuffersNV\0"
    "glEnableVertexAttribArrayARB\0"
    "glGetActiveAttribARB\0"
    "glGetActiveUniformARB\0"
    "glGetAttribLocationARB\0"
    "glGetShaderSourceARB\0"
    "glGetUniformLocationARB\0"
    "glGetUniformfvARB\0"
    "glGetUniformivARB\0"
    "glGetVertexAttribPointervARB\0"
    "glGetVertexAttribPointervNV\0"
    "glGetVertexAttribdvARB\0"
    "glGetVertexAttribfvARB\0"
    "glGetVertexAttribivARB\0"
    "glLinkProgramARB\0"
    "glShaderSourceARB\0"
    "glStencilOpSeparateATI\0"
    "glUniform1fARB\0"
    "glUniform1fvARB\0"
    "glUniform1iARB\0"
    "glUniform1ivARB\0"
    "glUniform2fARB\0"
    "glUniform2fvARB\0"
    "glUniform2iARB\0"
    "glUniform2ivARB\0"
    "glUniform3fARB\0"
    "glUniform3fvARB\0"
    "glUniform3iARB\0"
    "glUniform3ivARB\0"
    "glUniform4fARB\0"
    "glUniform4fvARB\0"
    "glUniform4iARB\0"
    "glUniform4ivARB\0"
    "glUniformMatrix2fvARB\0"
    "glUniformMatrix3fvARB\0"
    "glUniformMatrix4fvARB\0"
    "glUseProgramObjectARB\0"
    "glValidateProgramARB\0"
    "glVertexAttrib1dARB\0"
    "glVertexAttrib1dvARB\0"
    "glVertexAttrib1sARB\0"
    "glVertexAttrib1svARB\0"
    "glVertexAttrib2dARB\0"
    "glVertexAttrib2dvARB\0"
    "glVertexAttrib2sARB\0"
    "glVertexAttrib2svARB\0"
    "glVertexAttrib3dARB\0"
    "glVertexAttrib3dvARB\0"
    "glVertexAttrib3sARB\0"
    "glVertexAttrib3svARB\0"
    "glVertexAttrib4NbvARB\0"
    "glVertexAttrib4NivARB\0"
    "glVertexAttrib4NsvARB\0"
    "glVertexAttrib4NubARB\0"
    "glVertexAttrib4NubvARB\0"
    "glVertexAttrib4NuivARB\0"
    "glVertexAttrib4NusvARB\0"
    "glVertexAttrib4bvARB\0"
    "glVertexAttrib4dARB\0"
    "glVertexAttrib4dvARB\0"
    "glVertexAttrib4ivARB\0"
    "glVertexAttrib4sARB\0"
    "glVertexAttrib4svARB\0"
    "glVertexAttrib4ubvARB\0"
    "glVertexAttrib4uivARB\0"
    "glVertexAttrib4usvARB\0"
    "glVertexAttribPointerARB\0"
    "glBeginConditionalRenderNV\0"
    "glBeginTransformFeedbackEXT\0"
    "glBindBufferBaseEXT\0"
    "glBindBufferRangeEXT\0"
    "glBindFragDataLocationEXT\0"
    "glClampColorARB\0"
    "glColorMaskIndexedEXT\0"
    "glDisableIndexedEXT\0"
    "glEnableIndexedEXT\0"
    "glEndConditionalRenderNV\0"
    "glEndTransformFeedbackEXT\0"
    "glGetBooleanIndexedvEXT\0"
    "glGetFragDataLocationEXT\0"
    "glGetIntegerIndexedvEXT\0"
    "glGetTexParameterIivEXT\0"
    "glGetTexParameterIuivEXT\0"
    "glGetTransformFeedbackVaryingEXT\0"
    "glGetUniformuivEXT\0"
    "glGetVertexAttribIivEXT\0"
    "glGetVertexAttribIuivEXT\0"
    "glIsEnabledIndexedEXT\0"
    "glTexParameterIivEXT\0"
    "glTexParameterIuivEXT\0"
    "glTransformFeedbackVaryingsEXT\0"
    "glUniform1uiEXT\0"
    "glUniform1uivEXT\0"
    "glUniform2uiEXT\0"
    "glUniform2uivEXT\0"
    "glUniform3uiEXT\0"
    "glUniform3uivEXT\0"
    "glUniform4uiEXT\0"
    "glUniform4uivEXT\0"
    "glVertexAttribI1ivEXT\0"
    "glVertexAttribI1uivEXT\0"
    "glVertexAttribI4bvEXT\0"
    "glVertexAttribI4svEXT\0"
    "glVertexAttribI4ubvEXT\0"
    "glVertexAttribI4usvEXT\0"
    "glVertexAttribIPointerEXT\0"
    "glPrimitiveRestartIndexNV\0"
    "glTexBufferARB\0"
    "glFramebufferTextureARB\0"
    "glVertexAttribDivisorARB\0"
    "glBindProgramNV\0"
    "glDeleteProgramsNV\0"
    "glGenProgramsNV\0"
    "glIsProgramNV\0"
    "glProgramParameter4dNV\0"
    "glProgramParameter4dvNV\0"
    "glProgramParameter4fNV\0"
    "glProgramParameter4fvNV\0"
    "glVertexAttrib1f\0"
    "glVertexAttrib1fv\0"
    "glVertexAttrib2f\0"
    "glVertexAttrib2fv\0"
    "glVertexAttrib3f\0"
    "glVertexAttrib3fv\0"
    "glVertexAttrib4f\0"
    "glVertexAttrib4fv\0"
    "glDrawArraysInstancedEXT\0"
    "glDrawArraysInstanced\0"
    "glDrawElementsInstancedEXT\0"
    "glDrawElementsInstanced\0"
    "glBindFramebufferEXT\0"
    "glBindFramebufferOES\0"
    "glBindRenderbufferEXT\0"
    "glBindRenderbufferOES\0"
    "glBlitFramebufferEXT\0"
    "glCheckFramebufferStatusEXT\0"
    "glCheckFramebufferStatusOES\0"
    "glDeleteFramebuffersEXT\0"
    "glDeleteFramebuffersOES\0"
    "glDeleteRenderbuffersEXT\0"
    "glDeleteRenderbuffersOES\0"
    "glFramebufferRenderbufferEXT\0"
    "glFramebufferRenderbufferOES\0"
    "glFramebufferTexture1DEXT\0"
    "glFramebufferTexture2DEXT\0"
    "glFramebufferTexture2DOES\0"
    "glFramebufferTexture3DEXT\0"
    "glFramebufferTexture3DOES\0"
    "glFramebufferTextureLayerARB\0"
    "glFramebufferTextureLayerEXT\0"
    "glGenFramebuffersEXT\0"
    "glGenFramebuffersOES\0"
    "glGenRenderbuffersEXT\0"
    "glGenRenderbuffersOES\0"
    "glGenerateMipmapEXT\0"
    "glGenerateMipmapOES\0"
    "glGetFramebufferAttachmentParameterivEXT\0"
    "glGetFramebufferAttachmentParameterivOES\0"
    "glGetRenderbufferParameterivEXT\0"
    "glGetRenderbufferParameterivOES\0"
    "glIsFramebufferEXT\0"
    "glIsFramebufferOES\0"
    "glIsRenderbufferEXT\0"
    "glIsRenderbufferOES\0"
    "glRenderbufferStorageEXT\0"
    "glRenderbufferStorageOES\0"
    "glRenderbufferStorageMultisampleEXT\0"
    "glFlushMappedBufferRangeEXT\0"
    "glMapBufferRangeEXT\0"
    "glBindVertexArrayOES\0"
    "glDeleteVertexArraysAPPLE\0"
    "glDeleteVertexArraysOES\0"
    "glGenVertexArraysOES\0"
    "glIsVertexArrayAPPLE\0"
    "glIsVertexArrayOES\0"
    "glProvokingVertexEXT\0"
    "glBlendEquationSeparateIndexedAMD\0"
    "glBlendEquationIndexedAMD\0"
    "glBlendFuncSeparateIndexedAMD\0"
    "glBlendFuncIndexedAMD\0"
    "glGetQueryObjecti64vEXT\0"
    "glGetQueryObjectui64vEXT\0"
    "glClearDepthfOES\0"
    "glDepthRangefOES\0"
    "glGetProgramBinaryOES\0"
    "glProgramBinaryOES\0"
    "glProgramParameteriARB\0"
    "glSampleMaskEXT\0"
    "glSamplePatternEXT\0"
    "glSecondaryColor3f\0"
    "glSecondaryColor3fv\0"
    "glMultiDrawElements\0"
    "glFogCoordf\0"
    "glFogCoordfv\0"
    "glVertexAttribI1i\0"
    "glVertexAttribI1ui\0"
    "glVertexAttribI2i\0"
    "glVertexAttribI2iv\0"
    "glVertexAttribI2ui\0"
    "glVertexAttribI2uiv\0"
    "glVertexAttribI3i\0"
    "glVertexAttribI3iv\0"
    "glVertexAttribI3ui\0"
    "glVertexAttribI3uiv\0"
    "glVertexAttribI4i\0"
    "glVertexAttribI4iv\0"
    "glVertexAttribI4ui\0"
    "glVertexAttribI4uiv\0"
    "glAlphaFuncxOES\0"
    "glClearColorxOES\0"
    "glClearDepthxOES\0"
    "glColor4xOES\0"
    "glDepthRangexOES\0"
    "glFogxOES\0"
    "glFogxvOES\0"
    "glFrustumfOES\0"
    "glFrustumxOES\0"
    "glLightModelxOES\0"
    "glLightModelxvOES\0"
    "glLightxOES\0"
    "glLightxvOES\0"
    "glLineWidthxOES\0"
    "glLoadMatrixxOES\0"
    "glMaterialxOES\0"
    "glMaterialxvOES\0"
    "glMultMatrixxOES\0"
    "glMultiTexCoord4xOES\0"
    "glNormal3xOES\0"
    "glOrthofOES\0"
    "glOrthoxOES\0"
    "glPointSizexOES\0"
    "glPolygonOffsetxOES\0"
    "glRotatexOES\0"
    "glSampleCoveragexOES\0"
    "glScalexOES\0"
    "glTexEnvxOES\0"
    "glTexEnvxvOES\0"
    "glTexParameterxOES\0"
    "glTranslatexOES\0"
    "glClipPlanefOES\0"
    "glClipPlanexOES\0"
    "glGetClipPlanefOES\0"
    "glGetClipPlanexOES\0"
    "glGetFixedvOES\0"
    "glGetLightxvOES\0"
    "glGetMaterialxvOES\0"
    "glGetTexEnvxvOES\0"
    "glGetTexParameterxvOES\0"
    "glPointParameterxOES\0"
    "glPointParameterxvOES\0"
    "glTexParameterxvOES\0"
    ;


#ifdef USE_MGL_NAMESPACE
#define gl_dispatch_stub_343 mgl_dispatch_stub_343
#define gl_dispatch_stub_344 mgl_dispatch_stub_344
#define gl_dispatch_stub_345 mgl_dispatch_stub_345
#define gl_dispatch_stub_356 mgl_dispatch_stub_356
#define gl_dispatch_stub_357 mgl_dispatch_stub_357
#define gl_dispatch_stub_358 mgl_dispatch_stub_358
#define gl_dispatch_stub_359 mgl_dispatch_stub_359
#define gl_dispatch_stub_361 mgl_dispatch_stub_361
#define gl_dispatch_stub_362 mgl_dispatch_stub_362
#define gl_dispatch_stub_363 mgl_dispatch_stub_363
#define gl_dispatch_stub_364 mgl_dispatch_stub_364
#define gl_dispatch_stub_365 mgl_dispatch_stub_365
#define gl_dispatch_stub_366 mgl_dispatch_stub_366
#define gl_dispatch_stub_726 mgl_dispatch_stub_726
#define gl_dispatch_stub_727 mgl_dispatch_stub_727
#define gl_dispatch_stub_728 mgl_dispatch_stub_728
#define gl_dispatch_stub_815 mgl_dispatch_stub_815
#define gl_dispatch_stub_829 mgl_dispatch_stub_829
#define gl_dispatch_stub_830 mgl_dispatch_stub_830
#define gl_dispatch_stub_831 mgl_dispatch_stub_831
#define gl_dispatch_stub_832 mgl_dispatch_stub_832
#define gl_dispatch_stub_833 mgl_dispatch_stub_833
#define gl_dispatch_stub_834 mgl_dispatch_stub_834
#define gl_dispatch_stub_835 mgl_dispatch_stub_835
#define gl_dispatch_stub_836 mgl_dispatch_stub_836
#define gl_dispatch_stub_837 mgl_dispatch_stub_837
#define gl_dispatch_stub_838 mgl_dispatch_stub_838
#define gl_dispatch_stub_839 mgl_dispatch_stub_839
#define gl_dispatch_stub_840 mgl_dispatch_stub_840
#define gl_dispatch_stub_863 mgl_dispatch_stub_863
#define gl_dispatch_stub_864 mgl_dispatch_stub_864
#define gl_dispatch_stub_938 mgl_dispatch_stub_938
#define gl_dispatch_stub_939 mgl_dispatch_stub_939
#define gl_dispatch_stub_940 mgl_dispatch_stub_940
#define gl_dispatch_stub_948 mgl_dispatch_stub_948
#define gl_dispatch_stub_949 mgl_dispatch_stub_949
#define gl_dispatch_stub_950 mgl_dispatch_stub_950
#define gl_dispatch_stub_951 mgl_dispatch_stub_951
#define gl_dispatch_stub_952 mgl_dispatch_stub_952
#define gl_dispatch_stub_953 mgl_dispatch_stub_953
#define gl_dispatch_stub_978 mgl_dispatch_stub_978
#define gl_dispatch_stub_979 mgl_dispatch_stub_979
#define gl_dispatch_stub_980 mgl_dispatch_stub_980
#define gl_dispatch_stub_983 mgl_dispatch_stub_983
#define gl_dispatch_stub_984 mgl_dispatch_stub_984
#define gl_dispatch_stub_985 mgl_dispatch_stub_985
#define gl_dispatch_stub_986 mgl_dispatch_stub_986
#define gl_dispatch_stub_987 mgl_dispatch_stub_987
#define gl_dispatch_stub_988 mgl_dispatch_stub_988
#define gl_dispatch_stub_989 mgl_dispatch_stub_989
#define gl_dispatch_stub_990 mgl_dispatch_stub_990
#define gl_dispatch_stub_991 mgl_dispatch_stub_991
#define gl_dispatch_stub_992 mgl_dispatch_stub_992
#define gl_dispatch_stub_993 mgl_dispatch_stub_993
#define gl_dispatch_stub_994 mgl_dispatch_stub_994
#define gl_dispatch_stub_995 mgl_dispatch_stub_995
#define gl_dispatch_stub_996 mgl_dispatch_stub_996
#define gl_dispatch_stub_997 mgl_dispatch_stub_997
#define gl_dispatch_stub_998 mgl_dispatch_stub_998
#define gl_dispatch_stub_999 mgl_dispatch_stub_999
#define gl_dispatch_stub_1000 mgl_dispatch_stub_1000
#define gl_dispatch_stub_1001 mgl_dispatch_stub_1001
#define gl_dispatch_stub_1002 mgl_dispatch_stub_1002
#define gl_dispatch_stub_1003 mgl_dispatch_stub_1003
#define gl_dispatch_stub_1004 mgl_dispatch_stub_1004
#define gl_dispatch_stub_1005 mgl_dispatch_stub_1005
#define gl_dispatch_stub_1006 mgl_dispatch_stub_1006
#define gl_dispatch_stub_1007 mgl_dispatch_stub_1007
#define gl_dispatch_stub_1008 mgl_dispatch_stub_1008
#define gl_dispatch_stub_1009 mgl_dispatch_stub_1009
#define gl_dispatch_stub_1010 mgl_dispatch_stub_1010
#define gl_dispatch_stub_1011 mgl_dispatch_stub_1011
#define gl_dispatch_stub_1012 mgl_dispatch_stub_1012
#define gl_dispatch_stub_1013 mgl_dispatch_stub_1013
#define gl_dispatch_stub_1014 mgl_dispatch_stub_1014
#define gl_dispatch_stub_1015 mgl_dispatch_stub_1015
#define gl_dispatch_stub_1016 mgl_dispatch_stub_1016
#define gl_dispatch_stub_1017 mgl_dispatch_stub_1017
#define gl_dispatch_stub_1018 mgl_dispatch_stub_1018
#define gl_dispatch_stub_1019 mgl_dispatch_stub_1019
#define gl_dispatch_stub_1020 mgl_dispatch_stub_1020
#define gl_dispatch_stub_1021 mgl_dispatch_stub_1021
#define gl_dispatch_stub_1022 mgl_dispatch_stub_1022
#define gl_dispatch_stub_1023 mgl_dispatch_stub_1023
#define gl_dispatch_stub_1024 mgl_dispatch_stub_1024
#define gl_dispatch_stub_1025 mgl_dispatch_stub_1025
#endif /* USE_MGL_NAMESPACE */


#if defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING)
void GLAPIENTRY gl_dispatch_stub_343(GLenum target, GLenum format, GLenum type, GLvoid * table);
void GLAPIENTRY gl_dispatch_stub_344(GLenum target, GLenum pname, GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_345(GLenum target, GLenum pname, GLint * params);
void GLAPIENTRY gl_dispatch_stub_356(GLenum target, GLenum format, GLenum type, GLvoid * image);
void GLAPIENTRY gl_dispatch_stub_357(GLenum target, GLenum pname, GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_358(GLenum target, GLenum pname, GLint * params);
void GLAPIENTRY gl_dispatch_stub_359(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span);
void GLAPIENTRY gl_dispatch_stub_361(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);
void GLAPIENTRY gl_dispatch_stub_362(GLenum target, GLenum pname, GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_363(GLenum target, GLenum pname, GLint * params);
void GLAPIENTRY gl_dispatch_stub_364(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);
void GLAPIENTRY gl_dispatch_stub_365(GLenum target, GLenum pname, GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_366(GLenum target, GLenum pname, GLint * params);
void GLAPIENTRY gl_dispatch_stub_726(GLuint id, GLenum pname, GLint64 * params);
void GLAPIENTRY gl_dispatch_stub_727(GLuint id, GLenum pname, GLuint64 * params);
void GLAPIENTRY gl_dispatch_stub_728(GLuint id, GLenum target);
void GLAPIENTRY gl_dispatch_stub_815(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params);
void GLAPIENTRY gl_dispatch_stub_829(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
void GLAPIENTRY gl_dispatch_stub_830(const GLfloat * coords);
void GLAPIENTRY gl_dispatch_stub_831(GLint x, GLint y, GLint z, GLint width, GLint height);
void GLAPIENTRY gl_dispatch_stub_832(const GLint * coords);
void GLAPIENTRY gl_dispatch_stub_833(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
void GLAPIENTRY gl_dispatch_stub_834(const GLshort * coords);
void GLAPIENTRY gl_dispatch_stub_835(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
void GLAPIENTRY gl_dispatch_stub_836(const GLfixed * coords);
void GLAPIENTRY gl_dispatch_stub_837(GLenum type, GLsizei stride, const GLvoid * pointer);
GLbitfield GLAPIENTRY gl_dispatch_stub_838(GLfixed * mantissa, GLint * exponent);
void GLAPIENTRY gl_dispatch_stub_839(GLclampf value, GLboolean invert);
void GLAPIENTRY gl_dispatch_stub_840(GLenum pattern);
void GLAPIENTRY gl_dispatch_stub_863(const GLenum * mode, const GLint * first, const GLsizei * count, GLsizei primcount, GLint modestride);
void GLAPIENTRY gl_dispatch_stub_864(const GLenum * mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, GLint modestride);
void GLAPIENTRY gl_dispatch_stub_938(GLenum face);
void GLAPIENTRY gl_dispatch_stub_939(GLuint array);
void GLAPIENTRY gl_dispatch_stub_940(GLsizei n, GLuint * arrays);
void GLAPIENTRY gl_dispatch_stub_948(GLenum coord, GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_949(GLenum coord, GLenum pname, GLint param);
void GLAPIENTRY gl_dispatch_stub_950(GLenum coord, GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_951(GLclampd zmin, GLclampd zmax);
void GLAPIENTRY gl_dispatch_stub_952(GLenum target, GLenum pname, GLint param);
void GLAPIENTRY gl_dispatch_stub_953(GLenum target, GLintptr offset, GLsizeiptr size);
void GLAPIENTRY gl_dispatch_stub_978(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
void GLAPIENTRY gl_dispatch_stub_979(GLenum target, GLuint index, GLsizei count, const GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_980(GLenum target, GLuint index, GLsizei count, const GLfloat * params);
void GLAPIENTRY gl_dispatch_stub_983(GLenum func, GLclampx ref);
void GLAPIENTRY gl_dispatch_stub_984(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
void GLAPIENTRY gl_dispatch_stub_985(GLclampx depth);
void GLAPIENTRY gl_dispatch_stub_986(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
void GLAPIENTRY gl_dispatch_stub_987(GLclampx zNear, GLclampx zFar);
void GLAPIENTRY gl_dispatch_stub_988(GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_989(GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_990(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
void GLAPIENTRY gl_dispatch_stub_991(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
void GLAPIENTRY gl_dispatch_stub_992(GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_993(GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_994(GLenum light, GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_995(GLenum light, GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_996(GLfixed width);
void GLAPIENTRY gl_dispatch_stub_997(const GLfixed * m);
void GLAPIENTRY gl_dispatch_stub_998(GLenum face, GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_999(GLenum face, GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1000(const GLfixed * m);
void GLAPIENTRY gl_dispatch_stub_1001(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
void GLAPIENTRY gl_dispatch_stub_1002(GLfixed nx, GLfixed ny, GLfixed nz);
void GLAPIENTRY gl_dispatch_stub_1003(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
void GLAPIENTRY gl_dispatch_stub_1004(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
void GLAPIENTRY gl_dispatch_stub_1005(GLfixed size);
void GLAPIENTRY gl_dispatch_stub_1006(GLfixed factor, GLfixed units);
void GLAPIENTRY gl_dispatch_stub_1007(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
void GLAPIENTRY gl_dispatch_stub_1008(GLclampx value, GLboolean invert);
void GLAPIENTRY gl_dispatch_stub_1009(GLfixed x, GLfixed y, GLfixed z);
void GLAPIENTRY gl_dispatch_stub_1010(GLenum target, GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_1011(GLenum target, GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1012(GLenum target, GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_1013(GLfixed x, GLfixed y, GLfixed z);
void GLAPIENTRY gl_dispatch_stub_1014(GLenum plane, const GLfloat * equation);
void GLAPIENTRY gl_dispatch_stub_1015(GLenum plane, const GLfixed * equation);
void GLAPIENTRY gl_dispatch_stub_1016(GLenum plane, GLfloat * equation);
void GLAPIENTRY gl_dispatch_stub_1017(GLenum plane, GLfixed * equation);
void GLAPIENTRY gl_dispatch_stub_1018(GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1019(GLenum light, GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1020(GLenum face, GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1021(GLenum target, GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1022(GLenum target, GLenum pname, GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1023(GLenum pname, GLfixed param);
void GLAPIENTRY gl_dispatch_stub_1024(GLenum pname, const GLfixed * params);
void GLAPIENTRY gl_dispatch_stub_1025(GLenum target, GLenum pname, const GLfixed * params);
#endif /* defined(NEED_FUNCTION_POINTER) || defined(GLX_INDIRECT_RENDERING) */

static const glprocs_table_t static_functions[] = {
    NAME_FUNC_OFFSET(    0, glNewList, glNewList, NULL, 0),
    NAME_FUNC_OFFSET(   10, glEndList, glEndList, NULL, 1),
    NAME_FUNC_OFFSET(   20, glCallList, glCallList, NULL, 2),
    NAME_FUNC_OFFSET(   31, glCallLists, glCallLists, NULL, 3),
    NAME_FUNC_OFFSET(   43, glDeleteLists, glDeleteLists, NULL, 4),
    NAME_FUNC_OFFSET(   57, glGenLists, glGenLists, NULL, 5),
    NAME_FUNC_OFFSET(   68, glListBase, glListBase, NULL, 6),
    NAME_FUNC_OFFSET(   79, glBegin, glBegin, NULL, 7),
    NAME_FUNC_OFFSET(   87, glBitmap, glBitmap, NULL, 8),
    NAME_FUNC_OFFSET(   96, glColor3b, glColor3b, NULL, 9),
    NAME_FUNC_OFFSET(  106, glColor3bv, glColor3bv, NULL, 10),
    NAME_FUNC_OFFSET(  117, glColor3d, glColor3d, NULL, 11),
    NAME_FUNC_OFFSET(  127, glColor3dv, glColor3dv, NULL, 12),
    NAME_FUNC_OFFSET(  138, glColor3f, glColor3f, NULL, 13),
    NAME_FUNC_OFFSET(  148, glColor3fv, glColor3fv, NULL, 14),
    NAME_FUNC_OFFSET(  159, glColor3i, glColor3i, NULL, 15),
    NAME_FUNC_OFFSET(  169, glColor3iv, glColor3iv, NULL, 16),
    NAME_FUNC_OFFSET(  180, glColor3s, glColor3s, NULL, 17),
    NAME_FUNC_OFFSET(  190, glColor3sv, glColor3sv, NULL, 18),
    NAME_FUNC_OFFSET(  201, glColor3ub, glColor3ub, NULL, 19),
    NAME_FUNC_OFFSET(  212, glColor3ubv, glColor3ubv, NULL, 20),
    NAME_FUNC_OFFSET(  224, glColor3ui, glColor3ui, NULL, 21),
    NAME_FUNC_OFFSET(  235, glColor3uiv, glColor3uiv, NULL, 22),
    NAME_FUNC_OFFSET(  247, glColor3us, glColor3us, NULL, 23),
    NAME_FUNC_OFFSET(  258, glColor3usv, glColor3usv, NULL, 24),
    NAME_FUNC_OFFSET(  270, glColor4b, glColor4b, NULL, 25),
    NAME_FUNC_OFFSET(  280, glColor4bv, glColor4bv, NULL, 26),
    NAME_FUNC_OFFSET(  291, glColor4d, glColor4d, NULL, 27),
    NAME_FUNC_OFFSET(  301, glColor4dv, glColor4dv, NULL, 28),
    NAME_FUNC_OFFSET(  312, glColor4f, glColor4f, NULL, 29),
    NAME_FUNC_OFFSET(  322, glColor4fv, glColor4fv, NULL, 30),
    NAME_FUNC_OFFSET(  333, glColor4i, glColor4i, NULL, 31),
    NAME_FUNC_OFFSET(  343, glColor4iv, glColor4iv, NULL, 32),
    NAME_FUNC_OFFSET(  354, glColor4s, glColor4s, NULL, 33),
    NAME_FUNC_OFFSET(  364, glColor4sv, glColor4sv, NULL, 34),
    NAME_FUNC_OFFSET(  375, glColor4ub, glColor4ub, NULL, 35),
    NAME_FUNC_OFFSET(  386, glColor4ubv, glColor4ubv, NULL, 36),
    NAME_FUNC_OFFSET(  398, glColor4ui, glColor4ui, NULL, 37),
    NAME_FUNC_OFFSET(  409, glColor4uiv, glColor4uiv, NULL, 38),
    NAME_FUNC_OFFSET(  421, glColor4us, glColor4us, NULL, 39),
    NAME_FUNC_OFFSET(  432, glColor4usv, glColor4usv, NULL, 40),
    NAME_FUNC_OFFSET(  444, glEdgeFlag, glEdgeFlag, NULL, 41),
    NAME_FUNC_OFFSET(  455, glEdgeFlagv, glEdgeFlagv, NULL, 42),
    NAME_FUNC_OFFSET(  467, glEnd, glEnd, NULL, 43),
    NAME_FUNC_OFFSET(  473, glIndexd, glIndexd, NULL, 44),
    NAME_FUNC_OFFSET(  482, glIndexdv, glIndexdv, NULL, 45),
    NAME_FUNC_OFFSET(  492, glIndexf, glIndexf, NULL, 46),
    NAME_FUNC_OFFSET(  501, glIndexfv, glIndexfv, NULL, 47),
    NAME_FUNC_OFFSET(  511, glIndexi, glIndexi, NULL, 48),
    NAME_FUNC_OFFSET(  520, glIndexiv, glIndexiv, NULL, 49),
    NAME_FUNC_OFFSET(  530, glIndexs, glIndexs, NULL, 50),
    NAME_FUNC_OFFSET(  539, glIndexsv, glIndexsv, NULL, 51),
    NAME_FUNC_OFFSET(  549, glNormal3b, glNormal3b, NULL, 52),
    NAME_FUNC_OFFSET(  560, glNormal3bv, glNormal3bv, NULL, 53),
    NAME_FUNC_OFFSET(  572, glNormal3d, glNormal3d, NULL, 54),
    NAME_FUNC_OFFSET(  583, glNormal3dv, glNormal3dv, NULL, 55),
    NAME_FUNC_OFFSET(  595, glNormal3f, glNormal3f, NULL, 56),
    NAME_FUNC_OFFSET(  606, glNormal3fv, glNormal3fv, NULL, 57),
    NAME_FUNC_OFFSET(  618, glNormal3i, glNormal3i, NULL, 58),
    NAME_FUNC_OFFSET(  629, glNormal3iv, glNormal3iv, NULL, 59),
    NAME_FUNC_OFFSET(  641, glNormal3s, glNormal3s, NULL, 60),
    NAME_FUNC_OFFSET(  652, glNormal3sv, glNormal3sv, NULL, 61),
    NAME_FUNC_OFFSET(  664, glRasterPos2d, glRasterPos2d, NULL, 62),
    NAME_FUNC_OFFSET(  678, glRasterPos2dv, glRasterPos2dv, NULL, 63),
    NAME_FUNC_OFFSET(  693, glRasterPos2f, glRasterPos2f, NULL, 64),
    NAME_FUNC_OFFSET(  707, glRasterPos2fv, glRasterPos2fv, NULL, 65),
    NAME_FUNC_OFFSET(  722, glRasterPos2i, glRasterPos2i, NULL, 66),
    NAME_FUNC_OFFSET(  736, glRasterPos2iv, glRasterPos2iv, NULL, 67),
    NAME_FUNC_OFFSET(  751, glRasterPos2s, glRasterPos2s, NULL, 68),
    NAME_FUNC_OFFSET(  765, glRasterPos2sv, glRasterPos2sv, NULL, 69),
    NAME_FUNC_OFFSET(  780, glRasterPos3d, glRasterPos3d, NULL, 70),
    NAME_FUNC_OFFSET(  794, glRasterPos3dv, glRasterPos3dv, NULL, 71),
    NAME_FUNC_OFFSET(  809, glRasterPos3f, glRasterPos3f, NULL, 72),
    NAME_FUNC_OFFSET(  823, glRasterPos3fv, glRasterPos3fv, NULL, 73),
    NAME_FUNC_OFFSET(  838, glRasterPos3i, glRasterPos3i, NULL, 74),
    NAME_FUNC_OFFSET(  852, glRasterPos3iv, glRasterPos3iv, NULL, 75),
    NAME_FUNC_OFFSET(  867, glRasterPos3s, glRasterPos3s, NULL, 76),
    NAME_FUNC_OFFSET(  881, glRasterPos3sv, glRasterPos3sv, NULL, 77),
    NAME_FUNC_OFFSET(  896, glRasterPos4d, glRasterPos4d, NULL, 78),
    NAME_FUNC_OFFSET(  910, glRasterPos4dv, glRasterPos4dv, NULL, 79),
    NAME_FUNC_OFFSET(  925, glRasterPos4f, glRasterPos4f, NULL, 80),
    NAME_FUNC_OFFSET(  939, glRasterPos4fv, glRasterPos4fv, NULL, 81),
    NAME_FUNC_OFFSET(  954, glRasterPos4i, glRasterPos4i, NULL, 82),
    NAME_FUNC_OFFSET(  968, glRasterPos4iv, glRasterPos4iv, NULL, 83),
    NAME_FUNC_OFFSET(  983, glRasterPos4s, glRasterPos4s, NULL, 84),
    NAME_FUNC_OFFSET(  997, glRasterPos4sv, glRasterPos4sv, NULL, 85),
    NAME_FUNC_OFFSET( 1012, glRectd, glRectd, NULL, 86),
    NAME_FUNC_OFFSET( 1020, glRectdv, glRectdv, NULL, 87),
    NAME_FUNC_OFFSET( 1029, glRectf, glRectf, NULL, 88),
    NAME_FUNC_OFFSET( 1037, glRectfv, glRectfv, NULL, 89),
    NAME_FUNC_OFFSET( 1046, glRecti, glRecti, NULL, 90),
    NAME_FUNC_OFFSET( 1054, glRectiv, glRectiv, NULL, 91),
    NAME_FUNC_OFFSET( 1063, glRects, glRects, NULL, 92),
    NAME_FUNC_OFFSET( 1071, glRectsv, glRectsv, NULL, 93),
    NAME_FUNC_OFFSET( 1080, glTexCoord1d, glTexCoord1d, NULL, 94),
    NAME_FUNC_OFFSET( 1093, glTexCoord1dv, glTexCoord1dv, NULL, 95),
    NAME_FUNC_OFFSET( 1107, glTexCoord1f, glTexCoord1f, NULL, 96),
    NAME_FUNC_OFFSET( 1120, glTexCoord1fv, glTexCoord1fv, NULL, 97),
    NAME_FUNC_OFFSET( 1134, glTexCoord1i, glTexCoord1i, NULL, 98),
    NAME_FUNC_OFFSET( 1147, glTexCoord1iv, glTexCoord1iv, NULL, 99),
    NAME_FUNC_OFFSET( 1161, glTexCoord1s, glTexCoord1s, NULL, 100),
    NAME_FUNC_OFFSET( 1174, glTexCoord1sv, glTexCoord1sv, NULL, 101),
    NAME_FUNC_OFFSET( 1188, glTexCoord2d, glTexCoord2d, NULL, 102),
    NAME_FUNC_OFFSET( 1201, glTexCoord2dv, glTexCoord2dv, NULL, 103),
    NAME_FUNC_OFFSET( 1215, glTexCoord2f, glTexCoord2f, NULL, 104),
    NAME_FUNC_OFFSET( 1228, glTexCoord2fv, glTexCoord2fv, NULL, 105),
    NAME_FUNC_OFFSET( 1242, glTexCoord2i, glTexCoord2i, NULL, 106),
    NAME_FUNC_OFFSET( 1255, glTexCoord2iv, glTexCoord2iv, NULL, 107),
    NAME_FUNC_OFFSET( 1269, glTexCoord2s, glTexCoord2s, NULL, 108),
    NAME_FUNC_OFFSET( 1282, glTexCoord2sv, glTexCoord2sv, NULL, 109),
    NAME_FUNC_OFFSET( 1296, glTexCoord3d, glTexCoord3d, NULL, 110),
    NAME_FUNC_OFFSET( 1309, glTexCoord3dv, glTexCoord3dv, NULL, 111),
    NAME_FUNC_OFFSET( 1323, glTexCoord3f, glTexCoord3f, NULL, 112),
    NAME_FUNC_OFFSET( 1336, glTexCoord3fv, glTexCoord3fv, NULL, 113),
    NAME_FUNC_OFFSET( 1350, glTexCoord3i, glTexCoord3i, NULL, 114),
    NAME_FUNC_OFFSET( 1363, glTexCoord3iv, glTexCoord3iv, NULL, 115),
    NAME_FUNC_OFFSET( 1377, glTexCoord3s, glTexCoord3s, NULL, 116),
    NAME_FUNC_OFFSET( 1390, glTexCoord3sv, glTexCoord3sv, NULL, 117),
    NAME_FUNC_OFFSET( 1404, glTexCoord4d, glTexCoord4d, NULL, 118),
    NAME_FUNC_OFFSET( 1417, glTexCoord4dv, glTexCoord4dv, NULL, 119),
    NAME_FUNC_OFFSET( 1431, glTexCoord4f, glTexCoord4f, NULL, 120),
    NAME_FUNC_OFFSET( 1444, glTexCoord4fv, glTexCoord4fv, NULL, 121),
    NAME_FUNC_OFFSET( 1458, glTexCoord4i, glTexCoord4i, NULL, 122),
    NAME_FUNC_OFFSET( 1471, glTexCoord4iv, glTexCoord4iv, NULL, 123),
    NAME_FUNC_OFFSET( 1485, glTexCoord4s, glTexCoord4s, NULL, 124),
    NAME_FUNC_OFFSET( 1498, glTexCoord4sv, glTexCoord4sv, NULL, 125),
    NAME_FUNC_OFFSET( 1512, glVertex2d, glVertex2d, NULL, 126),
    NAME_FUNC_OFFSET( 1523, glVertex2dv, glVertex2dv, NULL, 127),
    NAME_FUNC_OFFSET( 1535, glVertex2f, glVertex2f, NULL, 128),
    NAME_FUNC_OFFSET( 1546, glVertex2fv, glVertex2fv, NULL, 129),
    NAME_FUNC_OFFSET( 1558, glVertex2i, glVertex2i, NULL, 130),
    NAME_FUNC_OFFSET( 1569, glVertex2iv, glVertex2iv, NULL, 131),
    NAME_FUNC_OFFSET( 1581, glVertex2s, glVertex2s, NULL, 132),
    NAME_FUNC_OFFSET( 1592, glVertex2sv, glVertex2sv, NULL, 133),
    NAME_FUNC_OFFSET( 1604, glVertex3d, glVertex3d, NULL, 134),
    NAME_FUNC_OFFSET( 1615, glVertex3dv, glVertex3dv, NULL, 135),
    NAME_FUNC_OFFSET( 1627, glVertex3f, glVertex3f, NULL, 136),
    NAME_FUNC_OFFSET( 1638, glVertex3fv, glVertex3fv, NULL, 137),
    NAME_FUNC_OFFSET( 1650, glVertex3i, glVertex3i, NULL, 138),
    NAME_FUNC_OFFSET( 1661, glVertex3iv, glVertex3iv, NULL, 139),
    NAME_FUNC_OFFSET( 1673, glVertex3s, glVertex3s, NULL, 140),
    NAME_FUNC_OFFSET( 1684, glVertex3sv, glVertex3sv, NULL, 141),
    NAME_FUNC_OFFSET( 1696, glVertex4d, glVertex4d, NULL, 142),
    NAME_FUNC_OFFSET( 1707, glVertex4dv, glVertex4dv, NULL, 143),
    NAME_FUNC_OFFSET( 1719, glVertex4f, glVertex4f, NULL, 144),
    NAME_FUNC_OFFSET( 1730, glVertex4fv, glVertex4fv, NULL, 145),
    NAME_FUNC_OFFSET( 1742, glVertex4i, glVertex4i, NULL, 146),
    NAME_FUNC_OFFSET( 1753, glVertex4iv, glVertex4iv, NULL, 147),
    NAME_FUNC_OFFSET( 1765, glVertex4s, glVertex4s, NULL, 148),
    NAME_FUNC_OFFSET( 1776, glVertex4sv, glVertex4sv, NULL, 149),
    NAME_FUNC_OFFSET( 1788, glClipPlane, glClipPlane, NULL, 150),
    NAME_FUNC_OFFSET( 1800, glColorMaterial, glColorMaterial, NULL, 151),
    NAME_FUNC_OFFSET( 1816, glCullFace, glCullFace, NULL, 152),
    NAME_FUNC_OFFSET( 1827, glFogf, glFogf, NULL, 153),
    NAME_FUNC_OFFSET( 1834, glFogfv, glFogfv, NULL, 154),
    NAME_FUNC_OFFSET( 1842, glFogi, glFogi, NULL, 155),
    NAME_FUNC_OFFSET( 1849, glFogiv, glFogiv, NULL, 156),
    NAME_FUNC_OFFSET( 1857, glFrontFace, glFrontFace, NULL, 157),
    NAME_FUNC_OFFSET( 1869, glHint, glHint, NULL, 158),
    NAME_FUNC_OFFSET( 1876, glLightf, glLightf, NULL, 159),
    NAME_FUNC_OFFSET( 1885, glLightfv, glLightfv, NULL, 160),
    NAME_FUNC_OFFSET( 1895, glLighti, glLighti, NULL, 161),
    NAME_FUNC_OFFSET( 1904, glLightiv, glLightiv, NULL, 162),
    NAME_FUNC_OFFSET( 1914, glLightModelf, glLightModelf, NULL, 163),
    NAME_FUNC_OFFSET( 1928, glLightModelfv, glLightModelfv, NULL, 164),
    NAME_FUNC_OFFSET( 1943, glLightModeli, glLightModeli, NULL, 165),
    NAME_FUNC_OFFSET( 1957, glLightModeliv, glLightModeliv, NULL, 166),
    NAME_FUNC_OFFSET( 1972, glLineStipple, glLineStipple, NULL, 167),
    NAME_FUNC_OFFSET( 1986, glLineWidth, glLineWidth, NULL, 168),
    NAME_FUNC_OFFSET( 1998, glMaterialf, glMaterialf, NULL, 169),
    NAME_FUNC_OFFSET( 2010, glMaterialfv, glMaterialfv, NULL, 170),
    NAME_FUNC_OFFSET( 2023, glMateriali, glMateriali, NULL, 171),
    NAME_FUNC_OFFSET( 2035, glMaterialiv, glMaterialiv, NULL, 172),
    NAME_FUNC_OFFSET( 2048, glPointSize, glPointSize, NULL, 173),
    NAME_FUNC_OFFSET( 2060, glPolygonMode, glPolygonMode, NULL, 174),
    NAME_FUNC_OFFSET( 2074, glPolygonStipple, glPolygonStipple, NULL, 175),
    NAME_FUNC_OFFSET( 2091, glScissor, glScissor, NULL, 176),
    NAME_FUNC_OFFSET( 2101, glShadeModel, glShadeModel, NULL, 177),
    NAME_FUNC_OFFSET( 2114, glTexParameterf, glTexParameterf, NULL, 178),
    NAME_FUNC_OFFSET( 2130, glTexParameterfv, glTexParameterfv, NULL, 179),
    NAME_FUNC_OFFSET( 2147, glTexParameteri, glTexParameteri, NULL, 180),
    NAME_FUNC_OFFSET( 2163, glTexParameteriv, glTexParameteriv, NULL, 181),
    NAME_FUNC_OFFSET( 2180, glTexImage1D, glTexImage1D, NULL, 182),
    NAME_FUNC_OFFSET( 2193, glTexImage2D, glTexImage2D, NULL, 183),
    NAME_FUNC_OFFSET( 2206, glTexEnvf, glTexEnvf, NULL, 184),
    NAME_FUNC_OFFSET( 2216, glTexEnvfv, glTexEnvfv, NULL, 185),
    NAME_FUNC_OFFSET( 2227, glTexEnvi, glTexEnvi, NULL, 186),
    NAME_FUNC_OFFSET( 2237, glTexEnviv, glTexEnviv, NULL, 187),
    NAME_FUNC_OFFSET( 2248, glTexGend, glTexGend, NULL, 188),
    NAME_FUNC_OFFSET( 2258, glTexGendv, glTexGendv, NULL, 189),
    NAME_FUNC_OFFSET( 2269, glTexGenf, glTexGenf, NULL, 190),
    NAME_FUNC_OFFSET( 2279, glTexGenfv, glTexGenfv, NULL, 191),
    NAME_FUNC_OFFSET( 2290, glTexGeni, glTexGeni, NULL, 192),
    NAME_FUNC_OFFSET( 2300, glTexGeniv, glTexGeniv, NULL, 193),
    NAME_FUNC_OFFSET( 2311, glFeedbackBuffer, glFeedbackBuffer, NULL, 194),
    NAME_FUNC_OFFSET( 2328, glSelectBuffer, glSelectBuffer, NULL, 195),
    NAME_FUNC_OFFSET( 2343, glRenderMode, glRenderMode, NULL, 196),
    NAME_FUNC_OFFSET( 2356, glInitNames, glInitNames, NULL, 197),
    NAME_FUNC_OFFSET( 2368, glLoadName, glLoadName, NULL, 198),
    NAME_FUNC_OFFSET( 2379, glPassThrough, glPassThrough, NULL, 199),
    NAME_FUNC_OFFSET( 2393, glPopName, glPopName, NULL, 200),
    NAME_FUNC_OFFSET( 2403, glPushName, glPushName, NULL, 201),
    NAME_FUNC_OFFSET( 2414, glDrawBuffer, glDrawBuffer, NULL, 202),
    NAME_FUNC_OFFSET( 2427, glClear, glClear, NULL, 203),
    NAME_FUNC_OFFSET( 2435, glClearAccum, glClearAccum, NULL, 204),
    NAME_FUNC_OFFSET( 2448, glClearIndex, glClearIndex, NULL, 205),
    NAME_FUNC_OFFSET( 2461, glClearColor, glClearColor, NULL, 206),
    NAME_FUNC_OFFSET( 2474, glClearStencil, glClearStencil, NULL, 207),
    NAME_FUNC_OFFSET( 2489, glClearDepth, glClearDepth, NULL, 208),
    NAME_FUNC_OFFSET( 2502, glStencilMask, glStencilMask, NULL, 209),
    NAME_FUNC_OFFSET( 2516, glColorMask, glColorMask, NULL, 210),
    NAME_FUNC_OFFSET( 2528, glDepthMask, glDepthMask, NULL, 211),
    NAME_FUNC_OFFSET( 2540, glIndexMask, glIndexMask, NULL, 212),
    NAME_FUNC_OFFSET( 2552, glAccum, glAccum, NULL, 213),
    NAME_FUNC_OFFSET( 2560, glDisable, glDisable, NULL, 214),
    NAME_FUNC_OFFSET( 2570, glEnable, glEnable, NULL, 215),
    NAME_FUNC_OFFSET( 2579, glFinish, glFinish, NULL, 216),
    NAME_FUNC_OFFSET( 2588, glFlush, glFlush, NULL, 217),
    NAME_FUNC_OFFSET( 2596, glPopAttrib, glPopAttrib, NULL, 218),
    NAME_FUNC_OFFSET( 2608, glPushAttrib, glPushAttrib, NULL, 219),
    NAME_FUNC_OFFSET( 2621, glMap1d, glMap1d, NULL, 220),
    NAME_FUNC_OFFSET( 2629, glMap1f, glMap1f, NULL, 221),
    NAME_FUNC_OFFSET( 2637, glMap2d, glMap2d, NULL, 222),
    NAME_FUNC_OFFSET( 2645, glMap2f, glMap2f, NULL, 223),
    NAME_FUNC_OFFSET( 2653, glMapGrid1d, glMapGrid1d, NULL, 224),
    NAME_FUNC_OFFSET( 2665, glMapGrid1f, glMapGrid1f, NULL, 225),
    NAME_FUNC_OFFSET( 2677, glMapGrid2d, glMapGrid2d, NULL, 226),
    NAME_FUNC_OFFSET( 2689, glMapGrid2f, glMapGrid2f, NULL, 227),
    NAME_FUNC_OFFSET( 2701, glEvalCoord1d, glEvalCoord1d, NULL, 228),
    NAME_FUNC_OFFSET( 2715, glEvalCoord1dv, glEvalCoord1dv, NULL, 229),
    NAME_FUNC_OFFSET( 2730, glEvalCoord1f, glEvalCoord1f, NULL, 230),
    NAME_FUNC_OFFSET( 2744, glEvalCoord1fv, glEvalCoord1fv, NULL, 231),
    NAME_FUNC_OFFSET( 2759, glEvalCoord2d, glEvalCoord2d, NULL, 232),
    NAME_FUNC_OFFSET( 2773, glEvalCoord2dv, glEvalCoord2dv, NULL, 233),
    NAME_FUNC_OFFSET( 2788, glEvalCoord2f, glEvalCoord2f, NULL, 234),
    NAME_FUNC_OFFSET( 2802, glEvalCoord2fv, glEvalCoord2fv, NULL, 235),
    NAME_FUNC_OFFSET( 2817, glEvalMesh1, glEvalMesh1, NULL, 236),
    NAME_FUNC_OFFSET( 2829, glEvalPoint1, glEvalPoint1, NULL, 237),
    NAME_FUNC_OFFSET( 2842, glEvalMesh2, glEvalMesh2, NULL, 238),
    NAME_FUNC_OFFSET( 2854, glEvalPoint2, glEvalPoint2, NULL, 239),
    NAME_FUNC_OFFSET( 2867, glAlphaFunc, glAlphaFunc, NULL, 240),
    NAME_FUNC_OFFSET( 2879, glBlendFunc, glBlendFunc, NULL, 241),
    NAME_FUNC_OFFSET( 2891, glLogicOp, glLogicOp, NULL, 242),
    NAME_FUNC_OFFSET( 2901, glStencilFunc, glStencilFunc, NULL, 243),
    NAME_FUNC_OFFSET( 2915, glStencilOp, glStencilOp, NULL, 244),
    NAME_FUNC_OFFSET( 2927, glDepthFunc, glDepthFunc, NULL, 245),
    NAME_FUNC_OFFSET( 2939, glPixelZoom, glPixelZoom, NULL, 246),
    NAME_FUNC_OFFSET( 2951, glPixelTransferf, glPixelTransferf, NULL, 247),
    NAME_FUNC_OFFSET( 2968, glPixelTransferi, glPixelTransferi, NULL, 248),
    NAME_FUNC_OFFSET( 2985, glPixelStoref, glPixelStoref, NULL, 249),
    NAME_FUNC_OFFSET( 2999, glPixelStorei, glPixelStorei, NULL, 250),
    NAME_FUNC_OFFSET( 3013, glPixelMapfv, glPixelMapfv, NULL, 251),
    NAME_FUNC_OFFSET( 3026, glPixelMapuiv, glPixelMapuiv, NULL, 252),
    NAME_FUNC_OFFSET( 3040, glPixelMapusv, glPixelMapusv, NULL, 253),
    NAME_FUNC_OFFSET( 3054, glReadBuffer, glReadBuffer, NULL, 254),
    NAME_FUNC_OFFSET( 3067, glCopyPixels, glCopyPixels, NULL, 255),
    NAME_FUNC_OFFSET( 3080, glReadPixels, glReadPixels, NULL, 256),
    NAME_FUNC_OFFSET( 3093, glDrawPixels, glDrawPixels, NULL, 257),
    NAME_FUNC_OFFSET( 3106, glGetBooleanv, glGetBooleanv, NULL, 258),
    NAME_FUNC_OFFSET( 3120, glGetClipPlane, glGetClipPlane, NULL, 259),
    NAME_FUNC_OFFSET( 3135, glGetDoublev, glGetDoublev, NULL, 260),
    NAME_FUNC_OFFSET( 3148, glGetError, glGetError, NULL, 261),
    NAME_FUNC_OFFSET( 3159, glGetFloatv, glGetFloatv, NULL, 262),
    NAME_FUNC_OFFSET( 3171, glGetIntegerv, glGetIntegerv, NULL, 263),
    NAME_FUNC_OFFSET( 3185, glGetLightfv, glGetLightfv, NULL, 264),
    NAME_FUNC_OFFSET( 3198, glGetLightiv, glGetLightiv, NULL, 265),
    NAME_FUNC_OFFSET( 3211, glGetMapdv, glGetMapdv, NULL, 266),
    NAME_FUNC_OFFSET( 3222, glGetMapfv, glGetMapfv, NULL, 267),
    NAME_FUNC_OFFSET( 3233, glGetMapiv, glGetMapiv, NULL, 268),
    NAME_FUNC_OFFSET( 3244, glGetMaterialfv, glGetMaterialfv, NULL, 269),
    NAME_FUNC_OFFSET( 3260, glGetMaterialiv, glGetMaterialiv, NULL, 270),
    NAME_FUNC_OFFSET( 3276, glGetPixelMapfv, glGetPixelMapfv, NULL, 271),
    NAME_FUNC_OFFSET( 3292, glGetPixelMapuiv, glGetPixelMapuiv, NULL, 272),
    NAME_FUNC_OFFSET( 3309, glGetPixelMapusv, glGetPixelMapusv, NULL, 273),
    NAME_FUNC_OFFSET( 3326, glGetPolygonStipple, glGetPolygonStipple, NULL, 274),
    NAME_FUNC_OFFSET( 3346, glGetString, glGetString, NULL, 275),
    NAME_FUNC_OFFSET( 3358, glGetTexEnvfv, glGetTexEnvfv, NULL, 276),
    NAME_FUNC_OFFSET( 3372, glGetTexEnviv, glGetTexEnviv, NULL, 277),
    NAME_FUNC_OFFSET( 3386, glGetTexGendv, glGetTexGendv, NULL, 278),
    NAME_FUNC_OFFSET( 3400, glGetTexGenfv, glGetTexGenfv, NULL, 279),
    NAME_FUNC_OFFSET( 3414, glGetTexGeniv, glGetTexGeniv, NULL, 280),
    NAME_FUNC_OFFSET( 3428, glGetTexImage, glGetTexImage, NULL, 281),
    NAME_FUNC_OFFSET( 3442, glGetTexParameterfv, glGetTexParameterfv, NULL, 282),
    NAME_FUNC_OFFSET( 3462, glGetTexParameteriv, glGetTexParameteriv, NULL, 283),
    NAME_FUNC_OFFSET( 3482, glGetTexLevelParameterfv, glGetTexLevelParameterfv, NULL, 284),
    NAME_FUNC_OFFSET( 3507, glGetTexLevelParameteriv, glGetTexLevelParameteriv, NULL, 285),
    NAME_FUNC_OFFSET( 3532, glIsEnabled, glIsEnabled, NULL, 286),
    NAME_FUNC_OFFSET( 3544, glIsList, glIsList, NULL, 287),
    NAME_FUNC_OFFSET( 3553, glDepthRange, glDepthRange, NULL, 288),
    NAME_FUNC_OFFSET( 3566, glFrustum, glFrustum, NULL, 289),
    NAME_FUNC_OFFSET( 3576, glLoadIdentity, glLoadIdentity, NULL, 290),
    NAME_FUNC_OFFSET( 3591, glLoadMatrixf, glLoadMatrixf, NULL, 291),
    NAME_FUNC_OFFSET( 3605, glLoadMatrixd, glLoadMatrixd, NULL, 292),
    NAME_FUNC_OFFSET( 3619, glMatrixMode, glMatrixMode, NULL, 293),
    NAME_FUNC_OFFSET( 3632, glMultMatrixf, glMultMatrixf, NULL, 294),
    NAME_FUNC_OFFSET( 3646, glMultMatrixd, glMultMatrixd, NULL, 295),
    NAME_FUNC_OFFSET( 3660, glOrtho, glOrtho, NULL, 296),
    NAME_FUNC_OFFSET( 3668, glPopMatrix, glPopMatrix, NULL, 297),
    NAME_FUNC_OFFSET( 3680, glPushMatrix, glPushMatrix, NULL, 298),
    NAME_FUNC_OFFSET( 3693, glRotated, glRotated, NULL, 299),
    NAME_FUNC_OFFSET( 3703, glRotatef, glRotatef, NULL, 300),
    NAME_FUNC_OFFSET( 3713, glScaled, glScaled, NULL, 301),
    NAME_FUNC_OFFSET( 3722, glScalef, glScalef, NULL, 302),
    NAME_FUNC_OFFSET( 3731, glTranslated, glTranslated, NULL, 303),
    NAME_FUNC_OFFSET( 3744, glTranslatef, glTranslatef, NULL, 304),
    NAME_FUNC_OFFSET( 3757, glViewport, glViewport, NULL, 305),
    NAME_FUNC_OFFSET( 3768, glArrayElement, glArrayElement, NULL, 306),
    NAME_FUNC_OFFSET( 3783, glBindTexture, glBindTexture, NULL, 307),
    NAME_FUNC_OFFSET( 3797, glColorPointer, glColorPointer, NULL, 308),
    NAME_FUNC_OFFSET( 3812, glDisableClientState, glDisableClientState, NULL, 309),
    NAME_FUNC_OFFSET( 3833, glDrawArrays, glDrawArrays, NULL, 310),
    NAME_FUNC_OFFSET( 3846, glDrawElements, glDrawElements, NULL, 311),
    NAME_FUNC_OFFSET( 3861, glEdgeFlagPointer, glEdgeFlagPointer, NULL, 312),
    NAME_FUNC_OFFSET( 3879, glEnableClientState, glEnableClientState, NULL, 313),
    NAME_FUNC_OFFSET( 3899, glIndexPointer, glIndexPointer, NULL, 314),
    NAME_FUNC_OFFSET( 3914, glIndexub, glIndexub, NULL, 315),
    NAME_FUNC_OFFSET( 3924, glIndexubv, glIndexubv, NULL, 316),
    NAME_FUNC_OFFSET( 3935, glInterleavedArrays, glInterleavedArrays, NULL, 317),
    NAME_FUNC_OFFSET( 3955, glNormalPointer, glNormalPointer, NULL, 318),
    NAME_FUNC_OFFSET( 3971, glPolygonOffset, glPolygonOffset, NULL, 319),
    NAME_FUNC_OFFSET( 3987, glTexCoordPointer, glTexCoordPointer, NULL, 320),
    NAME_FUNC_OFFSET( 4005, glVertexPointer, glVertexPointer, NULL, 321),
    NAME_FUNC_OFFSET( 4021, glAreTexturesResident, glAreTexturesResident, NULL, 322),
    NAME_FUNC_OFFSET( 4043, glCopyTexImage1D, glCopyTexImage1D, NULL, 323),
    NAME_FUNC_OFFSET( 4060, glCopyTexImage2D, glCopyTexImage2D, NULL, 324),
    NAME_FUNC_OFFSET( 4077, glCopyTexSubImage1D, glCopyTexSubImage1D, NULL, 325),
    NAME_FUNC_OFFSET( 4097, glCopyTexSubImage2D, glCopyTexSubImage2D, NULL, 326),
    NAME_FUNC_OFFSET( 4117, glDeleteTextures, glDeleteTextures, NULL, 327),
    NAME_FUNC_OFFSET( 4134, glGenTextures, glGenTextures, NULL, 328),
    NAME_FUNC_OFFSET( 4148, glGetPointerv, glGetPointerv, NULL, 329),
    NAME_FUNC_OFFSET( 4162, glIsTexture, glIsTexture, NULL, 330),
    NAME_FUNC_OFFSET( 4174, glPrioritizeTextures, glPrioritizeTextures, NULL, 331),
    NAME_FUNC_OFFSET( 4195, glTexSubImage1D, glTexSubImage1D, NULL, 332),
    NAME_FUNC_OFFSET( 4211, glTexSubImage2D, glTexSubImage2D, NULL, 333),
    NAME_FUNC_OFFSET( 4227, glPopClientAttrib, glPopClientAttrib, NULL, 334),
    NAME_FUNC_OFFSET( 4245, glPushClientAttrib, glPushClientAttrib, NULL, 335),
    NAME_FUNC_OFFSET( 4264, glBlendColor, glBlendColor, NULL, 336),
    NAME_FUNC_OFFSET( 4277, glBlendEquation, glBlendEquation, NULL, 337),
    NAME_FUNC_OFFSET( 4293, glDrawRangeElements, glDrawRangeElements, NULL, 338),
    NAME_FUNC_OFFSET( 4313, glColorTable, glColorTable, NULL, 339),
    NAME_FUNC_OFFSET( 4326, glColorTableParameterfv, glColorTableParameterfv, NULL, 340),
    NAME_FUNC_OFFSET( 4350, glColorTableParameteriv, glColorTableParameteriv, NULL, 341),
    NAME_FUNC_OFFSET( 4374, glCopyColorTable, glCopyColorTable, NULL, 342),
    NAME_FUNC_OFFSET( 4391, glGetColorTable, glGetColorTable, NULL, 343),
    NAME_FUNC_OFFSET( 4407, glGetColorTableParameterfv, glGetColorTableParameterfv, NULL, 344),
    NAME_FUNC_OFFSET( 4434, glGetColorTableParameteriv, glGetColorTableParameteriv, NULL, 345),
    NAME_FUNC_OFFSET( 4461, glColorSubTable, glColorSubTable, NULL, 346),
    NAME_FUNC_OFFSET( 4477, glCopyColorSubTable, glCopyColorSubTable, NULL, 347),
    NAME_FUNC_OFFSET( 4497, glConvolutionFilter1D, glConvolutionFilter1D, NULL, 348),
    NAME_FUNC_OFFSET( 4519, glConvolutionFilter2D, glConvolutionFilter2D, NULL, 349),
    NAME_FUNC_OFFSET( 4541, glConvolutionParameterf, glConvolutionParameterf, NULL, 350),
    NAME_FUNC_OFFSET( 4565, glConvolutionParameterfv, glConvolutionParameterfv, NULL, 351),
    NAME_FUNC_OFFSET( 4590, glConvolutionParameteri, glConvolutionParameteri, NULL, 352),
    NAME_FUNC_OFFSET( 4614, glConvolutionParameteriv, glConvolutionParameteriv, NULL, 353),
    NAME_FUNC_OFFSET( 4639, glCopyConvolutionFilter1D, glCopyConvolutionFilter1D, NULL, 354),
    NAME_FUNC_OFFSET( 4665, glCopyConvolutionFilter2D, glCopyConvolutionFilter2D, NULL, 355),
    NAME_FUNC_OFFSET( 4691, glGetConvolutionFilter, glGetConvolutionFilter, NULL, 356),
    NAME_FUNC_OFFSET( 4714, glGetConvolutionParameterfv, glGetConvolutionParameterfv, NULL, 357),
    NAME_FUNC_OFFSET( 4742, glGetConvolutionParameteriv, glGetConvolutionParameteriv, NULL, 358),
    NAME_FUNC_OFFSET( 4770, glGetSeparableFilter, glGetSeparableFilter, NULL, 359),
    NAME_FUNC_OFFSET( 4791, glSeparableFilter2D, glSeparableFilter2D, NULL, 360),
    NAME_FUNC_OFFSET( 4811, glGetHistogram, glGetHistogram, NULL, 361),
    NAME_FUNC_OFFSET( 4826, glGetHistogramParameterfv, glGetHistogramParameterfv, NULL, 362),
    NAME_FUNC_OFFSET( 4852, glGetHistogramParameteriv, glGetHistogramParameteriv, NULL, 363),
    NAME_FUNC_OFFSET( 4878, glGetMinmax, glGetMinmax, NULL, 364),
    NAME_FUNC_OFFSET( 4890, glGetMinmaxParameterfv, glGetMinmaxParameterfv, NULL, 365),
    NAME_FUNC_OFFSET( 4913, glGetMinmaxParameteriv, glGetMinmaxParameteriv, NULL, 366),
    NAME_FUNC_OFFSET( 4936, glHistogram, glHistogram, NULL, 367),
    NAME_FUNC_OFFSET( 4948, glMinmax, glMinmax, NULL, 368),
    NAME_FUNC_OFFSET( 4957, glResetHistogram, glResetHistogram, NULL, 369),
    NAME_FUNC_OFFSET( 4974, glResetMinmax, glResetMinmax, NULL, 370),
    NAME_FUNC_OFFSET( 4988, glTexImage3D, glTexImage3D, NULL, 371),
    NAME_FUNC_OFFSET( 5001, glTexSubImage3D, glTexSubImage3D, NULL, 372),
    NAME_FUNC_OFFSET( 5017, glCopyTexSubImage3D, glCopyTexSubImage3D, NULL, 373),
    NAME_FUNC_OFFSET( 5037, glActiveTexture, glActiveTexture, NULL, 374),
    NAME_FUNC_OFFSET( 5053, glClientActiveTexture, glClientActiveTexture, NULL, 375),
    NAME_FUNC_OFFSET( 5075, glMultiTexCoord1d, glMultiTexCoord1d, NULL, 376),
    NAME_FUNC_OFFSET( 5093, glMultiTexCoord1dv, glMultiTexCoord1dv, NULL, 377),
    NAME_FUNC_OFFSET( 5112, glMultiTexCoord1fARB, glMultiTexCoord1fARB, NULL, 378),
    NAME_FUNC_OFFSET( 5133, glMultiTexCoord1fvARB, glMultiTexCoord1fvARB, NULL, 379),
    NAME_FUNC_OFFSET( 5155, glMultiTexCoord1i, glMultiTexCoord1i, NULL, 380),
    NAME_FUNC_OFFSET( 5173, glMultiTexCoord1iv, glMultiTexCoord1iv, NULL, 381),
    NAME_FUNC_OFFSET( 5192, glMultiTexCoord1s, glMultiTexCoord1s, NULL, 382),
    NAME_FUNC_OFFSET( 5210, glMultiTexCoord1sv, glMultiTexCoord1sv, NULL, 383),
    NAME_FUNC_OFFSET( 5229, glMultiTexCoord2d, glMultiTexCoord2d, NULL, 384),
    NAME_FUNC_OFFSET( 5247, glMultiTexCoord2dv, glMultiTexCoord2dv, NULL, 385),
    NAME_FUNC_OFFSET( 5266, glMultiTexCoord2fARB, glMultiTexCoord2fARB, NULL, 386),
    NAME_FUNC_OFFSET( 5287, glMultiTexCoord2fvARB, glMultiTexCoord2fvARB, NULL, 387),
    NAME_FUNC_OFFSET( 5309, glMultiTexCoord2i, glMultiTexCoord2i, NULL, 388),
    NAME_FUNC_OFFSET( 5327, glMultiTexCoord2iv, glMultiTexCoord2iv, NULL, 389),
    NAME_FUNC_OFFSET( 5346, glMultiTexCoord2s, glMultiTexCoord2s, NULL, 390),
    NAME_FUNC_OFFSET( 5364, glMultiTexCoord2sv, glMultiTexCoord2sv, NULL, 391),
    NAME_FUNC_OFFSET( 5383, glMultiTexCoord3d, glMultiTexCoord3d, NULL, 392),
    NAME_FUNC_OFFSET( 5401, glMultiTexCoord3dv, glMultiTexCoord3dv, NULL, 393),
    NAME_FUNC_OFFSET( 5420, glMultiTexCoord3fARB, glMultiTexCoord3fARB, NULL, 394),
    NAME_FUNC_OFFSET( 5441, glMultiTexCoord3fvARB, glMultiTexCoord3fvARB, NULL, 395),
    NAME_FUNC_OFFSET( 5463, glMultiTexCoord3i, glMultiTexCoord3i, NULL, 396),
    NAME_FUNC_OFFSET( 5481, glMultiTexCoord3iv, glMultiTexCoord3iv, NULL, 397),
    NAME_FUNC_OFFSET( 5500, glMultiTexCoord3s, glMultiTexCoord3s, NULL, 398),
    NAME_FUNC_OFFSET( 5518, glMultiTexCoord3sv, glMultiTexCoord3sv, NULL, 399),
    NAME_FUNC_OFFSET( 5537, glMultiTexCoord4d, glMultiTexCoord4d, NULL, 400),
    NAME_FUNC_OFFSET( 5555, glMultiTexCoord4dv, glMultiTexCoord4dv, NULL, 401),
    NAME_FUNC_OFFSET( 5574, glMultiTexCoord4fARB, glMultiTexCoord4fARB, NULL, 402),
    NAME_FUNC_OFFSET( 5595, glMultiTexCoord4fvARB, glMultiTexCoord4fvARB, NULL, 403),
    NAME_FUNC_OFFSET( 5617, glMultiTexCoord4i, glMultiTexCoord4i, NULL, 404),
    NAME_FUNC_OFFSET( 5635, glMultiTexCoord4iv, glMultiTexCoord4iv, NULL, 405),
    NAME_FUNC_OFFSET( 5654, glMultiTexCoord4s, glMultiTexCoord4s, NULL, 406),
    NAME_FUNC_OFFSET( 5672, glMultiTexCoord4sv, glMultiTexCoord4sv, NULL, 407),
    NAME_FUNC_OFFSET( 5691, glCompressedTexImage1D, glCompressedTexImage1D, NULL, 408),
    NAME_FUNC_OFFSET( 5714, glCompressedTexImage2D, glCompressedTexImage2D, NULL, 409),
    NAME_FUNC_OFFSET( 5737, glCompressedTexImage3D, glCompressedTexImage3D, NULL, 410),
    NAME_FUNC_OFFSET( 5760, glCompressedTexSubImage1D, glCompressedTexSubImage1D, NULL, 411),
    NAME_FUNC_OFFSET( 5786, glCompressedTexSubImage2D, glCompressedTexSubImage2D, NULL, 412),
    NAME_FUNC_OFFSET( 5812, glCompressedTexSubImage3D, glCompressedTexSubImage3D, NULL, 413),
    NAME_FUNC_OFFSET( 5838, glGetCompressedTexImage, glGetCompressedTexImage, NULL, 414),
    NAME_FUNC_OFFSET( 5862, glLoadTransposeMatrixd, glLoadTransposeMatrixd, NULL, 415),
    NAME_FUNC_OFFSET( 5885, glLoadTransposeMatrixf, glLoadTransposeMatrixf, NULL, 416),
    NAME_FUNC_OFFSET( 5908, glMultTransposeMatrixd, glMultTransposeMatrixd, NULL, 417),
    NAME_FUNC_OFFSET( 5931, glMultTransposeMatrixf, glMultTransposeMatrixf, NULL, 418),
    NAME_FUNC_OFFSET( 5954, glSampleCoverage, glSampleCoverage, NULL, 419),
    NAME_FUNC_OFFSET( 5971, glBlendFuncSeparate, glBlendFuncSeparate, NULL, 420),
    NAME_FUNC_OFFSET( 5991, glFogCoordPointer, glFogCoordPointer, NULL, 421),
    NAME_FUNC_OFFSET( 6009, glFogCoordd, glFogCoordd, NULL, 422),
    NAME_FUNC_OFFSET( 6021, glFogCoorddv, glFogCoorddv, NULL, 423),
    NAME_FUNC_OFFSET( 6034, glMultiDrawArrays, glMultiDrawArrays, NULL, 424),
    NAME_FUNC_OFFSET( 6052, glPointParameterf, glPointParameterf, NULL, 425),
    NAME_FUNC_OFFSET( 6070, glPointParameterfv, glPointParameterfv, NULL, 426),
    NAME_FUNC_OFFSET( 6089, glPointParameteri, glPointParameteri, NULL, 427),
    NAME_FUNC_OFFSET( 6107, glPointParameteriv, glPointParameteriv, NULL, 428),
    NAME_FUNC_OFFSET( 6126, glSecondaryColor3b, glSecondaryColor3b, NULL, 429),
    NAME_FUNC_OFFSET( 6145, glSecondaryColor3bv, glSecondaryColor3bv, NULL, 430),
    NAME_FUNC_OFFSET( 6165, glSecondaryColor3d, glSecondaryColor3d, NULL, 431),
    NAME_FUNC_OFFSET( 6184, glSecondaryColor3dv, glSecondaryColor3dv, NULL, 432),
    NAME_FUNC_OFFSET( 6204, glSecondaryColor3i, glSecondaryColor3i, NULL, 433),
    NAME_FUNC_OFFSET( 6223, glSecondaryColor3iv, glSecondaryColor3iv, NULL, 434),
    NAME_FUNC_OFFSET( 6243, glSecondaryColor3s, glSecondaryColor3s, NULL, 435),
    NAME_FUNC_OFFSET( 6262, glSecondaryColor3sv, glSecondaryColor3sv, NULL, 436),
    NAME_FUNC_OFFSET( 6282, glSecondaryColor3ub, glSecondaryColor3ub, NULL, 437),
    NAME_FUNC_OFFSET( 6302, glSecondaryColor3ubv, glSecondaryColor3ubv, NULL, 438),
    NAME_FUNC_OFFSET( 6323, glSecondaryColor3ui, glSecondaryColor3ui, NULL, 439),
    NAME_FUNC_OFFSET( 6343, glSecondaryColor3uiv, glSecondaryColor3uiv, NULL, 440),
    NAME_FUNC_OFFSET( 6364, glSecondaryColor3us, glSecondaryColor3us, NULL, 441),
    NAME_FUNC_OFFSET( 6384, glSecondaryColor3usv, glSecondaryColor3usv, NULL, 442),
    NAME_FUNC_OFFSET( 6405, glSecondaryColorPointer, glSecondaryColorPointer, NULL, 443),
    NAME_FUNC_OFFSET( 6429, glWindowPos2d, glWindowPos2d, NULL, 444),
    NAME_FUNC_OFFSET( 6443, glWindowPos2dv, glWindowPos2dv, NULL, 445),
    NAME_FUNC_OFFSET( 6458, glWindowPos2f, glWindowPos2f, NULL, 446),
    NAME_FUNC_OFFSET( 6472, glWindowPos2fv, glWindowPos2fv, NULL, 447),
    NAME_FUNC_OFFSET( 6487, glWindowPos2i, glWindowPos2i, NULL, 448),
    NAME_FUNC_OFFSET( 6501, glWindowPos2iv, glWindowPos2iv, NULL, 449),
    NAME_FUNC_OFFSET( 6516, glWindowPos2s, glWindowPos2s, NULL, 450),
    NAME_FUNC_OFFSET( 6530, glWindowPos2sv, glWindowPos2sv, NULL, 451),
    NAME_FUNC_OFFSET( 6545, glWindowPos3d, glWindowPos3d, NULL, 452),
    NAME_FUNC_OFFSET( 6559, glWindowPos3dv, glWindowPos3dv, NULL, 453),
    NAME_FUNC_OFFSET( 6574, glWindowPos3f, glWindowPos3f, NULL, 454),
    NAME_FUNC_OFFSET( 6588, glWindowPos3fv, glWindowPos3fv, NULL, 455),
    NAME_FUNC_OFFSET( 6603, glWindowPos3i, glWindowPos3i, NULL, 456),
    NAME_FUNC_OFFSET( 6617, glWindowPos3iv, glWindowPos3iv, NULL, 457),
    NAME_FUNC_OFFSET( 6632, glWindowPos3s, glWindowPos3s, NULL, 458),
    NAME_FUNC_OFFSET( 6646, glWindowPos3sv, glWindowPos3sv, NULL, 459),
    NAME_FUNC_OFFSET( 6661, glBeginQuery, glBeginQuery, NULL, 460),
    NAME_FUNC_OFFSET( 6674, glBindBuffer, glBindBuffer, NULL, 461),
    NAME_FUNC_OFFSET( 6687, glBufferData, glBufferData, NULL, 462),
    NAME_FUNC_OFFSET( 6700, glBufferSubData, glBufferSubData, NULL, 463),
    NAME_FUNC_OFFSET( 6716, glDeleteBuffers, glDeleteBuffers, NULL, 464),
    NAME_FUNC_OFFSET( 6732, glDeleteQueries, glDeleteQueries, NULL, 465),
    NAME_FUNC_OFFSET( 6748, glEndQuery, glEndQuery, NULL, 466),
    NAME_FUNC_OFFSET( 6759, glGenBuffers, glGenBuffers, NULL, 467),
    NAME_FUNC_OFFSET( 6772, glGenQueries, glGenQueries, NULL, 468),
    NAME_FUNC_OFFSET( 6785, glGetBufferParameteriv, glGetBufferParameteriv, NULL, 469),
    NAME_FUNC_OFFSET( 6808, glGetBufferPointerv, glGetBufferPointerv, NULL, 470),
    NAME_FUNC_OFFSET( 6828, glGetBufferSubData, glGetBufferSubData, NULL, 471),
    NAME_FUNC_OFFSET( 6847, glGetQueryObjectiv, glGetQueryObjectiv, NULL, 472),
    NAME_FUNC_OFFSET( 6866, glGetQueryObjectuiv, glGetQueryObjectuiv, NULL, 473),
    NAME_FUNC_OFFSET( 6886, glGetQueryiv, glGetQueryiv, NULL, 474),
    NAME_FUNC_OFFSET( 6899, glIsBuffer, glIsBuffer, NULL, 475),
    NAME_FUNC_OFFSET( 6910, glIsQuery, glIsQuery, NULL, 476),
    NAME_FUNC_OFFSET( 6920, glMapBuffer, glMapBuffer, NULL, 477),
    NAME_FUNC_OFFSET( 6932, glUnmapBuffer, glUnmapBuffer, NULL, 478),
    NAME_FUNC_OFFSET( 6946, glAttachShader, glAttachShader, NULL, 479),
    NAME_FUNC_OFFSET( 6961, glBindAttribLocation, glBindAttribLocation, NULL, 480),
    NAME_FUNC_OFFSET( 6982, glBlendEquationSeparate, glBlendEquationSeparate, NULL, 481),
    NAME_FUNC_OFFSET( 7006, glCompileShader, glCompileShader, NULL, 482),
    NAME_FUNC_OFFSET( 7022, glCreateProgram, glCreateProgram, NULL, 483),
    NAME_FUNC_OFFSET( 7038, glCreateShader, glCreateShader, NULL, 484),
    NAME_FUNC_OFFSET( 7053, glDeleteProgram, glDeleteProgram, NULL, 485),
    NAME_FUNC_OFFSET( 7069, glDeleteShader, glDeleteShader, NULL, 486),
    NAME_FUNC_OFFSET( 7084, glDetachShader, glDetachShader, NULL, 487),
    NAME_FUNC_OFFSET( 7099, glDisableVertexAttribArray, glDisableVertexAttribArray, NULL, 488),
    NAME_FUNC_OFFSET( 7126, glDrawBuffers, glDrawBuffers, NULL, 489),
    NAME_FUNC_OFFSET( 7140, glEnableVertexAttribArray, glEnableVertexAttribArray, NULL, 490),
    NAME_FUNC_OFFSET( 7166, glGetActiveAttrib, glGetActiveAttrib, NULL, 491),
    NAME_FUNC_OFFSET( 7184, glGetActiveUniform, glGetActiveUniform, NULL, 492),
    NAME_FUNC_OFFSET( 7203, glGetAttachedShaders, glGetAttachedShaders, NULL, 493),
    NAME_FUNC_OFFSET( 7224, glGetAttribLocation, glGetAttribLocation, NULL, 494),
    NAME_FUNC_OFFSET( 7244, glGetProgramInfoLog, glGetProgramInfoLog, NULL, 495),
    NAME_FUNC_OFFSET( 7264, glGetProgramiv, glGetProgramiv, NULL, 496),
    NAME_FUNC_OFFSET( 7279, glGetShaderInfoLog, glGetShaderInfoLog, NULL, 497),
    NAME_FUNC_OFFSET( 7298, glGetShaderSource, glGetShaderSource, NULL, 498),
    NAME_FUNC_OFFSET( 7316, glGetShaderiv, glGetShaderiv, NULL, 499),
    NAME_FUNC_OFFSET( 7330, glGetUniformLocation, glGetUniformLocation, NULL, 500),
    NAME_FUNC_OFFSET( 7351, glGetUniformfv, glGetUniformfv, NULL, 501),
    NAME_FUNC_OFFSET( 7366, glGetUniformiv, glGetUniformiv, NULL, 502),
    NAME_FUNC_OFFSET( 7381, glGetVertexAttribPointerv, glGetVertexAttribPointerv, NULL, 503),
    NAME_FUNC_OFFSET( 7407, glGetVertexAttribdv, glGetVertexAttribdv, NULL, 504),
    NAME_FUNC_OFFSET( 7427, glGetVertexAttribfv, glGetVertexAttribfv, NULL, 505),
    NAME_FUNC_OFFSET( 7447, glGetVertexAttribiv, glGetVertexAttribiv, NULL, 506),
    NAME_FUNC_OFFSET( 7467, glIsProgram, glIsProgram, NULL, 507),
    NAME_FUNC_OFFSET( 7479, glIsShader, glIsShader, NULL, 508),
    NAME_FUNC_OFFSET( 7490, glLinkProgram, glLinkProgram, NULL, 509),
    NAME_FUNC_OFFSET( 7504, glShaderSource, glShaderSource, NULL, 510),
    NAME_FUNC_OFFSET( 7519, glStencilFuncSeparate, glStencilFuncSeparate, NULL, 511),
    NAME_FUNC_OFFSET( 7541, glStencilMaskSeparate, glStencilMaskSeparate, NULL, 512),
    NAME_FUNC_OFFSET( 7563, glStencilOpSeparate, glStencilOpSeparate, NULL, 513),
    NAME_FUNC_OFFSET( 7583, glUniform1f, glUniform1f, NULL, 514),
    NAME_FUNC_OFFSET( 7595, glUniform1fv, glUniform1fv, NULL, 515),
    NAME_FUNC_OFFSET( 7608, glUniform1i, glUniform1i, NULL, 516),
    NAME_FUNC_OFFSET( 7620, glUniform1iv, glUniform1iv, NULL, 517),
    NAME_FUNC_OFFSET( 7633, glUniform2f, glUniform2f, NULL, 518),
    NAME_FUNC_OFFSET( 7645, glUniform2fv, glUniform2fv, NULL, 519),
    NAME_FUNC_OFFSET( 7658, glUniform2i, glUniform2i, NULL, 520),
    NAME_FUNC_OFFSET( 7670, glUniform2iv, glUniform2iv, NULL, 521),
    NAME_FUNC_OFFSET( 7683, glUniform3f, glUniform3f, NULL, 522),
    NAME_FUNC_OFFSET( 7695, glUniform3fv, glUniform3fv, NULL, 523),
    NAME_FUNC_OFFSET( 7708, glUniform3i, glUniform3i, NULL, 524),
    NAME_FUNC_OFFSET( 7720, glUniform3iv, glUniform3iv, NULL, 525),
    NAME_FUNC_OFFSET( 7733, glUniform4f, glUniform4f, NULL, 526),
    NAME_FUNC_OFFSET( 7745, glUniform4fv, glUniform4fv, NULL, 527),
    NAME_FUNC_OFFSET( 7758, glUniform4i, glUniform4i, NULL, 528),
    NAME_FUNC_OFFSET( 7770, glUniform4iv, glUniform4iv, NULL, 529),
    NAME_FUNC_OFFSET( 7783, glUniformMatrix2fv, glUniformMatrix2fv, NULL, 530),
    NAME_FUNC_OFFSET( 7802, glUniformMatrix3fv, glUniformMatrix3fv, NULL, 531),
    NAME_FUNC_OFFSET( 7821, glUniformMatrix4fv, glUniformMatrix4fv, NULL, 532),
    NAME_FUNC_OFFSET( 7840, glUseProgram, glUseProgram, NULL, 533),
    NAME_FUNC_OFFSET( 7853, glValidateProgram, glValidateProgram, NULL, 534),
    NAME_FUNC_OFFSET( 7871, glVertexAttrib1d, glVertexAttrib1d, NULL, 535),
    NAME_FUNC_OFFSET( 7888, glVertexAttrib1dv, glVertexAttrib1dv, NULL, 536),
    NAME_FUNC_OFFSET( 7906, glVertexAttrib1s, glVertexAttrib1s, NULL, 537),
    NAME_FUNC_OFFSET( 7923, glVertexAttrib1sv, glVertexAttrib1sv, NULL, 538),
    NAME_FUNC_OFFSET( 7941, glVertexAttrib2d, glVertexAttrib2d, NULL, 539),
    NAME_FUNC_OFFSET( 7958, glVertexAttrib2dv, glVertexAttrib2dv, NULL, 540),
    NAME_FUNC_OFFSET( 7976, glVertexAttrib2s, glVertexAttrib2s, NULL, 541),
    NAME_FUNC_OFFSET( 7993, glVertexAttrib2sv, glVertexAttrib2sv, NULL, 542),
    NAME_FUNC_OFFSET( 8011, glVertexAttrib3d, glVertexAttrib3d, NULL, 543),
    NAME_FUNC_OFFSET( 8028, glVertexAttrib3dv, glVertexAttrib3dv, NULL, 544),
    NAME_FUNC_OFFSET( 8046, glVertexAttrib3s, glVertexAttrib3s, NULL, 545),
    NAME_FUNC_OFFSET( 8063, glVertexAttrib3sv, glVertexAttrib3sv, NULL, 546),
    NAME_FUNC_OFFSET( 8081, glVertexAttrib4Nbv, glVertexAttrib4Nbv, NULL, 547),
    NAME_FUNC_OFFSET( 8100, glVertexAttrib4Niv, glVertexAttrib4Niv, NULL, 548),
    NAME_FUNC_OFFSET( 8119, glVertexAttrib4Nsv, glVertexAttrib4Nsv, NULL, 549),
    NAME_FUNC_OFFSET( 8138, glVertexAttrib4Nub, glVertexAttrib4Nub, NULL, 550),
    NAME_FUNC_OFFSET( 8157, glVertexAttrib4Nubv, glVertexAttrib4Nubv, NULL, 551),
    NAME_FUNC_OFFSET( 8177, glVertexAttrib4Nuiv, glVertexAttrib4Nuiv, NULL, 552),
    NAME_FUNC_OFFSET( 8197, glVertexAttrib4Nusv, glVertexAttrib4Nusv, NULL, 553),
    NAME_FUNC_OFFSET( 8217, glVertexAttrib4bv, glVertexAttrib4bv, NULL, 554),
    NAME_FUNC_OFFSET( 8235, glVertexAttrib4d, glVertexAttrib4d, NULL, 555),
    NAME_FUNC_OFFSET( 8252, glVertexAttrib4dv, glVertexAttrib4dv, NULL, 556),
    NAME_FUNC_OFFSET( 8270, glVertexAttrib4iv, glVertexAttrib4iv, NULL, 557),
    NAME_FUNC_OFFSET( 8288, glVertexAttrib4s, glVertexAttrib4s, NULL, 558),
    NAME_FUNC_OFFSET( 8305, glVertexAttrib4sv, glVertexAttrib4sv, NULL, 559),
    NAME_FUNC_OFFSET( 8323, glVertexAttrib4ubv, glVertexAttrib4ubv, NULL, 560),
    NAME_FUNC_OFFSET( 8342, glVertexAttrib4uiv, glVertexAttrib4uiv, NULL, 561),
    NAME_FUNC_OFFSET( 8361, glVertexAttrib4usv, glVertexAttrib4usv, NULL, 562),
    NAME_FUNC_OFFSET( 8380, glVertexAttribPointer, glVertexAttribPointer, NULL, 563),
    NAME_FUNC_OFFSET( 8402, glUniformMatrix2x3fv, glUniformMatrix2x3fv, NULL, 564),
    NAME_FUNC_OFFSET( 8423, glUniformMatrix2x4fv, glUniformMatrix2x4fv, NULL, 565),
    NAME_FUNC_OFFSET( 8444, glUniformMatrix3x2fv, glUniformMatrix3x2fv, NULL, 566),
    NAME_FUNC_OFFSET( 8465, glUniformMatrix3x4fv, glUniformMatrix3x4fv, NULL, 567),
    NAME_FUNC_OFFSET( 8486, glUniformMatrix4x2fv, glUniformMatrix4x2fv, NULL, 568),
    NAME_FUNC_OFFSET( 8507, glUniformMatrix4x3fv, glUniformMatrix4x3fv, NULL, 569),
    NAME_FUNC_OFFSET( 8528, glBeginConditionalRender, glBeginConditionalRender, NULL, 570),
    NAME_FUNC_OFFSET( 8553, glBeginTransformFeedback, glBeginTransformFeedback, NULL, 571),
    NAME_FUNC_OFFSET( 8578, glBindBufferBase, glBindBufferBase, NULL, 572),
    NAME_FUNC_OFFSET( 8595, glBindBufferRange, glBindBufferRange, NULL, 573),
    NAME_FUNC_OFFSET( 8613, glBindFragDataLocation, glBindFragDataLocation, NULL, 574),
    NAME_FUNC_OFFSET( 8636, glClampColor, glClampColor, NULL, 575),
    NAME_FUNC_OFFSET( 8649, glClearBufferfi, glClearBufferfi, NULL, 576),
    NAME_FUNC_OFFSET( 8665, glClearBufferfv, glClearBufferfv, NULL, 577),
    NAME_FUNC_OFFSET( 8681, glClearBufferiv, glClearBufferiv, NULL, 578),
    NAME_FUNC_OFFSET( 8697, glClearBufferuiv, glClearBufferuiv, NULL, 579),
    NAME_FUNC_OFFSET( 8714, glColorMaski, glColorMaski, NULL, 580),
    NAME_FUNC_OFFSET( 8727, glDisablei, glDisablei, NULL, 581),
    NAME_FUNC_OFFSET( 8738, glEnablei, glEnablei, NULL, 582),
    NAME_FUNC_OFFSET( 8748, glEndConditionalRender, glEndConditionalRender, NULL, 583),
    NAME_FUNC_OFFSET( 8771, glEndTransformFeedback, glEndTransformFeedback, NULL, 584),
    NAME_FUNC_OFFSET( 8794, glGetBooleani_v, glGetBooleani_v, NULL, 585),
    NAME_FUNC_OFFSET( 8810, glGetFragDataLocation, glGetFragDataLocation, NULL, 586),
    NAME_FUNC_OFFSET( 8832, glGetIntegeri_v, glGetIntegeri_v, NULL, 587),
    NAME_FUNC_OFFSET( 8848, glGetStringi, glGetStringi, NULL, 588),
    NAME_FUNC_OFFSET( 8861, glGetTexParameterIiv, glGetTexParameterIiv, NULL, 589),
    NAME_FUNC_OFFSET( 8882, glGetTexParameterIuiv, glGetTexParameterIuiv, NULL, 590),
    NAME_FUNC_OFFSET( 8904, glGetTransformFeedbackVarying, glGetTransformFeedbackVarying, NULL, 591),
    NAME_FUNC_OFFSET( 8934, glGetUniformuiv, glGetUniformuiv, NULL, 592),
    NAME_FUNC_OFFSET( 8950, glGetVertexAttribIiv, glGetVertexAttribIiv, NULL, 593),
    NAME_FUNC_OFFSET( 8971, glGetVertexAttribIuiv, glGetVertexAttribIuiv, NULL, 594),
    NAME_FUNC_OFFSET( 8993, glIsEnabledi, glIsEnabledi, NULL, 595),
    NAME_FUNC_OFFSET( 9006, glTexParameterIiv, glTexParameterIiv, NULL, 596),
    NAME_FUNC_OFFSET( 9024, glTexParameterIuiv, glTexParameterIuiv, NULL, 597),
    NAME_FUNC_OFFSET( 9043, glTransformFeedbackVaryings, glTransformFeedbackVaryings, NULL, 598),
    NAME_FUNC_OFFSET( 9071, glUniform1ui, glUniform1ui, NULL, 599),
    NAME_FUNC_OFFSET( 9084, glUniform1uiv, glUniform1uiv, NULL, 600),
    NAME_FUNC_OFFSET( 9098, glUniform2ui, glUniform2ui, NULL, 601),
    NAME_FUNC_OFFSET( 9111, glUniform2uiv, glUniform2uiv, NULL, 602),
    NAME_FUNC_OFFSET( 9125, glUniform3ui, glUniform3ui, NULL, 603),
    NAME_FUNC_OFFSET( 9138, glUniform3uiv, glUniform3uiv, NULL, 604),
    NAME_FUNC_OFFSET( 9152, glUniform4ui, glUniform4ui, NULL, 605),
    NAME_FUNC_OFFSET( 9165, glUniform4uiv, glUniform4uiv, NULL, 606),
    NAME_FUNC_OFFSET( 9179, glVertexAttribI1iv, glVertexAttribI1iv, NULL, 607),
    NAME_FUNC_OFFSET( 9198, glVertexAttribI1uiv, glVertexAttribI1uiv, NULL, 608),
    NAME_FUNC_OFFSET( 9218, glVertexAttribI4bv, glVertexAttribI4bv, NULL, 609),
    NAME_FUNC_OFFSET( 9237, glVertexAttribI4sv, glVertexAttribI4sv, NULL, 610),
    NAME_FUNC_OFFSET( 9256, glVertexAttribI4ubv, glVertexAttribI4ubv, NULL, 611),
    NAME_FUNC_OFFSET( 9276, glVertexAttribI4usv, glVertexAttribI4usv, NULL, 612),
    NAME_FUNC_OFFSET( 9296, glVertexAttribIPointer, glVertexAttribIPointer, NULL, 613),
    NAME_FUNC_OFFSET( 9319, glPrimitiveRestartIndex, glPrimitiveRestartIndex, NULL, 614),
    NAME_FUNC_OFFSET( 9343, glTexBuffer, glTexBuffer, NULL, 615),
    NAME_FUNC_OFFSET( 9355, glFramebufferTexture, glFramebufferTexture, NULL, 616),
    NAME_FUNC_OFFSET( 9376, glGetBufferParameteri64v, glGetBufferParameteri64v, NULL, 617),
    NAME_FUNC_OFFSET( 9401, glGetInteger64i_v, glGetInteger64i_v, NULL, 618),
    NAME_FUNC_OFFSET( 9419, glVertexAttribDivisor, glVertexAttribDivisor, NULL, 619),
    NAME_FUNC_OFFSET( 9441, glBindProgramARB, glBindProgramARB, NULL, 620),
    NAME_FUNC_OFFSET( 9458, glDeleteProgramsARB, glDeleteProgramsARB, NULL, 621),
    NAME_FUNC_OFFSET( 9478, glGenProgramsARB, glGenProgramsARB, NULL, 622),
    NAME_FUNC_OFFSET( 9495, glGetProgramEnvParameterdvARB, glGetProgramEnvParameterdvARB, NULL, 623),
    NAME_FUNC_OFFSET( 9525, glGetProgramEnvParameterfvARB, glGetProgramEnvParameterfvARB, NULL, 624),
    NAME_FUNC_OFFSET( 9555, glGetProgramLocalParameterdvARB, glGetProgramLocalParameterdvARB, NULL, 625),
    NAME_FUNC_OFFSET( 9587, glGetProgramLocalParameterfvARB, glGetProgramLocalParameterfvARB, NULL, 626),
    NAME_FUNC_OFFSET( 9619, glGetProgramStringARB, glGetProgramStringARB, NULL, 627),
    NAME_FUNC_OFFSET( 9641, glGetProgramivARB, glGetProgramivARB, NULL, 628),
    NAME_FUNC_OFFSET( 9659, glIsProgramARB, glIsProgramARB, NULL, 629),
    NAME_FUNC_OFFSET( 9674, glProgramEnvParameter4dARB, glProgramEnvParameter4dARB, NULL, 630),
    NAME_FUNC_OFFSET( 9701, glProgramEnvParameter4dvARB, glProgramEnvParameter4dvARB, NULL, 631),
    NAME_FUNC_OFFSET( 9729, glProgramEnvParameter4fARB, glProgramEnvParameter4fARB, NULL, 632),
    NAME_FUNC_OFFSET( 9756, glProgramEnvParameter4fvARB, glProgramEnvParameter4fvARB, NULL, 633),
    NAME_FUNC_OFFSET( 9784, glProgramLocalParameter4dARB, glProgramLocalParameter4dARB, NULL, 634),
    NAME_FUNC_OFFSET( 9813, glProgramLocalParameter4dvARB, glProgramLocalParameter4dvARB, NULL, 635),
    NAME_FUNC_OFFSET( 9843, glProgramLocalParameter4fARB, glProgramLocalParameter4fARB, NULL, 636),
    NAME_FUNC_OFFSET( 9872, glProgramLocalParameter4fvARB, glProgramLocalParameter4fvARB, NULL, 637),
    NAME_FUNC_OFFSET( 9902, glProgramStringARB, glProgramStringARB, NULL, 638),
    NAME_FUNC_OFFSET( 9921, glVertexAttrib1fARB, glVertexAttrib1fARB, NULL, 639),
    NAME_FUNC_OFFSET( 9941, glVertexAttrib1fvARB, glVertexAttrib1fvARB, NULL, 640),
    NAME_FUNC_OFFSET( 9962, glVertexAttrib2fARB, glVertexAttrib2fARB, NULL, 641),
    NAME_FUNC_OFFSET( 9982, glVertexAttrib2fvARB, glVertexAttrib2fvARB, NULL, 642),
    NAME_FUNC_OFFSET(10003, glVertexAttrib3fARB, glVertexAttrib3fARB, NULL, 643),
    NAME_FUNC_OFFSET(10023, glVertexAttrib3fvARB, glVertexAttrib3fvARB, NULL, 644),
    NAME_FUNC_OFFSET(10044, glVertexAttrib4fARB, glVertexAttrib4fARB, NULL, 645),
    NAME_FUNC_OFFSET(10064, glVertexAttrib4fvARB, glVertexAttrib4fvARB, NULL, 646),
    NAME_FUNC_OFFSET(10085, glAttachObjectARB, glAttachObjectARB, NULL, 647),
    NAME_FUNC_OFFSET(10103, glCreateProgramObjectARB, glCreateProgramObjectARB, NULL, 648),
    NAME_FUNC_OFFSET(10128, glCreateShaderObjectARB, glCreateShaderObjectARB, NULL, 649),
    NAME_FUNC_OFFSET(10152, glDeleteObjectARB, glDeleteObjectARB, NULL, 650),
    NAME_FUNC_OFFSET(10170, glDetachObjectARB, glDetachObjectARB, NULL, 651),
    NAME_FUNC_OFFSET(10188, glGetAttachedObjectsARB, glGetAttachedObjectsARB, NULL, 652),
    NAME_FUNC_OFFSET(10212, glGetHandleARB, glGetHandleARB, NULL, 653),
    NAME_FUNC_OFFSET(10227, glGetInfoLogARB, glGetInfoLogARB, NULL, 654),
    NAME_FUNC_OFFSET(10243, glGetObjectParameterfvARB, glGetObjectParameterfvARB, NULL, 655),
    NAME_FUNC_OFFSET(10269, glGetObjectParameterivARB, glGetObjectParameterivARB, NULL, 656),
    NAME_FUNC_OFFSET(10295, glDrawArraysInstancedARB, glDrawArraysInstancedARB, NULL, 657),
    NAME_FUNC_OFFSET(10320, glDrawElementsInstancedARB, glDrawElementsInstancedARB, NULL, 658),
    NAME_FUNC_OFFSET(10347, glBindFramebuffer, glBindFramebuffer, NULL, 659),
    NAME_FUNC_OFFSET(10365, glBindRenderbuffer, glBindRenderbuffer, NULL, 660),
    NAME_FUNC_OFFSET(10384, glBlitFramebuffer, glBlitFramebuffer, NULL, 661),
    NAME_FUNC_OFFSET(10402, glCheckFramebufferStatus, glCheckFramebufferStatus, NULL, 662),
    NAME_FUNC_OFFSET(10427, glDeleteFramebuffers, glDeleteFramebuffers, NULL, 663),
    NAME_FUNC_OFFSET(10448, glDeleteRenderbuffers, glDeleteRenderbuffers, NULL, 664),
    NAME_FUNC_OFFSET(10470, glFramebufferRenderbuffer, glFramebufferRenderbuffer, NULL, 665),
    NAME_FUNC_OFFSET(10496, glFramebufferTexture1D, glFramebufferTexture1D, NULL, 666),
    NAME_FUNC_OFFSET(10519, glFramebufferTexture2D, glFramebufferTexture2D, NULL, 667),
    NAME_FUNC_OFFSET(10542, glFramebufferTexture3D, glFramebufferTexture3D, NULL, 668),
    NAME_FUNC_OFFSET(10565, glFramebufferTextureLayer, glFramebufferTextureLayer, NULL, 669),
    NAME_FUNC_OFFSET(10591, glGenFramebuffers, glGenFramebuffers, NULL, 670),
    NAME_FUNC_OFFSET(10609, glGenRenderbuffers, glGenRenderbuffers, NULL, 671),
    NAME_FUNC_OFFSET(10628, glGenerateMipmap, glGenerateMipmap, NULL, 672),
    NAME_FUNC_OFFSET(10645, glGetFramebufferAttachmentParameteriv, glGetFramebufferAttachmentParameteriv, NULL, 673),
    NAME_FUNC_OFFSET(10683, glGetRenderbufferParameteriv, glGetRenderbufferParameteriv, NULL, 674),
    NAME_FUNC_OFFSET(10712, glIsFramebuffer, glIsFramebuffer, NULL, 675),
    NAME_FUNC_OFFSET(10728, glIsRenderbuffer, glIsRenderbuffer, NULL, 676),
    NAME_FUNC_OFFSET(10745, glRenderbufferStorage, glRenderbufferStorage, NULL, 677),
    NAME_FUNC_OFFSET(10767, glRenderbufferStorageMultisample, glRenderbufferStorageMultisample, NULL, 678),
    NAME_FUNC_OFFSET(10800, glFramebufferTextureFaceARB, glFramebufferTextureFaceARB, NULL, 679),
    NAME_FUNC_OFFSET(10828, glFlushMappedBufferRange, glFlushMappedBufferRange, NULL, 680),
    NAME_FUNC_OFFSET(10853, glMapBufferRange, glMapBufferRange, NULL, 681),
    NAME_FUNC_OFFSET(10870, glBindVertexArray, glBindVertexArray, NULL, 682),
    NAME_FUNC_OFFSET(10888, glDeleteVertexArrays, glDeleteVertexArrays, NULL, 683),
    NAME_FUNC_OFFSET(10909, glGenVertexArrays, glGenVertexArrays, NULL, 684),
    NAME_FUNC_OFFSET(10927, glIsVertexArray, glIsVertexArray, NULL, 685),
    NAME_FUNC_OFFSET(10943, glGetActiveUniformBlockName, glGetActiveUniformBlockName, NULL, 686),
    NAME_FUNC_OFFSET(10971, glGetActiveUniformBlockiv, glGetActiveUniformBlockiv, NULL, 687),
    NAME_FUNC_OFFSET(10997, glGetActiveUniformName, glGetActiveUniformName, NULL, 688),
    NAME_FUNC_OFFSET(11020, glGetActiveUniformsiv, glGetActiveUniformsiv, NULL, 689),
    NAME_FUNC_OFFSET(11042, glGetUniformBlockIndex, glGetUniformBlockIndex, NULL, 690),
    NAME_FUNC_OFFSET(11065, glGetUniformIndices, glGetUniformIndices, NULL, 691),
    NAME_FUNC_OFFSET(11085, glUniformBlockBinding, glUniformBlockBinding, NULL, 692),
    NAME_FUNC_OFFSET(11107, glCopyBufferSubData, glCopyBufferSubData, NULL, 693),
    NAME_FUNC_OFFSET(11127, glClientWaitSync, glClientWaitSync, NULL, 694),
    NAME_FUNC_OFFSET(11144, glDeleteSync, glDeleteSync, NULL, 695),
    NAME_FUNC_OFFSET(11157, glFenceSync, glFenceSync, NULL, 696),
    NAME_FUNC_OFFSET(11169, glGetInteger64v, glGetInteger64v, NULL, 697),
    NAME_FUNC_OFFSET(11185, glGetSynciv, glGetSynciv, NULL, 698),
    NAME_FUNC_OFFSET(11197, glIsSync, glIsSync, NULL, 699),
    NAME_FUNC_OFFSET(11206, glWaitSync, glWaitSync, NULL, 700),
    NAME_FUNC_OFFSET(11217, glDrawElementsBaseVertex, glDrawElementsBaseVertex, NULL, 701),
    NAME_FUNC_OFFSET(11242, glDrawElementsInstancedBaseVertex, glDrawElementsInstancedBaseVertex, NULL, 702),
    NAME_FUNC_OFFSET(11276, glDrawRangeElementsBaseVertex, glDrawRangeElementsBaseVertex, NULL, 703),
    NAME_FUNC_OFFSET(11306, glMultiDrawElementsBaseVertex, glMultiDrawElementsBaseVertex, NULL, 704),
    NAME_FUNC_OFFSET(11336, glProvokingVertex, glProvokingVertex, NULL, 705),
    NAME_FUNC_OFFSET(11354, glBlendEquationSeparateiARB, glBlendEquationSeparateiARB, NULL, 706),
    NAME_FUNC_OFFSET(11382, glBlendEquationiARB, glBlendEquationiARB, NULL, 707),
    NAME_FUNC_OFFSET(11402, glBlendFuncSeparateiARB, glBlendFuncSeparateiARB, NULL, 708),
    NAME_FUNC_OFFSET(11426, glBlendFunciARB, glBlendFunciARB, NULL, 709),
    NAME_FUNC_OFFSET(11442, glBindFragDataLocationIndexed, glBindFragDataLocationIndexed, NULL, 710),
    NAME_FUNC_OFFSET(11472, glGetFragDataIndex, glGetFragDataIndex, NULL, 711),
    NAME_FUNC_OFFSET(11491, glBindSampler, glBindSampler, NULL, 712),
    NAME_FUNC_OFFSET(11505, glDeleteSamplers, glDeleteSamplers, NULL, 713),
    NAME_FUNC_OFFSET(11522, glGenSamplers, glGenSamplers, NULL, 714),
    NAME_FUNC_OFFSET(11536, glGetSamplerParameterIiv, glGetSamplerParameterIiv, NULL, 715),
    NAME_FUNC_OFFSET(11561, glGetSamplerParameterIuiv, glGetSamplerParameterIuiv, NULL, 716),
    NAME_FUNC_OFFSET(11587, glGetSamplerParameterfv, glGetSamplerParameterfv, NULL, 717),
    NAME_FUNC_OFFSET(11611, glGetSamplerParameteriv, glGetSamplerParameteriv, NULL, 718),
    NAME_FUNC_OFFSET(11635, glIsSampler, glIsSampler, NULL, 719),
    NAME_FUNC_OFFSET(11647, glSamplerParameterIiv, glSamplerParameterIiv, NULL, 720),
    NAME_FUNC_OFFSET(11669, glSamplerParameterIuiv, glSamplerParameterIuiv, NULL, 721),
    NAME_FUNC_OFFSET(11692, glSamplerParameterf, glSamplerParameterf, NULL, 722),
    NAME_FUNC_OFFSET(11712, glSamplerParameterfv, glSamplerParameterfv, NULL, 723),
    NAME_FUNC_OFFSET(11733, glSamplerParameteri, glSamplerParameteri, NULL, 724),
    NAME_FUNC_OFFSET(11753, glSamplerParameteriv, glSamplerParameteriv, NULL, 725),
    NAME_FUNC_OFFSET(11774, gl_dispatch_stub_726, gl_dispatch_stub_726, NULL, 726),
    NAME_FUNC_OFFSET(11795, gl_dispatch_stub_727, gl_dispatch_stub_727, NULL, 727),
    NAME_FUNC_OFFSET(11817, gl_dispatch_stub_728, gl_dispatch_stub_728, NULL, 728),
    NAME_FUNC_OFFSET(11832, glColorP3ui, glColorP3ui, NULL, 729),
    NAME_FUNC_OFFSET(11844, glColorP3uiv, glColorP3uiv, NULL, 730),
    NAME_FUNC_OFFSET(11857, glColorP4ui, glColorP4ui, NULL, 731),
    NAME_FUNC_OFFSET(11869, glColorP4uiv, glColorP4uiv, NULL, 732),
    NAME_FUNC_OFFSET(11882, glMultiTexCoordP1ui, glMultiTexCoordP1ui, NULL, 733),
    NAME_FUNC_OFFSET(11902, glMultiTexCoordP1uiv, glMultiTexCoordP1uiv, NULL, 734),
    NAME_FUNC_OFFSET(11923, glMultiTexCoordP2ui, glMultiTexCoordP2ui, NULL, 735),
    NAME_FUNC_OFFSET(11943, glMultiTexCoordP2uiv, glMultiTexCoordP2uiv, NULL, 736),
    NAME_FUNC_OFFSET(11964, glMultiTexCoordP3ui, glMultiTexCoordP3ui, NULL, 737),
    NAME_FUNC_OFFSET(11984, glMultiTexCoordP3uiv, glMultiTexCoordP3uiv, NULL, 738),
    NAME_FUNC_OFFSET(12005, glMultiTexCoordP4ui, glMultiTexCoordP4ui, NULL, 739),
    NAME_FUNC_OFFSET(12025, glMultiTexCoordP4uiv, glMultiTexCoordP4uiv, NULL, 740),
    NAME_FUNC_OFFSET(12046, glNormalP3ui, glNormalP3ui, NULL, 741),
    NAME_FUNC_OFFSET(12059, glNormalP3uiv, glNormalP3uiv, NULL, 742),
    NAME_FUNC_OFFSET(12073, glSecondaryColorP3ui, glSecondaryColorP3ui, NULL, 743),
    NAME_FUNC_OFFSET(12094, glSecondaryColorP3uiv, glSecondaryColorP3uiv, NULL, 744),
    NAME_FUNC_OFFSET(12116, glTexCoordP1ui, glTexCoordP1ui, NULL, 745),
    NAME_FUNC_OFFSET(12131, glTexCoordP1uiv, glTexCoordP1uiv, NULL, 746),
    NAME_FUNC_OFFSET(12147, glTexCoordP2ui, glTexCoordP2ui, NULL, 747),
    NAME_FUNC_OFFSET(12162, glTexCoordP2uiv, glTexCoordP2uiv, NULL, 748),
    NAME_FUNC_OFFSET(12178, glTexCoordP3ui, glTexCoordP3ui, NULL, 749),
    NAME_FUNC_OFFSET(12193, glTexCoordP3uiv, glTexCoordP3uiv, NULL, 750),
    NAME_FUNC_OFFSET(12209, glTexCoordP4ui, glTexCoordP4ui, NULL, 751),
    NAME_FUNC_OFFSET(12224, glTexCoordP4uiv, glTexCoordP4uiv, NULL, 752),
    NAME_FUNC_OFFSET(12240, glVertexAttribP1ui, glVertexAttribP1ui, NULL, 753),
    NAME_FUNC_OFFSET(12259, glVertexAttribP1uiv, glVertexAttribP1uiv, NULL, 754),
    NAME_FUNC_OFFSET(12279, glVertexAttribP2ui, glVertexAttribP2ui, NULL, 755),
    NAME_FUNC_OFFSET(12298, glVertexAttribP2uiv, glVertexAttribP2uiv, NULL, 756),
    NAME_FUNC_OFFSET(12318, glVertexAttribP3ui, glVertexAttribP3ui, NULL, 757),
    NAME_FUNC_OFFSET(12337, glVertexAttribP3uiv, glVertexAttribP3uiv, NULL, 758),
    NAME_FUNC_OFFSET(12357, glVertexAttribP4ui, glVertexAttribP4ui, NULL, 759),
    NAME_FUNC_OFFSET(12376, glVertexAttribP4uiv, glVertexAttribP4uiv, NULL, 760),
    NAME_FUNC_OFFSET(12396, glVertexP2ui, glVertexP2ui, NULL, 761),
    NAME_FUNC_OFFSET(12409, glVertexP2uiv, glVertexP2uiv, NULL, 762),
    NAME_FUNC_OFFSET(12423, glVertexP3ui, glVertexP3ui, NULL, 763),
    NAME_FUNC_OFFSET(12436, glVertexP3uiv, glVertexP3uiv, NULL, 764),
    NAME_FUNC_OFFSET(12450, glVertexP4ui, glVertexP4ui, NULL, 765),
    NAME_FUNC_OFFSET(12463, glVertexP4uiv, glVertexP4uiv, NULL, 766),
    NAME_FUNC_OFFSET(12477, glBindTransformFeedback, glBindTransformFeedback, NULL, 767),
    NAME_FUNC_OFFSET(12501, glDeleteTransformFeedbacks, glDeleteTransformFeedbacks, NULL, 768),
    NAME_FUNC_OFFSET(12528, glDrawTransformFeedback, glDrawTransformFeedback, NULL, 769),
    NAME_FUNC_OFFSET(12552, glGenTransformFeedbacks, glGenTransformFeedbacks, NULL, 770),
    NAME_FUNC_OFFSET(12576, glIsTransformFeedback, glIsTransformFeedback, NULL, 771),
    NAME_FUNC_OFFSET(12598, glPauseTransformFeedback, glPauseTransformFeedback, NULL, 772),
    NAME_FUNC_OFFSET(12623, glResumeTransformFeedback, glResumeTransformFeedback, NULL, 773),
    NAME_FUNC_OFFSET(12649, glBeginQueryIndexed, glBeginQueryIndexed, NULL, 774),
    NAME_FUNC_OFFSET(12669, glDrawTransformFeedbackStream, glDrawTransformFeedbackStream, NULL, 775),
    NAME_FUNC_OFFSET(12699, glEndQueryIndexed, glEndQueryIndexed, NULL, 776),
    NAME_FUNC_OFFSET(12717, glGetQueryIndexediv, glGetQueryIndexediv, NULL, 777),
    NAME_FUNC_OFFSET(12737, glClearDepthf, glClearDepthf, NULL, 778),
    NAME_FUNC_OFFSET(12751, glDepthRangef, glDepthRangef, NULL, 779),
    NAME_FUNC_OFFSET(12765, glGetShaderPrecisionFormat, glGetShaderPrecisionFormat, NULL, 780),
    NAME_FUNC_OFFSET(12792, glReleaseShaderCompiler, glReleaseShaderCompiler, NULL, 781),
    NAME_FUNC_OFFSET(12816, glShaderBinary, glShaderBinary, NULL, 782),
    NAME_FUNC_OFFSET(12831, glGetProgramBinary, glGetProgramBinary, NULL, 783),
    NAME_FUNC_OFFSET(12850, glProgramBinary, glProgramBinary, NULL, 784),
    NAME_FUNC_OFFSET(12866, glProgramParameteri, glProgramParameteri, NULL, 785),
    NAME_FUNC_OFFSET(12886, glDebugMessageCallbackARB, glDebugMessageCallbackARB, NULL, 786),
    NAME_FUNC_OFFSET(12912, glDebugMessageControlARB, glDebugMessageControlARB, NULL, 787),
    NAME_FUNC_OFFSET(12937, glDebugMessageInsertARB, glDebugMessageInsertARB, NULL, 788),
    NAME_FUNC_OFFSET(12961, glGetDebugMessageLogARB, glGetDebugMessageLogARB, NULL, 789),
    NAME_FUNC_OFFSET(12985, glGetGraphicsResetStatusARB, glGetGraphicsResetStatusARB, NULL, 790),
    NAME_FUNC_OFFSET(13013, glGetnColorTableARB, glGetnColorTableARB, NULL, 791),
    NAME_FUNC_OFFSET(13033, glGetnCompressedTexImageARB, glGetnCompressedTexImageARB, NULL, 792),
    NAME_FUNC_OFFSET(13061, glGetnConvolutionFilterARB, glGetnConvolutionFilterARB, NULL, 793),
    NAME_FUNC_OFFSET(13088, glGetnHistogramARB, glGetnHistogramARB, NULL, 794),
    NAME_FUNC_OFFSET(13107, glGetnMapdvARB, glGetnMapdvARB, NULL, 795),
    NAME_FUNC_OFFSET(13122, glGetnMapfvARB, glGetnMapfvARB, NULL, 796),
    NAME_FUNC_OFFSET(13137, glGetnMapivARB, glGetnMapivARB, NULL, 797),
    NAME_FUNC_OFFSET(13152, glGetnMinmaxARB, glGetnMinmaxARB, NULL, 798),
    NAME_FUNC_OFFSET(13168, glGetnPixelMapfvARB, glGetnPixelMapfvARB, NULL, 799),
    NAME_FUNC_OFFSET(13188, glGetnPixelMapuivARB, glGetnPixelMapuivARB, NULL, 800),
    NAME_FUNC_OFFSET(13209, glGetnPixelMapusvARB, glGetnPixelMapusvARB, NULL, 801),
    NAME_FUNC_OFFSET(13230, glGetnPolygonStippleARB, glGetnPolygonStippleARB, NULL, 802),
    NAME_FUNC_OFFSET(13254, glGetnSeparableFilterARB, glGetnSeparableFilterARB, NULL, 803),
    NAME_FUNC_OFFSET(13279, glGetnTexImageARB, glGetnTexImageARB, NULL, 804),
    NAME_FUNC_OFFSET(13297, glGetnUniformdvARB, glGetnUniformdvARB, NULL, 805),
    NAME_FUNC_OFFSET(13316, glGetnUniformfvARB, glGetnUniformfvARB, NULL, 806),
    NAME_FUNC_OFFSET(13335, glGetnUniformivARB, glGetnUniformivARB, NULL, 807),
    NAME_FUNC_OFFSET(13354, glGetnUniformuivARB, glGetnUniformuivARB, NULL, 808),
    NAME_FUNC_OFFSET(13374, glReadnPixelsARB, glReadnPixelsARB, NULL, 809),
    NAME_FUNC_OFFSET(13391, glDrawArraysInstancedBaseInstance, glDrawArraysInstancedBaseInstance, NULL, 810),
    NAME_FUNC_OFFSET(13425, glDrawElementsInstancedBaseInstance, glDrawElementsInstancedBaseInstance, NULL, 811),
    NAME_FUNC_OFFSET(13461, glDrawElementsInstancedBaseVertexBaseInstance, glDrawElementsInstancedBaseVertexBaseInstance, NULL, 812),
    NAME_FUNC_OFFSET(13507, glDrawTransformFeedbackInstanced, glDrawTransformFeedbackInstanced, NULL, 813),
    NAME_FUNC_OFFSET(13540, glDrawTransformFeedbackStreamInstanced, glDrawTransformFeedbackStreamInstanced, NULL, 814),
    NAME_FUNC_OFFSET(13579, gl_dispatch_stub_815, gl_dispatch_stub_815, NULL, 815),
    NAME_FUNC_OFFSET(13601, glTexStorage1D, glTexStorage1D, NULL, 816),
    NAME_FUNC_OFFSET(13616, glTexStorage2D, glTexStorage2D, NULL, 817),
    NAME_FUNC_OFFSET(13631, glTexStorage3D, glTexStorage3D, NULL, 818),
    NAME_FUNC_OFFSET(13646, glTextureStorage1DEXT, glTextureStorage1DEXT, NULL, 819),
    NAME_FUNC_OFFSET(13668, glTextureStorage2DEXT, glTextureStorage2DEXT, NULL, 820),
    NAME_FUNC_OFFSET(13690, glTextureStorage3DEXT, glTextureStorage3DEXT, NULL, 821),
    NAME_FUNC_OFFSET(13712, glInvalidateBufferData, glInvalidateBufferData, NULL, 822),
    NAME_FUNC_OFFSET(13735, glInvalidateBufferSubData, glInvalidateBufferSubData, NULL, 823),
    NAME_FUNC_OFFSET(13761, glInvalidateFramebuffer, glInvalidateFramebuffer, NULL, 824),
    NAME_FUNC_OFFSET(13785, glInvalidateSubFramebuffer, glInvalidateSubFramebuffer, NULL, 825),
    NAME_FUNC_OFFSET(13812, glInvalidateTexImage, glInvalidateTexImage, NULL, 826),
    NAME_FUNC_OFFSET(13833, glInvalidateTexSubImage, glInvalidateTexSubImage, NULL, 827),
    NAME_FUNC_OFFSET(13857, glPolygonOffsetEXT, glPolygonOffsetEXT, NULL, 828),
    NAME_FUNC_OFFSET(13876, gl_dispatch_stub_829, gl_dispatch_stub_829, NULL, 829),
    NAME_FUNC_OFFSET(13890, gl_dispatch_stub_830, gl_dispatch_stub_830, NULL, 830),
    NAME_FUNC_OFFSET(13905, gl_dispatch_stub_831, gl_dispatch_stub_831, NULL, 831),
    NAME_FUNC_OFFSET(13919, gl_dispatch_stub_832, gl_dispatch_stub_832, NULL, 832),
    NAME_FUNC_OFFSET(13934, gl_dispatch_stub_833, gl_dispatch_stub_833, NULL, 833),
    NAME_FUNC_OFFSET(13948, gl_dispatch_stub_834, gl_dispatch_stub_834, NULL, 834),
    NAME_FUNC_OFFSET(13963, gl_dispatch_stub_835, gl_dispatch_stub_835, NULL, 835),
    NAME_FUNC_OFFSET(13977, gl_dispatch_stub_836, gl_dispatch_stub_836, NULL, 836),
    NAME_FUNC_OFFSET(13992, gl_dispatch_stub_837, gl_dispatch_stub_837, NULL, 837),
    NAME_FUNC_OFFSET(14014, gl_dispatch_stub_838, gl_dispatch_stub_838, NULL, 838),
    NAME_FUNC_OFFSET(14032, gl_dispatch_stub_839, gl_dispatch_stub_839, NULL, 839),
    NAME_FUNC_OFFSET(14049, gl_dispatch_stub_840, gl_dispatch_stub_840, NULL, 840),
    NAME_FUNC_OFFSET(14069, glColorPointerEXT, glColorPointerEXT, NULL, 841),
    NAME_FUNC_OFFSET(14087, glEdgeFlagPointerEXT, glEdgeFlagPointerEXT, NULL, 842),
    NAME_FUNC_OFFSET(14108, glIndexPointerEXT, glIndexPointerEXT, NULL, 843),
    NAME_FUNC_OFFSET(14126, glNormalPointerEXT, glNormalPointerEXT, NULL, 844),
    NAME_FUNC_OFFSET(14145, glTexCoordPointerEXT, glTexCoordPointerEXT, NULL, 845),
    NAME_FUNC_OFFSET(14166, glVertexPointerEXT, glVertexPointerEXT, NULL, 846),
    NAME_FUNC_OFFSET(14185, glLockArraysEXT, glLockArraysEXT, NULL, 847),
    NAME_FUNC_OFFSET(14201, glUnlockArraysEXT, glUnlockArraysEXT, NULL, 848),
    NAME_FUNC_OFFSET(14219, glSecondaryColor3fEXT, glSecondaryColor3fEXT, NULL, 849),
    NAME_FUNC_OFFSET(14241, glSecondaryColor3fvEXT, glSecondaryColor3fvEXT, NULL, 850),
    NAME_FUNC_OFFSET(14264, glMultiDrawElementsEXT, glMultiDrawElementsEXT, NULL, 851),
    NAME_FUNC_OFFSET(14287, glFogCoordfEXT, glFogCoordfEXT, NULL, 852),
    NAME_FUNC_OFFSET(14302, glFogCoordfvEXT, glFogCoordfvEXT, NULL, 853),
    NAME_FUNC_OFFSET(14318, glResizeBuffersMESA, glResizeBuffersMESA, NULL, 854),
    NAME_FUNC_OFFSET(14338, glWindowPos4dMESA, glWindowPos4dMESA, NULL, 855),
    NAME_FUNC_OFFSET(14356, glWindowPos4dvMESA, glWindowPos4dvMESA, NULL, 856),
    NAME_FUNC_OFFSET(14375, glWindowPos4fMESA, glWindowPos4fMESA, NULL, 857),
    NAME_FUNC_OFFSET(14393, glWindowPos4fvMESA, glWindowPos4fvMESA, NULL, 858),
    NAME_FUNC_OFFSET(14412, glWindowPos4iMESA, glWindowPos4iMESA, NULL, 859),
    NAME_FUNC_OFFSET(14430, glWindowPos4ivMESA, glWindowPos4ivMESA, NULL, 860),
    NAME_FUNC_OFFSET(14449, glWindowPos4sMESA, glWindowPos4sMESA, NULL, 861),
    NAME_FUNC_OFFSET(14467, glWindowPos4svMESA, glWindowPos4svMESA, NULL, 862),
    NAME_FUNC_OFFSET(14486, gl_dispatch_stub_863, gl_dispatch_stub_863, NULL, 863),
    NAME_FUNC_OFFSET(14511, gl_dispatch_stub_864, gl_dispatch_stub_864, NULL, 864),
    NAME_FUNC_OFFSET(14538, glAreProgramsResidentNV, glAreProgramsResidentNV, NULL, 865),
    NAME_FUNC_OFFSET(14562, glExecuteProgramNV, glExecuteProgramNV, NULL, 866),
    NAME_FUNC_OFFSET(14581, glGetProgramParameterdvNV, glGetProgramParameterdvNV, NULL, 867),
    NAME_FUNC_OFFSET(14607, glGetProgramParameterfvNV, glGetProgramParameterfvNV, NULL, 868),
    NAME_FUNC_OFFSET(14633, glGetProgramStringNV, glGetProgramStringNV, NULL, 869),
    NAME_FUNC_OFFSET(14654, glGetProgramivNV, glGetProgramivNV, NULL, 870),
    NAME_FUNC_OFFSET(14671, glGetTrackMatrixivNV, glGetTrackMatrixivNV, NULL, 871),
    NAME_FUNC_OFFSET(14692, glGetVertexAttribdvNV, glGetVertexAttribdvNV, NULL, 872),
    NAME_FUNC_OFFSET(14714, glGetVertexAttribfvNV, glGetVertexAttribfvNV, NULL, 873),
    NAME_FUNC_OFFSET(14736, glGetVertexAttribivNV, glGetVertexAttribivNV, NULL, 874),
    NAME_FUNC_OFFSET(14758, glLoadProgramNV, glLoadProgramNV, NULL, 875),
    NAME_FUNC_OFFSET(14774, glProgramParameters4dvNV, glProgramParameters4dvNV, NULL, 876),
    NAME_FUNC_OFFSET(14799, glProgramParameters4fvNV, glProgramParameters4fvNV, NULL, 877),
    NAME_FUNC_OFFSET(14824, glRequestResidentProgramsNV, glRequestResidentProgramsNV, NULL, 878),
    NAME_FUNC_OFFSET(14852, glTrackMatrixNV, glTrackMatrixNV, NULL, 879),
    NAME_FUNC_OFFSET(14868, glVertexAttrib1dNV, glVertexAttrib1dNV, NULL, 880),
    NAME_FUNC_OFFSET(14887, glVertexAttrib1dvNV, glVertexAttrib1dvNV, NULL, 881),
    NAME_FUNC_OFFSET(14907, glVertexAttrib1fNV, glVertexAttrib1fNV, NULL, 882),
    NAME_FUNC_OFFSET(14926, glVertexAttrib1fvNV, glVertexAttrib1fvNV, NULL, 883),
    NAME_FUNC_OFFSET(14946, glVertexAttrib1sNV, glVertexAttrib1sNV, NULL, 884),
    NAME_FUNC_OFFSET(14965, glVertexAttrib1svNV, glVertexAttrib1svNV, NULL, 885),
    NAME_FUNC_OFFSET(14985, glVertexAttrib2dNV, glVertexAttrib2dNV, NULL, 886),
    NAME_FUNC_OFFSET(15004, glVertexAttrib2dvNV, glVertexAttrib2dvNV, NULL, 887),
    NAME_FUNC_OFFSET(15024, glVertexAttrib2fNV, glVertexAttrib2fNV, NULL, 888),
    NAME_FUNC_OFFSET(15043, glVertexAttrib2fvNV, glVertexAttrib2fvNV, NULL, 889),
    NAME_FUNC_OFFSET(15063, glVertexAttrib2sNV, glVertexAttrib2sNV, NULL, 890),
    NAME_FUNC_OFFSET(15082, glVertexAttrib2svNV, glVertexAttrib2svNV, NULL, 891),
    NAME_FUNC_OFFSET(15102, glVertexAttrib3dNV, glVertexAttrib3dNV, NULL, 892),
    NAME_FUNC_OFFSET(15121, glVertexAttrib3dvNV, glVertexAttrib3dvNV, NULL, 893),
    NAME_FUNC_OFFSET(15141, glVertexAttrib3fNV, glVertexAttrib3fNV, NULL, 894),
    NAME_FUNC_OFFSET(15160, glVertexAttrib3fvNV, glVertexAttrib3fvNV, NULL, 895),
    NAME_FUNC_OFFSET(15180, glVertexAttrib3sNV, glVertexAttrib3sNV, NULL, 896),
    NAME_FUNC_OFFSET(15199, glVertexAttrib3svNV, glVertexAttrib3svNV, NULL, 897),
    NAME_FUNC_OFFSET(15219, glVertexAttrib4dNV, glVertexAttrib4dNV, NULL, 898),
    NAME_FUNC_OFFSET(15238, glVertexAttrib4dvNV, glVertexAttrib4dvNV, NULL, 899),
    NAME_FUNC_OFFSET(15258, glVertexAttrib4fNV, glVertexAttrib4fNV, NULL, 900),
    NAME_FUNC_OFFSET(15277, glVertexAttrib4fvNV, glVertexAttrib4fvNV, NULL, 901),
    NAME_FUNC_OFFSET(15297, glVertexAttrib4sNV, glVertexAttrib4sNV, NULL, 902),
    NAME_FUNC_OFFSET(15316, glVertexAttrib4svNV, glVertexAttrib4svNV, NULL, 903),
    NAME_FUNC_OFFSET(15336, glVertexAttrib4ubNV, glVertexAttrib4ubNV, NULL, 904),
    NAME_FUNC_OFFSET(15356, glVertexAttrib4ubvNV, glVertexAttrib4ubvNV, NULL, 905),
    NAME_FUNC_OFFSET(15377, glVertexAttribPointerNV, glVertexAttribPointerNV, NULL, 906),
    NAME_FUNC_OFFSET(15401, glVertexAttribs1dvNV, glVertexAttribs1dvNV, NULL, 907),
    NAME_FUNC_OFFSET(15422, glVertexAttribs1fvNV, glVertexAttribs1fvNV, NULL, 908),
    NAME_FUNC_OFFSET(15443, glVertexAttribs1svNV, glVertexAttribs1svNV, NULL, 909),
    NAME_FUNC_OFFSET(15464, glVertexAttribs2dvNV, glVertexAttribs2dvNV, NULL, 910),
    NAME_FUNC_OFFSET(15485, glVertexAttribs2fvNV, glVertexAttribs2fvNV, NULL, 911),
    NAME_FUNC_OFFSET(15506, glVertexAttribs2svNV, glVertexAttribs2svNV, NULL, 912),
    NAME_FUNC_OFFSET(15527, glVertexAttribs3dvNV, glVertexAttribs3dvNV, NULL, 913),
    NAME_FUNC_OFFSET(15548, glVertexAttribs3fvNV, glVertexAttribs3fvNV, NULL, 914),
    NAME_FUNC_OFFSET(15569, glVertexAttribs3svNV, glVertexAttribs3svNV, NULL, 915),
    NAME_FUNC_OFFSET(15590, glVertexAttribs4dvNV, glVertexAttribs4dvNV, NULL, 916),
    NAME_FUNC_OFFSET(15611, glVertexAttribs4fvNV, glVertexAttribs4fvNV, NULL, 917),
    NAME_FUNC_OFFSET(15632, glVertexAttribs4svNV, glVertexAttribs4svNV, NULL, 918),
    NAME_FUNC_OFFSET(15653, glVertexAttribs4ubvNV, glVertexAttribs4ubvNV, NULL, 919),
    NAME_FUNC_OFFSET(15675, glGetTexBumpParameterfvATI, glGetTexBumpParameterfvATI, NULL, 920),
    NAME_FUNC_OFFSET(15702, glGetTexBumpParameterivATI, glGetTexBumpParameterivATI, NULL, 921),
    NAME_FUNC_OFFSET(15729, glTexBumpParameterfvATI, glTexBumpParameterfvATI, NULL, 922),
    NAME_FUNC_OFFSET(15753, glTexBumpParameterivATI, glTexBumpParameterivATI, NULL, 923),
    NAME_FUNC_OFFSET(15777, glAlphaFragmentOp1ATI, glAlphaFragmentOp1ATI, NULL, 924),
    NAME_FUNC_OFFSET(15799, glAlphaFragmentOp2ATI, glAlphaFragmentOp2ATI, NULL, 925),
    NAME_FUNC_OFFSET(15821, glAlphaFragmentOp3ATI, glAlphaFragmentOp3ATI, NULL, 926),
    NAME_FUNC_OFFSET(15843, glBeginFragmentShaderATI, glBeginFragmentShaderATI, NULL, 927),
    NAME_FUNC_OFFSET(15868, glBindFragmentShaderATI, glBindFragmentShaderATI, NULL, 928),
    NAME_FUNC_OFFSET(15892, glColorFragmentOp1ATI, glColorFragmentOp1ATI, NULL, 929),
    NAME_FUNC_OFFSET(15914, glColorFragmentOp2ATI, glColorFragmentOp2ATI, NULL, 930),
    NAME_FUNC_OFFSET(15936, glColorFragmentOp3ATI, glColorFragmentOp3ATI, NULL, 931),
    NAME_FUNC_OFFSET(15958, glDeleteFragmentShaderATI, glDeleteFragmentShaderATI, NULL, 932),
    NAME_FUNC_OFFSET(15984, glEndFragmentShaderATI, glEndFragmentShaderATI, NULL, 933),
    NAME_FUNC_OFFSET(16007, glGenFragmentShadersATI, glGenFragmentShadersATI, NULL, 934),
    NAME_FUNC_OFFSET(16031, glPassTexCoordATI, glPassTexCoordATI, NULL, 935),
    NAME_FUNC_OFFSET(16049, glSampleMapATI, glSampleMapATI, NULL, 936),
    NAME_FUNC_OFFSET(16064, glSetFragmentShaderConstantATI, glSetFragmentShaderConstantATI, NULL, 937),
    NAME_FUNC_OFFSET(16095, gl_dispatch_stub_938, gl_dispatch_stub_938, NULL, 938),
    NAME_FUNC_OFFSET(16118, gl_dispatch_stub_939, gl_dispatch_stub_939, NULL, 939),
    NAME_FUNC_OFFSET(16141, gl_dispatch_stub_940, gl_dispatch_stub_940, NULL, 940),
    NAME_FUNC_OFFSET(16164, glGetProgramNamedParameterdvNV, glGetProgramNamedParameterdvNV, NULL, 941),
    NAME_FUNC_OFFSET(16195, glGetProgramNamedParameterfvNV, glGetProgramNamedParameterfvNV, NULL, 942),
    NAME_FUNC_OFFSET(16226, glProgramNamedParameter4dNV, glProgramNamedParameter4dNV, NULL, 943),
    NAME_FUNC_OFFSET(16254, glProgramNamedParameter4dvNV, glProgramNamedParameter4dvNV, NULL, 944),
    NAME_FUNC_OFFSET(16283, glProgramNamedParameter4fNV, glProgramNamedParameter4fNV, NULL, 945),
    NAME_FUNC_OFFSET(16311, glProgramNamedParameter4fvNV, glProgramNamedParameter4fvNV, NULL, 946),
    NAME_FUNC_OFFSET(16340, glPrimitiveRestartNV, glPrimitiveRestartNV, NULL, 947),
    NAME_FUNC_OFFSET(16361, gl_dispatch_stub_948, gl_dispatch_stub_948, NULL, 948),
    NAME_FUNC_OFFSET(16378, gl_dispatch_stub_949, gl_dispatch_stub_949, NULL, 949),
    NAME_FUNC_OFFSET(16391, gl_dispatch_stub_950, gl_dispatch_stub_950, NULL, 950),
    NAME_FUNC_OFFSET(16405, gl_dispatch_stub_951, gl_dispatch_stub_951, NULL, 951),
    NAME_FUNC_OFFSET(16422, gl_dispatch_stub_952, gl_dispatch_stub_952, NULL, 952),
    NAME_FUNC_OFFSET(16446, gl_dispatch_stub_953, gl_dispatch_stub_953, NULL, 953),
    NAME_FUNC_OFFSET(16476, glVertexAttribI1iEXT, glVertexAttribI1iEXT, NULL, 954),
    NAME_FUNC_OFFSET(16497, glVertexAttribI1uiEXT, glVertexAttribI1uiEXT, NULL, 955),
    NAME_FUNC_OFFSET(16519, glVertexAttribI2iEXT, glVertexAttribI2iEXT, NULL, 956),
    NAME_FUNC_OFFSET(16540, glVertexAttribI2ivEXT, glVertexAttribI2ivEXT, NULL, 957),
    NAME_FUNC_OFFSET(16562, glVertexAttribI2uiEXT, glVertexAttribI2uiEXT, NULL, 958),
    NAME_FUNC_OFFSET(16584, glVertexAttribI2uivEXT, glVertexAttribI2uivEXT, NULL, 959),
    NAME_FUNC_OFFSET(16607, glVertexAttribI3iEXT, glVertexAttribI3iEXT, NULL, 960),
    NAME_FUNC_OFFSET(16628, glVertexAttribI3ivEXT, glVertexAttribI3ivEXT, NULL, 961),
    NAME_FUNC_OFFSET(16650, glVertexAttribI3uiEXT, glVertexAttribI3uiEXT, NULL, 962),
    NAME_FUNC_OFFSET(16672, glVertexAttribI3uivEXT, glVertexAttribI3uivEXT, NULL, 963),
    NAME_FUNC_OFFSET(16695, glVertexAttribI4iEXT, glVertexAttribI4iEXT, NULL, 964),
    NAME_FUNC_OFFSET(16716, glVertexAttribI4ivEXT, glVertexAttribI4ivEXT, NULL, 965),
    NAME_FUNC_OFFSET(16738, glVertexAttribI4uiEXT, glVertexAttribI4uiEXT, NULL, 966),
    NAME_FUNC_OFFSET(16760, glVertexAttribI4uivEXT, glVertexAttribI4uivEXT, NULL, 967),
    NAME_FUNC_OFFSET(16783, glClearColorIiEXT, glClearColorIiEXT, NULL, 968),
    NAME_FUNC_OFFSET(16801, glClearColorIuiEXT, glClearColorIuiEXT, NULL, 969),
    NAME_FUNC_OFFSET(16820, glBindBufferOffsetEXT, glBindBufferOffsetEXT, NULL, 970),
    NAME_FUNC_OFFSET(16842, glGetObjectParameterivAPPLE, glGetObjectParameterivAPPLE, NULL, 971),
    NAME_FUNC_OFFSET(16870, glObjectPurgeableAPPLE, glObjectPurgeableAPPLE, NULL, 972),
    NAME_FUNC_OFFSET(16893, glObjectUnpurgeableAPPLE, glObjectUnpurgeableAPPLE, NULL, 973),
    NAME_FUNC_OFFSET(16918, glActiveProgramEXT, glActiveProgramEXT, NULL, 974),
    NAME_FUNC_OFFSET(16937, glCreateShaderProgramEXT, glCreateShaderProgramEXT, NULL, 975),
    NAME_FUNC_OFFSET(16962, glUseShaderProgramEXT, glUseShaderProgramEXT, NULL, 976),
    NAME_FUNC_OFFSET(16984, glTextureBarrierNV, glTextureBarrierNV, NULL, 977),
    NAME_FUNC_OFFSET(17003, gl_dispatch_stub_978, gl_dispatch_stub_978, NULL, 978),
    NAME_FUNC_OFFSET(17028, gl_dispatch_stub_979, gl_dispatch_stub_979, NULL, 979),
    NAME_FUNC_OFFSET(17057, gl_dispatch_stub_980, gl_dispatch_stub_980, NULL, 980),
    NAME_FUNC_OFFSET(17088, glEGLImageTargetRenderbufferStorageOES, glEGLImageTargetRenderbufferStorageOES, NULL, 981),
    NAME_FUNC_OFFSET(17127, glEGLImageTargetTexture2DOES, glEGLImageTargetTexture2DOES, NULL, 982),
    NAME_FUNC_OFFSET(17156, gl_dispatch_stub_983, gl_dispatch_stub_983, NULL, 983),
    NAME_FUNC_OFFSET(17169, gl_dispatch_stub_984, gl_dispatch_stub_984, NULL, 984),
    NAME_FUNC_OFFSET(17183, gl_dispatch_stub_985, gl_dispatch_stub_985, NULL, 985),
    NAME_FUNC_OFFSET(17197, gl_dispatch_stub_986, gl_dispatch_stub_986, NULL, 986),
    NAME_FUNC_OFFSET(17207, gl_dispatch_stub_987, gl_dispatch_stub_987, NULL, 987),
    NAME_FUNC_OFFSET(17221, gl_dispatch_stub_988, gl_dispatch_stub_988, NULL, 988),
    NAME_FUNC_OFFSET(17228, gl_dispatch_stub_989, gl_dispatch_stub_989, NULL, 989),
    NAME_FUNC_OFFSET(17236, gl_dispatch_stub_990, gl_dispatch_stub_990, NULL, 990),
    NAME_FUNC_OFFSET(17247, gl_dispatch_stub_991, gl_dispatch_stub_991, NULL, 991),
    NAME_FUNC_OFFSET(17258, gl_dispatch_stub_992, gl_dispatch_stub_992, NULL, 992),
    NAME_FUNC_OFFSET(17272, gl_dispatch_stub_993, gl_dispatch_stub_993, NULL, 993),
    NAME_FUNC_OFFSET(17287, gl_dispatch_stub_994, gl_dispatch_stub_994, NULL, 994),
    NAME_FUNC_OFFSET(17296, gl_dispatch_stub_995, gl_dispatch_stub_995, NULL, 995),
    NAME_FUNC_OFFSET(17306, gl_dispatch_stub_996, gl_dispatch_stub_996, NULL, 996),
    NAME_FUNC_OFFSET(17319, gl_dispatch_stub_997, gl_dispatch_stub_997, NULL, 997),
    NAME_FUNC_OFFSET(17333, gl_dispatch_stub_998, gl_dispatch_stub_998, NULL, 998),
    NAME_FUNC_OFFSET(17345, gl_dispatch_stub_999, gl_dispatch_stub_999, NULL, 999),
    NAME_FUNC_OFFSET(17358, gl_dispatch_stub_1000, gl_dispatch_stub_1000, NULL, 1000),
    NAME_FUNC_OFFSET(17372, gl_dispatch_stub_1001, gl_dispatch_stub_1001, NULL, 1001),
    NAME_FUNC_OFFSET(17390, gl_dispatch_stub_1002, gl_dispatch_stub_1002, NULL, 1002),
    NAME_FUNC_OFFSET(17401, gl_dispatch_stub_1003, gl_dispatch_stub_1003, NULL, 1003),
    NAME_FUNC_OFFSET(17410, gl_dispatch_stub_1004, gl_dispatch_stub_1004, NULL, 1004),
    NAME_FUNC_OFFSET(17419, gl_dispatch_stub_1005, gl_dispatch_stub_1005, NULL, 1005),
    NAME_FUNC_OFFSET(17432, gl_dispatch_stub_1006, gl_dispatch_stub_1006, NULL, 1006),
    NAME_FUNC_OFFSET(17449, gl_dispatch_stub_1007, gl_dispatch_stub_1007, NULL, 1007),
    NAME_FUNC_OFFSET(17459, gl_dispatch_stub_1008, gl_dispatch_stub_1008, NULL, 1008),
    NAME_FUNC_OFFSET(17477, gl_dispatch_stub_1009, gl_dispatch_stub_1009, NULL, 1009),
    NAME_FUNC_OFFSET(17486, gl_dispatch_stub_1010, gl_dispatch_stub_1010, NULL, 1010),
    NAME_FUNC_OFFSET(17496, gl_dispatch_stub_1011, gl_dispatch_stub_1011, NULL, 1011),
    NAME_FUNC_OFFSET(17507, gl_dispatch_stub_1012, gl_dispatch_stub_1012, NULL, 1012),
    NAME_FUNC_OFFSET(17523, gl_dispatch_stub_1013, gl_dispatch_stub_1013, NULL, 1013),
    NAME_FUNC_OFFSET(17536, gl_dispatch_stub_1014, gl_dispatch_stub_1014, NULL, 1014),
    NAME_FUNC_OFFSET(17549, gl_dispatch_stub_1015, gl_dispatch_stub_1015, NULL, 1015),
    NAME_FUNC_OFFSET(17562, gl_dispatch_stub_1016, gl_dispatch_stub_1016, NULL, 1016),
    NAME_FUNC_OFFSET(17578, gl_dispatch_stub_1017, gl_dispatch_stub_1017, NULL, 1017),
    NAME_FUNC_OFFSET(17594, gl_dispatch_stub_1018, gl_dispatch_stub_1018, NULL, 1018),
    NAME_FUNC_OFFSET(17606, gl_dispatch_stub_1019, gl_dispatch_stub_1019, NULL, 1019),
    NAME_FUNC_OFFSET(17619, gl_dispatch_stub_1020, gl_dispatch_stub_1020, NULL, 1020),
    NAME_FUNC_OFFSET(17635, gl_dispatch_stub_1021, gl_dispatch_stub_1021, NULL, 1021),
    NAME_FUNC_OFFSET(17649, gl_dispatch_stub_1022, gl_dispatch_stub_1022, NULL, 1022),
    NAME_FUNC_OFFSET(17669, gl_dispatch_stub_1023, gl_dispatch_stub_1023, NULL, 1023),
    NAME_FUNC_OFFSET(17687, gl_dispatch_stub_1024, gl_dispatch_stub_1024, NULL, 1024),
    NAME_FUNC_OFFSET(17706, gl_dispatch_stub_1025, gl_dispatch_stub_1025, NULL, 1025),
    NAME_FUNC_OFFSET(17723, glTexGenf, glTexGenf, NULL, 190),
    NAME_FUNC_OFFSET(17736, glTexGenfv, glTexGenfv, NULL, 191),
    NAME_FUNC_OFFSET(17750, glTexGeni, glTexGeni, NULL, 192),
    NAME_FUNC_OFFSET(17763, glTexGeniv, glTexGeniv, NULL, 193),
    NAME_FUNC_OFFSET(17777, glReadBuffer, glReadBuffer, NULL, 254),
    NAME_FUNC_OFFSET(17792, glGetTexGenfv, glGetTexGenfv, NULL, 279),
    NAME_FUNC_OFFSET(17809, glGetTexGeniv, glGetTexGeniv, NULL, 280),
    NAME_FUNC_OFFSET(17826, glArrayElement, glArrayElement, NULL, 306),
    NAME_FUNC_OFFSET(17844, glBindTexture, glBindTexture, NULL, 307),
    NAME_FUNC_OFFSET(17861, glDrawArrays, glDrawArrays, NULL, 310),
    NAME_FUNC_OFFSET(17877, glAreTexturesResident, glAreTexturesResidentEXT, glAreTexturesResidentEXT, 322),
    NAME_FUNC_OFFSET(17902, glCopyTexImage1D, glCopyTexImage1D, NULL, 323),
    NAME_FUNC_OFFSET(17922, glCopyTexImage2D, glCopyTexImage2D, NULL, 324),
    NAME_FUNC_OFFSET(17942, glCopyTexSubImage1D, glCopyTexSubImage1D, NULL, 325),
    NAME_FUNC_OFFSET(17965, glCopyTexSubImage2D, glCopyTexSubImage2D, NULL, 326),
    NAME_FUNC_OFFSET(17988, glDeleteTextures, glDeleteTexturesEXT, glDeleteTexturesEXT, 327),
    NAME_FUNC_OFFSET(18008, glGenTextures, glGenTexturesEXT, glGenTexturesEXT, 328),
    NAME_FUNC_OFFSET(18025, glGetPointerv, glGetPointerv, NULL, 329),
    NAME_FUNC_OFFSET(18042, glIsTexture, glIsTextureEXT, glIsTextureEXT, 330),
    NAME_FUNC_OFFSET(18057, glPrioritizeTextures, glPrioritizeTextures, NULL, 331),
    NAME_FUNC_OFFSET(18081, glTexSubImage1D, glTexSubImage1D, NULL, 332),
    NAME_FUNC_OFFSET(18100, glTexSubImage2D, glTexSubImage2D, NULL, 333),
    NAME_FUNC_OFFSET(18119, glBlendColor, glBlendColor, NULL, 336),
    NAME_FUNC_OFFSET(18135, glBlendEquation, glBlendEquation, NULL, 337),
    NAME_FUNC_OFFSET(18154, glBlendEquation, glBlendEquation, NULL, 337),
    NAME_FUNC_OFFSET(18173, glDrawRangeElements, glDrawRangeElements, NULL, 338),
    NAME_FUNC_OFFSET(18196, glColorTable, glColorTable, NULL, 339),
    NAME_FUNC_OFFSET(18212, glColorTable, glColorTable, NULL, 339),
    NAME_FUNC_OFFSET(18228, glColorTableParameterfv, glColorTableParameterfv, NULL, 340),
    NAME_FUNC_OFFSET(18255, glColorTableParameteriv, glColorTableParameteriv, NULL, 341),
    NAME_FUNC_OFFSET(18282, glCopyColorTable, glCopyColorTable, NULL, 342),
    NAME_FUNC_OFFSET(18302, glGetColorTable, glGetColorTableEXT, glGetColorTableEXT, 343),
    NAME_FUNC_OFFSET(18321, glGetColorTable, glGetColorTableEXT, glGetColorTableEXT, 343),
    NAME_FUNC_OFFSET(18340, glGetColorTableParameterfv, glGetColorTableParameterfvEXT, glGetColorTableParameterfvEXT, 344),
    NAME_FUNC_OFFSET(18370, glGetColorTableParameterfv, glGetColorTableParameterfvEXT, glGetColorTableParameterfvEXT, 344),
    NAME_FUNC_OFFSET(18400, glGetColorTableParameteriv, glGetColorTableParameterivEXT, glGetColorTableParameterivEXT, 345),
    NAME_FUNC_OFFSET(18430, glGetColorTableParameteriv, glGetColorTableParameterivEXT, glGetColorTableParameterivEXT, 345),
    NAME_FUNC_OFFSET(18460, glColorSubTable, glColorSubTable, NULL, 346),
    NAME_FUNC_OFFSET(18479, glCopyColorSubTable, glCopyColorSubTable, NULL, 347),
    NAME_FUNC_OFFSET(18502, glConvolutionFilter1D, glConvolutionFilter1D, NULL, 348),
    NAME_FUNC_OFFSET(18527, glConvolutionFilter2D, glConvolutionFilter2D, NULL, 349),
    NAME_FUNC_OFFSET(18552, glConvolutionParameterf, glConvolutionParameterf, NULL, 350),
    NAME_FUNC_OFFSET(18579, glConvolutionParameterfv, glConvolutionParameterfv, NULL, 351),
    NAME_FUNC_OFFSET(18607, glConvolutionParameteri, glConvolutionParameteri, NULL, 352),
    NAME_FUNC_OFFSET(18634, glConvolutionParameteriv, glConvolutionParameteriv, NULL, 353),
    NAME_FUNC_OFFSET(18662, glCopyConvolutionFilter1D, glCopyConvolutionFilter1D, NULL, 354),
    NAME_FUNC_OFFSET(18691, glCopyConvolutionFilter2D, glCopyConvolutionFilter2D, NULL, 355),
    NAME_FUNC_OFFSET(18720, glGetConvolutionFilter, gl_dispatch_stub_356, gl_dispatch_stub_356, 356),
    NAME_FUNC_OFFSET(18746, glGetConvolutionParameterfv, gl_dispatch_stub_357, gl_dispatch_stub_357, 357),
    NAME_FUNC_OFFSET(18777, glGetConvolutionParameteriv, gl_dispatch_stub_358, gl_dispatch_stub_358, 358),
    NAME_FUNC_OFFSET(18808, glGetSeparableFilter, gl_dispatch_stub_359, gl_dispatch_stub_359, 359),
    NAME_FUNC_OFFSET(18832, glSeparableFilter2D, glSeparableFilter2D, NULL, 360),
    NAME_FUNC_OFFSET(18855, glGetHistogram, gl_dispatch_stub_361, gl_dispatch_stub_361, 361),
    NAME_FUNC_OFFSET(18873, glGetHistogramParameterfv, gl_dispatch_stub_362, gl_dispatch_stub_362, 362),
    NAME_FUNC_OFFSET(18902, glGetHistogramParameteriv, gl_dispatch_stub_363, gl_dispatch_stub_363, 363),
    NAME_FUNC_OFFSET(18931, glGetMinmax, gl_dispatch_stub_364, gl_dispatch_stub_364, 364),
    NAME_FUNC_OFFSET(18946, glGetMinmaxParameterfv, gl_dispatch_stub_365, gl_dispatch_stub_365, 365),
    NAME_FUNC_OFFSET(18972, glGetMinmaxParameteriv, gl_dispatch_stub_366, gl_dispatch_stub_366, 366),
    NAME_FUNC_OFFSET(18998, glHistogram, glHistogram, NULL, 367),
    NAME_FUNC_OFFSET(19013, glMinmax, glMinmax, NULL, 368),
    NAME_FUNC_OFFSET(19025, glResetHistogram, glResetHistogram, NULL, 369),
    NAME_FUNC_OFFSET(19045, glResetMinmax, glResetMinmax, NULL, 370),
    NAME_FUNC_OFFSET(19062, glTexImage3D, glTexImage3D, NULL, 371),
    NAME_FUNC_OFFSET(19078, glTexImage3D, glTexImage3D, NULL, 371),
    NAME_FUNC_OFFSET(19094, glTexSubImage3D, glTexSubImage3D, NULL, 372),
    NAME_FUNC_OFFSET(19113, glTexSubImage3D, glTexSubImage3D, NULL, 372),
    NAME_FUNC_OFFSET(19132, glCopyTexSubImage3D, glCopyTexSubImage3D, NULL, 373),
    NAME_FUNC_OFFSET(19155, glCopyTexSubImage3D, glCopyTexSubImage3D, NULL, 373),
    NAME_FUNC_OFFSET(19178, glActiveTexture, glActiveTexture, NULL, 374),
    NAME_FUNC_OFFSET(19197, glClientActiveTexture, glClientActiveTexture, NULL, 375),
    NAME_FUNC_OFFSET(19222, glMultiTexCoord1d, glMultiTexCoord1d, NULL, 376),
    NAME_FUNC_OFFSET(19243, glMultiTexCoord1dv, glMultiTexCoord1dv, NULL, 377),
    NAME_FUNC_OFFSET(19265, glMultiTexCoord1fARB, glMultiTexCoord1fARB, NULL, 378),
    NAME_FUNC_OFFSET(19283, glMultiTexCoord1fvARB, glMultiTexCoord1fvARB, NULL, 379),
    NAME_FUNC_OFFSET(19302, glMultiTexCoord1i, glMultiTexCoord1i, NULL, 380),
    NAME_FUNC_OFFSET(19323, glMultiTexCoord1iv, glMultiTexCoord1iv, NULL, 381),
    NAME_FUNC_OFFSET(19345, glMultiTexCoord1s, glMultiTexCoord1s, NULL, 382),
    NAME_FUNC_OFFSET(19366, glMultiTexCoord1sv, glMultiTexCoord1sv, NULL, 383),
    NAME_FUNC_OFFSET(19388, glMultiTexCoord2d, glMultiTexCoord2d, NULL, 384),
    NAME_FUNC_OFFSET(19409, glMultiTexCoord2dv, glMultiTexCoord2dv, NULL, 385),
    NAME_FUNC_OFFSET(19431, glMultiTexCoord2fARB, glMultiTexCoord2fARB, NULL, 386),
    NAME_FUNC_OFFSET(19449, glMultiTexCoord2fvARB, glMultiTexCoord2fvARB, NULL, 387),
    NAME_FUNC_OFFSET(19468, glMultiTexCoord2i, glMultiTexCoord2i, NULL, 388),
    NAME_FUNC_OFFSET(19489, glMultiTexCoord2iv, glMultiTexCoord2iv, NULL, 389),
    NAME_FUNC_OFFSET(19511, glMultiTexCoord2s, glMultiTexCoord2s, NULL, 390),
    NAME_FUNC_OFFSET(19532, glMultiTexCoord2sv, glMultiTexCoord2sv, NULL, 391),
    NAME_FUNC_OFFSET(19554, glMultiTexCoord3d, glMultiTexCoord3d, NULL, 392),
    NAME_FUNC_OFFSET(19575, glMultiTexCoord3dv, glMultiTexCoord3dv, NULL, 393),
    NAME_FUNC_OFFSET(19597, glMultiTexCoord3fARB, glMultiTexCoord3fARB, NULL, 394),
    NAME_FUNC_OFFSET(19615, glMultiTexCoord3fvARB, glMultiTexCoord3fvARB, NULL, 395),
    NAME_FUNC_OFFSET(19634, glMultiTexCoord3i, glMultiTexCoord3i, NULL, 396),
    NAME_FUNC_OFFSET(19655, glMultiTexCoord3iv, glMultiTexCoord3iv, NULL, 397),
    NAME_FUNC_OFFSET(19677, glMultiTexCoord3s, glMultiTexCoord3s, NULL, 398),
    NAME_FUNC_OFFSET(19698, glMultiTexCoord3sv, glMultiTexCoord3sv, NULL, 399),
    NAME_FUNC_OFFSET(19720, glMultiTexCoord4d, glMultiTexCoord4d, NULL, 400),
    NAME_FUNC_OFFSET(19741, glMultiTexCoord4dv, glMultiTexCoord4dv, NULL, 401),
    NAME_FUNC_OFFSET(19763, glMultiTexCoord4fARB, glMultiTexCoord4fARB, NULL, 402),
    NAME_FUNC_OFFSET(19781, glMultiTexCoord4fvARB, glMultiTexCoord4fvARB, NULL, 403),
    NAME_FUNC_OFFSET(19800, glMultiTexCoord4i, glMultiTexCoord4i, NULL, 404),
    NAME_FUNC_OFFSET(19821, glMultiTexCoord4iv, glMultiTexCoord4iv, NULL, 405),
    NAME_FUNC_OFFSET(19843, glMultiTexCoord4s, glMultiTexCoord4s, NULL, 406),
    NAME_FUNC_OFFSET(19864, glMultiTexCoord4sv, glMultiTexCoord4sv, NULL, 407),
    NAME_FUNC_OFFSET(19886, glCompressedTexImage1D, glCompressedTexImage1D, NULL, 408),
    NAME_FUNC_OFFSET(19912, glCompressedTexImage2D, glCompressedTexImage2D, NULL, 409),
    NAME_FUNC_OFFSET(19938, glCompressedTexImage3D, glCompressedTexImage3D, NULL, 410),
    NAME_FUNC_OFFSET(19964, glCompressedTexImage3D, glCompressedTexImage3D, NULL, 410),
    NAME_FUNC_OFFSET(19990, glCompressedTexSubImage1D, glCompressedTexSubImage1D, NULL, 411),
    NAME_FUNC_OFFSET(20019, glCompressedTexSubImage2D, glCompressedTexSubImage2D, NULL, 412),
    NAME_FUNC_OFFSET(20048, glCompressedTexSubImage3D, glCompressedTexSubImage3D, NULL, 413),
    NAME_FUNC_OFFSET(20077, glCompressedTexSubImage3D, glCompressedTexSubImage3D, NULL, 413),
    NAME_FUNC_OFFSET(20106, glGetCompressedTexImage, glGetCompressedTexImage, NULL, 414),
    NAME_FUNC_OFFSET(20133, glLoadTransposeMatrixd, glLoadTransposeMatrixd, NULL, 415),
    NAME_FUNC_OFFSET(20159, glLoadTransposeMatrixf, glLoadTransposeMatrixf, NULL, 416),
    NAME_FUNC_OFFSET(20185, glMultTransposeMatrixd, glMultTransposeMatrixd, NULL, 417),
    NAME_FUNC_OFFSET(20211, glMultTransposeMatrixf, glMultTransposeMatrixf, NULL, 418),
    NAME_FUNC_OFFSET(20237, glSampleCoverage, glSampleCoverage, NULL, 419),
    NAME_FUNC_OFFSET(20257, glBlendFuncSeparate, glBlendFuncSeparate, NULL, 420),
    NAME_FUNC_OFFSET(20280, glBlendFuncSeparate, glBlendFuncSeparate, NULL, 420),
    NAME_FUNC_OFFSET(20304, glBlendFuncSeparate, glBlendFuncSeparate, NULL, 420),
    NAME_FUNC_OFFSET(20327, glFogCoordPointer, glFogCoordPointer, NULL, 421),
    NAME_FUNC_OFFSET(20348, glFogCoordd, glFogCoordd, NULL, 422),
    NAME_FUNC_OFFSET(20363, glFogCoorddv, glFogCoorddv, NULL, 423),
    NAME_FUNC_OFFSET(20379, glMultiDrawArrays, glMultiDrawArrays, NULL, 424),
    NAME_FUNC_OFFSET(20400, glPointParameterf, glPointParameterf, NULL, 425),
    NAME_FUNC_OFFSET(20421, glPointParameterf, glPointParameterf, NULL, 425),
    NAME_FUNC_OFFSET(20442, glPointParameterf, glPointParameterf, NULL, 425),
    NAME_FUNC_OFFSET(20464, glPointParameterfv, glPointParameterfv, NULL, 426),
    NAME_FUNC_OFFSET(20486, glPointParameterfv, glPointParameterfv, NULL, 426),
    NAME_FUNC_OFFSET(20508, glPointParameterfv, glPointParameterfv, NULL, 426),
    NAME_FUNC_OFFSET(20531, glPointParameteri, glPointParameteri, NULL, 427),
    NAME_FUNC_OFFSET(20551, glPointParameteriv, glPointParameteriv, NULL, 428),
    NAME_FUNC_OFFSET(20572, glSecondaryColor3b, glSecondaryColor3b, NULL, 429),
    NAME_FUNC_OFFSET(20594, glSecondaryColor3bv, glSecondaryColor3bv, NULL, 430),
    NAME_FUNC_OFFSET(20617, glSecondaryColor3d, glSecondaryColor3d, NULL, 431),
    NAME_FUNC_OFFSET(20639, glSecondaryColor3dv, glSecondaryColor3dv, NULL, 432),
    NAME_FUNC_OFFSET(20662, glSecondaryColor3i, glSecondaryColor3i, NULL, 433),
    NAME_FUNC_OFFSET(20684, glSecondaryColor3iv, glSecondaryColor3iv, NULL, 434),
    NAME_FUNC_OFFSET(20707, glSecondaryColor3s, glSecondaryColor3s, NULL, 435),
    NAME_FUNC_OFFSET(20729, glSecondaryColor3sv, glSecondaryColor3sv, NULL, 436),
    NAME_FUNC_OFFSET(20752, glSecondaryColor3ub, glSecondaryColor3ub, NULL, 437),
    NAME_FUNC_OFFSET(20775, glSecondaryColor3ubv, glSecondaryColor3ubv, NULL, 438),
    NAME_FUNC_OFFSET(20799, glSecondaryColor3ui, glSecondaryColor3ui, NULL, 439),
    NAME_FUNC_OFFSET(20822, glSecondaryColor3uiv, glSecondaryColor3uiv, NULL, 440),
    NAME_FUNC_OFFSET(20846, glSecondaryColor3us, glSecondaryColor3us, NULL, 441),
    NAME_FUNC_OFFSET(20869, glSecondaryColor3usv, glSecondaryColor3usv, NULL, 442),
    NAME_FUNC_OFFSET(20893, glSecondaryColorPointer, glSecondaryColorPointer, NULL, 443),
    NAME_FUNC_OFFSET(20920, glWindowPos2d, glWindowPos2d, NULL, 444),
    NAME_FUNC_OFFSET(20937, glWindowPos2d, glWindowPos2d, NULL, 444),
    NAME_FUNC_OFFSET(20955, glWindowPos2dv, glWindowPos2dv, NULL, 445),
    NAME_FUNC_OFFSET(20973, glWindowPos2dv, glWindowPos2dv, NULL, 445),
    NAME_FUNC_OFFSET(20992, glWindowPos2f, glWindowPos2f, NULL, 446),
    NAME_FUNC_OFFSET(21009, glWindowPos2f, glWindowPos2f, NULL, 446),
    NAME_FUNC_OFFSET(21027, glWindowPos2fv, glWindowPos2fv, NULL, 447),
    NAME_FUNC_OFFSET(21045, glWindowPos2fv, glWindowPos2fv, NULL, 447),
    NAME_FUNC_OFFSET(21064, glWindowPos2i, glWindowPos2i, NULL, 448),
    NAME_FUNC_OFFSET(21081, glWindowPos2i, glWindowPos2i, NULL, 448),
    NAME_FUNC_OFFSET(21099, glWindowPos2iv, glWindowPos2iv, NULL, 449),
    NAME_FUNC_OFFSET(21117, glWindowPos2iv, glWindowPos2iv, NULL, 449),
    NAME_FUNC_OFFSET(21136, glWindowPos2s, glWindowPos2s, NULL, 450),
    NAME_FUNC_OFFSET(21153, glWindowPos2s, glWindowPos2s, NULL, 450),
    NAME_FUNC_OFFSET(21171, glWindowPos2sv, glWindowPos2sv, NULL, 451),
    NAME_FUNC_OFFSET(21189, glWindowPos2sv, glWindowPos2sv, NULL, 451),
    NAME_FUNC_OFFSET(21208, glWindowPos3d, glWindowPos3d, NULL, 452),
    NAME_FUNC_OFFSET(21225, glWindowPos3d, glWindowPos3d, NULL, 452),
    NAME_FUNC_OFFSET(21243, glWindowPos3dv, glWindowPos3dv, NULL, 453),
    NAME_FUNC_OFFSET(21261, glWindowPos3dv, glWindowPos3dv, NULL, 453),
    NAME_FUNC_OFFSET(21280, glWindowPos3f, glWindowPos3f, NULL, 454),
    NAME_FUNC_OFFSET(21297, glWindowPos3f, glWindowPos3f, NULL, 454),
    NAME_FUNC_OFFSET(21315, glWindowPos3fv, glWindowPos3fv, NULL, 455),
    NAME_FUNC_OFFSET(21333, glWindowPos3fv, glWindowPos3fv, NULL, 455),
    NAME_FUNC_OFFSET(21352, glWindowPos3i, glWindowPos3i, NULL, 456),
    NAME_FUNC_OFFSET(21369, glWindowPos3i, glWindowPos3i, NULL, 456),
    NAME_FUNC_OFFSET(21387, glWindowPos3iv, glWindowPos3iv, NULL, 457),
    NAME_FUNC_OFFSET(21405, glWindowPos3iv, glWindowPos3iv, NULL, 457),
    NAME_FUNC_OFFSET(21424, glWindowPos3s, glWindowPos3s, NULL, 458),
    NAME_FUNC_OFFSET(21441, glWindowPos3s, glWindowPos3s, NULL, 458),
    NAME_FUNC_OFFSET(21459, glWindowPos3sv, glWindowPos3sv, NULL, 459),
    NAME_FUNC_OFFSET(21477, glWindowPos3sv, glWindowPos3sv, NULL, 459),
    NAME_FUNC_OFFSET(21496, glBeginQuery, glBeginQuery, NULL, 460),
    NAME_FUNC_OFFSET(21512, glBindBuffer, glBindBuffer, NULL, 461),
    NAME_FUNC_OFFSET(21528, glBufferData, glBufferData, NULL, 462),
    NAME_FUNC_OFFSET(21544, glBufferSubData, glBufferSubData, NULL, 463),
    NAME_FUNC_OFFSET(21563, glDeleteBuffers, glDeleteBuffers, NULL, 464),
    NAME_FUNC_OFFSET(21582, glDeleteQueries, glDeleteQueries, NULL, 465),
    NAME_FUNC_OFFSET(21601, glEndQuery, glEndQuery, NULL, 466),
    NAME_FUNC_OFFSET(21615, glGenBuffers, glGenBuffers, NULL, 467),
    NAME_FUNC_OFFSET(21631, glGenQueries, glGenQueries, NULL, 468),
    NAME_FUNC_OFFSET(21647, glGetBufferParameteriv, glGetBufferParameteriv, NULL, 469),
    NAME_FUNC_OFFSET(21673, glGetBufferPointerv, glGetBufferPointerv, NULL, 470),
    NAME_FUNC_OFFSET(21696, glGetBufferPointerv, glGetBufferPointerv, NULL, 470),
    NAME_FUNC_OFFSET(21719, glGetBufferSubData, glGetBufferSubData, NULL, 471),
    NAME_FUNC_OFFSET(21741, glGetQueryObjectiv, glGetQueryObjectiv, NULL, 472),
    NAME_FUNC_OFFSET(21763, glGetQueryObjectuiv, glGetQueryObjectuiv, NULL, 473),
    NAME_FUNC_OFFSET(21786, glGetQueryiv, glGetQueryiv, NULL, 474),
    NAME_FUNC_OFFSET(21802, glIsBuffer, glIsBuffer, NULL, 475),
    NAME_FUNC_OFFSET(21816, glIsQuery, glIsQuery, NULL, 476),
    NAME_FUNC_OFFSET(21829, glMapBuffer, glMapBuffer, NULL, 477),
    NAME_FUNC_OFFSET(21844, glMapBuffer, glMapBuffer, NULL, 477),
    NAME_FUNC_OFFSET(21859, glUnmapBuffer, glUnmapBuffer, NULL, 478),
    NAME_FUNC_OFFSET(21876, glUnmapBuffer, glUnmapBuffer, NULL, 478),
    NAME_FUNC_OFFSET(21893, glBindAttribLocation, glBindAttribLocation, NULL, 480),
    NAME_FUNC_OFFSET(21917, glBlendEquationSeparate, glBlendEquationSeparate, NULL, 481),
    NAME_FUNC_OFFSET(21944, glBlendEquationSeparate, glBlendEquationSeparate, NULL, 481),
    NAME_FUNC_OFFSET(21971, glBlendEquationSeparate, glBlendEquationSeparate, NULL, 481),
    NAME_FUNC_OFFSET(21998, glCompileShader, glCompileShader, NULL, 482),
    NAME_FUNC_OFFSET(22017, glDisableVertexAttribArray, glDisableVertexAttribArray, NULL, 488),
    NAME_FUNC_OFFSET(22047, glDrawBuffers, glDrawBuffers, NULL, 489),
    NAME_FUNC_OFFSET(22064, glDrawBuffers, glDrawBuffers, NULL, 489),
    NAME_FUNC_OFFSET(22081, glDrawBuffers, glDrawBuffers, NULL, 489),
    NAME_FUNC_OFFSET(22097, glEnableVertexAttribArray, glEnableVertexAttribArray, NULL, 490),
    NAME_FUNC_OFFSET(22126, glGetActiveAttrib, glGetActiveAttrib, NULL, 491),
    NAME_FUNC_OFFSET(22147, glGetActiveUniform, glGetActiveUniform, NULL, 492),
    NAME_FUNC_OFFSET(22169, glGetAttribLocation, glGetAttribLocation, NULL, 494),
    NAME_FUNC_OFFSET(22192, glGetShaderSource, glGetShaderSource, NULL, 498),
    NAME_FUNC_OFFSET(22213, glGetUniformLocation, glGetUniformLocation, NULL, 500),
    NAME_FUNC_OFFSET(22237, glGetUniformfv, glGetUniformfv, NULL, 501),
    NAME_FUNC_OFFSET(22255, glGetUniformiv, glGetUniformiv, NULL, 502),
    NAME_FUNC_OFFSET(22273, glGetVertexAttribPointerv, glGetVertexAttribPointerv, NULL, 503),
    NAME_FUNC_OFFSET(22302, glGetVertexAttribPointerv, glGetVertexAttribPointerv, NULL, 503),
    NAME_FUNC_OFFSET(22330, glGetVertexAttribdv, glGetVertexAttribdv, NULL, 504),
    NAME_FUNC_OFFSET(22353, glGetVertexAttribfv, glGetVertexAttribfv, NULL, 505),
    NAME_FUNC_OFFSET(22376, glGetVertexAttribiv, glGetVertexAttribiv, NULL, 506),
    NAME_FUNC_OFFSET(22399, glLinkProgram, glLinkProgram, NULL, 509),
    NAME_FUNC_OFFSET(22416, glShaderSource, glShaderSource, NULL, 510),
    NAME_FUNC_OFFSET(22434, glStencilOpSeparate, glStencilOpSeparate, NULL, 513),
    NAME_FUNC_OFFSET(22457, glUniform1f, glUniform1f, NULL, 514),
    NAME_FUNC_OFFSET(22472, glUniform1fv, glUniform1fv, NULL, 515),
    NAME_FUNC_OFFSET(22488, glUniform1i, glUniform1i, NULL, 516),
    NAME_FUNC_OFFSET(22503, glUniform1iv, glUniform1iv, NULL, 517),
    NAME_FUNC_OFFSET(22519, glUniform2f, glUniform2f, NULL, 518),
    NAME_FUNC_OFFSET(22534, glUniform2fv, glUniform2fv, NULL, 519),
    NAME_FUNC_OFFSET(22550, glUniform2i, glUniform2i, NULL, 520),
    NAME_FUNC_OFFSET(22565, glUniform2iv, glUniform2iv, NULL, 521),
    NAME_FUNC_OFFSET(22581, glUniform3f, glUniform3f, NULL, 522),
    NAME_FUNC_OFFSET(22596, glUniform3fv, glUniform3fv, NULL, 523),
    NAME_FUNC_OFFSET(22612, glUniform3i, glUniform3i, NULL, 524),
    NAME_FUNC_OFFSET(22627, glUniform3iv, glUniform3iv, NULL, 525),
    NAME_FUNC_OFFSET(22643, glUniform4f, glUniform4f, NULL, 526),
    NAME_FUNC_OFFSET(22658, glUniform4fv, glUniform4fv, NULL, 527),
    NAME_FUNC_OFFSET(22674, glUniform4i, glUniform4i, NULL, 528),
    NAME_FUNC_OFFSET(22689, glUniform4iv, glUniform4iv, NULL, 529),
    NAME_FUNC_OFFSET(22705, glUniformMatrix2fv, glUniformMatrix2fv, NULL, 530),
    NAME_FUNC_OFFSET(22727, glUniformMatrix3fv, glUniformMatrix3fv, NULL, 531),
    NAME_FUNC_OFFSET(22749, glUniformMatrix4fv, glUniformMatrix4fv, NULL, 532),
    NAME_FUNC_OFFSET(22771, glUseProgram, glUseProgram, NULL, 533),
    NAME_FUNC_OFFSET(22793, glValidateProgram, glValidateProgram, NULL, 534),
    NAME_FUNC_OFFSET(22814, glVertexAttrib1d, glVertexAttrib1d, NULL, 535),
    NAME_FUNC_OFFSET(22834, glVertexAttrib1dv, glVertexAttrib1dv, NULL, 536),
    NAME_FUNC_OFFSET(22855, glVertexAttrib1s, glVertexAttrib1s, NULL, 537),
    NAME_FUNC_OFFSET(22875, glVertexAttrib1sv, glVertexAttrib1sv, NULL, 538),
    NAME_FUNC_OFFSET(22896, glVertexAttrib2d, glVertexAttrib2d, NULL, 539),
    NAME_FUNC_OFFSET(22916, glVertexAttrib2dv, glVertexAttrib2dv, NULL, 540),
    NAME_FUNC_OFFSET(22937, glVertexAttrib2s, glVertexAttrib2s, NULL, 541),
    NAME_FUNC_OFFSET(22957, glVertexAttrib2sv, glVertexAttrib2sv, NULL, 542),
    NAME_FUNC_OFFSET(22978, glVertexAttrib3d, glVertexAttrib3d, NULL, 543),
    NAME_FUNC_OFFSET(22998, glVertexAttrib3dv, glVertexAttrib3dv, NULL, 544),
    NAME_FUNC_OFFSET(23019, glVertexAttrib3s, glVertexAttrib3s, NULL, 545),
    NAME_FUNC_OFFSET(23039, glVertexAttrib3sv, glVertexAttrib3sv, NULL, 546),
    NAME_FUNC_OFFSET(23060, glVertexAttrib4Nbv, glVertexAttrib4Nbv, NULL, 547),
    NAME_FUNC_OFFSET(23082, glVertexAttrib4Niv, glVertexAttrib4Niv, NULL, 548),
    NAME_FUNC_OFFSET(23104, glVertexAttrib4Nsv, glVertexAttrib4Nsv, NULL, 549),
    NAME_FUNC_OFFSET(23126, glVertexAttrib4Nub, glVertexAttrib4Nub, NULL, 550),
    NAME_FUNC_OFFSET(23148, glVertexAttrib4Nubv, glVertexAttrib4Nubv, NULL, 551),
    NAME_FUNC_OFFSET(23171, glVertexAttrib4Nuiv, glVertexAttrib4Nuiv, NULL, 552),
    NAME_FUNC_OFFSET(23194, glVertexAttrib4Nusv, glVertexAttrib4Nusv, NULL, 553),
    NAME_FUNC_OFFSET(23217, glVertexAttrib4bv, glVertexAttrib4bv, NULL, 554),
    NAME_FUNC_OFFSET(23238, glVertexAttrib4d, glVertexAttrib4d, NULL, 555),
    NAME_FUNC_OFFSET(23258, glVertexAttrib4dv, glVertexAttrib4dv, NULL, 556),
    NAME_FUNC_OFFSET(23279, glVertexAttrib4iv, glVertexAttrib4iv, NULL, 557),
    NAME_FUNC_OFFSET(23300, glVertexAttrib4s, glVertexAttrib4s, NULL, 558),
    NAME_FUNC_OFFSET(23320, glVertexAttrib4sv, glVertexAttrib4sv, NULL, 559),
    NAME_FUNC_OFFSET(23341, glVertexAttrib4ubv, glVertexAttrib4ubv, NULL, 560),
    NAME_FUNC_OFFSET(23363, glVertexAttrib4uiv, glVertexAttrib4uiv, NULL, 561),
    NAME_FUNC_OFFSET(23385, glVertexAttrib4usv, glVertexAttrib4usv, NULL, 562),
    NAME_FUNC_OFFSET(23407, glVertexAttribPointer, glVertexAttribPointer, NULL, 563),
    NAME_FUNC_OFFSET(23432, glBeginConditionalRender, glBeginConditionalRender, NULL, 570),
    NAME_FUNC_OFFSET(23459, glBeginTransformFeedback, glBeginTransformFeedback, NULL, 571),
    NAME_FUNC_OFFSET(23487, glBindBufferBase, glBindBufferBase, NULL, 572),
    NAME_FUNC_OFFSET(23507, glBindBufferRange, glBindBufferRange, NULL, 573),
    NAME_FUNC_OFFSET(23528, glBindFragDataLocation, glBindFragDataLocation, NULL, 574),
    NAME_FUNC_OFFSET(23554, glClampColor, glClampColor, NULL, 575),
    NAME_FUNC_OFFSET(23570, glColorMaski, glColorMaski, NULL, 580),
    NAME_FUNC_OFFSET(23592, glDisablei, glDisablei, NULL, 581),
    NAME_FUNC_OFFSET(23612, glEnablei, glEnablei, NULL, 582),
    NAME_FUNC_OFFSET(23631, glEndConditionalRender, glEndConditionalRender, NULL, 583),
    NAME_FUNC_OFFSET(23656, glEndTransformFeedback, glEndTransformFeedback, NULL, 584),
    NAME_FUNC_OFFSET(23682, glGetBooleani_v, glGetBooleani_v, NULL, 585),
    NAME_FUNC_OFFSET(23706, glGetFragDataLocation, glGetFragDataLocation, NULL, 586),
    NAME_FUNC_OFFSET(23731, glGetIntegeri_v, glGetIntegeri_v, NULL, 587),
    NAME_FUNC_OFFSET(23755, glGetTexParameterIiv, glGetTexParameterIiv, NULL, 589),
    NAME_FUNC_OFFSET(23779, glGetTexParameterIuiv, glGetTexParameterIuiv, NULL, 590),
    NAME_FUNC_OFFSET(23804, glGetTransformFeedbackVarying, glGetTransformFeedbackVarying, NULL, 591),
    NAME_FUNC_OFFSET(23837, glGetUniformuiv, glGetUniformuiv, NULL, 592),
    NAME_FUNC_OFFSET(23856, glGetVertexAttribIiv, glGetVertexAttribIiv, NULL, 593),
    NAME_FUNC_OFFSET(23880, glGetVertexAttribIuiv, glGetVertexAttribIuiv, NULL, 594),
    NAME_FUNC_OFFSET(23905, glIsEnabledi, glIsEnabledi, NULL, 595),
    NAME_FUNC_OFFSET(23927, glTexParameterIiv, glTexParameterIiv, NULL, 596),
    NAME_FUNC_OFFSET(23948, glTexParameterIuiv, glTexParameterIuiv, NULL, 597),
    NAME_FUNC_OFFSET(23970, glTransformFeedbackVaryings, glTransformFeedbackVaryings, NULL, 598),
    NAME_FUNC_OFFSET(24001, glUniform1ui, glUniform1ui, NULL, 599),
    NAME_FUNC_OFFSET(24017, glUniform1uiv, glUniform1uiv, NULL, 600),
    NAME_FUNC_OFFSET(24034, glUniform2ui, glUniform2ui, NULL, 601),
    NAME_FUNC_OFFSET(24050, glUniform2uiv, glUniform2uiv, NULL, 602),
    NAME_FUNC_OFFSET(24067, glUniform3ui, glUniform3ui, NULL, 603),
    NAME_FUNC_OFFSET(24083, glUniform3uiv, glUniform3uiv, NULL, 604),
    NAME_FUNC_OFFSET(24100, glUniform4ui, glUniform4ui, NULL, 605),
    NAME_FUNC_OFFSET(24116, glUniform4uiv, glUniform4uiv, NULL, 606),
    NAME_FUNC_OFFSET(24133, glVertexAttribI1iv, glVertexAttribI1iv, NULL, 607),
    NAME_FUNC_OFFSET(24155, glVertexAttribI1uiv, glVertexAttribI1uiv, NULL, 608),
    NAME_FUNC_OFFSET(24178, glVertexAttribI4bv, glVertexAttribI4bv, NULL, 609),
    NAME_FUNC_OFFSET(24200, glVertexAttribI4sv, glVertexAttribI4sv, NULL, 610),
    NAME_FUNC_OFFSET(24222, glVertexAttribI4ubv, glVertexAttribI4ubv, NULL, 611),
    NAME_FUNC_OFFSET(24245, glVertexAttribI4usv, glVertexAttribI4usv, NULL, 612),
    NAME_FUNC_OFFSET(24268, glVertexAttribIPointer, glVertexAttribIPointer, NULL, 613),
    NAME_FUNC_OFFSET(24294, glPrimitiveRestartIndex, glPrimitiveRestartIndex, NULL, 614),
    NAME_FUNC_OFFSET(24320, glTexBuffer, glTexBuffer, NULL, 615),
    NAME_FUNC_OFFSET(24335, glFramebufferTexture, glFramebufferTexture, NULL, 616),
    NAME_FUNC_OFFSET(24359, glVertexAttribDivisor, glVertexAttribDivisor, NULL, 619),
    NAME_FUNC_OFFSET(24384, glBindProgramARB, glBindProgramARB, NULL, 620),
    NAME_FUNC_OFFSET(24400, glDeleteProgramsARB, glDeleteProgramsARB, NULL, 621),
    NAME_FUNC_OFFSET(24419, glGenProgramsARB, glGenProgramsARB, NULL, 622),
    NAME_FUNC_OFFSET(24435, glIsProgramARB, glIsProgramARB, NULL, 629),
    NAME_FUNC_OFFSET(24449, glProgramEnvParameter4dARB, glProgramEnvParameter4dARB, NULL, 630),
    NAME_FUNC_OFFSET(24472, glProgramEnvParameter4dvARB, glProgramEnvParameter4dvARB, NULL, 631),
    NAME_FUNC_OFFSET(24496, glProgramEnvParameter4fARB, glProgramEnvParameter4fARB, NULL, 632),
    NAME_FUNC_OFFSET(24519, glProgramEnvParameter4fvARB, glProgramEnvParameter4fvARB, NULL, 633),
    NAME_FUNC_OFFSET(24543, glVertexAttrib1fARB, glVertexAttrib1fARB, NULL, 639),
    NAME_FUNC_OFFSET(24560, glVertexAttrib1fvARB, glVertexAttrib1fvARB, NULL, 640),
    NAME_FUNC_OFFSET(24578, glVertexAttrib2fARB, glVertexAttrib2fARB, NULL, 641),
    NAME_FUNC_OFFSET(24595, glVertexAttrib2fvARB, glVertexAttrib2fvARB, NULL, 642),
    NAME_FUNC_OFFSET(24613, glVertexAttrib3fARB, glVertexAttrib3fARB, NULL, 643),
    NAME_FUNC_OFFSET(24630, glVertexAttrib3fvARB, glVertexAttrib3fvARB, NULL, 644),
    NAME_FUNC_OFFSET(24648, glVertexAttrib4fARB, glVertexAttrib4fARB, NULL, 645),
    NAME_FUNC_OFFSET(24665, glVertexAttrib4fvARB, glVertexAttrib4fvARB, NULL, 646),
    NAME_FUNC_OFFSET(24683, glDrawArraysInstancedARB, glDrawArraysInstancedARB, NULL, 657),
    NAME_FUNC_OFFSET(24708, glDrawArraysInstancedARB, glDrawArraysInstancedARB, NULL, 657),
    NAME_FUNC_OFFSET(24730, glDrawElementsInstancedARB, glDrawElementsInstancedARB, NULL, 658),
    NAME_FUNC_OFFSET(24757, glDrawElementsInstancedARB, glDrawElementsInstancedARB, NULL, 658),
    NAME_FUNC_OFFSET(24781, glBindFramebuffer, glBindFramebuffer, NULL, 659),
    NAME_FUNC_OFFSET(24802, glBindFramebuffer, glBindFramebuffer, NULL, 659),
    NAME_FUNC_OFFSET(24823, glBindRenderbuffer, glBindRenderbuffer, NULL, 660),
    NAME_FUNC_OFFSET(24845, glBindRenderbuffer, glBindRenderbuffer, NULL, 660),
    NAME_FUNC_OFFSET(24867, glBlitFramebuffer, glBlitFramebuffer, NULL, 661),
    NAME_FUNC_OFFSET(24888, glCheckFramebufferStatus, glCheckFramebufferStatus, NULL, 662),
    NAME_FUNC_OFFSET(24916, glCheckFramebufferStatus, glCheckFramebufferStatus, NULL, 662),
    NAME_FUNC_OFFSET(24944, glDeleteFramebuffers, glDeleteFramebuffers, NULL, 663),
    NAME_FUNC_OFFSET(24968, glDeleteFramebuffers, glDeleteFramebuffers, NULL, 663),
    NAME_FUNC_OFFSET(24992, glDeleteRenderbuffers, glDeleteRenderbuffers, NULL, 664),
    NAME_FUNC_OFFSET(25017, glDeleteRenderbuffers, glDeleteRenderbuffers, NULL, 664),
    NAME_FUNC_OFFSET(25042, glFramebufferRenderbuffer, glFramebufferRenderbuffer, NULL, 665),
    NAME_FUNC_OFFSET(25071, glFramebufferRenderbuffer, glFramebufferRenderbuffer, NULL, 665),
    NAME_FUNC_OFFSET(25100, glFramebufferTexture1D, glFramebufferTexture1D, NULL, 666),
    NAME_FUNC_OFFSET(25126, glFramebufferTexture2D, glFramebufferTexture2D, NULL, 667),
    NAME_FUNC_OFFSET(25152, glFramebufferTexture2D, glFramebufferTexture2D, NULL, 667),
    NAME_FUNC_OFFSET(25178, glFramebufferTexture3D, glFramebufferTexture3D, NULL, 668),
    NAME_FUNC_OFFSET(25204, glFramebufferTexture3D, glFramebufferTexture3D, NULL, 668),
    NAME_FUNC_OFFSET(25230, glFramebufferTextureLayer, glFramebufferTextureLayer, NULL, 669),
    NAME_FUNC_OFFSET(25259, glFramebufferTextureLayer, glFramebufferTextureLayer, NULL, 669),
    NAME_FUNC_OFFSET(25288, glGenFramebuffers, glGenFramebuffers, NULL, 670),
    NAME_FUNC_OFFSET(25309, glGenFramebuffers, glGenFramebuffers, NULL, 670),
    NAME_FUNC_OFFSET(25330, glGenRenderbuffers, glGenRenderbuffers, NULL, 671),
    NAME_FUNC_OFFSET(25352, glGenRenderbuffers, glGenRenderbuffers, NULL, 671),
    NAME_FUNC_OFFSET(25374, glGenerateMipmap, glGenerateMipmap, NULL, 672),
    NAME_FUNC_OFFSET(25394, glGenerateMipmap, glGenerateMipmap, NULL, 672),
    NAME_FUNC_OFFSET(25414, glGetFramebufferAttachmentParameteriv, glGetFramebufferAttachmentParameteriv, NULL, 673),
    NAME_FUNC_OFFSET(25455, glGetFramebufferAttachmentParameteriv, glGetFramebufferAttachmentParameteriv, NULL, 673),
    NAME_FUNC_OFFSET(25496, glGetRenderbufferParameteriv, glGetRenderbufferParameteriv, NULL, 674),
    NAME_FUNC_OFFSET(25528, glGetRenderbufferParameteriv, glGetRenderbufferParameteriv, NULL, 674),
    NAME_FUNC_OFFSET(25560, glIsFramebuffer, glIsFramebuffer, NULL, 675),
    NAME_FUNC_OFFSET(25579, glIsFramebuffer, glIsFramebuffer, NULL, 675),
    NAME_FUNC_OFFSET(25598, glIsRenderbuffer, glIsRenderbuffer, NULL, 676),
    NAME_FUNC_OFFSET(25618, glIsRenderbuffer, glIsRenderbuffer, NULL, 676),
    NAME_FUNC_OFFSET(25638, glRenderbufferStorage, glRenderbufferStorage, NULL, 677),
    NAME_FUNC_OFFSET(25663, glRenderbufferStorage, glRenderbufferStorage, NULL, 677),
    NAME_FUNC_OFFSET(25688, glRenderbufferStorageMultisample, glRenderbufferStorageMultisample, NULL, 678),
    NAME_FUNC_OFFSET(25724, glFlushMappedBufferRange, glFlushMappedBufferRange, NULL, 680),
    NAME_FUNC_OFFSET(25752, glMapBufferRange, glMapBufferRange, NULL, 681),
    NAME_FUNC_OFFSET(25772, glBindVertexArray, glBindVertexArray, NULL, 682),
    NAME_FUNC_OFFSET(25793, glDeleteVertexArrays, glDeleteVertexArrays, NULL, 683),
    NAME_FUNC_OFFSET(25819, glDeleteVertexArrays, glDeleteVertexArrays, NULL, 683),
    NAME_FUNC_OFFSET(25843, glGenVertexArrays, glGenVertexArrays, NULL, 684),
    NAME_FUNC_OFFSET(25864, glIsVertexArray, glIsVertexArray, NULL, 685),
    NAME_FUNC_OFFSET(25885, glIsVertexArray, glIsVertexArray, NULL, 685),
    NAME_FUNC_OFFSET(25904, glProvokingVertex, glProvokingVertex, NULL, 705),
    NAME_FUNC_OFFSET(25925, glBlendEquationSeparateiARB, glBlendEquationSeparateiARB, NULL, 706),
    NAME_FUNC_OFFSET(25959, glBlendEquationiARB, glBlendEquationiARB, NULL, 707),
    NAME_FUNC_OFFSET(25985, glBlendFuncSeparateiARB, glBlendFuncSeparateiARB, NULL, 708),
    NAME_FUNC_OFFSET(26015, glBlendFunciARB, glBlendFunciARB, NULL, 709),
    NAME_FUNC_OFFSET(26037, gl_dispatch_stub_726, gl_dispatch_stub_726, NULL, 726),
    NAME_FUNC_OFFSET(26061, gl_dispatch_stub_727, gl_dispatch_stub_727, NULL, 727),
    NAME_FUNC_OFFSET(26086, glClearDepthf, glClearDepthf, NULL, 778),
    NAME_FUNC_OFFSET(26103, glDepthRangef, glDepthRangef, NULL, 779),
    NAME_FUNC_OFFSET(26120, glGetProgramBinary, glGetProgramBinary, NULL, 783),
    NAME_FUNC_OFFSET(26142, glProgramBinary, glProgramBinary, NULL, 784),
    NAME_FUNC_OFFSET(26161, glProgramParameteri, glProgramParameteri, NULL, 785),
    NAME_FUNC_OFFSET(26184, gl_dispatch_stub_839, gl_dispatch_stub_839, NULL, 839),
    NAME_FUNC_OFFSET(26200, gl_dispatch_stub_840, gl_dispatch_stub_840, NULL, 840),
    NAME_FUNC_OFFSET(26219, glSecondaryColor3fEXT, glSecondaryColor3fEXT, NULL, 849),
    NAME_FUNC_OFFSET(26238, glSecondaryColor3fvEXT, glSecondaryColor3fvEXT, NULL, 850),
    NAME_FUNC_OFFSET(26258, glMultiDrawElementsEXT, glMultiDrawElementsEXT, NULL, 851),
    NAME_FUNC_OFFSET(26278, glFogCoordfEXT, glFogCoordfEXT, NULL, 852),
    NAME_FUNC_OFFSET(26290, glFogCoordfvEXT, glFogCoordfvEXT, NULL, 853),
    NAME_FUNC_OFFSET(26303, glVertexAttribI1iEXT, glVertexAttribI1iEXT, NULL, 954),
    NAME_FUNC_OFFSET(26321, glVertexAttribI1uiEXT, glVertexAttribI1uiEXT, NULL, 955),
    NAME_FUNC_OFFSET(26340, glVertexAttribI2iEXT, glVertexAttribI2iEXT, NULL, 956),
    NAME_FUNC_OFFSET(26358, glVertexAttribI2ivEXT, glVertexAttribI2ivEXT, NULL, 957),
    NAME_FUNC_OFFSET(26377, glVertexAttribI2uiEXT, glVertexAttribI2uiEXT, NULL, 958),
    NAME_FUNC_OFFSET(26396, glVertexAttribI2uivEXT, glVertexAttribI2uivEXT, NULL, 959),
    NAME_FUNC_OFFSET(26416, glVertexAttribI3iEXT, glVertexAttribI3iEXT, NULL, 960),
    NAME_FUNC_OFFSET(26434, glVertexAttribI3ivEXT, glVertexAttribI3ivEXT, NULL, 961),
    NAME_FUNC_OFFSET(26453, glVertexAttribI3uiEXT, glVertexAttribI3uiEXT, NULL, 962),
    NAME_FUNC_OFFSET(26472, glVertexAttribI3uivEXT, glVertexAttribI3uivEXT, NULL, 963),
    NAME_FUNC_OFFSET(26492, glVertexAttribI4iEXT, glVertexAttribI4iEXT, NULL, 964),
    NAME_FUNC_OFFSET(26510, glVertexAttribI4ivEXT, glVertexAttribI4ivEXT, NULL, 965),
    NAME_FUNC_OFFSET(26529, glVertexAttribI4uiEXT, glVertexAttribI4uiEXT, NULL, 966),
    NAME_FUNC_OFFSET(26548, glVertexAttribI4uivEXT, glVertexAttribI4uivEXT, NULL, 967),
    NAME_FUNC_OFFSET(26568, gl_dispatch_stub_983, gl_dispatch_stub_983, NULL, 983),
    NAME_FUNC_OFFSET(26584, gl_dispatch_stub_984, gl_dispatch_stub_984, NULL, 984),
    NAME_FUNC_OFFSET(26601, gl_dispatch_stub_985, gl_dispatch_stub_985, NULL, 985),
    NAME_FUNC_OFFSET(26618, gl_dispatch_stub_986, gl_dispatch_stub_986, NULL, 986),
    NAME_FUNC_OFFSET(26631, gl_dispatch_stub_987, gl_dispatch_stub_987, NULL, 987),
    NAME_FUNC_OFFSET(26648, gl_dispatch_stub_988, gl_dispatch_stub_988, NULL, 988),
    NAME_FUNC_OFFSET(26658, gl_dispatch_stub_989, gl_dispatch_stub_989, NULL, 989),
    NAME_FUNC_OFFSET(26669, gl_dispatch_stub_990, gl_dispatch_stub_990, NULL, 990),
    NAME_FUNC_OFFSET(26683, gl_dispatch_stub_991, gl_dispatch_stub_991, NULL, 991),
    NAME_FUNC_OFFSET(26697, gl_dispatch_stub_992, gl_dispatch_stub_992, NULL, 992),
    NAME_FUNC_OFFSET(26714, gl_dispatch_stub_993, gl_dispatch_stub_993, NULL, 993),
    NAME_FUNC_OFFSET(26732, gl_dispatch_stub_994, gl_dispatch_stub_994, NULL, 994),
    NAME_FUNC_OFFSET(26744, gl_dispatch_stub_995, gl_dispatch_stub_995, NULL, 995),
    NAME_FUNC_OFFSET(26757, gl_dispatch_stub_996, gl_dispatch_stub_996, NULL, 996),
    NAME_FUNC_OFFSET(26773, gl_dispatch_stub_997, gl_dispatch_stub_997, NULL, 997),
    NAME_FUNC_OFFSET(26790, gl_dispatch_stub_998, gl_dispatch_stub_998, NULL, 998),
    NAME_FUNC_OFFSET(26805, gl_dispatch_stub_999, gl_dispatch_stub_999, NULL, 999),
    NAME_FUNC_OFFSET(26821, gl_dispatch_stub_1000, gl_dispatch_stub_1000, NULL, 1000),
    NAME_FUNC_OFFSET(26838, gl_dispatch_stub_1001, gl_dispatch_stub_1001, NULL, 1001),
    NAME_FUNC_OFFSET(26859, gl_dispatch_stub_1002, gl_dispatch_stub_1002, NULL, 1002),
    NAME_FUNC_OFFSET(26873, gl_dispatch_stub_1003, gl_dispatch_stub_1003, NULL, 1003),
    NAME_FUNC_OFFSET(26885, gl_dispatch_stub_1004, gl_dispatch_stub_1004, NULL, 1004),
    NAME_FUNC_OFFSET(26897, gl_dispatch_stub_1005, gl_dispatch_stub_1005, NULL, 1005),
    NAME_FUNC_OFFSET(26913, gl_dispatch_stub_1006, gl_dispatch_stub_1006, NULL, 1006),
    NAME_FUNC_OFFSET(26933, gl_dispatch_stub_1007, gl_dispatch_stub_1007, NULL, 1007),
    NAME_FUNC_OFFSET(26946, gl_dispatch_stub_1008, gl_dispatch_stub_1008, NULL, 1008),
    NAME_FUNC_OFFSET(26967, gl_dispatch_stub_1009, gl_dispatch_stub_1009, NULL, 1009),
    NAME_FUNC_OFFSET(26979, gl_dispatch_stub_1010, gl_dispatch_stub_1010, NULL, 1010),
    NAME_FUNC_OFFSET(26992, gl_dispatch_stub_1011, gl_dispatch_stub_1011, NULL, 1011),
    NAME_FUNC_OFFSET(27006, gl_dispatch_stub_1012, gl_dispatch_stub_1012, NULL, 1012),
    NAME_FUNC_OFFSET(27025, gl_dispatch_stub_1013, gl_dispatch_stub_1013, NULL, 1013),
    NAME_FUNC_OFFSET(27041, gl_dispatch_stub_1014, gl_dispatch_stub_1014, NULL, 1014),
    NAME_FUNC_OFFSET(27057, gl_dispatch_stub_1015, gl_dispatch_stub_1015, NULL, 1015),
    NAME_FUNC_OFFSET(27073, gl_dispatch_stub_1016, gl_dispatch_stub_1016, NULL, 1016),
    NAME_FUNC_OFFSET(27092, gl_dispatch_stub_1017, gl_dispatch_stub_1017, NULL, 1017),
    NAME_FUNC_OFFSET(27111, gl_dispatch_stub_1018, gl_dispatch_stub_1018, NULL, 1018),
    NAME_FUNC_OFFSET(27126, gl_dispatch_stub_1019, gl_dispatch_stub_1019, NULL, 1019),
    NAME_FUNC_OFFSET(27142, gl_dispatch_stub_1020, gl_dispatch_stub_1020, NULL, 1020),
    NAME_FUNC_OFFSET(27161, gl_dispatch_stub_1021, gl_dispatch_stub_1021, NULL, 1021),
    NAME_FUNC_OFFSET(27178, gl_dispatch_stub_1022, gl_dispatch_stub_1022, NULL, 1022),
    NAME_FUNC_OFFSET(27201, gl_dispatch_stub_1023, gl_dispatch_stub_1023, NULL, 1023),
    NAME_FUNC_OFFSET(27222, gl_dispatch_stub_1024, gl_dispatch_stub_1024, NULL, 1024),
    NAME_FUNC_OFFSET(27244, gl_dispatch_stub_1025, gl_dispatch_stub_1025, NULL, 1025),
    NAME_FUNC_OFFSET(-1, NULL, NULL, NULL, 0)
};

#undef NAME_FUNC_OFFSET
