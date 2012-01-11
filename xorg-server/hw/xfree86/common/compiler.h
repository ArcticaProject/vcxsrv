/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * Copyright (c) 1994-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifndef _COMPILER_H

# define _COMPILER_H

#if defined(__SUNPRO_C)
# define DO_PROTOTYPES
#endif

/* Map Sun compiler platform defines to gcc-style used in the code */
#if defined(__amd64) && !defined(__amd64__)
# define __amd64__
#endif
#if defined(__i386) && !defined(__i386__)
# define __i386__
#endif
#if defined(__sparc) && !defined(__sparc__)
# define __sparc__
#endif
#if defined(__sparcv9) && !defined(__sparc64__)
# define __sparc64__
#endif

#ifndef _X_EXPORT
# include <X11/Xfuncproto.h>
#endif

# include <pixman.h> /* for uint*_t types */

/* Allow drivers to use the GCC-supported __inline__ and/or __inline. */
# ifndef __inline__
#  if defined(__GNUC__)
    /* gcc has __inline__ */
#  elif defined(__HIGHC__)
#   define __inline__ _Inline
#  else
#   define __inline__ /**/
#  endif
# endif /* __inline__ */
# ifndef __inline
#  if defined(__GNUC__)
    /* gcc has __inline */
#  elif defined(__HIGHC__)
#   define __inline _Inline
#  else
#   define __inline /**/
#  endif
# endif /* __inline */

/* Support gcc's __FUNCTION__ for people using other compilers */
#if !defined(__GNUC__) && !defined(__FUNCTION__)
# define __FUNCTION__ __func__ /* C99 */
#endif

# if defined(NO_INLINE) || defined(DO_PROTOTYPES)

#  if !defined(__arm__)
#   if !defined(__sparc__) && !defined(__sparc) && !defined(__arm32__) && !defined(__nds32__) \
      && !(defined(__alpha__) && defined(linux)) \
      && !(defined(__ia64__) && defined(linux)) \

extern _X_EXPORT void outb(unsigned short, unsigned char);
extern _X_EXPORT void outw(unsigned short, unsigned short);
extern _X_EXPORT void outl(unsigned short, unsigned int);
extern _X_EXPORT unsigned int inb(unsigned short);
extern _X_EXPORT unsigned int inw(unsigned short);
extern _X_EXPORT unsigned int inl(unsigned short);

#   else /* __sparc__,  __arm32__, __alpha__, __nds32__ */

extern _X_EXPORT void outb(unsigned long, unsigned char);
extern _X_EXPORT void outw(unsigned long, unsigned short);
extern _X_EXPORT void outl(unsigned long, unsigned int);
extern _X_EXPORT unsigned int inb(unsigned long);
extern _X_EXPORT unsigned int inw(unsigned long);
extern _X_EXPORT unsigned int inl(unsigned long);

#   endif /* __sparc__,  __arm32__, __alpha__, __nds32__ */
#  endif /* __arm__ */

#  if defined(__powerpc__) && !defined(__OpenBSD__)
extern unsigned long ldq_u(unsigned long *);
extern unsigned long ldl_u(unsigned int *);
extern unsigned long ldw_u(unsigned short *);
extern void stq_u(unsigned long, unsigned long *);
extern void stl_u(unsigned long, unsigned int *);
extern void stw_u(unsigned long, unsigned short *);
extern void mem_barrier(void);
extern void write_mem_barrier(void);
extern void stl_brx(unsigned long, volatile unsigned char *, int);
extern void stw_brx(unsigned short, volatile unsigned char *, int);
extern unsigned long ldl_brx(volatile unsigned char *, int);
extern unsigned short ldw_brx(volatile unsigned char *, int);
#  endif /* __powerpc__ && !__OpenBSD */

# endif /* NO_INLINE || DO_PROTOTYPES */

# ifndef NO_INLINE
#  ifdef __GNUC__
#   ifdef __i386__

#    ifdef __SSE__
#     define write_mem_barrier() __asm__ __volatile__ ("sfence" : : : "memory")
#    else
#     define write_mem_barrier() __asm__ __volatile__ ("lock; addl $0,0(%%esp)" : : : "memory")
#    endif

#    ifdef __SSE2__
#     define mem_barrier() __asm__ __volatile__ ("mfence" : : : "memory")
#    else
#     define mem_barrier() __asm__ __volatile__ ("lock; addl $0,0(%%esp)" : : : "memory")
#    endif

#   elif defined __alpha__

#    define mem_barrier() __asm__ __volatile__ ("mb" : : : "memory")
#    define write_mem_barrier() __asm__ __volatile__ ("wmb" : : : "memory")

#   elif defined __amd64__

#    define mem_barrier() __asm__ __volatile__ ("mfence" : : : "memory")
#    define write_mem_barrier() __asm__ __volatile__ ("sfence" : : : "memory")

#   elif defined __ia64__

#    ifndef __INTEL_COMPILER
#     define mem_barrier()        __asm__ __volatile__ ("mf" : : : "memory")
#     define write_mem_barrier()  __asm__ __volatile__ ("mf" : : : "memory")
#    else
#     include "ia64intrin.h"
#     define mem_barrier() __mf()
#     define write_mem_barrier() __mf()
#    endif

#   elif defined __mips__
     /* Note: sync instruction requires MIPS II instruction set */
#    define mem_barrier()		\
	__asm__ __volatile__(		\
		".set   push\n\t"	\
		".set   noreorder\n\t"	\
		".set   mips2\n\t"	\
		"sync\n\t"		\
		".set   pop"		\
		: /* no output */	\
		: /* no input */	\
		: "memory")
