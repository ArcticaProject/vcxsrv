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
    DevPrivateKey      key;
    pointer            value;
    struct _Private    *next;
};

typedef struct _PrivateDesc {
    DevPrivateKey key;
    unsigned size;
    CallbackListPtr initfuncs;
    CallbackListPtr deletefuncs;
    struct _PrivateDesc *next;
} PrivateDescRec;

/* list of all allocated privates */
static PrivateDescRec *items = NULL;

static _X_INLINE PrivateDescRec *
findItem(const DevPrivateKey key)
{
    PrivateDescRec *item = items;
    while (item) {
	if (item->key == key)
	    return item;
	item = item->next;
    }
    return NULL;
}

/*
 * Request pre-allocated space.
 */
_X_EXPORT int
dixRequestPrivate(const DevPrivateKey key, unsigned size)
{
    PrivateDescRec *item = findItem(key);
    if (item) {
	if (size > item->size)
	    item->size = size;
    } else {
	item = (PrivateDescRec *)xalloc(sizeof(PrivateDescRec));
	if (!item)
	    return FALSE;
	memset(item, 0, sizeof(PrivateDescRec));

	/* add privates descriptor */
	item->key = key;
	item->size = size;
	item->next = items;
	items = item;
    }
    return TRUE;
}

/*
 * Allocate a private and attach it to an existing object.
 */
_X_EXPORT pointer *
dixAllocatePrivate(PrivateRec **privates, const DevPrivateKey key)
{
    PrivateDescRec *item = findItem(key);
    PrivateRec *ptr;
    unsigned size = sizeof(PrivateRec);
    
    if (item)
	size += item->size;

    ptr = (PrivateRec *)xcalloc(size, 1);
    if (!ptr)
	return NULL;
    ptr->key = key;
    ptr->value = (size > sizeof(PrivateRec)) ? (ptr + 1) : NULL;
    ptr->next = *privates;
    *privates = ptr;

    /* call any init funcs and return */
    if (item) {
	PrivateCallbackRec calldata = { key, &ptr->value };
	CallCallbacks(&item->initfuncs, &calldata);
    }
    return &ptr->value;
}

/*
 * Look up a private pointer.
 */
_X_EXPORT pointer
dixLookupPrivate(PrivateRec **privates, const DevPrivateKey key)
{
    PrivateRec *rec = *privates;
    pointer *ptr;

    while (rec) {
	if (rec->key == key)
	    return rec->value;
	rec = rec->next;
    }

    ptr = dixAllocatePrivate(privates, key);
    return ptr ? *ptr : NULL;
}

/*
 * Look up the address of a private pointer.
 */
_X_EXPORT pointer *
dixLookupPrivateAddr(PrivateRec **privates, const DevPrivateKey key)
{
    PrivateRec *rec = *privates;

    while (rec) {
	if (rec->key == key)
	    return &rec->value;
	rec = rec->next;
    }

    return dixAllocatePrivate(privates, key);
}

/*
 * Set a private pointer.
 */
_X_EXPORT int
dixSetPrivate(PrivateRec **privates, const DevPrivateKey key, pointer val)
{
    PrivateRec *rec;

 top:
    rec = *privates;
    while (rec) {
	if (rec->key == key) {
	    rec->value = val;
	    return TRUE;
	}
	rec = rec->next;
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
    PrivateRec *ptr, *next;
    PrivateDescRec *item;
    PrivateCallbackRec calldata;

    /* first pass calls the delete callbacks */
    for (ptr = privates; ptr; ptr = ptr->next) {
	item = findItem(ptr->key);
	if (item) {
	    calldata.key = ptr->key;
	    calldata.value = &ptr->value;
	    CallCallbacks(&item->deletefuncs, &calldata);
	}
    }
	
    /* second pass frees the memory */
    ptr = privates;
    while (ptr) {
	next = ptr->next;
	xfree(ptr);
	ptr = next;
    }
}

/*
 * Callback registration
 */
_X_EXPORT int
dixRegisterPrivateInitFunc(const DevPrivateKey key,
			   CallbackProcPtr callback, pointer data)
{
    PrivateDescRec *item = findItem(key);
    if (!item) {
	if (!dixRequestPrivate(key, 0))
	    return FALSE;
	item = findItem(key);
    }
    return AddCallback(&item->initfuncs, callback, data);
}

_X_EXPORT int
dixRegisterPrivateDeleteFunc(const DevPrivateKey key,
			     CallbackProcPtr callback, pointer data)
{
    PrivateDescRec *item = findItem(key);
    if (!item) {
	if (!dixRequestPrivate(key, 0))
	    return FALSE;
	item = findItem(key);
    }
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
    PrivateDescRec *next;

    /* reset internal structures */
    while (items) {
	next = items->next;
	DeleteCallbackList(&items->initfuncs);
	DeleteCallbackList(&items->deletefuncs);
	xfree(items);
	items = next;
    }
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
