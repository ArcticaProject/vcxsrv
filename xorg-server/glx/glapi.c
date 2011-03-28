/*
 * Mesa 3-D graphics library
 * Version:  7.1
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*
 * This file manages the OpenGL API dispatch layer.
 * The dispatch table (struct _glapi_table) is basically just a list
 * of function pointers.
 * There are functions to set/get the current dispatch table for the
 * current thread and to manage registration/dispatch of dynamically
 * added extension functions.
 *
 * It's intended that this file and the other glapi*.[ch] files are
 * flexible enough to be reused in several places:  XFree86, DRI-
 * based libGL.so, and perhaps the SGI SI.
 *
 * NOTE: There are no dependencies on Mesa in this code.
 *
 * Versions (API changes):
 *   2000/02/23  - original version for Mesa 3.3 and XFree86 4.0
 *   2001/01/16  - added dispatch override feature for Mesa 3.5
 *   2002/06/28  - added _glapi_set_warning_func(), Mesa 4.1.
 *   2002/10/01  - _glapi_get_proc_address() will now generate new entrypoints
 *                 itself (using offset ~0).  _glapi_add_entrypoint() can be
 *                 called afterward and it'll fill in the correct dispatch
 *                 offset.  This allows DRI libGL to avoid probing for DRI
 *                 drivers!  No changes to the public glapi interface.
 */



#ifdef HAVE_DIX_CONFIG_H

#include <dix-config.h>
#include <X11/Xfuncproto.h>
#ifdef _MSC_VER
#define PUBLIC _declspec(dllexport)
#else
#define PUBLIC _X_EXPORT
#endif

#else

#include "glheader.h"

#endif

#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <assert.h>
#endif
#include <unistd.h>

#include "glapi.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "glapitable.h"

#define FIRST_DYNAMIC_OFFSET (sizeof(struct _glapi_table) / sizeof(void *))

/***** BEGIN NO-OP DISPATCH *****/

static GLboolean WarnFlag = GL_FALSE;
static _glapi_proc warning_func;

#if defined(PTHREADS) || defined(GLX_USE_TLS)
static void init_glapi_relocs(void);
#endif

static _glapi_proc generate_entrypoint(GLuint functionOffset);
static void fill_in_entrypoint_offset(_glapi_proc entrypoint, GLuint offset);
static void init_glapi_relocs_once( void );

void _GLAPI_EXPORT
_glapi_noop_enable_warnings(unsigned char enable)
{
}

void _GLAPI_EXPORT
_glapi_set_warning_func(_glapi_proc func)
{
}

#ifdef DEBUG

/**
 * Called by each of the no-op GL entrypoints.
 */
static int
Warn(const char *func)
{
#if !defined(_WIN32_WCE)
   if (getenv("MESA_DEBUG") || getenv("LIBGL_DEBUG")) {
      fprintf(stderr, "GL User Error: gl%s called without a rendering context\n",
              func);
   }
#endif
   return 0;
}


/**
 * This is called if the user somehow calls an unassigned GL dispatch function.
 */
static GLint
NoOpUnused(void)
{
   return Warn(" function");
}

/*
 * Defines for the glapitemp.h functions.
 */
#define KEYWORD1 static
#define KEYWORD1_ALT static
#define KEYWORD2 GLAPIENTRY
#define NAME(func)  NoOp##func
#define DISPATCH(func, args, msg)  Warn(#func);
#define RETURN_DISPATCH(func, args, msg)  Warn(#func); return 0


/*
 * Defines for the table of no-op entry points.
 */
#define TABLE_ENTRY(name) (_glapi_proc) NoOp##name

#else

static int
NoOpGeneric(void)
{
#if !defined(_WIN32_WCE)
   if (getenv("MESA_DEBUG") || getenv("LIBGL_DEBUG")) {
      fprintf(stderr, "GL User Error: calling GL function without a rendering context\n");
   }
#endif
   return 0;
}

#define TABLE_ENTRY(name) (_glapi_proc) NoOpGeneric

#endif

#define DISPATCH_TABLE_NAME __glapi_noop_table
#define UNUSED_TABLE_NAME __unused_noop_functions

#include "glapitemp.h"

