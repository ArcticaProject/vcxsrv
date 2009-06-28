/*

Copyright 1996, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/*
 * (c) Copyright 1996 Hewlett-Packard Company
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc.
 * (c) Copyright 1996 Digital Equipment Corp.
 * (c) Copyright 1996 Fujitsu Limited
 * (c) Copyright 1996 Hitachi, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the names of the copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from said copyright holders.
 */

/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PsInit.c
**    *
**    *  Contents:      Initialization code of Ps driver for the print server.
**    *
**    *  Created By:	Roger Helmendach (Liberty Systems)
**    *
**    *  Copyright:	Copyright 1996 The Open Group, Inc.
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "Ps.h"
#include "mi.h"
#include "micmap.h"
#include "AttrValid.h"
#include "fb.h"

#include "windowstr.h"
#include "DiPrint.h"

static void AllocatePsPrivates(ScreenPtr pScreen);
static int PsInitContext(XpContextPtr pCon);
static int PsDestroyContext(XpContextPtr pCon);

DevPrivateKey PsScreenPrivateKey = &PsScreenPrivateKey;
DevPrivateKey PsContextPrivateKey = &PsContextPrivateKey;
DevPrivateKey PsPixmapPrivateKey = &PsPixmapPrivateKey;
DevPrivateKey PsWindowPrivateKey = &PsWindowPrivateKey;

#ifdef GLXEXT
extern void GlxWrapInitVisuals(miInitVisualsProcPtr *);
#endif /* GLXEXT */

