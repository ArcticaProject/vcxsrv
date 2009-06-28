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

/*
 * i810.c - KDrive driver for the i810 chipset
 *
 * Authors:
 *   Pontus Lidman  <pontus.lidman@nokia.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kxv.h"
#include "klinux.h"

#include "i810.h"
#include "agp.h"

#include "i810draw.h"

#ifndef I810_DEBUG
int I810_DEBUG = (0
/*      		  | DEBUG_ALWAYS_SYNC   */
/*     		  | DEBUG_VERBOSE_ACCEL   */
/*   		  | DEBUG_VERBOSE_SYNC  */
/*  		  | DEBUG_VERBOSE_VGA */
/*   		  | DEBUG_VERBOSE_RING     */
/*  		  | DEBUG_VERBOSE_OUTREG  */
/*  		  | DEBUG_VERBOSE_MEMORY */
/*  		  | DEBUG_VERBOSE_CURSOR  */
   );
#endif


static Bool
i810ModeInit(KdScreenInfo *screen, const KdMonitorTiming *t);

static void 
i810PrintMode( vgaRegPtr vgaReg, I810RegPtr mode );

Bool
i810CardInit (KdCardInfo *card)
{
    int i;

    I810CardInfo	*i810c;

/*     fprintf(stderr,"i810CardInit\n"); */

    i810c = (I810CardInfo *) xalloc (sizeof (I810CardInfo));

    if (!i810c)
	return FALSE;

    /* 2MB Video RAM */
    i810c->videoRam=2048;

    /* Find FB address */

    if (card->attr.address[1] != 0) {
        i810c->LinearAddr = card->attr.address[0] & 0xFF000000;

        if (!i810c->LinearAddr) {
            fprintf(stderr,"No valid FB address in PCI config space(1)\n");
            xfree(i810c);
            return FALSE;
        } else {
/*             fprintf(stderr,"Linear framebuffer at %lx\n",i810c->LinearAddr); */
        }
    } else {
        fprintf(stderr,"No valid FB address in PCI config space(2)\n");
        xfree(i810c);
        return FALSE;
    }

    if (card->attr.address[1]) {

        i810c->MMIOAddr = card->attr.address[1] & 0xFFF80000;

        i810c->MMIOBase = 
            KdMapDevice (i810c->MMIOAddr, I810_REG_SIZE);
        if (!i810c->MMIOBase) {
            fprintf(stderr,"No valid MMIO address in PCI config space(1)\n");
            xfree(i810c);
            return FALSE;
        } else {
            
        }
    } else {
        fprintf(stderr,"No valid MMIO address in PCI config space(2)\n");
        xfree(i810c);
        return FALSE;
    }

/*     fprintf(stderr,"Mapped 0x%x bytes of MMIO regs at phys 0x%lx virt %p\n", */
/*             I810_REG_SIZE,i810c->MMIOAddr,i810c->MMIOBase); */

   /* Find out memory bus frequency.
    */

    {
        unsigned long *p;

        if (!(p= (unsigned long *) LinuxGetPciCfg(&card->attr)))
            return FALSE;
        
/*         fprintf(stderr,"Frequency long %lx\n",p[WHTCFG_PAMR_DRP]); */

        if ( (p[WHTCFG_PAMR_DRP] & LM_FREQ_MASK) == LM_FREQ_133 )
            i810c->LmFreqSel = 133;
        else
            i810c->LmFreqSel = 100;

        xfree(p);

/*         fprintf(stderr,"Selected frequency %d\n",i810c->LmFreqSel); */
    }

/*     fprintf(stderr,"Will alloc AGP framebuffer: %d kByte\n",i810c->videoRam); */

    /* Since we always want write combining on first 32 mb of framebuffer
     * we pass a mapsize of 32 mb */
    i810c->FbMapSize = 32*1024*1024;

    for (i = 2 ; i < i810c->FbMapSize ; i <<= 1);
    i810c->FbMapSize = i;

    i810c->FbBase = 
        KdMapDevice (i810c->LinearAddr, i810c->FbMapSize);
    
    if (!i810c->FbBase) return FALSE; 
/*     fprintf(stderr,"Mapped 0x%lx bytes of framebuffer at %p\n", */
/*             i810c->FbMapSize,i810c->FbBase); */
    
    card->driver=i810c;
    
    return TRUE;
}

static void
i810ScreenFini (KdScreenInfo *screen)
{
    I810ScreenInfo    *i810s = (I810ScreenInfo *) screen->driver;
    
    xfree (i810s);
    screen->driver = 0;    
}

static Bool
i810InitScreen (ScreenPtr pScreen) {

#ifdef XV
    i810InitVideo(pScreen);
#endif
    return TRUE;
}

static Bool
i810FinishInitScreen(ScreenPtr pScreen)
{
    /* XXX: RandR init */
    return TRUE;
}

static void
i810CardFini (KdCardInfo *card)
{
    I810CardInfo	*i810c = (I810CardInfo *) card->driver;
    
    KdUnmapDevice (i810c->FbBase, i810c->FbMapSize);
    KdUnmapDevice (i810c->MMIOBase, I810_REG_SIZE);
    xfree (i810c);
    card->driver = 0;
}

struct wm_info {
   double freq;
   unsigned int wm;
};

struct wm_info i810_wm_8_100[] = {
   { 0,    0x22003000 },
   { 25.2, 0x22003000 },
   { 28.0, 0x22003000 },
   { 31.5, 0x22003000 },
   { 36.0, 0x22007000 },
   { 40.0, 0x22007000 },
   { 45.0, 0x22007000 },
   { 49.5, 0x22008000 },
   { 50.0, 0x22008000 },
   { 56.3, 0x22008000 },
   { 65.0, 0x22008000 },
   { 75.0, 0x22008000 },
   { 78.8, 0x22008000 },
   { 80.0, 0x22008000 },
   { 94.0, 0x22008000 },
   { 96.0, 0x22107000 },
   { 99.0, 0x22107000 },
   { 108.0, 0x22107000 },
   { 121.0, 0x22107000 },
   { 128.9, 0x22107000 },
   { 132.0, 0x22109000 },
   { 135.0, 0x22109000 },
   { 157.5, 0x2210b000 },
   { 162.0, 0x2210b000 },
   { 175.5, 0x2210b000 },
   { 189.0, 0x2220e000 },
   { 202.5, 0x2220e000 }
};

struct wm_info i810_wm_16_100[] = {
   { 0, 0x22004000 },
   { 25.2, 0x22006000 },
   { 28.0, 0x22006000 },
   { 31.5, 0x22007000 },
   { 36.0, 0x22007000 },
   { 40.0, 0x22007000 },
   { 45.0, 0x22007000 },
   { 49.5, 0x22009000 },
   { 50.0, 0x22009000 },
   { 56.3, 0x22108000 },
   { 65.0, 0x2210e000 },
   { 75.0, 0x2210e000 },
   { 78.8, 0x2210e000 },
   { 80.0, 0x22210000 },
   { 94.5, 0x22210000 },
   { 96.0, 0x22210000 },
   { 99.0, 0x22210000 },
   { 108.0, 0x22210000 },
   { 121.0, 0x22210000 },
   { 128.9, 0x22210000 },
   { 132.0, 0x22314000 },
   { 135.0, 0x22314000 },
   { 157.5, 0x22415000 },
   { 162.0, 0x22416000 },
   { 175.5, 0x22416000 },
   { 189.0, 0x22416000 },
   { 195.0, 0x22416000 },
   { 202.5, 0x22416000 }
};


struct wm_info i810_wm_24_100[] = {
   { 0,  0x22006000 },
   { 25.2, 0x22009000 },
   { 28.0, 0x22009000 },
   { 31.5, 0x2200a000 },
   { 36.0, 0x2210c000 },
   { 40.0, 0x2210c000 },
   { 45.0, 0x2210c000 },
   { 49.5, 0x22111000 },
   { 50.0, 0x22111000 },
   { 56.3, 0x22111000 },
   { 65.0, 0x22214000 },
   { 75.0, 0x22214000 },
   { 78.8, 0x22215000 },
   { 80.0, 0x22216000 },
   { 94.5, 0x22218000 },
   { 96.0, 0x22418000 },
   { 99.0, 0x22418000 },
   { 108.0, 0x22418000 },
   { 121.0, 0x22418000 },
   { 128.9, 0x22419000 },
   { 132.0, 0x22519000 },
   { 135.0, 0x4441d000 },
   { 157.5, 0x44419000 },
   { 162.0, 0x44419000 },
   { 175.5, 0x44419000 },
   { 189.0, 0x44419000 },
   { 195.0, 0x44419000 },
   { 202.5, 0x44419000 }
};

struct wm_info i810_wm_32_100[] = {
   { 0, 0x2210b000 },
   { 60, 0x22415000 },		/* 0x314000 works too */
   { 80, 0x22419000 }           /* 0x518000 works too */
};