/***** END NO-OP DISPATCH *****/



/**
 * \name Current dispatch and current context control variables
 *
 * Depending on whether or not multithreading is support, and the type of
 * support available, several variables are used to store the current context
 * pointer and the current dispatch table pointer.  In the non-threaded case,
 * the variables \c _glapi_Dispatch and \c _glapi_Context are used for this
 * purpose.
 *
 * In the "normal" threaded case, the variables \c _glapi_Dispatch and
 * \c _glapi_Context will be \c NULL if an application is detected as being
 * multithreaded.  Single-threaded applications will use \c _glapi_Dispatch
 * and \c _glapi_Context just like the case without any threading support.
 * When \c _glapi_Dispatch and \c _glapi_Context are \c NULL, the thread state
 * data \c _gl_DispatchTSD and \c ContextTSD are used.  Drivers and the
 * static dispatch functions access these variables via \c _glapi_get_dispatch
 * and \c _glapi_get_context.
 *
 * There is a race condition in setting \c _glapi_Dispatch to \c NULL.  It is
 * possible for the original thread to be setting it at the same instant a new
 * thread, perhaps running on a different processor, is clearing it.  Because
 * of that, \c ThreadSafe, which can only ever be changed to \c GL_TRUE, is
 * used to determine whether or not the application is multithreaded.
 * 
 * In the TLS case, the variables \c _glapi_Dispatch and \c _glapi_Context are
 * hardcoded to \c NULL.  Instead the TLS variables \c _glapi_tls_Dispatch and
 * \c _glapi_tls_Context are used.  Having \c _glapi_Dispatch and
 * \c _glapi_Context be hardcoded to \c NULL maintains binary compatability
 * between TLS enabled loaders and non-TLS DRI drivers.
 */
/*@{*/
#if defined(GLX_USE_TLS)

PUBLIC TLS struct _glapi_table * _glapi_tls_Dispatch
    __attribute__((tls_model("initial-exec")))
    = (struct _glapi_table *) __glapi_noop_table;

PUBLIC TLS void * _glapi_tls_Context
    __attribute__((tls_model("initial-exec")));

PUBLIC const struct _glapi_table *_glapi_Dispatch = NULL;

PUBLIC const void *_glapi_Context = NULL;

#else

#if defined(THREADS)

_glthread_TSD _gl_DispatchTSD;           /**< Per-thread dispatch pointer */

static _glthread_TSD ContextTSD;         /**< Per-thread context pointer */

#endif /* defined(THREADS) */

PUBLIC struct _glapi_table *_glapi_Dispatch = (struct _glapi_table *) __glapi_noop_table;

PUBLIC void *_glapi_Context = NULL;

#endif /* defined(GLX_USE_TLS) */
/*@}*/



#if defined(THREADS) && !defined(GLX_USE_TLS)

void
_glapi_init_multithread(void)
{
   _glthread_InitTSD(&_gl_DispatchTSD);
   _glthread_InitTSD(&ContextTSD);
}

void
_glapi_destroy_multithread(void)
{
#ifdef WIN32_THREADS
   _glthread_DestroyTSD(&_gl_DispatchTSD);
   _glthread_DestroyTSD(&ContextTSD);
#endif
}

/**
 * Mutex for multithread check.
 */
#ifdef WIN32_THREADS
/* _glthread_DECLARE_STATIC_MUTEX is broken on windows.  There will be race! */
#define CHECK_MULTITHREAD_LOCK()
#define CHECK_MULTITHREAD_UNLOCK()
#else
_glthread_DECLARE_STATIC_MUTEX(ThreadCheckMutex);
#define CHECK_MULTITHREAD_LOCK() _glthread_LOCK_MUTEX(ThreadCheckMutex)
#define CHECK_MULTITHREAD_UNLOCK() _glthread_UNLOCK_MUTEX(ThreadCheckMutex)
#endif
/**
 * xserver's gl is not multithreaded, we promise.
 */
PUBLIC void
_glapi_check_multithread(void)
{
}
#else

void
_glapi_init_multithread(void) { }

void
_glapi_destroy_multithread(void) { }

PUBLIC void
_glapi_check_multithread(void) { }

