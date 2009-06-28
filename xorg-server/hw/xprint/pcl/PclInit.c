/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclInit.c
**    *
**    *  Contents:
**    *                 Initialization code of Pcl driver for the print server.
**    *
**    *  Created:	1/30/95
**    *
**    *********************************************************
** 
********************************************************************/
/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Pcl.h"

#include "fb.h"
#include <X11/Xos.h>	/* for unlink() */

#include "attributes.h"
#include "DiPrint.h"

#define MODELDIRNAME "/models"

static void AllocatePclPrivates(ScreenPtr pScreen);
static int PclInitContext(XpContextPtr pCon);
static Bool PclDestroyContext(XpContextPtr pCon);

DevPrivateKey PclScreenPrivateKey = &PclScreenPrivateKey;
DevPrivateKey PclContextPrivateKey = &PclContextPrivateKey;
DevPrivateKey PclPixmapPrivateKey = &PclPixmapPrivateKey;
DevPrivateKey PclWindowPrivateKey = &PclWindowPrivateKey;
DevPrivateKey PclGCPrivateKey = &PclGCPrivateKey;

#ifdef XP_PCL_COLOR
/*
 * The supported visuals on this screen
 */
static VisualRec Visuals[] = 
{
    { 1, StaticGray, 1, 2, 1, 0, 0, 0, 0, 0, 0 },
    { 2, PseudoColor, 8, 256, 8, 0, 0, 0, 0, 0, 0 },
    { 3, TrueColor, 8, 256, 24, 0xFF0000, 0xFF00, 0xFF, 16, 8, 0 }
};

/*
 * The supported depths on this screen
 */
static DepthRec Depths[] = 
{
    { 1, 1, NULL },
    { 8, 1, NULL },
    { 24, 1, NULL }
};
#else
/*
 * The supported visuals on this screen
 */
static VisualRec Visuals[] = 
{
    { 1, StaticGray, 1, 2, 1, 0, 0, 0, 0, 0, 0}
};

/*
 * The supported depths on this screen
 */
static DepthRec Depths[] = 
{
    { 1, 1, NULL }
};
#endif /* XP_PCL_COLOR */


#define NUM_VISUALS(visuals) (sizeof(visuals) / sizeof(VisualRec))
#define NUM_DEPTHS(depths) (sizeof(depths) / sizeof(DepthRec))

Bool
PclCloseScreen(int index,
	       ScreenPtr pScreen)
{
    PclScreenPrivPtr pPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&pScreen->devPrivates, PclScreenPrivateKey);

    pScreen->CloseScreen = pPriv->CloseScreen;
    xfree( pPriv );
    
    return (*pScreen->CloseScreen)(index, pScreen);
}

Bool
InitializePclDriver(
     int ndx,
     ScreenPtr pScreen,
     int argc,
     char **argv)
{
    int maxRes, xRes, yRes, maxDim;
    unsigned i;
    PclScreenPrivPtr pPriv;
    
    /*
     * Register this driver's InitContext function with the print
     * extension.  This is a bit sleazy, as the extension hasn't yet
     * been initialized, but the extensionneeds to know this, and this
     * seems the best time to provide the information.
     */
#ifdef XP_PCL_COLOR
    XpRegisterInitFunc( pScreen, "XP-PCL-COLOR", PclInitContext );
#elif XP_PCL_MONO
    XpRegisterInitFunc( pScreen, "XP-PCL-MONO", PclInitContext );
#else
    XpRegisterInitFunc( pScreen, "XP-PCL-LJ3", PclInitContext );
#endif /* XP_PCL_MONO */
    
    /*
     * Create and fill in the devPrivate for the PCL driver.
     */
    AllocatePclPrivates(pScreen);
   
    pPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&pScreen->devPrivates, PclScreenPrivateKey);

    maxDim = MAX( pScreen->height, pScreen->width );
    xRes = pScreen->width / ( pScreen->mmWidth / 25.4 );
    yRes = pScreen->height / ( pScreen->mmHeight / 25.4 );
    maxRes = MAX( xRes, yRes );

#ifdef XP_PCL_COLOR
    fbScreenInit( pScreen, NULL, maxDim, maxDim, maxRes, maxRes,
		  maxRes, 8 ); /* XXX what's the depth here? */
    /* Clean up the fields that we stomp (code taken from fbCloseScreen) */
    for( i = 0; (int) i < pScreen->numDepths; i++ )
      xfree( pScreen->allowedDepths[i].vids );
    xfree( pScreen->allowedDepths );
    xfree( pScreen->visuals );
#else
    fbScreenInit( pScreen, NULL, maxDim, maxDim, maxRes, maxRes,
		  maxRes, 1 );
