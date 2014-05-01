/*
 * Mesa 3-D graphics library
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdbool.h>
#include "glheader.h"
#include "context.h"
#include "get.h"
#include "enums.h"
#include "extensions.h"
#include "mtypes.h"


/**
 * Return the string for a glGetString(GL_SHADING_LANGUAGE_VERSION) query.
 */
static const GLubyte *
shading_language_version(struct gl_context *ctx)
{
   switch (ctx->API) {
   case API_OPENGL_COMPAT:
   case API_OPENGL_CORE:
      switch (ctx->Const.GLSLVersion) {
      case 120:
         return (const GLubyte *) "1.20";
      case 130:
         return (const GLubyte *) "1.30";
      case 140:
         return (const GLubyte *) "1.40";
      case 150:
         return (const GLubyte *) "1.50";
      case 330:
         return (const GLubyte *) "3.30";
      case 400:
         return (const GLubyte *) "4.00";
      case 410:
         return (const GLubyte *) "4.10";
      case 420:
         return (const GLubyte *) "4.20";
      default:
         _mesa_problem(ctx,
                       "Invalid GLSL version in shading_language_version()");
         return (const GLubyte *) 0;
      }
      break;

   case API_OPENGLES2:
      return (ctx->Version < 30)
         ? (const GLubyte *) "OpenGL ES GLSL ES 1.0.16"
         : (const GLubyte *) "OpenGL ES GLSL ES 3.0";

   case API_OPENGLES:
      /* fall-through */

   default:
      _mesa_problem(ctx, "Unexpected API value in shading_language_version()");
      return (const GLubyte *) 0;
   }
}


/**
 * Query string-valued state.  The return value should _not_ be freed by
 * the caller.
 *
 * \param name  the state variable to query.
 *
 * \sa glGetString().
 *
 * Tries to get the string from dd_function_table::GetString, otherwise returns
 * the hardcoded strings.
 */
const GLubyte * GLAPIENTRY
_mesa_GetString( GLenum name )
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *vendor = "Brian Paul";
   static const char *renderer = "Mesa";

   if (!ctx)
      return NULL;

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, NULL);

   /* this is a required driver function */
   assert(ctx->Driver.GetString);
   {
      /* Give the driver the chance to handle this query */
      const GLubyte *str = (*ctx->Driver.GetString)(ctx, name);
      if (str)
         return str;
   }

   switch (name) {
      case GL_VENDOR:
         return (const GLubyte *) vendor;
      case GL_RENDERER:
         return (const GLubyte *) renderer;
      case GL_VERSION:
         return (const GLubyte *) ctx->VersionString;
      case GL_EXTENSIONS:
         if (ctx->API == API_OPENGL_CORE) {
            _mesa_error(ctx, GL_INVALID_ENUM, "glGetString(GL_EXTENSIONS)");
            return (const GLubyte *) 0;
         }
         return (const GLubyte *) ctx->Extensions.String;
      case GL_SHADING_LANGUAGE_VERSION:
         if (ctx->API == API_OPENGLES)
            break;
	 return shading_language_version(ctx);
      case GL_PROGRAM_ERROR_STRING_ARB:
         if (ctx->API == API_OPENGL_COMPAT &&
             (ctx->Extensions.ARB_fragment_program ||
              ctx->Extensions.ARB_vertex_program)) {
            return (const GLubyte *) ctx->Program.ErrorString;
         }
         break;
      default:
         break;
   }

   _mesa_error( ctx, GL_INVALID_ENUM, "glGetString" );
   return (const GLubyte *) 0;
}


/**
 * GL3
 */
const GLubyte * GLAPIENTRY
_mesa_GetStringi(GLenum name, GLuint index)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx)
      return NULL;

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, NULL);

   switch (name) {
   case GL_EXTENSIONS:
      if (index >= _mesa_get_extension_count(ctx)) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glGetStringi(index=%u)", index);
         return (const GLubyte *) 0;
      }
      return _mesa_get_enabled_extension(ctx, index);
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetStringi");
      return (const GLubyte *) 0;
   }
}



/**
 * Return pointer-valued state, such as a vertex array pointer.
 *
 * \param pname  names state to be queried
 * \param params  returns the pointer value
 *
 * \sa glGetPointerv().
 *
 * Tries to get the specified pointer via dd_function_table::GetPointerv,
 * otherwise gets the specified pointer from the current context.
 */
