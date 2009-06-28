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

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#include "dmx.h"
#include "dmxlog.h"

#undef Xmalloc
#undef Xcalloc
#undef Xrealloc
#undef Xfree

#include "glxserver.h"

#include <windowstr.h>

#include "glxfbconfig.h"

#ifdef PANORAMIX
#include "panoramiXsrv.h"
#endif

__GLXscreenInfo *__glXActiveScreens;
GLint __glXNumActiveScreens;

__GLXFBConfig **__glXFBConfigs;
int            __glXNumFBConfigs;

static char GLXServerVendorName[] = "SGI DMX/glxProxy";
static char GLXServerVersion[64];
static char GLXServerExtensions[] =
            "GLX_EXT_visual_info "
            "GLX_EXT_visual_rating "
            "GLX_EXT_import_context "
	    "GLX_SGIX_fbconfig "
	    "GLX_SGI_make_current_read "
	    "GLX_SGI_swap_control "
            ;

static char ExtensionsString[1024];

static void CalcServerVersionAndExtensions( void )
{
   int s;
   xGLXQueryVersionReq *req;
   xGLXQueryVersionReply reply;
   char **be_extensions;
   char *ext;
   char *denied_extensions;

   /*
    * set the server glx version to be the minimum version
    * supported by all back-end servers
    */
   __glXVersionMajor = 0;
   __glXVersionMinor = 0;
   for (s=0; s<__glXNumActiveScreens; s++) {
      DMXScreenInfo *dmxScreen = &dmxScreens[s];
      Display *dpy = dmxScreen->beDisplay;

      /* Send the glXQueryVersion request */
      LockDisplay(dpy);
      GetReq(GLXQueryVersion,req);
      req->reqType = dmxScreen->glxMajorOpcode;
      req->glxCode = X_GLXQueryVersion;
      req->majorVersion = GLX_SERVER_MAJOR_VERSION;
      req->minorVersion = GLX_SERVER_MINOR_VERSION;
      _XReply(dpy, (xReply*) &reply, 0, False);
      UnlockDisplay(dpy);
      SyncHandle();

      if (s == 0) {
	 __glXVersionMajor = reply.majorVersion;
	 __glXVersionMinor = reply.minorVersion;
      }
      else {
	 if (reply.majorVersion < __glXVersionMajor) {
	    __glXVersionMajor = reply.majorVersion;
	    __glXVersionMinor = reply.minorVersion;
	 }
	 else if ( (reply.majorVersion == __glXVersionMajor) &&
	           (reply.minorVersion < __glXVersionMinor)  ) {
	    __glXVersionMinor = reply.minorVersion;
	 }
      }

   }

   if (GLX_SERVER_MAJOR_VERSION < __glXVersionMajor) {
      __glXVersionMajor = GLX_SERVER_MAJOR_VERSION;
      __glXVersionMinor = GLX_SERVER_MINOR_VERSION;
   }
   else if ( (GLX_SERVER_MAJOR_VERSION == __glXVersionMajor) &&
	     (GLX_SERVER_MINOR_VERSION < __glXVersionMinor)  ) {
      __glXVersionMinor = GLX_SERVER_MINOR_VERSION;
   }

   sprintf(GLXServerVersion, "%d.%d DMX %d back-end server(s)",
              __glXVersionMajor, __glXVersionMinor, __glXNumActiveScreens );	 
   /*
    * set the ExtensionsString to the minimum extensions string
    */
   ExtensionsString[0] = '\0';

   /*
    * read extensions strings of all back-end servers
    */
   be_extensions = (char **)Xalloc( __glXNumActiveScreens * sizeof(char *) );
   if (!be_extensions)
      return;

   for (s=0; s<__glXNumActiveScreens; s++) {
      DMXScreenInfo *dmxScreen = &dmxScreens[s];
      Display *dpy = dmxScreen->beDisplay;
      xGLXQueryServerStringReq *req;
      xGLXQueryServerStringReply reply;
      int length, numbytes, slop;

      /* Send the glXQueryServerString request */
      LockDisplay(dpy);
      GetReq(GLXQueryServerString,req);
      req->reqType = dmxScreen->glxMajorOpcode;
      req->glxCode = X_GLXQueryServerString;
      req->screen = DefaultScreen(dpy);
      req->name = GLX_EXTENSIONS;
      _XReply(dpy, (xReply*) &reply, 0, False);

      length = (int)reply.length;
      numbytes = (int)reply.n;
      slop = numbytes * __GLX_SIZE_INT8 & 3;
      be_extensions[s] = (char *)Xalloc(numbytes);
      if (!be_extensions[s]) {
	 /* Throw data on the floor */
	 _XEatData(dpy, length);
      } else {
	 _XRead(dpy, (char *)be_extensions[s], numbytes);
	 if (slop) _XEatData(dpy,4-slop);
      }
      UnlockDisplay(dpy);
      SyncHandle();
   }

   /*
    * extensions string will include only extensions that our
    * server supports as well as all back-end servers supports.
    * extensions that are in the DMX_DENY_EXTENSIONS string will
    * not be supported.
    */
   denied_extensions = getenv("DMX_DENY_GLX_EXTENSIONS");
   ext = strtok(GLXServerExtensions, " ");
   while( ext ) {
      int supported = 1;

      if (denied_extensions && strstr(denied_extensions, ext)) {
	 supported = 0;
      }
      else {
	 for (s=0; s<__glXNumActiveScreens && supported; s++) {
	    if ( !strstr(be_extensions[s], ext) ) {
	       supported = 0;
	    }
	 }
      }

      if (supported) {
	 strcat(ExtensionsString, ext);
	 strcat(ExtensionsString, " ");
      }

      ext = strtok(NULL, " ");
   }

   /*
    * release temporary storage
    */
   for (s=0; s<__glXNumActiveScreens; s++) {
      if (be_extensions[s]) Xfree(be_extensions[s]); 
   }
   Xfree( be_extensions );

   if (dmxGLXSwapGroupSupport) {
       if (!denied_extensions ||
	   !strstr(denied_extensions, "GLX_SGIX_swap_group")) {
	   strcat(ExtensionsString, "GLX_SGIX_swap_group");
	   if (!denied_extensions ||
	       !strstr(denied_extensions, "GLX_SGIX_swap_barrier")) {
	       strcat(ExtensionsString, " GLX_SGIX_swap_barrier");
	   }
       }
   }

}

