
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb32.h"
#include "cfb8_32.h"
#include "mi.h"

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

void
cfb8_32FillBoxSolid8(
   DrawablePtr pDraw,
   int nbox,
   BoxPtr pbox,
   unsigned long color
){
    CARD8 *ptr, *data;
    int pitch, height, width, i;
    CARD8 c = (CARD8)color;

    cfbGetByteWidthAndPointer(pDraw, pitch, ptr);
    ptr += 3; /* point to the top byte */

    while(nbox--) {
	data = ptr + (pbox->y1 * pitch) + (pbox->x1 << 2);
	width = (pbox->x2 - pbox->x1) << 2;
	height = pbox->y2 - pbox->y1;

	while(height--) {
            for(i = 0; i < width; i+=4)
		data[i] = c;
            data += pitch;
	}
	pbox++;
    }
}
