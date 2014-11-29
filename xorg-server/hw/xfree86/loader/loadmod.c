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
 * Copyright (c) 1997-2002 by The XFree86 Project, Inc.
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

#include "os.h"
/* For stat() and related stuff */
#define NO_OSLIB_PROTOTYPES
#include "xf86_OSlib.h"
#define LOADERDECLARATIONS
#include "loaderProcs.h"
#include "misc.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Xinput.h"
#include "loader.h"
#include "xf86Optrec.h"

#include <sys/types.h>
#include <regex.h>
#include <dirent.h>
#include <limits.h>

typedef struct _pattern {
    const char *pattern;
    regex_t rex;
} PatternRec, *PatternPtr;

/* Prototypes for static functions */
static char *FindModule(const char *, const char *, const char **, PatternPtr);
static Bool CheckVersion(const char *, XF86ModuleVersionInfo *,
                         const XF86ModReqInfo *);
static char *LoaderGetCanonicalName(const char *, PatternPtr);
static void RemoveChild(ModuleDescPtr);

const ModuleVersions LoaderVersionInfo = {
    XORG_VERSION_CURRENT,
    ABI_ANSIC_VERSION,
    ABI_VIDEODRV_VERSION,
    ABI_XINPUT_VERSION,
    ABI_EXTENSION_VERSION,
    ABI_FONT_VERSION
};

static int ModuleDuplicated[] = { };

static void
FreeStringList(char **paths)
{
    char **p;

    if (!paths)
        return;

    for (p = paths; *p; p++)
        free(*p);

    free(paths);
}

static char **defaultPathList = NULL;

static Bool
PathIsAbsolute(const char *path)
{
    return *path == '/';
}

/*
 * Convert a comma-separated path into a NULL-terminated array of path
 * elements, rejecting any that are not full absolute paths, and appending
 * a '/' when it isn't already present.
 */
static char **
InitPathList(const char *path)
{
    char *fullpath = NULL;
    char *elem = NULL;
    char **list = NULL, **save = NULL;
    int len;
    int addslash;
    int n = 0;

    if (!path)
        return defaultPathList;

    fullpath = strdup(path);
    if (!fullpath)
        return NULL;
    elem = strtok(fullpath, ",");
    while (elem) {
        if (PathIsAbsolute(elem)) {
            len = strlen(elem);
            addslash = (elem[len - 1] != '/');
            if (addslash)
                len++;
            save = list;
            list = realloc(list, (n + 2) * sizeof(char *));
            if (!list) {
                if (save) {
                    save[n] = NULL;
                    FreeStringList(save);
                }
                free(fullpath);
                return NULL;
            }
            list[n] = malloc(len + 1);
            if (!list[n]) {
                FreeStringList(list);
                free(fullpath);
                return NULL;
            }
            strcpy(list[n], elem);
            if (addslash) {
                list[n][len - 1] = '/';
                list[n][len] = '\0';
            }
            n++;
        }
        elem = strtok(NULL, ",");
    }
    if (list)
        list[n] = NULL;
    free(fullpath);
    return list;
}

static void
FreePathList(char **pathlist)
{
    if (pathlist && pathlist != defaultPathList)
        FreeStringList(pathlist);
}

void
LoaderSetPath(const char *path)
{
    if (!path)
        return;

    defaultPathList = InitPathList(path);
}

/* Standard set of module subdirectories to search, in order of preference */
static const char *stdSubdirs[] = {
    "",
    "input/",
    "drivers/",
    "extensions/",
    "internal/",
    NULL
};

/*
 * Standard set of module name patterns to check, in order of preference
 * These are regular expressions (suitable for use with POSIX regex(3)).
 *
 * This list assumes that you're an ELFish platform and therefore your
 * shared libraries are named something.so.  If we're ever nuts enough
 * to port this DDX to, say, Darwin, we'll need to fix this.
 */
static PatternRec stdPatterns[] = {
#ifdef __CYGWIN__
    {"^cyg(.*)\\.dll$",},
    {"(.*)_drv\\.dll$",},
    {"(.*)\\.dll$",},
#else
    {"^lib(.*)\\.so$",},
    {"(.*)_drv\\.so$",},
    {"(.*)\\.so$",},
#endif
    {NULL,}
};

