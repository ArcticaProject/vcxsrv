/*
 * Copyright © 2008 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Zhigang Gong <zhigang.gong@linux.intel.com>
 *
 */

#ifndef GLAMOR_H
#define GLAMOR_H

#include <scrnintstr.h>
#include <pixmapstr.h>
#include <gcstruct.h>
#include <picturestr.h>
#include <fb.h>
#include <fbpict.h>
#ifdef GLAMOR_FOR_XORG
#include <xf86xv.h>
#endif

struct glamor_context;

/*
 * glamor_pixmap_type : glamor pixmap's type.
 * @MEMORY: pixmap is in memory.
 * @TEXTURE_DRM: pixmap is in a texture created from a DRM buffer.
 * @SEPARATE_TEXTURE: The texture is created from a DRM buffer, but
 * 		      the format is incompatible, so this type of pixmap
 * 		      will never fallback to DDX layer.
 * @DRM_ONLY: pixmap is in a external DRM buffer.
 * @TEXTURE_ONLY: pixmap is in an internal texture.
 */
typedef enum glamor_pixmap_type {
    GLAMOR_MEMORY,
    GLAMOR_MEMORY_MAP,
    GLAMOR_TEXTURE_DRM,
    GLAMOR_SEPARATE_TEXTURE,
    GLAMOR_DRM_ONLY,
    GLAMOR_TEXTURE_ONLY,
    GLAMOR_TEXTURE_LARGE,
    GLAMOR_TEXTURE_PACK
} glamor_pixmap_type_t;

#define GLAMOR_EGL_EXTERNAL_BUFFER 3
#define GLAMOR_INVERTED_Y_AXIS  	1
#define GLAMOR_USE_SCREEN		(1 << 1)
#define GLAMOR_USE_PICTURE_SCREEN 	(1 << 2)
#define GLAMOR_USE_EGL_SCREEN		(1 << 3)
#define GLAMOR_NO_DRI3			(1 << 4)
#define GLAMOR_VALID_FLAGS      (GLAMOR_INVERTED_Y_AXIS  		\
				 | GLAMOR_USE_SCREEN 			\
                                 | GLAMOR_USE_PICTURE_SCREEN		\
				 | GLAMOR_USE_EGL_SCREEN                \
                                 | GLAMOR_NO_DRI3)

/* @glamor_init: Initialize glamor internal data structure.
 *
 * @screen: Current screen pointer.
 * @flags:  Please refer the flags description above.
 *
 * 	@GLAMOR_INVERTED_Y_AXIS:
 * 	set 1 means the GL env's origin (0,0) is at top-left.
 * 	EGL/DRM platform is an example need to set this bit.
 * 	glx platform's origin is at bottom-left thus need to
 * 	clear this bit.
 *
 * 	@GLAMOR_USE_SCREEN:
 *	If running in an pre-existing X environment, and the
 * 	gl context is GLX, then you should set this bit and
 * 	let the glamor to handle all the screen related
 * 	functions such as GC ops and CreatePixmap/DestroyPixmap.
 *
 * 	@GLAMOR_USE_PICTURE_SCREEN:
 * 	If don't use any other underlying DDX driver to handle
 * 	the picture related rendering functions, please set this
 * 	bit on. Otherwise, clear this bit. And then it is the DDX
 * 	driver's responsibility to determine how/when to jump to
 * 	glamor's picture compositing path.
 *
 * 	@GLAMOR_USE_EGL_SCREEN:
 * 	If you are using EGL layer, then please set this bit
 * 	on, otherwise, clear it.
 *
 * This function initializes necessary internal data structure
 * for glamor. And before calling into this function, the OpenGL
 * environment should be ready. Should be called before any real
 * glamor rendering or texture allocation functions. And should
 * be called after the DDX's screen initialization or at the last
 * step of the DDX's screen initialization.
 */
