/**
 * \file texstate.h
 * Texture state management.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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


#ifndef TEXSTATE_H
#define TEXSTATE_H


#include "compiler.h"
#include "enums.h"
#include "macros.h"
#include "mtypes.h"


static inline struct gl_texture_unit *
_mesa_get_tex_unit(struct gl_context *ctx, GLuint unit)
{
   assert(unit < ARRAY_SIZE(ctx->Texture.Unit));
   return &(ctx->Texture.Unit[unit]);
}

/**
 * Return pointer to current texture unit.
 * This the texture unit set by glActiveTexture(), not glClientActiveTexture().
 */
static inline struct gl_texture_unit *
_mesa_get_current_tex_unit(struct gl_context *ctx)
{
   return _mesa_get_tex_unit(ctx, ctx->Texture.CurrentUnit);
}

static inline GLuint
_mesa_max_tex_unit(struct gl_context *ctx)
{
   /* See OpenGL spec for glActiveTexture: */
   return MAX2(ctx->Const.MaxCombinedTextureImageUnits,
               ctx->Const.MaxTextureCoordUnits);
}

static inline struct gl_texture_unit *
_mesa_get_tex_unit_err(struct gl_context *ctx, GLuint unit, const char *func)
{
   if (unit < _mesa_max_tex_unit(ctx))
      return _mesa_get_tex_unit(ctx, unit);

   /* Note: This error is a precedent set by glBindTextures. From the GL 4.5
    * specification (30.10.2014) Section 8.1 ("Texture Objects"):
    *
    *    "An INVALID_OPERATION error is generated if first + count is greater
    *     than the number of texture image units supported by the
    *     implementation."
    */
   _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unit=%s)", func,
               _mesa_lookup_enum_by_nr(GL_TEXTURE0+unit));
   return NULL;
}


extern void
_mesa_copy_texture_state( const struct gl_context *src, struct gl_context *dst );

extern void
_mesa_print_texunit_state( struct gl_context *ctx, GLuint unit );



/**
 * \name Called from API
 */
/*@{*/

extern void GLAPIENTRY
_mesa_ActiveTexture( GLenum target );

extern void GLAPIENTRY
_mesa_ClientActiveTexture( GLenum target );

/*@}*/


/**
 * \name Initialization, state maintenance
 */
/*@{*/

extern void 
_mesa_update_texture( struct gl_context *ctx, GLuint new_state );

extern GLboolean
_mesa_init_texture( struct gl_context *ctx );

extern void 
_mesa_free_texture_data( struct gl_context *ctx );

extern void
_mesa_update_default_objects_texture(struct gl_context *ctx);

/*@}*/

#endif