static PatternPtr
InitPatterns(const char **patternlist)
{
    char errmsg[80];
    int i, e;
    PatternPtr patterns = NULL;
    PatternPtr p = NULL;
    static int firstTime = 1;
    const char **s;

    if (firstTime) {
        /* precompile stdPatterns */
        firstTime = 0;
        for (p = stdPatterns; p->pattern; p++)
            if ((e = regcomp(&p->rex, p->pattern, REG_EXTENDED)) != 0) {
                regerror(e, &p->rex, errmsg, sizeof(errmsg));
                FatalError("InitPatterns: regcomp error for `%s': %s\n",
                           p->pattern, errmsg);
            }
    }

    if (patternlist) {
        for (i = 0, s = patternlist; *s; i++, s++)
            if (*s == DEFAULT_LIST)
                i += sizeof(stdPatterns) / sizeof(stdPatterns[0]) - 1 - 1;
        patterns = malloc((i + 1) * sizeof(PatternRec));
        if (!patterns) {
            return NULL;
        }
        for (i = 0, s = patternlist; *s; i++, s++)
            if (*s != DEFAULT_LIST) {
                p = patterns + i;
                p->pattern = *s;
                if ((e = regcomp(&p->rex, p->pattern, REG_EXTENDED)) != 0) {
                    regerror(e, &p->rex, errmsg, sizeof(errmsg));
                    ErrorF("InitPatterns: regcomp error for `%s': %s\n",
                           p->pattern, errmsg);
                    i--;
                }
            }
            else {
                for (p = stdPatterns; p->pattern; p++, i++)
                    patterns[i] = *p;
                if (p != stdPatterns)
                    i--;
            }
        patterns[i].pattern = NULL;
    }
    else
        patterns = stdPatterns;
    return patterns;
}

static void
FreePatterns(PatternPtr patterns)
{
    if (patterns && patterns != stdPatterns)
        free(patterns);
}

static const char **
InitSubdirs(const char **subdirlist)
{
    int i;
    const char **tmp_subdirlist = NULL;
    char **subdirs = NULL;
    const char **s, **stmp = NULL;
    const char *osname;
    const char *slash;
    int oslen = 0, len;
    Bool indefault;

    if (subdirlist == NULL) {
        subdirlist = tmp_subdirlist = malloc(2 * sizeof(char *));
        if (subdirlist == NULL)
            return NULL;
        subdirlist[0] = DEFAULT_LIST;
        subdirlist[1] = NULL;
    }

    LoaderGetOS(&osname, NULL, NULL, NULL);
    oslen = strlen(osname);

    {
        /* Count number of entries and check for invalid paths */
        for (i = 0, s = subdirlist; *s; i++, s++) {
            if (*s == DEFAULT_LIST) {
                i += sizeof(stdSubdirs) / sizeof(stdSubdirs[0]) - 1 - 1;
            }
            else {
                /*
                 * Path validity check.  Don't allow absolute paths, or
                 * paths containing "..".  To catch absolute paths on
                 * platforms that use driver letters, don't allow the ':'
                 * character to appear at all.
                 */
                if (**s == '/' || **s == '\\' || strchr(*s, ':') ||
                    strstr(*s, "..")) {
                    xf86Msg(X_ERROR, "InitSubdirs: Bad subdir: \"%s\"\n", *s);
                    free(tmp_subdirlist);
                    return NULL;
                }
            }
        }
        subdirs = malloc((i * 2 + 1) * sizeof(char *));
        if (!subdirs) {
            free(tmp_subdirlist);
            return NULL;
        }
        i = 0;
        s = subdirlist;
        indefault = FALSE;
        while (*s) {
            if (*s == DEFAULT_LIST) {
                /* Divert to the default list */
                indefault = TRUE;
                stmp = ++s;
                s = stdSubdirs;
            }
            len = strlen(*s);
            if (**s && (*s)[len - 1] != '/') {
                slash = "/";
                len++;
            }
            else
                slash = "";
            len += oslen + 2;
            if (!(subdirs[i] = malloc(len))) {
                while (--i >= 0)
                    free(subdirs[i]);
                free(subdirs);
                free(tmp_subdirlist);
                return NULL;
            }
            /* tack on the OS name */
            sprintf(subdirs[i], "%s%s%s/", *s, slash, osname);
            i++;
            /* path as given */
            subdirs[i] = strdup(*s);
            i++;
            s++;
            if (indefault && !s) {
                /* revert back to the main list */
                indefault = FALSE;
                s = stmp;
            }
        }
        subdirs[i] = NULL;
    }
    free(tmp_subdirlist);
    return (const char **) subdirs;
}

