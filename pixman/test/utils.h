#include <stdlib.h>
#include <config.h>
#include "pixman-private.h" /* For 'inline' definition */

/* A primitive pseudorandom number generator,
 * taken from POSIX.1-2001 example
 */

extern uint32_t lcg_seed;
#ifdef USE_OPENMP
#pragma omp threadprivate(lcg_seed)
#endif

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

static inline uint32_t
lcg_rand_N (int max)
{
    uint32_t lo = lcg_rand ();
    uint32_t hi = lcg_rand () << 15;
    return (lo | hi) % max;
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

/* main body of the fuzzer test */
int
fuzzer_test_main (const char *test_name,
		  int         default_number_of_iterations,
		  uint32_t    expected_checksum,
		  uint32_t    (*test_function)(int testnum, int verbose),
		  int         argc,
		  const char *argv[]);
