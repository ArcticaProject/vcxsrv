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

/**
 * @file
 * Simple cache.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef U_CACHE_H_
#define U_CACHE_H_


#include "pipe/p_compiler.h"


#ifdef __cplusplus
extern "C" {
#endif

   
/**
 * Least Recently Used (LRU) cache.
 */
struct util_cache;


/**
 * Create a cache.
 * 
 * @param hash hash function
 * @param compare should return 0 for two equal keys
 * @param destroy destruction callback (optional)
 * @param size maximum number of entries
 */
struct util_cache *
util_cache_create(uint32_t (*hash)(const void *key),
                  int (*compare)(const void *key1, const void *key2),
                  void (*destroy)(void *key, void *value),
                  uint32_t size);

void
util_cache_set(struct util_cache *cache,
               void *key,
               void *value);

void *
util_cache_get(struct util_cache *cache, 
               const void *key);

void
util_cache_clear(struct util_cache *cache);

void
util_cache_destroy(struct util_cache *cache);

void
util_cache_remove(struct util_cache *cache,
                  const void *key);


#ifdef __cplusplus
}
#endif

#endif /* U_CACHE_H_ */