static void
FreeSubdirs(const char **subdirs)
{
    const char **s;

    if (subdirs) {
        for (s = subdirs; *s; s++)
            free((char *) *s);
        free(subdirs);
    }
}

static char *
FindModuleInSubdir(const char *dirpath, const char *module)
{
    struct dirent *direntry = NULL;
    DIR *dir = NULL;
    char *ret = NULL, tmpBuf[PATH_MAX];
    struct stat stat_buf;

    dir = opendir(dirpath);
    if (!dir)
        return NULL;

    while ((direntry = readdir(dir))) {
        if (direntry->d_name[0] == '.')
            continue;
        snprintf(tmpBuf, PATH_MAX, "%s%s/", dirpath, direntry->d_name);
        /* the stat with the appended / fails for normal files,
           and works for sub dirs fine, looks a bit strange in strace
           but does seem to work */
        if ((stat(tmpBuf, &stat_buf) == 0) && S_ISDIR(stat_buf.st_mode)) {
            if ((ret = FindModuleInSubdir(tmpBuf, module)))
                break;
            continue;
        }

#ifdef __CYGWIN__
        snprintf(tmpBuf, PATH_MAX, "cyg%s.dll", module);
#else
        snprintf(tmpBuf, PATH_MAX, "lib%s.so", module);
#endif
        if (strcmp(direntry->d_name, tmpBuf) == 0) {
            if (asprintf(&ret, "%s%s", dirpath, tmpBuf) == -1)
                ret = NULL;
            break;
        }

#ifdef __CYGWIN__
        snprintf(tmpBuf, PATH_MAX, "%s_drv.dll", module);
#else
        snprintf(tmpBuf, PATH_MAX, "%s_drv.so", module);
#endif
        if (strcmp(direntry->d_name, tmpBuf) == 0) {
            if (asprintf(&ret, "%s%s", dirpath, tmpBuf) == -1)
                ret = NULL;
            break;
        }

#ifdef __CYGWIN__
        snprintf(tmpBuf, PATH_MAX, "%s.dll", module);
#else
        snprintf(tmpBuf, PATH_MAX, "%s.so", module);
#endif
        if (strcmp(direntry->d_name, tmpBuf) == 0) {
            if (asprintf(&ret, "%s%s", dirpath, tmpBuf) == -1)
                ret = NULL;
            break;
        }
    }

    closedir(dir);
    return ret;
}

static char *
FindModule(const char *module, const char *dirname, const char **subdirlist,
           PatternPtr patterns)
{
    char buf[PATH_MAX + 1];
    char *name = NULL;
    const char **subdirs = NULL;
    const char **s;

    if (strlen(dirname) > PATH_MAX)
        return NULL;

    subdirs = InitSubdirs(subdirlist);
    if (!subdirs)
        return NULL;

    for (s = subdirs; *s; s++) {
        if ((strlen(dirname) + strlen(*s)) > PATH_MAX)
            continue;
        strcpy(buf, dirname);
        strcat(buf, *s);
        if ((name = FindModuleInSubdir(buf, module)))
            break;
    }

    FreeSubdirs(subdirs);

    return name;
}