extern _X_EXPORT Bool glamor_init(ScreenPtr screen, unsigned int flags);
extern _X_EXPORT void glamor_fini(ScreenPtr screen);

/* This function is used to free the glamor private screen's
 * resources. If the DDX driver is not set GLAMOR_USE_SCREEN,
 * then, DDX need to call this function at proper stage, if
 * it is the xorg DDX driver,then it should be called at free
 * screen stage not the close screen stage. The reason is after
 * call to this function, the xorg DDX may need to destroy the
 * screen pixmap which must be a glamor pixmap and requires
 * the internal data structure still exist at that time.
 * Otherwise, the glamor internal structure will not be freed.*/
extern _X_EXPORT Bool glamor_close_screen(ScreenPtr screen);

/* Let glamor to know the screen's fbo. The low level
 * driver should already assign a tex
 * to this pixmap through the set_pixmap_texture. */
extern _X_EXPORT void glamor_set_screen_pixmap(PixmapPtr screen_pixmap,
                                               PixmapPtr *back_pixmap);

extern _X_EXPORT uint32_t glamor_get_pixmap_texture(PixmapPtr pixmap);

extern _X_EXPORT Bool glamor_glyphs_init(ScreenPtr pScreen);

extern _X_EXPORT void glamor_set_pixmap_texture(PixmapPtr pixmap,
                                                unsigned int tex);

extern _X_EXPORT void glamor_set_pixmap_type(PixmapPtr pixmap,
                                             glamor_pixmap_type_t type);
extern _X_EXPORT void glamor_destroy_textured_pixmap(PixmapPtr pixmap);
extern _X_EXPORT void glamor_block_handler(ScreenPtr screen);

extern _X_EXPORT PixmapPtr glamor_create_pixmap(ScreenPtr screen, int w, int h,
                                                int depth, unsigned int usage);
extern _X_EXPORT Bool glamor_destroy_pixmap(PixmapPtr pixmap);

#define GLAMOR_CREATE_PIXMAP_CPU        0x100
#define GLAMOR_CREATE_PIXMAP_FIXUP      0x101
#define GLAMOR_CREATE_FBO_NO_FBO        0x103
#define GLAMOR_CREATE_PIXMAP_MAP        0x104
#define GLAMOR_CREATE_NO_LARGE          0x105
#define GLAMOR_CREATE_PIXMAP_NO_TEXTURE 0x106

/* @glamor_egl_exchange_buffers: Exchange the underlying buffers(KHR image,fbo).
 *
 * @front: front pixmap.
 * @back: back pixmap.
 *
 * Used by the DRI2 page flip. This function will exchange the KHR images and
 * fbos of the two pixmaps.
 * */
extern _X_EXPORT void glamor_egl_exchange_buffers(PixmapPtr front,
                                                  PixmapPtr back);

extern _X_EXPORT void glamor_pixmap_exchange_fbos(PixmapPtr front,
                                                  PixmapPtr back);

/* The DDX is not supposed to call these three functions */
extern _X_EXPORT void glamor_enable_dri3(ScreenPtr screen);
extern _X_EXPORT unsigned int glamor_egl_create_argb8888_based_texture(ScreenPtr
                                                                       screen,
                                                                       int w,
                                                                       int h);
extern _X_EXPORT int glamor_egl_dri3_fd_name_from_tex(ScreenPtr, PixmapPtr,
                                                      unsigned int, Bool,
                                                      CARD16 *, CARD32 *);

/* @glamor_supports_pixmap_import_export: Returns whether
 * glamor_fd_from_pixmap(), glamor_name_from_pixmap(), and
 * glamor_pixmap_from_fd() are supported.
 *
 * @screen: Current screen pointer.
 *
 * To have DRI3 support enabled, glamor and glamor_egl need to be
 * initialized. glamor also has to be compiled with gbm support.
 *
 * The EGL layer needs to have the following extensions working:
 *
 * .EGL_KHR_gl_texture_2D_image
 * .EGL_EXT_image_dma_buf_import
 * */
