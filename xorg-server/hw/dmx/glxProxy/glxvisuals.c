/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#include <assert.h>
#include "dmx.h"
#include "glxserver.h"
#include "glxutil.h"
#include "dmx_glxvisuals.h"

#undef Xmalloc
#undef Xcalloc
#undef Xrealloc
#undef Xfree


static int                 numConfigs     = 0;
static __GLXvisualConfig  *visualConfigs  = NULL;
static void              **visualPrivates = NULL;

int glxVisualsMatch( __GLXvisualConfig *v1, __GLXvisualConfig *v2 )
{
      if ( (v1->class == v2->class) &&
           (v1->rgba == v2->rgba) &&
	   (v1->redSize == v2->redSize) &&
	   (v1->greenSize == v2->greenSize) &&
	   (v1->blueSize == v2->blueSize) &&
	   (v1->alphaSize == v2->alphaSize) &&
	   (v1->redMask == v2->redMask) &&
	   (v1->greenMask == v2->greenMask) &&
	   (v1->blueMask == v2->blueMask) &&
	   (v1->alphaMask == v2->alphaMask) &&
	   (v1->accumRedSize == v2->accumRedSize) &&
	   (v1->accumGreenSize == v2->accumGreenSize) &&
	   (v1->accumBlueSize == v2->accumBlueSize) &&
	   (v1->accumAlphaSize == v2->accumAlphaSize) &&
	   (v1->doubleBuffer == v2->doubleBuffer) &&
	   (v1->stereo == v2->stereo) &&
	   (v1->bufferSize == v2->bufferSize) &&
	   (v1->depthSize == v2->depthSize) &&
	   (v1->stencilSize == v2->stencilSize) &&
	   (v1->auxBuffers == v2->auxBuffers) &&
	   (v1->level == v2->level) &&
	   (v1->visualRating == v2->visualRating) &&
	   (v1->transparentPixel == v2->transparentPixel) &&
	   (v1->transparentRed == v2->transparentRed) &&
	   (v1->transparentGreen == v2->transparentGreen) &&
	   (v1->transparentBlue == v2->transparentBlue) &&
	   (v1->transparentAlpha == v2->transparentAlpha) &&
	   (v1->transparentIndex == v2->transparentIndex) &&
	   (v1->multiSampleSize == v2->multiSampleSize) &&
	   (v1->nMultiSampleBuffers == v2->nMultiSampleBuffers) &&
	   (v1->visualSelectGroup == v2->visualSelectGroup)         ) {

	      return(1);

      }

      return(0);

}

VisualID glxMatchGLXVisualInConfigList( __GLXvisualConfig *pGlxVisual, __GLXvisualConfig *configs, int nconfigs )
{
    int i;

    for (i=0; i<nconfigs; i++) {

       if (glxVisualsMatch( pGlxVisual, &configs[i] )) {

	  return( configs[i].vid );

       }
    }

    return(0);
}

VisualID glxMatchVisualInConfigList( ScreenPtr pScreen, VisualPtr pVisual, __GLXvisualConfig *configs, int nconfigs )
{
    __GLXscreenInfo *pGlxScreen;
    __GLXvisualConfig *pGlxVisual;
    int i;

    /* check that the glx extension has been initialized */
    if ( !__glXActiveScreens ) 
       return(0);

    pGlxScreen = &__glXActiveScreens[pScreen->myNum];
    pGlxVisual = pGlxScreen->pGlxVisual;

    /* find the glx visual info for pVisual */
    for (i = 0; i < pGlxScreen->numVisuals; i++, pGlxVisual++) {
	if (pGlxVisual->vid == pVisual->vid) {
	    break;
	}
    }
    if (i == pGlxScreen->numVisuals) {
	/*
	 * the visual is not supported by glx
	 */
        return(0);
    }

    return( glxMatchGLXVisualInConfigList(pGlxVisual, configs, nconfigs) );
}

