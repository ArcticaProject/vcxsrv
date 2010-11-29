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

#include <string.h>
#include "u_execmem.h"
#include "u_macros.h"

__asm__(".text");

__asm__("x86_64_current_tls:\n\t"
	"movq u_current_table_tls@GOTTPOFF(%rip), %rax\n\t"
	"ret");

#define STUB_ASM_ENTRY(func)                             \
   ".globl " func "\n"                                   \
   ".type " func ", @function\n"                         \
   ".balign 32\n"                                        \
   func ":"

#define STUB_ASM_CODE(slot)                              \
   "movq u_current_table_tls@GOTTPOFF(%rip), %rax\n\t"   \
   "movq %fs:(%rax), %r11\n\t"                           \
   "jmp *(8 * " slot ")(%r11)"

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

extern unsigned long
x86_64_current_tls();

void
entry_patch_public(void)
{
}

void
entry_patch(mapi_func entry, int slot)
{
   char *code = (char *) entry;
   *((unsigned int *) (code + 12)) = slot * sizeof(mapi_func);
}

mapi_func
entry_generate(int slot)
{
   const char code_templ[16] = {
      /* movq %fs:0, %r11 */
      0x64, 0x4c, 0x8b, 0x1c, 0x25, 0x00, 0x00, 0x00, 0x00,
      /* jmp *0x1234(%r11) */
      0x41, 0xff, 0xa3, 0x34, 0x12, 0x00, 0x00,
   };
   unsigned long addr;
   void *code;
   mapi_func entry;

   addr = x86_64_current_tls();
   if ((addr >> 32) != 0xffffffff)
      return NULL;
   addr &= 0xffffffff;

   code = u_execmem_alloc(sizeof(code_templ));
   if (!code)
      return NULL;

   memcpy(code, code_templ, sizeof(code_templ));

   *((unsigned int *) (code + 5)) = addr;
   entry = (mapi_func) code;
   entry_patch(entry, slot);

   return entry;
}
