/*
 * Test program, which stresses the use of different color formats and
 * compositing operations.
 *
 * Just run it without any command line arguments, and it will report either
 *   "blitters test passed" - everything is ok
 *   "blitters test failed!" - there is some problem
 *
 * In the case of failure, finding the problem involves the following steps:
 * 1. Get the reference 'blitters-test' binary. It makes sense to disable all
 *    the cpu specific optimizations in pixman and also configure it with
 *    '--disable-shared' option. Those who are paranoid can also tweak the
 *    sources to disable all fastpath functions. The resulting binary
 *    can be renamed to something like 'blitters-test.ref'.
 * 2. Compile the buggy binary (also with the '--disable-shared' option).
 * 3. Run 'ruby blitters-test-bisect.rb ./blitters-test.ref ./blitters-test'
 * 4. Look at the information about failed case (destination buffer content
 *    will be shown) and try to figure out what is wrong. Loading
 *    test program in gdb, specifying failed test number in the command
 *    line with '-' character prepended and setting breakpoint on
 *    'pixman_image_composite' function can provide detailed information
 *    about function arguments
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include "utils.h"

/* A primitive pseudorandom number generator, taken from POSIX.1-2001 example */

static void *
aligned_malloc (size_t align, size_t size)
{
    void *result;

#ifdef HAVE_POSIX_MEMALIGN
    if (posix_memalign (&result, align, size) != 0)
      result = NULL;
#else
    result = malloc (size);
#endif

    return result;
}

/* Create random image for testing purposes */
static pixman_image_t *
create_random_image (pixman_format_code_t *allowed_formats,
		     int                   max_width,
		     int                   max_height,
		     int                   max_extra_stride,
		     pixman_format_code_t *used_fmt)
{
    int n = 0, i, width, height, stride;
    pixman_format_code_t fmt;
    uint32_t *buf;
    pixman_image_t *img;

    while (allowed_formats[n] != -1)
	n++;
    fmt = allowed_formats[lcg_rand_n (n)];
    width = lcg_rand_n (max_width) + 1;
    height = lcg_rand_n (max_height) + 1;
    stride = (width * PIXMAN_FORMAT_BPP (fmt) + 7) / 8 +
	lcg_rand_n (max_extra_stride + 1);
    stride = (stride + 3) & ~3;

    /* do the allocation */
    buf = aligned_malloc (64, stride * height);

    /* initialize image with random data */
    for (i = 0; i < stride * height; i++)
    {
	/* generation is biased to having more 0 or 255 bytes as
	 * they are more likely to be special-cased in code
	 */
	*((uint8_t *)buf + i) = lcg_rand_n (4) ? lcg_rand_n (256) :
	    (lcg_rand_n (2) ? 0 : 255);
    }

    img = pixman_image_create_bits (fmt, width, height, buf, stride);

    image_endian_swap (img, PIXMAN_FORMAT_BPP (fmt));

    if (used_fmt) *used_fmt = fmt;
    return img;
}

/* Free random image, and optionally update crc32 based on its data */
static uint32_t
free_random_image (uint32_t initcrc,
		   pixman_image_t *img,
		   pixman_format_code_t fmt)
{
    uint32_t crc32 = 0;
    int stride = pixman_image_get_stride (img);
    uint32_t *data = pixman_image_get_data (img);
    int height = pixman_image_get_height (img);

    if (fmt != -1)
    {
	/* mask unused 'x' part */
	if (PIXMAN_FORMAT_BPP (fmt) - PIXMAN_FORMAT_DEPTH (fmt) &&
	    PIXMAN_FORMAT_DEPTH (fmt) != 0)
	{
	    int i;
	    uint32_t *data = pixman_image_get_data (img);
	    uint32_t mask = (1 << PIXMAN_FORMAT_DEPTH (fmt)) - 1;

	    if (PIXMAN_FORMAT_TYPE (fmt) == PIXMAN_TYPE_BGRA)
		mask <<= (PIXMAN_FORMAT_BPP (fmt) - PIXMAN_FORMAT_DEPTH (fmt));

	    for (i = 0; i < 32; i++)
		mask |= mask << (i * PIXMAN_FORMAT_BPP (fmt));

	    for (i = 0; i < stride * height / 4; i++)
		data[i] &= mask;
	}

	/* swap endiannes in order to provide identical results on both big
	 * and litte endian systems
	 */
	image_endian_swap (img, PIXMAN_FORMAT_BPP (fmt));
	crc32 = compute_crc32 (initcrc, data, stride * height);
    }

    pixman_image_unref (img);
    free (data);

    return crc32;
}