#endif



/**
 * Set the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast to
 * void from the real context pointer type.
 */
void
_glapi_set_context(void *context)
{
#if defined(GLX_USE_TLS)
   _glapi_tls_Context = context;
#elif defined(THREADS)
   _glthread_SetTSD(&ContextTSD, context);
   _glapi_Context = context;
#else
   _glapi_Context = context;
#endif
}



/**
 * Get the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast from
 * void to the real context pointer type.
 */
void *
_glapi_get_context(void)
{
#if defined(GLX_USE_TLS)
   return _glapi_tls_Context;
#else
   return _glapi_Context;
#endif
}



/**
 * Set the global or per-thread dispatch table pointer.
 * If the dispatch parameter is NULL we'll plug in the no-op dispatch
 * table (__glapi_noop_table).
 */
void
_glapi_set_dispatch(struct _glapi_table *dispatch)
{
   init_glapi_relocs_once();

   if (dispatch == NULL) {
      /* use the no-op functions */
      dispatch = (struct _glapi_table *) __glapi_noop_table;
   }

#if defined(GLX_USE_TLS)
   _glapi_tls_Dispatch = dispatch;
#else
   _glapi_Dispatch = dispatch;
#endif
}



/**
 * Return pointer to current dispatch table for calling thread.
 */
struct _glapi_table *
_glapi_get_dispatch(void)
{
#if defined(GLX_USE_TLS)
   return _glapi_tls_Dispatch;
#else
   return _glapi_Dispatch;
#endif
}



/***
 *** The rest of this file is pretty much concerned with GetProcAddress
 *** functionality.
 ***/

#if defined(USE_X86_ASM)
# if defined(GLX_USE_TLS)
#  define DISPATCH_FUNCTION_SIZE  16
# elif defined(THREADS)
#  define DISPATCH_FUNCTION_SIZE  32
# else
#  define DISPATCH_FUNCTION_SIZE  16
# endif
#endif

#if defined(USE_X64_64_ASM)
# if defined(GLX_USE_TLS)
#  define DISPATCH_FUNCTION_SIZE  16
# endif
#endif

#if !defined(DISPATCH_FUNCTION_SIZE) && !defined(XFree86Server) && !defined(XGLServer)
# define NEED_FUNCTION_POINTER
#endif

/* The code in this file is auto-generated with Python */
#include "glprocs.h"


/**
 * Search the table of static entrypoint functions for the named function
 * and return the corresponding glprocs_table_t entry.
 */
static const glprocs_table_t *
get_static_proc( const char * n )
{
   GLuint i;
   for (i = 0; static_functions[i].Name_offset >= 0; i++) {
      const char *testName = gl_string_table + static_functions[i].Name_offset;
#ifdef MANGLE
      /* skip the prefix on the name */
      if (strcmp(testName, n + 1) == 0)
#else
      if (strcmp(testName, n) == 0)
#endif
      {
	 return &static_functions[i];
      }
   }
   return NULL;
}


/**
 * Return dispatch table offset of the named static (built-in) function.
 * Return -1 if function not found.
 */
static GLint
get_static_proc_offset(const char *funcName)
{
   const glprocs_table_t * const f = get_static_proc( funcName );
   if (f == NULL) {
      return -1;
   }

   return f->Offset;
}


/**********************************************************************
 * Extension function management.
 */

/**
 * Number of extension functions which we can dynamically add at runtime.
 *
 * Number of extension functions is also subject to the size of backing exec
 * mem we allocate. For the common case of dispatch stubs with size 16 bytes,
 * the two limits will be hit simultaneously. For larger dispatch function
 * sizes, MAX_EXTENSION_FUNCS is effectively reduced.
 */
#define MAX_EXTENSION_FUNCS 256


/**
 * Track information about a function added to the GL API.
 */
struct _glapi_function {
   /**
    * Name of the function.
    */
   const char * name;


   /**
    * Text string that describes the types of the parameters passed to the
    * named function.   Parameter types are converted to characters using the
    * following rules:
    *   - 'i' for \c GLint, \c GLuint, and \c GLenum
    *   - 'p' for any pointer type
    *   - 'f' for \c GLfloat and \c GLclampf
    *   - 'd' for \c GLdouble and \c GLclampd
    */
   const char * parameter_signature;