struct wm_info i810_wm_8_133[] = {
  { 0,    0x22003000 },
   { 25.2, 0x22003000 },
   { 28.0, 0x22003000 },
   { 31.5, 0x22003000 },
   { 36.0, 0x22007000 },
   { 40.0, 0x22007000 },
   { 45.0, 0x22007000 },
   { 49.5, 0x22008000 },
   { 50.0, 0x22008000 },
   { 56.3, 0x22008000 },
   { 65.0, 0x22008000 },
   { 75.0, 0x22008000 },
   { 78.8, 0x22008000 },
   { 80.0, 0x22008000 },
   { 94.0, 0x22008000 },
   { 96.0, 0x22107000 },
   { 99.0, 0x22107000 },
   { 108.0, 0x22107000 },
   { 121.0, 0x22107000 },
   { 128.9, 0x22107000 },
   { 132.0, 0x22109000 },
   { 135.0, 0x22109000 },
   { 157.5, 0x2210b000 },
   { 162.0, 0x2210b000 },
   { 175.5, 0x2210b000 },
   { 189.0, 0x2220e000 },
   { 202.5, 0x2220e000 }
};


struct wm_info i810_wm_16_133[] = {
   { 0, 0x22004000 },
   { 25.2, 0x22006000 },
   { 28.0, 0x22006000 },
   { 31.5, 0x22007000 },
   { 36.0, 0x22007000 },
   { 40.0, 0x22007000 },
   { 45.0, 0x22007000 },
   { 49.5, 0x22009000 },
   { 50.0, 0x22009000 },
   { 56.3, 0x22108000 },
   { 65.0, 0x2210e000 },
   { 75.0, 0x2210e000 },
   { 78.8, 0x2210e000 },
   { 80.0, 0x22210000 },
   { 94.5, 0x22210000 },
   { 96.0, 0x22210000 },
   { 99.0, 0x22210000 },
   { 108.0, 0x22210000 },
   { 121.0, 0x22210000 },
   { 128.9, 0x22210000 },
   { 132.0, 0x22314000 },
   { 135.0, 0x22314000 },
   { 157.5, 0x22415000 },
   { 162.0, 0x22416000 },
   { 175.5, 0x22416000 },
   { 189.0, 0x22416000 },
   { 195.0, 0x22416000 },
   { 202.5, 0x22416000 }
};

struct wm_info i810_wm_24_133[] = {
  { 0,  0x22006000 },
   { 25.2, 0x22009000 },
   { 28.0, 0x22009000 },
   { 31.5, 0x2200a000 },
   { 36.0, 0x2210c000 },
   { 40.0, 0x2210c000 },
   { 45.0, 0x2210c000 },
   { 49.5, 0x22111000 },
   { 50.0, 0x22111000 },
   { 56.3, 0x22111000 },
   { 65.0, 0x22214000 },
   { 75.0, 0x22214000 },
   { 78.8, 0x22215000 },
   { 80.0, 0x22216000 },
   { 94.5, 0x22218000 },
   { 96.0, 0x22418000 },
   { 99.0, 0x22418000 },
   { 108.0, 0x22418000 },
   { 121.0, 0x22418000 },
   { 128.9, 0x22419000 },
   { 132.0, 0x22519000 },
   { 135.0, 0x4441d000 },
   { 157.5, 0x44419000 },
   { 162.0, 0x44419000 },
   { 175.5, 0x44419000 },
   { 189.0, 0x44419000 },
   { 195.0, 0x44419000 },
   { 202.5, 0x44419000 }
};

static void
i810WriteControlMMIO(I810CardInfo *i810c, int addr, CARD8 index, CARD8 val) {
  moutb(addr, index);
  moutb(addr+1, val);
}

static CARD8
i810ReadControlMMIO(I810CardInfo *i810c, int addr, CARD8 index) {
  moutb(addr, index);
  return minb(addr+1);
}

static Bool
i810ModeSupported (KdScreenInfo *screen, const KdMonitorTiming *t)
{
    /* This is just a guess. */
    if (t->horizontal > 1600 || t->horizontal < 640) return FALSE;
    if (t->vertical > 1200 || t->horizontal < 350) return FALSE;
    return TRUE;
}

static Bool
i810ModeUsable (KdScreenInfo *screen)
{
    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = (I810CardInfo *) card->driver;
    int		    byte_width, pixel_width, screen_size;

/*     fprintf(stderr,"i810ModeUsable\n"); */
    
    if (screen->fb[0].depth >= 24)
    {
	screen->fb[0].depth = 24;
	screen->fb[0].bitsPerPixel = 24;
	screen->dumb = TRUE;
    }
    else if (screen->fb[0].depth >= 16)
    {
	screen->fb[0].depth = 16;
	screen->fb[0].bitsPerPixel = 16;
    }
    else if (screen->fb[0].depth >= 15)
    {
	screen->fb[0].depth = 15;
	screen->fb[0].bitsPerPixel = 16;
    }
    else
    {
	screen->fb[0].depth = 8;
	screen->fb[0].bitsPerPixel = 8;
    }
    byte_width = screen->width * (screen->fb[0].bitsPerPixel >> 3);
    pixel_width = screen->width;

    screen->fb[0].pixelStride = pixel_width;
    screen->fb[0].byteStride = byte_width;

    screen_size = byte_width * screen->height;

    return screen_size <= (i810c->videoRam * 1024);
}

static int i810AllocateGARTMemory( KdScreenInfo *screen ) 
{
   KdCardInfo	    *card = screen->card;
   I810CardInfo    *i810c = (I810CardInfo *) card->driver;
   unsigned long size = i810c->videoRam * 1024;

   int key;
   long tom = 0;
   unsigned long physical;

   if (!KdAgpGARTSupported()) 
        return FALSE; 

   if (!KdAcquireGART(screen->mynum))
      return FALSE;

   /* This allows the 2d only Xserver to regen */
   i810c->agpAcquired2d = TRUE;
   
   /* Treat the gart like video memory - we assume we own all that is
    * there, so ignore EBUSY errors.  Don't try to remove it on
    * failure, either, as other X server may be using it.
    */

   if ((key = KdAllocateGARTMemory(screen->mynum, size, 0, NULL)) == -1)
      return FALSE;

   i810c->VramOffset = 0;
   i810c->VramKey = key;

   if (!KdBindGARTMemory(screen->mynum, key, 0))
      return FALSE;


   i810c->SysMem.Start = 0;
   i810c->SysMem.Size = size;
   i810c->SysMem.End = size;
   i810c->SavedSysMem = i810c->SysMem;

   tom = i810c->SysMem.End;

   i810c->DcacheMem.Start = 0;
   i810c->DcacheMem.End = 0;
   i810c->DcacheMem.Size = 0;
   i810c->CursorPhysical = 0;

   /* Dcache - half the speed of normal ram, so not really useful for
    * a 2d server.  Don't bother reporting its presence.  This is
    * mapped in addition to the requested amount of system ram.
    */
   size = 1024 * 4096;

   /* Keep it 512K aligned for the sake of tiled regions.
    */
   tom += 0x7ffff;
   tom &= ~0x7ffff;

   if ((key = KdAllocateGARTMemory(screen->mynum, size, AGP_DCACHE_MEMORY, NULL)) != -1) {
      i810c->DcacheOffset= tom;
      i810c->DcacheKey = key;
      if (!KdBindGARTMemory(screen->mynum, key, tom)) {
	 fprintf(stderr,"Allocation of %ld bytes for DCACHE failed\n", size);
	 i810c->DcacheKey = -1;
      }	else {
	 i810c->DcacheMem.Start = tom;
	 i810c->DcacheMem.Size = size;
	 i810c->DcacheMem.End = i810c->DcacheMem.Start + i810c->DcacheMem.Size;
	 tom = i810c->DcacheMem.End;
      }
   } else {
      fprintf(stderr,
		 "No physical memory available for %ld bytes of DCACHE\n",
		 size);
      i810c->DcacheKey = -1;
   }
   
   /* Mouse cursor -- The i810 (crazy) needs a physical address in
    * system memory from which to upload the cursor.  We get this from 
    * the agpgart module using a special memory type.
    */

   /* 4k for the cursor is excessive, I'm going to steal 3k for
    * overlay registers later
    */

   size = 4096;

   if ((key = KdAllocateGARTMemory(screen->mynum, size, AGP_PHYS_MEMORY,
				     &physical)) == -1) {
      fprintf(stderr,
		    "No physical memory available for HW cursor\n");
      i810c->HwcursKey = -1;
   } else {
      i810c->HwcursOffset= tom;
      i810c->HwcursKey = key;
      if (!KdBindGARTMemory(screen->mynum, key, tom)) {
	 fprintf(stderr,
		    "Allocation of %ld bytes for HW cursor failed\n", 
		    size);
	 i810c->HwcursKey = -1;
      }	else {
	 i810c->CursorPhysical = physical;
	 i810c->CursorStart = tom;
	 tom += size;
      }
   }

   /* Overlay register buffer -- Just like the cursor, the i810 needs a
    * physical address in system memory from which to upload the overlay
    * registers.
    */
   if (i810c->CursorStart != 0) {
        i810c->OverlayPhysical = i810c->CursorPhysical + 1024;
        i810c->OverlayStart = i810c->CursorStart + 1024;
   }


   i810c->GttBound = 1;

   return TRUE;
}

