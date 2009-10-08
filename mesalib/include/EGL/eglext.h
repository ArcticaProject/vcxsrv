#ifndef __eglext_h_
#define __eglext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright (c) 2007 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include <EGL/eglplatform.h>

/*************************************************************/

/* Header file version number */
/* eglext.h last updated 2007/11/20 */
/* Current version at http://www.khronos.org/registry/egl/ */
#define EGL_EGLEXT_VERSION 1

#ifndef EGL_KHR_config_attribs
#define EGL_KHR_config_attribs 1
#define EGL_CONFORMANT_KHR			0x3042	/* EGLConfig attribute */
#define EGL_VG_COLORSPACE_LINEAR_BIT_KHR	0x0020	/* EGL_SURFACE_TYPE bitfield */
#define EGL_VG_ALPHA_FORMAT_PRE_BIT_KHR		0x0040	/* EGL_SURFACE_TYPE bitfield */
#endif

#ifndef EGL_KHR_lock_surface
#define EGL_KHR_lock_surface 1
#define EGL_READ_SURFACE_BIT_KHR		0x0001	/* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_WRITE_SURFACE_BIT_KHR		0x0002	/* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_LOCK_SURFACE_BIT_KHR		0x0080	/* EGL_SURFACE_TYPE bitfield */
#define EGL_OPTIMAL_FORMAT_BIT_KHR		0x0100	/* EGL_SURFACE_TYPE bitfield */
#define EGL_MATCH_FORMAT_KHR			0x3043	/* EGLConfig attribute */
#define EGL_FORMAT_RGB_565_EXACT_KHR		0x30C0	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGB_565_KHR			0x30C1	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_EXACT_KHR		0x30C2	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_KHR		0x30C3	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_MAP_PRESERVE_PIXELS_KHR		0x30C4	/* eglLockSurfaceKHR attribute */
#define EGL_LOCK_USAGE_HINT_KHR			0x30C5	/* eglLockSurfaceKHR attribute */
#define EGL_BITMAP_POINTER_KHR			0x30C6	/* eglQuerySurface attribute */
#define EGL_BITMAP_PITCH_KHR			0x30C7	/* eglQuerySurface attribute */
#define EGL_BITMAP_ORIGIN_KHR			0x30C8	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_RED_OFFSET_KHR		0x30C9	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR	0x30CA	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR	0x30CB	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR	0x30CC	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR	0x30CD	/* eglQuerySurface attribute */
#define EGL_LOWER_LEFT_KHR			0x30CE	/* EGL_BITMAP_ORIGIN_KHR value */
#define EGL_UPPER_LEFT_KHR			0x30CF	/* EGL_BITMAP_ORIGIN_KHR value */
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglLockSurfaceKHR (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglUnlockSurfaceKHR (EGLDisplay display, EGLSurface surface);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface);
#endif

#ifndef EGL_KHR_image
#define EGL_KHR_image 1
#define EGL_NATIVE_PIXMAP_KHR			0x30B0	/* eglCreateImageKHR target */
typedef void *EGLImageKHR;
extern const EGLImageKHR EGL_NO_IMAGE_KHR;
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, EGLint *attr_list);
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR (EGLDisplay dpy, EGLImageKHR image);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, EGLint *attr_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef EGL_KHR_vg_parent_image
#define EGL_KHR_vg_parent_image 1
#define EGL_VG_PARENT_IMAGE_KHR			0x30BA	/* eglCreateImageKHR target */
#endif

#ifndef EGL_KHR_gl_texture_2D_image
#define EGL_KHR_gl_texture_2D_image 1
#define EGL_GL_TEXTURE_2D_KHR			0x30B1	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_LEVEL_KHR		0x30BC	/* eglCreateImageKHR attribute */
#endif

#ifndef EGL_KHR_gl_texture_cubemap_image
#define EGL_KHR_gl_texture_cubemap_image 1
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR	0x30B3	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR	0x30B4	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR	0x30B5	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR	0x30B6	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR	0x30B7	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR	0x30B8	/* eglCreateImageKHR target */
#endif

