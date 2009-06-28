/*
 * Copyright 2002 Red Hat Inc., Durham, North Carolina.
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

/** \file
 * Interface for DMX configuration file support.  \see dmxconfig.c */

#ifndef _DMXCONFIG_H_
#define _DMXCONFIG_H_
#define DMX_DEFAULT_XKB_RULES  "xfree86"
#define DMX_DEFAULT_XKB_MODEL  "pc101"
#define DMX_DEFAULT_XKB_LAYOUT "us"
#define DMX_DEFAULT_XKB_VARIANT NULL
#define DMX_DEFAULT_XKB_OPTIONS NULL

extern void dmxConfigStoreDisplay(const char *display);
extern void dmxConfigStoreInput(const char *input); /* Core devices */
extern void dmxConfigStoreXInput(const char *input); /* Non-core devices */
extern void dmxConfigStoreFile(const char *file);
extern void dmxConfigStoreConfig(const char *config);
extern void dmxConfigConfigure(void);
extern void dmxConfigSetMaxScreens(void);

extern void dmxConfigSetXkbRules(const char *rules);
extern void dmxConfigSetXkbModel(const char *model);
extern void dmxConfigSetXkbLayout(const char *layout);
extern void dmxConfigSetXkbVariant(const char *variant);
extern void dmxConfigSetXkbOptions(const char *options);

extern char *dmxConfigGetXkbRules(void);
extern char *dmxConfigGetXkbModel(void);
extern char *dmxConfigGetXkbLayout(void);
extern char *dmxConfigGetXkbVariant(void);
extern char *dmxConfigGetXkbOptions(void);
#endif