void GLAPIENTRY
_mesa_GetPointerv( GLenum pname, GLvoid **params )
{
   GET_CURRENT_CONTEXT(ctx);
   const GLuint clientUnit = ctx->Array.ActiveTexture;

   if (!params)
      return;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGetPointerv %s\n", _mesa_lookup_enum_by_nr(pname));

   switch (pname) {
      case GL_VERTEX_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_POS].Ptr;
         break;
      case GL_NORMAL_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_NORMAL].Ptr;
         break;
      case GL_COLOR_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_COLOR0].Ptr;
         break;
      case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_COLOR1].Ptr;
         break;
      case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_FOG].Ptr;
         break;
      case GL_INDEX_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_COLOR_INDEX].Ptr;
         break;
      case GL_TEXTURE_COORD_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_TEX(clientUnit)].Ptr;
         break;
      case GL_EDGE_FLAG_ARRAY_POINTER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_EDGEFLAG].Ptr;
         break;
      case GL_FEEDBACK_BUFFER_POINTER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = ctx->Feedback.Buffer;
         break;
      case GL_SELECTION_BUFFER_POINTER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = ctx->Select.Buffer;
         break;
      case GL_POINT_SIZE_ARRAY_POINTER_OES:
         if (ctx->API != API_OPENGLES)
            goto invalid_pname;
         *params = (GLvoid *) ctx->Array.VAO->VertexAttrib[VERT_ATTRIB_POINT_SIZE].Ptr;
         break;
      case GL_DEBUG_CALLBACK_FUNCTION_ARB:
      case GL_DEBUG_CALLBACK_USER_PARAM_ARB:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_pname;
         else
            *params = _mesa_get_debug_state_ptr(ctx, pname);
         break;
      default:
         goto invalid_pname;
   }

   return;

invalid_pname:
   _mesa_error( ctx, GL_INVALID_ENUM, "glGetPointerv" );
   return;
}


/**
 * Returns the current GL error code, or GL_NO_ERROR.
 * \return current error code
 *
 * Returns __struct gl_contextRec::ErrorValue.
 */
GLenum GLAPIENTRY
_mesa_GetError( void )
{
   GET_CURRENT_CONTEXT(ctx);
   GLenum e = ctx->ErrorValue;
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, 0);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glGetError <-- %s\n", _mesa_lookup_enum_by_nr(e));

   ctx->ErrorValue = (GLenum) GL_NO_ERROR;
   ctx->ErrorDebugCount = 0;
   return e;
}

/**
 * Returns an error code specified by GL_ARB_robustness, or GL_NO_ERROR.
 * \return current context status
 */
GLenum GLAPIENTRY
_mesa_GetGraphicsResetStatusARB( void )
{
   GET_CURRENT_CONTEXT(ctx);
   GLenum status = GL_NO_ERROR;

   /* The ARB_robustness specification says:
    *
    *     "If the reset notification behavior is NO_RESET_NOTIFICATION_ARB,
    *     then the implementation will never deliver notification of reset
    *     events, and GetGraphicsResetStatusARB will always return NO_ERROR."
    */
   if (ctx->Const.ResetStrategy == GL_NO_RESET_NOTIFICATION_ARB) {
      if (MESA_VERBOSE & VERBOSE_API)
         _mesa_debug(ctx,
                     "glGetGraphicsResetStatusARB always returns GL_NO_ERROR "
                     "because reset notifictation was not requested at context "
                     "creation.\n");

      return GL_NO_ERROR;
   }

   if (ctx->Driver.GetGraphicsResetStatus) {
      /* Query the reset status of this context from the driver core.
       */
      status = ctx->Driver.GetGraphicsResetStatus(ctx);

      mtx_lock(&ctx->Shared->Mutex);

      /* If this context has not been affected by a GPU reset, check to see if
       * some other context in the share group has been affected by a reset.
       * If another context saw a reset but this context did not, assume that
       * this context was not guilty.
       */
      if (status != GL_NO_ERROR) {
         ctx->Shared->ShareGroupReset = true;
      } else if (ctx->Shared->ShareGroupReset && !ctx->ShareGroupReset) {
         status = GL_INNOCENT_CONTEXT_RESET_ARB;
      }

      ctx->ShareGroupReset = ctx->Shared->ShareGroupReset;
      mtx_unlock(&ctx->Shared->Mutex);
   }

   if (!ctx->Driver.GetGraphicsResetStatus && (MESA_VERBOSE & VERBOSE_API))
      _mesa_debug(ctx,
                  "glGetGraphicsResetStatusARB always returns GL_NO_ERROR "
                  "because the driver doesn't track reset status.\n");

   return status;
}