/* Allocate from a memrange, returns success */

static int i810AllocLow( I810MemRange *result, I810MemRange *pool, int size )
{
   if (size > pool->Size) return FALSE;

   pool->Size -= size;
   result->Size = size;
   result->Start = pool->Start;
   result->End = pool->Start += size;
   return TRUE;
}

static int i810AllocHigh( I810MemRange *result, I810MemRange *pool, int size )
{
   if (size > pool->Size) return 0;

   pool->Size -= size;
   result->Size = size;
   result->End = pool->End;
   result->Start = pool->End -= size;
   return 1;
}

static Bool
i810AllocateFront(KdScreenInfo *screen) {

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = (I810CardInfo *) card->driver;

   int cache_lines = -1;

   if(i810c->DoneFrontAlloc) 
      return TRUE;
      
   memset(&(i810c->FbMemBox), 0, sizeof(BoxRec));
   /* Alloc FrontBuffer/Ring/Accel memory */
   i810c->FbMemBox.x1=0;
   i810c->FbMemBox.x2=screen->width;
   i810c->FbMemBox.y1=0;
   i810c->FbMemBox.y2=screen->height;

   /* This could be made a command line option */
   cache_lines = 0;

   if(cache_lines >= 0)	
	i810c->FbMemBox.y2 += cache_lines;
   else {
       /* make sure there is enough for two DVD sized YUV buffers */
	i810c->FbMemBox.y2 += (screen->fb[0].depth == 24) ? 256 : 384;
	if (screen->width <= 1024)
	    i810c->FbMemBox.y2 += (screen->fb[0].depth == 24) ? 256 : 384;
	cache_lines = i810c->FbMemBox.y2 - screen->height;
   }

   if (I810_DEBUG)
       ErrorF("Adding %i scanlines for pixmap caching\n", cache_lines);

   /* Reserve room for the framebuffer and pixcache.  Put at the top
    * of memory so we can have nice alignment for the tiled regions at
    * the start of memory.
    */
   i810AllocLow( &(i810c->FrontBuffer), 
		 &(i810c->SysMem), 
		 ((i810c->FbMemBox.x2 * 
		   i810c->FbMemBox.y2 * 
		   i810c->cpp) + 4095) & ~4095);
   
   memset( &(i810c->LpRing), 0, sizeof( I810RingBuffer ) );
   if(i810AllocLow( &(i810c->LpRing.mem), &(i810c->SysMem), 16*4096 )) {
	 if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
	    ErrorF( "ring buffer at local %lx\n", 
		    i810c->LpRing.mem.Start);

	 i810c->LpRing.tail_mask = i810c->LpRing.mem.Size - 1;
	 i810c->LpRing.virtual_start = i810c->FbBase + i810c->LpRing.mem.Start;
	 i810c->LpRing.head = 0;
	 i810c->LpRing.tail = 0;      
	 i810c->LpRing.space = 0;		 
   }
   
   if ( i810AllocLow( &i810c->Scratch, &(i810c->SysMem), 64*1024 ) || 
	i810AllocLow( &i810c->Scratch, &(i810c->SysMem), 16*1024 ) ) {       
       if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
           ErrorF("Allocated Scratch Memory\n");
   }

#ifdef XV
   /* 720x720 is just how much memory the mpeg player needs for overlays */

   if ( i810AllocHigh( &i810c->XvMem, &(i810c->SysMem), 720*720*2 )) {       
       if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
           ErrorF("Allocated overlay Memory\n");
   }
#endif
   
   i810c->DoneFrontAlloc = TRUE;
   return TRUE;
}

static Bool
i810MapMem(KdScreenInfo *screen)
{

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = (I810CardInfo *) card->driver;
    
    i810c->LpRing.virtual_start = i810c->FbBase + i810c->LpRing.mem.Start;
    
    return TRUE;
}


Bool
i810ScreenInit (KdScreenInfo *screen)
{
    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = (I810CardInfo *) card->driver;
    I810ScreenInfo   *i810s;

    int i;

    const KdMonitorTiming *t;

/*     fprintf(stderr,"i810ScreenInit\n"); */

    i810s = (I810ScreenInfo *) xalloc (sizeof (I810ScreenInfo));
    if (!i810s)
	return FALSE;

    memset (i810s, '\0', sizeof (I810ScreenInfo));

    i810s->i810c = i810c;

    /* Default dimensions */
    if (!screen->width || !screen->height)
    {
        screen->width = 720;
        screen->height = 576;
        screen->rate = 52;
#if 0
        screen->width = 1024;
        screen->height = 768;
        screen->rate = 72;
#endif
    }

    if (!screen->fb[0].depth)
	screen->fb[0].depth = 16;
    
    t = KdFindMode (screen, i810ModeSupported);
    
    screen->rate = t->rate;
    screen->width = t->horizontal;
    screen->height = t->vertical;
    
    if (!KdTuneMode (screen, i810ModeUsable, i810ModeSupported))
    {
	xfree (i810c);
	return FALSE;
    }

/*     fprintf(stderr,"Screen rate %d horiz %d vert %d\n",t->rate,t->horizontal,t->vertical); */

    switch (screen->fb[0].depth) {
    case 8:
	screen->fb[0].visuals = ((1 << StaticGray) |
			   (1 << GrayScale) |
			   (1 << StaticColor) |
			   (1 << PseudoColor) |
			   (1 << TrueColor) |
			   (1 << DirectColor));
	screen->fb[0].blueMask  = 0x00;
	screen->fb[0].greenMask = 0x00;
	screen->fb[0].redMask   = 0x00;
	break;
    case 15:
	screen->fb[0].visuals = (1 << TrueColor);
	screen->fb[0].blueMask  = 0x001f;
	screen->fb[0].greenMask = 0x03e0;
	screen->fb[0].redMask   = 0x7c00;

        i810c->colorKey = 0x043f;

	break;
    case 16:
	screen->fb[0].visuals = (1 << TrueColor);
	screen->fb[0].blueMask  = 0x001f;
	screen->fb[0].greenMask = 0x07e0;
	screen->fb[0].redMask   = 0xf800;

        i810c->colorKey = 0x083f;

	break;
    case 24:
	screen->fb[0].visuals = (1 << TrueColor);
	screen->fb[0].blueMask  = 0x0000ff;
	screen->fb[0].greenMask = 0x00ff00;
	screen->fb[0].redMask   = 0xff0000;

        i810c->colorKey = 0x0101ff;
        
	break;
    default:
        fprintf(stderr,"Unsupported depth %d\n",screen->fb[0].depth);
        return FALSE;
    }



    /* Set all colours to black */
    for (i=0; i<768; i++) i810c->vga.ModeReg.DAC[i] = 0x00;

    /* ... and the overscan */
    if (screen->fb[0].depth >= 4)
        i810c->vga.ModeReg.Attribute[OVERSCAN] = 0xFF;

    /* Could be made a command-line option */

#ifdef I810CFG_SHOW_OVERSCAN
	i810c->vga.ModeReg.DAC[765] = 0x3F; 
	i810c->vga.ModeReg.DAC[766] = 0x00; 
	i810c->vga.ModeReg.DAC[767] = 0x3F; 
	i810c->vga.ModeReg.Attribute[OVERSCAN] = 0xFF;
	i810c->vga.ShowOverscan = TRUE;
#else
	i810c->vga.ShowOverscan = FALSE;
#endif

    i810c->vga.paletteEnabled = FALSE;
    i810c->vga.cmapSaved = FALSE;
    i810c->vga.MMIOBase		= i810c->MMIOBase;

    i810c->cpp = screen->fb[0].bitsPerPixel/8;

    /* move to initscreen? */
    
    switch (screen->fb[0].bitsPerPixel) {
    case 8:
        i810c->MaxClock = 203000;
        break;
    case 16:
        i810c->MaxClock = 163000;
        break;
    case 24:
        i810c->MaxClock = 136000;
        break;
    case 32:  /* not supported */
        i810c->MaxClock = 86000;
    default:
        fprintf(stderr,"Unsupported bpp %d\n",screen->fb[0].bitsPerPixel);
        return FALSE;
    }

   if (!i810AllocateGARTMemory( screen )) {
       return FALSE;
   }

   i810AllocateFront(screen);

   /* Map LpRing memory */
   if (!i810MapMem(screen)) return FALSE;

   screen->fb[0].frameBuffer = i810c->FbBase;

   screen->driver = i810s;
   
   return TRUE;
}
   