const char **
LoaderListDirs(const char **subdirlist, const char **patternlist)
{
    char buf[PATH_MAX + 1];
    char **pathlist;
    char **elem;
    const char **subdirs;
    const char **s;
    PatternPtr patterns = NULL;
    PatternPtr p;
    DIR *d;
    struct dirent *dp;
    regmatch_t match[2];
    struct stat stat_buf;
    int len, dirlen;
    char *fp;
    char **listing = NULL;
    char **save;
    char **ret = NULL;
    int n = 0;

    if (!(pathlist = InitPathList(NULL)))
        return NULL;
    if (!(subdirs = InitSubdirs(subdirlist)))
        goto bail;
    if (!(patterns = InitPatterns(patternlist)))
        goto bail;

    for (elem = pathlist; *elem; elem++) {
        for (s = subdirs; *s; s++) {
            if ((dirlen = strlen(*elem) + strlen(*s)) > PATH_MAX)
                continue;
            strcpy(buf, *elem);
            strcat(buf, *s);
            fp = buf + dirlen;
            if (stat(buf, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode) &&
                (d = opendir(buf))) {
                if (buf[dirlen - 1] != '/') {
                    buf[dirlen++] = '/';
                    fp++;
                }
                while ((dp = readdir(d))) {
                    if (dirlen + strlen(dp->d_name) > PATH_MAX)
                        continue;
                    strcpy(fp, dp->d_name);
                    if (!(stat(buf, &stat_buf) == 0 &&
                          S_ISREG(stat_buf.st_mode)))
                        continue;
                    for (p = patterns; p->pattern; p++) {
                        if (regexec(&p->rex, dp->d_name, 2, match, 0) == 0 &&
                            match[1].rm_so != -1) {
                            len = match[1].rm_eo - match[1].rm_so;
                            save = listing;
                            listing = realloc(listing,
                                              (n + 2) * sizeof(char *));
                            if (!listing) {
                                if (save) {
                                    save[n] = NULL;
                                    FreeStringList(save);
                                }
                                closedir(d);
                                goto bail;
                            }
                            listing[n] = malloc(len + 1);
                            if (!listing[n]) {
                                FreeStringList(listing);
                                closedir(d);
                                goto bail;
                            }
                            strncpy(listing[n], dp->d_name + match[1].rm_so,
                                    len);
                            listing[n][len] = '\0';
                            n++;
                            break;
                        }
                    }
                }
                closedir(d);
            }
        }
    }
    if (listing)
        listing[n] = NULL;
    ret = listing;

 bail:
    FreePatterns(patterns);
    FreeSubdirs(subdirs);
    FreePathList(pathlist);
    return (const char **) ret;
}

void
LoaderFreeDirList(char **list)
{
    FreeStringList(list);
}