#endif /* XP_PCL_COLOR */

    miInitializeBackingStore ( pScreen );

    pScreen->defColormap = FakeClientID(0);
    pScreen->blackPixel = 1;
    pScreen->whitePixel = 0;

    pPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = PclCloseScreen;
    
    pScreen->QueryBestSize = (QueryBestSizeProcPtr)PclQueryBestSize;
    pScreen->SaveScreen = (SaveScreenProcPtr)_XpBoolNoop;
    pScreen->GetImage = (GetImageProcPtr)_XpVoidNoop;
    pScreen->GetSpans = (GetSpansProcPtr)_XpVoidNoop;
    pScreen->CreateWindow = PclCreateWindow;
    pScreen->DestroyWindow = PclDestroyWindow;
/*
    pScreen->PositionWindow = PclPositionWindow;
*/
    pScreen->ChangeWindowAttributes = PclChangeWindowAttributes;
/*
    pScreen->RealizeWindow = PclMapWindow;
    pScreen->UnrealizeWindow = PclUnmapWindow;
*/
    pScreen->CopyWindow = PclCopyWindow; /* XXX Hard routine to write! */

    pScreen->CreatePixmap = fbCreatePixmap;
    pScreen->DestroyPixmap = fbDestroyPixmap;
    pScreen->RealizeFont = PclRealizeFont;
    pScreen->UnrealizeFont = PclUnrealizeFont;
    pScreen->CreateGC = PclCreateGC;

    pScreen->CreateColormap = PclCreateColormap;
    pScreen->DestroyColormap = PclDestroyColormap;
    pScreen->InstallColormap = (InstallColormapProcPtr)NoopDDA;
    pScreen->UninstallColormap = (UninstallColormapProcPtr)NoopDDA;
    pScreen->ListInstalledColormaps = PclListInstalledColormaps;
    pScreen->StoreColors = PclStoreColors;
/*
    pScreen->ResolveColor = PclResolveColor;
*/

    pScreen->BitmapToRegion = fbPixmapToRegion;

    pScreen->ConstrainCursor = PclConstrainCursor;
    pScreen->CursorLimits = PclCursorLimits;
    pScreen->DisplayCursor = PclDisplayCursor;
    pScreen->RealizeCursor = PclRealizeCursor;
    pScreen->UnrealizeCursor = PclUnrealizeCursor;
    pScreen->RecolorCursor = PclRecolorCursor;
    pScreen->SetCursorPosition = PclSetCursorPosition;

    pScreen->visuals = Visuals;
    pScreen->numVisuals = NUM_VISUALS( Visuals );
    pScreen->allowedDepths = Depths;
    pScreen->numDepths = NUM_DEPTHS( Depths );

    for( i = 0; i < NUM_DEPTHS( Depths ); i++ )
      {
	  pScreen->allowedDepths[i].vids =
	    (VisualID *)xalloc( sizeof(VisualID ) );
	  pScreen->allowedDepths[i].vids[0] = i + 1;
      }
    
#ifdef XP_PCL_COLOR
    pScreen->rootVisual = 2;
    pScreen->rootDepth = 8;
#else
    pScreen->rootVisual = 1;
    pScreen->rootDepth = 1;
#endif /* XP_PCL_COLOR */

    pPriv->colormaps = NULL;
    PclCreateDefColormap( pScreen );
    
    return TRUE;
}

static void
AllocatePclPrivates(ScreenPtr pScreen)
{
    dixRequestPrivate(PclWindowPrivateKey, sizeof( PclWindowPrivRec ) );
    dixRequestPrivate(PclContextPrivateKey, sizeof( PclContextPrivRec ) );
    dixRequestPrivate(PclGCPrivateKey, sizeof( PclGCPrivRec ) );
    dixRequestPrivate(PclPixmapPrivateKey, sizeof( PclPixmapPrivRec ) );

    dixSetPrivate(&pScreen->devPrivates, PclScreenPrivateKey,
		  xalloc(sizeof(PclScreenPrivRec)));
}

