#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pixman.h"

/*
 * We have a source image filled with solid color, set NORMAL or PAD repeat,
 * and some transform which results in nearest neighbour scaling.
 *
 * The expected result is the destination image filled with this solid
 * color.
 */
static int
do_test (int32_t		dst_size,
	 int32_t		src_size,
	 int32_t		src_offs,
	 int32_t		scale_factor,
	 pixman_repeat_t	repeat)
{
    int i;
    pixman_image_t *   src_img;
    pixman_image_t *   dst_img;
    pixman_transform_t transform;
    uint32_t *         srcbuf;
    uint32_t *         dstbuf;

    srcbuf = (uint32_t *)malloc (src_size * 4);
    dstbuf = (uint32_t *)malloc (dst_size * 4);

    /* horizontal test */
    memset (srcbuf, 0xCC, src_size * 4);
    memset (dstbuf, 0x33, dst_size * 4);

    src_img = pixman_image_create_bits (
        PIXMAN_a8r8g8b8, src_size, 1, srcbuf, src_size * 4);
    dst_img = pixman_image_create_bits (
        PIXMAN_a8r8g8b8, dst_size, 1, dstbuf, dst_size * 4);

    pixman_transform_init_scale (&transform, scale_factor, 65536);
    pixman_image_set_transform (src_img, &transform);
    pixman_image_set_repeat (src_img, repeat);
    pixman_image_set_filter (src_img, PIXMAN_FILTER_NEAREST, NULL, 0);

    pixman_image_composite (PIXMAN_OP_SRC, src_img, NULL, dst_img,
                            src_offs, 0, 0, 0, 0, 0, dst_size, 1);

    pixman_image_unref (src_img);
    pixman_image_unref (dst_img);

    for (i = 0; i < dst_size; i++)
    {
	if (dstbuf[i] != 0xCCCCCCCC)
	{
	    free (srcbuf);
	    free (dstbuf);
	    return 1;
	}
    }

    /* vertical test */
    memset (srcbuf, 0xCC, src_size * 4);
    memset (dstbuf, 0x33, dst_size * 4);

    src_img = pixman_image_create_bits (
        PIXMAN_a8r8g8b8, 1, src_size, srcbuf, 4);
    dst_img = pixman_image_create_bits (
        PIXMAN_a8r8g8b8, 1, dst_size, dstbuf, 4);

    pixman_transform_init_scale (&transform, 65536, scale_factor);
    pixman_image_set_transform (src_img, &transform);
    pixman_image_set_repeat (src_img, repeat);
    pixman_image_set_filter (src_img, PIXMAN_FILTER_NEAREST, NULL, 0);

    pixman_image_composite (PIXMAN_OP_SRC, src_img, NULL, dst_img,
                            0, src_offs, 0, 0, 0, 0, 1, dst_size);

    pixman_image_unref (src_img);
    pixman_image_unref (dst_img);

    for (i = 0; i < dst_size; i++)
    {
	if (dstbuf[i] != 0xCCCCCCCC)
	{
	    free (srcbuf);
	    free (dstbuf);
	    return 1;
	}
    }

    free (srcbuf);
    free (dstbuf);
    return 0;
}

int
main (int argc, char *argv[])
{
    pixman_disable_out_of_bounds_workaround ();

    /* can potentially crash */
    assert (do_test (
	48000, 32767, 1, 65536 * 128, PIXMAN_REPEAT_NORMAL) == 0);

    /* can potentially get into a deadloop */
    assert (do_test (
	16384, 65536, 32, 32768, PIXMAN_REPEAT_NORMAL) == 0);

#if 0
    /* can potentially access memory outside source image buffer */
    assert (do_test (
	10, 10, 0, 1, PIXMAN_REPEAT_PAD) == 0);
    assert (do_test (
	10, 10, 0, 0, PIXMAN_REPEAT_PAD) == 0);
#endif

#if 0
    /* can potentially provide invalid results (out of range matrix stuff) */
    assert (do_test (
	48000, 32767, 16384, 65536 * 128, PIXMAN_REPEAT_NORMAL) == 0);
#endif

    return 0;
}