static Bool
CheckVersion(const char *module, XF86ModuleVersionInfo * data,
             const XF86ModReqInfo * req)
{
    int vercode[4];
    long ver = data->xf86version;
    MessageType errtype;

    xf86Msg(X_INFO, "Module %s: vendor=\"%s\"\n",
            data->modname ? data->modname : "UNKNOWN!",
            data->vendor ? data->vendor : "UNKNOWN!");

    vercode[0] = ver / 10000000;
    vercode[1] = (ver / 100000) % 100;
    vercode[2] = (ver / 1000) % 100;
    vercode[3] = ver % 1000;
    xf86ErrorF("\tcompiled for %d.%d.%d", vercode[0], vercode[1], vercode[2]);
    if (vercode[3] != 0)
        xf86ErrorF(".%d", vercode[3]);
    xf86ErrorF(", module version = %d.%d.%d\n", data->majorversion,
               data->minorversion, data->patchlevel);

    if (data->moduleclass)
        xf86ErrorFVerb(2, "\tModule class: %s\n", data->moduleclass);

    ver = -1;
    if (data->abiclass) {
        int abimaj, abimin;
        int vermaj, vermin;

        if (!strcmp(data->abiclass, ABI_CLASS_ANSIC))
            ver = LoaderVersionInfo.ansicVersion;
        else if (!strcmp(data->abiclass, ABI_CLASS_VIDEODRV))
            ver = LoaderVersionInfo.videodrvVersion;
        else if (!strcmp(data->abiclass, ABI_CLASS_XINPUT))
            ver = LoaderVersionInfo.xinputVersion;
        else if (!strcmp(data->abiclass, ABI_CLASS_EXTENSION))
            ver = LoaderVersionInfo.extensionVersion;
        else if (!strcmp(data->abiclass, ABI_CLASS_FONT))
            ver = LoaderVersionInfo.fontVersion;

        abimaj = GET_ABI_MAJOR(data->abiversion);
        abimin = GET_ABI_MINOR(data->abiversion);
        xf86ErrorFVerb(2, "\tABI class: %s, version %d.%d\n",
                       data->abiclass, abimaj, abimin);
        if (ver != -1) {
            vermaj = GET_ABI_MAJOR(ver);
            vermin = GET_ABI_MINOR(ver);
            if (abimaj != vermaj) {
                if (LoaderOptions & LDR_OPT_ABI_MISMATCH_NONFATAL)
                    errtype = X_WARNING;
                else
                    errtype = X_ERROR;
                xf86MsgVerb(errtype, 0,
                            "module ABI major version (%d) doesn't"
                            " match the server's version (%d)\n",
                            abimaj, vermaj);
                if (!(LoaderOptions & LDR_OPT_ABI_MISMATCH_NONFATAL))
                    return FALSE;
            }
            else if (abimin > vermin) {
                if (LoaderOptions & LDR_OPT_ABI_MISMATCH_NONFATAL)
                    errtype = X_WARNING;
                else
                    errtype = X_ERROR;
                xf86MsgVerb(errtype, 0,
                            "module ABI minor version (%d) is "
                            "newer than the server's version "
                            "(%d)\n", abimin, vermin);
                if (!(LoaderOptions & LDR_OPT_ABI_MISMATCH_NONFATAL))
                    return FALSE;
            }
        }
    }

    /* Check against requirements that the caller has specified */
    if (req) {
        if (req->majorversion != MAJOR_UNSPEC) {
            if (data->majorversion != req->majorversion) {
                xf86MsgVerb(X_WARNING, 2, "module major version (%d) "
                            "doesn't match required major version (%d)\n",
                            data->majorversion, req->majorversion);
                return FALSE;
            }
            else if (req->minorversion != MINOR_UNSPEC) {
                if (data->minorversion < req->minorversion) {
                    xf86MsgVerb(X_WARNING, 2, "module minor version (%d) "
                                "is less than the required minor version (%d)\n",
                                data->minorversion, req->minorversion);
                    return FALSE;
                }
                else if (data->minorversion == req->minorversion &&
                         req->patchlevel != PATCH_UNSPEC) {
                    if (data->patchlevel < req->patchlevel) {
                        xf86MsgVerb(X_WARNING, 2, "module patch level (%d) "
                                    "is less than the required patch level (%d)\n",
                                    data->patchlevel, req->patchlevel);
                        return FALSE;
                    }
                }
            }
        }
        if (req->moduleclass) {
            if (!data->moduleclass ||
                strcmp(req->moduleclass, data->moduleclass)) {
                xf86MsgVerb(X_WARNING, 2, "Module class (%s) doesn't match "
                            "the required class (%s)\n",
                            data->moduleclass ? data->moduleclass : "<NONE>",
                            req->moduleclass);
                return FALSE;
            }
        }
        else if (req->abiclass != ABI_CLASS_NONE) {
            if (!data->abiclass || strcmp(req->abiclass, data->abiclass)) {
                xf86MsgVerb(X_WARNING, 2, "ABI class (%s) doesn't match the "
                            "required ABI class (%s)\n",
                            data->abiclass ? data->abiclass : "<NONE>",
                            req->abiclass);
                return FALSE;
            }
        }
        if ((req->abiclass != ABI_CLASS_NONE) &&
            req->abiversion != ABI_VERS_UNSPEC) {
            int reqmaj, reqmin, maj, min;

            reqmaj = GET_ABI_MAJOR(req->abiversion);
            reqmin = GET_ABI_MINOR(req->abiversion);
            maj = GET_ABI_MAJOR(data->abiversion);
            min = GET_ABI_MINOR(data->abiversion);
            if (maj != reqmaj) {
                xf86MsgVerb(X_WARNING, 2, "ABI major version (%d) doesn't "
                            "match the required ABI major version (%d)\n",
                            maj, reqmaj);
                return FALSE;
            }
            /* XXX Maybe this should be the other way around? */
            if (min > reqmin) {
                xf86MsgVerb(X_WARNING, 2, "module ABI minor version (%d) "
                            "is newer than that available (%d)\n", min, reqmin);
                return FALSE;
            }
        }
    }
    return TRUE;
}

static ModuleDescPtr
AddSibling(ModuleDescPtr head, ModuleDescPtr new)
{
    new->sib = head;
    return new;
}

void *
LoadSubModule(void *_parent, const char *module,
              const char **subdirlist, const char **patternlist,
              void *options, const XF86ModReqInfo * modreq,
              int *errmaj, int *errmin)
{
    ModuleDescPtr submod;
    ModuleDescPtr parent = (ModuleDescPtr) _parent;

    xf86MsgVerb(X_INFO, 3, "Loading sub module \"%s\"\n", module);

    if (PathIsAbsolute(module)) {
        xf86Msg(X_ERROR,
                "LoadSubModule: Absolute module path not permitted: \"%s\"\n",
                module);
        if (errmaj)
            *errmaj = LDR_BADUSAGE;
        if (errmin)
            *errmin = 0;
        return NULL;
    }

    submod = LoadModule(module, NULL, subdirlist, patternlist, options,
                        modreq, errmaj, errmin);
    if (submod && submod != (ModuleDescPtr) 1) {
        parent->child = AddSibling(parent->child, submod);
        submod->parent = parent;
    }
    return submod;
}