void __glXScreenInit(GLint numscreens)
{
   int s;
   int c;
   DMXScreenInfo *dmxScreen0 = &dmxScreens[0];
    __glXNumActiveScreens = numscreens;


   CalcServerVersionAndExtensions();


   __glXFBConfigs = NULL;
   __glXNumFBConfigs = 0;

   if ( (__glXVersionMajor == 1 && __glXVersionMinor >= 3) ||
        (__glXVersionMajor > 1) ||
	( strstr(ExtensionsString, "GLX_SGIX_fbconfig") )      ) {

      /*
      // Initialize FBConfig info.
      // find the set of FBConfigs that are present on all back-end
      // servers - only those configs will be supported
       */
      __glXFBConfigs = (__GLXFBConfig **)Xalloc( dmxScreen0->numFBConfigs * 
	                      (numscreens+1) * sizeof(__GLXFBConfig *) );
      __glXNumFBConfigs = 0;
   
      for (c=0; c<dmxScreen0->numFBConfigs; c++) { 
	 __GLXFBConfig *cfg = NULL;

	 if (numscreens > 1) {
	    for (s=1; s<numscreens; s++) {
	       DMXScreenInfo *dmxScreen = &dmxScreens[s];
	       __GLXscreenInfo *glxScreen = &__glXActiveScreens[s];
	  
	       cfg = FindMatchingFBConfig( &dmxScreen0->fbconfigs[c],
		                           dmxScreen->fbconfigs, 
		                           dmxScreen->numFBConfigs );
	       __glXFBConfigs[ __glXNumFBConfigs * (numscreens+1) + s + 1 ] = cfg;
	       if (!cfg) {
		  dmxLog(dmxInfo,"screen0 FBConfig 0x%x is missing on screen#%d\n", dmxScreen0->fbconfigs[c].id, s);
		  break;
	       }
	       else {
		  dmxLog(dmxInfo,"screen0 FBConfig 0x%x matched to  0x%x on screen#%d\n", dmxScreen0->fbconfigs[c].id, cfg->id, s);
	       }
	    }
         }
	 else {
	    cfg = &dmxScreen0->fbconfigs[c];
	 }

	 if (cfg) {

	    /* filter out overlay visuals */
	    if (cfg->level == 0) {
	       __GLXFBConfig *proxy_cfg;

	       __glXFBConfigs[ __glXNumFBConfigs * (numscreens+1) + 1 ] = 
	               &dmxScreen0->fbconfigs[c];

	       proxy_cfg = Xalloc( sizeof(__GLXFBConfig) );
	       memcpy( proxy_cfg, cfg, sizeof(__GLXFBConfig) );
	       proxy_cfg->id =  FakeClientID(0);
	       /* visual will be associated later in __glXGetFBConfigs */
	       proxy_cfg->associatedVisualId =  (unsigned int)-1;

	       __glXFBConfigs[ __glXNumFBConfigs * (numscreens+1) + 0 ] = proxy_cfg;

	       __glXNumFBConfigs++;
	    }

	 }

      }

    }

}

void __glXScreenReset(void)
{
  __glXNumActiveScreens = 0;
}

char *__glXGetServerString( unsigned int name ) 
{
   char *ret = NULL;

   switch( name) {

      case GLX_VENDOR:
	 ret = GLXServerVendorName;
	 break;

      case GLX_VERSION:
	 ret = GLXServerVersion;
	 break;

      case GLX_EXTENSIONS:
	 ret = ExtensionsString;
	 break;

      default:
	 break;
   }

   return( ret );

}


__GLXFBConfig *glxLookupFBConfig( GLXFBConfigID id )
{
   int i,j;

   for (i=0, j=0; i<__glXNumFBConfigs; i++,j+=(__glXNumActiveScreens+1) ) {
      if ( __glXFBConfigs[j]->id == id) 
	 return( __glXFBConfigs[j] );
   }

   return(NULL);
}

__GLXFBConfig *glxLookupFBConfigByVID( VisualID vid )
{
   int i,j;

   for (i=0, j=0; i<__glXNumFBConfigs; i++,j+=(__glXNumActiveScreens+1) ) {
      if ( __glXFBConfigs[j]->associatedVisualId == vid) 
	 return( __glXFBConfigs[j] );
   }

   return(NULL);
}

__GLXFBConfig *glxLookupBackEndFBConfig( GLXFBConfigID id, int screen )
{
   int i;
   int j;

   for (i=0, j=0; i<__glXNumFBConfigs; i++,j+=(__glXNumActiveScreens+1) ) {
      if ( __glXFBConfigs[j]->id == id) 
	 return( __glXFBConfigs[j+screen+1] );
   }

   return(NULL);

}

int glxIsExtensionSupported( char *ext )
{
   return( strstr(ExtensionsString, ext) != NULL );
}