#    define write_mem_barrier() mem_barrier()

#   elif defined __powerpc__

#    if defined(linux) && defined(__powerpc64__)
#     include <linux/version.h>
#     if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#      include <asm/memory.h>
#     endif
#    endif /* defined(linux) && defined(__powerpc64__) */

#    ifndef eieio /* We deal with arch-specific eieio() routines above... */
#     define eieio() __asm__ __volatile__ ("eieio" ::: "memory")
#    endif /* eieio */
#    define mem_barrier()	eieio()
#    define write_mem_barrier()	eieio()

#   elif defined __sparc__

#    define barrier() __asm__ __volatile__ (".word 0x8143e00a" : : : "memory")
#    define mem_barrier()         /* XXX: nop for now */
#    define write_mem_barrier()   /* XXX: nop for now */
#   endif
#  endif /* __GNUC__ */
# endif /* NO_INLINE */

# ifndef mem_barrier
#  define mem_barrier() /* NOP */
# endif

# ifndef write_mem_barrier
#  define write_mem_barrier() /* NOP */
# endif


# ifndef NO_INLINE
#  ifdef __GNUC__

/* Define some packed structures to use with unaligned accesses */

struct __una_u64 { uint64_t x __attribute__((packed)); };
struct __una_u32 { uint32_t x __attribute__((packed)); };
struct __una_u16 { uint16_t x __attribute__((packed)); };

/* Elemental unaligned loads */

static __inline__ uint64_t ldq_u(uint64_t *p)
{
	const struct __una_u64 *ptr = (const struct __una_u64 *) p;
	return ptr->x;
}

static __inline__ uint32_t ldl_u(uint32_t *p)
{
	const struct __una_u32 *ptr = (const struct __una_u32 *) p;
	return ptr->x;
}

static __inline__ uint16_t ldw_u(uint16_t *p)
{
	const struct __una_u16 *ptr = (const struct __una_u16 *) p;
	return ptr->x;
}

/* Elemental unaligned stores */

static __inline__ void stq_u(uint64_t val, uint64_t *p)
{
	struct __una_u64 *ptr = (struct __una_u64 *) p;
	ptr->x = val;
}

static __inline__ void stl_u(uint32_t val, uint32_t *p)
{
	struct __una_u32 *ptr = (struct __una_u32 *) p;
	ptr->x = val;
}

static __inline__ void stw_u(uint16_t val, uint16_t *p)
{
	struct __una_u16 *ptr = (struct __una_u16 *) p;
	ptr->x = val;
}
#  else /* !__GNUC__ */

#include <string.h> /* needed for memmove */

static __inline__ uint64_t ldq_u(uint64_t *p)
{
	uint64_t ret;
	memmove(&ret, p, sizeof(*p));
	return ret;
}

static __inline__ uint32_t ldl_u(uint32_t *p)
{
	uint32_t ret;
	memmove(&ret, p, sizeof(*p));
	return ret;
}

static __inline__ uint16_t ldw_u(uint16_t *p)
{
	uint16_t ret;
	memmove(&ret, p, sizeof(*p));
	return ret;
}

static __inline__ void stq_u(uint64_t val, uint64_t *p)
{
	uint64_t tmp = val;
	memmove(p, &tmp, sizeof(*p));
}

static __inline__ void stl_u(uint32_t val, uint32_t *p)
{
	uint32_t tmp = val;
	memmove(p, &tmp, sizeof(*p));
}

static __inline__ void stw_u(uint16_t val, uint16_t *p)
{
	uint16_t tmp = val;
	memmove(p, &tmp, sizeof(*p));
}

#  endif /* __GNUC__ */
# endif /* NO_INLINE */

# ifndef NO_INLINE
#  ifdef __GNUC__
#   if (defined(linux) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)) && (defined(__alpha__))

#    ifdef linux
/* for Linux on Alpha, we use the LIBC _inx/_outx routines */
/* note that the appropriate setup via "ioperm" needs to be done */
/*  *before* any inx/outx is done. */

extern _X_EXPORT void _outb(unsigned char val, unsigned long port);
extern _X_EXPORT void _outw(unsigned short val, unsigned long port);
extern _X_EXPORT void _outl(unsigned int val, unsigned long port);
extern _X_EXPORT unsigned int _inb(unsigned long port);
extern _X_EXPORT unsigned int _inw(unsigned long port);
extern _X_EXPORT unsigned int _inl(unsigned long port);

static __inline__ void
outb(unsigned long port, unsigned char val)
{
    _outb(val, port);
}

static __inline__ void
outw(unsigned long port, unsigned short val)
{
    _outw(val, port);
}

static __inline__ void
outl(unsigned long port, unsigned int val)
{
    _outl(val, port);
}

static __inline__ unsigned int
inb(unsigned long port)
{
  return _inb(port);
}

static __inline__ unsigned int
inw(unsigned long port)
{
  return _inw(port);
}

static __inline__ unsigned int
inl(unsigned long port)
{
  return _inl(port);
}

#    endif /* linux */

#    if (defined(__FreeBSD__) || defined(__OpenBSD__)) \
      && !defined(DO_PROTOTYPES)

/* for FreeBSD and OpenBSD on Alpha, we use the libio (resp. libalpha) */
/*  inx/outx routines */
/* note that the appropriate setup via "ioperm" needs to be done */
/*  *before* any inx/outx is done. */

