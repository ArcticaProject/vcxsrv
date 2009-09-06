/*
 * Copyright 1995-1998 by Metro Link, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Metro Link, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Metro Link, Inc. makes no
 * representations about the suitability of this software for any purpose.
 *  It is provided "as is" without express or implied warranty.
 *
 * METRO LINK, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL METRO LINK, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#if defined(UseMMAP) || (defined(linux) && defined(__ia64__))
#include <sys/mman.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#if defined(linux) && \
    (defined(__alpha__) || defined(__powerpc__) || defined(__ia64__) \
    || defined(__amd64__))
#include <malloc.h>
#endif
#include <stdarg.h>

#include "os.h"
#include "loader.h"
#include "loaderProcs.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "compiler.h"

extern void *xorg_symbols[];

#define MAX_HANDLE 256
static int refCount[MAX_HANDLE];

static int moduleseq = 0;

/* Prototypes for static functions. */
static loaderPtr listHead = NULL;

static loaderPtr
_LoaderListPush(void)
{
    loaderPtr item = calloc(1, sizeof(struct _loader));

    item->next = listHead;
    listHead = item;

    return item;
}

static loaderPtr
_LoaderListPop(int handle)
{
    loaderPtr item = listHead;
    loaderPtr *bptr = &listHead;	/* pointer to previous node */

    while (item) {
	if (item->handle == handle) {
	    *bptr = item->next;	/* remove this from the list */
	    return item;
	}
	bptr = &(item->next);
	item = item->next;
    }

    return 0;
}

void
LoaderInit(void)
{
    xf86MsgVerb(X_INFO, 2, "Loader magic: %p\n", (void *)xorg_symbols);
    xf86MsgVerb(X_INFO, 2, "Module ABI versions:\n");
    xf86ErrorFVerb(2, "\t%s: %d.%d\n", ABI_CLASS_ANSIC,
		   GET_ABI_MAJOR(LoaderVersionInfo.ansicVersion),
		   GET_ABI_MINOR(LoaderVersionInfo.ansicVersion));
    xf86ErrorFVerb(2, "\t%s: %d.%d\n", ABI_CLASS_VIDEODRV,
		   GET_ABI_MAJOR(LoaderVersionInfo.videodrvVersion),
		   GET_ABI_MINOR(LoaderVersionInfo.videodrvVersion));
    xf86ErrorFVerb(2, "\t%s : %d.%d\n", ABI_CLASS_XINPUT,
		   GET_ABI_MAJOR(LoaderVersionInfo.xinputVersion),
		   GET_ABI_MINOR(LoaderVersionInfo.xinputVersion));
    xf86ErrorFVerb(2, "\t%s : %d.%d\n", ABI_CLASS_EXTENSION,
		   GET_ABI_MAJOR(LoaderVersionInfo.extensionVersion),
		   GET_ABI_MINOR(LoaderVersionInfo.extensionVersion));

#if defined(__UNIXWARE__) && !defined(__GNUC__)
    /* For UnixWare we need to load the C Runtime libraries which are
     * normally auto-linked by the compiler. Otherwise we are bound to
     * see unresolved symbols when trying to use the type "long long".
     * Obviously, this does not apply if the GNU C compiler is used.
     */
    {
	int errmaj, errmin, wasLoaded; /* place holders */
	char *xcrtpath = DEFAULT_MODULE_PATH "/libcrt.a";
	char *uwcrtpath = "/usr/ccs/lib/libcrt.a";
	char *path;
	struct stat st;

	if(stat(xcrtpath, &st) < 0)
	    path = uwcrtpath; /* fallback: try to get libcrt.a from the uccs */
	else
	    path = xcrtpath; /* get the libcrt.a we compiled with */
	LoaderOpen (path, "libcrt", 0, &errmaj, &errmin, &wasLoaded);
    }
#endif
}

/* Public Interface to the loader. */