#ifndef EGL_KHR_gl_texture_3D_image
#define EGL_KHR_gl_texture_3D_image 1
#define EGL_GL_TEXTURE_3D_KHR			0x30B2	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_ZOFFSET_KHR		0x30BD	/* eglCreateImageKHR attribute */
#endif

#ifndef EGL_KHR_gl_renderbuffer_image
#define EGL_KHR_gl_renderbuffer_image 1
#define EGL_GL_RENDERBUFFER_KHR			0x30B9	/* eglCreateImageKHR target */
#endif


/* EGL_MESA_screen extension  >>> PRELIMINARY <<< */
#ifndef EGL_MESA_screen_surface
#define EGL_MESA_screen_surface 1

#define EGL_BAD_SCREEN_MESA                    0x4000
#define EGL_BAD_MODE_MESA                      0x4001
#define EGL_SCREEN_COUNT_MESA                  0x4002
#define EGL_SCREEN_POSITION_MESA               0x4003
#define EGL_SCREEN_POSITION_GRANULARITY_MESA   0x4004
#define EGL_MODE_ID_MESA                       0x4005
#define EGL_REFRESH_RATE_MESA                  0x4006
#define EGL_OPTIMAL_MESA                       0x4007
#define EGL_INTERLACED_MESA                    0x4008
#define EGL_SCREEN_BIT_MESA                    0x08

typedef uint32_t EGLScreenMESA;
typedef uint32_t EGLModeMESA;

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglChooseModeMESA(EGLDisplay dpy, EGLScreenMESA screen, const EGLint *attrib_list, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
EGLAPI EGLBoolean EGLAPIENTRY eglGetModesMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
EGLAPI EGLBoolean EGLAPIENTRY eglGetModeAttribMESA(EGLDisplay dpy, EGLModeMESA mode, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY eglGetScreensMESA(EGLDisplay dpy, EGLScreenMESA *screens, EGLint max_screens, EGLint *num_screens);
EGLAPI EGLSurface EGLAPIENTRY eglCreateScreenSurfaceMESA(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglShowScreenSurfaceMESA(EGLDisplay dpy, EGLint screen, EGLSurface surface, EGLModeMESA mode);
EGLAPI EGLBoolean EGLAPIENTRY eglScreenPositionMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLint x, EGLint y);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenSurfaceMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLSurface *surface);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenModeMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *mode);
EGLAPI const char * EGLAPIENTRY eglQueryModeStringMESA(EGLDisplay dpy, EGLModeMESA mode);
#endif /* EGL_EGLEXT_PROTOTYPES */

typedef EGLBoolean (EGLAPIENTRYP PFNEGLCHOOSEMODEMESA) (EGLDisplay dpy, EGLScreenMESA screen, const EGLint *attrib_list, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETMODESMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGetModeATTRIBMESA) (EGLDisplay dpy, EGLModeMESA mode, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSCRREENSMESA) (EGLDisplay dpy, EGLScreenMESA *screens, EGLint max_screens, EGLint *num_screens);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATESCREENSURFACEMESA) (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSHOWSCREENSURFACEMESA) (EGLDisplay dpy, EGLint screen, EGLSurface surface, EGLModeMESA mode);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSCREENPOSIITONMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLint x, EGLint y);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENSURFACEMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLSurface *surface);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENMODEMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *mode);
typedef const char * (EGLAPIENTRYP PFNEGLQUERYMODESTRINGMESA) (EGLDisplay dpy, EGLModeMESA mode);

#endif /* EGL_MESA_screen_surface */


#ifndef EGL_MESA_copy_context
#define EGL_MESA_copy_context 1

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglCopyContextMESA(EGLDisplay dpy, EGLContext source, EGLContext dest, EGLint mask);
#endif /* EGL_EGLEXT_PROTOTYPES */

typedef EGLBoolean (EGLAPIENTRYP PFNEGLCOPYCONTEXTMESA) (EGLDisplay dpy, EGLContext source, EGLContext dest, EGLint mask);

#endif /* EGL_MESA_copy_context */

#ifdef __cplusplus
}
#endif

#endif
