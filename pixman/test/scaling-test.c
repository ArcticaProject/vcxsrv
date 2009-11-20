/*
 * Test program, which can detect problems with nearest neighbout scaling
 * implementation. Also SRC and OVER opetations tested for 16bpp and 32bpp
 * images.
 *
 * Just run it without any command line arguments, and it will report either
 *   "scaling test passed" - everything is ok
 *   "scaling test failed!" - there is some problem
 *
 * In the case of failure, finding the problem involves the following steps:
 * 1. Get the reference 'scaling-test' binary. It makes sense to disable all
 *    the cpu specific optimizations in pixman and also configure it with
 *    '--disable-shared' option. Those who are paranoid can also tweak the
 *    sources to disable all fastpath functions. The resulting binary
 *    can be renamed to something like 'scaling-test.ref'.
 * 2. Compile the buggy binary (also with the '--disable-shared' option).
 * 3. Run 'ruby scaling-test-bisect.rb ./scaling-test.ref ./scaling-test'
 * 4. Look at the information about failed case (destination buffer content
 *    will be shown) and try to figure out what is wrong. It is possible
 *    to use debugging print to stderr in pixman to get more information,
 *    this does not interfere with the testing script.
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define MAX_SRC_WIDTH  10
#define MAX_SRC_HEIGHT 10
#define MAX_DST_WIDTH  10
#define MAX_DST_HEIGHT 10
#define MAX_STRIDE     4

/*
 * Composite operation with pseudorandom images
 */
