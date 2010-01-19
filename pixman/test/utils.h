#include <stdlib.h>
#include <config.h>
#include "pixman-private.h" /* For 'inline' definition */

/* A primitive pseudorandom number generator,
 * taken from POSIX.1-2001 example
 */

extern uint32_t lcg_seed;

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


/* CRC 32 computation
 */
uint32_t
compute_crc32 (uint32_t    in_crc32,
	       const void *buf,
	       size_t      buf_len);

/* perform endian conversion of pixel data
 */
void
image_endian_swap (pixman_image_t *img, int bpp);

/* Generate n_bytes random bytes in malloced memory */
uint8_t *
make_random_bytes (int n_bytes);
