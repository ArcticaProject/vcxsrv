/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/


/*
 * Memory functions
 */


#ifndef U_MEMORY_H
#define U_MEMORY_H


#include "util/u_pointer.h"
#include "util/u_debug.h"
#include "os/os_memory.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MALLOC(_size)  os_malloc(_size)

#define CALLOC(_count, _size) os_calloc(_count, _size)

#define FREE(_ptr ) os_free(_ptr)

#define REALLOC(_ptr, _old_size, _size) os_realloc(_ptr, _old_size, _size)

#define MALLOC_STRUCT(T)   (struct T *) MALLOC(sizeof(struct T))

#define CALLOC_STRUCT(T)   (struct T *) CALLOC(1, sizeof(struct T))

#define CALLOC_VARIANT_LENGTH_STRUCT(T,more_size)   ((struct T *) CALLOC(1, sizeof(struct T) + more_size))


#define align_malloc(_size, _alignment) os_malloc_aligned(_size, _alignment)
#define align_free(_ptr) os_free_aligned(_ptr)


/**
 * Duplicate a block of memory.
 */
static INLINE void *
mem_dup(const void *src, uint size)
{
   void *dup = MALLOC(size);
   if (dup)
      memcpy(dup, src, size);
   return dup;
}


/**
 * Number of elements in an array.
 */
#ifndef Elements
#define Elements(x) (sizeof(x)/sizeof((x)[0]))
#endif


/**
 * Offset of a field in a struct, in bytes.
 */
#define Offset(TYPE, MEMBER) ((uintptr_t)&(((TYPE *)NULL)->MEMBER))



#ifdef __cplusplus
}
#endif


#endif /* U_MEMORY_H */
