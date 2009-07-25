/*
 * Copyright (c) 1997 The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * XFree86 Project, Inc. not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The Xfree86 Project, Inc. makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE XFREE86 PROJECT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE XFREE86 PROJECT, INC. BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Once upon a time, X had multiple loader backends, three of which were
 * essentially libdl reimplementations.  This was nonsense so we chucked
 * it, but we still retain the factorization between loader API and
 * platform implementation.  This file is the libdl implementation, and
 * currently the only backend.  If you find yourself porting to a platform
 * without working libdl - hpux, win32, some forsaken a.out host, etc. -
 * make a new backend rather than hacking up this file.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <X11/Xos.h>
#include "os.h"

#include "loader.h"
#include "dlloader.h"

#if defined(DL_LAZY)
#define DLOPEN_LAZY DL_LAZY
#elif defined(RTLD_LAZY)
#define DLOPEN_LAZY RTLD_LAZY
#elif defined(__FreeBSD__)
#define DLOPEN_LAZY 1
#else
#define DLOPEN_LAZY 0
#endif

#if defined(LD_GLOBAL)
#define DLOPEN_GLOBAL LD_GLOBAL
#elif defined(RTLD_GLOBAL)
#define DLOPEN_GLOBAL RTLD_GLOBAL
#else
#define DLOPEN_GLOBAL 0
#endif

#if defined(CSRG_BASED) && !defined(__ELF__)
#define DLSYM_PREFIX "_"
#else
#define DLSYM_PREFIX ""
#endif

/* Hooray, yet another open coded linked list! FIXME */
typedef struct DLModuleList {
    void *module;
    struct DLModuleList *next;
} DLModuleList;

static DLModuleList *dlModuleList = NULL;

static void *
DLFindSymbolLocal(pointer module, const char *name)
{
    void *p;
    char *n;

    static const char symPrefix[] = DLSYM_PREFIX;

    if (sizeof(symPrefix) > 1) {
	n = malloc(strlen(symPrefix) + strlen(name) + 1);
	sprintf(n, "%s%s", symPrefix, name);
	name = n;
    }

    p = dlsym(module, name);

    if (sizeof(symPrefix) > 1)
	free(n);

    return p;
}

static void *global_scope = NULL;

void *
DLFindSymbol(const char *name)
{
    DLModuleList *l;
    void *p;

    p = dlsym(RTLD_DEFAULT, name);
    if (p != NULL)
	return p;

    for (l = dlModuleList; l != NULL; l = l->next) {
        p = DLFindSymbolLocal(l->module, name);
	if (p)
	    return p;
    }

    if (!global_scope)
	global_scope = dlopen(NULL, DLOPEN_LAZY | DLOPEN_GLOBAL);

    if (global_scope)
	return DLFindSymbolLocal(global_scope, name);

    return NULL;
}

void *
DLLoadModule(loaderPtr modrec, int flags)
{
    void * dlfile;
    DLModuleList *l;
    int dlopen_flags;

    if (flags & LD_FLAG_GLOBAL)
	dlopen_flags = DLOPEN_LAZY | DLOPEN_GLOBAL;
    else
	dlopen_flags = DLOPEN_LAZY;
    dlfile = dlopen(modrec->name, dlopen_flags);
    if (dlfile == NULL) {
	ErrorF("dlopen: %s\n", dlerror());
	return NULL;
    }

    l = malloc(sizeof(DLModuleList));
    l->module = dlfile;
    l->next = dlModuleList;
    dlModuleList = l;

    return (void *)dlfile;
}

void
DLUnloadModule(void *modptr)
{
    DLModuleList *l, *p;

    /* remove it from dlModuleList. */
    if (dlModuleList->module == modptr) {
	l = dlModuleList;
	dlModuleList = l->next;
	free(l);
    } else {
	p = dlModuleList;
	for (l = dlModuleList->next; l != NULL; l = l->next) {
	    if (l->module == modptr) {
		p->next = l->next;
		free(l);
		break;
	    }
	    p = l;
	}
    }
    dlclose(modptr);
}