extern _X_EXPORT Bool glamor_supports_pixmap_import_export(ScreenPtr screen);

/* @glamor_fd_from_pixmap: Get a dma-buf fd from a pixmap.
 *
 * @screen: Current screen pointer.
 * @pixmap: The pixmap from which we want the fd.
 * @stride, @size: Pointers to fill the stride and size of the
 * 		   buffer associated to the fd.
 *
 * the pixmap and the buffer associated by the fd will share the same
 * content.
 * Returns the fd on success, -1 on error.
 * */
extern _X_EXPORT int glamor_fd_from_pixmap(ScreenPtr screen,
                                           PixmapPtr pixmap,
                                           CARD16 *stride, CARD32 *size);

/**
 * @glamor_name_from_pixmap: Gets a gem name from a pixmap.
 *
 * @pixmap: The pixmap from which we want the gem name.
 *
 * the pixmap and the buffer associated by the gem name will share the
 * same content. This function can be used by the DDX to support DRI2,
 * and needs the same set of buffer export GL extensions as DRI3
 * support.
 *
 * Returns the name on success, -1 on error.
 * */
extern _X_EXPORT int glamor_name_from_pixmap(PixmapPtr pixmap,
                                             CARD16 *stride, CARD32 *size);

/* @glamor_pixmap_from_fd: Creates a pixmap to wrap a dma-buf fd.
 *
 * @screen: Current screen pointer.
 * @fd: The dma-buf fd to import.
 * @width: The width of the buffer.
 * @height: The height of the buffer.
 * @stride: The stride of the buffer.
 * @depth: The depth of the buffer.
 * @bpp: The number of bpp of the buffer.
 *
 * Returns a valid pixmap if the import succeeded, else NULL.
 * */
extern _X_EXPORT PixmapPtr glamor_pixmap_from_fd(ScreenPtr screen,
                                                 int fd,
                                                 CARD16 width,
                                                 CARD16 height,
                                                 CARD16 stride,
                                                 CARD8 depth,
                                                 CARD8 bpp);

#ifdef GLAMOR_FOR_XORG

#define GLAMOR_EGL_MODULE_NAME  "glamoregl"

/* @glamor_egl_init: Initialize EGL environment.
 *
 * @scrn: Current screen info pointer.
 * @fd:   Current drm fd.
 *
 * This function creates and intialize EGL contexts.
 * Should be called from DDX's preInit function.
 * Return TRUE if success, otherwise return FALSE.
 * */
extern _X_EXPORT Bool glamor_egl_init(ScrnInfoPtr scrn, int fd);

extern _X_EXPORT Bool glamor_egl_init_textured_pixmap(ScreenPtr screen);

/* @glamor_egl_create_textured_screen: Create textured screen pixmap.
 *
 * @screen: screen pointer to be processed.
 * @handle: screen pixmap's BO handle.
 * @stride: screen pixmap's stride in bytes.
 *
 * This function is similar with the create_textured_pixmap. As the
 * screen pixmap is a special, we handle it separately in this function.
 */
extern _X_EXPORT Bool glamor_egl_create_textured_screen(ScreenPtr screen,
                                                        int handle, int stride);

/* @glamor_egl_create_textured_screen_ext:
 *
 * extent one parameter to track the pointer of the DDX layer's back pixmap.
 * We need this pointer during the closing screen stage. As before back to
 * the DDX's close screen, we have to free all the glamor related resources.
 */
extern _X_EXPORT Bool glamor_egl_create_textured_screen_ext(ScreenPtr screen,
                                                            int handle,
                                                            int stride,
                                                            PixmapPtr
                                                            *back_pixmap);

/*
 * @glamor_egl_create_textured_pixmap: Try to create a textured pixmap from
 * 				       a BO handle.
 *
 * @pixmap: The pixmap need to be processed.
 * @handle: The BO's handle attached to this pixmap at DDX layer.
 * @stride: Stride in bytes for this pixmap.
 *
 * This function try to create a texture from the handle and attach
 * the texture to the pixmap , thus glamor can render to this pixmap
 * as well. Return true if successful, otherwise return FALSE.
 */