static pixman_op_t op_list[] = {
    PIXMAN_OP_SRC,
    PIXMAN_OP_OVER,
    PIXMAN_OP_ADD,
    PIXMAN_OP_CLEAR,
    PIXMAN_OP_SRC,
    PIXMAN_OP_DST,
    PIXMAN_OP_OVER,
    PIXMAN_OP_OVER_REVERSE,
    PIXMAN_OP_IN,
    PIXMAN_OP_IN_REVERSE,
    PIXMAN_OP_OUT,
    PIXMAN_OP_OUT_REVERSE,
    PIXMAN_OP_ATOP,
    PIXMAN_OP_ATOP_REVERSE,
    PIXMAN_OP_XOR,
    PIXMAN_OP_ADD,
    PIXMAN_OP_SATURATE,
    PIXMAN_OP_DISJOINT_CLEAR,
    PIXMAN_OP_DISJOINT_SRC,
    PIXMAN_OP_DISJOINT_DST,
    PIXMAN_OP_DISJOINT_OVER,
    PIXMAN_OP_DISJOINT_OVER_REVERSE,
    PIXMAN_OP_DISJOINT_IN,
    PIXMAN_OP_DISJOINT_IN_REVERSE,
    PIXMAN_OP_DISJOINT_OUT,
    PIXMAN_OP_DISJOINT_OUT_REVERSE,
    PIXMAN_OP_DISJOINT_ATOP,
    PIXMAN_OP_DISJOINT_ATOP_REVERSE,
    PIXMAN_OP_DISJOINT_XOR,
    PIXMAN_OP_CONJOINT_CLEAR,
    PIXMAN_OP_CONJOINT_SRC,
    PIXMAN_OP_CONJOINT_DST,
    PIXMAN_OP_CONJOINT_OVER,
    PIXMAN_OP_CONJOINT_OVER_REVERSE,
    PIXMAN_OP_CONJOINT_IN,
    PIXMAN_OP_CONJOINT_IN_REVERSE,
    PIXMAN_OP_CONJOINT_OUT,
    PIXMAN_OP_CONJOINT_OUT_REVERSE,
    PIXMAN_OP_CONJOINT_ATOP,
    PIXMAN_OP_CONJOINT_ATOP_REVERSE,
    PIXMAN_OP_CONJOINT_XOR,
    PIXMAN_OP_MULTIPLY,
    PIXMAN_OP_SCREEN,
    PIXMAN_OP_OVERLAY,
    PIXMAN_OP_DARKEN,
    PIXMAN_OP_LIGHTEN,
    PIXMAN_OP_COLOR_DODGE,
    PIXMAN_OP_COLOR_BURN,
    PIXMAN_OP_HARD_LIGHT,
    PIXMAN_OP_DIFFERENCE,
    PIXMAN_OP_EXCLUSION,
#if 0 /* these use floating point math and are not always bitexact on different platforms */
    PIXMAN_OP_SOFT_LIGHT,
    PIXMAN_OP_HSL_HUE,
    PIXMAN_OP_HSL_SATURATION,
    PIXMAN_OP_HSL_COLOR,
    PIXMAN_OP_HSL_LUMINOSITY,
#endif
};

