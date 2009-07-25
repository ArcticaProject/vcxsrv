/*
 * GLX implementation that uses Win32's OpenGL
 * Wrapper functions for Win32's OpenGL
 *
 * Authors: Alexander Gottwald
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xwindows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glxserver.h>
#include <glxext.h>

#define RESOLVE_RET(procname, symbol, retval) \
    static Bool init = TRUE; \
    static procname proc = NULL; \
    if (init) { \
        proc = (procname)wglGetProcAddress(symbol); \
        init = FALSE; \
        if (proc == NULL) { \
            ErrorF("glwrap: Can't resolve \"%s\"\n", symbol); \
        } else \
            ErrorF("glwrap: resolved \"%s\"\n", symbol); \
    } \
    if (proc == NULL) { \
        __glXErrorCallBack(NULL, 0); \
        return retval; \
    }
#define RESOLVE(procname, symbol) RESOLVE_RET(procname, symbol,)
        
        
/*
 * GL_ARB_imaging
 */


GLAPI void GLAPIENTRY glColorTable( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table )
{
    RESOLVE(PFNGLCOLORTABLEPROC, "glColorTable");
    proc(target, internalformat, width, format, type, table);
}

GLAPI void GLAPIENTRY glColorSubTable( GLenum target,
                                       GLsizei start, GLsizei count,
                                       GLenum format, GLenum type,
                                       const GLvoid *data )
{
    RESOLVE(PFNGLCOLORSUBTABLEPROC, "glColorSubTable");
    proc(target, start, count, format, type, data);
}