/*
 * PclInitContext
 *
 * Establish the appropriate values for a PrintContext used with the PCL
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
PclInitContext(XpContextPtr pCon)
{
    XpDriverFuncsPtr pFuncs;
    PclContextPrivPtr pConPriv;
    char *server, *attrStr;
    char *modelID;
    char *configDir;
    char *pathName;
    int i, j;
    float width, height;
    XpOidMediumDiscreteSizeList* ds_list;
    XpOidArea* repro;
    XpOid page_size;
    XpOidMediumSS* m;
    
    /*
     * Initialize the attribute store for this printer.
     */
    XpInitAttributes( pCon );

    /*
     * Initialize the function pointers
     */
    pFuncs = &( pCon->funcs );
    pFuncs->StartJob = PclStartJob;
    pFuncs->EndJob = PclEndJob;
    pFuncs->StartDoc = PclStartDoc;
    pFuncs->EndDoc = PclEndDoc;
    pFuncs->StartPage = PclStartPage;
    pFuncs->EndPage = PclEndPage;
    pFuncs->PutDocumentData = PclDocumentData;
    pFuncs->GetDocumentData = PclGetDocumentData;
    pFuncs->GetAttributes = PclGetAttributes;
    pFuncs->SetAttributes = PclSetAttributes;
    pFuncs->AugmentAttributes = PclAugmentAttributes;
    pFuncs->GetOneAttribute = PclGetOneAttribute;
    pFuncs->DestroyContext = PclDestroyContext;
    pFuncs->GetMediumDimensions = PclGetMediumDimensions;
    pFuncs->GetReproducibleArea = PclGetReproducibleArea;
    

    /*
     * Set up the context privates
     */
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    
    pConPriv->jobFileName = (char *)NULL;
    pConPriv->pageFileName = (char *)NULL;
    pConPriv->pJobFile = (FILE *)NULL;
    pConPriv->pPageFile = (FILE *)NULL;
    pConPriv->dash = NULL;
    pConPriv->validGC = 0;

    pConPriv->getDocClient = (ClientPtr)NULL;
    pConPriv->getDocBufSize = 0;
    modelID = XpGetOneAttribute(pCon, XPPrinterAttr, "xp-model-identifier");
    if ( (configDir = XpGetConfigDir(False)) != (char *) NULL ) {
	pathName = (char *)xalloc(strlen(configDir) + strlen(MODELDIRNAME) +
				strlen(modelID) + strlen("color.map") + 4);
	if (pathName) {
	    sprintf(pathName, "%s/%s/%s/%s", configDir, MODELDIRNAME, modelID,
				"color.map");
	    pConPriv->ctbl = PclReadMap(pathName, &pConPriv->ctbldim);
	    xfree(pathName);

	} else
	    pConPriv->ctbl = NULL;
    } else
	pConPriv->ctbl = NULL;

#ifdef XP_PCL_LJ3
    /*
     * Initialize the spooling buffer for saving the figures temporary
     * (LaserJet IIIs printers don't support the macro function which
     *  includes some HP-GL/2 commands.)
     */
    pConPriv->fcount = 0;
    if ( !(pConPriv->figures = (char *)xalloc(1024)) )
	pConPriv->fcount_max = 0;
    else
	pConPriv->fcount_max = 1024;
#endif /* XP_PCL_LJ3 */
    
    /*
     * document-attributes-supported
     */
    server = XpGetOneAttribute( pCon, XPServerAttr, DOC_ATT_SUPP );
    if( ( attrStr = (char *)xalloc(strlen(server) + strlen(DOC_ATT_SUPP)
				   + strlen(DOC_ATT_VAL) +
				   strlen(PAGE_ATT_VAL) + 8 ) ) 
       == (char *)NULL )
      return BadAlloc;
    sprintf( attrStr, "*%s:\t%s %s %s", DOC_ATT_SUPP, server,
	    DOC_ATT_VAL, PAGE_ATT_VAL );
    XpAugmentAttributes( pCon, XPPrinterAttr, attrStr );
    xfree( attrStr );
    
    /*
     * job-attributes-supported
     */
    server = XpGetOneAttribute( pCon, XPServerAttr, JOB_ATT_SUPP );
    if( ( attrStr = (char *)xalloc(strlen(server) + strlen(JOB_ATT_SUPP)
				   + strlen(JOB_ATT_VAL) + 8 ) ) 
       == (char *)NULL )
      return BadAlloc;
    sprintf( attrStr, "*%s:\t%s %s", JOB_ATT_SUPP, server, JOB_ATT_VAL );
    XpAugmentAttributes( pCon, XPPrinterAttr, attrStr );
    xfree( attrStr );
    
    /*
     * xp-page-attributes-supported
     */
    server = XpGetOneAttribute( pCon, XPServerAttr, PAGE_ATT_SUPP );
    if( ( attrStr = (char *)xalloc(strlen(server) + strlen(PAGE_ATT_SUPP)
				   + strlen(PAGE_ATT_VAL) + 8 ) ) 
       == (char *)NULL )
      return BadAlloc;
    sprintf( attrStr, "*%s:\t%s %s", PAGE_ATT_SUPP, server, PAGE_ATT_VAL );
    XpAugmentAttributes( pCon, XPPrinterAttr, attrStr );
    xfree( attrStr );

    /*
     * Validate the attribute pools
     */
    XpValidateAttributePool( pCon, XPPrinterAttr, &PclValidatePoolsRec );

    /*
     * Munge the reproducible areas to reflect the fact that PCL will not let
     * me move the right or left margins closer than .25" to the edge of the
     * paper.
     */
    m = XpGetMediumSSAttr( pCon, XPPrinterAttr,
			  xpoid_att_medium_source_sizes_supported, 
			  (const XpOidList*) NULL,
			  (const XpOidList*) NULL );
    for( i = 0; i < XpOidMediumSSCount( m ); i++ )
      {
	  if( XpOidMediumSS_DISCRETE == (m->mss)[i].mstag )
	    {
		ds_list = (m->mss)[i].ms.discrete;
		for( j = 0; j < ds_list->count; j++ )
		  {
		      repro = &(ds_list->list)[j].assured_reproduction_area;
		      page_size = (ds_list->list)[j].page_size;
		      XpGetMediumMillimeters( page_size, &width, &height );
		      
		      if( repro->minimum_x < 6.35 )
			repro->minimum_x = 6.35;
		      if( width - repro->maximum_x < 6.35 )
			repro->maximum_x = width - 6.35;
		  }
	    }
      }
    XpPutMediumSSAttr( pCon, XPPrinterAttr,
		      xpoid_att_medium_source_sizes_supported, m );
    XpOidMediumSSDelete( m );

    /*
     * Finish validating the attribute pools
     */

    XpValidateAttributePool( pCon, XPDocAttr, &PclValidatePoolsRec );
    XpValidateAttributePool( pCon, XPJobAttr, &PclValidatePoolsRec );
    XpValidateAttributePool( pCon, XPPageAttr, &PclValidatePoolsRec );

    /*
     * Clear out the colormap storage
     */
    pConPriv->palettes = NULL;
    
    return Success;
}