int
LoaderOpen(const char *module, const char *cname, int handle,
	   int *errmaj, int *errmin, int *wasLoaded, int flags)
{
    loaderPtr tmp;
    int new_handle;

#if defined(DEBUG)
    ErrorF("LoaderOpen(%s)\n", module);
#endif

    /* Is the module already loaded? */
    if (handle >= 0) {
	tmp = listHead;
	while (tmp) {
#ifdef DEBUGLIST
	    ErrorF("strcmp(%x(%s),{%x} %x(%s))\n", module, module,
		   &(tmp->name), tmp->name, tmp->name);
#endif
	    if (!strcmp(module, tmp->name)) {
		refCount[tmp->handle]++;
		if (wasLoaded)
		    *wasLoaded = 1;
		xf86MsgVerb(X_INFO, 2, "Reloading %s\n", module);
		return tmp->handle;
	    }
	    tmp = tmp->next;
	}
    }

    /*
     * OK, it's a new one. Add it.
     */
    xf86Msg(X_INFO, "Loading %s\n", module);
    if (wasLoaded)
	*wasLoaded = 0;

    /*
     * Find a free handle.
     */
    new_handle = 1;
    while (new_handle < MAX_HANDLE && refCount[new_handle])
	new_handle++;

    if (new_handle == MAX_HANDLE) {
	xf86Msg(X_ERROR, "Out of loader space\n");	/* XXX */
	if (errmaj)
	    *errmaj = LDR_NOSPACE;
	if (errmin)
	    *errmin = LDR_NOSPACE;
	return -1;
    }

    refCount[new_handle] = 1;

    tmp = _LoaderListPush();
    tmp->name = malloc(strlen(module) + 1);
    strcpy(tmp->name, module);
    tmp->cname = malloc(strlen(cname) + 1);
    strcpy(tmp->cname, cname);
    tmp->handle = new_handle;
    tmp->module = moduleseq++;

    if ((tmp->private = DLLoadModule(tmp, flags)) == NULL) {
	xf86Msg(X_ERROR, "Failed to load %s\n", module);
	_LoaderListPop(new_handle);
	refCount[new_handle] = 0;
	if (errmaj)
	    *errmaj = LDR_NOLOAD;
	if (errmin)
	    *errmin = LDR_NOLOAD;
	return -1;
    }

    return new_handle;
}

int
LoaderHandleOpen(int handle)
{
    if (handle < 0 || handle >= MAX_HANDLE)
	return -1;

    if (!refCount[handle])
	return -1;

    refCount[handle]++;
    return handle;
}

void *
LoaderSymbol(const char *sym)
{
    return (DLFindSymbol(sym));
}

int
LoaderUnload(int handle)
{
    loaderRec fakeHead;
    loaderPtr tmp = &fakeHead;

    if (handle < 0 || handle >= MAX_HANDLE)
	return -1;

    /*
     * check the reference count, only free it if it goes to zero
     */
    if (--refCount[handle])
	return 0;
    /*
     * find the loaderRecs associated with this handle.
     */

    while ((tmp = _LoaderListPop(handle)) != NULL) {
	xf86Msg(X_INFO, "Unloading %s\n", tmp->name);
	DLUnloadModule(tmp->private);
	free(tmp->name);
	free(tmp->cname);
	free(tmp);
    }

    return 0;
}

unsigned long LoaderOptions = 0;

void
LoaderSetOptions(unsigned long opts)
{
    LoaderOptions |= opts;
}

Bool
LoaderShouldIgnoreABI(void)
{
    return (LoaderOptions & LDR_OPT_ABI_MISMATCH_NONFATAL) != 0;
}

int
LoaderGetABIVersion(const char *abiclass)
{
    struct {
        const char *name;
        int version;
    } classes[] = {
        { ABI_CLASS_ANSIC,     LoaderVersionInfo.ansicVersion },
        { ABI_CLASS_VIDEODRV,  LoaderVersionInfo.videodrvVersion },
        { ABI_CLASS_XINPUT,    LoaderVersionInfo.xinputVersion },
        { ABI_CLASS_EXTENSION, LoaderVersionInfo.extensionVersion },
        { ABI_CLASS_FONT,      LoaderVersionInfo.fontVersion },
        { NULL,                0 }
    };
    int i;

    for(i = 0; classes[i].name; i++) {
        if(!strcmp(classes[i].name, abiclass)) {
            return classes[i].version;
        }
    }

    return 0;
}
