/* app-main.m
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
#include <unistd.h> /*for getpid*/
#include <Cocoa/Cocoa.h>

static const char *app_prefs_domain = "org.x.X11";
CFStringRef app_prefs_domain_cfstr;

/* Stubs */
char *display = NULL;
BOOL serverInitComplete = YES;
pthread_mutex_t serverInitCompleteMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t serverInitCompleteCond = PTHREAD_COND_INITIALIZER;

static void signal_handler (int sig) {
    switch(sig) {
        case SIGHUP:
            xpbproxy_prefs_reload = YES;
            break;
        default:
            _exit(EXIT_SUCCESS);
    }
}

int main (int argc, const char *argv[]) {
    const char *s;
    int i;

#ifdef DEBUG
    printf("pid: %u\n", getpid());
#endif

    xpbproxy_is_standalone = YES;

    if((s = getenv("X11_PREFS_DOMAIN")))
        app_prefs_domain = s;

    for (i = 1; i < argc; i++) {
        if(strcmp (argv[i], "--prefs-domain") == 0 && i+1 < argc) {
            app_prefs_domain = argv[++i];
        } else if (strcmp (argv[i], "--help") == 0) {
            printf("usage: xpbproxy OPTIONS\n"
                   "Pasteboard proxying for X11.\n\n"
                   "--prefs-domain <domain>   Change the domain used for reading preferences\n"
                   "                          (default: org.x.X11)\n");
            return 0;
        } else {
            fprintf(stderr, "usage: xpbproxy OPTIONS...\n"
                    "Try 'xpbproxy --help' for more information.\n");
            return 1;
        }
    }
    
    app_prefs_domain_cfstr = CFStringCreateWithCString(NULL, app_prefs_domain, kCFStringEncodingUTF8);
    
    if(!xpbproxy_init())
        return EXIT_FAILURE;
    
    signal (SIGINT, signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGHUP, signal_handler);
    signal (SIGPIPE, SIG_IGN);

    [NSApplication sharedApplication];
    [NSApp run];
    
    return EXIT_SUCCESS;
}