static Bool
PclDestroyContext(XpContextPtr pCon)
{
    PclContextPrivPtr pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    PclPaletteMapPtr p, t;
    PclCmapToContexts *pCmap;
    ScreenPtr screen;
    PclScreenPrivPtr sPriv;
    PclContextListPtr con, prevCon, temp;
    
    
    /*
     * Clean up the temporary files
     */
    if( pConPriv->pPageFile != (FILE *)NULL )
      {
	  fclose( pConPriv->pPageFile );
	  pConPriv->pPageFile = (FILE *)NULL;
      }
    if( pConPriv->pageFileName != (char *)NULL )
      {
	  unlink( pConPriv->pageFileName );
	  xfree( pConPriv->pageFileName );
	  pConPriv->pageFileName = (char *)NULL;
      }
    
    if( pConPriv->pJobFile != (FILE *)NULL )
      {
	  fclose( pConPriv->pJobFile );
	  pConPriv->pJobFile = NULL;
      }
    if( pConPriv->jobFileName != (char *)NULL )
      {
	  unlink( pConPriv->jobFileName );
	  xfree( pConPriv->jobFileName );
	  pConPriv->jobFileName = (char *)NULL;
      }

    xfree( pConPriv->dash );
    xfree(pConPriv->ctbl);
    pConPriv->ctbl = NULL;
#ifdef XP_PCL_LJ3
    xfree( pConPriv->figures );
#endif /* XP_PCL_LJ3 */

    /*
     * Destroy the colormap<->palette mappings
     */
    p = pConPriv->palettes;
    while( p )
      {
	  t = p;
	  p = p->next;
	  xfree( t );
      }
    pConPriv->palettes = NULL;

    /*
     * Remove the context from the screen-level colormap<->contexts mappings
     */
    screen = screenInfo.screens[pCon->screenNum];
    sPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&screen->devPrivates, PclScreenPrivateKey);
    pCmap = sPriv->colormaps;
    while( pCmap )
      {
	  con = pCmap->contexts;
	  prevCon = NULL;
	  
	  while( con )
	    {
		if( con->context->contextID == pCon->contextID )
		  {
		      if( prevCon )
			{
			    temp = con;
			    prevCon->next = con = con->next;
			}
		      else
			{
			    temp = pCmap->contexts;
			    pCmap->contexts = con = con->next;
			}
		      xfree( temp );
		  }
		else
		  con = con->next;
	    }

	  pCmap = pCmap->next;
      }
    
    XpDestroyAttributes(pCon);

    return Success;
}

XpContextPtr
PclGetContextFromWindow(WindowPtr win)
{
    PclWindowPrivPtr pPriv;
    
    while( win )
      {
	  pPriv = (PclWindowPrivPtr)
	      dixLookupPrivate(&win->devPrivates, PclWindowPrivateKey);
	  if( pPriv->validContext )
	    return pPriv->context;
      
	  win = win->parent;
      }
    
    return NULL;
}