/*
 * I810Save --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaI810Rec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void
DoSave(KdCardInfo *card, vgaRegPtr vgaReg, I810RegPtr i810Reg, Bool saveFonts)
{

    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;

    int i;

    /* Save VGA registers */

    vgaReg->MiscOutReg = mmioReadMiscOut(vgap);
    if (vgaReg->MiscOutReg & 0x01)
	vgap->IOBase = VGA_IOBASE_COLOR;
    else
	vgap->IOBase = VGA_IOBASE_MONO;

    for (i = 0; i < VGA_NUM_CRTC; i++) {
	vgaReg->CRTC[i] = mmioReadCrtc(vgap, i);
    }

    mmioEnablePalette(vgap);
    for (i = 0; i < VGA_NUM_ATTR; i++) {
	vgaReg->Attribute[i] = mmioReadAttr(vgap, i);
    }
    mmioDisablePalette(vgap);

    for (i = 0; i < VGA_NUM_GFX; i++) {
	vgaReg->Graphics[i] = mmioReadGr(vgap, i);
    }

    for (i = 1; i < VGA_NUM_SEQ; i++) {
	vgaReg->Sequencer[i] = mmioReadSeq(vgap, i);
    }

    /*
     * The port I/O code necessary to read in the extended registers 
     * into the fields of the I810Rec structure goes here.
     */
    i810Reg->IOControl = mmioReadCrtc(vgap, IO_CTNL);
    i810Reg->AddressMapping = i810ReadControlMMIO(i810c, GRX, ADDRESS_MAPPING);
    i810Reg->BitBLTControl = INREG8(BITBLT_CNTL);
    i810Reg->VideoClk2_M = INREG16(VCLK2_VCO_M);
    i810Reg->VideoClk2_N = INREG16(VCLK2_VCO_N);
    i810Reg->VideoClk2_DivisorSel = INREG8(VCLK2_VCO_DIV_SEL);

    i810Reg->ExtVertTotal=mmioReadCrtc(vgap, EXT_VERT_TOTAL);
    i810Reg->ExtVertDispEnd=mmioReadCrtc(vgap, EXT_VERT_DISPLAY);
    i810Reg->ExtVertSyncStart=mmioReadCrtc(vgap, EXT_VERT_SYNC_START);
    i810Reg->ExtVertBlankStart=mmioReadCrtc(vgap, EXT_VERT_BLANK_START);
    i810Reg->ExtHorizTotal=mmioReadCrtc(vgap, EXT_HORIZ_TOTAL);
    i810Reg->ExtHorizBlank=mmioReadCrtc(vgap, EXT_HORIZ_BLANK);
    i810Reg->ExtOffset=mmioReadCrtc(vgap, EXT_OFFSET);
    i810Reg->InterlaceControl=mmioReadCrtc(vgap, INTERLACE_CNTL);

    i810Reg->PixelPipeCfg0 = INREG8(PIXPIPE_CONFIG_0);
    i810Reg->PixelPipeCfg1 = INREG8(PIXPIPE_CONFIG_1);
    i810Reg->PixelPipeCfg2 = INREG8(PIXPIPE_CONFIG_2);
    i810Reg->DisplayControl = INREG8(DISPLAY_CNTL);  
    i810Reg->LMI_FIFO_Watermark = INREG(FWATER_BLC);

    for (i = 0 ; i < 8 ; i++)
        i810Reg->Fence[i] = INREG(FENCE+i*4);

    i810Reg->LprbTail = INREG(LP_RING + RING_TAIL);
    i810Reg->LprbHead = INREG(LP_RING + RING_HEAD);
    i810Reg->LprbStart = INREG(LP_RING + RING_START);
    i810Reg->LprbLen = INREG(LP_RING + RING_LEN);

    if ((i810Reg->LprbTail & TAIL_ADDR) != (i810Reg->LprbHead & HEAD_ADDR) &&
        i810Reg->LprbLen & RING_VALID) {
        i810PrintErrorState( i810c );
        FatalError( "Active ring not flushed\n");
    }

    if (I810_DEBUG) {
        fprintf(stderr,"Got mode in I810Save:\n");
        i810PrintMode( vgaReg, i810Reg );
    }       
}

static void
i810Preserve(KdCardInfo *card)
{
    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;

/*     fprintf(stderr,"i810Preserve\n"); */
    DoSave(card, &vgap->SavedReg, &i810c->SavedReg, TRUE);
}

/* Famous last words
 */
void 
i810PrintErrorState(i810CardInfo *i810c)
{
    
   fprintf(stderr, "pgetbl_ctl: 0x%lx pgetbl_err: 0x%lx\n", 
	   INREG(PGETBL_CTL),
	   INREG(PGE_ERR));

   fprintf(stderr, "ipeir: %lx iphdr: %lx\n", 
	   INREG(IPEIR),
	   INREG(IPEHR));

   fprintf(stderr, "LP ring tail: %lx head: %lx len: %lx start %lx\n",
	   INREG(LP_RING + RING_TAIL),
	   INREG(LP_RING + RING_HEAD) & HEAD_ADDR,
	   INREG(LP_RING + RING_LEN),
	   INREG(LP_RING + RING_START));

   fprintf(stderr, "eir: %x esr: %x emr: %x\n",
	   INREG16(EIR),
	   INREG16(ESR),
	   INREG16(EMR));

   fprintf(stderr, "instdone: %x instpm: %x\n",
	   INREG16(INST_DONE),
	   INREG8(INST_PM));

   fprintf(stderr, "memmode: %lx instps: %lx\n",
	   INREG(MEMMODE),
	   INREG(INST_PS));

   fprintf(stderr, "hwstam: %x ier: %x imr: %x iir: %x\n",
	   INREG16(HWSTAM),
	   INREG16(IER),
	   INREG16(IMR),
	   INREG16(IIR));
}

static Bool
i810BindGARTMemory( KdScreenInfo *screen ) 
{
    
    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;
    
    if (!i810c->GttBound) {
        if (!KdAcquireGART(screen->mynum))
            return FALSE;
        if (!KdBindGARTMemory(screen->mynum, i810c->VramKey,
                              i810c->VramOffset))

            return FALSE;
        if (i810c->DcacheKey != -1) {
            if (!KdBindGARTMemory(screen->mynum, i810c->DcacheKey,
                                  i810c->DcacheOffset))
                return FALSE;
        }
        if (i810c->HwcursKey != -1) {
            if (!KdBindGARTMemory(screen->mynum, i810c->HwcursKey,
                                  i810c->HwcursOffset))
                return FALSE;
        }
        i810c->GttBound = 1;
    }
    return TRUE;
}

static Bool
i810UnbindGARTMemory(KdScreenInfo  *screen) 
{
    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;


    if (KdAgpGARTSupported() && i810c->GttBound) {
        if (!KdUnbindGARTMemory(screen->mynum, i810c->VramKey))
            return FALSE;
        if (i810c->DcacheKey != -1) {
            if (!KdUnbindGARTMemory(screen->mynum, i810c->DcacheKey))
                return FALSE;
        }
        if (i810c->HwcursKey != -1) {
            if (!KdUnbindGARTMemory(screen->mynum, i810c->HwcursKey))
                return FALSE;
        }
        if (!KdReleaseGART(screen->mynum))
            return FALSE;
        i810c->GttBound = 0;
    }
    return TRUE;
}

/*
 * I810CalcVCLK --
 *
 * Determine the closest clock frequency to the one requested.
 */

#define MAX_VCO_FREQ 600.0
#define TARGET_MAX_N 30
#define REF_FREQ 24.0

#define CALC_VCLK(m,n,p) \
    (double)m / ((double)n * (1 << p)) * 4 * REF_FREQ

static void
i810CalcVCLK( KdScreenInfo *screen, double freq )
{

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;
    I810RegPtr i810Reg = &i810c->ModeReg;

   int m, n, p;
   double f_out, f_best;
   double f_err;
   double f_vco;
   int m_best = 0, n_best = 0, p_best = 0;
   double f_target = freq;
   double err_max = 0.005;
   double err_target = 0.001;
   double err_best = 999999.0;

   p_best = p = log(MAX_VCO_FREQ/f_target)/log((double)2);
   f_vco = f_target * (1 << p);

   n = 2;
   do {
      n++;
      m = f_vco / (REF_FREQ / (double)n) / (double)4.0 + 0.5;
      if (m < 3) m = 3;
      f_out = CALC_VCLK(m,n,p);
      f_err = 1.0 - (f_target/f_out);
      if (fabs(f_err) < err_max) {
	 m_best = m;
	 n_best = n;
	 f_best = f_out;
	 err_best = f_err;
      }
   } while ((fabs(f_err) >= err_target) &&
	    ((n <= TARGET_MAX_N) || (fabs(err_best) > err_max)));

   if (fabs(f_err) < err_target) {
      m_best = m;
      n_best = n;
   }

   i810Reg->VideoClk2_M          = (m_best-2) & 0x3FF;
   i810Reg->VideoClk2_N          = (n_best-2) & 0x3FF;
   i810Reg->VideoClk2_DivisorSel = (p_best << 4);

/*    fprintf(stderr, "Setting dot clock to %.1f MHz " */
/*            "[ 0x%x 0x%x 0x%x ] " */
/*            "[ %d %d %d ]\n", */
/*            CALC_VCLK(m_best,n_best,p_best), */
/*            i810Reg->VideoClk2_M, */
/*            i810Reg->VideoClk2_N, */
/*            i810Reg->VideoClk2_DivisorSel, */
/*            m_best, n_best, p_best); */
}