extern _X_EXPORT void outb(unsigned int port, unsigned char val);
extern _X_EXPORT void outw(unsigned int port, unsigned short val);
extern _X_EXPORT void outl(unsigned int port, unsigned int val);
extern _X_EXPORT unsigned char inb(unsigned int port);
extern _X_EXPORT unsigned short inw(unsigned int port);
extern _X_EXPORT unsigned int inl(unsigned int port);

#    endif /* (__FreeBSD__ || __OpenBSD__ ) && !DO_PROTOTYPES */


#if defined(__NetBSD__)
#include <machine/pio.h>
#endif /* __NetBSD__ */

#   elif (defined(linux) || defined(__FreeBSD__)) && defined(__amd64__)
 
#    include <inttypes.h>

static __inline__ void
outb(unsigned short port, unsigned char val)
{
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
}


static __inline__ void
outw(unsigned short port, unsigned short val)
{
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
outl(unsigned short port, unsigned int val)
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(unsigned short port)
{
   unsigned char ret;
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inw(unsigned short port)
{
   unsigned short ret;
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inl(unsigned short port)
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

#   elif (defined(linux) || defined(sun) || defined(__OpenBSD__) || defined(__FreeBSD__)) && defined(__sparc__)

#     ifndef ASI_PL
#      define ASI_PL 0x88
#     endif

static __inline__ void
outb(unsigned long port, unsigned char val)
{
	__asm__ __volatile__("stba %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (port), "i" (ASI_PL));
	barrier();
}

static __inline__ void
outw(unsigned long port, unsigned short val)
{
	__asm__ __volatile__("stha %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (port), "i" (ASI_PL));
	barrier();
}

static __inline__ void
outl(unsigned long port, unsigned int val)
{
	__asm__ __volatile__("sta %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (port), "i" (ASI_PL));
	barrier();
}

static __inline__ unsigned int
inb(unsigned long port)
{
	unsigned int ret;
	__asm__ __volatile__("lduba [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (port), "i" (ASI_PL));
	return ret;
}

static __inline__ unsigned int
inw(unsigned long port)
{
	unsigned int ret;
	__asm__ __volatile__("lduha [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (port), "i" (ASI_PL));
	return ret;
}

static __inline__ unsigned int
inl(unsigned long port)
{
	unsigned int ret;
	__asm__ __volatile__("lda [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (port), "i" (ASI_PL));
	return ret;
}

static __inline__ unsigned char
xf86ReadMmio8(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned char ret;

	__asm__ __volatile__("lduba [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (addr), "i" (ASI_PL));
	return ret;
}

static __inline__ unsigned short
xf86ReadMmio16Be(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned short ret;

	__asm__ __volatile__("lduh [%1], %0"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned short
xf86ReadMmio16Le(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned short ret;

	__asm__ __volatile__("lduha [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (addr), "i" (ASI_PL));
	return ret;
}

static __inline__ unsigned int
xf86ReadMmio32Be(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned int ret;

	__asm__ __volatile__("ld [%1], %0"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned int
xf86ReadMmio32Le(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned int ret;

	__asm__ __volatile__("lda [%1] %2, %0"
			     : "=r" (ret)
			     : "r" (addr), "i" (ASI_PL));
	return ret;
}

static __inline__ void
xf86WriteMmio8(__volatile__ void *base, const unsigned long offset,
	       const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("stba %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
	barrier();
}

static __inline__ void
xf86WriteMmio16Be(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("sth %0, [%1]"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
	barrier();
}

static __inline__ void
xf86WriteMmio16Le(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("stha %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
	barrier();
}

static __inline__ void
xf86WriteMmio32Be(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("st %0, [%1]"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
	barrier();
}

static __inline__ void
xf86WriteMmio32Le(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("sta %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
	barrier();
}

static __inline__ void
xf86WriteMmio8NB(__volatile__ void *base, const unsigned long offset,
		 const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("stba %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
}

static __inline__ void
xf86WriteMmio16BeNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("sth %0, [%1]"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void
xf86WriteMmio16LeNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("stha %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
}

static __inline__ void
xf86WriteMmio32BeNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("st %0, [%1]"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void
xf86WriteMmio32LeNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("sta %0, [%1] %2"
			     : /* No outputs */
			     : "r" (val), "r" (addr), "i" (ASI_PL));
}

#   elif defined(__mips__) || (defined(__arm32__) && !defined(__linux__))
#    ifdef __arm32__
#     define PORT_SIZE long
#    else
#     define PORT_SIZE short
#    endif

_X_EXPORT unsigned int IOPortBase;  /* Memory mapped I/O port area */

static __inline__ void
outb(unsigned PORT_SIZE port, unsigned char val)
{
	*(volatile unsigned char*)(((unsigned PORT_SIZE)(port))+IOPortBase) = val;
}

static __inline__ void
outw(unsigned PORT_SIZE port, unsigned short val)
{
	*(volatile unsigned short*)(((unsigned PORT_SIZE)(port))+IOPortBase) = val;
}

static __inline__ void
outl(unsigned PORT_SIZE port, unsigned int val)
{
	*(volatile unsigned int*)(((unsigned PORT_SIZE)(port))+IOPortBase) = val;
}

static __inline__ unsigned int
inb(unsigned PORT_SIZE port)
{
	return *(volatile unsigned char*)(((unsigned PORT_SIZE)(port))+IOPortBase);
}

static __inline__ unsigned int
inw(unsigned PORT_SIZE port)
{
	return *(volatile unsigned short*)(((unsigned PORT_SIZE)(port))+IOPortBase);
}

static __inline__ unsigned int
inl(unsigned PORT_SIZE port)
{
	return *(volatile unsigned int*)(((unsigned PORT_SIZE)(port))+IOPortBase);
}


#    if defined(__mips__)
#     ifdef linux	/* don't mess with other OSs */
#       if X_BYTE_ORDER == X_BIG_ENDIAN
static __inline__ unsigned int
xf86ReadMmio32Be(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned int ret;

	__asm__ __volatile__("lw %0, 0(%1)"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ void
xf86WriteMmio32Be(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__("sw %0, 0(%1)"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}
#      endif
#     endif /* !linux */
#    endif /* __mips__ */

#   elif (defined(linux) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)) && defined(__powerpc__)

#    ifndef MAP_FAILED
#     define MAP_FAILED ((void *)-1)
#    endif

extern _X_EXPORT volatile unsigned char *ioBase;

static __inline__ unsigned char
xf86ReadMmio8(__volatile__ void *base, const unsigned long offset)
{
        register unsigned char val;
        __asm__ __volatile__(
                        "lbzx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return val;
}

static __inline__ unsigned short
xf86ReadMmio16Be(__volatile__ void *base, const unsigned long offset)
{
        register unsigned short val;
        __asm__ __volatile__(
                        "lhzx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return val;
}

static __inline__ unsigned short
xf86ReadMmio16Le(__volatile__ void *base, const unsigned long offset)
{
        register unsigned short val;
        __asm__ __volatile__(
                        "lhbrx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return val;
}

static __inline__ unsigned int
xf86ReadMmio32Be(__volatile__ void *base, const unsigned long offset)
{
        register unsigned int val;
        __asm__ __volatile__(
                        "lwzx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return val;
}

static __inline__ unsigned int
xf86ReadMmio32Le(__volatile__ void *base, const unsigned long offset)
{
        register unsigned int val;
        __asm__ __volatile__(
                        "lwbrx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return val;
}

static __inline__ void
xf86WriteMmioNB8(__volatile__ void *base, const unsigned long offset,
		 const unsigned char val)
{
        __asm__ __volatile__(
                        "stbx %1,%2,%3\n\t"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}

static __inline__ void
xf86WriteMmioNB16Le(__volatile__ void *base, const unsigned long offset,
		    const unsigned short val)
{
        __asm__ __volatile__(
                        "sthbrx %1,%2,%3\n\t"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}

static __inline__ void
xf86WriteMmioNB16Be(__volatile__ void *base, const unsigned long offset,
		    const unsigned short val)
{
        __asm__ __volatile__(
                        "sthx %1,%2,%3\n\t"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}

static __inline__ void
xf86WriteMmioNB32Le(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
        __asm__ __volatile__(
                        "stwbrx %1,%2,%3\n\t"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}

static __inline__ void
xf86WriteMmioNB32Be(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
        __asm__ __volatile__(
                        "stwx %1,%2,%3\n\t"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}

static __inline__ void
xf86WriteMmio8(__volatile__ void *base, const unsigned long offset,
               const unsigned char val)
{
        xf86WriteMmioNB8(base, offset, val);
        eieio();
}

static __inline__ void
xf86WriteMmio16Le(__volatile__ void *base, const unsigned long offset,
                  const unsigned short val)
{
        xf86WriteMmioNB16Le(base, offset, val);
        eieio();
}

static __inline__ void
xf86WriteMmio16Be(__volatile__ void *base, const unsigned long offset,
                  const unsigned short val)
{
        xf86WriteMmioNB16Be(base, offset, val);
        eieio();
}

static __inline__ void
xf86WriteMmio32Le(__volatile__ void *base, const unsigned long offset,
                  const unsigned int val)
{
        xf86WriteMmioNB32Le(base, offset, val);
        eieio();
}

static __inline__ void
xf86WriteMmio32Be(__volatile__ void *base, const unsigned long offset,
                  const unsigned int val)
{
        xf86WriteMmioNB32Be(base, offset, val);
        eieio();
}


static __inline__ void
outb(unsigned short port, unsigned char value)
{
        if(ioBase == MAP_FAILED) return;
        xf86WriteMmio8((void *)ioBase, port, value);
}

static __inline__ void
outw(unsigned short port, unsigned short value)
{
        if(ioBase == MAP_FAILED) return;
        xf86WriteMmio16Le((void *)ioBase, port, value);
}

static __inline__ void
outl(unsigned short port, unsigned int value)
{
        if(ioBase == MAP_FAILED) return;
        xf86WriteMmio32Le((void *)ioBase, port, value);
}

static __inline__ unsigned int
inb(unsigned short port)
{
        if(ioBase == MAP_FAILED) return 0;
        return xf86ReadMmio8((void *)ioBase, port);
}

static __inline__ unsigned int
inw(unsigned short port)
{
        if(ioBase == MAP_FAILED) return 0;
        return xf86ReadMmio16Le((void *)ioBase, port);
}

static __inline__ unsigned int
inl(unsigned short port)
{
        if(ioBase == MAP_FAILED) return 0;
        return xf86ReadMmio32Le((void *)ioBase, port);
}

#elif defined(__arm__) && defined(__linux__)

/* for Linux on ARM, we use the LIBC inx/outx routines */
/* note that the appropriate setup via "ioperm" needs to be done */
/*  *before* any inx/outx is done. */

#include <sys/io.h>

static __inline__ void
xf_outb(unsigned short port, unsigned char val)
{
    outb(val, port);
}

static __inline__ void
xf_outw(unsigned short port, unsigned short val)
{
    outw(val, port);
}

static __inline__ void
xf_outl(unsigned short port, unsigned int val)
{
    outl(val, port);
}

#define outb xf_outb
#define outw xf_outw
#define outl xf_outl

#   elif defined(__nds32__)

/*
 * Assume all port access are aligned.  We need to revise this implementation
 * if there is unaligned port access.  For ldq_u, ldl_u, ldw_u, stq_u, stl_u and
 * stw_u, they are assumed unaligned.
 */

#define barrier()		/* no barrier */

#define PORT_SIZE long

static __inline__ unsigned char
xf86ReadMmio8(__volatile__ void *base, const unsigned long offset)
{
	return *(volatile unsigned char *)((unsigned char *)base + offset) ;
}

static __inline__ void
xf86WriteMmio8(__volatile__ void *base, const unsigned long offset,
	       const unsigned int val)
{
	*(volatile unsigned char *)((unsigned char *)base + offset) = val ;
	barrier();
}

static __inline__ void
xf86WriteMmio8NB(__volatile__ void *base, const unsigned long offset,
		 const unsigned int val)
{
	*(volatile unsigned char *)((unsigned char *)base + offset) = val ;
}

static __inline__ unsigned short
xf86ReadMmio16Swap(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned short ret;

	__asm__ __volatile__(
	           "lhi %0, [%1];\n\t"
	           "wsbh %0, %0;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned short
xf86ReadMmio16(__volatile__ void *base, const unsigned long offset)
{
	return *(volatile unsigned short *)((char *)base + offset) ;
}

static __inline__ void
xf86WriteMmio16Swap(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
	           "shi %0, [%1];\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
	barrier();
}

static __inline__ void
xf86WriteMmio16(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	*(volatile unsigned short *)((unsigned char *)base + offset) = val ;
	barrier();
}

static __inline__ void
xf86WriteMmio16SwapNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
	           "shi %0, [%1];\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void
xf86WriteMmio16NB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	*(volatile unsigned short *)((unsigned char *)base + offset) = val ;
}

static __inline__ unsigned int
xf86ReadMmio32Swap(__volatile__ void *base, const unsigned long offset)
{
	unsigned long addr = ((unsigned long)base) + offset;
	unsigned int ret;

	__asm__ __volatile__(
	           "lwi %0, [%1];\n\t"
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned int
xf86ReadMmio32(__volatile__ void *base, const unsigned long offset)
{
	return *(volatile unsigned int *)((unsigned char *)base + offset) ;
}

static __inline__ void
xf86WriteMmio32Swap(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
	           "rotri %0, %0, 16;\n\t"
				  "swi %0, [%1];\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
	barrier();
}

static __inline__ void
xf86WriteMmio32(__volatile__ void *base, const unsigned long offset,
		  const unsigned int val)
{
	*(volatile unsigned int *)((unsigned char *)base + offset) = val ;
	barrier();
}

static __inline__ void
xf86WriteMmio32SwapNB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	unsigned long addr = ((unsigned long)base) + offset;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
				  "swi %0, [%1];\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void
xf86WriteMmio32NB(__volatile__ void *base, const unsigned long offset,
		    const unsigned int val)
{
	*(volatile unsigned int *)((unsigned char *)base + offset) = val ;
}

#    if defined(NDS32_MMIO_SWAP)
static __inline__ void
outb(unsigned PORT_SIZE port, unsigned char val)
{
   xf86WriteMmio8(IOPortBase, port, val);
}

static __inline__ void
outw(unsigned PORT_SIZE port, unsigned short val)
{
   xf86WriteMmio16Swap(IOPortBase, port, val);
}

static __inline__ void
outl(unsigned PORT_SIZE port, unsigned int val)
{
   xf86WriteMmio32Swap(IOPortBase, port, val);
}

static __inline__ unsigned int
inb(unsigned PORT_SIZE port)
{
   return xf86ReadMmio8(IOPortBase, port);
}

static __inline__ unsigned int
inw(unsigned PORT_SIZE port)
{
   return xf86ReadMmio16Swap(IOPortBase, port);
}

static __inline__ unsigned int
inl(unsigned PORT_SIZE port)
{
   return xf86ReadMmio32Swap(IOPortBase, port);
}

static __inline__ unsigned long ldq_u(unsigned long *p)
{
	unsigned long addr = (unsigned long)p;
	unsigned int ret;

	__asm__ __volatile__(
				  "lmw.bi %0, [%1], %0, 0;\n\t"
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned long ldl_u(unsigned int *p)
{
	unsigned long addr = (unsigned long)p;
	unsigned int ret;

	__asm__ __volatile__(
				  "lmw.bi %0, [%1], %0, 0;\n\t"
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ void stq_u(unsigned long val, unsigned long *p)
{
	unsigned long addr = (unsigned long)p;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
				  "smw.bi %0, [%1], %0, 0;\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void stl_u(unsigned long val, unsigned int *p)
{
	unsigned long addr = (unsigned long)p;

	__asm__ __volatile__(
	           "wsbh %0, %0;\n\t"
				  "rotri %0, %0, 16;\n\t"
				  "smw.bi %0, [%1], %0, 0;\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

#    else /* !NDS32_MMIO_SWAP */
static __inline__ void
outb(unsigned PORT_SIZE port, unsigned char val)
{
	*(volatile unsigned char*)(((unsigned PORT_SIZE)(port))) = val;
	barrier();
}

static __inline__ void
outw(unsigned PORT_SIZE port, unsigned short val)
{
	*(volatile unsigned short*)(((unsigned PORT_SIZE)(port))) = val;
	barrier();
}

static __inline__ void
outl(unsigned PORT_SIZE port, unsigned int val)
{
	*(volatile unsigned int*)(((unsigned PORT_SIZE)(port))) = val;
	barrier();
}
static __inline__ unsigned int
inb(unsigned PORT_SIZE port)
{
	return *(volatile unsigned char*)(((unsigned PORT_SIZE)(port)));
}

static __inline__ unsigned int
inw(unsigned PORT_SIZE port)
{
	return *(volatile unsigned short*)(((unsigned PORT_SIZE)(port)));
}

static __inline__ unsigned int
inl(unsigned PORT_SIZE port)
{
	return *(volatile unsigned int*)(((unsigned PORT_SIZE)(port)));
}

static __inline__ unsigned long ldq_u(unsigned long *p)
{
	unsigned long addr = (unsigned long)p;
	unsigned int ret;

	__asm__ __volatile__(
				  "lmw.bi %0, [%1], %0, 0;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}

static __inline__ unsigned long ldl_u(unsigned int *p)
{
	unsigned long addr = (unsigned long)p;
	unsigned int ret;

	__asm__ __volatile__(
				  "lmw.bi %0, [%1], %0, 0;\n\t"
			     : "=r" (ret)
			     : "r" (addr));
	return ret;
}


static __inline__ void stq_u(unsigned long val, unsigned long *p)
{
	unsigned long addr = (unsigned long)p;

	__asm__ __volatile__(
				  "smw.bi %0, [%1], %0, 0;\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}

static __inline__ void stl_u(unsigned long val, unsigned int *p)
{
	unsigned long addr = (unsigned long)p;

	__asm__ __volatile__(
				  "smw.bi %0, [%1], %0, 0;\n\t"
			     : /* No outputs */
			     : "r" (val), "r" (addr));
}
#    endif /* NDS32_MMIO_SWAP */

#    if (((X_BYTE_ORDER == X_BIG_ENDIAN) && !defined(NDS32_MMIO_SWAP)) || ((X_BYTE_ORDER != X_BIG_ENDIAN) && defined(NDS32_MMIO_SWAP)))
#    define ldw_u(p)	((*(unsigned char *)(p)) << 8 | \
			(*((unsigned char *)(p)+1)))
#    define stw_u(v,p)	(*(unsigned char *)(p)) = ((v) >> 8); \
				(*((unsigned char *)(p)+1)) = (v)
#    else
#    define ldw_u(p)	((*(unsigned char *)(p)) | \
			(*((unsigned char *)(p)+1)<<8))
#    define stw_u(v,p)	(*(unsigned char *)(p)) = (v); \
				(*((unsigned char *)(p)+1)) = ((v) >> 8)
#    endif

#    define mem_barrier()         /* XXX: nop for now */
#    define write_mem_barrier()   /* XXX: nop for now */

#   else /* ix86 */

#    if !defined(__SUNPRO_C)
#    if !defined(FAKEIT) && !defined(__mc68000__) && !defined(__arm__) && !defined(__sh__) && !defined(__hppa__) && !defined(__s390__) && !defined(__m32r__)
#     ifdef GCCUSESGAS

/*
 * If gcc uses gas rather than the native assembler, the syntax of these
 * inlines has to be different.		DHD
 */

static __inline__ void
outb(unsigned short port, unsigned char val)
{
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
}


static __inline__ void
outw(unsigned short port, unsigned short val)
{
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
outl(unsigned short port, unsigned int val)
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(unsigned short port)
{
   unsigned char ret;
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inw(unsigned short port)
{
   unsigned short ret;
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inl(unsigned short port)
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

#     else	/* GCCUSESGAS */

static __inline__ void
outb(unsigned short port, unsigned char val)
{
  __asm__ __volatile__("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outw(unsigned short port, unsigned short val)
{
  __asm__ __volatile__("out%W0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outl(unsigned short port, unsigned int val)
{
  __asm__ __volatile__("out%L0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(unsigned short port)
{
  unsigned char ret;
  __asm__ __volatile__("in%B0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
inw(unsigned short port)
{
  unsigned short ret;
  __asm__ __volatile__("in%W0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
inl(unsigned short port)
{
  unsigned int ret;
  __asm__ __volatile__("in%L0 (%1)" :
                   "=a" (ret) :
                   "d" (port));
  return ret;
}

#     endif /* GCCUSESGAS */

#    else /* !defined(FAKEIT) && !defined(__mc68000__)  && !defined(__arm__) && !defined(__sh__) && !defined(__hppa__) && !defined(__m32r__) */

static __inline__ void
outb(unsigned short port, unsigned char val)
{
}

static __inline__ void
outw(unsigned short port, unsigned short val)
{
}

static __inline__ void
outl(unsigned short port, unsigned int val)
{
}

static __inline__ unsigned int
inb(unsigned short port)
{
  return 0;
}

static __inline__ unsigned int
inw(unsigned short port)
{
  return 0;
}

static __inline__ unsigned int
inl(unsigned short port)
{
  return 0;
}

#    endif /* FAKEIT */
#    endif /* __SUNPRO_C */

#   endif /* ix86 */

#  else /* !GNUC */
#    if defined(__STDC__) && (__STDC__ == 1)
#     ifndef asm
#      define asm __asm
#     endif
#    endif
#     if !defined(__SUNPRO_C)
#      include <sys/inline.h>
#     endif
#    if !defined(__HIGHC__) && !defined(__SUNPRO_C) || \
	defined(__USLC__)
#     pragma asm partial_optimization outl
#     pragma asm partial_optimization outw
#     pragma asm partial_optimization outb
#     pragma asm partial_optimization inl
#     pragma asm partial_optimization inw
#     pragma asm partial_optimization inb
#    endif
#  endif /* __GNUC__ */

# endif /* NO_INLINE */

# ifdef __alpha__
/* entry points for Mmio memory access routines */
extern _X_EXPORT int (*xf86ReadMmio8)(void *, unsigned long);
extern _X_EXPORT int (*xf86ReadMmio16)(void *, unsigned long);
#  ifndef STANDALONE_MMIO
extern _X_EXPORT int (*xf86ReadMmio32)(void *, unsigned long);
#  else
/* Some DRI 3D drivers need MMIO_IN32. */
static __inline__ int
xf86ReadMmio32(void *Base, unsigned long Offset)
{
	mem_barrier();
	return *(volatile unsigned int*)((unsigned long)Base+(Offset));
}
#  endif
extern _X_EXPORT void (*xf86WriteMmio8)(int, void *, unsigned long);
extern _X_EXPORT void (*xf86WriteMmio16)(int, void *, unsigned long);
extern _X_EXPORT void (*xf86WriteMmio32)(int, void *, unsigned long);
extern _X_EXPORT void (*xf86WriteMmioNB8)(int, void *, unsigned long);
extern _X_EXPORT void (*xf86WriteMmioNB16)(int, void *, unsigned long);
extern _X_EXPORT void (*xf86WriteMmioNB32)(int, void *, unsigned long);
extern _X_EXPORT void xf86SlowBCopyFromBus(unsigned char *, unsigned char *, int);
extern _X_EXPORT void xf86SlowBCopyToBus(unsigned char *, unsigned char *, int);

/* Some macros to hide the system dependencies for MMIO accesses */
/* Changed to kill noise generated by gcc's -Wcast-align */
#  define MMIO_IN8(base, offset) (*xf86ReadMmio8)(base, offset)
#  define MMIO_IN16(base, offset) (*xf86ReadMmio16)(base, offset)
#  ifndef STANDALONE_MMIO
#   define MMIO_IN32(base, offset) (*xf86ReadMmio32)(base, offset)
#  else
#   define MMIO_IN32(base, offset) xf86ReadMmio32(base, offset)
#  endif

#  define MMIO_OUT32(base, offset, val) \
    do { \
	write_mem_barrier(); \
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset)) = (val); \
    } while (0)
#  define MMIO_ONB32(base, offset, val) \
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset)) = (val)

#  define MMIO_OUT8(base, offset, val) \
    (*xf86WriteMmio8)((CARD8)(val), base, offset)
#  define MMIO_OUT16(base, offset, val) \
    (*xf86WriteMmio16)((CARD16)(val), base, offset)
#  define MMIO_ONB8(base, offset, val) \
    (*xf86WriteMmioNB8)((CARD8)(val), base, offset)
#  define MMIO_ONB16(base, offset, val) \
    (*xf86WriteMmioNB16)((CARD16)(val), base, offset)
#  define MMIO_MOVE32(base, offset, val) \
    MMIO_OUT32(base, offset, val)

# elif defined(__powerpc__)  
 /* 
  * we provide byteswapping and no byteswapping functions here
  * with byteswapping as default, 
  * drivers that don't need byteswapping should define PPC_MMIO_IS_BE 
  */
#  define MMIO_IN8(base, offset) xf86ReadMmio8(base, offset)
#  define MMIO_OUT8(base, offset, val) \
    xf86WriteMmio8(base, offset, (CARD8)(val))
#  define MMIO_ONB8(base, offset, val) \
    xf86WriteMmioNB8(base, offset, (CARD8)(val))

#  if defined(PPC_MMIO_IS_BE) /* No byteswapping */
#   define MMIO_IN16(base, offset) xf86ReadMmio16Be(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32Be(base, offset)
#   define MMIO_OUT16(base, offset, val) \
    xf86WriteMmio16Be(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
    xf86WriteMmio32Be(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
    xf86WriteMmioNB16Be(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
    xf86WriteMmioNB32Be(base, offset, (CARD32)(val))
#  else /* byteswapping is the default */
#   define MMIO_IN16(base, offset) xf86ReadMmio16Le(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32Le(base, offset)
#   define MMIO_OUT16(base, offset, val) \
     xf86WriteMmio16Le(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
     xf86WriteMmio32Le(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
     xf86WriteMmioNB16Le(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
     xf86WriteMmioNB32Le(base, offset, (CARD32)(val))
#  endif

#  define MMIO_MOVE32(base, offset, val) \
       xf86WriteMmio32Be(base, offset, (CARD32)(val))

# elif defined(__sparc__) || defined(sparc) || defined(__sparc)
 /*
  * Like powerpc, we provide byteswapping and no byteswapping functions
  * here with byteswapping as default, drivers that don't need byteswapping
  * should define SPARC_MMIO_IS_BE (perhaps create a generic macro so that we
  * do not need to use PPC_MMIO_IS_BE and the sparc one in all the same places
  * of drivers?).
  */
#  define MMIO_IN8(base, offset) xf86ReadMmio8(base, offset)
#  define MMIO_OUT8(base, offset, val) \
    xf86WriteMmio8(base, offset, (CARD8)(val))
#  define MMIO_ONB8(base, offset, val) \
    xf86WriteMmio8NB(base, offset, (CARD8)(val))

#  if defined(SPARC_MMIO_IS_BE) /* No byteswapping */
#   define MMIO_IN16(base, offset) xf86ReadMmio16Be(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32Be(base, offset)
#   define MMIO_OUT16(base, offset, val) \
     xf86WriteMmio16Be(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
     xf86WriteMmio32Be(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
     xf86WriteMmio16BeNB(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
     xf86WriteMmio32BeNB(base, offset, (CARD32)(val))
#  else /* byteswapping is the default */
#   define MMIO_IN16(base, offset) xf86ReadMmio16Le(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32Le(base, offset)
#   define MMIO_OUT16(base, offset, val) \
     xf86WriteMmio16Le(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
     xf86WriteMmio32Le(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
     xf86WriteMmio16LeNB(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
     xf86WriteMmio32LeNB(base, offset, (CARD32)(val))
#  endif

#  define MMIO_MOVE32(base, offset, val) \
       xf86WriteMmio32Be(base, offset, (CARD32)(val))

# elif defined(__nds32__)
 /*
  * we provide byteswapping and no byteswapping functions here
  * with no byteswapping as default; when endianness of CPU core
  * and I/O devices don't match, byte swapping is necessary
  * drivers that need byteswapping should define NDS32_MMIO_SWAP
  */
#  define MMIO_IN8(base, offset) xf86ReadMmio8(base, offset)
#  define MMIO_OUT8(base, offset, val) \
    xf86WriteMmio8(base, offset, (CARD8)(val))
#  define MMIO_ONB8(base, offset, val) \
    xf86WriteMmioNB8(base, offset, (CARD8)(val))

#  if defined(NDS32_MMIO_SWAP) /* byteswapping */
#   define MMIO_IN16(base, offset) xf86ReadMmio16Swap(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32Swap(base, offset)
#   define MMIO_OUT16(base, offset, val) \
    xf86WriteMmio16Swap(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
    xf86WriteMmio32Swap(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
    xf86WriteMmioNB16Swap(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
    xf86WriteMmioNB32Swap(base, offset, (CARD32)(val))
#  else /* no byteswapping is the default */
#   define MMIO_IN16(base, offset) xf86ReadMmio16(base, offset)
#   define MMIO_IN32(base, offset) xf86ReadMmio32(base, offset)
#   define MMIO_OUT16(base, offset, val) \
     xf86WriteMmio16(base, offset, (CARD16)(val))
#   define MMIO_OUT32(base, offset, val) \
     xf86WriteMmio32(base, offset, (CARD32)(val))
#   define MMIO_ONB16(base, offset, val) \
     xf86WriteMmioNB16(base, offset, (CARD16)(val))
#   define MMIO_ONB32(base, offset, val) \
     xf86WriteMmioNB32(base, offset, (CARD32)(val))
#  endif

#  define MMIO_MOVE32(base, offset, val) \
       xf86WriteMmio32(base, offset, (CARD32)(val))

#ifdef N1213_HC /* for NDS32 N1213 hardcore */
static __inline__ void nds32_flush_icache(char *addr)
{
	__asm__ volatile (
		"isync %0;"
		"msync;"
		"isb;"
		"cctl %0,L1I_VA_INVAL;"
		"isb;"
		: : "r"(addr) : "memory");
}
#else
static __inline__ void nds32_flush_icache(char *addr)
{
	__asm__ volatile (
		"isync %0;"
		"isb;"
		: : "r"(addr) : "memory");
}
#endif

# else /* !__alpha__ && !__powerpc__ && !__sparc__ */

#  define MMIO_IN8(base, offset) \
	*(volatile CARD8 *)(((CARD8*)(base)) + (offset))
#  define MMIO_IN16(base, offset) \
	*(volatile CARD16 *)(void *)(((CARD8*)(base)) + (offset))
#  define MMIO_IN32(base, offset) \
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset))
#  define MMIO_OUT8(base, offset, val) \
	*(volatile CARD8 *)(((CARD8*)(base)) + (offset)) = (val)
#  define MMIO_OUT16(base, offset, val) \
	*(volatile CARD16 *)(void *)(((CARD8*)(base)) + (offset)) = (val)
#  define MMIO_OUT32(base, offset, val) \
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset)) = (val)
#  define MMIO_ONB8(base, offset, val) MMIO_OUT8(base, offset, val) 
#  define MMIO_ONB16(base, offset, val) MMIO_OUT16(base, offset, val) 
#  define MMIO_ONB32(base, offset, val) MMIO_OUT32(base, offset, val) 

#  define MMIO_MOVE32(base, offset, val) MMIO_OUT32(base, offset, val)

# endif /* __alpha__ */

/*
 * With Intel, the version in os-support/misc/SlowBcopy.s is used.
 * This avoids port I/O during the copy (which causes problems with
 * some hardware).
 */
# ifdef __alpha__
#  define slowbcopy_tobus(src,dst,count) xf86SlowBCopyToBus(src,dst,count)
#  define slowbcopy_frombus(src,dst,count) xf86SlowBCopyFromBus(src,dst,count)
# else /* __alpha__ */
#  define slowbcopy_tobus(src,dst,count) xf86SlowBcopy(src,dst,count)
#  define slowbcopy_frombus(src,dst,count) xf86SlowBcopy(src,dst,count)
# endif /* __alpha__ */

#endif /* _COMPILER_H */
