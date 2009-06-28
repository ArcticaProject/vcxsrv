/* COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2000, 2001 Nokia Home Communications

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group */

/* Hardware accelerated drawing for KDrive i810 driver.
   Author: Pontus Lidman <pontus.lidman@nokia.com>
*/

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"
#ifdef XV
#include "kxv.h"
#endif
#include "i810.h"
#include "i810_reg.h"

//#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"dixfontstr.h"
#include	"fb.h"
#include	"migc.h"
#include	"miline.h"
#include	"picturestr.h"

#define NUM_STACK_RECTS	1024

i810ScreenInfo    *accel_i810s;

static int
i810WaitLpRing(i810ScreenInfo *i810s, int n, int timeout_millis)
{
    i810CardInfo *i810c = i810s->i810c;
    I810RingBuffer *ring = &(i810c->LpRing);
    int iters = 0;
    int start = 0;
    int now = 0;
    int last_head = 0;
    int first = 0;
    
    /* If your system hasn't moved the head pointer in 2 seconds, I'm going to
    * call it crashed.
    */
   if (timeout_millis == 0)
      timeout_millis = 2000;

   if (I810_DEBUG) {
      fprintf(stderr, "i810WaitLpRing %d\n", n); 
      first = GetTimeInMillis();
   }

   while (ring->space < n) 
   {
      int i;

      ring->head = INREG(LP_RING + RING_HEAD) & HEAD_ADDR;
      ring->space = ring->head - (ring->tail+8);

      if (ring->space < 0) 
	 ring->space += ring->mem.Size;
      
      iters++;
      now = GetTimeInMillis();
      if ( start == 0 || now < start || ring->head != last_head) {
	 if (I810_DEBUG)
	    if (now > start) 
	       fprintf(stderr, "space: %d wanted %d\n", ring->space, n ); 
	 start = now;
	 last_head = ring->head;
      } else if ( now - start > timeout_millis ) { 

	 i810PrintErrorState(i810c); 
	 fprintf(stderr, "space: %d wanted %d\n", ring->space, n );
	 FatalError("lockup\n"); 
      }

      for (i = 0 ; i < 2000 ; i++)
	 ;
   }

   if (I810_DEBUG)
   {
      now = GetTimeInMillis();
      if (now - first) {
	 fprintf(stderr,"Elapsed %d ms\n", now - first);
	 fprintf(stderr, "space: %d wanted %d\n", ring->space, n );
      }
   }

   return iters;
}

static void
i810Sync(i810ScreenInfo *i810s) 
{
    i810CardInfo *i810c = i810s->i810c;
    LP_RING_LOCALS;

    if (I810_DEBUG)
	fprintf(stderr, "i810Sync\n");
   
   /* Send a flush instruction and then wait till the ring is empty.
    * This is stronger than waiting for the blitter to finish as it also
    * flushes the internal graphics caches.
    */
    BEGIN_LP_RING(2);
    OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE );
    OUT_RING( 0 );		/* pad to quadword */
    ADVANCE_LP_RING();

    i810WaitLpRing(i810s, i810c->LpRing.mem.Size - 8, 0);

    i810c->LpRing.space = i810c->LpRing.mem.Size - 8;
    i810c->nextColorExpandBuf = 0;
}

static void
i810WaitMarker(ScreenPtr pScreen, int marker)
{
    KdScreenPriv(pScreen);
    i810ScreenInfo(pScreenPriv);

    i810Sync(i810s);
}

#if 0
static void
i810EmitInvarientState(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    i810CardInfo(pScreenPriv);
    i810ScreenInfo(pScreenPriv);
    LP_RING_LOCALS;

    BEGIN_LP_RING( 10 );

    OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE );
    OUT_RING( GFX_CMD_CONTEXT_SEL | CS_UPDATE_USE | CS_USE_CTX0 );
    OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE);
    OUT_RING( 0 );


    OUT_RING( GFX_OP_COLOR_CHROMA_KEY );
    OUT_RING( CC1_UPDATE_KILL_WRITE | 
              CC1_DISABLE_KILL_WRITE | 
              CC1_UPDATE_COLOR_IDX |
              CC1_UPDATE_CHROMA_LOW |
              CC1_UPDATE_CHROMA_HI |
              0);
    OUT_RING( 0 );
    OUT_RING( 0 );

    /* No depth buffer in KDrive yet */
    /*     OUT_RING( CMD_OP_Z_BUFFER_INFO ); */
    /*     OUT_RING( pI810->DepthBuffer.Start | pI810->auxPitchBits); */

    ADVANCE_LP_RING();
}
#endif

