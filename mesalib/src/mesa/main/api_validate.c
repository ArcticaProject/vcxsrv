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

#include <stdbool.h>
#include "glheader.h"
#include "api_validate.h"
#include "bufferobj.h"
#include "context.h"
#include "imports.h"
#include "mtypes.h"
#include "enums.h"
#include "vbo/vbo.h"
#include "transformfeedback.h"
#include <stdbool.h>


/**
 * \return  number of bytes in array [count] of type.
 */
static GLsizei
index_bytes(GLenum type, GLsizei count)
{
   if (type == GL_UNSIGNED_INT) {
      return count * sizeof(GLuint);
   }
   else if (type == GL_UNSIGNED_BYTE) {
      return count * sizeof(GLubyte);
   }
   else {
      ASSERT(type == GL_UNSIGNED_SHORT);
      return count * sizeof(GLushort);
   }
}


/**
 * Find the max index in the given element/index buffer
 */
GLuint
_mesa_max_buffer_index(struct gl_context *ctx, GLuint count, GLenum type,
                       const void *indices,
                       struct gl_buffer_object *elementBuf)
{
   const GLubyte *map = NULL;
   GLuint max = 0;
   GLuint i;

   if (_mesa_is_bufferobj(elementBuf)) {
      /* elements are in a user-defined buffer object.  need to map it */
      map = ctx->Driver.MapBufferRange(ctx, 0, elementBuf->Size,
				       GL_MAP_READ_BIT, elementBuf,
                                       MAP_INTERNAL);
      /* Actual address is the sum of pointers */
      indices = (const GLvoid *) ADD_POINTERS(map, (const GLubyte *) indices);
   }

   if (type == GL_UNSIGNED_INT) {
      for (i = 0; i < count; i++)
         if (((GLuint *) indices)[i] > max)
            max = ((GLuint *) indices)[i];
   }
   else if (type == GL_UNSIGNED_SHORT) {
      for (i = 0; i < count; i++)
         if (((GLushort *) indices)[i] > max)
            max = ((GLushort *) indices)[i];
   }
   else {
      ASSERT(type == GL_UNSIGNED_BYTE);
      for (i = 0; i < count; i++)
         if (((GLubyte *) indices)[i] > max)
            max = ((GLubyte *) indices)[i];
   }

   if (map) {
      ctx->Driver.UnmapBuffer(ctx, elementBuf, MAP_INTERNAL);
   }

   return max;
}


/**
 * Check if OK to draw arrays/elements.
 */
static GLboolean
check_valid_to_render(struct gl_context *ctx, const char *function)
{
   if (!_mesa_valid_to_render(ctx, function)) {
      return GL_FALSE;
   }

   switch (ctx->API) {
   case API_OPENGLES2:
      /* For ES2, we can draw if any vertex array is enabled (and we
       * should always have a vertex program/shader). */
      if (ctx->Array.VAO->_Enabled == 0x0 || !ctx->VertexProgram._Current)
	 return GL_FALSE;
      break;

   case API_OPENGLES:
      /* For OpenGL ES, only draw if we have vertex positions
       */
      if (!ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_POS].Enabled)
	 return GL_FALSE;
      break;

   case API_OPENGL_CORE:
      if (ctx->Array.VAO == ctx->Array.DefaultVAO)
         return GL_FALSE;
      /* fallthrough */
   case API_OPENGL_COMPAT:
      {
         const struct gl_shader_program *vsProg =
            ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX];
         GLboolean haveVertexShader = (vsProg && vsProg->LinkStatus);
         GLboolean haveVertexProgram = ctx->VertexProgram._Enabled;
         if (haveVertexShader || haveVertexProgram) {
            /* Draw regardless of whether or not we have any vertex arrays.
             * (Ex: could draw a point using a constant vertex pos)
             */
            return GL_TRUE;
         }
         else {
            /* Draw if we have vertex positions (GL_VERTEX_ARRAY or generic
             * array [0]).
             */
            return (ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_POS].Enabled ||
                    ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_GENERIC0].Enabled);
         }
      }
      break;

   default:
      assert(!"Invalid API value in check_valid_to_render()");
   }

   return GL_TRUE;
}


/**
 * Do bounds checking on array element indexes.  Check that the vertices
 * pointed to by the indices don't lie outside buffer object bounds.
 * \return GL_TRUE if OK, GL_FALSE if any indexed vertex goes is out of bounds
 */
