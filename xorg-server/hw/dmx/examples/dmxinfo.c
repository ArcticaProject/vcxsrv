/*
 * Copyright 2001,2002 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Rickard E. (Rik) Faith <faith@redhat.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/dmxext.h>

static void indent(int level)
{
    int i;
    for (i = 0; i < level; i++) printf("    ");
}

static void print_window_id(const char *displayName, Display *display,
                            Window window, int level, int child)
{
    char                 *name;
    
    if (!XFetchName(display, window, &name)) name = NULL;
    indent(level);
    if (child) printf("(%d) ", child);
    printf("%s window 0x%08lx: %s%s\n",
           displayName,
           (long unsigned)window,
           name ? name : "",
           (window == DefaultRootWindow(display))
           ? " (DMX root window)" : "");
    if (name) XFree(name);
}

static void print_info(Display *display, Window window, int level, int child)
{
    DMXWindowAttributes winfo[128];
    int                 count;
    int                 i;
    
    if (!DMXGetWindowAttributes(display, window, &count, 128, winfo)) {
        printf("Could not get window information for 0x%08lx\n",
               (long unsigned)window);
        exit(-2);
    }
    printf("\n");
    print_window_id("DMX", display, window, level, child);
    for (i = 0; i < count; i++) {
        DMXScreenAttributes  sinfo;
        Display              *backend;

        /* This could also be cached -- the information doesn't change. */
        if (!DMXGetScreenAttributes(display, winfo[i].screen, &sinfo)) {
            printf("Could not get screen information for screen %d\n", i);
            exit(-2);
        }
        if (!(backend = XOpenDisplay(sinfo.displayName))) {
            printf("Cannot open backend display %s\n", sinfo.displayName);
            exit(-2);
        }
        XCloseDisplay(backend);
        
        indent(level+1);
        printf("%s window 0x%08lx: %dx%d%+d%+d",
               sinfo.displayName,
               (long unsigned)winfo[i].window,
               winfo[i].pos.width, winfo[i].pos.height,
               winfo[i].pos.x, winfo[i].pos.y);
        if (!winfo[i].vis.width
            && !winfo[i].vis.height
            && !winfo[i].vis.x
            && !winfo[i].vis.y) printf(" not visible\n");
        else if (winfo[i].vis.width == winfo[i].pos.width
                 && winfo[i].vis.height == winfo[i].pos.height) {
            printf( " %+d%+d\n", winfo[i].vis.x, winfo[i].vis.y);
        } else {
            printf( " %dx%d%+d%+d\n",
                    winfo[i].vis.width, winfo[i].vis.height,
                    winfo[i].vis.x, winfo[i].vis.y);
        }
    }
}

static void print_tree(Display *display, Window window, int level, int child)
{
    Window       root, parent;
    Window       *list;
    unsigned int count;
    unsigned int i;

    print_info(display, window, level, child);
    
    if (!XQueryTree(display, window, &root, &parent, &list, &count)) {
        printf("Cannot query window tree for 0x%08lx\n",
               (long unsigned)window);
        exit(-3);
    }

    if (count) {
        indent(level+1);
        printf("%d child%s:\n", count, count > 1 ? "ren" : "");
        for (i = 0; i < count; i++) {
            print_tree(display, list[i], level+1, i+1);
        }
    }
}

static const char *core(DMXInputAttributes *iinfo)
{
    if (iinfo->isCore)         return "core";
    else if (iinfo->sendsCore) return "extension (sends core)";
    else                       return "extension";
}

int main(int argc, char **argv)
{
    Display              *display = NULL;
    Window               window   = 0;
    int                  event_base;
    int                  error_base;
    int                  major_version, minor_version, patch_version;
    DMXScreenAttributes  sinfo;
    DMXInputAttributes   iinfo;
    int                  count;
    int                  i;

    if (argc == 2 || argc == 3) {
        if (!(display = XOpenDisplay(argv[1]))) {
            printf("Cannot open display %s\n", argv[1]);
            return -1;
        }
        if (argc == 3) window = strtol(argv[2], NULL, 0);
    } else {
        printf("Usage: %s display [windowid]\n", argv[0]);
        return -1;
    }

    if (!display && !(display = XOpenDisplay(NULL))) {
        printf("Cannot open default display\n");
        return -1;
    }

    if (!DMXQueryExtension(display, &event_base, &error_base)) {
        printf("DMX extension not present\n");
        return -1;
    }
    printf("DMX extension present: event_base = %d, error_base = %d\n",
           event_base, error_base);

    if (!DMXQueryVersion(display,
                         &major_version, &minor_version, &patch_version)) {
        printf("Could not get extension version\n");
        return -1;
    }
    printf("Extension version: %d.%d patch %d\n",
           major_version, minor_version, patch_version);

    if (!DMXGetScreenCount(display, &count)) {
        printf("Could not get screen count\n");
        return -1;
    }
    printf("Screen count = %d\n", count);

    for (i = 0; i < count; i++) {
        if (!DMXGetScreenAttributes(display, i, &sinfo)) {
            printf("Could not get screen information for %d\n", i);
            return -1;
        }
        printf("%d: %s %ux%u+%d+%d %d @%dx%d (root: %dx%d%+d%+d)\n",
               i, sinfo.displayName,
               sinfo.screenWindowWidth, sinfo.screenWindowHeight,
               sinfo.screenWindowXoffset, sinfo.screenWindowYoffset,
               sinfo.logicalScreen,
               sinfo.rootWindowXorigin, sinfo.rootWindowYorigin,
               sinfo.rootWindowWidth, sinfo.rootWindowHeight,
               sinfo.rootWindowXoffset, sinfo.rootWindowYoffset);
    }

    if (major_version == 1 && minor_version >= 1) {
        if (!DMXGetInputCount(display, &count)) {
            printf("Could not get input count\n");
            return -1;
        }
        printf("Input count = %d\n", count);
        for (i = 0; i < count; i++) {
            if (!DMXGetInputAttributes(display, i, &iinfo)) {
                printf("Could not get input information for id %d\n", i);
                return -1;
            }
            switch (iinfo.inputType) {
            case DMXLocalInputType:
                printf("  %2d local   %-20.20s %s\n", i, "", core(&iinfo));
                break;
            case DMXConsoleInputType:
                printf("  %2d console %-20.20s %s\n",
                       i, iinfo.name, core(&iinfo));
                break;
            case DMXBackendInputType:
                printf("  %2d backend %-20.20s id=%2d screen=%2d %s\n",
                       i, iinfo.name, iinfo.physicalId, iinfo.physicalScreen,
                       core(&iinfo));
                break;
            }
        }
    }

    if (window) print_info(display, window, 0, 0);
    else        print_tree(display, DefaultRootWindow(display), 0, 0);
    
    XCloseDisplay(display);
    return 0;
}