/*
 * I810CalcFIFO --
 *
 * Calculate burst length and FIFO watermark.
 */

#define Elements(x) (sizeof(x)/sizeof(*x))

static unsigned int 
i810CalcWatermark( KdScreenInfo *screen, double freq, Bool dcache )
{

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;
    

    struct wm_info *tab;
    int nr;
    int i;

    if (i810c->LmFreqSel == 100) {
        switch(screen->fb[0].bitsPerPixel) {
        case 8:
            tab = i810_wm_8_100;
            nr = Elements(i810_wm_8_100);
            break;
        case 16:
            tab = i810_wm_16_100;
            nr = Elements(i810_wm_16_100);
            break;
        case 24:
            tab = i810_wm_24_100;
            nr = Elements(i810_wm_24_100);
            break;
        default: 
            return 0;
        }
    } else {
        switch(screen->fb[0].bitsPerPixel) {
        case 8:
            tab = i810_wm_8_133;
            nr = Elements(i810_wm_8_133);
            break;
        case 16:
            tab = i810_wm_16_133;
            nr = Elements(i810_wm_16_133);
            break;
        case 24:
            tab = i810_wm_24_133;
            nr = Elements(i810_wm_24_133);
            break;
        default:
            return 0;
        }
    }

    for (i = 0 ; i < nr && tab[i].freq < freq ; i++);
   
    if (i == nr)
        i--;

/*     fprintf(stderr,"chose watermark 0x%x: (tab.freq %.1f)\n", */
/*             tab[i].wm, tab[i].freq); */

    /* None of these values (sourced from intel) have watermarks for
     * the dcache memory.  Fake it for now by using the same watermark
     * for both...  
     *
     * Update: this is probably because dcache isn't real useful as
     * framebuffer memory, so intel's drivers don't need watermarks
     * for that memory because they never use it to feed the ramdacs.
     * We do use it in the fallback mode, so keep the watermarks for
     * now.
     */
    if (dcache)
        return (tab[i].wm & ~0xffffff) | ((tab[i].wm>>12) & 0xfff);
    else
        return tab[i].wm;
}

static void i810PrintMode( vgaRegPtr vgaReg, I810RegPtr mode )
{
   int i;

   fprintf(stderr,"   MiscOut: %x\n", vgaReg->MiscOutReg);
   

   fprintf(stderr,"SEQ: ");   
   for (i = 0 ; i < VGA_NUM_SEQ ; i++) {
      if ((i&7)==0) fprintf(stderr,"\n");
      fprintf(stderr,"   %d: %x", i, vgaReg->Sequencer[i]);
   }
   fprintf(stderr,"\n");

   fprintf(stderr,"CRTC: ");   
   for (i = 0 ; i < VGA_NUM_CRTC ; i++) {
      if ((i&3)==0) fprintf(stderr,"\n");
      fprintf(stderr,"   CR%02x: %2x", i, vgaReg->CRTC[i]);
   }
   fprintf(stderr,"\n");

   fprintf(stderr,"GFX: ");   
   for (i = 0 ; i < VGA_NUM_GFX ; i++) {
      if ((i&3)==0) fprintf(stderr,"\n");
      fprintf(stderr,"   GR%02x: %02x", i, vgaReg->Graphics[i]);
   }
   fprintf(stderr,"\n");

   fprintf(stderr,"ATTR: ");   
   for (i = 0 ; i < VGA_NUM_ATTR ; i++) {
      if ((i&7)==0) fprintf(stderr,"\n");
      fprintf(stderr,"   %d: %x", i, vgaReg->Attribute[i]);
   }
   fprintf(stderr,"\n");


   fprintf(stderr,"   DisplayControl: %x\n", mode->DisplayControl);
   fprintf(stderr,"   PixelPipeCfg0: %x\n", mode->PixelPipeCfg0);
   fprintf(stderr,"   PixelPipeCfg1: %x\n", mode->PixelPipeCfg1);
   fprintf(stderr,"   PixelPipeCfg2: %x\n", mode->PixelPipeCfg2);
   fprintf(stderr,"   VideoClk2_M: %x\n", mode->VideoClk2_M);
   fprintf(stderr,"   VideoClk2_N: %x\n", mode->VideoClk2_N);
   fprintf(stderr,"   VideoClk2_DivisorSel: %x\n", mode->VideoClk2_DivisorSel);
   fprintf(stderr,"   AddressMapping: %x\n", mode->AddressMapping);
   fprintf(stderr,"   IOControl: %x\n", mode->IOControl);
   fprintf(stderr,"   BitBLTControl: %x\n", mode->BitBLTControl);
   fprintf(stderr,"   ExtVertTotal: %x\n", mode->ExtVertTotal);
   fprintf(stderr,"   ExtVertDispEnd: %x\n", mode->ExtVertDispEnd);
   fprintf(stderr,"   ExtVertSyncStart: %x\n", mode->ExtVertSyncStart);
   fprintf(stderr,"   ExtVertBlankStart: %x\n", mode->ExtVertBlankStart);
   fprintf(stderr,"   ExtHorizTotal: %x\n", mode->ExtHorizTotal);
   fprintf(stderr,"   ExtHorizBlank: %x\n", mode->ExtHorizBlank);
   fprintf(stderr,"   ExtOffset: %x\n", mode->ExtOffset);
   fprintf(stderr,"   InterlaceControl: %x\n", mode->InterlaceControl);
   fprintf(stderr,"   LMI_FIFO_Watermark: %x\n", mode->LMI_FIFO_Watermark);   
   fprintf(stderr,"   LprbTail: %x\n", mode->LprbTail);
   fprintf(stderr,"   LprbHead: %x\n", mode->LprbHead);
   fprintf(stderr,"   LprbStart: %x\n", mode->LprbStart);
   fprintf(stderr,"   LprbLen: %x\n", mode->LprbLen);
   fprintf(stderr,"   OverlayActiveStart: %x\n", mode->OverlayActiveStart);
   fprintf(stderr,"   OverlayActiveEnd: %x\n", mode->OverlayActiveEnd);
}


/*
 * i810VGASeqReset
 *      perform a sequencer reset.
 *
 * The i815 documentation states that these bits are not used by the
 * HW, but still warns about not programming them...
 */

static void
i810VGASeqReset(i810VGAPtr vgap, Bool start)
{
    if (start)
    {
        mmioWriteSeq(vgap, 0x00, 0x01); 	/* Synchronous Reset */
    }	
    else
    {
        mmioWriteSeq(vgap, 0x00, 0x03);		/* End Reset */
    }
}

static void
i810VGAProtect(KdCardInfo *card, Bool on)
{

    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;
    
    unsigned char tmp;
  
    if (on) {
        /*
         * Turn off screen and disable sequencer.
         */
        tmp = mmioReadSeq(vgap, 0x01);

        i810VGASeqReset(vgap, TRUE); /* start synchronous reset */
        mmioWriteSeq(vgap, 0x01, tmp | 0x20); /* disable the display */

        mmioEnablePalette(vgap);
    } else {
        /*
         * Reenable sequencer, then turn on screen.
         */
  
        tmp = mmioReadSeq(vgap, 0x01);

        mmioWriteSeq(vgap, 0x01, tmp & ~0x20);	/* reenable display */
        i810VGASeqReset(vgap, FALSE);		/* clear synchronousreset */

        mmioDisablePalette(vgap);
    }
}

/*
 * i810VGABlankScreen -- blank the screen.
 */

void
i810VGABlankScreen(KdCardInfo *card, Bool on)
{
    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;

    unsigned char scrn;

    scrn = mmioReadSeq(vgap, 0x01);

    if (on) {
        scrn &= ~0x20;			/* enable screen */
    } else {
        scrn |= 0x20;			/* blank screen */
    }
    
    mmioWriteSeq(vgap,0x00,0x01);
    mmioWriteSeq(vgap, 0x01, scrn);	/* change mode */
    mmioWriteSeq(vgap,0x00,0x03);
}

/* Restore hardware state */