   /**
    * Offset in the dispatch table where the pointer to the real function is
    * located.  If the driver has not requested that the named function be
    * added to the dispatch table, this will have the value ~0.
    */
   unsigned dispatch_offset;


   /**
    * Pointer to the dispatch stub for the named function.
    * 
    * \todo
    * The semantic of this field should be changed slightly.  Currently, it
    * is always expected to be non-\c NULL.  However, it would be better to
    * only allocate the entry-point stub when the application requests the
    * function via \c glXGetProcAddress.  This would save memory for all the
    * functions that the driver exports but that the application never wants
    * to call.
    */
   _glapi_proc dispatch_stub;
};


static struct _glapi_function ExtEntryTable[MAX_EXTENSION_FUNCS];
static GLuint NumExtEntryPoints = 0;


static struct _glapi_function *
get_extension_proc(const char *funcName)
{
   GLuint i;
   for (i = 0; i < NumExtEntryPoints; i++) {
      if (strcmp(ExtEntryTable[i].name, funcName) == 0) {
         return & ExtEntryTable[i];
      }
   }
   return NULL;
}


static GLint
get_extension_proc_offset(const char *funcName)
{
   const struct _glapi_function * const f = get_extension_proc( funcName );
   if (f == NULL) {
      return -1;
   }

   return f->dispatch_offset;
}


static _glapi_proc
get_extension_proc_address(const char *funcName)
{
   const struct _glapi_function * const f = get_extension_proc( funcName );
   if (f == NULL) {
      return NULL;
   }

   return f->dispatch_stub;
}


static const char *
get_extension_proc_name(GLuint offset)
{
   GLuint i;
   for (i = 0; i < NumExtEntryPoints; i++) {
      if (ExtEntryTable[i].dispatch_offset == offset) {
         return ExtEntryTable[i].name;
      }
   }
   return NULL;
}


/**
 * strdup() is actually not a standard ANSI C or POSIX routine.
 * Irix will not define it if ANSI mode is in effect.
 */
static char *
str_dup(const char *str)
{
   char *copy;
   copy = (char*) malloc(strlen(str) + 1);
   if (!copy)
      return NULL;
   strcpy(copy, str);
   return copy;
}


/**
 * Generate new entrypoint
 *
 * Use a temporary dispatch offset of ~0 (i.e. -1).  Later, when the driver
 * calls \c _glapi_add_dispatch we'll put in the proper offset.  If that
 * never happens, and the user calls this function, he'll segfault.  That's
 * what you get when you try calling a GL function that doesn't really exist.
 * 
 * \param funcName  Name of the function to create an entry-point for.
 * 
 * \sa _glapi_add_entrypoint
 */

static struct _glapi_function *
add_function_name( const char * funcName )
{
   struct _glapi_function * entry = NULL;
   _glapi_proc entrypoint = NULL;
   char * name_dup = NULL;

   if (NumExtEntryPoints >= MAX_EXTENSION_FUNCS)
      return NULL;

   if (funcName == NULL)
      return NULL;

   name_dup = str_dup(funcName);
   if (name_dup == NULL)
      return NULL;

   entrypoint = generate_entrypoint(~0);

   if (entrypoint == NULL) {
      free(name_dup);
      return NULL;
   }

   entry = & ExtEntryTable[NumExtEntryPoints];
   NumExtEntryPoints++;

   entry->name = name_dup;
   entry->parameter_signature = NULL;
   entry->dispatch_offset = ~0;
   entry->dispatch_stub = entrypoint;

   return entry;
}


static struct _glapi_function *
set_entry_info( struct _glapi_function * entry, const char * signature, unsigned offset )
{
   char * sig_dup = NULL;

   if (signature == NULL)
      return NULL;

   sig_dup = str_dup(signature);
   if (sig_dup == NULL)
      return NULL;

   fill_in_entrypoint_offset(entry->dispatch_stub, offset);

   entry->parameter_signature = sig_dup;
   entry->dispatch_offset = offset;

   return entry;
}

#if defined(USE_X86_ASM)