static GLboolean
check_index_bounds(struct gl_context *ctx, GLsizei count, GLenum type,
		   const GLvoid *indices, GLint basevertex)
{
   struct _mesa_prim prim;
   struct _mesa_index_buffer ib;
   GLuint min, max;

   /* Only the X Server needs to do this -- otherwise, accessing outside
    * array/BO bounds allows application termination.
    */
   if (!ctx->Const.CheckArrayBounds)
      return GL_TRUE;

   memset(&prim, 0, sizeof(prim));
   prim.count = count;

   memset(&ib, 0, sizeof(ib));
   ib.type = type;
   ib.ptr = indices;
   ib.obj = ctx->Array.VAO->IndexBufferObj;

   vbo_get_minmax_indices(ctx, &prim, &ib, &min, &max, 1);

   if ((int)(min + basevertex) < 0 ||
       max + basevertex >= ctx->Array.VAO->_MaxElement) {
      /* the max element is out of bounds of one or more enabled arrays */
      _mesa_warning(ctx, "glDrawElements() index=%u is out of bounds (max=%u)",
                    max, ctx->Array.VAO->_MaxElement);
      return GL_FALSE;
   }

   return GL_TRUE;
}


/**
 * Is 'mode' a valid value for glBegin(), glDrawArrays(), glDrawElements(),
 * etc?  The set of legal values depends on whether geometry shaders/programs
 * are supported.
 * Note: This may be called during display list compilation.
 */
bool
_mesa_is_valid_prim_mode(struct gl_context *ctx, GLenum mode)
{
   switch (mode) {
   case GL_POINTS:
   case GL_LINES:
   case GL_LINE_LOOP:
   case GL_LINE_STRIP:
   case GL_TRIANGLES:
   case GL_TRIANGLE_STRIP:
   case GL_TRIANGLE_FAN:
      return true;
   case GL_QUADS:
   case GL_QUAD_STRIP:
   case GL_POLYGON:
      return (ctx->API == API_OPENGL_COMPAT);
   case GL_LINES_ADJACENCY:
   case GL_LINE_STRIP_ADJACENCY:
   case GL_TRIANGLES_ADJACENCY:
   case GL_TRIANGLE_STRIP_ADJACENCY:
      return _mesa_has_geometry_shaders(ctx);
   default:
      return false;
   }
}


/**
 * Is 'mode' a valid value for glBegin(), glDrawArrays(), glDrawElements(),
 * etc?  Also, do additional checking related to transformation feedback.
 * Note: this function cannot be called during glNewList(GL_COMPILE) because
 * this code depends on current transform feedback state.
 */