static unsigned int i810PatternRop[16] = {
    0x00, /* GXclear      */
    0xA0, /* GXand        */
    0x50, /* GXandReverse */
    0xF0, /* GXcopy       */
    0x0A, /* GXandInvert  */
    0xAA, /* GXnoop       */
    0x5A, /* GXxor        */
    0xFA, /* GXor         */
    0x05, /* GXnor        */
    0xA5, /* GXequiv      */
    0x55, /* GXinvert     */
    0xF5, /* GXorReverse  */
    0x0F, /* GXcopyInvert */
    0xAF, /* GXorInverted */
    0x5F, /* GXnand       */
    0xFF  /* GXset        */
};

static Bool
i810PrepareSolid(PixmapPtr pPix, int alu, Pixel pm, Pixel fg)
{
    KdScreenPriv(pPix->drawable.pScreen);
    i810ScreenInfo(pScreenPriv);
    i810CardInfo(pScreenPriv);

    if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "i810PrepareSolid color: %x rop: %x mask: %x\n", 
	      fg, alu, pm);

    /* Color blit, p166 */
    i810c->BR[13] = BR13_SOLID_PATTERN | 
		    (i810PatternRop[alu] << 16) |
		    (pPix->drawable.pScreen->width * i810c->cpp);
    i810c->BR[16] = fg;

    accel_i810s = i810s;

    return TRUE;
}

static void
i810Solid(int x1, int y1, int x2, int y2)
{
    I810ScreenInfo *i810s = accel_i810s;
    I810CardInfo *i810c = i810s->i810c;
    LP_RING_LOCALS;

    if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
	ErrorF( "i810SubsequentFillRectSolid %d,%d %d,%d\n", x1, y1, x2, y2);

    BEGIN_LP_RING(6);

    OUT_RING( BR00_BITBLT_CLIENT | BR00_OP_COLOR_BLT | 0x3 );
    OUT_RING( i810c->BR[13] );
    OUT_RING( ((y2 - y1) << 16) | ((x2 - x1) * i810c->cpp));
    OUT_RING( i810c->bufferOffset + y1 * i810s->pitch + x1 * i810c->cpp );

    OUT_RING( i810c->BR[16]);
    OUT_RING( 0 );		/* pad to quadword */

    ADVANCE_LP_RING();
}

static void
i810DoneSolid(void)
{
}

static Bool
i810PrepareCopy(PixmapPtr pSrc, PixmapPtr pDst, int dx, int dy, int alu, Pixel pm)
{
	return FALSE;
}

static void 
i810RefreshRing(i810CardInfo *i810c)
{
    i810c->LpRing.head = INREG(LP_RING + RING_HEAD) & HEAD_ADDR;
    i810c->LpRing.tail = INREG(LP_RING + RING_TAIL);
    i810c->LpRing.space = i810c->LpRing.head - (i810c->LpRing.tail+8);
    if (i810c->LpRing.space < 0) 
	i810c->LpRing.space += i810c->LpRing.mem.Size;
}


static void
i810SetRingRegs(i810CardInfo *i810c)
{
   unsigned int itemp;

   OUTREG(LP_RING + RING_TAIL, 0 );
   OUTREG(LP_RING + RING_HEAD, 0 );

   itemp = INREG(LP_RING + RING_START);
   itemp &= ~(START_ADDR);
   itemp |= i810c->LpRing.mem.Start;
   OUTREG(LP_RING + RING_START, itemp );

   itemp = INREG(LP_RING + RING_LEN);
   itemp &= ~(RING_NR_PAGES | RING_REPORT_MASK | RING_VALID_MASK);
   itemp |= ((i810c->LpRing.mem.Size-4096) | RING_NO_REPORT | RING_VALID);
   OUTREG(LP_RING + RING_LEN, itemp );
}

Bool
i810InitAccel(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    i810ScreenInfo(pScreenPriv);
    i810CardInfo(pScreenPriv);

    memset(&i810s->kaa, 0, sizeof(KaaScreenInfoRec));
    i810s->kaa.waitMarker	= i810WaitMarker;
    i810s->kaa.PrepareSolid	= i810PrepareSolid;
    i810s->kaa.Solid		= i810Solid;
    i810s->kaa.DoneSolid	= i810DoneSolid;
    i810s->kaa.PrepareCopy	= i810PrepareCopy;
    i810s->kaa.Copy		= NULL;
    i810s->kaa.DoneCopy		= NULL;

    i810s->pitch = pScreen->width * i810c->cpp;

    return FALSE;
}

void
i810EnableAccel(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    i810CardInfo(pScreenPriv);

    if (i810c->LpRing.mem.Size == 0) {
        ErrorF("No memory for LpRing!! Acceleration not functional!!\n");
    }

    i810SetRingRegs(i810c);

    kaaMarkSync (pScreen);
}


void
i810DisableAccel(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    i810CardInfo(pScreenPriv);
    i810ScreenInfo(pScreenPriv);
    
    i810RefreshRing(i810c);
    i810Sync(i810s);
}

void
i810FiniAccel(ScreenPtr pScreen)
{
}