static ModuleDescPtr
NewModuleDesc(const char *name)
{
    ModuleDescPtr mdp = calloc(1, sizeof(ModuleDesc));

    if (mdp)
        mdp->name = xstrdup(name);

    return mdp;
}

ModuleDescPtr
DuplicateModule(ModuleDescPtr mod, ModuleDescPtr parent)
{
    ModuleDescPtr ret;

    if (!mod)
        return NULL;

    ret = NewModuleDesc(mod->name);
    if (ret == NULL)
        return NULL;

    ret->handle = mod->handle;

    ret->SetupProc = mod->SetupProc;
    ret->TearDownProc = mod->TearDownProc;
    ret->TearDownData = ModuleDuplicated;
    ret->child = DuplicateModule(mod->child, ret);
    ret->sib = DuplicateModule(mod->sib, parent);
    ret->parent = parent;
    ret->VersionInfo = mod->VersionInfo;
    ret->path = strdup(mod->path);

    return ret;
}

static const char *compiled_in_modules[] = {
    "ddc",
    "i2c",
    "ramdac",
    "dbe",
    "record",
    "extmod",
    "dri",
    "dri2",
#if DRI3
    "dri3",
#endif
#if PRESENT
    "present",
#endif
    NULL
};

/*
 * LoadModule: load a module
 *
 * module       The module name.  Normally this is not a filename but the
 *              module's "canonical name.  A full pathname is, however,
 *              also accepted.
 * path         A comma separated list of module directories.
 * subdirlist   A NULL terminated list of subdirectories to search.  When
 *              NULL, the default "stdSubdirs" list is used.  The default
 *              list is also substituted for entries with value DEFAULT_LIST.
 * patternlist  A NULL terminated list of regular expressions used to find
 *              module filenames.  Each regex should contain exactly one
 *              subexpression that corresponds to the canonical module name.
 *              When NULL, the default "stdPatterns" list is used.  The
 *              default list is also substituted for entries with value
 *              DEFAULT_LIST.
 * options      A NULL terminated list of Options that are passed to the
 *              module's SetupProc function.
 * modreq       An optional XF86ModReqInfo* containing
 *              version/ABI/vendor-ABI requirements to check for when
 *              loading the module.  The following fields of the
 *              XF86ModReqInfo struct are checked:
 *                majorversion - must match the module's majorversion exactly
 *                minorversion - the module's minorversion must be >= this
 *                patchlevel   - the module's minorversion.patchlevel must be
 *                               >= this.  Patchlevel is ignored when
 *                               minorversion is not set.
 *                abiclass     - (string) must match the module's abiclass
 *                abiversion   - must be consistent with the module's
 *                               abiversion (major equal, minor no older)
 *                moduleclass  - string must match the module's moduleclass
 *                               string
 *              "don't care" values are ~0 for numbers, and NULL for strings
 * errmaj       Major error return.
 * errmin       Minor error return.
 *
 */
