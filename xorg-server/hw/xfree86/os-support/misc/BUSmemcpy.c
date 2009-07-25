
/****************************************************************************

 For Alpha Linux, BusToMem() and MemToBus() can be simply memcpy(), BUT:
  we need to prevent unaligned operations when accessing DENSE space on the BUS,
  as the video memory is mmap'd that way. The below code does this.

NOTE: we could simply use the "memcpy()" from LIBC here, but that, currently, is
      not as fast.

Thanks to Linus Torvalds for contributing this code.

****************************************************************************/


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#ifdef __alpha__

#include "compiler.h"

#define LWORD_CODING (0x60)
#define SPARSE (7)

static unsigned long __memcpy(unsigned long dest, unsigned long src, int n);

_X_EXPORT void
xf86BusToMem(unsigned char *dst, unsigned char *src, int len)
{
	__memcpy((unsigned long)dst, (unsigned long)src, len);
}
_X_EXPORT void
xf86MemToBus(unsigned char *dst, unsigned char *src, int len)
{
  if (len == sizeof(int))
    if (!(((long)src | (long)dst) & 3))
      *((unsigned int*)dst) = *((unsigned int*)(src));
    else {
      int i;
      if (((long)src) & 3)
	i = ldl_u((unsigned int*)src);
      else
	i = *(unsigned int*)src;
      if (((long)dst) & 3)
	stl_u(i,(unsigned int*)dst);
      else
	*(unsigned int*)dst = i;
    }
  else
    __memcpy((unsigned long)dst, (unsigned long)src, len);
}

/*
 *  linux/arch/alpha/lib/memcpy.c
 *
 *  Copyright (C) 1995  Linus Torvalds, used with his permission.
 */

/*
 * This is a reasonably optimized memcpy() routine.
 */

/*
 * Note that the C code is written to be optimized into good assembly. However,
 * at this point gcc is unable to sanely compile "if (n >= 0)", resulting in a
 * explicit compare against 0 (instead of just using the proper "blt reg, xx" or
 * "bge reg, xx"). I hope alpha-gcc will be fixed to notice this eventually..
 */

/*
 * This should be done in one go with ldq_u*2/mask/stq_u. Do it
 * with a macro so that we can fix it up later..
 */
#define ALIGN_DEST_TO8(d,s,n) \
	while (d & 7) { \
		if (n <= 0) return; \
		n--; \
		*(char *) d = *(char *) s; \
		d++; s++; \
	}

/*
 * This should similarly be done with ldq_u*2/mask/stq. The destination
 * is aligned, but we don't fill in a full quad-word
 */
#define DO_REST(d,s,n) \
	while (n > 0) { \
		n--; \
		*(char *) d = *(char *) s; \
		d++; s++; \
	}

/*
 * This should be done with ldq/mask/stq. The source and destination are
 * aligned, but we don't fill in a full quad-word
 */
#define DO_REST_ALIGNED(d,s,n) DO_REST(d,s,n)

/*
 * This does unaligned memory copies. We want to avoid storing to
 * an unaligned address, as that would do a read-modify-write cycle.
 * We also want to avoid double-reading the unaligned reads.
 *
 * Note the ordering to try to avoid load (and address generation) latencies.
 */
static __inline__ void __memcpy_unaligned(unsigned long d, unsigned long s, long n)
{
	ALIGN_DEST_TO8(d,s,n);
	n -= 8;			/* to avoid compare against 8 in the loop */
	if (n >= 0) {
		unsigned long low_word, high_word;
		__asm__("ldq_u %0,%1":"=r" (low_word):"m" (*(unsigned long *) s));
		do {
			unsigned long tmp;
			__asm__("ldq_u %0,%1":"=r" (high_word):"m" (*(unsigned long *)(s+8)));
			n -= 8;
			__asm__("extql %1,%2,%0"
				:"=r" (low_word)
				:"r" (low_word), "r" (s));
			__asm__("extqh %1,%2,%0"
				:"=r" (tmp)
				:"r" (high_word), "r" (s));
			s += 8;
			*(unsigned long *) d = low_word | tmp;
			d += 8;
			low_word = high_word;
		} while (n >= 0);
	}
	n += 8;
	DO_REST(d,s,n);
}

/*
 * Hmm.. Strange. The __asm__ here is there to make gcc use a integer register
 * for the load-store. I don't know why, but it would seem that using a floating
 * point register for the move seems to slow things down (very small difference,
 * though).
 *
 * Note the ordering to try to avoid load (and address generation) latencies.
 */
static __inline__ void __memcpy_aligned(unsigned long d, unsigned long s, long n)
{
	ALIGN_DEST_TO8(d,s,n);
	n -= 8;
	while (n >= 0) {
		unsigned long tmp;
		__asm__("ldq %0,%1":"=r" (tmp):"m" (*(unsigned long *) s));
		n -= 8;
		s += 8;
		*(unsigned long *) d = tmp;
		d += 8;
	}
	n += 8;
	DO_REST_ALIGNED(d,s,n);
}

static unsigned long __memcpy(unsigned long dest, unsigned long src, int n)
{
	if (!((dest ^ src) & 7)) {
		__memcpy_aligned(dest, src, n);
		return dest;
	}
	__memcpy_unaligned(dest, src, n);
	return dest;
}

#else /* __alpha__ */

void
xf86BusToMem(unsigned char *dst, unsigned char *src, int len)
{
	memcpy(dst, src, len);
}
void
xf86MemToBus(unsigned char *dst, unsigned char *src, int len)
{
	memcpy(dst, src, len);
}

#endif /* __alpha__ */
