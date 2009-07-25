/* main.m
   Copyright (c) 2002, 2008 Apple Computer, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
   HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name(s) of the above
   copyright holders shall not be used in advertising or otherwise to
   promote the sale, use or other dealings in this Software without
   prior written authorization.
 */

#include "pbproxy.h"
#import "x-selection.h"

#include <pthread.h>
#include <unistd.h>
#include <X11/extensions/applewm.h>

Display *xpbproxy_dpy;
int xpbproxy_apple_wm_event_base, xpbproxy_apple_wm_error_base;
int xpbproxy_xfixes_event_base, xpbproxy_xfixes_error_base;
BOOL xpbproxy_have_xfixes;

extern char *display;

#ifdef STANDALONE_XPBPROXY
BOOL xpbproxy_is_standalone = NO;
#endif

x_selection *_selection_object;

static int x_io_error_handler (Display *dpy) {
    /* We lost our connection to the server. */
    
    TRACE ();

    /* trigger the thread to restart?
     *   NO - this would be to a "deeper" problem, and restarts would just
     *        make things worse...
     */
#ifdef STANDALONE_XPBPROXY
    if(xpbproxy_is_standalone)
        exit(EXIT_FAILURE);
#endif

    return 0;
}

static int x_error_handler (Display *dpy, XErrorEvent *errevent) {
    return 0;
}

static inline pthread_t create_thread(void *func, void *arg) {
    pthread_attr_t attr;
    pthread_t tid;
    
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, func, arg);
    pthread_attr_destroy(&attr);
    
    return tid;
}

static void *xpbproxy_x_thread(void *args) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    size_t i;

    for(i=0, xpbproxy_dpy=NULL; !xpbproxy_dpy && i<5; i++) {
        xpbproxy_dpy = XOpenDisplay(NULL);
        
        if(!xpbproxy_dpy && display) {
            char _display[32];
            snprintf(_display, sizeof(_display), ":%s", display);
            setenv("DISPLAY", _display, TRUE);

            xpbproxy_dpy=XOpenDisplay(_display);
        }
        if(!xpbproxy_dpy)
            sleep(1);
    }
    
    if (xpbproxy_dpy == NULL) {
        fprintf (stderr, "xpbproxy: can't open default display\n");
        [pool release];
        return NULL;
    }
    
    XSetIOErrorHandler (x_io_error_handler);
    XSetErrorHandler (x_error_handler);
    
    if (!XAppleWMQueryExtension (xpbproxy_dpy, &xpbproxy_apple_wm_event_base,
                                 &xpbproxy_apple_wm_error_base)) {
        fprintf (stderr, "xpbproxy: can't open AppleWM server extension\n");
        [pool release];
        return NULL;
    }
    
    xpbproxy_have_xfixes = XFixesQueryExtension(xpbproxy_dpy, &xpbproxy_xfixes_event_base, &xpbproxy_xfixes_error_base);
    
    XAppleWMSelectInput (xpbproxy_dpy, AppleWMActivationNotifyMask |
                         AppleWMPasteboardNotifyMask);
    
    _selection_object = [[x_selection alloc] init];
    
    if(!xpbproxy_input_register()) {
        [pool release];
        return NULL;
    }

    [pool release];
 
    xpbproxy_input_loop();
    return NULL;
}

BOOL xpbproxy_init (void) {
    create_thread(xpbproxy_x_thread, NULL);
    return TRUE;
}

id xpbproxy_selection_object (void) {
    return _selection_object;
}

Time xpbproxy_current_timestamp (void) {
    /* FIXME: may want to fetch a timestamp from the server.. */
    return CurrentTime;
}

void debug_printf (const char *fmt, ...) {
    static int spew = -1;
    
    if (spew == -1) {
        char *x = getenv ("DEBUG");
        spew = (x != NULL && atoi (x) != 0);
    }
    
    if (spew) {
        va_list args;
        va_start(args, fmt);
        vfprintf (stderr, fmt, args);
        va_end(args);
    }
}