GLboolean
_mesa_valid_prim_mode(struct gl_context *ctx, GLenum mode, const char *name)
{
   bool valid_enum = _mesa_is_valid_prim_mode(ctx, mode);

   if (!valid_enum) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(mode=%x)", name, mode);
      return GL_FALSE;
   }

   /* From the ARB_geometry_shader4 spec:
    *
    * The error INVALID_OPERATION is generated if Begin, or any command that
    * implicitly calls Begin, is called when a geometry shader is active and:
    *
    * * the input primitive type of the current geometry shader is
    *   POINTS and <mode> is not POINTS,
    *
    * * the input primitive type of the current geometry shader is
    *   LINES and <mode> is not LINES, LINE_STRIP, or LINE_LOOP,
    *
    * * the input primitive type of the current geometry shader is
    *   TRIANGLES and <mode> is not TRIANGLES, TRIANGLE_STRIP or
    *   TRIANGLE_FAN,
    *
    * * the input primitive type of the current geometry shader is
    *   LINES_ADJACENCY_ARB and <mode> is not LINES_ADJACENCY_ARB or
    *   LINE_STRIP_ADJACENCY_ARB, or
    *
    * * the input primitive type of the current geometry shader is
    *   TRIANGLES_ADJACENCY_ARB and <mode> is not
    *   TRIANGLES_ADJACENCY_ARB or TRIANGLE_STRIP_ADJACENCY_ARB.
    *
   */
   if (ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY]) {
      const GLenum geom_mode =
         ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY]->Geom.InputType;
      switch (mode) {
      case GL_POINTS:
         valid_enum = (geom_mode == GL_POINTS);
         break;
      case GL_LINES:
      case GL_LINE_LOOP:
      case GL_LINE_STRIP:
         valid_enum = (geom_mode == GL_LINES);
         break;
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
         valid_enum = (geom_mode == GL_TRIANGLES);
         break;
      case GL_QUADS:
      case GL_QUAD_STRIP:
      case GL_POLYGON:
         valid_enum = false;
         break;
      case GL_LINES_ADJACENCY:
      case GL_LINE_STRIP_ADJACENCY:
         valid_enum = (geom_mode == GL_LINES_ADJACENCY);
         break;
      case GL_TRIANGLES_ADJACENCY:
      case GL_TRIANGLE_STRIP_ADJACENCY:
         valid_enum = (geom_mode == GL_TRIANGLES_ADJACENCY);
         break;
      default:
         valid_enum = false;
         break;
      }
      if (!valid_enum) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(mode=%s vs geometry shader input %s)",
                     name,
                     _mesa_lookup_prim_by_nr(mode),
                     _mesa_lookup_prim_by_nr(geom_mode));
         return GL_FALSE;
      }
   }

   /* From the GL_EXT_transform_feedback spec:
    *
    *     "The error INVALID_OPERATION is generated if Begin, or any command
    *      that performs an explicit Begin, is called when:
    *
    *      * a geometry shader is not active and <mode> does not match the
    *        allowed begin modes for the current transform feedback state as
    *        given by table X.1.
    *
    *      * a geometry shader is active and the output primitive type of the
    *        geometry shader does not match the allowed begin modes for the
    *        current transform feedback state as given by table X.1.
    *
    */
   if (_mesa_is_xfb_active_and_unpaused(ctx)) {
      GLboolean pass = GL_TRUE;

      if(ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY]) {
         switch (ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY]->Geom.OutputType) {
         case GL_POINTS:
            pass = ctx->TransformFeedback.Mode == GL_POINTS;
            break;
         case GL_LINE_STRIP:
            pass = ctx->TransformFeedback.Mode == GL_LINES;
            break;
         case GL_TRIANGLE_STRIP:
            pass = ctx->TransformFeedback.Mode == GL_TRIANGLES;
            break;
         default:
            pass = GL_FALSE;
         }
      }
      else {
         switch (mode) {
         case GL_POINTS:
            pass = ctx->TransformFeedback.Mode == GL_POINTS;
            break;
         case GL_LINES:
         case GL_LINE_STRIP:
         case GL_LINE_LOOP:
            pass = ctx->TransformFeedback.Mode == GL_LINES;
            break;
         default:
            pass = ctx->TransformFeedback.Mode == GL_TRIANGLES;
            break;
         }
      }
      if (!pass) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
	                 "%s(mode=%s vs transform feedback %s)",
	                 name,
	                 _mesa_lookup_prim_by_nr(mode),
	                 _mesa_lookup_prim_by_nr(ctx->TransformFeedback.Mode));
         return GL_FALSE;
      }
   }

   return GL_TRUE;
}

/**
 * Verify that the element type is valid.
 *
 * Generates \c GL_INVALID_ENUM and returns \c false if it is not.
 */
static bool
valid_elements_type(struct gl_context *ctx, GLenum type, const char *name)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
   case GL_UNSIGNED_SHORT:
   case GL_UNSIGNED_INT:
      return true;

   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(type = %s)", name,
                  _mesa_lookup_enum_by_nr(type));
      return false;
   }
}

/**
 * Error checking for glDrawElements().  Includes parameter checking
 * and VBO bounds checking.
 * \return GL_TRUE if OK to render, GL_FALSE if error found
 */
