/**************************************************************
 *
 * Startup code for the Quartz Darwin X Server
 *
 * Copyright (c) 2001-2004 Torrey T. Lyons. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include "quartzCommon.h"
#include "X11Controller.h"
#include "darwin.h"
#include "quartz.h"
#include "opaque.h"
#include "micmap.h"

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG 1
#else
#include <assert.h>
#endif

char **envpGlobal;      // argcGlobal and argvGlobal
                        // are from dix/globals.c

int main(int argc, char **argv, char **envp);
void _InitHLTB(void);
void DarwinHandleGUI(int argc, char **argv, char **envp);

static void server_thread (void *arg) {
  exit (main (argcGlobal, argvGlobal, envpGlobal));
}

/*
 * DarwinHandleGUI
 *  This function is called first from main(). The first time
 *  it is called we start the Mac OS X front end. The front end
 *  will call main() again from another thread to run the X
 *  server. On the second call this function loads the user
 *  preferences set by the Mac OS X front end.
 */
void DarwinHandleGUI(int argc, char **argv, char **envp) {
    static Bool been_here = FALSE;
    int         i;
    int         fd[2];

    if (been_here) {
        return;
    }
    been_here = TRUE;
    
    // Make a pipe to pass events
    assert( pipe(fd) == 0 );
    darwinEventReadFD = fd[0];
    darwinEventWriteFD = fd[1];
    fcntl(darwinEventReadFD, F_SETFL, O_NONBLOCK);

    // Store command line arguments to pass back to main()
    argcGlobal = argc;
    argvGlobal = argv;
    envpGlobal = envp;

    for (i = 1; i < argc; i++) {
        // Display version info without starting Mac OS X UI if requested
        if (!strcmp( argv[i], "-showconfig" ) || !strcmp( argv[i], "-version" )) {
            DarwinPrintBanner();
            exit(0);
        }
    }

    /* Initially I ran the X server on the main thread, and received
       events on the second thread. But now we may be using Carbon,
       that needs to run on the main thread. (Otherwise, when it's
       prebound, it will initialize itself on the wrong thread)
       
       grr.. but doing that means that if the X thread gets scheduled
       before the main thread when we're _not_ prebound, things fail,
       so initialize by hand. */

    _InitHLTB();    
    X11ControllerMain(argc, (const char **)argv, server_thread, NULL);
    exit(0);
}
