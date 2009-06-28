/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#include "glxfbconfig.h"

int AreFBConfigsMatch( __GLXFBConfig *c1, __GLXFBConfig *c2 )
{
   int match;

   match = (
	     (c1->visualType == c2->visualType) &&
	     (c1->transparentType == c2->transparentType) &&
	     (c1->transparentRed == c2->transparentRed) &&
	     (c1->transparentGreen == c2->transparentGreen) &&
	     (c1->transparentBlue == c2->transparentBlue) &&
	     (c1->transparentAlpha == c2->transparentAlpha) &&
	     (c1->transparentIndex == c2->transparentIndex) &&
	     (c1->visualCaveat == c2->visualCaveat) &&
	     (c1->drawableType == c2->drawableType) &&
	     (c1->renderType == c2->renderType) &&
#if 0	     
	     (c1->maxPbufferWidth == c2->maxPbufferWidth) &&
	     (c1->maxPbufferHeight == c2->maxPbufferHeight) &&
	     (c1->maxPbufferPixels == c2->maxPbufferPixels) &&
	     (c1->optimalPbufferWidth == c2->optimalPbufferWidth) &&
	     (c1->optimalPbufferHeight == c2->optimalPbufferHeight) &&
#endif
	     (c1->visualSelectGroup == c2->visualSelectGroup) &&
	     (c1->rgbMode == c2->rgbMode) &&
	     (c1->colorIndexMode == c2->colorIndexMode) &&
	     (c1->doubleBufferMode == c2->doubleBufferMode) &&
	     (c1->stereoMode == c2->stereoMode) &&
	     (c1->haveAccumBuffer == c2->haveAccumBuffer) &&
	     (c1->haveDepthBuffer == c2->haveDepthBuffer) &&
	     (c1->haveStencilBuffer == c2->haveStencilBuffer) &&
	     (c1->accumRedBits == c2->accumRedBits) &&
	     (c1->accumGreenBits == c2->accumGreenBits) &&
	     (c1->accumBlueBits == c2->accumBlueBits) &&
	     (c1->accumAlphaBits == c2->accumAlphaBits) &&
	     (c1->depthBits == c2->depthBits) &&
	     (c1->stencilBits == c2->stencilBits) &&
	     (c1->indexBits == c2->indexBits) &&
	     (c1->redBits == c2->redBits) &&
	     (c1->greenBits == c2->greenBits) &&
	     (c1->blueBits == c2->blueBits) &&
	     (c1->alphaBits == c2->alphaBits) &&
	     (c1->redMask == c2->redMask) &&
	     (c1->greenMask == c2->greenMask) &&
	     (c1->blueMask == c2->blueMask) &&
	     (c1->alphaMask == c2->alphaMask) &&
	     (c1->multiSampleSize == c2->multiSampleSize) &&
	     (c1->nMultiSampleBuffers == c2->nMultiSampleBuffers) &&
	     (c1->maxAuxBuffers == c2->maxAuxBuffers) &&
	     (c1->level == c2->level) &&
	     (c1->extendedRange == c2->extendedRange) &&
	     (c1->minRed == c2->minRed) &&
	     (c1->maxRed == c2->maxRed) &&
	     (c1->minGreen == c2->minGreen) &&
	     (c1->maxGreen == c2->maxGreen) &&
	     (c1->minBlue == c2->minBlue) &&
	     (c1->maxBlue == c2->maxBlue) &&
	     (c1->minAlpha == c2->minAlpha) &&
	     (c1->maxAlpha == c2->maxAlpha) 
	   );

   return( match );
}

__GLXFBConfig *FindMatchingFBConfig( __GLXFBConfig *c, __GLXFBConfig *configs, int nconfigs )
{
   int i;

   for (i=0; i<nconfigs; i++) {
      if ( AreFBConfigsMatch( c, configs + i ) ) 
	 return( configs + i );
   }

   return(0);
}