GLboolean
_mesa_validate_DrawElements(struct gl_context *ctx,
			    GLenum mode, GLsizei count, GLenum type,
			    const GLvoid *indices, GLint basevertex)
{
   FLUSH_CURRENT(ctx, 0);

   /* From the GLES3 specification, section 2.14.2 (Transform Feedback
    * Primitive Capture):
    *
    *   The error INVALID_OPERATION is also generated by DrawElements,
    *   DrawElementsInstanced, and DrawRangeElements while transform feedback
    *   is active and not paused, regardless of mode.
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glDrawElements(transform feedback active)");
      return GL_FALSE;
   }

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawElements(count)" );
      return GL_FALSE;
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawElements")) {
      return GL_FALSE;
   }

   if (!valid_elements_type(ctx, type, "glDrawElements"))
      return GL_FALSE;

   if (!check_valid_to_render(ctx, "glDrawElements"))
      return GL_FALSE;

   /* Vertex buffer object tests */
   if (_mesa_is_bufferobj(ctx->Array.VAO->IndexBufferObj)) {
      /* use indices in the buffer object */
      /* make sure count doesn't go outside buffer bounds */
      if (index_bytes(type, count) > ctx->Array.VAO->IndexBufferObj->Size) {
         _mesa_warning(ctx, "glDrawElements index out of buffer bounds");
         return GL_FALSE;
      }
   }
   else {
      /* not using a VBO */
      if (!indices)
         return GL_FALSE;
   }

   if (!check_index_bounds(ctx, count, type, indices, basevertex))
      return GL_FALSE;

   if (count == 0)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Error checking for glMultiDrawElements().  Includes parameter checking
 * and VBO bounds checking.
 * \return GL_TRUE if OK to render, GL_FALSE if error found
 */
GLboolean
_mesa_validate_MultiDrawElements(struct gl_context *ctx,
                                 GLenum mode, const GLsizei *count,
                                 GLenum type, const GLvoid * const *indices,
                                 GLuint primcount, const GLint *basevertex)
{
   unsigned i;

   FLUSH_CURRENT(ctx, 0);

   for (i = 0; i < primcount; i++) {
      if (count[i] < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glMultiDrawElements(count)" );
         return GL_FALSE;
      }
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glMultiDrawElements")) {
      return GL_FALSE;
   }

   if (!valid_elements_type(ctx, type, "glMultiDrawElements"))
      return GL_FALSE;

   if (!check_valid_to_render(ctx, "glMultiDrawElements"))
      return GL_FALSE;

   /* Vertex buffer object tests */
   if (_mesa_is_bufferobj(ctx->Array.VAO->IndexBufferObj)) {
      /* use indices in the buffer object */
      /* make sure count doesn't go outside buffer bounds */
      for (i = 0; i < primcount; i++) {
         if (index_bytes(type, count[i]) >
             ctx->Array.VAO->IndexBufferObj->Size) {
            _mesa_warning(ctx,
                          "glMultiDrawElements index out of buffer bounds");
            return GL_FALSE;
         }
      }
   }
   else {
      /* not using a VBO */
      for (i = 0; i < primcount; i++) {
         if (!indices[i])
            return GL_FALSE;
      }
   }

   for (i = 0; i < primcount; i++) {
      if (!check_index_bounds(ctx, count[i], type, indices[i],
                              basevertex ? basevertex[i] : 0))
         return GL_FALSE;
   }

   return GL_TRUE;
}


/**
 * Error checking for glDrawRangeElements().  Includes parameter checking
 * and VBO bounds checking.
 * \return GL_TRUE if OK to render, GL_FALSE if error found
 */
GLboolean
_mesa_validate_DrawRangeElements(struct gl_context *ctx, GLenum mode,
				 GLuint start, GLuint end,
				 GLsizei count, GLenum type,
				 const GLvoid *indices, GLint basevertex)
{
   FLUSH_CURRENT(ctx, 0);

   /* From the GLES3 specification, section 2.14.2 (Transform Feedback
    * Primitive Capture):
    *
    *   The error INVALID_OPERATION is also generated by DrawElements,
    *   DrawElementsInstanced, and DrawRangeElements while transform feedback
    *   is active and not paused, regardless of mode.
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glDrawElements(transform feedback active)");
      return GL_FALSE;
   }

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawRangeElements(count)" );
      return GL_FALSE;
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawRangeElements")) {
      return GL_FALSE;
   }

   if (end < start) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawRangeElements(end<start)");
      return GL_FALSE;
   }

   if (!valid_elements_type(ctx, type, "glDrawRangeElements"))
      return GL_FALSE;

   if (!check_valid_to_render(ctx, "glDrawRangeElements"))
      return GL_FALSE;

   /* Vertex buffer object tests */
   if (_mesa_is_bufferobj(ctx->Array.VAO->IndexBufferObj)) {
      /* use indices in the buffer object */
      /* make sure count doesn't go outside buffer bounds */
      if (index_bytes(type, count) > ctx->Array.VAO->IndexBufferObj->Size) {
         _mesa_warning(ctx, "glDrawRangeElements index out of buffer bounds");
         return GL_FALSE;
      }
   }
   else {
      /* not using a VBO */
      if (!indices)
         return GL_FALSE;
   }

   if (!check_index_bounds(ctx, count, type, indices, basevertex))
      return GL_FALSE;

   if (count == 0)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Called from the tnl module to error check the function parameters and
 * verify that we really can draw something.
 * \return GL_TRUE if OK to render, GL_FALSE if error found
 */
GLboolean
_mesa_validate_DrawArrays(struct gl_context *ctx,
			  GLenum mode, GLint start, GLsizei count)
{
   struct gl_transform_feedback_object *xfb_obj
      = ctx->TransformFeedback.CurrentObject;
   FLUSH_CURRENT(ctx, 0);

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawArrays(count)" );
      return GL_FALSE;
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawArrays")) {
      return GL_FALSE;
   }

   if (!check_valid_to_render(ctx, "glDrawArrays"))
      return GL_FALSE;

   if (ctx->Const.CheckArrayBounds) {
      if (start + count > (GLint) ctx->Array.VAO->_MaxElement)
         return GL_FALSE;
   }

   /* From the GLES3 specification, section 2.14.2 (Transform Feedback
    * Primitive Capture):
    *
    *   The error INVALID_OPERATION is generated by DrawArrays and
    *   DrawArraysInstanced if recording the vertices of a primitive to the
    *   buffer objects being used for transform feedback purposes would result
    *   in either exceeding the limits of any buffer object’s size, or in
    *   exceeding the end position offset + size − 1, as set by
    *   BindBufferRange.
    *
    * This is in contrast to the behaviour of desktop GL, where the extra
    * primitives are silently dropped from the transform feedback buffer.
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_xfb_active_and_unpaused(ctx)) {
      size_t prim_count = vbo_count_tessellated_primitives(mode, count, 1);
      if (xfb_obj->GlesRemainingPrims < prim_count) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glDrawArrays(exceeds transform feedback size)");
         return GL_FALSE;
      }
      xfb_obj->GlesRemainingPrims -= prim_count;
   }

   if (count == 0)
      return GL_FALSE;

   return GL_TRUE;
}


GLboolean
_mesa_validate_DrawArraysInstanced(struct gl_context *ctx, GLenum mode, GLint first,
                                   GLsizei count, GLsizei numInstances)
{
   struct gl_transform_feedback_object *xfb_obj
      = ctx->TransformFeedback.CurrentObject;
   FLUSH_CURRENT(ctx, 0);

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDrawArraysInstanced(count=%d)", count);
      return GL_FALSE;
   }

   if (first < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glDrawArraysInstanced(start=%d)", first);
      return GL_FALSE;
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawArraysInstanced")) {
      return GL_FALSE;
   }

   if (numInstances <= 0) {
      if (numInstances < 0)
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glDrawArraysInstanced(numInstances=%d)", numInstances);
      return GL_FALSE;
   }

   if (!check_valid_to_render(ctx, "glDrawArraysInstanced(invalid to render)"))
      return GL_FALSE;

   if (ctx->Const.CheckArrayBounds) {
      if (first + count > (GLint) ctx->Array.VAO->_MaxElement)
         return GL_FALSE;
   }

   /* From the GLES3 specification, section 2.14.2 (Transform Feedback
    * Primitive Capture):
    *
    *   The error INVALID_OPERATION is generated by DrawArrays and
    *   DrawArraysInstanced if recording the vertices of a primitive to the
    *   buffer objects being used for transform feedback purposes would result
    *   in either exceeding the limits of any buffer object’s size, or in
    *   exceeding the end position offset + size − 1, as set by
    *   BindBufferRange.
    *
    * This is in contrast to the behaviour of desktop GL, where the extra
    * primitives are silently dropped from the transform feedback buffer.
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_xfb_active_and_unpaused(ctx)) {
      size_t prim_count
         = vbo_count_tessellated_primitives(mode, count, numInstances);
      if (xfb_obj->GlesRemainingPrims < prim_count) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glDrawArraysInstanced(exceeds transform feedback size)");
         return GL_FALSE;
      }
      xfb_obj->GlesRemainingPrims -= prim_count;
   }

   if (count == 0)
      return GL_FALSE;

   return GL_TRUE;
}


GLboolean
_mesa_validate_DrawElementsInstanced(struct gl_context *ctx,
                                     GLenum mode, GLsizei count, GLenum type,
                                     const GLvoid *indices, GLsizei numInstances,
                                     GLint basevertex)
{
   FLUSH_CURRENT(ctx, 0);

   /* From the GLES3 specification, section 2.14.2 (Transform Feedback
    * Primitive Capture):
    *
    *   The error INVALID_OPERATION is also generated by DrawElements,
    *   DrawElementsInstanced, and DrawRangeElements while transform feedback
    *   is active and not paused, regardless of mode.
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glDrawElements(transform feedback active)");
      return GL_FALSE;
   }

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDrawElementsInstanced(count=%d)", count);
      return GL_FALSE;
   }

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawElementsInstanced")) {
      return GL_FALSE;
   }

   if (!valid_elements_type(ctx, type, "glDrawElementsInstanced"))
      return GL_FALSE;

   if (numInstances <= 0) {
      if (numInstances < 0)
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glDrawElementsInstanced(numInstances=%d)", numInstances);
      return GL_FALSE;
   }

   if (!check_valid_to_render(ctx, "glDrawElementsInstanced"))
      return GL_FALSE;

   /* Vertex buffer object tests */
   if (_mesa_is_bufferobj(ctx->Array.VAO->IndexBufferObj)) {
      /* use indices in the buffer object */
      /* make sure count doesn't go outside buffer bounds */
      if (index_bytes(type, count) > ctx->Array.VAO->IndexBufferObj->Size) {
         _mesa_warning(ctx,
                       "glDrawElementsInstanced index out of buffer bounds");
         return GL_FALSE;
      }
   }
   else {
      /* not using a VBO */
      if (!indices)
         return GL_FALSE;
   }

   if (count == 0)
      return GL_FALSE;

   if (!check_index_bounds(ctx, count, type, indices, basevertex))
      return GL_FALSE;

   return GL_TRUE;
}


GLboolean
_mesa_validate_DrawTransformFeedback(struct gl_context *ctx,
                                     GLenum mode,
                                     struct gl_transform_feedback_object *obj,
                                     GLuint stream,
                                     GLsizei numInstances)
{
   FLUSH_CURRENT(ctx, 0);

   if (!_mesa_valid_prim_mode(ctx, mode, "glDrawTransformFeedback*(mode)")) {
      return GL_FALSE;
   }

   if (!obj) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawTransformFeedback*(name)");
      return GL_FALSE;
   }

   if (!obj->EndedAnytime) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glDrawTransformFeedback*");
      return GL_FALSE;
   }

   if (stream >= ctx->Const.MaxVertexStreams) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDrawTransformFeedbackStream*(index>=MaxVertexStream)");
      return GL_FALSE;
   }

   if (numInstances <= 0) {
      if (numInstances < 0)
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glDrawTransformFeedback*Instanced(numInstances=%d)",
                     numInstances);
      return GL_FALSE;
   }

   if (!check_valid_to_render(ctx, "glDrawTransformFeedback*")) {
      return GL_FALSE;
   }

   return GL_TRUE;
}

