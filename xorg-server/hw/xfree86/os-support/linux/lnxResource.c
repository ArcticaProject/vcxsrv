
/* Resource information code */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#include "xf86Resources.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"
#include "lnx.h"

/* Avoid Imakefile changes */
#include "bus/Pci.h"

#ifdef INCLUDE_XF86_NO_DOMAIN

#ifdef __alpha__

resPtr
xf86AccResFromOS(resPtr ret)
{
    resRange range;

    /*
     * Fallback is to claim the following areas:
     *
     * 0x000c0000 - 0x000effff  location of VGA and other extensions ROMS
     */

    RANGE(range, 0x000c0000, 0x000effff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /*
     * Fallback would be to claim well known ports in the 0x0 - 0x3ff range
     * along with their sparse I/O aliases, but that's too imprecise.  Instead
     * claim a bare minimum here.
     */
    RANGE(range, 0x00000000, 0x000000ff, ResExcIoBlock); /* For mainboard */
    ret = xf86AddResToList(ret, &range, -1);

    /*
     * At minimum, the top and bottom resources must be claimed, so that
     * resources that are (or appear to be) unallocated can be relocated.
     */
    RANGE(range, 0x00000000, 0x00000000, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0xffffffff, 0xffffffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
/*  RANGE(range, 0x00000000, 0x00000000, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1); */
    RANGE(range, 0xffffffff, 0xffffffff, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* XXX add others */
    return ret;
}

#elif defined(__powerpc__) || \
      defined(__sparc__) || \
      defined(__mips__) || \
      defined(__sh__) || \
      defined(__mc68000__) || \
      defined(__arm__) || \
      defined(__s390__) || \
      defined(__hppa__)

resPtr
xf86AccResFromOS(resPtr ret)
{
    resRange range;

    /*
     * At minimum, the top and bottom resources must be claimed, so that
     * resources that are (or appear to be) unallocated can be relocated.
     */
    RANGE(range, 0x00000000, 0x00000000, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0xffffffff, 0xffffffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0x00000000, 0x00000000, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
#if defined(__sparc__) || defined(__powerpc__)
    RANGE(range, 0x00ffffff, 0x00ffffff, ResExcIoBlock);
#else
    RANGE(range, 0x0000ffff, 0x0000ffff, ResExcIoBlock);
#endif
    ret = xf86AddResToList(ret, &range, -1);

    return ret;
}

#else

#error : Put your platform dependent code here!!

#endif

#endif /* INCLUDE_XF86_NO_DOMAIN */
