#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "pixman.h"

/* This used to go into an infinite loop before pixman-region.c
 * was fixed to not use explict "short" variables
 */
int
main ()
{
    pixman_region32_t r1;
    pixman_region32_t r2;
    pixman_region32_t r3;

    pixman_region32_init_rect (&r1, 0, 0, 20, 64000);
    pixman_region32_init_rect (&r2, 0, 0, 20, 64000);
    pixman_region32_init_rect (&r3, 0, 0, 20, 64000);

    pixman_region32_subtract (&r1, &r2, &r3);

}