static void
DoRestore(KdCardInfo *card, vgaRegPtr vgaReg, I810RegPtr i810Reg, 
	  Bool restoreFonts) {

    
    I810CardInfo    *i810c = card->driver;

    i810VGAPtr      vgap = &i810c->vga;

    unsigned char temp;
    unsigned int  itemp;
    int i;

    if (I810_DEBUG & DEBUG_VERBOSE_VGA) {
        fprintf(stderr,"Setting mode in DoRestore:\n");
        i810PrintMode( vgaReg, i810Reg );
    }
    
    /* Blank screen (i810vgaprotect) */
    i810VGAProtect(card, TRUE);
    
    /* Should wait for at least two hsync and no more than two vsync
       before writing PIXCONF and turning the display on (?) */
    usleep(50000);

    /* Turn off DRAM Refresh */
    temp = INREG8( DRAM_ROW_CNTL_HI );
    temp &= ~DRAM_REFRESH_RATE;
    temp |= DRAM_REFRESH_DISABLE;
    OUTREG8( DRAM_ROW_CNTL_HI, temp );

    usleep(1000); /* Wait 1 ms */

    /* Write the M, N and P values */
    OUTREG16( VCLK2_VCO_M, i810Reg->VideoClk2_M);
    OUTREG16( VCLK2_VCO_N, i810Reg->VideoClk2_N);
    OUTREG8( VCLK2_VCO_DIV_SEL, i810Reg->VideoClk2_DivisorSel);

    /*
     * Turn on 8 bit dac mode, if requested.  This is needed to make
     * sure that vgaHWRestore writes the values into the DAC properly.
     * The problem occurs if 8 bit dac mode is requested and the HW is
     * in 6 bit dac mode.  If this happens, all the values are
     * automatically shifted left twice by the HW and incorrect colors
     * will be displayed on the screen.  The only time this can happen
     * is at server startup time and when switching back from a VT.
     */
    temp = INREG8(PIXPIPE_CONFIG_0); 
    temp &= 0x7F; /* Save all but the 8 bit dac mode bit */
    temp |= (i810Reg->PixelPipeCfg0 & DAC_8_BIT);
    OUTREG8( PIXPIPE_CONFIG_0, temp );

    /*
     * Code to restore any SVGA registers that have been saved/modified
     * goes here.  Note that it is allowable, and often correct, to 
     * only modify certain bits in a register by a read/modify/write cycle.
     *
     * A special case - when using an external clock-setting program,
     * this function must not change bits associated with the clock
     * selection.  This condition can be checked by the condition:
     *
     *	if (i810Reg->std.NoClock >= 0)
     *		restore clock-select bits.
     */

    /* VGA restore */
    if (vgaReg->MiscOutReg & 0x01)
	vgap->IOBase = VGA_IOBASE_COLOR;
    else
	vgap->IOBase = VGA_IOBASE_MONO;

    mmioWriteMiscOut(vgap, vgaReg->MiscOutReg);

    for (i = 1; i < VGA_NUM_SEQ; i++)
	mmioWriteSeq(vgap, i, vgaReg->Sequencer[i]);
  
    /* Ensure CRTC registers 0-7 are unlocked by clearing bit 7 or CRTC[17] */
    /* = CR11 */
    mmioWriteCrtc(vgap, 17, vgaReg->CRTC[17] & ~0x80);

    for (i = 0; i < VGA_NUM_CRTC; i++) {
	mmioWriteCrtc(vgap, i, vgaReg->CRTC[i]);
    }

    for (i = 0; i < VGA_NUM_GFX; i++)
	mmioWriteGr(vgap, i, vgaReg->Graphics[i]);

    mmioEnablePalette(vgap);
    for (i = 0; i < VGA_NUM_ATTR; i++)
	mmioWriteAttr(vgap, i, vgaReg->Attribute[i]);
    mmioDisablePalette(vgap);


    mmioWriteCrtc(vgap, EXT_VERT_TOTAL, i810Reg->ExtVertTotal);
    mmioWriteCrtc(vgap, EXT_VERT_DISPLAY, i810Reg->ExtVertDispEnd);
    mmioWriteCrtc(vgap, EXT_VERT_SYNC_START, i810Reg->ExtVertSyncStart);
    mmioWriteCrtc(vgap, EXT_VERT_BLANK_START, i810Reg->ExtVertBlankStart);
    mmioWriteCrtc(vgap, EXT_HORIZ_TOTAL, i810Reg->ExtHorizTotal);
    mmioWriteCrtc(vgap, EXT_HORIZ_BLANK, i810Reg->ExtHorizBlank);

    /* write CR40, CR42 first etc to get CR13 written as described in PRM */

    mmioWriteCrtc(vgap, EXT_START_ADDR_HI, 0);
    mmioWriteCrtc(vgap, EXT_START_ADDR, EXT_START_ADDR_ENABLE);

    mmioWriteCrtc(vgap, EXT_OFFSET, i810Reg->ExtOffset);
    mmioWriteCrtc(vgap, 0x13, vgaReg->CRTC[0x13]);

    temp=mmioReadCrtc(vgap, INTERLACE_CNTL);
    temp &= ~INTERLACE_ENABLE;
    temp |= i810Reg->InterlaceControl;
    mmioWriteCrtc(vgap, INTERLACE_CNTL, temp);

    temp=i810ReadControlMMIO(i810c, GRX, ADDRESS_MAPPING);
    temp &= 0xE0; /* Save reserved bits 7:5 */
    temp |= i810Reg->AddressMapping;
    i810WriteControlMMIO(i810c, GRX, ADDRESS_MAPPING, temp);

    /* Setting the OVRACT Register for video overlay*/
    OUTREG(0x6001C, (i810Reg->OverlayActiveEnd << 16) | i810Reg->OverlayActiveStart);

    /* Turn on DRAM Refresh */
    temp = INREG8( DRAM_ROW_CNTL_HI );
    temp &= ~DRAM_REFRESH_RATE;
    temp |= DRAM_REFRESH_60HZ;
    OUTREG8( DRAM_ROW_CNTL_HI, temp );

    temp = INREG8( BITBLT_CNTL );
    temp &= ~COLEXP_MODE;
    temp |= i810Reg->BitBLTControl;
    OUTREG8( BITBLT_CNTL, temp );

    temp = INREG8( DISPLAY_CNTL );
    temp &= ~(VGA_WRAP_MODE | GUI_MODE);
    temp |= i810Reg->DisplayControl;
    OUTREG8( DISPLAY_CNTL, temp );
   

    temp = INREG8( PIXPIPE_CONFIG_0 );
    temp &= 0x64; /* Save reserved bits 6:5,2 */
    temp |= i810Reg->PixelPipeCfg0;
    OUTREG8( PIXPIPE_CONFIG_0, temp );

    temp = INREG8( PIXPIPE_CONFIG_2 );
    temp &= 0xF3; /* Save reserved bits 7:4,1:0 */
    temp |= i810Reg->PixelPipeCfg2;
    OUTREG8( PIXPIPE_CONFIG_2, temp );

    temp = INREG8( PIXPIPE_CONFIG_1 );
    temp &= ~DISPLAY_COLOR_MODE;
    temp &= 0xEF; /* Restore the CRT control bit */
    temp |= i810Reg->PixelPipeCfg1;
    OUTREG8( PIXPIPE_CONFIG_1, temp );
   
    OUTREG16(EIR, 0);

    itemp = INREG(FWATER_BLC);
    itemp &= ~(LM_BURST_LENGTH | LM_FIFO_WATERMARK | 
               MM_BURST_LENGTH | MM_FIFO_WATERMARK );
    itemp |= i810Reg->LMI_FIFO_Watermark;
    OUTREG(FWATER_BLC, itemp);


    for (i = 0 ; i < 8 ; i++) {
        OUTREG( FENCE+i*4, i810Reg->Fence[i] );
        if (I810_DEBUG & DEBUG_VERBOSE_VGA)
            fprintf(stderr,"Fence Register : %x\n",  i810Reg->Fence[i]);
    }
   
    /* First disable the ring buffer (Need to wait for empty first?, if so
     * should probably do it before entering this section)
     */
    itemp = INREG(LP_RING + RING_LEN);
    itemp &= ~RING_VALID_MASK;
    OUTREG(LP_RING + RING_LEN, itemp );

    /* Set up the low priority ring buffer.
     */
    OUTREG(LP_RING + RING_TAIL, 0 );
    OUTREG(LP_RING + RING_HEAD, 0 );

    i810c->LpRing.head = 0;
    i810c->LpRing.tail = 0;

    itemp = INREG(LP_RING + RING_START);
    itemp &= ~(START_ADDR);
    itemp |= i810Reg->LprbStart;
    OUTREG(LP_RING + RING_START, itemp );

    itemp = INREG(LP_RING + RING_LEN);
    itemp &= ~(RING_NR_PAGES | RING_REPORT_MASK | RING_VALID_MASK);
    itemp |= i810Reg->LprbLen;
    OUTREG(LP_RING + RING_LEN, itemp );

    i810VGAProtect(card, FALSE);

    temp=mmioReadCrtc(vgap, IO_CTNL);
    temp &= ~(EXTENDED_ATTR_CNTL | EXTENDED_CRTC_CNTL);
    temp |= i810Reg->IOControl;
    mmioWriteCrtc(vgap, IO_CTNL, temp);
    /* Protect CRTC[0-7] */
    mmioWriteCrtc(vgap, 0x11, mmioReadCrtc(vgap, 0x11) | 0x80);
}