uint32_t
test_composite (uint32_t initcrc,
		int      testnum,
		int      verbose)
{
    int                i;
    pixman_image_t *   src_img;
    pixman_image_t *   dst_img;
    pixman_transform_t transform;
    pixman_region16_t  clip;
    int                src_width, src_height;
    int                dst_width, dst_height;
    int                src_stride, dst_stride;
    int                src_x, src_y;
    int                dst_x, dst_y;
    int                src_bpp;
    int                dst_bpp;
    int                w, h;
    int                scale_x = 32768, scale_y = 32768;
    int                op;
    int                repeat = 0;
    int                src_fmt, dst_fmt;
    uint32_t *         srcbuf;
    uint32_t *         dstbuf;
    uint32_t           crc32;

    lcg_srand (testnum);

    src_bpp = (lcg_rand_n (2) == 0) ? 2 : 4;
    dst_bpp = (lcg_rand_n (2) == 0) ? 2 : 4;
    op = (lcg_rand_n (2) == 0) ? PIXMAN_OP_SRC : PIXMAN_OP_OVER;

    src_width = lcg_rand_n (MAX_SRC_WIDTH) + 1;
    src_height = lcg_rand_n (MAX_SRC_HEIGHT) + 1;
    dst_width = lcg_rand_n (MAX_DST_WIDTH) + 1;
    dst_height = lcg_rand_n (MAX_DST_HEIGHT) + 1;
    src_stride = src_width * src_bpp + lcg_rand_n (MAX_STRIDE) * src_bpp;
    dst_stride = dst_width * dst_bpp + lcg_rand_n (MAX_STRIDE) * dst_bpp;

    if (src_stride & 3)
	src_stride += 2;

    if (dst_stride & 3)
	dst_stride += 2;

    src_x = -(src_width / 4) + lcg_rand_n (src_width * 3 / 2);
    src_y = -(src_height / 4) + lcg_rand_n (src_height * 3 / 2);
    dst_x = -(dst_width / 4) + lcg_rand_n (dst_width * 3 / 2);
    dst_y = -(dst_height / 4) + lcg_rand_n (dst_height * 3 / 2);
    w = lcg_rand_n (dst_width * 3 / 2 - dst_x);
    h = lcg_rand_n (dst_height * 3 / 2 - dst_y);

    srcbuf = (uint32_t *)malloc (src_stride * src_height);
    dstbuf = (uint32_t *)malloc (dst_stride * dst_height);

    for (i = 0; i < src_stride * src_height; i++)
	*((uint8_t *)srcbuf + i) = lcg_rand_n (256);

    for (i = 0; i < dst_stride * dst_height; i++)
	*((uint8_t *)dstbuf + i) = lcg_rand_n (256);

    src_fmt = src_bpp == 4 ? (lcg_rand_n (2) == 0 ?
                              PIXMAN_a8r8g8b8 : PIXMAN_x8r8g8b8) : PIXMAN_r5g6b5;

    dst_fmt = dst_bpp == 4 ? (lcg_rand_n (2) == 0 ?
                              PIXMAN_a8r8g8b8 : PIXMAN_x8r8g8b8) : PIXMAN_r5g6b5;

    src_img = pixman_image_create_bits (
        src_fmt, src_width, src_height, srcbuf, src_stride);

    dst_img = pixman_image_create_bits (
        dst_fmt, dst_width, dst_height, dstbuf, dst_stride);

    image_endian_swap (src_img, src_bpp * 8);
    image_endian_swap (dst_img, dst_bpp * 8);

    if (lcg_rand_n (8) > 0)
    {
	scale_x = 32768 + lcg_rand_n (65536);
	scale_y = 32768 + lcg_rand_n (65536);
	pixman_transform_init_scale (&transform, scale_x, scale_y);
	pixman_image_set_transform (src_img, &transform);
    }

    switch (lcg_rand_n (4))
    {
    case 0:
	repeat = PIXMAN_REPEAT_NONE;
	break;

    case 1:
	repeat = PIXMAN_REPEAT_NORMAL;
	break;

    case 2:
	repeat = PIXMAN_REPEAT_PAD;
	break;

    case 3:
	repeat = PIXMAN_REPEAT_REFLECT;
	break;

    default:
        break;
    }
    pixman_image_set_repeat (src_img, repeat);

    if (lcg_rand_n (2))
	pixman_image_set_filter (src_img, PIXMAN_FILTER_NEAREST, NULL, 0);
    else
	pixman_image_set_filter (src_img, PIXMAN_FILTER_BILINEAR, NULL, 0);

    if (verbose)
    {
	printf ("src_fmt=%08X, dst_fmt=%08X\n", src_fmt, dst_fmt);
	printf ("op=%d, scale_x=%d, scale_y=%d, repeat=%d\n",
	        op, scale_x, scale_y, repeat);
	printf ("src_width=%d, src_height=%d, dst_width=%d, dst_height=%d\n",
	        src_width, src_height, dst_width, dst_height);
	printf ("src_x=%d, src_y=%d, dst_x=%d, dst_y=%d\n",
	        src_x, src_y, dst_x, dst_y);
	printf ("w=%d, h=%d\n", w, h);
    }

    if (lcg_rand_n (8) == 0)
    {
	pixman_box16_t clip_boxes[2];
	int            n = lcg_rand_n (2) + 1;

	for (i = 0; i < n; i++)
	{
	    clip_boxes[i].x1 = lcg_rand_n (src_width);
	    clip_boxes[i].y1 = lcg_rand_n (src_height);
	    clip_boxes[i].x2 =
		clip_boxes[i].x1 + lcg_rand_n (src_width - clip_boxes[i].x1);
	    clip_boxes[i].y2 =
		clip_boxes[i].y1 + lcg_rand_n (src_height - clip_boxes[i].y1);

	    if (verbose)
	    {
		printf ("source clip box: [%d,%d-%d,%d]\n",
		        clip_boxes[i].x1, clip_boxes[i].y1,
		        clip_boxes[i].x2, clip_boxes[i].y2);
	    }
	}

	pixman_region_init_rects (&clip, clip_boxes, n);
	pixman_image_set_clip_region (src_img, &clip);
	pixman_image_set_source_clipping (src_img, 1);
	pixman_region_fini (&clip);
    }

    if (lcg_rand_n (8) == 0)
    {
	pixman_box16_t clip_boxes[2];
	int            n = lcg_rand_n (2) + 1;
	for (i = 0; i < n; i++)
	{
	    clip_boxes[i].x1 = lcg_rand_n (dst_width);
	    clip_boxes[i].y1 = lcg_rand_n (dst_height);
	    clip_boxes[i].x2 =
		clip_boxes[i].x1 + lcg_rand_n (dst_width - clip_boxes[i].x1);
	    clip_boxes[i].y2 =
		clip_boxes[i].y1 + lcg_rand_n (dst_height - clip_boxes[i].y1);

	    if (verbose)
	    {
		printf ("destination clip box: [%d,%d-%d,%d]\n",
		        clip_boxes[i].x1, clip_boxes[i].y1,
		        clip_boxes[i].x2, clip_boxes[i].y2);
	    }
	}
	pixman_region_init_rects (&clip, clip_boxes, n);
	pixman_image_set_clip_region (dst_img, &clip);
	pixman_region_fini (&clip);
    }

    pixman_image_composite (op, src_img, NULL, dst_img,
                            src_x, src_y, 0, 0, dst_x, dst_y, w, h);

    if (dst_fmt == PIXMAN_x8r8g8b8)
    {
	/* ignore unused part */
	for (i = 0; i < dst_stride * dst_height / 4; i++)
	    dstbuf[i] &= 0xFFFFFF;
    }

    image_endian_swap (dst_img, dst_bpp * 8);

    if (verbose)
    {
	int j;
	
	for (i = 0; i < dst_height; i++)
	{
	    for (j = 0; j < dst_stride; j++)
		printf ("%02X ", *((uint8_t *)dstbuf + i * dst_stride + j));

	    printf ("\n");
	}
    }

    pixman_image_unref (src_img);
    pixman_image_unref (dst_img);

    crc32 = compute_crc32 (initcrc, dstbuf, dst_stride * dst_height);
    free (srcbuf);
    free (dstbuf);
    return crc32;
}

int
main (int   argc, char *argv[])
{
    int      i, n = 0;
    uint32_t crc = 0;

    pixman_disable_out_of_bounds_workaround ();

    if (argc >= 2)
	n = atoi (argv[1]);

    if (n == 0) n = 3000000;

    if (n < 0)
    {
	crc = test_composite (0, -n, 1);
	printf ("crc32=%08X\n", crc);
    }
    else
    {
	for (i = 1; i <= n; i++)
	    crc = test_composite (crc, i, 0);

	printf ("crc32=%08X\n", crc);

	if (n == 3000000)
	{
	    /* predefined value for running with all the fastpath functions disabled  */
	    /* it needs to be updated every time changes are introduced to this program! */

	    if (crc == 0x2168ACD1)
	    {
		printf ("scaling test passed\n");
	    }
	    else
	    {
		printf ("scaling test failed!\n");
		return 1;
	    }
	}
    }

    return 0;
}