VisualPtr glxMatchVisual( ScreenPtr pScreen, VisualPtr pVisual, ScreenPtr pMatchScreen )
{
    __GLXscreenInfo *pGlxScreen2;
    int j;
    VisualID vid;

    /* check that the glx extension has been initialized */
    if ( !__glXActiveScreens ) 
       return NULL;

    pGlxScreen2 = &__glXActiveScreens[pMatchScreen->myNum];

    vid = glxMatchVisualInConfigList( pScreen, pVisual,
                                      pGlxScreen2->pGlxVisual,
				      pGlxScreen2->numVisuals );
    if (vid) {
       /*
    	* find the X visual of the matching glx visual
	*/
       for (j=0; j<pMatchScreen->numVisuals; j++) {
	  if (vid == pMatchScreen->visuals[j].vid) {
	     return( &pMatchScreen->visuals[j] );
	  }
       }
    }

    return(0);
}

void glxSetVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
                 void **privates)
{
    numConfigs = nconfigs;
    visualConfigs = configs;
    visualPrivates = privates;
}

static int count_bits(unsigned int n)
{
   int bits = 0;

   while (n > 0) {
      if (n & 1) bits++;
      n >>= 1;
   }
   return bits;
}

static VisualID FindClosestVisual( VisualPtr pVisual, int rootDepth, 
                                   DepthPtr pdepth, int ndepths,
				   VisualPtr pNewVisual, int numNewVisuals)
{
   int d, v;
   VisualPtr vis;

   /*
    * find the first visual with the same or deeper depth
    * of the same class.
    */
   for (d=0; d<ndepths; d++) {
      if (pdepth[d].depth >= rootDepth) {
	 for (v=0; v<pdepth[d].numVids; v++) {

	    /* find the new visual structure */
	    vis = pNewVisual;
	    while( pdepth[d].vids[v] != vis->vid ) vis++;

	    if (vis->class == pVisual->class) {
	       return( pdepth[d].vids[v] );
	    }
	 }
      }
   }

   /*
    * did not find any.
    * try to look for the same class only.
    */
   for (d=0; d<ndepths; d++) {
      for (v=0; v<pdepth[d].numVids; v++) {

	 /* find the new visual structure */
      	 vis = pNewVisual;
	 while( pdepth[d].vids[v] != vis->vid ) vis++;

      	 if (vis->class == pVisual->class) {
	    return( pdepth[d].vids[v] );
	 }
      }
   }

   /*
    * if not found - just take the first visual
    */
   return( pdepth[0].vids[0] );
}