ModuleDescPtr
LoadModule(const char *module, const char *path, const char **subdirlist,
           const char **patternlist, void *options,
           const XF86ModReqInfo * modreq, int *errmaj, int *errmin)
{
    XF86ModuleData *initdata = NULL;
    char **pathlist = NULL;
    char *found = NULL;
    char *name = NULL;
    char **path_elem = NULL;
    char *p = NULL;
    ModuleDescPtr ret = NULL;
    PatternPtr patterns = NULL;
    int noncanonical = 0;
    char *m = NULL;
    const char **cim;

    xf86MsgVerb(X_INFO, 3, "LoadModule: \"%s\"", module);

    patterns = InitPatterns(patternlist);
    name = LoaderGetCanonicalName(module, patterns);
    noncanonical = (name && strcmp(module, name) != 0);
    if (noncanonical) {
        xf86ErrorFVerb(3, " (%s)\n", name);
        xf86MsgVerb(X_WARNING, 1,
                    "LoadModule: given non-canonical module name \"%s\"\n",
                    module);
        m = name;
    }
    else {
        xf86ErrorFVerb(3, "\n");
        m = (char *) module;
    }

    for (cim = compiled_in_modules; *cim; cim++)
        if (!strcmp(m, *cim)) {
            xf86MsgVerb(X_INFO, 3, "Module \"%s\" already built-in\n", m);
            ret = (ModuleDescPtr) 1;
            goto LoadModule_exit;
        }

    if (!name) {
        if (errmaj)
            *errmaj = LDR_BADUSAGE;
        if (errmin)
            *errmin = 0;
        goto LoadModule_fail;
    }
    ret = NewModuleDesc(name);
    if (!ret) {
        if (errmaj)
            *errmaj = LDR_NOMEM;
        if (errmin)
            *errmin = 0;
        goto LoadModule_fail;
    }

    pathlist = InitPathList(path);
    if (!pathlist) {
        /* This could be a malloc failure too */
        if (errmaj)
            *errmaj = LDR_BADUSAGE;
        if (errmin)
            *errmin = 1;
        goto LoadModule_fail;
    }

    /*
     * if the module name is not a full pathname, we need to
     * check the elements in the path
     */
    if (PathIsAbsolute(module))
        found = xstrdup(module);
    path_elem = pathlist;
    while (!found && *path_elem != NULL) {
        found = FindModule(m, *path_elem, subdirlist, patterns);
        path_elem++;
        /*
         * When the module name isn't the canonical name, search for the
         * former if no match was found for the latter.
         */
        if (!*path_elem && m == name) {
            path_elem = pathlist;
            m = (char *) module;
        }
    }

    /*
     * did we find the module?
     */
    if (!found) {
        xf86Msg(X_WARNING, "Warning, couldn't open module %s\n", module);
        if (errmaj)
            *errmaj = LDR_NOENT;
        if (errmin)
            *errmin = 0;
        goto LoadModule_fail;
    }
    ret->handle = LoaderOpen(found, errmaj, errmin);
    if (ret->handle == NULL)
        goto LoadModule_fail;
    ret->path = strdup(found);

    /* drop any explicit suffix from the module name */
    p = strchr(name, '.');
    if (p)
        *p = '\0';

    /*
     * now check if the special data object <modulename>ModuleData is
     * present.
     */
    if (asprintf(&p, "%sModuleData", name) == -1) {
        p = NULL;
        if (errmaj)
            *errmaj = LDR_NOMEM;
        if (errmin)
            *errmin = 0;
        goto LoadModule_fail;
    }
    initdata = LoaderSymbolFromModule(ret->handle, p);
    if (initdata) {
        ModuleSetupProc setup;
        ModuleTearDownProc teardown;
        XF86ModuleVersionInfo *vers;

        vers = initdata->vers;
        setup = initdata->setup;
        teardown = initdata->teardown;

        if (vers) {
            if (!CheckVersion(module, vers, modreq)) {
                if (errmaj)
                    *errmaj = LDR_MISMATCH;
                if (errmin)
                    *errmin = 0;
                goto LoadModule_fail;
            }
        }
        else {
            xf86Msg(X_ERROR,
                    "LoadModule: Module %s does not supply"
                    " version information\n", module);
            if (errmaj)
                *errmaj = LDR_INVALID;
            if (errmin)
                *errmin = 0;
            goto LoadModule_fail;
        }
        if (setup)
            ret->SetupProc = setup;
        if (teardown)
            ret->TearDownProc = teardown;
        ret->VersionInfo = vers;
    }
    else {
        /* no initdata, fail the load */
        xf86Msg(X_ERROR, "LoadModule: Module %s does not have a %s "
                "data object.\n", module, p);
        if (errmaj)
            *errmaj = LDR_INVALID;
        if (errmin)
            *errmin = 0;
        goto LoadModule_fail;
    }
    if (ret->SetupProc) {
        ret->TearDownData = ret->SetupProc(ret, options, errmaj, errmin);
        if (!ret->TearDownData) {
            goto LoadModule_fail;
        }
    }
    else if (options) {
        xf86Msg(X_WARNING, "Module Options present, but no SetupProc "
                "available for %s\n", module);
    }
    goto LoadModule_exit;

 LoadModule_fail:
    UnloadModule(ret);
    ret = NULL;

 LoadModule_exit:
    FreePathList(pathlist);
    FreePatterns(patterns);
    free(found);
    free(name);
    free(p);

    return ret;
}

void
UnloadModule(void *_mod)
{
    ModuleDescPtr mod = _mod;

    if (mod == (ModuleDescPtr) 1)
        return;

    if (mod == NULL || mod->name == NULL)
        return;

    if (mod->parent)
        LogMessageVerbSigSafe(X_INFO, 3, "UnloadSubModule: \"%s\"\n",
                              mod->name);
    else
        LogMessageVerbSigSafe(X_INFO, 3, "UnloadModule: \"%s\"\n", mod->name);

    if (mod->TearDownData != ModuleDuplicated) {
        if ((mod->TearDownProc) && (mod->TearDownData))
            mod->TearDownProc(mod->TearDownData);
        LoaderUnload(mod->name, mod->handle);
    }

    if (mod->child)
        UnloadModule(mod->child);
    if (mod->sib)
        UnloadModule(mod->sib);
    free(mod->path);
    free(mod->name);
    free(mod);
}

