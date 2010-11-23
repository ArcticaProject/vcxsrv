/*
 * Mesa 3-D graphics library
 * Version:  7.9
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <stdlib.h>
#include <stddef.h> /* for offsetof */
#include <string.h>
#include <assert.h>

#include "u_current.h"
#include "u_thread.h"
#include "entry.h"
#include "stub.h"
#include "table.h"

#define MAPI_TABLE_FIRST_DYNAMIC \
   (offsetof(struct mapi_table, dynamic0) / sizeof(mapi_func))
#define MAPI_TABLE_NUM_DYNAMIC \
   ((offsetof(struct mapi_table, last) - \
     offsetof(struct mapi_table, dynamic0)) / sizeof(mapi_func))
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

/*
 * This will define public_string_pool, public_sorted_indices, and
 * public_stubs.
 */
#define MAPI_TMP_PUBLIC_STUBS
#include "mapi_tmp.h"

static struct mapi_stub dynamic_stubs[MAPI_TABLE_NUM_DYNAMIC];
static int num_dynamic_stubs;
static int next_dynamic_slot = MAPI_TABLE_FIRST_DYNAMIC;

void
stub_init_once(void)
{
#ifdef PTHREADS
   static pthread_once_t once = PTHREAD_ONCE_INIT;
   pthread_once(&once, entry_patch_public);
#else
   static int first = 1;
   if (first) {
      first = 0;
      entry_patch_public();
   }
#endif
}

static int
stub_compare(const void *key, const void *elem)
{
   const char *name = (const char *) key;
   const int *index = (const int *) elem;
   const struct mapi_stub *stub;
   const char *stub_name;

   stub = &public_stubs[*index];
   stub_name = &public_string_pool[(unsigned long) stub->name];

   return strcmp(name, stub_name);
}

/**
 * Return the public stub with the given name.
 */
const struct mapi_stub *
stub_find_public(const char *name)
{
   const int *index;

   index = (const int *) bsearch(name, public_sorted_indices,
         ARRAY_SIZE(public_sorted_indices) - 1,
         sizeof(public_sorted_indices[0]), stub_compare);

   return (index) ? &public_stubs[*index] : NULL;
}

/**
 * Add a dynamic stub.
 */
static struct mapi_stub *
stub_add_dynamic(const char *name)
{
   struct mapi_stub *stub;
   int idx;

   idx = num_dynamic_stubs;
   if (idx >= MAPI_TABLE_NUM_DYNAMIC)
      return NULL;

   stub = &dynamic_stubs[idx];

   /* dispatch to mapi_table->last, which is always no-op */
   stub->addr =
      entry_generate(MAPI_TABLE_FIRST_DYNAMIC + MAPI_TABLE_NUM_DYNAMIC);
   if (!stub->addr)
      return NULL;

   stub->name = (const void *) name;
   /* to be fixed later */
   stub->slot = -1;

   num_dynamic_stubs = idx + 1;

   return stub;
}

/**
 * Return the dynamic stub with the given name.  If no such stub exists and
 * generate is true, a new stub is generated.
 */
struct mapi_stub *
stub_find_dynamic(const char *name, int generate)
{
   u_mutex_declare_static(dynamic_mutex);
   struct mapi_stub *stub = NULL;
   int count, i;
   
   u_mutex_lock(dynamic_mutex);

   if (generate)
      assert(!stub_find_public(name));

   count = num_dynamic_stubs;
   for (i = 0; i < count; i++) {
      if (strcmp(name, (const char *) dynamic_stubs[i].name) == 0) {
         stub = &dynamic_stubs[i];
         break;
      }
   }

   /* generate a dynamic stub */
   if (generate && !stub)
         stub = stub_add_dynamic(name);

   u_mutex_unlock(dynamic_mutex);

   return stub;
}

void
stub_fix_dynamic(struct mapi_stub *stub, const struct mapi_stub *alias)
{
   int slot;

   if (stub->slot >= 0)
      return;

   if (alias)
      slot = alias->slot;
   else
      slot = next_dynamic_slot++;

   entry_patch(stub->addr, slot);
   stub->slot = slot;
}