Bool glxInitVisuals(int *nvisualp, VisualPtr *visualp,
			 VisualID *defaultVisp,
			 int ndepth, DepthPtr pdepth,
			 int rootDepth)
{
    int numRGBconfigs;
    int numCIconfigs;
    int numVisuals = *nvisualp;
    int numNewVisuals;
    int numNewConfigs;
    VisualPtr pVisual = *visualp;
    VisualPtr pVisualNew = NULL;
    VisualID *orig_vid = NULL;
    __GLXvisualConfig *glXVisualPtr = NULL;
    __GLXvisualConfig *pNewVisualConfigs = NULL;
    void **glXVisualPriv;
    dmxGlxVisualPrivate **pNewVisualPriv;
    int found_default;
    int i, j, k;
    int numGLXvis = 0;
    GLint *isGLXvis;

    if (numConfigs > 0)
        numNewConfigs = numConfigs;
    else
        return False;

    MAXSCREENSALLOC(__glXActiveScreens);
    if (!__glXActiveScreens)
        return False;

    /* Alloc space for the list of new GLX visuals */
    pNewVisualConfigs = (__GLXvisualConfig *)
                     __glXMalloc(numNewConfigs * sizeof(__GLXvisualConfig));
    if (!pNewVisualConfigs) {
	return FALSE;
    }

    /* Alloc space for the list of new GLX visual privates */
    pNewVisualPriv = (dmxGlxVisualPrivate **) __glXMalloc(numNewConfigs * sizeof(dmxGlxVisualPrivate *));
    if (!pNewVisualPriv) {
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* copy driver's visual config info */
    for (i = 0; i < numConfigs; i++) {
       pNewVisualConfigs[i] = visualConfigs[i];
       pNewVisualPriv[i] = (dmxGlxVisualPrivate *)visualPrivates[i];
    }

#if 1
    /* FIXME: This is a hack to workaround a hang in xtest caused by a
     * mismatch between what the front end (i.e., DMX) server calculates
     * for the visual configs and what the back-end servers have.
     */
    {
	int numTCRGBconfigs = 0;
	int numDCRGBconfigs = 0;

	numRGBconfigs = 0;
	numCIconfigs = 0;

	for (i = 0; i < numNewConfigs; i++) {
	    if (pNewVisualConfigs[i].rgba) {
		if (pNewVisualConfigs[i].class == TrueColor)
		    numTCRGBconfigs++;
		else
		    numDCRGBconfigs++;
		numRGBconfigs++;
	    } else
		numCIconfigs++;
	}

	/* Count the total number of visuals to compute */
	numNewVisuals = 0;
	for (i = 0; i < numVisuals; i++) {
	    numNewVisuals +=
		(pVisual[i].class == TrueColor)   ? numTCRGBconfigs :
		(pVisual[i].class == DirectColor) ? numDCRGBconfigs :
						    numCIconfigs;
	}
    }
#else
    /* Count the number of RGB and CI visual configs */
    numRGBconfigs = 0;
    numCIconfigs = 0;
    for (i = 0; i < numNewConfigs; i++) {
	if (pNewVisualConfigs[i].rgba)
	    numRGBconfigs++;
	else
	    numCIconfigs++;
    }

    /* Count the total number of visuals to compute */
    numNewVisuals = 0;
    for (i = 0; i < numVisuals; i++) {
        numNewVisuals +=
	    (pVisual[i].class == TrueColor || pVisual[i].class == DirectColor)
	    ? numRGBconfigs : numCIconfigs;
    }
#endif

    /* Reset variables for use with the next screen/driver's visual configs */
    visualConfigs = NULL;
    numConfigs = 0;

    /* Alloc temp space for the list of orig VisualIDs for each new visual */
    orig_vid = (VisualID *)__glXMalloc(numNewVisuals * sizeof(VisualID));
    if (!orig_vid) {
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the list of glXVisuals */
    glXVisualPtr = (__GLXvisualConfig *)__glXMalloc(numNewVisuals *
						    sizeof(__GLXvisualConfig));
    if (!glXVisualPtr) {
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the list of glXVisualPrivates */
    glXVisualPriv = (void **)__glXMalloc(numNewVisuals * sizeof(void *));
    if (!glXVisualPriv) {
	__glXFree(glXVisualPtr);
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the new list of the X server's visuals */
    pVisualNew = (VisualPtr)__glXMalloc(numNewVisuals * sizeof(VisualRec));
    if (!pVisualNew) {
	__glXFree(glXVisualPriv);
	__glXFree(glXVisualPtr);
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    isGLXvis = (GLint *) __glXMalloc(numNewVisuals * sizeof(GLint));
    if (!isGLXvis) {
	__glXFree(glXVisualPriv);
	__glXFree(glXVisualPtr);
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	__glXFree(pVisualNew);
	return FALSE;
    }

    /* Initialize the new visuals */
    found_default = FALSE;
    for (i = j = 0; i < numVisuals; i++) {

	for (k = 0; k < numNewConfigs; k++) {

	    int new_depth;
	    int depth;
	    int d,v;

	    /* find the depth of the new visual config */
	    new_depth = pNewVisualPriv[k]->x_visual_depth;

	    /* find the depth of the original visual */
	    depth = 0;
	    d = 0;
	    while( (depth==0) && (d < ndepth) ) {
	       v = 0;
	       while( (depth==0) && (v < pdepth[d].numVids) ) {
		  if (pdepth[d].vids[v] ==  pVisual[i].vid) {
		     depth = pdepth[d].depth;
		  }
		  v++;
	       }
	       d++;
	    }

	    /* check that the visual has the same class and depth 
	     * as the new config
             */
	    if ( pVisual[i].class != pNewVisualPriv[k]->x_visual_class ||
		  (depth != new_depth) )
		continue;

	    /* Initialize the new visual */
	    pVisualNew[j] = pVisual[i];
	    pVisualNew[j].vid = FakeClientID(0);

	    /* Check for the default visual */
	    if (!found_default && pVisual[i].vid == *defaultVisp) {
		*defaultVisp = pVisualNew[j].vid;
		found_default = TRUE;
	    }

	    /* Save the old VisualID */
	    orig_vid[j] = pVisual[i].vid;

	    /* Initialize the glXVisual */
	    glXVisualPtr[j] = pNewVisualConfigs[k];
	    glXVisualPtr[j].vid = pVisualNew[j].vid;

	    /*
	     * If the class is -1, then assume the X visual information
	     * is identical to what GLX needs, and take them from the X
	     * visual.  NOTE: if class != -1, then all other fields MUST
	     * be initialized.
	     */
	    if (glXVisualPtr[j].class == -1) {
		glXVisualPtr[j].class      = pVisual[i].class;
		glXVisualPtr[j].redSize    = count_bits(pVisual[i].redMask);
		glXVisualPtr[j].greenSize  = count_bits(pVisual[i].greenMask);
		glXVisualPtr[j].blueSize   = count_bits(pVisual[i].blueMask);
		glXVisualPtr[j].alphaSize  = glXVisualPtr[j].alphaSize;
		glXVisualPtr[j].redMask    = pVisual[i].redMask;
		glXVisualPtr[j].greenMask  = pVisual[i].greenMask;
		glXVisualPtr[j].blueMask   = pVisual[i].blueMask;
		glXVisualPtr[j].alphaMask  = glXVisualPtr[j].alphaMask;
		glXVisualPtr[j].bufferSize = rootDepth;
	    }

	    /* Save the device-dependent private for this visual */
	    glXVisualPriv[j] = pNewVisualPriv[k];

            isGLXvis[j] = glxMatchGLXVisualInConfigList( &glXVisualPtr[j], 
	                           dmxScreens[screenInfo.numScreens-1].glxVisuals,
	                           dmxScreens[screenInfo.numScreens-1].numGlxVisuals );
	    if (isGLXvis[j]) numGLXvis++;

	    j++;
	}
    }

    assert(j <= numNewVisuals);
    numNewVisuals = j;   /* correct number of new visuals */

    /* Save the GLX visuals in the screen structure */
    __glXActiveScreens[screenInfo.numScreens-1].numVisuals = numNewVisuals;
    __glXActiveScreens[screenInfo.numScreens-1].numGLXVisuals = numGLXvis;
    __glXActiveScreens[screenInfo.numScreens-1].isGLXvis = isGLXvis;
    __glXActiveScreens[screenInfo.numScreens-1].pGlxVisual = glXVisualPtr;


    /* Set up depth's VisualIDs */
    for (i = 0; i < ndepth; i++) {
	int numVids = 0;
	VisualID *pVids = NULL;
	int k, n = 0;

	/* Count the new number of VisualIDs at this depth */
	for (j = 0; j < pdepth[i].numVids; j++)
	    for (k = 0; k < numNewVisuals; k++)
		if (pdepth[i].vids[j] == orig_vid[k])
		    numVids++;

	/* Allocate a new list of VisualIDs for this depth */
	pVids = (VisualID *)__glXMalloc(numVids * sizeof(VisualID));

	/* Initialize the new list of VisualIDs for this depth */
	for (j = 0; j < pdepth[i].numVids; j++)
	    for (k = 0; k < numNewVisuals; k++)
		if (pdepth[i].vids[j] == orig_vid[k])
		    pVids[n++] = pVisualNew[k].vid;

	/* Update this depth's list of VisualIDs */
	__glXFree(pdepth[i].vids);
	pdepth[i].vids = pVids;
	pdepth[i].numVids = numVids;
    }

    /*
     * if the default visual was rejected - need to choose new
     * default visual !
     */ 
    if ( !found_default ) {

       for (i=0; i<numVisuals; i++)
	  if (pVisual[i].vid == *defaultVisp)
	     break;

       if (i < numVisuals) {
	  *defaultVisp = FindClosestVisual( &pVisual[i], rootDepth, pdepth, ndepth, pVisualNew, numNewVisuals );
       }
    }

    /* Update the X server's visuals */
    *nvisualp = numNewVisuals;
    *visualp = pVisualNew;

    /* Free the old list of the X server's visuals */
    __glXFree(pVisual);

    /* Clean up temporary allocations */
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);

    /* Free the private list created by DDX HW driver */
    if (visualPrivates)
        xfree(visualPrivates);
    visualPrivates = NULL;

    return TRUE;
}
