/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* The panoramix components contained the following notice */
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#include <version-config.h>
#endif

#include <X11/X.h>
#include <X11/Xos.h>   /* for unistd.h  */
#include <X11/Xproto.h>
#include <pixman.h>
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "extension.h"
#include "colormap.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "selection.h"
#include <X11/fonts/font.h>
#include "opaque.h"
#include "servermd.h"
#include "hotplug.h"
#include "site.h"
#include "dixfont.h"
#include "extnsionst.h"
#include "privates.h"
#include "registry.h"
#ifdef PANORAMIX
#include "panoramiXsrv.h"
#else
#include "dixevents.h"		/* InitEvents() */
#include "dispatch.h"		/* InitProcVectors() */
#endif

#ifdef DPMSExtension
#include <X11/extensions/dpmsconst.h>
#include "dpmsproc.h"
#endif

extern void Dispatch(void);

extern void InitProcVectors(void);

#ifdef XQUARTZ
#include <pthread.h>

BOOL serverInitComplete = FALSE;
pthread_mutex_t serverInitCompleteMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t serverInitCompleteCond = PTHREAD_COND_INITIALIZER;

int dix_main(int argc, char *argv[], char *envp[])
#else
int main(int argc, char *argv[], char *envp[])
#endif
{
    int		i;
    HWEventQueueType	alwaysCheckForInput[2];

    display = "0";

    InitRegions();

    pixman_disable_out_of_bounds_workaround();

    CheckUserParameters(argc, argv, envp);

    CheckUserAuthorization();

    InitConnectionLimits();

    ProcessCommandLine(argc, argv);

    alwaysCheckForInput[0] = 0;
    alwaysCheckForInput[1] = 1;
    while(1)
    {
	serverGeneration++;
	ScreenSaverTime = defaultScreenSaverTime;
	ScreenSaverInterval = defaultScreenSaverInterval;
	ScreenSaverBlanking = defaultScreenSaverBlanking;
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;
#ifdef DPMSExtension
	DPMSStandbyTime = DEFAULT_SCREEN_SAVER_TIME;
	DPMSSuspendTime = DEFAULT_SCREEN_SAVER_TIME;
	DPMSOffTime = DEFAULT_SCREEN_SAVER_TIME;
	DPMSEnabled = TRUE;
	DPMSPowerLevel = 0;
#endif
	InitBlockAndWakeupHandlers();
	/* Perform any operating system dependent initializations you'd like */
	OsInit();
        config_init();
	if(serverGeneration == 1)
	{
	    CreateWellKnownSockets();
	    InitProcVectors();
	    for (i=1; i<MAXCLIENTS; i++)
		clients[i] = NullClient;
	    serverClient = xalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
	    InitClient(serverClient, 0, (pointer)NULL);
	}
	else
	    ResetWellKnownSockets ();
	clients[0] = serverClient;
	currentMaxClients = 1;

	if (!InitClientResources(serverClient))      /* for root resources */
	    FatalError("couldn't init server resources");

	SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
	screenInfo.arraySize = MAXSCREENS;
	screenInfo.numScreens = 0;

	InitAtoms();
	InitEvents();
	InitSelections();
	InitGlyphCaching();
	if (!dixResetPrivates())
	    FatalError("couldn't init private data storage");
	dixResetRegistry();
	ResetFontPrivateIndex();
	InitCallbackManager();
	InitOutput(&screenInfo, argc, argv);

	if (screenInfo.numScreens < 1)
	    FatalError("no screens found");
	InitExtensions(argc, argv);
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ScreenPtr pScreen = screenInfo.screens[i];
	    if (!CreateScratchPixmapsForScreen(i))
		FatalError("failed to create scratch pixmaps");
	    if (pScreen->CreateScreenResources &&
		!(*pScreen->CreateScreenResources)(pScreen))
		FatalError("failed to create screen resources");
	    if (!CreateGCperDepth(i))
		FatalError("failed to create scratch GCs");
	    if (!CreateDefaultStipple(i))
		FatalError("failed to create default stipple");
	    if (!CreateRootWindow(pScreen))
		FatalError("failed to create root window");
	}

	InitFonts();
	if (SetDefaultFontPath(defaultFontPath) != Success) {
	    ErrorF("[dix] failed to set default font path '%s'", defaultFontPath);
	}
	if (!SetDefaultFont(defaultTextFont)) {
	    FatalError("could not open default font '%s'", defaultTextFont);
	}

	if (!(rootCursor = CreateRootCursor(NULL, 0))) {
	    FatalError("could not open default cursor font '%s'",
		       defaultCursorFont);
	}

#ifdef DPMSExtension
 	/* check all screens, looking for DPMS Capabilities */
 	DPMSCapableFlag = DPMSSupported();
	if (!DPMSCapableFlag)
     	    DPMSEnabled = FALSE;
#endif

#ifdef PANORAMIX
	/*
	 * Consolidate window and colourmap information for each screen
	 */
	if (!noPanoramiXExtension)
	    PanoramiXConsolidate();
#endif

	for (i = 0; i < screenInfo.numScreens; i++)
	    InitRootWindow(WindowTable[i]);

        InitCoreDevices();
	InitInput(argc, argv);
	InitAndStartDevices();

	dixSaveScreens(serverClient, SCREEN_SAVER_FORCER, ScreenSaverReset);

#ifdef PANORAMIX
	if (!noPanoramiXExtension) {
	    if (!PanoramiXCreateConnectionBlock()) {
		FatalError("could not create connection block info");
	    }
	} else
#endif
	{
	    if (!CreateConnectionBlock()) {
	    	FatalError("could not create connection block info");
	    }
	}

#ifdef XQUARTZ
    /* Let the other threads know the server is done with its init */
    pthread_mutex_lock(&serverInitCompleteMutex);
    serverInitComplete = TRUE;
    pthread_cond_broadcast(&serverInitCompleteCond);
    pthread_mutex_unlock(&serverInitCompleteMutex);
#endif
        
	NotifyParentProcess();

	Dispatch();

        UndisplayDevices();

	/* Now free up whatever must be freed */
	if (screenIsSaved == SCREEN_SAVER_ON)
	    dixSaveScreens(serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);
	FreeScreenSaverTimer();
	CloseDownExtensions();

#ifdef PANORAMIX
	{
	    Bool remember_it = noPanoramiXExtension;
	    noPanoramiXExtension = TRUE;
	    FreeAllResources();
	    noPanoramiXExtension = remember_it;
	}
#else
	FreeAllResources();
#endif

        config_fini();

        memset(WindowTable, 0, sizeof(WindowTable));
	CloseDownDevices();
	CloseDownEvents();

	for (i = screenInfo.numScreens - 1; i >= 0; i--)
	{
	    FreeScratchPixmapsForScreen(i);
	    FreeGCperDepth(i);
	    FreeDefaultStipple(i);
	    (* screenInfo.screens[i]->CloseScreen)(i, screenInfo.screens[i]);
	    dixFreePrivates(screenInfo.screens[i]->devPrivates);
	    xfree(screenInfo.screens[i]);
	    screenInfo.numScreens = i;
	}
	FreeFonts();

	FreeAuditTimer();

	dixFreePrivates(serverClient->devPrivates);
	serverClient->devPrivates = NULL;

	if (dispatchException & DE_TERMINATE)
	{
	    CloseWellKnownConnections();
	}

	OsCleanup((dispatchException & DE_TERMINATE) != 0);

	if (dispatchException & DE_TERMINATE)
	{
	    ddxGiveUp();
	    break;
	}

	xfree(ConnectionInfo);
	ConnectionInfo = NULL;
    }
    return(0);
}