extern _X_EXPORT Bool glamor_egl_create_textured_pixmap(PixmapPtr pixmap,
                                                        int handle, int stride);

/*
 * @glamor_egl_create_textured_pixmap_from_bo: Try to create a textured pixmap
 * 					       from a gbm_bo.
 *
 * @pixmap: The pixmap need to be processed.
 * @bo: a pointer on a gbm_bo structure attached to this pixmap at DDX layer.
 *
 * This function is similar to glamor_egl_create_textured_pixmap.
 */
extern _X_EXPORT Bool
 glamor_egl_create_textured_pixmap_from_gbm_bo(PixmapPtr pixmap, void *bo);

#endif

extern _X_EXPORT void glamor_egl_screen_init(ScreenPtr screen,
                                             struct glamor_context *glamor_ctx);
extern _X_EXPORT void glamor_egl_destroy_textured_pixmap(PixmapPtr pixmap);

extern _X_EXPORT int glamor_create_gc(GCPtr gc);

extern _X_EXPORT void glamor_validate_gc(GCPtr gc, unsigned long changes,
                                         DrawablePtr drawable);

extern Bool _X_EXPORT glamor_change_window_attributes(WindowPtr pWin, unsigned long mask);
extern void _X_EXPORT glamor_copy_window(WindowPtr window, DDXPointRec old_origin, RegionPtr src_region);

/* Glamor rendering/drawing functions with XXX_nf.
 * nf means no fallback within glamor internal if possible. If glamor
 * fail to accelerate the operation, glamor will return a false, and the
 * caller need to implement fallback method. Return a true means the
 * rendering request get done successfully. */
extern _X_EXPORT Bool glamor_fill_spans_nf(DrawablePtr drawable,
                                           GCPtr gc,
                                           int n, DDXPointPtr points,
                                           int *widths, int sorted);

extern _X_EXPORT Bool glamor_poly_fill_rect_nf(DrawablePtr drawable,
                                               GCPtr gc,
                                               int nrect, xRectangle *prect);

extern _X_EXPORT Bool glamor_put_image_nf(DrawablePtr drawable,
                                          GCPtr gc, int depth, int x, int y,
                                          int w, int h, int left_pad,
                                          int image_format, char *bits);

extern _X_EXPORT Bool glamor_copy_n_to_n_nf(DrawablePtr src,
                                            DrawablePtr dst,
                                            GCPtr gc,
                                            BoxPtr box,
                                            int nbox,
                                            int dx,
                                            int dy,
                                            Bool reverse,
                                            Bool upsidedown, Pixel bitplane,
                                            void *closure);

extern _X_EXPORT Bool glamor_composite_nf(CARD8 op,
                                          PicturePtr source,
                                          PicturePtr mask,
                                          PicturePtr dest,
                                          INT16 x_source,
                                          INT16 y_source,
                                          INT16 x_mask,
                                          INT16 y_mask,
                                          INT16 x_dest, INT16 y_dest,
                                          CARD16 width, CARD16 height);

extern _X_EXPORT Bool glamor_trapezoids_nf(CARD8 op,
                                           PicturePtr src, PicturePtr dst,
                                           PictFormatPtr mask_format,
                                           INT16 x_src, INT16 y_src,
                                           int ntrap, xTrapezoid *traps);

extern _X_EXPORT Bool glamor_glyphs_nf(CARD8 op,
                                       PicturePtr src,
                                       PicturePtr dst,
                                       PictFormatPtr mask_format,
                                       INT16 x_src,
                                       INT16 y_src, int nlist,
                                       GlyphListPtr list, GlyphPtr *glyphs);

