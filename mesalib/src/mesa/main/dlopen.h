/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef DLOPEN_H
#define DLOPEN_H

/**
 * Wrapper functions for dlopen(), dlsym(), dlclose().
 * Note that the #ifdef tests for various environments should be expanded.
 */

#if defined(HAVE_DLOPEN)
#include <dlfcn.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

typedef void (*GenericFunc)(void);

/**
 * Wrapper for dlopen().
 * Note that 'flags' isn't used at this time.
 */
static inline void *
_mesa_dlopen(const char *libname, int flags)
{
#if defined(HAVE_DLOPEN)
   flags = RTLD_LAZY | RTLD_GLOBAL; /* Overriding flags at this time */
   return dlopen(libname, flags);
#elif defined(_WIN32)
   return LoadLibraryA(libname);
#else
   return NULL;
#endif
}

/**
 * Wrapper for dlsym() that does a cast to a generic function type,
 * rather than a void *.  This reduces the number of warnings that are
 * generated.
 */
static inline GenericFunc
_mesa_dlsym(void *handle, const char *fname)
{
   union {
      void *v;
      GenericFunc f;
   } u;
#if defined(HAVE_DLOPEN)
   u.v = dlsym(handle, fname);
#elif defined(_WIN32)
   u.v = (void *) GetProcAddress(handle, fname);
#else
   u.v = NULL;
#endif
   return u.f;
}

/**
 * Wrapper for dlclose().
 */
static inline void
_mesa_dlclose(void *handle)
{
#if defined(HAVE_DLOPEN)
   dlclose(handle);
#elif defined(_WIN32)
   FreeLibrary(handle);
#else
   (void) handle;
#endif
}

#endif
