/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclPrint.c
**    *
**    *  Contents:	Print extension code of Pcl driver
**    *
**    *  Created:	2/03/95
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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xprotostr.h>

#define NEED_EVENTS
#include <X11/Xproto.h>
#undef NEED_EVENTS

#include "Pcl.h"

#include "windowstr.h"
#include "attributes.h"
#include "AttrValid.h"
#include "Oid.h"

int
PclStartJob(
     XpContextPtr pCon,
     Bool sendClientData,
     ClientPtr client)
{
    PclContextPrivPtr pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    PclPaletteMap *pal;
    
    /*
     * Check for existing page file, and delete it if it exists.
     */
    if(pConPriv->pageFileName != (char *)NULL)
    {
	if(pConPriv->pPageFile != (FILE *)NULL)
	{
	    fclose(pConPriv->pPageFile);
	    pConPriv->pPageFile = (FILE *)NULL;
	}
	unlink(pConPriv->pageFileName);
	xfree(pConPriv->pageFileName);
	pConPriv->pageFileName = (char *)NULL;
    }

    /* 
     * Create a temporary file to store the printer output.
     */
    if (!XpOpenTmpFile("w+", &pConPriv->jobFileName, &pConPriv->pJobFile))
	return BadAlloc;

    /*
     * Create/Initialize the SoftFontInfo structure
     */
    pConPriv->pSoftFontInfo = PclCreateSoftFontInfo();

    /*
     * Set up the colormap handling
     */
    pConPriv->palettes = NULL;
    pConPriv->nextPaletteId = 4;
    pConPriv->currentPalette = 0;

    pal = &( pConPriv->staticGrayPalette );
    pal->paletteId = 1;
    pal->downloaded = 0;
    
    pal = &( pConPriv->trueColorPalette );
    pal->paletteId = 2;
    pal->downloaded = 0;
    
    pal = &( pConPriv->specialTrueColorPalette );
    pal->paletteId = 3;
    pal->downloaded = 0;

    return Success;
}

int
PclEndJob(
     XpContextPtr pCon,
     Bool cancel)
{
    PclContextPrivPtr priv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);

#ifdef CCP_DEBUG
    FILE *xpoutput;
#endif
    FILE *fp;
    int retVal;
    char *fileName, *trailer;
    struct stat statBuf;
    PclPaletteMapPtr p;

    trailer = "\033%-12345X@PJL RESET\n";

    if( cancel == True )
      {
	  if( priv->getDocClient != (ClientPtr)NULL ) {
	      XpFinishDocData( priv->getDocClient );

	      priv->getDocClient = NULL;
	      priv->getDocBufSize = 0;
	  }

	  return Success;
      }
    
    if( priv->getDocClient != (ClientPtr)NULL && priv->getDocBufSize > 0  )
      {
	  /*
	   * We need to stash the trailer information somewhere...
	   */
	  if (!XpOpenTmpFile("w+", &fileName, &fp))
	      return BadAlloc;

#ifndef XP_PCL_LJ3
	  SEND_PCL( fp, trailer );
	  rewind( fp );

	  retVal = XpSendDocumentData( priv->getDocClient, fp,
				      strlen( trailer ),
				      priv->getDocBufSize );
#endif /* XP_PCL_LJ3 */

	  fclose( fp );
	  unlink( fileName );
	  xfree( fileName );

	  if( priv->getDocClient != (ClientPtr)NULL ) {
	      XpFinishDocData( priv->getDocClient );

	      priv->getDocClient = NULL;
	      priv->getDocBufSize = 0;
	  }

	  return retVal;
      }
    
#ifndef XP_PCL_LJ3
    SEND_PCL( priv->pJobFile, trailer );
#endif /* XP_PCL_LJ3 */
    
    /*
     * Submit the job to the spooler
     */
    fflush( priv->pJobFile );
    
    /*
     * Dump the job file to another output file, for testing
     * purposes.
     */
    rewind( priv->pJobFile );
    stat( priv->jobFileName, &statBuf );
    
#ifdef CCP_DEBUG
    unlink( "/users/prince/XpOutput" );
    xpoutput = fopen( "/users/prince/XpOutput", "w" );
    
    rewind( priv->pJobFile );
    TransferBytes( priv->pJobFile, xpoutput,
		      (int)statBuf.st_size );
    fclose( xpoutput );
