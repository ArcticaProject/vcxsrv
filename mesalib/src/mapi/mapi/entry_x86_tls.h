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

__asm__("x86_current_tls:\n\t"
	"call 1f\n"
        "1:\n\t"
        "popl %eax\n\t"
	"addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %eax\n\t"
	"movl u_current_table_tls@GOTNTPOFF(%eax), %eax\n\t"
	"ret");

#ifndef GLX_X86_READONLY_TEXT
__asm__(".section wtext, \"awx\", @progbits\n"
        ".balign 16\n"
        "x86_entry_start:");
#endif /* GLX_X86_READONLY_TEXT */

#define STUB_ASM_ENTRY(func)     \
   ".globl " func "\n"           \
   ".type " func ", @function\n" \
   ".balign 16\n"                \
   func ":"

#define STUB_ASM_CODE(slot)      \
   "call x86_current_tls\n\t"    \
   "movl %gs:(%eax), %eax\n\t"   \
   "jmp *(4 * " slot ")(%eax)"

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

#ifndef GLX_X86_READONLY_TEXT
__asm__(".balign 16\n"
        "x86_entry_end:");
__asm__(".text");
#endif /* GLX_X86_READONLY_TEXT */

extern unsigned long
x86_current_tls();

void
entry_patch_public(void)
{
#ifndef GLX_X86_READONLY_TEXT
   extern char x86_entry_start[];
   extern char x86_entry_end[];
   char patch[8] = {
      0x65, 0xa1, 0x00, 0x00, 0x00, 0x00, /* movl %gs:0x0, %eax */
      0x90, 0x90                          /* nop's */
   };
   char *entry;

   *((unsigned long *) (patch + 2)) = x86_current_tls();

   for (entry = x86_entry_start; entry < x86_entry_end; entry += 16)
      memcpy(entry, patch, sizeof(patch));
#endif
}

void
entry_patch(mapi_func entry, int slot)
{
   char *code = (char *) entry;
   *((unsigned long *) (code + 8)) = slot * sizeof(mapi_func);
}

mapi_func
entry_generate(int slot)
{
   const char code_templ[16] = {
      0x65, 0xa1, 0x00, 0x00, 0x00, 0x00, /* movl %gs:0x0, %eax */
      0xff, 0xa0, 0x34, 0x12, 0x00, 0x00, /* jmp *0x1234(%eax) */
      0x90, 0x90, 0x90, 0x90              /* nop's */
   };
   void *code;
   mapi_func entry;

   code = u_execmem_alloc(sizeof(code_templ));
   if (!code)
      return NULL;

   memcpy(code, code_templ, sizeof(code_templ));

   *((unsigned long *) (code + 2)) = x86_current_tls();
   entry = (mapi_func) code;
   entry_patch(entry, slot);

   return entry;
}
