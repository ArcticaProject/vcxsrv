#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include <time.h>

_X_EXPORT void
xf86UDelay(long usec)
{
#if 0
    struct timeval start, interrupt;
#else
    int sigio;

    sigio = xf86BlockSIGIO();
    usleep(usec);
    xf86UnblockSIGIO(sigio);
#endif

#if 0
    gettimeofday(&start,NULL);

    do {
	usleep(usec);
	gettimeofday(&interrupt,NULL);
	
	if ((usec = usec - (interrupt.tv_sec - start.tv_sec) * 1000000
	      - (interrupt.tv_usec - start.tv_usec)) < 0)
	    break;
	start = interrupt;
    } while (1);
#endif
}

