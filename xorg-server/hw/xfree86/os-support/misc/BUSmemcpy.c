
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

/*
 * The Jensen lacks dense memory, thus we have to address the bus via
 * the sparse addressing scheme. These routines are only used in s3im.c
 * Non time critical code uses SlowBCopy_{from/to} bus.
 *
 * Martin Ostermann (ost@comnets.rwth-aachen.de) - Apr.-Sep. 1996
 */

#ifdef TEST_JENSEN_CODE /* define to test the Sparse addressing on a non-Jensen */
#define LWORD_CODING (0x18)
#define SPARSE (5)
#else
#define LWORD_CODING (0x60)
#define SPARSE (7)
#endif

void
xf86JensenMemToBus(char *Base, long dst, long src, int count)
{
    if( ((long)src^((long)dst)) & 3) {  
                               /* src & dst are NOT aligned to each other */
	unsigned long addr;
	unsigned long low_word, high_word,last_read;
	long  rm,loop;
	unsigned long tmp,org,org2,mask,src_org,count_org;
	
	src_org=src;
	count_org=count;
    
	/* add EISA longword coding and round off*/
	addr = (long)(Base+(dst<<SPARSE) + LWORD_CODING) & ~(3<<SPARSE);
	rm = (long)dst & 3;
	count += rm;
	
	count = count_org + rm;
	org = *(volatile unsigned int *)addr;
	__asm__("ldq_u %0,%1"
		:"=r" (low_word):"m" (*(unsigned long *)(src_org)));
	src = src_org - rm;
	if( count > 4  ) {
	    last_read = src_org+count_org - 1;
	    __asm__("ldq_u %0,%1"
		    :"=r" (high_word):"m" (*(unsigned long *)(src+4)));
	    __asm__("extll %1,%2,%0"
		    :"=r" (low_word)
		    :"r" (low_word), "r" ((unsigned long)(src)));
	    __asm__("extlh %1,%2,%0"
		    :"=r" (tmp)
		    :"r" (high_word), "r" ((unsigned long)(src)));
	    tmp |= low_word;
	    src += 4;
	    __asm__("mskqh %1,%2,%0"
		    :"=r" (tmp)
		    :"r" (tmp), "r" (rm));
	    __asm__("mskql %1,%2,%0"
		    :"=r" (org2)
		    :"r" (org), "r" (rm));
	    tmp |= org2;
      
	    loop = (count-4) >> 2; /* loop eqv. count>=4 ; count -= 4 */
	    while (loop) {
                     /* tmp to be stored completly -- need to read next word*/
		low_word = high_word;
		*(volatile unsigned int *) (addr) = tmp;
		__asm__("ldq_u %0,%1"
			:"=r" (high_word):"m" (*(unsigned long*)(src+4)));
		loop --;
		__asm__("extll %1,%2,%0"
			:"=r" (low_word)
			:"r" (low_word), "r" ((unsigned long)src));
		__asm__("extlh %1,%2,%0"
			:"=r" (tmp)
			:"r" (high_word), "r" ((unsigned long)src));
		src += 4;
		tmp |= low_word;
		addr += 4<<SPARSE;
	    }
	    if ( count & 3 ) {
                     /* Store tmp completly, and possibly read one more word.*/
		*(volatile unsigned int *) (addr) = tmp;
		__asm__("ldq_u %0,%1"
			:"=r" (tmp):"m" (*((unsigned long *)(last_read)) ));
		addr += 4<<SPARSE;
		__asm__("extll %1,%2,%0"
			:"=r" (low_word)
			:"r" (high_word), "r" ((unsigned long)src));
		__asm__("extlh %1,%2,%0"
			:"=r" (tmp)
			:"r" (tmp), "r" ((unsigned long)src));
		tmp |= low_word;
		org = *(volatile unsigned int *)addr;
		
		__asm__("mskql %1,%2,%0"
			:"=r" (tmp)
			:"r" (tmp), "r" (count&3));
		__asm__("mskqh %1,%2,%0"
			:"=r" (org)
			:"r" (org), "r" (count&3));
		
		tmp |= org;
	    } 
	    *(volatile unsigned int *) (addr) = tmp;
	    return;
	} else {         /* count > 4  */
	    __asm__("ldq_u %0,%1"
		    :"=r" (high_word):"m" (*(unsigned long *)(src+4)));
	    __asm__("extll %1,%2,%0"
		    :"=r" (low_word)
		    :"r" (low_word), "r" ((unsigned long)(src)));
	    __asm__("extlh %1,%2,%0"
		    :"=r" (tmp)
		    :"r" (high_word), "r" ((unsigned long)(src)));
	    tmp |= low_word;
	    if( count < 4 ) {
		
		mask = -1;
		__asm__("mskqh %1,%2,%0"
			:"=r" (mask)
			:"r" (mask), "r" (rm));
		__asm__("mskql %1,%2,%0"
			:"=r" (mask)
			:"r" (mask), "r" (count));
		tmp = (tmp & mask) | (org & ~mask);
		*(volatile unsigned int *) (addr) = tmp;
		return;
	    }  else {
		__asm__("mskqh %1,%2,%0"
			:"=r" (tmp)
			:"r" (tmp), "r" (rm));
		__asm__("mskql %1,%2,%0"
			:"=r" (org2)
			:"r" (org), "r" (rm));
		
		tmp |= org2;
		*(volatile unsigned int *) (addr) = tmp;
		return;
	    }
	}
    } else {          /* src & dst are aligned to each other */
	unsigned long addr;
	unsigned int tmp,org,rm;
	unsigned int *src_r;
	
	/* add EISA longword coding and round off*/
	addr = (long)(Base+(dst<<SPARSE) + LWORD_CODING) & ~(3<<SPARSE);
	
	src_r = (unsigned int*)((long)src & ~3L);
	rm=(long)src & 3;
	count += rm;
	
	tmp = *src_r;
	org = *(volatile unsigned int *)addr;
	
	__asm__("mskqh %1,%2,%0"
		:"=r" (tmp)
		:"r" (tmp), "r" (rm));
	__asm__("mskql %1,%2,%0"
		:"=r" (org)
		:"r" (org), "r" (rm));
	
	tmp |= org;
	
	while (count > 4) {
	    *(volatile unsigned int *) addr = tmp;
	    addr += 4<<SPARSE;
	    src_r += 1;
	    tmp = *src_r;
	    count -= 4;
	}
	
	org = *(volatile unsigned int *)addr;
	__asm__("mskql %1,%2,%0"
		:"=r" (tmp)
		:"r" (tmp), "r" (count));
	__asm__("mskqh %1,%2,%0"
		:"=r" (org)
		:"r" (org), "r" (count));
	tmp |= org;
	*(volatile unsigned int *) (addr) = tmp;
    }
}

void
xf86JensenBusToMem(char *Base, char *dst, unsigned long src, int count)
{
#if 0
  /* Optimization of BusToMem() is left as an exercise to the reader ;-)    
   * Consider that ldq_u/extlh/extll won't work because of the bus being
   * only 4 bytes wide! 
   */
#else
  unsigned long addr;
  long result;

  addr = (unsigned long)(Base+(src<<SPARSE)) ;
  while( addr & (3<<SPARSE) ){
    if(count <= 0) return;
    result = *(volatile int *) addr;
    result >>= ((addr>>SPARSE) & 3) * 8;
    *dst++ = (char) result;
    addr += 1<<SPARSE;
    count--;
  }
  count -=4;
  while(count >= 0){
    int i;

    result = *(volatile int *) (addr+LWORD_CODING);
    for(i=4;i--;) {
      *dst++ = (char) result;
      result >>= 8;
    }
    addr += 4<<SPARSE;
    count -= 4;
  }
  count +=4;
  
  while( count ){
    result = *(volatile int *) addr;
    result >>= ((addr>>SPARSE) & 3) * 8;
    *dst++ = (char) result;
    addr += 1<<SPARSE;
    count--;
  }
#endif  
}


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