static GLboolean
valid_draw_indirect(struct gl_context *ctx,
                    GLenum mode, const GLvoid *indirect,
                    GLsizei size, const char *name)
{
   const GLsizeiptr end = (GLsizeiptr)indirect + size;

   if (!_mesa_valid_prim_mode(ctx, mode, name))
      return GL_FALSE;


   /* From the ARB_draw_indirect specification:
    * "An INVALID_OPERATION error is generated [...] if <indirect> is no
    *  word aligned."
    */
   if ((GLsizeiptr)indirect & (sizeof(GLuint) - 1)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(indirect is not aligned)", name);
      return GL_FALSE;
   }

   if (!_mesa_is_bufferobj(ctx->DrawIndirectBuffer)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s: no buffer bound to DRAW_INDIRECT_BUFFER", name);
      return GL_FALSE;
   }

   if (_mesa_check_disallowed_mapping(ctx->DrawIndirectBuffer)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(DRAW_INDIRECT_BUFFER is mapped)", name);
      return GL_FALSE;
   }

   /* From the ARB_draw_indirect specification:
    * "An INVALID_OPERATION error is generated if the commands source data
    *  beyond the end of the buffer object [...]"
    */
   if (ctx->DrawIndirectBuffer->Size < end) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(DRAW_INDIRECT_BUFFER too small)", name);
      return GL_FALSE;
   }

   if (!check_valid_to_render(ctx, name))
      return GL_FALSE;

   return GL_TRUE;
}

