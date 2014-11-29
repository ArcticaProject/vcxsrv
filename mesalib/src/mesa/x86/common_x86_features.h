
/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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

/*
 * x86 CPUID feature information.  The raw data is returned by
 * _mesa_identify_x86_cpu_features() and interpreted with the cpu_has_*
 * helper macros.
 *
 * Gareth Hughes
 */

#ifndef __COMMON_X86_FEATURES_H__
#define __COMMON_X86_FEATURES_H__

#define X86_FEATURE_FPU		(1<<0)
#define X86_FEATURE_CMOV	(1<<1)
#define X86_FEATURE_MMXEXT	(1<<2)
#define X86_FEATURE_MMX		(1<<3)
#define X86_FEATURE_FXSR	(1<<4)
#define X86_FEATURE_XMM		(1<<5)
#define X86_FEATURE_XMM2	(1<<6)
#define X86_FEATURE_3DNOWEXT	(1<<7)
#define X86_FEATURE_3DNOW	(1<<8)
#define X86_FEATURE_SSE4_1	(1<<9)

/* standard X86 CPU features */
#define X86_CPU_FPU		(1<<0)
#define X86_CPU_CMOV		(1<<15)
#define X86_CPU_MMX		(1<<23)
#define X86_CPU_XMM		(1<<25)
#define X86_CPU_XMM2		(1<<26)
/* ECX. */
#define X86_CPU_SSE4_1		(1<<19)

/* extended X86 CPU features */
#define X86_CPUEXT_MMX_EXT	(1<<22)
#define X86_CPUEXT_3DNOW_EXT	(1<<30)
#define X86_CPUEXT_3DNOW	(1<<31)

#ifdef __MMX__
#define cpu_has_mmx		1
#else
#define cpu_has_mmx		(_mesa_x86_cpu_features & X86_FEATURE_MMX)
#endif

#define cpu_has_mmxext		(_mesa_x86_cpu_features & X86_FEATURE_MMXEXT)

#ifdef __SSE__
#define cpu_has_xmm		1
#else
#define cpu_has_xmm		(_mesa_x86_cpu_features & X86_FEATURE_XMM)
#endif

#ifdef __SSE2__
#define cpu_has_xmm2		1
#else
#define cpu_has_xmm2		(_mesa_x86_cpu_features & X86_FEATURE_XMM2)
#endif

#ifdef __3dNOW__
#define cpu_has_3dnow		1
#else
#define cpu_has_3dnow		(_mesa_x86_cpu_features & X86_FEATURE_3DNOW)
#endif

#define cpu_has_3dnowext	(_mesa_x86_cpu_features & X86_FEATURE_3DNOWEXT)

#ifdef __SSE4_1__
#define cpu_has_sse4_1		1
#else
#define cpu_has_sse4_1		(_mesa_x86_cpu_features & X86_FEATURE_SSE4_1)
#endif

#endif

