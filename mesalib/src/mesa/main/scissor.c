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


#include "main/glheader.h"
#include "main/context.h"
#include "main/mtypes.h"
#include "main/scissor.h"


/**
 * Set scissor rectangle data directly in ScissorArray
 *
 * This is an internal function that performs no error checking on the
 * supplied data.  It also does \b not call \c dd_function_table::Scissor.
 *
 * \sa _mesa_set_scissor
 */
static void
set_scissor_no_notify(struct gl_context *ctx, unsigned idx,
                      GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (x == ctx->Scissor.ScissorArray[idx].X &&
       y == ctx->Scissor.ScissorArray[idx].Y &&
       width == ctx->Scissor.ScissorArray[idx].Width &&
       height == ctx->Scissor.ScissorArray[idx].Height)
      return;

   FLUSH_VERTICES(ctx, _NEW_SCISSOR);
   ctx->Scissor.ScissorArray[idx].X = x;
   ctx->Scissor.ScissorArray[idx].Y = y;
   ctx->Scissor.ScissorArray[idx].Width = width;
   ctx->Scissor.ScissorArray[idx].Height = height;
}

/**
 * Called via glScissor
 */
void GLAPIENTRY
_mesa_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
   unsigned i;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glScissor %d %d %d %d\n", x, y, width, height);

   if (width < 0 || height < 0) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glScissor" );
      return;
   }

   /* The GL_ARB_viewport_array spec says:
    *
    *     "Scissor sets the scissor rectangle for all viewports to the same
    *     values and is equivalent (assuming no errors are generated) to:
    *
    *     for (uint i = 0; i < MAX_VIEWPORTS; i++) {
    *         ScissorIndexed(i, left, bottom, width, height);
    *     }"
    *
    * Set the scissor rectangle for all of the viewports supported by the
    * implementation, but only signal the driver once at the end.
    */
   for (i = 0; i < ctx->Const.MaxViewports; i++)
      set_scissor_no_notify(ctx, i, x, y, width, height);

   if (ctx->Driver.Scissor)
      ctx->Driver.Scissor(ctx);
}


/**
 * Define the scissor box.
 *
 * \param x, y coordinates of the scissor box lower-left corner.
 * \param width width of the scissor box.
 * \param height height of the scissor box.
 *
 * \sa glScissor().
 *
 * Verifies the parameters and updates __struct gl_contextRec::Scissor. On a
 * change flushes the vertices and notifies the driver via
 * the dd_function_table::Scissor callback.
 */
void
_mesa_set_scissor(struct gl_context *ctx, unsigned idx,
                  GLint x, GLint y, GLsizei width, GLsizei height)
{
   set_scissor_no_notify(ctx, idx, x, y, width, height);

   if (ctx->Driver.Scissor)
      ctx->Driver.Scissor(ctx);
}

/**
 * Define count scissor boxes starting at index.
 *
 * \param index  index of first scissor records to set
 * \param count  number of scissor records to set
 * \param x, y   pointer to array of struct gl_scissor_rects
 *
 * \sa glScissorArrayv().
 *
 * Verifies the parameters and call set_scissor_no_notify to do the work.
 */
void GLAPIENTRY
_mesa_ScissorArrayv(GLuint first, GLsizei count, const GLint *v)
{
   int i;
   struct gl_scissor_rect *p = (struct gl_scissor_rect *) v;
   GET_CURRENT_CONTEXT(ctx);

   if ((first + count) > ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glScissorArrayv: first (%d) + count (%d) >= MaxViewports (%d)",
                  first, count, ctx->Const.MaxViewports);
      return;
   }

   /* Verify width & height */
   for (i = 0; i < count; i++) {
      if (p[i].Width < 0 || p[i].Height < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glScissorArrayv: index (%d) width or height < 0 (%d, %d)",
                     i, p[i].Width, p[i].Height);
         return;
      }
   }

   for (i = 0; i < count; i++)
      set_scissor_no_notify(ctx, i + first,
                            p[i].X, p[i].Y, p[i].Width, p[i].Height);

   if (ctx->Driver.Scissor)
      ctx->Driver.Scissor(ctx);
}

/**
 * Define the scissor box.
 *
 * \param index  index of scissor records to set
 * \param x, y   coordinates of the scissor box lower-left corner.
 * \param width  width of the scissor box.
 * \param height height of the scissor box.
 *
 * Verifies the parameters call set_scissor_no_notify to do the work.
 */
static void
ScissorIndexed(GLuint index, GLint left, GLint bottom,
               GLsizei width, GLsizei height, const char *function)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%d, %d, %d, %d, %d)\n",
                  function, index, left, bottom, width, height);

   if (index >= ctx->Const.MaxViewports) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s: index (%d) >= MaxViewports (%d)",
                  function, index, ctx->Const.MaxViewports);
      return;
   }

   if (width < 0 || height < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s: index (%d) width or height < 0 (%d, %d)",
                  function, index, width, height);
      return;
   }

   set_scissor_no_notify(ctx, index, left, bottom, width, height);

   if (ctx->Driver.Scissor)
      ctx->Driver.Scissor(ctx);
}

void GLAPIENTRY
_mesa_ScissorIndexed(GLuint index, GLint left, GLint bottom,
                     GLsizei width, GLsizei height)
{
   ScissorIndexed(index, left, bottom, width, height, "glScissorIndexd");
}

void GLAPIENTRY
_mesa_ScissorIndexedv(GLuint index, const GLint *v)
{
   ScissorIndexed(index, v[0], v[1], v[2], v[3], "glScissorIndexdv");
}

/**
 * Initialize the context's scissor state.
 * \param ctx  the GL context.
 */
void
_mesa_init_scissor(struct gl_context *ctx)
{
   unsigned i;

   /* Scissor group */
   ctx->Scissor.EnableFlags = 0;

   /* Note: ctx->Const.MaxViewports may not have been set by the driver yet,
    * so just initialize all of them.
    */
   for (i = 0; i < MAX_VIEWPORTS; i++)
      set_scissor_no_notify(ctx, i, 0, 0, 0, 0);
}