static pixman_format_code_t img_fmt_list[] = {
    PIXMAN_a8r8g8b8,
    PIXMAN_x8r8g8b8,
    PIXMAN_r5g6b5,
    PIXMAN_r3g3b2,
    PIXMAN_a8,
    PIXMAN_a8b8g8r8,
    PIXMAN_x8b8g8r8,
    PIXMAN_b8g8r8a8,
    PIXMAN_b8g8r8x8,
    PIXMAN_r8g8b8,
    PIXMAN_b8g8r8,
    PIXMAN_r5g6b5,
    PIXMAN_b5g6r5,
    PIXMAN_x2r10g10b10,
    PIXMAN_a2r10g10b10,
    PIXMAN_x2b10g10r10,
    PIXMAN_a2b10g10r10,
    PIXMAN_a1r5g5b5,
    PIXMAN_x1r5g5b5,
    PIXMAN_a1b5g5r5,
    PIXMAN_x1b5g5r5,
    PIXMAN_a4r4g4b4,
    PIXMAN_x4r4g4b4,
    PIXMAN_a4b4g4r4,
    PIXMAN_x4b4g4r4,
    PIXMAN_a8,
    PIXMAN_r3g3b2,
    PIXMAN_b2g3r3,
    PIXMAN_a2r2g2b2,
    PIXMAN_a2b2g2r2,
#if 0 /* using these crashes the test */
    PIXMAN_c8,
    PIXMAN_g8,
    PIXMAN_x4c4,
    PIXMAN_x4g4,
    PIXMAN_c4,
    PIXMAN_g4,
    PIXMAN_g1,
#endif
    PIXMAN_x4a4,
    PIXMAN_a4,
    PIXMAN_r1g2b1,
    PIXMAN_b1g2r1,
    PIXMAN_a1r1g1b1,
    PIXMAN_a1b1g1r1,
    PIXMAN_a1,
    -1
};

static pixman_format_code_t mask_fmt_list[] = {
    PIXMAN_a8r8g8b8,
    PIXMAN_a8,
    PIXMAN_a4,
    PIXMAN_a1,
    -1
};


/*
 * Composite operation with pseudorandom images
 */