void
UnloadSubModule(void *_mod)
{
    ModuleDescPtr mod = (ModuleDescPtr) _mod;

    /* Some drivers are calling us on built-in submodules, ignore them */
    if (mod == (ModuleDescPtr) 1)
        return;
    RemoveChild(mod);
    UnloadModule(mod);
}

static void
RemoveChild(ModuleDescPtr child)
{
    ModuleDescPtr mdp;
    ModuleDescPtr prevsib;
    ModuleDescPtr parent;

    if (!child->parent)
        return;

    parent = child->parent;
    if (parent->child == child) {
        parent->child = child->sib;
        return;
    }

    prevsib = parent->child;
    mdp = prevsib->sib;
    while (mdp && mdp != child) {
        prevsib = mdp;
        mdp = mdp->sib;
    }
    if (mdp == child)
        prevsib->sib = child->sib;
    child->sib = NULL;
    return;
}

void
LoaderErrorMsg(const char *name, const char *modname, int errmaj, int errmin)
{
    const char *msg;
    MessageType type = X_ERROR;

    switch (errmaj) {
    case LDR_NOERROR:
        msg = "no error";
        break;
    case LDR_NOMEM:
        msg = "out of memory";
        break;
    case LDR_NOENT:
        msg = "module does not exist";
        break;
    case LDR_NOSUBENT:
        msg = "a required submodule could not be loaded";
        break;
    case LDR_NOSPACE:
        msg = "too many modules";
        break;
    case LDR_NOMODOPEN:
        msg = "open failed";
        break;
    case LDR_UNKTYPE:
        msg = "unknown module type";
        break;
    case LDR_NOLOAD:
        msg = "loader failed";
        break;
    case LDR_ONCEONLY:
        msg = "already loaded";
        type = X_INFO;
        break;
    case LDR_NOPORTOPEN:
        msg = "port open failed";
        break;
    case LDR_NOHARDWARE:
        msg = "no hardware found";
        break;
    case LDR_MISMATCH:
        msg = "module requirement mismatch";
        break;
    case LDR_BADUSAGE:
        msg = "invalid argument(s) to LoadModule()";
        break;
    case LDR_INVALID:
        msg = "invalid module";
        break;
    case LDR_BADOS:
        msg = "module doesn't support this OS";
        break;
    case LDR_MODSPECIFIC:
        msg = "module-specific error";
        break;
    default:
        msg = "unknown error";
    }
    if (name)
        xf86Msg(type, "%s: Failed to load module \"%s\" (%s, %d)\n",
                name, modname, msg, errmin);
    else
        xf86Msg(type, "Failed to load module \"%s\" (%s, %d)\n",
                modname, msg, errmin);
}

/* Given a module path or file name, return the module's canonical name */
static char *
LoaderGetCanonicalName(const char *modname, PatternPtr patterns)
{
    char *str;
    const char *s;
    int len;
    PatternPtr p;
    regmatch_t match[2];

    /* Strip off any leading path */
    s = strrchr(modname, '/');
    if (s == NULL)
        s = modname;
    else
        s++;

    /* Find the first regex that is matched */
    for (p = patterns; p->pattern; p++)
        if (regexec(&p->rex, s, 2, match, 0) == 0 && match[1].rm_so != -1) {
            len = match[1].rm_eo - match[1].rm_so;
            str = malloc(len + 1);
            if (!str)
                return NULL;
            strncpy(str, s + match[1].rm_so, len);
            str[len] = '\0';
            return str;
        }

    /* If there is no match, return the whole name minus the leading path */
    return strdup(s);
}

/*
 * Return the module version information.
 */
unsigned long
LoaderGetModuleVersion(ModuleDescPtr mod)
{
    if (!mod || mod == (ModuleDescPtr) 1 || !mod->VersionInfo)
        return 0;

    return MODULE_VERSION_NUMERIC(mod->VersionInfo->majorversion,
                                  mod->VersionInfo->minorversion,
                                  mod->VersionInfo->patchlevel);
}
