/* main.c -- X application launcher
 
 Copyright (c) 2007 Jeremy Huddleston
 Copyright (c) 2007 Apple Inc
 
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
 prior written authorization. */

#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <CoreFoundation/CoreFoundation.h>

#define DEFAULT_CLIENT "/usr/X11/bin/xterm"
#define DEFAULT_STARTX "/usr/X11/bin/startx"
#define DEFAULT_SHELL  "/bin/sh"

static int execute(const char *command);
static char *command_from_prefs(const char *key, const char *default_value);

int main(int argc, char **argv) {
    Display *display;
    const char *s;

    size_t i;
    fprintf(stderr, "X11.app: main(): argc=%d\n", argc);
    for(i=0; i < argc; i++) {
        fprintf(stderr, "\targv[%u] = %s\n", (unsigned)i, argv[i]);
    }

    /* If we have a process serial number and it's our only arg, act as if
     * the user double clicked the app bundle: launch app_to_run if possible
     */
    if(argc == 1 || (argc == 2 && !strncmp(argv[1], "-psn_", 5))) {
        /* Now, try to open a display, if so, run the launcher */
        display = XOpenDisplay(NULL);
        if(display) {
            fprintf(stderr, "X11.app: Closing the display and sleeping for 2s to allow the X server to start up.\n");
            /* Could open the display, start the launcher */
            XCloseDisplay(display);

            /* Give 2 seconds for the server to start... 
             * TODO: *Really* fix this race condition
             */
            usleep(2000);
            return execute(command_from_prefs("app_to_run", DEFAULT_CLIENT));
        }
    }

    /* Start the server */
    if(s = getenv("DISPLAY")) {
        fprintf(stderr, "X11.app: Could not connect to server (DISPLAY=\"%s\", unsetting).  Starting X server.\n", s);
        unsetenv("DISPLAY");
    } else {
        fprintf(stderr, "X11.app: Could not connect to server (DISPLAY is not set).  Starting X server.\n");
    }
    return execute(command_from_prefs("startx_script", DEFAULT_STARTX));
}

static int execute(const char *command) {
    const char *newargv[7];
    const char **s;

    newargv[0] = "/usr/bin/login";
    newargv[1] = "-fp";
    newargv[2] = getlogin();
    newargv[3] = command_from_prefs("login_shell", DEFAULT_SHELL);
    newargv[4] = "-c";
    newargv[5] = command;
    newargv[6] = NULL;
    
    fprintf(stderr, "X11.app: Launching %s:\n", command);
    for(s=newargv; *s; s++) {
        fprintf(stderr, "\targv[%d] = %s\n", s - newargv, *s);
    }

    execvp (newargv[0], (char * const *) newargv);
    perror ("X11.app: Couldn't exec.");
    return(1);
}

static char *command_from_prefs(const char *key, const char *default_value) {
    char *command = NULL;
    
    CFStringRef cfKey = CFStringCreateWithCString(NULL, key, kCFStringEncodingASCII);
    CFPropertyListRef PlistRef = CFPreferencesCopyAppValue(cfKey, kCFPreferencesCurrentApplication);
    
    if ((PlistRef == NULL) || (CFGetTypeID(PlistRef) != CFStringGetTypeID())) {
        CFStringRef cfDefaultValue = CFStringCreateWithCString(NULL, default_value, kCFStringEncodingASCII);

        CFPreferencesSetAppValue(cfKey, cfDefaultValue, kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        
        int len = strlen(default_value) + 1;
        command = (char *)malloc(len * sizeof(char));
        if(!command)
            return NULL;
        strcpy(command, default_value);
    } else {
        int len = CFStringGetLength((CFStringRef)PlistRef) + 1;
        command = (char *)malloc(len * sizeof(char));
        if(!command)
            return NULL;
        CFStringGetCString((CFStringRef)PlistRef, command, len,  kCFStringEncodingASCII);
	}
    
    if (PlistRef)
        CFRelease(PlistRef);
    
    return command;
}
