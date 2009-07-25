/**************************************************************
 * quartzPasteboard.c
 *
 * Aqua pasteboard <-> X cut buffer
 * Greg Parker     gparker@cs.stanford.edu     March 8, 2001
 **************************************************************/
/*
 * Copyright (c) 2001 Greg Parker. All Rights Reserved.
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

#include "misc.h"
#include "inputstr.h"
#include "quartzPasteboard.h"

#include <X11/Xatom.h>
#include "windowstr.h"
#include "propertyst.h"
#include "scrnintstr.h"
#include "selection.h"
#include "globals.h"


// Helper function to read the X11 cut buffer
// FIXME: What about multiple screens? Currently, this reads the first
// CUT_BUFFER0 from the first screen where the buffer content is a string.
// Returns a string on the heap that the caller must free.
// Returns NULL if there is no cut text or there is not enough memory.
static char * QuartzReadCutBuffer(void)
{
    int rc, i;
    char *text = NULL;

    for (i = 0; i < screenInfo.numScreens; i++) {
        ScreenPtr pScreen = screenInfo.screens[i];
        PropertyPtr pProp;

	rc = dixLookupProperty(&pProp, WindowTable[pScreen->myNum],
			       XA_CUT_BUFFER0, serverClient, DixReadAccess);
        if (rc != Success) continue;
        if (pProp->type != XA_STRING) continue;
        if (pProp->format != 8) continue;

        text = xalloc(1 + pProp->size);
        if (! text) continue;
        memcpy(text, pProp->data, pProp->size);
        text[pProp->size] = '\0';
        return text;
    }

    // didn't find any text
    return NULL;
}

// Write X cut buffer to Mac OS X pasteboard
// Called by mieqProcessInputEvents() in response to request from X server thread.
void QuartzWritePasteboard(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents)
{
    char *text;
    text = QuartzReadCutBuffer();
    if (text) {
        QuartzWriteCocoaPasteboard(text);
        free(text);
    }
}

#define strequal(a, b) (0 == strcmp((a), (b)))

// Read Mac OS X pasteboard into X cut buffer
// Called by mieqProcessInputEvents() in response to request from X server thread.
void QuartzReadPasteboard(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents)
{
    char *oldText = QuartzReadCutBuffer();
    char *text = QuartzReadCocoaPasteboard();

    // Compare text with current cut buffer contents.
    // Change the buffer if both exist and are different
    //   OR if there is new text but no old text.
    // Otherwise, don't clear the selection unnecessarily.

    if ((text && oldText && !strequal(text, oldText)) ||
        (text && !oldText)) {
        int scrn, rc;
	Selection *pSel;

        for (scrn = 0; scrn < screenInfo.numScreens; scrn++) {
	    ScreenPtr pScreen = screenInfo.screens[scrn];
	    // Set the cut buffers on each screen
	    // fixme really on each screen?
	    dixChangeWindowProperty(serverClient, WindowTable[pScreen->myNum],
				    XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace,
				    strlen(text), (pointer)text, TRUE);
        }

        // Undo any current X selection (similar to code in dispatch.c)
        // FIXME: what about secondary selection?
        // FIXME: only touch first XA_PRIMARY selection?
	rc = dixLookupSelection(&pSel, XA_PRIMARY, serverClient,
				DixSetAttrAccess);
        if (rc == Success) {
	    // Notify client if necessary
	    if (pSel->client) {
	        xEvent event;

	        event.u.u.type = SelectionClear;
		event.u.selectionClear.time = GetTimeInMillis();
		event.u.selectionClear.window = pSel->window;
		event.u.selectionClear.atom = pSel->selection;
		TryClientEvents(pSel->client, dev, &event, 1, NoEventMask,
				NoEventMask /*CantBeFiltered*/, NullGrab);
	    }

	    // Erase it
	    pSel->pWin = NullWindow;
	    pSel->window = None;
	    pSel->client = NullClient;
        }
    }

    if (text) free(text);
    if (oldText) free(oldText);
}