GLAPI void GLAPIENTRY glColorTableParameteriv(GLenum target, GLenum pname,
                                              const GLint *params)
{
    RESOLVE(PFNGLCOLORTABLEPARAMETERIVPROC, "glColorTableParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glColorTableParameterfv(GLenum target, GLenum pname,
                                              const GLfloat *params)
{
    RESOLVE(PFNGLCOLORTABLEPARAMETERFVPROC, "glColorTableParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glCopyColorSubTable( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width )
{
    RESOLVE(PFNGLCOPYCOLORSUBTABLEPROC, "glCopyColorSubTable");
    proc(target, start, x, y, width);
}

GLAPI void GLAPIENTRY glCopyColorTable( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width )
{
    RESOLVE(PFNGLCOPYCOLORTABLEPROC, "glCopyColorTable");
    proc(target, internalformat, x, y, width);
}


GLAPI void GLAPIENTRY glGetColorTable( GLenum target, GLenum format,
                                       GLenum type, GLvoid *table )
{
    RESOLVE(PFNGLGETCOLORTABLEPROC, "glGetColorTable");
    proc(target, format, type, table);
}

GLAPI void GLAPIENTRY glGetColorTableParameterfv( GLenum target, GLenum pname,
                                                  GLfloat *params )
{
    RESOLVE(PFNGLGETCOLORTABLEPARAMETERFVPROC, "glGetColorTableParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glGetColorTableParameteriv( GLenum target, GLenum pname,
                                                  GLint *params )
{
    RESOLVE(PFNGLGETCOLORTABLEPARAMETERIVPROC, "glGetColorTableParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glBlendEquation( GLenum mode )
{
    RESOLVE(PFNGLBLENDEQUATIONPROC, "glBlendEquation");
    proc(mode);
}

GLAPI void GLAPIENTRY glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha )
{
    RESOLVE(PFNGLBLENDCOLORPROC, "glBlendColor");
    proc(red, green, blue, alpha);
}

GLAPI void GLAPIENTRY glHistogram( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink )
{
    RESOLVE(PFNGLHISTOGRAMPROC, "glHistogram");
    proc(target, width, internalformat, sink);
}

GLAPI void GLAPIENTRY glResetHistogram( GLenum target )
{
    RESOLVE(PFNGLRESETHISTOGRAMPROC, "glResetHistogram");
    proc(target);
}

GLAPI void GLAPIENTRY glGetHistogram( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values )
{
    RESOLVE(PFNGLGETHISTOGRAMPROC, "glGetHistogram");
    proc(target, reset, format, type, values);
};

GLAPI void GLAPIENTRY glGetHistogramParameterfv( GLenum target, GLenum pname,
						 GLfloat *params )
{
    RESOLVE(PFNGLGETHISTOGRAMPARAMETERFVPROC, "glGetHistogramParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glGetHistogramParameteriv( GLenum target, GLenum pname,
						 GLint *params )
{
    RESOLVE(PFNGLGETHISTOGRAMPARAMETERIVPROC, "glGetHistogramParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glMinmax( GLenum target, GLenum internalformat,
				GLboolean sink )
{
    RESOLVE(PFNGLMINMAXPROC, "glMinmax");
    proc(target, internalformat, sink);
}

GLAPI void GLAPIENTRY glResetMinmax( GLenum target )
{
    RESOLVE(PFNGLRESETMINMAXPROC, "glResetMinmax");
    proc(target);
}

GLAPI void GLAPIENTRY glGetMinmax( GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values )
{
    RESOLVE(PFNGLGETMINMAXPROC, "glGetMinmax");
    proc(target, reset, format, types, values);
}

GLAPI void GLAPIENTRY glGetMinmaxParameterfv( GLenum target, GLenum pname,
					      GLfloat *params )
{
    RESOLVE(PFNGLGETMINMAXPARAMETERFVPROC, "glGetMinmaxParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glGetMinmaxParameteriv( GLenum target, GLenum pname,
					      GLint *params )
{
    RESOLVE(PFNGLGETMINMAXPARAMETERIVPROC, "glGetMinmaxParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image )
{
    RESOLVE(PFNGLCONVOLUTIONFILTER1DPROC, "glConvolutionFilter1D");
    proc(target, internalformat, width, format, type, image);
}

GLAPI void GLAPIENTRY glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image )
{
    RESOLVE(PFNGLCONVOLUTIONFILTER2DPROC, "glConvolutionFilter2D");
    proc(target, internalformat, width, height, format, type, image);
}

GLAPI void GLAPIENTRY glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params )
{
    RESOLVE(PFNGLCONVOLUTIONPARAMETERFPROC, "glConvolutionParameterf");
    proc(target, pname, params); 
}

GLAPI void GLAPIENTRY glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params )
{
    RESOLVE(PFNGLCONVOLUTIONPARAMETERFVPROC, "glConvolutionParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params )
{
    RESOLVE(PFNGLCONVOLUTIONPARAMETERIPROC, "glConvolutionParameteri");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params )
{
    RESOLVE(PFNGLCONVOLUTIONPARAMETERIVPROC, "glConvolutionParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glCopyConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width )
{
    RESOLVE(PFNGLCOPYCONVOLUTIONFILTER1DPROC, "glCopyConvolutionFilter1D");
    proc(target, internalformat, x, y, width);
}

GLAPI void GLAPIENTRY glCopyConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height)
{
    RESOLVE(PFNGLCOPYCONVOLUTIONFILTER2DPROC, "glCopyConvolutionFilter2D");
    proc(target, internalformat, x, y, width, height);
}

GLAPI void GLAPIENTRY glGetConvolutionFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *image )
{
    RESOLVE(PFNGLGETCONVOLUTIONFILTERPROC, "glGetConvolutionFilter");
    proc(target, format, type, image);
}

GLAPI void GLAPIENTRY glGetConvolutionParameterfv( GLenum target, GLenum pname,
	GLfloat *params )
{
    RESOLVE(PFNGLGETCONVOLUTIONPARAMETERFVPROC, "glGetConvolutionParameterfv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glGetConvolutionParameteriv( GLenum target, GLenum pname,
	GLint *params )
{
    RESOLVE(PFNGLGETCONVOLUTIONPARAMETERIVPROC, "glGetConvolutionParameteriv");
    proc(target, pname, params);
}

GLAPI void GLAPIENTRY glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column )
{
    RESOLVE(PFNGLSEPARABLEFILTER2DPROC, "glSeparableFilter2D");
    proc(target, internalformat, width, height, format, type, row, column);
}

GLAPI void GLAPIENTRY glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span )
{
    RESOLVE(PFNGLGETSEPARABLEFILTERPROC, "glGetSeparableFilter");
    proc(target, format, type, row, column, span);
}

/*
 * OpenGL 1.2
 */

GLAPI void GLAPIENTRY glTexImage3D( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels )
{
    RESOLVE(PFNGLTEXIMAGE3DPROC, "glTexImage3D");
    proc(target, level, internalFormat, width, height, depth, border, format, type, pixels);
}

GLAPI void GLAPIENTRY glTexSubImage3D( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels)
{
    RESOLVE(PFNGLTEXSUBIMAGE3DPROC, "glTexSubImage3D");
    proc(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void GLAPIENTRY glCopyTexSubImage3D( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height )
{
    RESOLVE(PFNGLCOPYTEXSUBIMAGE3DPROC, "glCopyTexSubImage3D");
    proc(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}


/*
 * 20. GL_EXT_texture_object
 */
GLAPI void GLAPIENTRY glGenTexturesEXT( GLsizei n, GLuint *textures )
{
    glGenTextures(n, textures);
}

GLAPI void GLAPIENTRY glDeleteTexturesEXT( GLsizei n, const GLuint *textures)
{
    glDeleteTextures(n, textures);
}

GLAPI void GLAPIENTRY glBindTextureEXT( GLenum target, GLuint texture )
{
    glBindTexture(target, target);
}

GLAPI void GLAPIENTRY glPrioritizeTexturesEXT( GLsizei n, const GLuint *textures, const GLclampf *priorities )
{
    glPrioritizeTextures(n, textures, priorities);
}

GLAPI GLboolean GLAPIENTRY glAreTexturesResidentEXT( GLsizei n, const GLuint *textures, GLboolean *residences )
{
    return glAreTexturesResident(n, textures, residences);
}

GLAPI GLboolean GLAPIENTRY glIsTextureEXT( GLuint texture )
{
    return glIsTexture(texture); 
}

/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */

GLAPI void GLAPIENTRY glActiveTextureARB(GLenum texture)
{
    RESOLVE(PFNGLACTIVETEXTUREARBPROC, "glActiveTextureARB");
    proc(texture);
}

GLAPI void GLAPIENTRY glMultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1DVARBPROC, "glMultiTexCoord1dvARB");
    proc(target, v);
}

GLAPI void GLAPIENTRY glMultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1FVARBPROC, "glMultiTexCoord1fvARB");
    proc(target, v);
}

GLAPI void GLAPIENTRY glMultiTexCoord1ivARB(GLenum target, const GLint *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1IVARBPROC, "glMultiTexCoord1ivARB");
    proc(target, v);
}

GLAPI void GLAPIENTRY glMultiTexCoord1svARB(GLenum target, const GLshort *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
    RESOLVE(PFNGLMULTITEXCOORD2DVARBPROC, "glMultiTexCoord2dvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
    RESOLVE(PFNGLMULTITEXCOORD2FVARBPROC, "glMultiTexCoord2fvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord2ivARB(GLenum target, const GLint *v)
{
    RESOLVE(PFNGLMULTITEXCOORD2IVARBPROC, "glMultiTexCoord2ivARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord2svARB(GLenum target, const GLshort *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
    RESOLVE(PFNGLMULTITEXCOORD3DVARBPROC, "glMultiTexCoord3dvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
    RESOLVE(PFNGLMULTITEXCOORD3FVARBPROC, "glMultiTexCoord3fvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord3ivARB(GLenum target, const GLint *v)
{
    RESOLVE(PFNGLMULTITEXCOORD3IVARBPROC, "glMultiTexCoord3ivARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord3svARB(GLenum target, const GLshort *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
    RESOLVE(PFNGLMULTITEXCOORD4DVARBPROC, "glMultiTexCoord4dvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
    RESOLVE(PFNGLMULTITEXCOORD4FVARBPROC, "glMultiTexCoord4fvARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord4ivARB(GLenum target, const GLint *v)
{
    RESOLVE(PFNGLMULTITEXCOORD4IVARBPROC, "glMultiTexCoord4ivARB");
    proc(target, v);
}
GLAPI void GLAPIENTRY glMultiTexCoord4svARB(GLenum target, const GLshort *v)
{
    RESOLVE(PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");
    proc(target, v);
}


GLAPI void GLAPIENTRY glActiveStencilFaceEXT(GLenum face)
{
    RESOLVE(PFNGLACTIVESTENCILFACEEXTPROC, "glActiveStencilFaceEXT");
    proc(face);
}

GLAPI void APIENTRY glPointParameterfARB(GLenum pname, GLfloat param)
{
    RESOLVE(PFNGLPOINTPARAMETERFARBPROC, "glPointParameterfARB");
    proc(pname, param);
}

GLAPI void APIENTRY glPointParameterfvARB(GLenum pname, const GLfloat *params)
{
    RESOLVE(PFNGLPOINTPARAMETERFVARBPROC, "glPointParameterfvARB");
    proc(pname, params);
}


GLAPI void APIENTRY glWindowPos3fARB(GLfloat x, GLfloat y, GLfloat z)
{
    RESOLVE(PFNGLWINDOWPOS3FARBPROC, "glWindowPos3fARB");
    proc(x, y, z);
}

GLAPI void APIENTRY glPointParameteri(GLenum pname, GLint param)
{
    RESOLVE(PFNGLPOINTPARAMETERIPROC, "glPointParameteri");
    proc(pname, param);
}

GLAPI void APIENTRY glPointParameteriv(GLenum pname, const GLint *params)
{
    RESOLVE(PFNGLPOINTPARAMETERIVPROC, "glPointParameteriv");
    proc(pname, params);
}

GLAPI void APIENTRY glPointParameteriNV(GLenum pname, GLint param)
{
    RESOLVE(PFNGLPOINTPARAMETERINVPROC, "glPointParameteriNV");
    proc(pname, param);
}

GLAPI void APIENTRY glPointParameterivNV(GLenum pname, const GLint *params)
{
    RESOLVE(PFNGLPOINTPARAMETERIVNVPROC, "glPointParameterivNV");
    proc(pname, params);
}

GLAPI void APIENTRY glSecondaryColor3bv(const GLbyte *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3BVPROC, "glSecondaryColor3bv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3dv(const GLdouble *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3DVPROC, "glSecondaryColor3dv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3fv(const GLfloat *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3FVPROC, "glSecondaryColor3fv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3iv(const GLint *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3IVPROC, "glSecondaryColor3iv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3sv(const GLshort *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3SVPROC, "glSecondaryColor3sv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3ubv(const GLubyte *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3UBVPROC, "glSecondaryColor3ubv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3uiv(const GLuint *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3UIVPROC, "glSecondaryColor3uiv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColor3usv(const GLushort *v)
{
    RESOLVE(PFNGLSECONDARYCOLOR3USVPROC, "glSecondaryColor3usv");
    proc(v);
}
GLAPI void APIENTRY glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    RESOLVE(PFNGLSECONDARYCOLORPOINTERPROC, "glSecondaryColorPointer");
    proc(size, type, stride, pointer);
}


GLAPI void APIENTRY glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    RESOLVE(PFNGLBLENDFUNCSEPARATEPROC, "glBlendFuncSeparate");
    proc(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}
GLAPI void APIENTRY glFogCoordfv(const GLfloat *coord)
{
    RESOLVE(PFNGLFOGCOORDFVPROC, "glFogCoordfv");
    proc(coord);
}
GLAPI void APIENTRY glFogCoorddv(const GLdouble *coord)
{
    RESOLVE(PFNGLFOGCOORDDVPROC, "glFogCoorddv");
    proc(coord);
}
GLAPI void APIENTRY glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    RESOLVE(PFNGLFOGCOORDPOINTERPROC, "glFogCoordPointer");
    proc(type, stride, pointer);
}


GLAPI void APIENTRY glSampleCoverageARB(GLclampf value, GLboolean invert)
{
    RESOLVE(PFNGLSAMPLECOVERAGEARBPROC, "glSampleCoverageARB");
    proc(value, invert);
}
GLAPI void APIENTRY glSampleMaskSGIS(GLclampf value, GLboolean invert)
{
    RESOLVE(PFNGLSAMPLEMASKSGISPROC, "glSampleMaskSGIS");
    proc(value, invert);
}
GLAPI void APIENTRY glSamplePatternSGIS(GLenum pattern)
{
    RESOLVE(PFNGLSAMPLEPATTERNSGISPROC, "glSamplePatternSGIS");
    proc(pattern);
}