static Bool
i810SetMode(KdScreenInfo *screen, const KdMonitorTiming *t) 
{

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;

    I810RegPtr i810Reg = &i810c->ModeReg;
    vgaRegPtr pVga = &vgap->ModeReg;

    double dclk = t->clock/1000.0;

    switch (screen->fb[0].bitsPerPixel) {
    case 8:
        pVga->CRTC[0x13]        = screen->width >> 3;
        i810Reg->ExtOffset      = screen->width >> 11;
        i810Reg->PixelPipeCfg1 = DISPLAY_8BPP_MODE;
        i810Reg->BitBLTControl = COLEXP_8BPP;
        break;
    case 16:
        i810Reg->PixelPipeCfg1 = DISPLAY_16BPP_MODE;
        pVga->CRTC[0x13] = screen->width >> 2;
        i810Reg->ExtOffset      = screen->width >> 10;
        i810Reg->BitBLTControl = COLEXP_16BPP;
        break;
    case 24:
        pVga->CRTC[0x13]       = (screen->width * 3) >> 3;
        i810Reg->ExtOffset     = (screen->width * 3) >> 11;

        i810Reg->PixelPipeCfg1 = DISPLAY_24BPP_MODE;
        i810Reg->BitBLTControl = COLEXP_24BPP;
        break;
    default:
        break;
    }

    i810Reg->PixelPipeCfg0 = DAC_8_BIT;

    /* Do not delay CRT Blank: needed for video overlay */
    i810Reg->PixelPipeCfg1 |= 0x10;

    /* Turn on Extended VGA Interpretation */
    i810Reg->IOControl = EXTENDED_CRTC_CNTL;

    /* Turn on linear and page mapping */
    i810Reg->AddressMapping = (LINEAR_MODE_ENABLE | 
                               GTT_MEM_MAP_ENABLE);

    /* Turn on GUI mode */
    i810Reg->DisplayControl = HIRES_MODE;

    i810Reg->OverlayActiveStart = t->horizontal + t->hblank - 32;
    i810Reg->OverlayActiveEnd = t->horizontal  - 32;

    /* Turn on interlaced mode if necessary (it's not) */
    i810Reg->InterlaceControl = INTERLACE_DISABLE;

    /*
     * Set the overscan color to 0.
     * NOTE: This only affects >8bpp mode.
     */
    pVga->Attribute[0x11] = 0;

    /*
     * Calculate the VCLK that most closely matches the requested dot
     * clock.
     */
    i810CalcVCLK(screen, dclk);

    /* Since we program the clocks ourselves, always use VCLK2. */
    pVga->MiscOutReg |= 0x0C;

    /* Calculate the FIFO Watermark and Burst Length. */
    i810Reg->LMI_FIFO_Watermark = i810CalcWatermark(screen, dclk, FALSE);
    
    /* Setup the ring buffer */
    i810Reg->LprbTail = 0;
    i810Reg->LprbHead = 0;
    i810Reg->LprbStart = i810c->LpRing.mem.Start;

    if (i810Reg->LprbStart) 
        i810Reg->LprbLen = ((i810c->LpRing.mem.Size-4096) |
                            RING_NO_REPORT | RING_VALID);
    else
        i810Reg->LprbLen = RING_INVALID;

    return TRUE;
}

static Bool
i810ModeInit(KdScreenInfo *screen, const KdMonitorTiming *t)
{

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;
    i810VGAPtr      vgap = &i810c->vga;
    vgaRegPtr pVga;

/*     fprintf(stderr,"i810ModeInit\n"); */
    
    i810VGAUnlock(vgap);

    if (!i810VGAInit(screen, t)) return FALSE;
    pVga = &vgap->ModeReg;

    if (!i810SetMode(screen, t)) return FALSE;

    DoRestore(screen->card, &vgap->ModeReg, &i810c->ModeReg, FALSE);

    return TRUE;
}

Bool
i810VGAInit(KdScreenInfo *screen, const KdMonitorTiming *t)
{
    unsigned int       i;

    int hactive, hblank, hbp, hfp;
    int vactive, vblank, vbp, vfp;
    int h_screen_off = 0, h_adjust = 0, h_total, h_display_end, h_blank_start;
    int h_blank_end, h_sync_start, h_sync_end, v_total, v_retrace_start;
    int v_retrace_end, v_display_end, v_blank_start, v_blank_end;

    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = card->driver;

    i810VGAPtr      vgap = &i810c->vga;
    I810RegPtr ireg = &i810c->ModeReg;
    

    vgaRegPtr regp;
    int depth = screen->fb[0].depth;

    regp = &vgap->ModeReg;
    
    /*
     * compute correct Hsync & Vsync polarity 
     */

    regp->MiscOutReg = 0x23;
    if (t->vpol == KdSyncNegative) regp->MiscOutReg |= 0x40;
    if (t->hpol == KdSyncNegative) regp->MiscOutReg |= 0x80;

    /*
     * Time Sequencer
     */
    if (depth == 4)
        regp->Sequencer[0] = 0x02;
    else
        regp->Sequencer[0] = 0x00;
    /* No support for 320 or 360 x resolution */
    regp->Sequencer[1] = 0x01;

    if (depth == 1)
        regp->Sequencer[2] = 1 << BIT_PLANE;
    else
        regp->Sequencer[2] = 0x0F;

    regp->Sequencer[3] = 0x00;                             /* Font select */

    if (depth < 8)
        regp->Sequencer[4] = 0x06;                             /* Misc */
    else
        regp->Sequencer[4] = 0x0E;                             /* Misc */

    hactive = t->horizontal;
    hblank = t->hblank;
    hbp = t->hbp;
    hfp = t->hfp;
    
    vactive = t->vertical;
    vblank = t->vblank;
    vbp = t->vbp;
    vfp = t->vfp;
        
    switch (screen->fb[0].bitsPerPixel) {
    case 8:
	hactive /= 8;
	hblank /= 8;
	hfp /= 8;
	hbp /= 8;
	h_screen_off = hactive;
	h_adjust = 1;	
	break;
    case 16:
	hactive /= 8;
	hblank /= 8;
	hfp /= 8;
	hbp /= 8;

	h_screen_off = hactive * 2;
	h_adjust = 1;
	break;
    case 24:
	hactive /= 8;
	hblank /= 8;
	hfp /= 8;
	hbp /= 8;
	
	h_screen_off = hactive * 3;
	h_adjust = 1;
	break;
    case 32:
	hactive /= 8;
	hblank /= 8;
	hfp /= 8;
	hbp /= 8;
	
	h_screen_off = hactive * 4;
	h_adjust = 1;
	break;
    }
	    
    /*
     * Compute horizontal register values from timings
     */
    h_total = hactive + hblank - 5;
    h_display_end = hactive - 1;
    h_blank_start = h_display_end;
    h_blank_end = h_blank_start + hblank;
    
    h_sync_start = hactive + hfp + h_adjust;
    h_sync_end = h_sync_start + hblank - hbp - hfp;

    /* Set CRTC regs for horizontal timings */
    regp->CRTC[0x0] = h_total;
    ireg->ExtHorizTotal=(h_total & 0x100) >> 8;
    
    regp->CRTC[0x1] = h_display_end;
    
    regp->CRTC[0x2] = h_blank_start;

    regp->CRTC[0x3] = 0x80 | (h_blank_end & 0x1f);
    regp->CRTC[0x5] = (h_blank_end & 0x20) << 2;

    regp->CRTC[0x4] = h_sync_start;

    regp->CRTC[0x5] |= h_sync_end & 0x1f;
    
    regp->CRTC[0x13] = h_screen_off;
    ireg->ExtOffset = h_screen_off >> 8;

    /* Compute vertical timings */
    v_total = vactive + vblank - 2;
    v_retrace_start = vactive + vfp - 1;
    v_retrace_end = v_retrace_start + vblank - vbp - vfp;
    v_display_end = vactive - 1;
    v_blank_start = vactive - 1;
    v_blank_end = v_blank_start + vblank /* - 1 */;

    regp->CRTC[0x6] = v_total;
    ireg->ExtVertTotal = v_total >> 8;
   
    regp->CRTC[0x10] = v_retrace_start;
    ireg->ExtVertSyncStart = v_retrace_start >> 8;

    regp->CRTC[0x11] = v_retrace_end;

    regp->CRTC[0x12] = v_display_end;
    ireg->ExtVertDispEnd = v_display_end >> 8;

    regp->CRTC[0x15] = v_blank_start;
    ireg->ExtVertBlankStart = v_blank_start >> 8;

    regp->CRTC[0x16] = v_blank_end;
    
    if (depth < 8)
	regp->CRTC[23] = 0xE3;
    else
	regp->CRTC[23] = 0xC3;
    regp->CRTC[24] = 0xFF;

    /*
     * Graphics Display Controller
     */
    regp->Graphics[0] = 0x00;
    regp->Graphics[1] = 0x00;
    regp->Graphics[2] = 0x00;
    regp->Graphics[3] = 0x00;
    if (depth == 1) {
        regp->Graphics[4] = BIT_PLANE;
        regp->Graphics[5] = 0x00;
    } else {
        regp->Graphics[4] = 0x00;
        if (depth == 4)
            regp->Graphics[5] = 0x02;
        else
            regp->Graphics[5] = 0x40;
    }
    regp->Graphics[6] = 0x05;
    regp->Graphics[7] = 0x0F;
    regp->Graphics[8] = 0xFF;
  
    if (depth == 1) {
        /* Initialise the Mono map according to which bit-plane gets used */

        Bool flipPixels = FALSE; /* maybe support this in the future? */

        for (i=0; i<16; i++)
            if (((i & (1 << BIT_PLANE)) != 0) != flipPixels)
                regp->Attribute[i] = WHITE_VALUE;
            else
                regp->Attribute[i] = BLACK_VALUE;

        regp->Attribute[16] = 0x01;  /* -VGA2- */
	if (!vgap->ShowOverscan)
            regp->Attribute[OVERSCAN] = OVERSCAN_VALUE;  /* -VGA2- */
    } else {
        regp->Attribute[0]  = 0x00; /* standard colormap translation */
        regp->Attribute[1]  = 0x01;
        regp->Attribute[2]  = 0x02;
        regp->Attribute[3]  = 0x03;
        regp->Attribute[4]  = 0x04;
        regp->Attribute[5]  = 0x05;
        regp->Attribute[6]  = 0x06;
        regp->Attribute[7]  = 0x07;
        regp->Attribute[8]  = 0x08;
        regp->Attribute[9]  = 0x09;
        regp->Attribute[10] = 0x0A;
        regp->Attribute[11] = 0x0B;
        regp->Attribute[12] = 0x0C;
        regp->Attribute[13] = 0x0D;
        regp->Attribute[14] = 0x0E;
        regp->Attribute[15] = 0x0F;
        if (depth == 4)
            regp->Attribute[16] = 0x81;
        else
            regp->Attribute[16] = 0x41;
        /* Attribute[17] (overscan) was initialised earlier */
    }
    regp->Attribute[18] = 0x0F;
    regp->Attribute[19] = 0x00;
    regp->Attribute[20] = 0x00;

    return(TRUE);
}

