/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stddef.h>
#include "windowstr.h"
#include "resource.h"
#include "privates.h"
#include "gcstruct.h"
#include "cursorstr.h"
#include "colormapst.h"
#include "inputstr.h"

struct _Private {
    int state;
    pointer value;
};

typedef struct _PrivateDesc {
    DevPrivateKey key;
    unsigned size;
    CallbackListPtr initfuncs;
    CallbackListPtr deletefuncs;
} PrivateDescRec;

#define PRIV_MAX 256
#define PRIV_STEP 16

/* list of all allocated privates */
static PrivateDescRec items[PRIV_MAX];
static int nextPriv;

static PrivateDescRec *
findItem(const DevPrivateKey key)
{
    if (!*key) {
	if (nextPriv >= PRIV_MAX)
	    return NULL;

	items[nextPriv].key = key;
	*key = nextPriv;
	nextPriv++;
    }

    return items + *key;
}

static _X_INLINE int
privateExists(PrivateRec **privates, const DevPrivateKey key)
{
    return *key && *privates &&
	(*privates)[0].state > *key &&
	(*privates)[*key].state;
}

/*
 * Request pre-allocated space.
 */
_X_EXPORT int
dixRequestPrivate(const DevPrivateKey key, unsigned size)
{
    PrivateDescRec *item = findItem(key);
    if (!item)
	return FALSE;
    if (size > item->size)
	item->size = size;
    return TRUE;
}

/*
 * Allocate a private and attach it to an existing object.
 */
_X_EXPORT pointer *
dixAllocatePrivate(PrivateRec **privates, const DevPrivateKey key)
{
    PrivateDescRec *item = findItem(key);
    PrivateCallbackRec calldata;
    PrivateRec *ptr;
    pointer value;
    int oldsize, newsize;

    newsize = (*key / PRIV_STEP + 1) * PRIV_STEP;

    /* resize or init privates array */
    if (!item)
	return NULL;

    /* initialize privates array if necessary */
    if (!*privates) {
	ptr = xcalloc(newsize, sizeof(*ptr));
	if (!ptr)
	    return NULL;
	*privates = ptr;
	(*privates)[0].state = newsize;
    }

    oldsize = (*privates)[0].state;

    /* resize privates array if necessary */
    if (*key >= oldsize) {
	ptr = xrealloc(*privates, newsize * sizeof(*ptr));
	if (!ptr)
	    return NULL;
	memset(ptr + oldsize, 0, (newsize - oldsize) * sizeof(*ptr));
	*privates = ptr;
	(*privates)[0].state = newsize;
    }

    /* initialize slot */
    ptr = *privates + *key;
    ptr->state = 1;
    if (item->size) {
	value = xcalloc(item->size, 1);
	if (!value)
	    return NULL;
	ptr->value = value;
    }

    calldata.key = key;
    calldata.value = &ptr->value;
    CallCallbacks(&item->initfuncs, &calldata);

    return &ptr->value;
}

/*
 * Look up a private pointer.
 */
_X_EXPORT pointer
dixLookupPrivate(PrivateRec **privates, const DevPrivateKey key)
{
    pointer *ptr;

    if (privateExists(privates, key))
	return (*privates)[*key].value;

    ptr = dixAllocatePrivate(privates, key);
    return ptr ? *ptr : NULL;
}

/*
 * Look up the address of a private pointer.
 */
_X_EXPORT pointer *
dixLookupPrivateAddr(PrivateRec **privates, const DevPrivateKey key)
{
    if (privateExists(privates, key))
	return &(*privates)[*key].value;

    return dixAllocatePrivate(privates, key);
}

/*
 * Set a private pointer.
 */
_X_EXPORT int
dixSetPrivate(PrivateRec **privates, const DevPrivateKey key, pointer val)
{
 top:
    if (privateExists(privates, key)) {
	(*privates)[*key].value = val;
	return TRUE;
    }

    if (!dixAllocatePrivate(privates, key))
	return FALSE;
    goto top;
}

/*
 * Called to free privates at object deletion time.
 */
_X_EXPORT void
dixFreePrivates(PrivateRec *privates)
{
    int i;
    PrivateCallbackRec calldata;

    if (privates)
	for (i = 1; i < privates->state; i++)
	    if (privates[i].state) {
		/* call the delete callbacks */
		calldata.key = items[i].key;
		calldata.value = &privates[i].value;
		CallCallbacks(&items[i].deletefuncs, &calldata);

		/* free pre-allocated memory */
		if (items[i].size)
		    xfree(privates[i].value);
	    }

    xfree(privates);
}

/*
 * Callback registration
 */
_X_EXPORT int
dixRegisterPrivateInitFunc(const DevPrivateKey key,
			   CallbackProcPtr callback, pointer data)
{
    PrivateDescRec *item = findItem(key);
    if (!item)
	return FALSE;

    return AddCallback(&item->initfuncs, callback, data);
}

_X_EXPORT int
dixRegisterPrivateDeleteFunc(const DevPrivateKey key,
			     CallbackProcPtr callback, pointer data)
{
    PrivateDescRec *item = findItem(key);
    if (!item)
	return FALSE;

    return AddCallback(&item->deletefuncs, callback, data);
}

/* Table of devPrivates offsets */
static const int offsetDefaults[] = {
    -1,					/* RT_NONE */
    offsetof(WindowRec, devPrivates),	/* RT_WINDOW */
    offsetof(PixmapRec, devPrivates),	/* RT_PIXMAP */
    offsetof(GC, devPrivates),		/* RT_GC */
    -1,		    			/* RT_FONT */
    offsetof(CursorRec, devPrivates),	/* RT_CURSOR */
    offsetof(ColormapRec, devPrivates),	/* RT_COLORMAP */
    -1,			  		/* RT_CMAPENTRY */
    -1,					/* RT_OTHERCLIENT */
    -1					/* RT_PASSIVEGRAB */
};
    
static int *offsets = NULL;
static int offsetsSize = 0;

/*
 * Specify where the devPrivates field is located in a structure type
 */
_X_EXPORT int
dixRegisterPrivateOffset(RESTYPE type, int offset)
{
    type = type & TypeMask;

    /* resize offsets table if necessary */
    while (type >= offsetsSize) {
	unsigned i = offsetsSize * 2 * sizeof(int);
	offsets = (int *)xrealloc(offsets, i);
	if (!offsets) {
	    offsetsSize = 0;
	    return FALSE;
	}
	for (i=offsetsSize; i < 2*offsetsSize; i++)
	    offsets[i] = -1;
	offsetsSize *= 2;
    }

    offsets[type] = offset;
    return TRUE;
}

_X_EXPORT int
dixLookupPrivateOffset(RESTYPE type)
{
    type = type & TypeMask;
    assert(type < offsetsSize);
    return offsets[type];
}

int
dixResetPrivates(void)
{
    int i;

    /* reset private descriptors */
    for (i = 1; i < nextPriv; i++) {
	*items[i].key = 0;
	DeleteCallbackList(&items[i].initfuncs);
	DeleteCallbackList(&items[i].deletefuncs);
    }
    nextPriv = 1;

    /* reset offsets */
    if (offsets)
	xfree(offsets);
    offsetsSize = sizeof(offsetDefaults);
    offsets = (int *)xalloc(offsetsSize);
    offsetsSize /= sizeof(int);
    if (!offsets)
	return FALSE;
    memcpy(offsets, offsetDefaults, sizeof(offsetDefaults));
    return TRUE;
}
