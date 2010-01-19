#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define WIDTH 400
#define HEIGHT 200

int
main (int argc, char **argv)
{
    uint8_t *alpha = make_random_bytes (WIDTH * HEIGHT);
    uint32_t *src = (uint32_t *)make_random_bytes (WIDTH * HEIGHT * 4);
    uint32_t *dest = (uint32_t *)make_random_bytes (WIDTH * HEIGHT * 4);
    int i;

    pixman_image_t *a = pixman_image_create_bits (PIXMAN_a8, WIDTH, HEIGHT, (uint32_t *)alpha, WIDTH);
    pixman_image_t *d = pixman_image_create_bits (PIXMAN_a8r8g8b8, WIDTH, HEIGHT, dest, WIDTH * 4);

    for (i = 0; i < 2; ++i)
    {
	pixman_format_code_t sformat = (i == 0)? PIXMAN_a8r8g8b8 : PIXMAN_a2r10g10b10;
	pixman_image_t *s = pixman_image_create_bits (sformat, WIDTH, HEIGHT, src, WIDTH * 4);
	int j, k;

	pixman_image_set_alpha_map (s, a, 0, 0);

	pixman_image_composite (PIXMAN_OP_SRC, s, NULL, d, 0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

	for (j = 0; j < HEIGHT; ++j)
	{
	    for (k = 0; k < WIDTH; ++k)
	    {
		uint8_t ap = ((uint8_t *)alpha)[j * WIDTH + k];
		uint32_t dap = (dest[j * WIDTH + k] >> 24);
		uint32_t sap = (src[j * WIDTH + k] >> 24);

		if (ap != dap)
		{
		    printf ("Wrong alpha value at (%d, %d). Should be %d; got %d (src was %d)\n", k, j, ap, dap, sap);
		    return 1;
		}
	    }
	}

	pixman_image_unref (s);
    }

    return 0;
}