#endif
    
    XpSubmitJob( priv->jobFileName, pCon );
    fclose( priv->pJobFile );
    unlink( priv->jobFileName );
    xfree( priv->jobFileName );
    priv->jobFileName = NULL;

    PclDestroySoftFontInfo(priv->pSoftFontInfo);
    priv->pSoftFontInfo = (PclSoftFontInfoPtr) NULL;

    /*
     * Clear out the colormap cache
     */
    p = priv->palettes;
    while( p )
      {
	  p->downloaded = 0;
	  p = p->next;
      }

    return Success;
}

/* StartPage 
 *
 * If page file exists
 *     close page file
 *     set page file pointer = NULL
 *     unlink page file
 * Create a new page file
 *     Send the page header information to the page file
 * ClearArea the window and all descendant windows
 */
int
PclStartPage(
     XpContextPtr pCon,
     WindowPtr pWin)
{
    PclContextPrivPtr pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    PclWindowPrivPtr pWinPriv = (PclWindowPrivPtr)
	dixLookupPrivate(&pWin->devPrivates, PclWindowPrivateKey);
    xRectangle repro;
    char t[80];
    XpOid orient, plex, tray, medium;
    int dir, plexNum, num;
    
    /*
     * Put a pointer to the context in the window private structure
     */
    pWinPriv->validContext = 1;
    pWinPriv->context = pCon;
    
    /*
     * Clear out the old page file, if necessary
     */
    if(pConPriv->pPageFile != (FILE *)NULL)
    {
	fclose(pConPriv->pPageFile);
	pConPriv->pPageFile = (FILE *)NULL;
    }
    if(pConPriv->pageFileName != (char *)NULL)
    {
	unlink(pConPriv->pageFileName);
	pConPriv->pageFileName = (char *)NULL;
    }

    /*
     * Make up a new page file.
     */
    if (!XpOpenTmpFile("w+", &pConPriv->pageFileName, &pConPriv->pPageFile))
	return BadAlloc;

    /*
     * Reset the GC cached in the context private struct.
     */
    pConPriv->validGC = 0;

    /*
     * Set the page orientation
     */
    orient = XpGetContentOrientation( pCon );
    switch( orient )
      {
	case xpoid_val_content_orientation_landscape:
	  dir = 1;
	  break;
	case xpoid_val_content_orientation_reverse_portrait:
	  dir = 2;
	  break;
	case xpoid_val_content_orientation_reverse_landscape:
	  dir = 3;
	  break;
	case xpoid_val_content_orientation_portrait:
	default:
	  dir = 0;
	  break;
      }
    sprintf( t, "\033&l%dO", dir );
    SEND_PCL( pConPriv->pPageFile, t );
    
    /*
     * Set the duplexing method.  Since PCL wants to think of it in
     * terms of the "binding edge," and the attribute store thinks in
     * "duplex/tumble," this is a little complicated.
     *
     * Actually, this has no bearing on the output, since the HP1600C
     * will only print on one side of the paper, and ignore all
     * requests to enable duplexing.  But, in an attempt to keep this
     * driver somewhat generic, we'll enable it anyway.
     */
    plex = XpGetPlex( pCon );
    
    if( plex == xpoid_val_plex_duplex )
      {
	  if( dir == 0 || dir == 2 )
	    plexNum = 1;
	  else
	    plexNum = 2;
       }
    else if( plex == xpoid_val_plex_tumble )
      {
	  if( dir == 0 || dir == 2 )
	    plexNum = 2;
	  else
	    plexNum = 1;
      }
    else
      plexNum = 0;
    sprintf( t, "\033&l%dS", plexNum );
    SEND_PCL( pConPriv->pPageFile, t );

    /*
     * Set the input tray or medium.  If XpGetPageSize gives us a valid medium,
     * we can just send that to the printer, and let the printer handle the
     * details.  Otherwise, we select the tray returned from XpGetPageSize,
     * which will be either a tray that should contain the correct medium
     * (possibly with operator intervention), or the default tray from the
     * config files.
     */
    medium = XpGetPageSize( pCon, &tray, NULL );
    if( medium != xpoid_none )
      {
	  switch( medium )
	    {
	      case xpoid_val_medium_size_na_legal:
		num = 3;
		break;
	      case xpoid_val_medium_size_iso_a3:
		num = 27;
		break;
	      case xpoid_val_medium_size_iso_a4:
		num = 26;
		break;
	      case xpoid_val_medium_size_executive:
		num = 1;
		break;
	      case xpoid_val_medium_size_ledger:
		num = 6;
		break;
	      case xpoid_val_medium_size_monarch_envelope:
		num = 80;
		break;
	      case xpoid_val_medium_size_na_number_10_envelope:
		num = 81;
		break;
	      case xpoid_val_medium_size_iso_designated_long:
		num = 90;
		break;
	      case xpoid_val_medium_size_iso_c5:
		num = 91;
		break;
	      case xpoid_val_medium_size_iso_b5:
		num = 100;
		break;
	      case xpoid_val_medium_size_jis_b5:
		num = 45;
		break;
	      case xpoid_val_medium_size_na_letter:
	      default:
		num = 2;
		break;
	    }
	  sprintf( t, "\033&l%dA", num );
	  SEND_PCL( pConPriv->pPageFile, t );
      }
    else
      {
	  switch( tray )
	    {
	      case xpoid_val_input_tray_manual:
		num = 2;
		break;
	      case xpoid_val_input_tray_envelope:
		num = 3;
		break;
	      case xpoid_val_input_tray_large_capacity:
		num = 5;
		break;
	      case xpoid_val_input_tray_bottom:
		num = 4;
		break;
	      case xpoid_val_input_tray_main:
	      default:
		num = 1;
		break;
	    }
	  sprintf( t, "\033&l%dH", num );
	  SEND_PCL( pConPriv->pPageFile, t );
      }
    
    /*
     * Set the scaling factors so that the HP-GL/2 coordinate system
     * matches the X coordinate system, both in axis orientation and
     * in unit<->pixel conversion.
     */
    XpGetReproductionArea( pCon, &repro );

    sprintf( t, "\033&l0E\033*p%dx%dY", repro.x - 75, repro.y );
    SEND_PCL( pConPriv->pPageFile, t );

    sprintf( t, "\033*c%dx%dY\033*c0T", (int)(repro.width / 300.0 * 720.0),
	    (int)(repro.height / 300.0 * 720.0) );
    SEND_PCL( pConPriv->pPageFile, t );
    
    sprintf( t, "\033%%0BSC%d,%d,%d,%d;\033%%0A", repro.x, repro.x +
	    repro.width, repro.y + repro.height, repro.y );
    SEND_PCL( pConPriv->pPageFile, t );

    return Success;
}