extern _X_EXPORT Bool glamor_triangles_nf(CARD8 op,
                                          PicturePtr pSrc,
                                          PicturePtr pDst,
                                          PictFormatPtr maskFormat,
                                          INT16 xSrc, INT16 ySrc,
                                          int ntris, xTriangle *tris);

extern _X_EXPORT void glamor_glyph_unrealize(ScreenPtr screen, GlyphPtr glyph);

extern _X_EXPORT Bool glamor_set_spans_nf(DrawablePtr drawable, GCPtr gc,
                                          char *src, DDXPointPtr points,
                                          int *widths, int n, int sorted);

extern _X_EXPORT Bool glamor_get_spans_nf(DrawablePtr drawable, int wmax,
                                          DDXPointPtr points, int *widths,
                                          int count, char *dst);

extern _X_EXPORT Bool glamor_composite_rects_nf(CARD8 op,
                                                PicturePtr pDst,
                                                xRenderColor *color,
                                                int nRect, xRectangle *rects);

extern _X_EXPORT Bool glamor_get_image_nf(DrawablePtr pDrawable, int x, int y,
                                          int w, int h, unsigned int format,
                                          unsigned long planeMask, char *d);

extern _X_EXPORT Bool glamor_add_traps_nf(PicturePtr pPicture,
                                          INT16 x_off,
                                          INT16 y_off, int ntrap,
                                          xTrap *traps);

extern _X_EXPORT Bool glamor_copy_plane_nf(DrawablePtr pSrc, DrawablePtr pDst,
                                           GCPtr pGC, int srcx, int srcy, int w,
                                           int h, int dstx, int dsty,
                                           unsigned long bitPlane,
                                           RegionPtr *pRegion);

extern _X_EXPORT Bool glamor_image_glyph_blt_nf(DrawablePtr pDrawable,
                                                GCPtr pGC, int x, int y,
                                                unsigned int nglyph,
                                                CharInfoPtr *ppci,
                                                void *pglyphBase);

extern _X_EXPORT Bool glamor_poly_glyph_blt_nf(DrawablePtr pDrawable, GCPtr pGC,
                                               int x, int y,
                                               unsigned int nglyph,
                                               CharInfoPtr *ppci,
                                               void *pglyphBase);

extern _X_EXPORT Bool glamor_push_pixels_nf(GCPtr pGC, PixmapPtr pBitmap,
                                            DrawablePtr pDrawable, int w, int h,
                                            int x, int y);

extern _X_EXPORT Bool glamor_poly_point_nf(DrawablePtr pDrawable, GCPtr pGC,
                                           int mode, int npt, DDXPointPtr ppt);

extern _X_EXPORT Bool glamor_poly_segment_nf(DrawablePtr pDrawable, GCPtr pGC,
                                             int nseg, xSegment *pSeg);

extern _X_EXPORT Bool glamor_poly_lines_nf(DrawablePtr drawable, GCPtr gc,
                                           int mode, int n, DDXPointPtr points);

extern _X_EXPORT Bool glamor_poly_text8_nf(DrawablePtr drawable, GCPtr gc,
                                           int x, int y, int count, char *chars, int *final_pos);

extern _X_EXPORT Bool glamor_poly_text16_nf(DrawablePtr drawable, GCPtr gc,
                                            int x, int y, int count, unsigned short *chars, int *final_pos);

extern _X_EXPORT Bool glamor_image_text8_nf(DrawablePtr drawable, GCPtr gc,
                                            int x, int y, int count, char *chars);

extern _X_EXPORT Bool glamor_image_text16_nf(DrawablePtr drawable, GCPtr gc,
                                             int x, int y, int count, unsigned short *chars);

#define HAS_GLAMOR_TEXT 1

#ifdef GLAMOR_FOR_XORG
extern _X_EXPORT XF86VideoAdaptorPtr glamor_xv_init(ScreenPtr pScreen,
                                                    int num_texture_ports);
#endif

#endif                          /* GLAMOR_H */
