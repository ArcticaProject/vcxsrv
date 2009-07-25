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
#include "pixman.h"

/* A primitive pseudorandom number generator, taken from POSIX.1-2001 example */

static uint32_t lcg_seed;

uint32_t
lcg_rand (void)
{
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return ((uint32_t)(lcg_seed / 65536) % 32768);
}

void
lcg_srand (uint32_t seed)
{
    lcg_seed = seed;
}

uint32_t
lcg_rand_n (int max)
{
    return lcg_rand () % max;
}

/*----------------------------------------------------------------------------*\
*  CRC-32 version 2.0.0 by Craig Bruce, 2006-04-29.
*
*  This program generates the CRC-32 values for the files named in the
*  command-line arguments.  These are the same CRC-32 values used by GZIP,
*  PKZIP, and ZMODEM.  The compute_crc32() can also be detached and
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
*     compute_crc32() - computes the CRC-32 value of a memory buffer
*  DESCRIPTION:
*     Computes or accumulates the CRC-32 value for a memory buffer.
*     The 'in_crc32' gives a previously accumulated CRC-32 value to allow
*     a CRC to be generated for multiple sequential buffer-fuls of data.
*     The 'in_crc32' for the first buffer must be zero.
*  ARGUMENTS:
*     in_crc32 - accumulated CRC-32 value, must be 0 on first call
*     buf     - buffer to compute CRC-32 value for
*     buf_len  - number of bytes in buffer
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

    /** accumulate crc32 for buffer **/
    crc32 = in_crc32 ^ 0xFFFFFFFF;
    byte_buf = (unsigned char*) buf;

    for (i = 0; i < buf_len; i++)
	crc32 = (crc32 >> 8) ^ crc_table[(crc32 ^ byte_buf[i]) & 0xFF];
    
    return (crc32 ^ 0xFFFFFFFF);
}

/* perform endian conversion of pixel data */
static void
image_endian_swap (pixman_image_t *img,
		   int             bpp)
{
    int       stride = pixman_image_get_stride (img);
    uint32_t *data = pixman_image_get_data (img);
    int       height = pixman_image_get_height (img);
    int i, j;

    /* swap bytes only on big endian systems */
    volatile uint16_t endian_check_var = 0x1234;
    if (*(volatile uint8_t *)&endian_check_var != 0x12)
	return;

    for (i = 0; i < height; i++)
    {
	char *line_data = (char *)data + stride * i;
	
	/* swap bytes only for 16, 24 and 32 bpp for now */
	switch (bpp)
	{
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
    }
    pixman_image_set_repeat (src_img, repeat);

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

	    if (crc == 0x0B633CF4)
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