/*
 * When sending the generated PCL code back to the client, we send everything
 * that we have generated so far for the job.  After sending the data, we clean
 * out the job file, to avoid repeatedly sending the same data.
 */

static int
SendDocData( PclContextPrivPtr pPriv )
{
    struct stat statBuf;
    int ret;
    
    rewind( pPriv->pJobFile );
    if( stat( pPriv->jobFileName, &statBuf ) < 0 )
      return BadAlloc;
    
    ret = XpSendDocumentData( pPriv->getDocClient, pPriv->pJobFile,
			     (int)statBuf.st_size, pPriv->getDocBufSize );

    /*
     * Clean out the job file
     */
    fclose( pPriv->pJobFile );
    unlink( pPriv->jobFileName );

    xfree(pPriv->jobFileName);

    if (!XpOpenTmpFile("w+", &pPriv->jobFileName, &pPriv->pJobFile))
	return BadAlloc;

    return ret;
}

/*
 * EndPage:
 *
 * Write page trailer to page file
 * Write page file to job file
 */
int
PclEndPage(
     XpContextPtr pCon,
     WindowPtr pWin)
{
    PclContextPrivPtr pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);

    struct stat statBuf;

    /*
     * Send the page trailer to the page file.
     */
    SEND_PCL( pConPriv->pPageFile, "\014" );
    fflush( pConPriv->pPageFile );
    
    /*
     * Write the page file contents to the job file, or to the
     * whatever client has called GetDocumentData.
     *
     * pWinPriv->pPageFile must first be set to the start of the page file.
     */
    rewind(pConPriv->pPageFile);
    if(stat(pConPriv->pageFileName, &statBuf) < 0)
      return BadAlloc;

    if(TransferBytes(pConPriv->pPageFile, pConPriv->pJobFile, 
       (int)statBuf.st_size) != (int)statBuf.st_size)
      return BadAlloc;

    if( pConPriv->getDocClient != (ClientPtr)NULL &&
       pConPriv->getDocBufSize > 0 )
      {
	  return SendDocData( pConPriv );
      }
    
    return Success;
}

/*
 * The PclStartDoc() and PclEndDoc() functions serve basically as NOOP
 * placeholders.  This driver doesn't deal with the notion of multiple
 * documents per page.
 */