static inline GLboolean
valid_draw_indirect_elements(struct gl_context *ctx,
                             GLenum mode, GLenum type, const GLvoid *indirect,
                             GLsizeiptr size, const char *name)
{
   if (!valid_elements_type(ctx, type, name))
      return GL_FALSE;

   /*
    * Unlike regular DrawElementsInstancedBaseVertex commands, the indices
    * may not come from a client array and must come from an index buffer.
    * If no element array buffer is bound, an INVALID_OPERATION error is
    * generated.
    */
   if (!_mesa_is_bufferobj(ctx->Array.VAO->IndexBufferObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(no buffer bound to GL_ELEMENT_ARRAY_BUFFER)", name);
      return GL_FALSE;
   }

   return valid_draw_indirect(ctx, mode, indirect, size, name);
}

static inline GLboolean
valid_draw_indirect_multi(struct gl_context *ctx,
                          GLsizei primcount, GLsizei stride,
                          const char *name)
{

   /* From the ARB_multi_draw_indirect specification:
    * "INVALID_VALUE is generated by MultiDrawArraysIndirect or
    *  MultiDrawElementsIndirect if <primcount> is negative."
    *
    * "<primcount> must be positive, otherwise an INVALID_VALUE error will
    *  be generated."
    */
   if (primcount < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(primcount < 0)", name);
      return GL_FALSE;
   }


   /* From the ARB_multi_draw_indirect specification:
    * "<stride> must be a multiple of four, otherwise an INVALID_VALUE
    *  error is generated."
    */
   if (stride % 4) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(stride %% 4)", name);
      return GL_FALSE;
   }

   return GL_TRUE;
}