/**
 * Perform platform-specific GL API entry-point fixups.
 */
static void
init_glapi_relocs( void )
{
#if defined(GLX_USE_TLS) && !defined(GLX_X86_READONLY_TEXT)
    extern unsigned long _x86_get_dispatch(void);
    char run_time_patch[] = {
       0x65, 0xa1, 0, 0, 0, 0 /* movl %gs:0,%eax */
    };
    GLuint *offset = (GLuint *) &run_time_patch[2]; /* 32-bits for x86/32 */
    const GLubyte * const get_disp = (const GLubyte *) run_time_patch;
    GLubyte * curr_func = (GLubyte *) gl_dispatch_functions_start;

    *offset = _x86_get_dispatch();
    while ( curr_func != (GLubyte *) gl_dispatch_functions_end ) {
	(void) memcpy( curr_func, get_disp, sizeof(run_time_patch));
	curr_func += DISPATCH_FUNCTION_SIZE;
    }
#endif
}


/**
 * Generate a dispatch function (entrypoint) which jumps through
 * the given slot number (offset) in the current dispatch table.
 * We need assembly language in order to accomplish this.
 */
_glapi_proc
generate_entrypoint(unsigned int functionOffset)
{
   /* 32 is chosen as something of a magic offset.  For x86, the dispatch
    * at offset 32 is the first one where the offset in the
    * "jmp OFFSET*4(%eax)" can't be encoded in a single byte.
    */
   const GLubyte * const template_func = gl_dispatch_functions_start 
     + (DISPATCH_FUNCTION_SIZE * 32);
   GLubyte * const code = (GLubyte *) u_execmem_alloc(DISPATCH_FUNCTION_SIZE);


   if ( code != NULL ) {
      (void) memcpy(code, template_func, DISPATCH_FUNCTION_SIZE);
      fill_in_entrypoint_offset( (_glapi_proc) code, functionOffset );
   }

   return (_glapi_proc) code;
}


/**
 * This function inserts a new dispatch offset into the assembly language
 * stub that was generated with the preceeding function.
 */
void
fill_in_entrypoint_offset(_glapi_proc entrypoint, unsigned int offset)
{
   GLubyte * const code = (GLubyte *) entrypoint;

#if defined(GLX_USE_TLS)
   *((unsigned int *)(code +  8)) = 4 * offset;
#elif defined(THREADS)
   *((unsigned int *)(code + 11)) = 4 * offset;
   *((unsigned int *)(code + 22)) = 4 * offset;
#else
   *((unsigned int *)(code +  7)) = 4 * offset;
#endif
}


#elif defined(USE_SPARC_ASM)

extern void __glapi_sparc_icache_flush(unsigned int *);

