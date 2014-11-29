/*
 * Copyright © 2007 Red Hat, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *     Dave Airlie <airlied@redhat.com>
 *
 */
#ifndef DRMMODE_DISPLAY_H
#define DRMMODE_DISPLAY_H

#include "xf86drmMode.h"
#ifdef HAVE_UDEV
#include "libudev.h"
#endif

struct dumb_bo {
    uint32_t handle;
    uint32_t size;
    void *ptr;
    int map_count;
    uint32_t pitch;
};

typedef struct {
    int fd;
    unsigned fb_id;
    unsigned old_fb_id;
    drmModeResPtr mode_res;
    drmModeFBPtr mode_fb;
    int cpp;
    ScrnInfoPtr scrn;
#ifdef HAVE_UDEV
    struct udev_monitor *uevent_monitor;
    InputHandlerProc uevent_handler;
#endif
    drmEventContext event_context;
    struct dumb_bo *front_bo;
    Bool sw_cursor;

    Bool shadow_enable;
    void *shadow_fb;

    /**
     * A screen-sized pixmap when we're doing triple-buffered DRI2
     * pageflipping.
     *
     * One is shared between all drawables that flip to the front
     * buffer, and it only gets reallocated when root pixmap size
     * changes.
     */
    PixmapPtr triple_buffer_pixmap;

    /** The GEM name for triple_buffer_pixmap */
    uint32_t triple_buffer_name;

    DevPrivateKeyRec pixmapPrivateKeyRec;
} drmmode_rec, *drmmode_ptr;

typedef struct {
    drmmode_ptr drmmode;
    drmModeCrtcPtr mode_crtc;
    uint32_t vblank_pipe;
    struct dumb_bo *cursor_bo;
    unsigned rotate_fb_id;
    uint16_t lut_r[256], lut_g[256], lut_b[256];
    DamagePtr slave_damage;

    /**
     * @{ MSC (vblank count) handling for the PRESENT extension.
     *
     * The kernel's vblank counters are 32 bits and apparently full of
     * lies, and we need to give a reliable 64-bit msc for GL, so we
     * have to track and convert to a userland-tracked 64-bit msc.
     */
    int32_t vblank_offset;
    uint32_t msc_prev;
    uint64_t msc_high;
    /** @} */
} drmmode_crtc_private_rec, *drmmode_crtc_private_ptr;

typedef struct {
    drmModePropertyPtr mode_prop;
    uint64_t value;
    int num_atoms;              /* if range prop, num_atoms == 1; if enum prop, num_atoms == num_enums + 1 */
    Atom *atoms;
} drmmode_prop_rec, *drmmode_prop_ptr;

typedef struct {
    drmmode_ptr drmmode;
    int output_id;
    drmModeConnectorPtr mode_output;
    drmModeEncoderPtr *mode_encoders;
    drmModePropertyBlobPtr edid_blob;
    int dpms_enum_id;
    int num_props;
    drmmode_prop_ptr props;
    int enc_mask;
    int enc_clone_mask;
} drmmode_output_private_rec, *drmmode_output_private_ptr;

typedef struct _msPixmapPriv {
    uint32_t fb_id;
    struct dumb_bo *backing_bo; /* if this pixmap is backed by a dumb bo */
} msPixmapPrivRec, *msPixmapPrivPtr;

extern DevPrivateKeyRec msPixmapPrivateKeyRec;

#define msPixmapPrivateKey (&msPixmapPrivateKeyRec)

#define msGetPixmapPriv(drmmode, p) ((msPixmapPrivPtr)dixGetPrivateAddr(&(p)->devPrivates, &(drmmode)->pixmapPrivateKeyRec))

void *drmmode_map_slave_bo(drmmode_ptr drmmode, msPixmapPrivPtr ppriv);
Bool drmmode_SetSlaveBO(PixmapPtr ppix,
                        drmmode_ptr drmmode,
                        int fd_handle, int pitch, int size);

extern Bool drmmode_pre_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int cpp);
void drmmode_adjust_frame(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int x, int y);
extern Bool drmmode_set_desired_modes(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
extern Bool drmmode_setup_colormap(ScreenPtr pScreen, ScrnInfoPtr pScrn);

extern void drmmode_uevent_init(ScrnInfoPtr scrn, drmmode_ptr drmmode);
extern void drmmode_uevent_fini(ScrnInfoPtr scrn, drmmode_ptr drmmode);

Bool drmmode_create_initial_bos(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
void *drmmode_map_front_bo(drmmode_ptr drmmode);
Bool drmmode_map_cursor_bos(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
void drmmode_free_bos(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
void drmmode_get_default_bpp(ScrnInfoPtr pScrn, drmmode_ptr drmmmode,
                             int *depth, int *bpp);
struct dumb_bo *dumb_get_bo_from_fd(int drm_fd, int fd, int pitch, int size);
int dumb_bo_destroy(int fd, struct dumb_bo *bo);


#ifndef DRM_CAP_DUMB_PREFERRED_DEPTH
#define DRM_CAP_DUMB_PREFERRED_DEPTH 3
#endif
#ifndef DRM_CAP_DUMB_PREFER_SHADOW
#define DRM_CAP_DUMB_PREFER_SHADOW 4
#endif

#define MS_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#endif
