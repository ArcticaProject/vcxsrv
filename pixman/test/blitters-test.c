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
#include "pixman.h"

/* A primitive pseudorandom number generator, taken from POSIX.1-2001 example */

static uint32_t lcg_seed;

static inline uint32_t
lcg_rand (void)
{
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return ((uint32_t)(lcg_seed / 65536) % 32768);
}

static inline void
lcg_srand (uint32_t seed)
{
    lcg_seed = seed;
}

static inline uint32_t
lcg_rand_n (int max)
{
    return lcg_rand () % max;
}

static void *
aligned_malloc (size_t align, size_t size)
{
    void *result;

#ifdef HAVE_POSIX_MEMALIGN
    posix_memalign (&result, align, size);
#else
    result = malloc (size);
#endif

    return result;
}

/*----------------------------------------------------------------------------*\
 *  CRC-32 version 2.0.0 by Craig Bruce, 2006-04-29.
 *
 *  This program generates the CRC-32 values for the files named in the
 *  command-line arguments.  These are the same CRC-32 values used by GZIP,
 *  PKZIP, and ZMODEM.  The Crc32_ComputeBuf () can also be detached and
 *  used independently.
 *
 *  THIS PROGRAM IS PUBLIC-DOMAIN SOFTWARE.
 *
 *  Based on the byte-oriented implementation "File Verification Using CRC"
 *  by Mark R. Nelson in Dr. Dobb's Journal, May 1992, pp. 64-67.
 *
 *  v1.0.0: original release.
 *  v1.0.1: fixed printf formats.
 *  v1.0.2: fixed something else.
 *  v1.0.3: replaced CRC constant table by generator function.
 *  v1.0.4: reformatted code, made ANSI C.  1994-12-05.
 *  v2.0.0: rewrote to use memory buffer & static table, 2006-04-29.
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
 *  NAME:
 *     Crc32_ComputeBuf () - computes the CRC-32 value of a memory buffer
 *  DESCRIPTION:
 *     Computes or accumulates the CRC-32 value for a memory buffer.
 *     The 'inCrc32' gives a previously accumulated CRC-32 value to allow
 *     a CRC to be generated for multiple sequential buffer-fuls of data.
 *     The 'inCrc32' for the first buffer must be zero.
 *  ARGUMENTS:
 *     inCrc32 - accumulated CRC-32 value, must be 0 on first call
 *     buf     - buffer to compute CRC-32 value for
 *     bufLen  - number of bytes in buffer
 *  RETURNS:
 *     crc32 - computed CRC-32 value
 *  ERRORS:
 *     (no errors are possible)
\*----------------------------------------------------------------------------*/

static uint32_t
compute_crc32 (uint32_t    in_crc32,
	       const void *buf,
	       size_t      buf_len)
{
    static const uint32_t crc_table[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
	0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD,	0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
	0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,	0x14015C4F, 0x63066CD9,
	0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
	0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
	0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
	0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
	0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
	0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
	0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
	0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
	0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
	0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
	0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
	0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
	0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
	0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
	0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
	0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
	0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    uint32_t              crc32;
    unsigned char *       byte_buf;
    size_t                i;

    /* accumulate crc32 for buffer */
    crc32 = in_crc32 ^ 0xFFFFFFFF;
    byte_buf = (unsigned char*) buf;

    for (i = 0; i < buf_len; i++)
	crc32 = (crc32 >> 8) ^ crc_table[(crc32 ^ byte_buf[i]) & 0xFF];

    return (crc32 ^ 0xFFFFFFFF);
}

/* perform endian conversion of pixel data */
static void
image_endian_swap (pixman_image_t *img, int bpp)
{
    int stride = pixman_image_get_stride (img);
    uint32_t *data = pixman_image_get_data (img);
    int height = pixman_image_get_height (img);;
    int i, j;

    /* swap bytes only on big endian systems */
    volatile uint16_t endian_check_var = 0x1234;
    if (*(volatile uint8_t *)&endian_check_var != 0x12)
	return;

    for (i = 0; i < height; i++)
    {
	uint8_t *line_data = (uint8_t *)data + stride * i;
	/* swap bytes only for 16, 24 and 32 bpp for now */
	switch (bpp)
	{
	case 1:
	    for (j = 0; j < stride; j++)
	    {
		line_data[j] =
		    ((line_data[j] & 0x80) >> 7) |
		    ((line_data[j] & 0x40) >> 5) |
		    ((line_data[j] & 0x20) >> 3) |
		    ((line_data[j] & 0x10) >> 1) |
		    ((line_data[j] & 0x08) << 1) |
		    ((line_data[j] & 0x04) << 3) |
		    ((line_data[j] & 0x02) << 5) |
		    ((line_data[j] & 0x01) << 7);
	    }
	    break;
	case 4:
	    for (j = 0; j < stride; j++)
	    {
		line_data[j] = (line_data[j] >> 4) | (line_data[j] << 4);
	    }
	    break;
	case 16:
	    for (j = 0; j + 2 <= stride; j += 2)
	    {
		char t1 = line_data[j + 0];
		char t2 = line_data[j + 1];

		line_data[j + 1] = t1;
		line_data[j + 0] = t2;
	    }
	    break;
	case 24:
	    for (j = 0; j + 3 <= stride; j += 3)
	    {
		char t1 = line_data[j + 0];
		char t2 = line_data[j + 1];
		char t3 = line_data[j + 2];

		line_data[j + 2] = t1;
		line_data[j + 1] = t2;
		line_data[j + 0] = t3;
	    }
	    break;
	case 32:
	    for (j = 0; j + 4 <= stride; j += 4)
	    {
		char t1 = line_data[j + 0];
		char t2 = line_data[j + 1];
		char t3 = line_data[j + 2];
		char t4 = line_data[j + 3];

		line_data[j + 3] = t1;
		line_data[j + 2] = t2;
		line_data[j + 1] = t3;
		line_data[j + 0] = t4;
	    }
	    break;
	default:
	    break;
	}
    }
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
    int height = pixman_image_get_height (img);;

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
			    src_x, src_y, src_x, src_y, dst_x, dst_y, w, h);

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
	    if (crc == 0x06D8EDB6)
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