Bool
InitializePsDriver(ndx, pScreen, argc, argv)
  int         ndx;
  ScreenPtr   pScreen;
  int         argc;
  char      **argv;
{
#if 0
  int               maxXres, maxYres, maxWidth, maxHeight;
  int               maxRes, maxDim, numBytes;
  PsScreenPrivPtr   pPriv;
#endif
  int               nv,       /* total number of visuals */
                    nv_1bit,  /* number of 8bit visuals */
                    nv_8bit,  /* number of 8bit visuals */
                    nv_12bit, /* number of 12bit visuals */
                    nv_14bit, /* number of 14bit visuals */
                    nv_16bit, /* number of 16bit visuals */
                    nv_24bit, /* number of 24bit visuals*/
                    nv_30bit; /* number of 30bit visuals*/
  int               nd;       /* number of depths */
  int               defaultVisualIndex = -1;
  VisualID         *vids_1bit,
                   *vids_8bit,
                   *vids_12bit,
                   *vids_14bit,
                   *vids_16bit,
                   *vids_24bit,
                   *vids_30bit;
  VisualPtr         visuals;
  DepthPtr          depths;
  VisualID          defaultVisual;
  int               rootDepth;

/*
 * Register this driver's InitContext function with the print
 * extension.
 */
  XpRegisterInitFunc(pScreen, "XP-POSTSCRIPT", PsInitContext);

/*
 * Create and fill in the devPrivate for the PS driver.
 */
  AllocatePsPrivates(pScreen);

#if 0
  pPriv = (PsScreenPrivPtr)
      dixLookupPrivate(&pScreen->devPrivates, PsScreenPrivateKey);
  pPriv->resDB = rmdb;
#endif

  pScreen->defColormap            = (Colormap) FakeClientID(0);
  pScreen->blackPixel             = 1;
  pScreen->whitePixel             = 0;
  pScreen->QueryBestSize          = (QueryBestSizeProcPtr)PsQueryBestSize;
  pScreen->SaveScreen             = (SaveScreenProcPtr)_XpBoolNoop;
  pScreen->GetImage               = (GetImageProcPtr)_XpVoidNoop;
  pScreen->GetSpans               = (GetSpansProcPtr)_XpVoidNoop;
  pScreen->CreateWindow           = PsCreateWindow;
  pScreen->DestroyWindow          = PsDestroyWindow;
  pScreen->PositionWindow         = PsPositionWindow;
  pScreen->ChangeWindowAttributes = PsChangeWindowAttributes;
  pScreen->RealizeWindow          = PsMapWindow;
  pScreen->UnrealizeWindow        = PsUnmapWindow;
  pScreen->CloseScreen            = PsCloseScreen;
  pScreen->CopyWindow             = PsCopyWindow;
       /* XXX Hard routine to write! */

/*
 * These two are going to be VERY different...
 */
  pScreen->CreatePixmap           = PsCreatePixmap;
  pScreen->DestroyPixmap          = PsDestroyPixmap;
  pScreen->RealizeFont            = PsRealizeFont;
  pScreen->UnrealizeFont          = PsUnrealizeFont;
  pScreen->CreateGC               = PsCreateGC;
  pScreen->CreateColormap         = PsCreateColormap;
  pScreen->DestroyColormap        = PsDestroyColormap;
  pScreen->InstallColormap        = PsInstallColormap;
  pScreen->UninstallColormap      = PsUninstallColormap;
  pScreen->ListInstalledColormaps = PsListInstalledColormaps;
  pScreen->StoreColors            = PsStoreColors;
  pScreen->ResolveColor           = PsResolveColor;
    /* Will BitmapToRegion make any difference at all? */
  pScreen->BitmapToRegion         = fbPixmapToRegion;

  visuals    = (VisualPtr) xalloc(16*sizeof(VisualRec));
  depths     = (DepthPtr)  xalloc(16*sizeof(DepthRec));
  vids_1bit  = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_8bit  = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_12bit = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_14bit = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_16bit = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_24bit = (VisualID *)xalloc(16*sizeof(VisualID));
  vids_30bit = (VisualID *)xalloc(16*sizeof(VisualID));

  nv = nv_1bit = nv_8bit = nv_12bit = nv_14bit = nv_16bit = nv_24bit = nv_30bit = nd = 0;

#ifdef PSOUT_USE_DEEPCOLOR
/* gisburn: 30bit TrueColor has been disabled for now since it causes problems
 * with GLX - see https://bugs.freedesktop.org/show_bug.cgi?id=2868 ("Mesa
 * seems to be unable to handle 30bit TrueColor visuals") for details... 
 */
#ifdef DISABLED_FOR_NOW
  /* TrueColor, 30bit, 10bit per R-,G-,B-gun */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = TrueColor;
  visuals[nv].bitsPerRGBValue = 10;
  visuals[nv].ColormapEntries = 1024;
  visuals[nv].nplanes         = 30;
  visuals[nv].redMask         = 0X3FF00000;
  visuals[nv].greenMask       = 0X000FFC00;
  visuals[nv].blueMask        = 0X000003FF;
  visuals[nv].offsetRed       = 20;
  visuals[nv].offsetGreen     = 10;
  visuals[nv].offsetBlue      = 0;
  vids_30bit[nv_30bit] = visuals[nv].vid;
  nv++; nv_30bit++;
#endif /* DISABLED_FOR_NOW */
#endif /* PSOUT_USE_DEEPCOLOR */

  /* TrueColor, 24bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = TrueColor;
  visuals[nv].bitsPerRGBValue = 8;
  visuals[nv].ColormapEntries = 256;
  visuals[nv].nplanes         = 24;
  visuals[nv].redMask         = 0X00FF0000;
  visuals[nv].greenMask       = 0X0000FF00;
  visuals[nv].blueMask        = 0X000000FF;
  visuals[nv].offsetRed       = 16;
  visuals[nv].offsetGreen     = 8;
  visuals[nv].offsetBlue      = 0;
  vids_24bit[nv_24bit] = visuals[nv].vid;
  nv++; nv_24bit++;

  /* TrueColor, 16bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = TrueColor;
  visuals[nv].bitsPerRGBValue = 6;
  visuals[nv].ColormapEntries = 64;
  visuals[nv].nplanes         = 16;
  visuals[nv].redMask         = 0x0000f800;
  visuals[nv].greenMask       = 0x000007e0;
  visuals[nv].blueMask        = 0x0000001f;
  visuals[nv].offsetRed       = 11;
  visuals[nv].offsetGreen     = 5;
  visuals[nv].offsetBlue      = 0;
  vids_16bit[nv_16bit] = visuals[nv].vid;
  nv++; nv_16bit++;
  
#ifdef PSOUT_USE_DEEPCOLOR
  /* PostScript Level 2 and above, colors can have 12 bits per component
   * (36 bit for RGB) */

  /* PseudoColor, 14bit (15bit won't work as |ColormapEntries==32768|
   * is too large for a |signed short|... xx@@!!!... ;-( ) */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = PseudoColor;
  visuals[nv].bitsPerRGBValue = 12;
  visuals[nv].ColormapEntries = 16384;
  visuals[nv].nplanes         = 14;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_14bit[nv_14bit] = visuals[nv].vid;
  nv++; nv_14bit++;

  /* PseudoColor, 12bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = PseudoColor;
  visuals[nv].bitsPerRGBValue = 12;
  visuals[nv].ColormapEntries = 4096;
  visuals[nv].nplanes         = 12;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_12bit[nv_12bit] = visuals[nv].vid;
  defaultVisualIndex = nv;
  nv++; nv_12bit++;

  /* GrayScale, 12bit, 12bit per R-,G-,B-gun */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = GrayScale;
  visuals[nv].bitsPerRGBValue = 12;
  visuals[nv].ColormapEntries = 4096;
  visuals[nv].nplanes         = 12;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_12bit[nv_12bit] = visuals[nv].vid;
  nv++; nv_12bit++;

  /* StaticGray, 12bit, 12bit per R-,G-,B-gun */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = StaticGray;
  visuals[nv].bitsPerRGBValue = 12;
  visuals[nv].ColormapEntries = 4096;
  visuals[nv].nplanes         = 12;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_12bit[nv_12bit] = visuals[nv].vid;
  nv++; nv_12bit++;
#endif /* PSOUT_USE_DEEPCOLOR */

  /* PseudoColor, 8bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = PseudoColor;
  visuals[nv].bitsPerRGBValue = 8;
  visuals[nv].ColormapEntries = 256;
  visuals[nv].nplanes         = 8;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_8bit[nv_8bit] = visuals[nv].vid;
#ifndef PSOUT_USE_DEEPCOLOR
  defaultVisualIndex = nv;
#endif /* !PSOUT_USE_DEEPCOLOR */
  nv++; nv_8bit++;

  /* GrayScale, 8bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = GrayScale;
  visuals[nv].bitsPerRGBValue = 8;
  visuals[nv].ColormapEntries = 256;
  visuals[nv].nplanes         = 8;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_8bit[nv_8bit] = visuals[nv].vid;
  nv++; nv_8bit++;

  /* StaticGray, 8bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = StaticGray;
  visuals[nv].bitsPerRGBValue = 8;
  visuals[nv].ColormapEntries = 256;
  visuals[nv].nplanes         = 8;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_8bit[nv_8bit] = visuals[nv].vid;
  nv++; nv_8bit++;

  /* StaticGray, 1bit */
  visuals[nv].vid             = FakeClientID(0);
  visuals[nv].class           = StaticGray;
  visuals[nv].bitsPerRGBValue = 1;
  visuals[nv].ColormapEntries = 2;
  visuals[nv].nplanes         = 1;
  visuals[nv].redMask         = 0x0;
  visuals[nv].greenMask       = 0x0;
  visuals[nv].blueMask        = 0x0;
  visuals[nv].offsetRed       = 0x0;
  visuals[nv].offsetGreen     = 0x0;
  visuals[nv].offsetBlue      = 0x0;
  vids_1bit[nv_1bit] = visuals[nv].vid;
  nv++; nv_1bit++;

  if( nv_30bit > 0 )
  {
    depths[nd].depth   = 30;
    depths[nd].numVids = nv_30bit;
    depths[nd].vids    = vids_30bit;
    nd++;
  }

  if( nv_24bit > 0 )
  {
    depths[nd].depth   = 24;
    depths[nd].numVids = nv_24bit;
    depths[nd].vids    = vids_24bit;
    nd++;
  }

  if( nv_16bit > 0 )
  {
    depths[nd].depth   = 16;
    depths[nd].numVids = nv_16bit;
    depths[nd].vids    = vids_16bit;
    nd++;
  }

  if( nv_14bit > 0 )
  {
    depths[nd].depth   = 14;
    depths[nd].numVids = nv_14bit;
    depths[nd].vids    = vids_14bit;
    nd++;
  }
  
  if( nv_12bit > 0 )
  {
    depths[nd].depth   = 12;
    depths[nd].numVids = nv_12bit;
    depths[nd].vids    = vids_12bit;
    nd++;
  }
  
  if( nv_8bit > 0 )
  {
    depths[nd].depth   = 8;
    depths[nd].numVids = nv_8bit;
    depths[nd].vids    = vids_8bit;
    nd++;
  }

  if( nv_1bit > 0 )
  {
    depths[nd].depth   = 1;
    depths[nd].numVids = nv_1bit;
    depths[nd].vids    = vids_1bit;
    nd++;
  }

  /* Defaul visual is 12bit PseudoColor */
  defaultVisual = visuals[defaultVisualIndex].vid;
  rootDepth = visuals[defaultVisualIndex].nplanes;

#ifdef GLXEXT
  {
    miInitVisualsProcPtr proc = NULL;

    GlxWrapInitVisuals(&proc);
    /* GlxInitVisuals ignores the last three arguments. */
    proc(&visuals, &depths, &nv, &nd,
         &rootDepth, &defaultVisual, 0, 0, 0);
  }
#endif /* GLXEXT */

  miScreenInit(pScreen, (pointer)0,
               pScreen->width, pScreen->height,
               (int) (pScreen->width / (pScreen->mmWidth / 25.40)), 
               (int) (pScreen->height / (pScreen->mmHeight / 25.40)),
               0, rootDepth, nd,
               depths, defaultVisual, nv, visuals);

  if( miCreateDefColormap(pScreen)==FALSE ) return FALSE;

/*scalingScreenInit(pScreen);*/

  return TRUE;
}

