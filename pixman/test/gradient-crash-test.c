#include <stdio.h>
#include <stdlib.h>
#include "pixman.h"

int
main (int argc, char **argv)
{
#define WIDTH 400
#define HEIGHT 200
    
    uint32_t *dest = malloc (WIDTH * HEIGHT * 4);
    pixman_image_t *src_img;
    pixman_image_t *dest_img;
    int i, j;

    pixman_gradient_stop_t onestop[1] =
	{
	    { pixman_int_to_fixed (1), { 0xffff, 0xeeee, 0xeeee, 0xeeee } },
	};

    pixman_gradient_stop_t subsetstops[2] =
	{
	    { pixman_int_to_fixed (1), { 0xffff, 0xeeee, 0xeeee, 0xeeee } },
	    { pixman_int_to_fixed (1), { 0xffff, 0xeeee, 0xeeee, 0xeeee } },
	};

    pixman_gradient_stop_t stops01[2] =
	{
	    { pixman_int_to_fixed (0), { 0xffff, 0xeeee, 0xeeee, 0xeeee } },
	    { pixman_int_to_fixed (1), { 0xffff, 0x1111, 0x1111, 0x1111 } }
	};

    pixman_point_fixed_t p1 = { pixman_double_to_fixed (0), 0 };
    pixman_point_fixed_t p2 = { pixman_double_to_fixed (WIDTH / 8.),
				pixman_int_to_fixed (0) };

#if 0
    pixman_transform_t trans = {
	{ { pixman_double_to_fixed (2), pixman_double_to_fixed (0.5), pixman_double_to_fixed (-100), },
	  { pixman_double_to_fixed (0), pixman_double_to_fixed (3), pixman_double_to_fixed (0), },
	  { pixman_double_to_fixed (0), pixman_double_to_fixed (0.000), pixman_double_to_fixed (1.0) } 
	}
    };
#else
    pixman_transform_t trans = {
	{ { pixman_fixed_1, 0, 0 },
	  { 0, pixman_fixed_1, 0 },
	  { 0, 0, pixman_fixed_1 } }
    };
#endif

    pixman_point_fixed_t c_inner;
    pixman_point_fixed_t c_outer;
    pixman_fixed_t r_inner;
    pixman_fixed_t r_outer;
    
    for (i = 0; i < WIDTH * HEIGHT; ++i)
	dest[i] = 0x4f00004f; /* pale blue */
    
    dest_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					 WIDTH, HEIGHT, 
					 dest,
					 WIDTH * 4);

    c_inner.x = pixman_double_to_fixed (50.0);
    c_inner.y = pixman_double_to_fixed (50.0);
    c_outer.x = pixman_double_to_fixed (50.0);
    c_outer.y = pixman_double_to_fixed (50.0);
    r_inner = 0;
    r_outer = pixman_double_to_fixed (50.0);
    
    for (i = 0; i < 3; ++i)
    {
	pixman_gradient_stop_t *stops;
        int num_stops;
	if (i == 0)
	{
	    stops = onestop;
	    num_stops = sizeof(onestop) / sizeof(onestop[0]);
	}
	else if (i == 1)
	{
	    stops = subsetstops;
	    num_stops = sizeof(subsetstops) / sizeof(subsetstops[0]);
	}
	else
	{
	    stops = stops01;
	    num_stops = sizeof(stops01) / sizeof(stops01[0]);
	}
	
	for (j = 0; j < 3; ++j)
	{
	    if (j == 0)
	        src_img = pixman_image_create_conical_gradient (&c_inner, r_inner,
								stops, num_stops);
	    else if (j == 1)
	        src_img = pixman_image_create_radial_gradient  (&c_inner, &c_outer,
								r_inner, r_outer,
								stops, num_stops);
	    else
	        src_img = pixman_image_create_linear_gradient  (&p1, &p2,
								stops, num_stops);
	    pixman_image_set_transform (src_img, &trans);
	    pixman_image_set_repeat (src_img, PIXMAN_REPEAT_NONE);
	    pixman_image_composite (PIXMAN_OP_OVER, src_img, NULL, dest_img,
				    0, 0, 0, 0, 0, 0, 10 * WIDTH, HEIGHT);

	}
	pixman_image_unref (src_img);
    }

    pixman_image_unref (dest_img);
    free (dest);
    
    return 0;
}