int
PclStartDoc(XpContextPtr pCon,
	    XPDocumentType type)
{
    PclContextPrivPtr pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    
#ifndef XP_PCL_LJ3
    /*
     * Set the printer resolution for the page.  Since we can only
     * render color at 300dpi, we just hard-code this.
     */
    SEND_PCL( pConPriv->pJobFile, 
	     "\033%-12345X@PJL SET RESOLUTION = 300\r\n" );
#endif /* XP_PCL_LJ3 */
    
    /*
     * Initialize HP-GL/2
     */
    SEND_PCL( pConPriv->pJobFile, "\033E\033%0BIN,SP1,TR0;\033%0A" );

    /*
     * Stash the type of the document (used by PutDocumentData operation)
     */
    pConPriv->isRaw = (type == XPDocRaw);
    
    return Success;
}

int
PclEndDoc(
     XpContextPtr pCon,
     Bool cancel)
{
    /*
     * XXX What should I do if I get cancel == TRUE?
     */
    return Success;
}

/*
 * PclDocumentData()
 *
 * Hand any pre-generated PDL down to the spool files, formatting it
 * as necessary to fit the given window.
 *
 */

#define DOC_PCL 1
#define DOC_HPGL 2

int
PclDocumentData(
     XpContextPtr pCon,
     DrawablePtr pDraw,
     char *pData,
     int len_data,
     char *pFmt,
     int len_fmt,
     char *pOpt,
     int len_opt,
     ClientPtr client)
{
    int type = 0;
    PclContextPrivPtr pPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    XpOidDocFmtList *formats;
    XpOidDocFmt *f;
    char t[80];
    xRectangle repro;
    
    /*
     * Verify the input format
     */
    formats = XpGetDocFmtListAttr( pCon, XPPrinterAttr,
				  (pPriv->isRaw) ?
				  xpoid_att_xp_raw_formats_supported :
				  xpoid_att_xp_embedded_formats_supported, 
				  NULL );
    f = XpOidDocFmtNew( pFmt );
    if( !XpOidDocFmtListHasFmt( formats, f ) )
      {
	  XpOidDocFmtListDelete( formats );
	  XpOidDocFmtDelete( f );
	  return BadMatch;
      }
    XpOidDocFmtListDelete( formats );
    
    if( !(pPriv->isRaw) )
      {
	  if( !strcmp( f->format, "PCL" ) )
	    type = DOC_PCL;
	  else if( !strcmp( f->format, "HPGL" ) )
	    type = DOC_HPGL;
	  else
	    {
		XpOidDocFmtDelete( f );
		return BadMatch;
	    }
	  
	  switch( type )
	    {
	      case DOC_HPGL:
		/*
		 * Move the picture frame to the appropriate place on the page,
		 * then assume that the embedded code will scale it properly.
		 */
		sprintf( t, "\033&l0E\033*p%dx%dY", 
			pDraw->x - 75,
			pDraw->y );
		SEND_PCL( pPriv->pPageFile, t );
		
		sprintf( t, "\033*c%dx%dY\033*coT",
			(int)( pDraw->width / 300.0 * 720.0 ),
			(int)( pDraw->height / 300.0 * 720.0 ) );
		SEND_PCL( pPriv->pPageFile, t );
		break;
	    }
      }
    
    
    /*
     * Send the data down the pipe
     */
    SEND_PCL_COUNT( pPriv->pPageFile, pData, len_data );
    
    /*
     * If it's not a raw document, clean up the embedding
     */
    if( !(pPriv->isRaw) )
      switch( type )
	{
	  case DOC_HPGL:
	    /*
	     * Reset the picture frame
	     */
	    XpGetReproductionArea( pCon, &repro );
	    
	    sprintf( t, "\033&l0E\033*p%dx%dY", repro.x - 75, repro.y );
	    SEND_PCL( pPriv->pPageFile, t );
	    
	    sprintf( t, "\033*c%dx%dY\033*c0T",
		    (int)(repro.width / 300.0 * 720.0),
		    (int)(repro.height / 300.0 * 720.0) );
	    SEND_PCL( pPriv->pPageFile, t );
	    
	    sprintf( t, "\033%%0BSC%d,%d,%d,%d;\033%%0A", repro.x, repro.x +
		    repro.width, repro.y + repro.height, repro.y );
	    SEND_PCL( pPriv->pPageFile, t );
	    break;
	}
    
    XpOidDocFmtDelete( f );
    return Success;
}

/*
 * 
 * PclGetDocumentData()
 *
 * This function allows the driver to send the generated PCL back to
 * the client.
 *
 * XXX This function is barely spec'ed, much less implemented!
 */

int
PclGetDocumentData(
     XpContextPtr pCon,
     ClientPtr client,
     int maxBufferSize)
{
    PclContextPrivPtr pPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    
    pPriv->getDocClient = client;
    pPriv->getDocBufSize = maxBufferSize;
    
    return Success;
}