static void
AllocatePsPrivates(ScreenPtr pScreen)
{
    dixRequestPrivate(PsWindowPrivateKey, sizeof(PsWindowPrivRec));
    dixRequestPrivate(PsContextPrivateKey, sizeof(PsContextPrivRec));
    dixRequestPrivate(PsPixmapPrivateKey, sizeof(PsPixmapPrivRec));

    dixSetPrivate(&pScreen->devPrivates, PsScreenPrivateKey,
		  xalloc(sizeof(PsScreenPrivRec)));
}

/*
 * PsInitContext
 *
 * Establish the appropriate values for a PrintContext used with the PS
 * driver.
 */

static char DOC_ATT_SUPP[]="document-attributes-supported";
static char DOC_ATT_VAL[]="document-format xp-listfonts-modes";
static char JOB_ATT_SUPP[]="job-attributes-supported";
static char JOB_ATT_VAL[]="";
static char PAGE_ATT_SUPP[]="xp-page-attributes-supported";
static char PAGE_ATT_VAL[]="content-orientation default-printer-resolution \
default-input-tray default-medium plex xp-listfonts-modes";

static int
PsInitContext(pCon)
  XpContextPtr pCon;
{
  XpDriverFuncsPtr pFuncs;
  PsContextPrivPtr pConPriv;
  char *server, *attrStr;
    
  /*
   * Initialize the attribute store for this printer.
   */
  XpInitAttributes(pCon);

  /*
   * Initialize the function pointers
   */
  pFuncs = &(pCon->funcs);
  pFuncs->StartJob          = PsStartJob;
  pFuncs->EndJob            = PsEndJob;
  pFuncs->StartDoc          = PsStartDoc;
  pFuncs->EndDoc            = PsEndDoc;
  pFuncs->StartPage         = PsStartPage;
  pFuncs->EndPage           = PsEndPage;
  pFuncs->PutDocumentData   = PsDocumentData;
  pFuncs->GetDocumentData   = PsGetDocumentData;
  pFuncs->GetAttributes     = PsGetAttributes;
  pFuncs->SetAttributes     = PsSetAttributes;
  pFuncs->AugmentAttributes = PsAugmentAttributes;
  pFuncs->GetOneAttribute   = PsGetOneAttribute;
  pFuncs->DestroyContext    = PsDestroyContext;
  pFuncs->GetMediumDimensions = PsGetMediumDimensions;
  pFuncs->GetReproducibleArea = PsGetReproducibleArea;
  pFuncs->SetImageResolution = PsSetImageResolution;
    
  /*
   * Set up the context privates
   */
  pConPriv = (PsContextPrivPtr)
      dixLookupPrivate(&pCon->devPrivates, PsContextPrivateKey);

  memset(pConPriv, 0, sizeof(PsContextPrivRec));
  pConPriv->jobFileName         = (char *)NULL;
  pConPriv->pJobFile            = (FILE *)NULL;
  pConPriv->dash                = (unsigned char *)NULL;
  pConPriv->validGC             = 0;
  pConPriv->getDocClient        = (ClientPtr)NULL;
  pConPriv->getDocBufSize       = 0;
  pConPriv->pPsOut              = NULL;
  pConPriv->fontInfoRecords     = NULL;
  pConPriv->fontTypeInfoRecords = NULL;

  /*
   * document-attributes-supported
   */
  server = XpGetOneAttribute( pCon, XPServerAttr, DOC_ATT_SUPP );
  if ((attrStr = (char *) xalloc(strlen(server) +
				strlen(DOC_ATT_SUPP) + strlen(DOC_ATT_VAL)
				+ strlen(PAGE_ATT_VAL) + 8)) == NULL) 
  {
      return BadAlloc;
  }
  sprintf(attrStr, "*%s:\t%s %s %s", 
	  DOC_ATT_SUPP, server, DOC_ATT_VAL, PAGE_ATT_VAL);
  XpAugmentAttributes( pCon, XPPrinterAttr, attrStr);
  xfree(attrStr);
    
  /*
   * job-attributes-supported
   */
  server = XpGetOneAttribute( pCon, XPServerAttr, JOB_ATT_SUPP );
  if ((attrStr = (char *) xalloc(strlen(server) + strlen(JOB_ATT_SUPP) +
				 strlen(JOB_ATT_VAL) + 8)) == NULL)
  {
      return BadAlloc;
  }
  sprintf(attrStr, "*%s:\t%s %s", JOB_ATT_SUPP, server, JOB_ATT_VAL);
  XpAugmentAttributes(pCon, XPPrinterAttr, attrStr);
  xfree(attrStr);
    
  /*
   * xp-page-attributes-supported
   */
  server = XpGetOneAttribute( pCon, XPServerAttr, PAGE_ATT_SUPP );
  if ((attrStr = (char *) xalloc(strlen(server) + strlen(PAGE_ATT_SUPP) +
				 strlen(PAGE_ATT_VAL) + 8)) == NULL)
  {
      return BadAlloc;
  }
  sprintf(attrStr, "*%s:\t%s %s", PAGE_ATT_SUPP, server, PAGE_ATT_VAL);
  XpAugmentAttributes(pCon, XPPrinterAttr, attrStr);
  xfree(attrStr);

  /*
   * Validate the attribute pools
   */
  XpValidateAttributePool(pCon, XPPrinterAttr, &PsValidatePoolsRec);
  XpValidateAttributePool(pCon, XPDocAttr, &PsValidatePoolsRec);
  XpValidateAttributePool(pCon, XPJobAttr, &PsValidatePoolsRec);
  XpValidateAttributePool(pCon, XPPageAttr, &PsValidatePoolsRec);

  return Success;
}