static void
init_glapi_relocs( void )
{
#if defined(PTHREADS) || defined(GLX_USE_TLS)
    static const unsigned int template[] = {
#ifdef GLX_USE_TLS
	0x05000000, /* sethi %hi(_glapi_tls_Dispatch), %g2 */
	0x8730e00a, /* srl %g3, 10, %g3 */
	0x8410a000, /* or %g2, %lo(_glapi_tls_Dispatch), %g2 */
#ifdef __arch64__
	0xc259c002, /* ldx [%g7 + %g2], %g1 */
	0xc2584003, /* ldx [%g1 + %g3], %g1 */
#else
	0xc201c002, /* ld [%g7 + %g2], %g1 */
	0xc2004003, /* ld [%g1 + %g3], %g1 */
#endif
	0x81c04000, /* jmp %g1 */
	0x01000000, /* nop  */
#else
#ifdef __arch64__
	0x03000000, /* 64-bit 0x00 --> sethi %hh(_glapi_Dispatch), %g1 */
	0x05000000, /* 64-bit 0x04 --> sethi %lm(_glapi_Dispatch), %g2 */
	0x82106000, /* 64-bit 0x08 --> or %g1, %hm(_glapi_Dispatch), %g1 */
	0x8730e00a, /* 64-bit 0x0c --> srl %g3, 10, %g3 */
	0x83287020, /* 64-bit 0x10 --> sllx %g1, 32, %g1 */
	0x82004002, /* 64-bit 0x14 --> add %g1, %g2, %g1 */
	0xc2586000, /* 64-bit 0x18 --> ldx [%g1 + %lo(_glapi_Dispatch)], %g1 */
#else
	0x03000000, /* 32-bit 0x00 --> sethi %hi(_glapi_Dispatch), %g1 */
	0x8730e00a, /* 32-bit 0x04 --> srl %g3, 10, %g3 */
	0xc2006000, /* 32-bit 0x08 --> ld [%g1 + %lo(_glapi_Dispatch)], %g1 */
#endif
	0x80a06000, /*             --> cmp %g1, 0 */
	0x02800005, /*             --> be +4*5 */
	0x01000000, /*             -->  nop  */
#ifdef __arch64__
	0xc2584003, /* 64-bit      --> ldx [%g1 + %g3], %g1 */
#else
	0xc2004003, /* 32-bit      --> ld [%g1 + %g3], %g1 */
#endif
	0x81c04000, /*             --> jmp %g1 */
	0x01000000, /*             --> nop  */
#ifdef __arch64__
	0x9de3bf80, /* 64-bit      --> save  %sp, -128, %sp */
#else
	0x9de3bfc0, /* 32-bit      --> save  %sp, -64, %sp */
#endif
	0xa0100003, /*             --> mov  %g3, %l0 */
	0x40000000, /*             --> call _glapi_get_dispatch */
	0x01000000, /*             -->  nop */
	0x82100008, /*             --> mov %o0, %g1 */
	0x86100010, /*             --> mov %l0, %g3 */
	0x10bffff7, /*             --> ba -4*9 */
	0x81e80000, /*             -->  restore  */
#endif
    };
#ifdef GLX_USE_TLS
    extern unsigned int __glapi_sparc_tls_stub;
    extern unsigned long __glapi_sparc_get_dispatch(void);
    unsigned int *code = &__glapi_sparc_tls_stub;
    unsigned long dispatch = __glapi_sparc_get_dispatch();
#else
    extern unsigned int __glapi_sparc_pthread_stub;
    unsigned int *code = &__glapi_sparc_pthread_stub;
    unsigned long dispatch = (unsigned long) &_glapi_Dispatch;
    unsigned long call_dest = (unsigned long ) &_glapi_get_dispatch;
    int idx;
#endif

#ifdef GLX_USE_TLS
    code[0] = template[0] | (dispatch >> 10);
    code[1] = template[1];
    __glapi_sparc_icache_flush(&code[0]);
    code[2] = template[2] | (dispatch & 0x3ff);
    code[3] = template[3];
    __glapi_sparc_icache_flush(&code[2]);
    code[4] = template[4];
    code[5] = template[5];
    __glapi_sparc_icache_flush(&code[4]);
    code[6] = template[6];
    __glapi_sparc_icache_flush(&code[6]);
#else
#if defined(__arch64__)
    code[0] = template[0] | (dispatch >> (32 + 10));
    code[1] = template[1] | ((dispatch & 0xffffffff) >> 10);
    __glapi_sparc_icache_flush(&code[0]);
    code[2] = template[2] | ((dispatch >> 32) & 0x3ff);
    code[3] = template[3];
    __glapi_sparc_icache_flush(&code[2]);
    code[4] = template[4];
    code[5] = template[5];
    __glapi_sparc_icache_flush(&code[4]);
    code[6] = template[6] | (dispatch & 0x3ff);
    idx = 7;
#else
    code[0] = template[0] | (dispatch >> 10);
    code[1] = template[1];
    __glapi_sparc_icache_flush(&code[0]);
    code[2] = template[2] | (dispatch & 0x3ff);
    idx = 3;
#endif
    code[idx + 0] = template[idx + 0];
    __glapi_sparc_icache_flush(&code[idx - 1]);
    code[idx + 1] = template[idx + 1];
    code[idx + 2] = template[idx + 2];
    __glapi_sparc_icache_flush(&code[idx + 1]);
    code[idx + 3] = template[idx + 3];
    code[idx + 4] = template[idx + 4];
    __glapi_sparc_icache_flush(&code[idx + 3]);
    code[idx + 5] = template[idx + 5];
    code[idx + 6] = template[idx + 6];
    __glapi_sparc_icache_flush(&code[idx + 5]);
    code[idx + 7] = template[idx + 7];
    code[idx + 8] = template[idx + 8] |
	    (((call_dest - ((unsigned long) &code[idx + 8]))
	      >> 2) & 0x3fffffff);
    __glapi_sparc_icache_flush(&code[idx + 7]);
    code[idx + 9] = template[idx + 9];
    code[idx + 10] = template[idx + 10];
    __glapi_sparc_icache_flush(&code[idx + 9]);
    code[idx + 11] = template[idx + 11];
    code[idx + 12] = template[idx + 12];
    __glapi_sparc_icache_flush(&code[idx + 11]);
    code[idx + 13] = template[idx + 13];
    __glapi_sparc_icache_flush(&code[idx + 13]);
#endif
#endif
}