void
i810VGALock(i810VGAPtr vgap)
{
    /* Protect CRTC[0-7] */
    mmioWriteCrtc(vgap, 0x11, mmioReadCrtc(vgap, 0x11) & ~0x80);
}

void
i810VGAUnlock(i810VGAPtr vgap)
{
    /* Unprotect CRTC[0-7] */
    mmioWriteCrtc(vgap, 0x11, mmioReadCrtc(vgap, 0x11) | 0x80);
}

static void
i810Restore(KdCardInfo *card) {

    I810CardInfo    *i810c = card->driver;

    i810VGAPtr      vgap = &i810c->vga;

    if (I810_DEBUG)
        fprintf(stderr,"i810Restore\n");

    DoRestore(card, &vgap->SavedReg, &i810c->SavedReg, TRUE);
}

static Bool
i810Enable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    KdCardInfo	    *card = pScreenPriv->card;
    I810CardInfo    *i810c = card->driver;
    i810VGAPtr vgap = &i810c->vga;
    const KdMonitorTiming *t;

    if (I810_DEBUG)
        fprintf(stderr,"i810Enable\n");

    vgap->IOBase = (mmioReadMiscOut(vgap) & 0x01) ?
        VGA_IOBASE_COLOR : VGA_IOBASE_MONO;
    
    {
        I810RegPtr i810Reg = &i810c->ModeReg;
        int i;
	
        for (i = 0 ; i < 8 ; i++)
            i810Reg->Fence[i] = 0;
    }

    t = KdFindMode (screen, i810ModeSupported);
    
    if (!i810BindGARTMemory(screen))
        return FALSE;

    if (!i810ModeInit(screen, t)) return FALSE;

    {
        /* DPMS power on state */

        unsigned char SEQ01=0;
        int DPMSSyncSelect=0;

        SEQ01 = 0x00;
        DPMSSyncSelect = HSYNC_ON | VSYNC_ON;

        SEQ01 |= i810ReadControlMMIO(i810c, SRX, 0x01) & ~0x20;
        i810WriteControlMMIO(i810c, SRX, 0x01, SEQ01);

        /* Set the DPMS mode */
        OUTREG8(DPMS_SYNC_SELECT, DPMSSyncSelect);
    }
#ifdef XV
    KdXVEnable (pScreen);
#endif
    return TRUE;
}


static void
i810Disable(ScreenPtr pScreen) {

    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    KdCardInfo	    *card = pScreenPriv->card;
    I810CardInfo    *i810c = card->driver;

    i810VGAPtr      vgap = &i810c->vga;

    if (I810_DEBUG)
        fprintf(stderr,"i810Disable\n");

#ifdef XV
    KdXVDisable (pScreen);
#endif
    i810Restore(screen->card);

    if (!i810UnbindGARTMemory(screen))
        return;

    i810VGALock(vgap);
}


static Bool
i810DPMS(ScreenPtr pScreen, int mode) 
{
    KdScreenPriv(pScreen);
    KdCardInfo	    *card = pScreenPriv->card;
    I810CardInfo    *i810c = card->driver;

   unsigned char SEQ01=0;
   int DPMSSyncSelect=0;

   if (I810_DEBUG)
       fprintf(stderr,"i810DPMS: %d\n",mode);

   switch (mode) {
   case KD_DPMS_NORMAL:
      /* Screen: On; HSync: On, VSync: On */
      SEQ01 = 0x00;
      DPMSSyncSelect = HSYNC_ON | VSYNC_ON;
      break;
   case KD_DPMS_STANDBY:
      /* Screen: Off; HSync: Off, VSync: On */
      SEQ01 = 0x20;
      DPMSSyncSelect = HSYNC_OFF | VSYNC_ON;
      break;
   case KD_DPMS_SUSPEND:
      /* Screen: Off; HSync: On, VSync: Off */
      SEQ01 = 0x20;
      DPMSSyncSelect = HSYNC_ON | VSYNC_OFF;
      break;
   case KD_DPMS_POWERDOWN:
      /* Screen: Off; HSync: Off, VSync: Off */
      SEQ01 = 0x20;
      DPMSSyncSelect = HSYNC_OFF | VSYNC_OFF;
      break;
   }

   /* Turn the screen on/off */
   SEQ01 |= i810ReadControlMMIO(i810c, SRX, 0x01) & ~0x20;
   i810WriteControlMMIO(i810c, SRX, 0x01, SEQ01);

   /* Set the DPMS mode */
   OUTREG8(DPMS_SYNC_SELECT, DPMSSyncSelect);
   return TRUE;
}


static void
i810GetColors (ScreenPtr pScreen, int fb, int ndefs, xColorItem *c)
{

    if (I810_DEBUG)
        fprintf(stderr,"i810GetColors (NOT IMPLEMENTED)\n");
}

#define DACDelay(hw)							     \
	do {								     \
	    unsigned char temp = Vminb((hw)->IOBase + VGA_IN_STAT_1_OFFSET);   \
	    temp = Vminb((hw)->IOBase + VGA_IN_STAT_1_OFFSET);		     \
	} while (0)

static void
i810PutColors (ScreenPtr pScreen, int fb, int ndef, xColorItem *pdefs)
{

    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    KdCardInfo	    *card = screen->card;
    I810CardInfo    *i810c = (I810CardInfo *) card->driver;

    i810VGAPtr vgap = &i810c->vga;

    if (I810_DEBUG)
        fprintf(stderr,"i810PutColors\n");

    while (ndef--)
    {
        mmioWriteDacWriteAddr(vgap, pdefs->pixel);
	DACDelay(vgap);
	mmioWriteDacData(vgap, pdefs->red);
	DACDelay(vgap);
	mmioWriteDacData(vgap, pdefs->green);
	DACDelay(vgap);
	mmioWriteDacData(vgap, pdefs->blue);
	DACDelay(vgap);

	pdefs++;
    }
}


KdCardFuncs	i810Funcs = {
    i810CardInit,               /* cardinit */
    i810ScreenInit,             /* scrinit */
    i810InitScreen,             /* initScreen */
    i810FinishInitScreen,       /* finishInitScreen */
    NULL,			/* createResources */
    i810Preserve,               /* preserve */
    i810Enable,                 /* enable */
    i810DPMS,                   /* dpms */
    i810Disable,                /* disable */
    i810Restore,                /* restore */
    i810ScreenFini,             /* scrfini */
    i810CardFini,               /* cardfini */
    
    i810CursorInit,             /* initCursor */
    i810CursorEnable,           /* enableCursor */
    i810CursorDisable,          /* disableCursor */
    i810CursorFini,             /* finiCursor */
    NULL,                       /* recolorCursor */

    i810InitAccel,              /* initAccel */
    i810EnableAccel,            /* enableAccel */
    i810DisableAccel,           /* disableAccel */
    i810FiniAccel,              /* finiAccel */
    
    i810GetColors,    	    /* getColors */
    i810PutColors,	    /* putColors */
};