static Bool
PsDestroyContext(pCon)
  XpContextPtr pCon;
{
  PsContextPrivPtr pConPriv = (PsContextPrivPtr)
      dixLookupPrivate(&pCon->devPrivates, PsContextPrivateKey);
    
  if( pConPriv->pJobFile!=(FILE *)NULL )
  {
    fclose(pConPriv->pJobFile);
    pConPriv->pJobFile = NULL;
  }
  if( pConPriv->jobFileName!=(char *)NULL )
  {
    unlink(pConPriv->jobFileName);
    xfree(pConPriv->jobFileName);
    pConPriv->jobFileName = (char *)NULL;
  }

  PsFreeFontInfoRecords(pConPriv);

  /* Reset context to make sure we do not use any stale/invalid/obsolete data */
  memset(pConPriv, 0, sizeof(PsContextPrivRec));

/*### free up visuals/depths ###*/

  return Success;
}

XpContextPtr
PsGetContextFromWindow(win)
  WindowPtr win;
{
  PsWindowPrivPtr pPriv;

  while( win )
  {
    pPriv = (PsWindowPrivPtr)
	dixLookupPrivate(&win->devPrivates, PsWindowPrivateKey);
    if( pPriv->validContext ) return pPriv->context;
    win = win->parent;
  }

  return NULL;
}