_glapi_proc
generate_entrypoint(GLuint functionOffset)
{
#if defined(PTHREADS) || defined(GLX_USE_TLS)
   static const unsigned int template[] = {
      0x07000000, /* sethi %hi(0), %g3 */
      0x8210000f, /* mov  %o7, %g1 */
      0x40000000, /* call */
      0x9e100001, /* mov  %g1, %o7 */
   };
#ifdef GLX_USE_TLS
   extern unsigned int __glapi_sparc_tls_stub;
   unsigned long call_dest = (unsigned long ) &__glapi_sparc_tls_stub;
#else
   extern unsigned int __glapi_sparc_pthread_stub;
   unsigned long call_dest = (unsigned long ) &__glapi_sparc_pthread_stub;
#endif
   unsigned int *code = (unsigned int *) u_execmem_alloc(sizeof(template));
   if (code) {
      code[0] = template[0] | (functionOffset & 0x3fffff);
      code[1] = template[1];
      __glapi_sparc_icache_flush(&code[0]);
      code[2] = template[2] |
         (((call_dest - ((unsigned long) &code[2]))
	   >> 2) & 0x3fffffff);
      code[3] = template[3];
      __glapi_sparc_icache_flush(&code[2]);
   }
   return (_glapi_proc) code;
#endif
}


void
fill_in_entrypoint_offset(_glapi_proc entrypoint, GLuint offset)
{
   unsigned int *code = (unsigned int *) entrypoint;

   code[0] &= ~0x3fffff;
   code[0] |= (offset * sizeof(void *)) & 0x3fffff;
   __glapi_sparc_icache_flush(&code[0]);
}


#else /* USE_*_ASM */

static void
init_glapi_relocs( void )
{
}


_glapi_proc
generate_entrypoint(GLuint functionOffset)
{
   (void) functionOffset;
   return NULL;
}


void
fill_in_entrypoint_offset(_glapi_proc entrypoint, GLuint offset)
{
   /* an unimplemented architecture */
   (void) entrypoint;
   (void) offset;
}

#endif /* USE_*_ASM */


void
init_glapi_relocs_once( void )
{
#if defined(PTHREADS) || defined(GLX_USE_TLS)
   static pthread_once_t once_control = PTHREAD_ONCE_INIT;
   pthread_once( & once_control, init_glapi_relocs );
#endif
}

/**
 * Fill-in the dispatch stub for the named function.
 * 
 * This function is intended to be called by a hardware driver.  When called,
 * a dispatch stub may be created created for the function.  A pointer to this
 * dispatch function will be returned by glXGetProcAddress.
 *
 * \param function_names       Array of pointers to function names that should
 *                             share a common dispatch offset.
 * \param parameter_signature  String representing the types of the parameters
 *                             passed to the named function.  Parameter types
 *                             are converted to characters using the following
 *                             rules:
 *                               - 'i' for \c GLint, \c GLuint, and \c GLenum
 *                               - 'p' for any pointer type
 *                               - 'f' for \c GLfloat and \c GLclampf
 *                               - 'd' for \c GLdouble and \c GLclampd
 *
 * \returns
 * The offset in the dispatch table of the named function.  A pointer to the
 * driver's implementation of the named function should be stored at
 * \c dispatch_table[\c offset].  Return -1 if error/problem.
 *
 * \sa glXGetProcAddress
 *
 * \warning
 * This function can only handle up to 8 names at a time.  As far as I know,
 * the maximum number of names ever associated with an existing GL function is
 * 4 (\c glPointParameterfSGIS, \c glPointParameterfEXT,
 * \c glPointParameterfARB, and \c glPointParameterf), so this should not be
 * too painful of a limitation.
 *
 * \todo
 * Determine whether or not \c parameter_signature should be allowed to be
 * \c NULL.  It doesn't seem like much of a hardship for drivers to have to
 * pass in an empty string.
 *
 * \todo
 * Determine if code should be added to reject function names that start with
 * 'glX'.
 * 
 * \bug
 * Add code to compare \c parameter_signature with the parameter signature of
 * a static function.  In order to do that, we need to find a way to \b get
 * the parameter signature of a static function.
 */