uint32_t
test_composite (uint32_t initcrc, int testnum, int verbose)
{
    int i;
    pixman_image_t *src_img = NULL;
    pixman_image_t *dst_img = NULL;
    pixman_image_t *mask_img = NULL;
    int src_width, src_height;
    int dst_width, dst_height;
    int src_stride, dst_stride;
    int src_x, src_y;
    int dst_x, dst_y;
    int mask_x, mask_y;
    int w, h;
    int op;
    pixman_format_code_t src_fmt, dst_fmt, mask_fmt;
    uint32_t *dstbuf;
    uint32_t crc32;
    int max_width, max_height, max_extra_stride;

    max_width = max_height = 24 + testnum / 10000;
    max_extra_stride = 4 + testnum / 1000000;

    if (max_width > 256)
	max_width = 256;

    if (max_height > 16)
	max_height = 16;

    if (max_extra_stride > 8)
	max_extra_stride = 8;

    lcg_srand (testnum);

    op = op_list[lcg_rand_n (sizeof (op_list) / sizeof (op_list[0]))];

    if (lcg_rand_n (8))
    {
	/* normal image */
	src_img = create_random_image (img_fmt_list, max_width, max_height,
				       max_extra_stride, &src_fmt);
    }
    else
    {
	/* solid case */
	src_img = create_random_image (img_fmt_list, 1, 1,
				       max_extra_stride, &src_fmt);

	pixman_image_set_repeat (src_img, PIXMAN_REPEAT_NORMAL);
    }

    dst_img = create_random_image (img_fmt_list, max_width, max_height,
				   max_extra_stride, &dst_fmt);

    mask_img = NULL;
    mask_fmt = -1;
    mask_x = 0;
    mask_y = 0;

    if (lcg_rand_n (2))
    {
	if (lcg_rand_n (2))
	{
	    mask_img = create_random_image (mask_fmt_list, max_width, max_height,
					   max_extra_stride, &mask_fmt);
	}
	else
	{
	    /* solid case */
	    mask_img = create_random_image (mask_fmt_list, 1, 1,
					   max_extra_stride, &mask_fmt);
	    pixman_image_set_repeat (mask_img, PIXMAN_REPEAT_NORMAL);
	}

	if (lcg_rand_n (2))
	    pixman_image_set_component_alpha (mask_img, 1);

	mask_x = lcg_rand_n (pixman_image_get_width (mask_img));
	mask_y = lcg_rand_n (pixman_image_get_height (mask_img));
    }

    src_width = pixman_image_get_width (src_img);
    src_height = pixman_image_get_height (src_img);
    src_stride = pixman_image_get_stride (src_img);

    dst_width = pixman_image_get_width (dst_img);
    dst_height = pixman_image_get_height (dst_img);
    dst_stride = pixman_image_get_stride (dst_img);

    dstbuf = pixman_image_get_data (dst_img);

    src_x = lcg_rand_n (src_width);
    src_y = lcg_rand_n (src_height);
    dst_x = lcg_rand_n (dst_width);
    dst_y = lcg_rand_n (dst_height);

    w = lcg_rand_n (dst_width - dst_x + 1);
    h = lcg_rand_n (dst_height - dst_y + 1);

    if (verbose)
    {
	printf ("op=%d, src_fmt=%08X, dst_fmt=%08X, mask_fmt=%08X\n",
	    op, src_fmt, dst_fmt, mask_fmt);
	printf ("src_width=%d, src_height=%d, dst_width=%d, dst_height=%d\n",
	    src_width, src_height, dst_width, dst_height);
	printf ("src_x=%d, src_y=%d, dst_x=%d, dst_y=%d\n",
	    src_x, src_y, dst_x, dst_y);
	printf ("src_stride=%d, dst_stride=%d\n",
	    src_stride, dst_stride);
	printf ("w=%d, h=%d\n", w, h);
    }

    pixman_image_composite (op, src_img, mask_img, dst_img,
			    src_x, src_y, mask_x, mask_y, dst_x, dst_y, w, h);

    if (verbose)
    {
	int j;

	printf ("---\n");
	for (i = 0; i < dst_height; i++)
	{
	    for (j = 0; j < dst_stride; j++)
	    {
		if (j == (dst_width * PIXMAN_FORMAT_BPP (dst_fmt) + 7) / 8)
		    printf ("| ");

		printf ("%02X ", *((uint8_t *)dstbuf + i * dst_stride + j));
	    }
	    printf ("\n");
	}
	printf ("---\n");
    }

    free_random_image (initcrc, src_img, -1);
    crc32 = free_random_image (initcrc, dst_img, dst_fmt);

    if (mask_img)
	free_random_image (initcrc, mask_img, -1);

    return crc32;
}

int
main (int argc, char *argv[])
{
    int i, n1 = 1, n2 = 0;
    uint32_t crc = 0;
    int verbose = getenv ("VERBOSE") != NULL;

    if (argc >= 3)
    {
	n1 = atoi (argv[1]);
	n2 = atoi (argv[2]);
    }
    else if (argc >= 2)
    {
	n2 = atoi (argv[1]);
    }
    else
    {
	n1 = 1;
	n2 = 2000000;
    }

    if (n2 < 0)
    {
	crc = test_composite (0, abs (n2), 1);
	printf ("crc32=%08X\n", crc);
    }
    else
    {
	for (i = n1; i <= n2; i++)
	{
	    crc = test_composite (crc, i, 0);

	    if (verbose)
		printf ("%d: %08X\n", i, crc);
	}
	printf ("crc32=%08X\n", crc);

	if (n2 == 2000000)
	{
	    /* Predefined value for running with all the fastpath functions
	       disabled. It needs to be updated every time when changes are
	       introduced to this program or behavior of pixman changes! */
	    if (crc == 0x1911E2C3)
	    {
		printf ("blitters test passed\n");
	    }
	    else
	    {
		printf ("blitters test failed!\n");
		return 1;
	    }
	}
    }
    return 0;
}
