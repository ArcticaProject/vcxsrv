/*******************************************************************************
  for Alpha Linux
*******************************************************************************/
 
/* 
 *   Create a dependency that should be immune from the effect of register
 *   renaming as is commonly seen in superscalar processors.  This should
 *   insert a minimum of 100-ns delays between reads/writes at clock rates
 *   up to 100 MHz---GGL
 *   
 *   Slowbcopy(char *src, char *dst, int count)   
 *   
 */ 

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "compiler.h"

static int really_slow_bcopy;

_X_EXPORT void
xf86SetReallySlowBcopy(void)
{
	really_slow_bcopy = 1;
}

#if defined(__i386__) || defined(__x86_64__)
static void xf86_really_slow_bcopy(unsigned char *src, unsigned char *dst, int len)
{
    while(len--)
    {
	*dst++ = *src++;
	outb(0x80, 0x00);
    }
}
#endif

/* The outb() isn't needed on my machine, but who knows ... -- ost */
_X_EXPORT void
xf86SlowBcopy(unsigned char *src, unsigned char *dst, int len)
{
#if defined(__i386__) || defined(__x86_64__)
    if (really_slow_bcopy) {
	xf86_really_slow_bcopy(src, dst, len);
	return;
    }
#endif
    while(len--)
	*dst++ = *src++;
}

#ifdef __alpha__
/*
 * The Jensen lacks dense memory, thus we have to address the bus via
 * the sparse addressing scheme. Time critical code uses routines from
 * BUSmemcpy.c
 *
 * Martin Ostermann (ost@comnets.rwth-aachen.de) - Apr.-Sep. 1996
 */

#ifdef linux

unsigned long _bus_base(void);

#ifdef TEST_JENSEN_CODE /* define to test the Sparse addressing on a non-Jensen */
#define SPARSE (5)
#else
#define SPARSE (7)
#endif

#define isJensen() (!_bus_base())

#else

#define isJensen() 0
#define SPARSE 0

#endif

_X_EXPORT void
xf86SlowBCopyFromBus(unsigned char *src, unsigned char *dst, int count)
{
    if (isJensen())
    {
	unsigned long addr;
	long result;

	addr = (unsigned long) src;
	while( count ){
	    result = *(volatile int *) addr;
	    result >>= ((addr>>SPARSE) & 3) * 8;
	    *dst++ = (unsigned char) (0xffUL & result);
	    addr += 1<<SPARSE;
	    count--;
	    outb(0x80, 0x00);
	}
    }
    else
	xf86SlowBcopy(src,dst,count);
}
  
_X_EXPORT void
xf86SlowBCopyToBus(unsigned char *src, unsigned char *dst, int count)
{
    if (isJensen())
    {
	unsigned long addr;

	addr = (unsigned long) dst;
	while(count) {
	    *(volatile unsigned int *) addr = (unsigned short)(*src) * 0x01010101;
	    src++;
	    addr += 1<<SPARSE;
	    count--;
	    outb(0x80, 0x00);
	}
    }
    else
	xf86SlowBcopy(src,dst,count);    
}
#endif
