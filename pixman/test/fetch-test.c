#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "pixman.h"


#define SIZE 1024


pixman_indexed_t mono_pallete = {
    .rgba = { 0x00000000, 0x00ffffff },
};


typedef struct {
    pixman_format_code_t format;
    int width, height;
    int stride;
    uint32_t src[SIZE];
    uint32_t dst[SIZE];
    pixman_indexed_t *indexed;
} testcase_t;

testcase_t testcases[] = {
    {
	.format = PIXMAN_a8r8g8b8,
	.width = 2, .height = 2,
	.stride = 8,
	.src = { 0x00112233, 0x44556677, 
	         0x8899aabb, 0xccddeeff },
	.dst = { 0x00112233, 0x44556677, 
	         0x8899aabb, 0xccddeeff },
	.indexed = NULL,
    },
    {
	.format = PIXMAN_g1,
	.width = 8, .height = 2,
	.stride = 4,
	.src = { 0x00000055, 
	         0x000000aa },
	.dst = { 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000,
	         0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff },
	.indexed = &mono_pallete,
    },
#if 0
    {
	.format = PIXMAN_g8,
	.width = 4, .height = 2,
	.stride = 4,
	.src = { 0x01234567, 
	         0x89abcdef },
	.dst = { 0x00010101, 0x00232323, 0x00454545, 0x00676767, 
	         0x00898989, 0x00ababab, 0x00cdcdcd, 0x00efefef, },
    },
#endif
    {
	.format = PIXMAN_yv12,
	.width = 8, .height = 2,
	.stride = 8,
	.src = { 0xff00ff00, 0xff00ff00, 
	         0x00ff00ff, 0x00ff00ff, 
	         0x0080ff80, 
		 0xff800080},
	.dst = { 
		0xff000000, 0xffffffff, 0xffb80000, 0xffffe113,
		0xff000000, 0xffffffff, 0xff0023ee, 0xff4affff,
		0xffffffff, 0xff000000, 0xffffe113, 0xffb80000,
		0xffffffff, 0xff000000, 0xff4affff, 0xff0023ee,
	},
    },
};

const int ntestcases = sizeof(testcases)/sizeof(testcases[0]);


static uint32_t
reader (const void *src, int size)
{
    switch (size)
    {
    case 1:
	return *(uint8_t *)src;
    case 2:
	return *(uint16_t *)src;
    case 4:
	return *(uint32_t *)src;
    default:
	assert(0);
    }
}


static void
writer (void *src, uint32_t value, int size)
{
    switch (size)
    {
    case 1:
	*(uint8_t *)src = value;
	break;
    case 2:
	*(uint16_t *)src = value;
	break;
    case 4:
	*(uint32_t *)src = value;
	break;
    default:
	assert(0);
    }
}


int
main (int argc, char **argv)
{
    uint32_t dst[SIZE];
    pixman_image_t *src_img;
    pixman_image_t *dst_img;
    int i, j, x, y;
    int ret = 0;

    for (i = 0; i < ntestcases; ++i) {
	for (j = 0; j < 2; ++j) {
	    src_img = pixman_image_create_bits (testcases[i].format,
						testcases[i].width, 
						testcases[i].height,
						testcases[i].src,
						testcases[i].stride);
	    pixman_image_set_indexed(src_img, testcases[i].indexed);

	    dst_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
						testcases[i].width, 
						testcases[i].height,
						dst,
						testcases[i].width*4);

	    if (j) {
		pixman_image_set_accessors (src_img, reader, writer);
		pixman_image_set_accessors (dst_img, reader, writer);
	    }
	    
	    pixman_image_composite (PIXMAN_OP_SRC, src_img, NULL, dst_img,
				    0, 0, 0, 0, 0, 0, testcases[i].width, testcases[i].height);

	    pixman_image_unref (src_img);
	    pixman_image_unref (dst_img);

	    for (y = 0; y < testcases[i].height; ++y)
		for (x = 0; x < testcases[i].width; ++x) {
		    int offset = y*testcases[i].width + x;
		    if (dst[offset] != testcases[i].dst[offset]) {
			printf ("test %i%c: pixel mismatch at (x=%d,y=%d): %08x expected, %08x obtained\n",
			        i + 1, 'a' + j,
			        x, y, 
			        testcases[i].dst[offset], dst[offset]);
			ret = 1;
		    }
		}
	}
    }
    
    return ret;
}