GLboolean
_mesa_validate_DrawArraysIndirect(struct gl_context *ctx,
                                  GLenum mode,
                                  const GLvoid *indirect)
{
   const unsigned drawArraysNumParams = 4;

   FLUSH_CURRENT(ctx, 0);

   return valid_draw_indirect(ctx, mode,
                              indirect, drawArraysNumParams * sizeof(GLuint),
                              "glDrawArraysIndirect");
}

GLboolean
_mesa_validate_DrawElementsIndirect(struct gl_context *ctx,
                                    GLenum mode, GLenum type,
                                    const GLvoid *indirect)
{
   const unsigned drawElementsNumParams = 5;

   FLUSH_CURRENT(ctx, 0);

   return valid_draw_indirect_elements(ctx, mode, type,
                                       indirect, drawElementsNumParams * sizeof(GLuint),
                                       "glDrawElementsIndirect");
}

GLboolean
_mesa_validate_MultiDrawArraysIndirect(struct gl_context *ctx,
                                       GLenum mode,
                                       const GLvoid *indirect,
                                       GLsizei primcount, GLsizei stride)
{
   GLsizeiptr size = 0;
   const unsigned drawArraysNumParams = 4;

   FLUSH_CURRENT(ctx, 0);

   /* caller has converted stride==0 to drawArraysNumParams * sizeof(GLuint) */
   assert(stride != 0);

   if (!valid_draw_indirect_multi(ctx, primcount, stride,
                                  "glMultiDrawArraysIndirect"))
      return GL_FALSE;

   /* number of bytes of the indirect buffer which will be read */
   size = primcount
      ? (primcount - 1) * stride + drawArraysNumParams * sizeof(GLuint)
      : 0;

   if (!valid_draw_indirect(ctx, mode, indirect, size,
                            "glMultiDrawArraysIndirect"))
      return GL_FALSE;

   return GL_TRUE;
}

GLboolean
_mesa_validate_MultiDrawElementsIndirect(struct gl_context *ctx,
                                         GLenum mode, GLenum type,
                                         const GLvoid *indirect,
                                         GLsizei primcount, GLsizei stride)
{
   GLsizeiptr size = 0;
   const unsigned drawElementsNumParams = 5;

   FLUSH_CURRENT(ctx, 0);

   /* caller has converted stride==0 to drawElementsNumParams * sizeof(GLuint) */
   assert(stride != 0);

   if (!valid_draw_indirect_multi(ctx, primcount, stride,
                                  "glMultiDrawElementsIndirect"))
      return GL_FALSE;

   /* number of bytes of the indirect buffer which will be read */
   size = primcount
      ? (primcount - 1) * stride + drawElementsNumParams * sizeof(GLuint)
      : 0;

   if (!valid_draw_indirect_elements(ctx, mode, type,
                                     indirect, size,
                                     "glMultiDrawElementsIndirect"))
      return GL_FALSE;

   return GL_TRUE;
}