PUBLIC
int
_glapi_add_dispatch( const char * const * function_names,
		     const char * parameter_signature )
{
   static int next_dynamic_offset = FIRST_DYNAMIC_OFFSET;
   const char * const real_sig = (parameter_signature != NULL)
     ? parameter_signature : "";
   struct _glapi_function * entry[8];
   GLboolean is_static[8];
   unsigned i;
   int offset = ~0;

   init_glapi_relocs_once();

   (void) memset( is_static, 0, sizeof( is_static ) );
   (void) memset( entry, 0, sizeof( entry ) );

   /* Find the _single_ dispatch offset for all function names that already
    * exist (and have a dispatch offset).
    */

   for ( i = 0 ; function_names[i] != NULL ; i++ ) {
      const char * funcName = function_names[i];
      int static_offset;
      int extension_offset;

      if (funcName[0] != 'g' || funcName[1] != 'l')
         return -1;

      /* search built-in functions */
      static_offset = get_static_proc_offset(funcName);

      if (static_offset >= 0) {

	 is_static[i] = GL_TRUE;

	 /* FIXME: Make sure the parameter signatures match!  How do we get
	  * FIXME: the parameter signature for static functions?
	  */

	 if ( (offset != ~0) && (static_offset != offset) ) {
	    return -1;
	 }

	 offset = static_offset;

	 continue;
      }

      /* search added extension functions */
      entry[i] = get_extension_proc(funcName);

      if (entry[i] != NULL) {
	 extension_offset = entry[i]->dispatch_offset;

	 /* The offset may be ~0 if the function name was added by
	  * glXGetProcAddress but never filled in by the driver.
	  */

	 if (extension_offset == ~0) {
	    continue;
	 }

	 if (strcmp(real_sig, entry[i]->parameter_signature) != 0) {
	    return -1;
	 }

	 if ( (offset != ~0) && (extension_offset != offset) ) {
	    return -1;
	 }

	 offset = extension_offset;
      }
   }

   /* If all function names are either new (or with no dispatch offset),
    * allocate a new dispatch offset.
    */

   if (offset == ~0) {
      offset = next_dynamic_offset;
      next_dynamic_offset++;
   }

   /* Fill in the dispatch offset for the new function names (and those with
    * no dispatch offset).
    */

   for ( i = 0 ; function_names[i] != NULL ; i++ ) {
      if (is_static[i]) {
	 continue;
      }

      /* generate entrypoints for new function names */
      if (entry[i] == NULL) {
	 entry[i] = add_function_name( function_names[i] );
	 if (entry[i] == NULL) {
	    /* FIXME: Possible memory leak here. */
	    return -1;
	 }
      }

      if (entry[i]->dispatch_offset == ~0) {
	 set_entry_info( entry[i], real_sig, offset );
      }
   }

   return offset;
}

/**
 * Return size of dispatch table struct as number of functions (or
 * slots).
 */
GLuint
_glapi_get_dispatch_table_size(void)
{
   /*
    * The dispatch table size (number of entries) is the size of the
    * _glapi_table struct plus the number of dynamic entries we can add.
    * The extra slots can be filled in by DRI drivers that register new
    * extension functions.
    */
   return FIRST_DYNAMIC_OFFSET + MAX_EXTENSION_FUNCS;
}

