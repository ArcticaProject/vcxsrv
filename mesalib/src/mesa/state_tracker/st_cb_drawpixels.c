/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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

 /*
  * Authors:
  *   Brian Paul
  */

#include "main/imports.h"
#include "main/image.h"
#include "main/bufferobj.h"
#include "main/format_pack.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/pack.h"
#include "main/pbo.h"
#include "main/readpix.h"
#include "main/texformat.h"
#include "main/teximage.h"
#include "main/texstore.h"
#include "main/glformats.h"
#include "program/program.h"
#include "program/prog_print.h"
#include "program/prog_instruction.h"

#include "st_atom.h"
#include "st_atom_constbuf.h"
#include "st_cb_drawpixels.h"
#include "st_cb_readpixels.h"
#include "st_cb_fbo.h"
#include "st_context.h"
#include "st_debug.h"
#include "st_format.h"
#include "st_program.h"
#include "st_texture.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "tgsi/tgsi_ureg.h"
#include "util/u_draw_quad.h"
#include "util/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_tile.h"
#include "util/u_upload_mgr.h"
#include "cso_cache/cso_context.h"


/**
 * Check if the given program is:
 * 0: MOVE result.color, fragment.color;
 * 1: END;
 */
static GLboolean
is_passthrough_program(const struct gl_fragment_program *prog)
{
   if (prog->Base.NumInstructions == 2) {
      const struct prog_instruction *inst = prog->Base.Instructions;
      if (inst[0].Opcode == OPCODE_MOV &&
          inst[1].Opcode == OPCODE_END &&
          inst[0].DstReg.File == PROGRAM_OUTPUT &&
          inst[0].DstReg.Index == FRAG_RESULT_COLOR &&
          inst[0].DstReg.WriteMask == WRITEMASK_XYZW &&
          inst[0].SrcReg[0].File == PROGRAM_INPUT &&
          inst[0].SrcReg[0].Index == VARYING_SLOT_COL0 &&
          inst[0].SrcReg[0].Swizzle == SWIZZLE_XYZW) {
         return GL_TRUE;
      }
   }
   return GL_FALSE;
}


/**
 * Returns a fragment program which implements the current pixel transfer ops.
 */
static struct gl_fragment_program *
get_glsl_pixel_transfer_program(struct st_context *st,
                                struct st_fragment_program *orig)
{
   int pixelMaps = 0, scaleAndBias = 0;
   struct gl_context *ctx = st->ctx;
   struct st_fragment_program *fp = (struct st_fragment_program *)
      ctx->Driver.NewProgram(ctx, GL_FRAGMENT_PROGRAM_ARB, 0);

   if (!fp)
      return NULL;

   if (ctx->Pixel.RedBias != 0.0 || ctx->Pixel.RedScale != 1.0 ||
       ctx->Pixel.GreenBias != 0.0 || ctx->Pixel.GreenScale != 1.0 ||
       ctx->Pixel.BlueBias != 0.0 || ctx->Pixel.BlueScale != 1.0 ||
       ctx->Pixel.AlphaBias != 0.0 || ctx->Pixel.AlphaScale != 1.0) {
      scaleAndBias = 1;
   }

   pixelMaps = ctx->Pixel.MapColorFlag;

   if (pixelMaps) {
      /* create the colormap/texture now if not already done */
      if (!st->pixel_xfer.pixelmap_texture) {
         st->pixel_xfer.pixelmap_texture = st_create_color_map_texture(ctx);
         st->pixel_xfer.pixelmap_sampler_view =
            st_create_texture_sampler_view(st->pipe,
                                           st->pixel_xfer.pixelmap_texture);
      }
   }

   get_pixel_transfer_visitor(fp, orig->glsl_to_tgsi,
                              scaleAndBias, pixelMaps);

   return &fp->Base;
}


/**
 * Make fragment shader for glDraw/CopyPixels.  This shader is made
 * by combining the pixel transfer shader with the user-defined shader.
 * \param fpIn  the current/incoming fragment program
 * \param fpOut  returns the combined fragment program
 */
void
st_make_drawpix_fragment_program(struct st_context *st,
                                 struct gl_fragment_program *fpIn,
                                 struct gl_fragment_program **fpOut)
{
   struct gl_program *newProg;
   struct st_fragment_program *stfp = (struct st_fragment_program *) fpIn;

   if (is_passthrough_program(fpIn)) {
      newProg = (struct gl_program *) _mesa_clone_fragment_program(st->ctx,
                                             &st->pixel_xfer.program->Base);
   }
   else if (stfp->glsl_to_tgsi != NULL) {
      newProg = (struct gl_program *) get_glsl_pixel_transfer_program(st, stfp);
   }
   else {
#if 0
      /* debug */
      printf("Base program:\n");
      _mesa_print_program(&fpIn->Base);
      printf("DrawPix program:\n");
      _mesa_print_program(&st->pixel_xfer.program->Base.Base);
#endif
      newProg = _mesa_combine_programs(st->ctx,
                                       &st->pixel_xfer.program->Base.Base,
                                       &fpIn->Base);
   }

#if 0
   /* debug */
   printf("Combined DrawPixels program:\n");
   _mesa_print_program(newProg);
   printf("InputsRead: 0x%x\n", newProg->InputsRead);
   printf("OutputsWritten: 0x%x\n", newProg->OutputsWritten);
   _mesa_print_parameter_list(newProg->Parameters);
#endif

   *fpOut = (struct gl_fragment_program *) newProg;
}


/**
 * Create fragment program that does a TEX() instruction to get a Z and/or
 * stencil value value, then writes to FRAG_RESULT_DEPTH/FRAG_RESULT_STENCIL.
 * Used for glDrawPixels(GL_DEPTH_COMPONENT / GL_STENCIL_INDEX).
 * Pass fragment color through as-is.
 * \return pointer to the gl_fragment program
 */
struct gl_fragment_program *
st_make_drawpix_z_stencil_program(struct st_context *st,
                                  GLboolean write_depth,
                                  GLboolean write_stencil)
{
   struct gl_context *ctx = st->ctx;
   struct gl_program *p;
   struct gl_fragment_program *fp;
   GLuint ic = 0;
   const GLuint shaderIndex = write_depth * 2 + write_stencil;

   assert(shaderIndex < Elements(st->drawpix.shaders));

   if (st->drawpix.shaders[shaderIndex]) {
      /* already have the proper shader */
      return st->drawpix.shaders[shaderIndex];
   }

   /*
    * Create shader now
    */
   p = ctx->Driver.NewProgram(ctx, GL_FRAGMENT_PROGRAM_ARB, 0);
   if (!p)
      return NULL;

   p->NumInstructions = write_depth ? 3 : 1;
   p->NumInstructions += write_stencil ? 1 : 0;

   p->Instructions = _mesa_alloc_instructions(p->NumInstructions);
   if (!p->Instructions) {
      ctx->Driver.DeleteProgram(ctx, p);
      return NULL;
   }
   _mesa_init_instructions(p->Instructions, p->NumInstructions);

   if (write_depth) {
      /* TEX result.depth, fragment.texcoord[0], texture[0], 2D; */
      p->Instructions[ic].Opcode = OPCODE_TEX;
      p->Instructions[ic].DstReg.File = PROGRAM_OUTPUT;
      p->Instructions[ic].DstReg.Index = FRAG_RESULT_DEPTH;
      p->Instructions[ic].DstReg.WriteMask = WRITEMASK_Z;
      p->Instructions[ic].SrcReg[0].File = PROGRAM_INPUT;
      p->Instructions[ic].SrcReg[0].Index = VARYING_SLOT_TEX0;
      p->Instructions[ic].TexSrcUnit = 0;
      p->Instructions[ic].TexSrcTarget = TEXTURE_2D_INDEX;
      ic++;
      /* MOV result.color, fragment.color; */
      p->Instructions[ic].Opcode = OPCODE_MOV;
      p->Instructions[ic].DstReg.File = PROGRAM_OUTPUT;
      p->Instructions[ic].DstReg.Index = FRAG_RESULT_COLOR;
      p->Instructions[ic].SrcReg[0].File = PROGRAM_INPUT;
      p->Instructions[ic].SrcReg[0].Index = VARYING_SLOT_COL0;
      ic++;
   }

   if (write_stencil) {
      /* TEX result.stencil, fragment.texcoord[0], texture[0], 2D; */
      p->Instructions[ic].Opcode = OPCODE_TEX;
      p->Instructions[ic].DstReg.File = PROGRAM_OUTPUT;
      p->Instructions[ic].DstReg.Index = FRAG_RESULT_STENCIL;
      p->Instructions[ic].DstReg.WriteMask = WRITEMASK_Y;
      p->Instructions[ic].SrcReg[0].File = PROGRAM_INPUT;
      p->Instructions[ic].SrcReg[0].Index = VARYING_SLOT_TEX0;
      p->Instructions[ic].TexSrcUnit = 1;
      p->Instructions[ic].TexSrcTarget = TEXTURE_2D_INDEX;
      ic++;
   }

   /* END; */
   p->Instructions[ic++].Opcode = OPCODE_END;

   assert(ic == p->NumInstructions);

   p->InputsRead = VARYING_BIT_TEX0 | VARYING_BIT_COL0;
   p->OutputsWritten = 0;
   if (write_depth) {
      p->OutputsWritten |= BITFIELD64_BIT(FRAG_RESULT_DEPTH);
      p->OutputsWritten |= BITFIELD64_BIT(FRAG_RESULT_COLOR);
   }
   if (write_stencil)
      p->OutputsWritten |= BITFIELD64_BIT(FRAG_RESULT_STENCIL);

   p->SamplersUsed =  0x1;  /* sampler 0 (bit 0) is used */
   if (write_stencil)
      p->SamplersUsed |= 1 << 1;

   fp = (struct gl_fragment_program *) p;

   /* save the new shader */
   st->drawpix.shaders[shaderIndex] = fp;

   return fp;
}


/**
 * Create a simple vertex shader that just passes through the
 * vertex position and texcoord (and optionally, color).
 */
static void *
make_passthrough_vertex_shader(struct st_context *st, 
                               GLboolean passColor)
{
   const unsigned texcoord_semantic = st->needs_texcoord_semantic ?
      TGSI_SEMANTIC_TEXCOORD : TGSI_SEMANTIC_GENERIC;

   if (!st->drawpix.vert_shaders[passColor]) {
      struct ureg_program *ureg = ureg_create( TGSI_PROCESSOR_VERTEX );

      if (ureg == NULL)
         return NULL;

      /* MOV result.pos, vertex.pos; */
      ureg_MOV(ureg, 
               ureg_DECL_output( ureg, TGSI_SEMANTIC_POSITION, 0 ),
               ureg_DECL_vs_input( ureg, 0 ));
      
      /* MOV result.texcoord0, vertex.attr[1]; */
      ureg_MOV(ureg, 
               ureg_DECL_output( ureg, texcoord_semantic, 0 ),
               ureg_DECL_vs_input( ureg, 1 ));
      
      if (passColor) {
         /* MOV result.color0, vertex.attr[2]; */
         ureg_MOV(ureg, 
                  ureg_DECL_output( ureg, TGSI_SEMANTIC_COLOR, 0 ),
                  ureg_DECL_vs_input( ureg, 2 ));
      }

      ureg_END( ureg );
      
      st->drawpix.vert_shaders[passColor] = 
         ureg_create_shader_and_destroy( ureg, st->pipe );
   }

   return st->drawpix.vert_shaders[passColor];
}


/**
 * Return a texture internalFormat for drawing/copying an image
 * of the given format and type.
 */
static GLenum
internal_format(struct gl_context *ctx, GLenum format, GLenum type)
{
   switch (format) {
   case GL_DEPTH_COMPONENT:
      switch (type) {
      case GL_UNSIGNED_SHORT:
         return GL_DEPTH_COMPONENT16;

      case GL_UNSIGNED_INT:
         return GL_DEPTH_COMPONENT32;

      case GL_FLOAT:
         if (ctx->Extensions.ARB_depth_buffer_float)
            return GL_DEPTH_COMPONENT32F;
         else
            return GL_DEPTH_COMPONENT;

      default:
         return GL_DEPTH_COMPONENT;
      }

   case GL_DEPTH_STENCIL:
      switch (type) {
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         return GL_DEPTH32F_STENCIL8;

      case GL_UNSIGNED_INT_24_8:
      default:
         return GL_DEPTH24_STENCIL8;
      }

   case GL_STENCIL_INDEX:
      return GL_STENCIL_INDEX;

   default:
      if (_mesa_is_enum_format_integer(format)) {
         switch (type) {
         case GL_BYTE:
            return GL_RGBA8I;
         case GL_UNSIGNED_BYTE:
            return GL_RGBA8UI;
         case GL_SHORT:
            return GL_RGBA16I;
         case GL_UNSIGNED_SHORT:
            return GL_RGBA16UI;
         case GL_INT:
            return GL_RGBA32I;
         case GL_UNSIGNED_INT:
            return GL_RGBA32UI;
         default:
            assert(0 && "Unexpected type in internal_format()");
            return GL_RGBA_INTEGER;
         }
      }
      else {
         switch (type) {
         case GL_UNSIGNED_BYTE:
         case GL_UNSIGNED_INT_8_8_8_8:
         case GL_UNSIGNED_INT_8_8_8_8_REV:
         default:
            return GL_RGBA8;

         case GL_UNSIGNED_BYTE_3_3_2:
         case GL_UNSIGNED_BYTE_2_3_3_REV:
            return GL_R3_G3_B2;

         case GL_UNSIGNED_SHORT_4_4_4_4:
         case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            return GL_RGBA4;

         case GL_UNSIGNED_SHORT_5_6_5:
         case GL_UNSIGNED_SHORT_5_6_5_REV:
            return GL_RGB565;

         case GL_UNSIGNED_SHORT_5_5_5_1:
         case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            return GL_RGB5_A1;

         case GL_UNSIGNED_INT_10_10_10_2:
         case GL_UNSIGNED_INT_2_10_10_10_REV:
            return GL_RGB10_A2;

         case GL_UNSIGNED_SHORT:
         case GL_UNSIGNED_INT:
            return GL_RGBA16;

         case GL_BYTE:
            return
               ctx->Extensions.EXT_texture_snorm ? GL_RGBA8_SNORM : GL_RGBA8;

         case GL_SHORT:
         case GL_INT:
            return
               ctx->Extensions.EXT_texture_snorm ? GL_RGBA16_SNORM : GL_RGBA16;

         case GL_HALF_FLOAT_ARB:
            return
               ctx->Extensions.ARB_texture_float ? GL_RGBA16F :
               ctx->Extensions.EXT_texture_snorm ? GL_RGBA16_SNORM : GL_RGBA16;

         case GL_FLOAT:
         case GL_DOUBLE:
            return
               ctx->Extensions.ARB_texture_float ? GL_RGBA32F :
               ctx->Extensions.EXT_texture_snorm ? GL_RGBA16_SNORM : GL_RGBA16;

         case GL_UNSIGNED_INT_5_9_9_9_REV:
            assert(ctx->Extensions.EXT_texture_shared_exponent);
            return GL_RGB9_E5;

         case GL_UNSIGNED_INT_10F_11F_11F_REV:
            assert(ctx->Extensions.EXT_packed_float);
            return GL_R11F_G11F_B10F;
         }
      }
   }
}


/**
 * Create a temporary texture to hold an image of the given size.
 * If width, height are not POT and the driver only handles POT textures,
 * allocate the next larger size of texture that is POT.
 */
static struct pipe_resource *
alloc_texture(struct st_context *st, GLsizei width, GLsizei height,
              enum pipe_format texFormat, unsigned bind)
{
   struct pipe_resource *pt;

   pt = st_texture_create(st, st->internal_target, texFormat, 0,
                          width, height, 1, 1, 0, bind);

   return pt;
}


/**
 * Make texture containing an image for glDrawPixels image.
 * If 'pixels' is NULL, leave the texture image data undefined.
 */
static struct pipe_resource *
make_texture(struct st_context *st,
	     GLsizei width, GLsizei height, GLenum format, GLenum type,
	     const struct gl_pixelstore_attrib *unpack,
	     const GLvoid *pixels)
{
   struct gl_context *ctx = st->ctx;
   struct pipe_context *pipe = st->pipe;
   mesa_format mformat;
   struct pipe_resource *pt;
   enum pipe_format pipeFormat;
   GLenum baseInternalFormat;

   /* Choose a pixel format for the temp texture which will hold the
    * image to draw.
    */
   pipeFormat = st_choose_matching_format(st, PIPE_BIND_SAMPLER_VIEW,
                                          format, type, unpack->SwapBytes);

   if (pipeFormat == PIPE_FORMAT_NONE) {
      /* Use the generic approach. */
      GLenum intFormat = internal_format(ctx, format, type);

      pipeFormat = st_choose_format(st, intFormat, format, type,
                                    PIPE_TEXTURE_2D, 0, PIPE_BIND_SAMPLER_VIEW,
                                    FALSE);
      assert(pipeFormat != PIPE_FORMAT_NONE);
   }

   mformat = st_pipe_format_to_mesa_format(pipeFormat);
   baseInternalFormat = _mesa_get_format_base_format(mformat);

   pixels = _mesa_map_pbo_source(ctx, unpack, pixels);
   if (!pixels)
      return NULL;

   /* alloc temporary texture */
   pt = alloc_texture(st, width, height, pipeFormat, PIPE_BIND_SAMPLER_VIEW);
   if (!pt) {
      _mesa_unmap_pbo_source(ctx, unpack);
      return NULL;
   }

   {
      struct pipe_transfer *transfer;
      GLboolean success;
      GLubyte *dest;
      const GLbitfield imageTransferStateSave = ctx->_ImageTransferState;

      /* we'll do pixel transfer in a fragment shader */
      ctx->_ImageTransferState = 0x0;

      /* map texture transfer */
      dest = pipe_transfer_map(pipe, pt, 0, 0,
                               PIPE_TRANSFER_WRITE, 0, 0,
                               width, height, &transfer);


      /* Put image into texture transfer.
       * Note that the image is actually going to be upside down in
       * the texture.  We deal with that with texcoords.
       */
      success = _mesa_texstore(ctx, 2,           /* dims */
                               baseInternalFormat, /* baseInternalFormat */
                               mformat,          /* mesa_format */
                               transfer->stride, /* dstRowStride, bytes */
                               &dest,            /* destSlices */
                               width, height, 1, /* size */
                               format, type,     /* src format/type */
                               pixels,           /* data source */
                               unpack);

      /* unmap */
      pipe_transfer_unmap(pipe, transfer);

      assert(success);

      /* restore */
      ctx->_ImageTransferState = imageTransferStateSave;
   }

   _mesa_unmap_pbo_source(ctx, unpack);

   return pt;
}


/**
 * Draw quad with texcoords and optional color.
 * Coords are gallium window coords with y=0=top.
 * \param color  may be null
 * \param invertTex  if true, flip texcoords vertically
 */
static void
draw_quad(struct gl_context *ctx, GLfloat x0, GLfloat y0, GLfloat z,
          GLfloat x1, GLfloat y1, const GLfloat *color,
          GLboolean invertTex, GLfloat maxXcoord, GLfloat maxYcoord)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   GLfloat (*verts)[3][4]; /* four verts, three attribs, XYZW */
   struct pipe_resource *buf = NULL;
   unsigned offset;

   if (u_upload_alloc(st->uploader, 0, 4 * sizeof(verts[0]), &offset,
                      &buf, (void **) &verts) != PIPE_OK) {
      return;
   }

   /* setup vertex data */
   {
      const struct gl_framebuffer *fb = st->ctx->DrawBuffer;
      const GLfloat fb_width = (GLfloat) fb->Width;
      const GLfloat fb_height = (GLfloat) fb->Height;
      const GLfloat clip_x0 = x0 / fb_width * 2.0f - 1.0f;
      const GLfloat clip_y0 = y0 / fb_height * 2.0f - 1.0f;
      const GLfloat clip_x1 = x1 / fb_width * 2.0f - 1.0f;
      const GLfloat clip_y1 = y1 / fb_height * 2.0f - 1.0f;
      const GLfloat sLeft = 0.0f, sRight = maxXcoord;
      const GLfloat tTop = invertTex ? maxYcoord : 0.0f;
      const GLfloat tBot = invertTex ? 0.0f : maxYcoord;
      GLuint i;

      /* upper-left */
      verts[0][0][0] = clip_x0;    /* v[0].attr[0].x */
      verts[0][0][1] = clip_y0;    /* v[0].attr[0].y */

      /* upper-right */
      verts[1][0][0] = clip_x1;
      verts[1][0][1] = clip_y0;

      /* lower-right */
      verts[2][0][0] = clip_x1;
      verts[2][0][1] = clip_y1;

      /* lower-left */
      verts[3][0][0] = clip_x0;
      verts[3][0][1] = clip_y1;

      verts[0][1][0] = sLeft; /* v[0].attr[1].S */
      verts[0][1][1] = tTop;  /* v[0].attr[1].T */
      verts[1][1][0] = sRight;
      verts[1][1][1] = tTop;
      verts[2][1][0] = sRight;
      verts[2][1][1] = tBot;
      verts[3][1][0] = sLeft;
      verts[3][1][1] = tBot;

      /* same for all verts: */
      if (color) {
         for (i = 0; i < 4; i++) {
            verts[i][0][2] = z;         /* v[i].attr[0].z */
            verts[i][0][3] = 1.0f;      /* v[i].attr[0].w */
            verts[i][2][0] = color[0];  /* v[i].attr[2].r */
            verts[i][2][1] = color[1];  /* v[i].attr[2].g */
            verts[i][2][2] = color[2];  /* v[i].attr[2].b */
            verts[i][2][3] = color[3];  /* v[i].attr[2].a */
            verts[i][1][2] = 0.0f;      /* v[i].attr[1].R */
            verts[i][1][3] = 1.0f;      /* v[i].attr[1].Q */
         }
      }
      else {
         for (i = 0; i < 4; i++) {
            verts[i][0][2] = z;    /*Z*/
            verts[i][0][3] = 1.0f; /*W*/
            verts[i][1][2] = 0.0f; /*R*/
            verts[i][1][3] = 1.0f; /*Q*/
         }
      }
   }

   u_upload_unmap(st->uploader);
   util_draw_vertex_buffer(pipe, st->cso_context, buf,
                           cso_get_aux_vertex_buffer_slot(st->cso_context),
                           offset,
			   PIPE_PRIM_QUADS,
			   4,  /* verts */
			   3); /* attribs/vert */
   pipe_resource_reference(&buf, NULL);
}



static void
draw_textured_quad(struct gl_context *ctx, GLint x, GLint y, GLfloat z,
                   GLsizei width, GLsizei height,
                   GLfloat zoomX, GLfloat zoomY,
                   struct pipe_sampler_view **sv,
                   int num_sampler_view,
                   void *driver_vp,
                   void *driver_fp,
                   const GLfloat *color,
                   GLboolean invertTex,
                   GLboolean write_depth, GLboolean write_stencil)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct cso_context *cso = st->cso_context;
   GLfloat x0, y0, x1, y1;
   GLsizei maxSize;
   boolean normalized = sv[0]->texture->target != PIPE_TEXTURE_RECT;

   /* limit checks */
   /* XXX if DrawPixels image is larger than max texture size, break
    * it up into chunks.
    */
   maxSize = 1 << (pipe->screen->get_param(pipe->screen,
                                        PIPE_CAP_MAX_TEXTURE_2D_LEVELS) - 1);
   assert(width <= maxSize);
   assert(height <= maxSize);

   cso_save_rasterizer(cso);
   cso_save_viewport(cso);
   cso_save_samplers(cso, PIPE_SHADER_FRAGMENT);
   cso_save_sampler_views(cso, PIPE_SHADER_FRAGMENT);
   cso_save_fragment_shader(cso);
   cso_save_stream_outputs(cso);
   cso_save_vertex_shader(cso);
   cso_save_geometry_shader(cso);
   cso_save_vertex_elements(cso);
   cso_save_aux_vertex_buffer_slot(cso);
   if (write_stencil) {
      cso_save_depth_stencil_alpha(cso);
      cso_save_blend(cso);
   }

   /* rasterizer state: just scissor */
   {
      struct pipe_rasterizer_state rasterizer;
      memset(&rasterizer, 0, sizeof(rasterizer));
      rasterizer.clamp_fragment_color = !st->clamp_frag_color_in_shader &&
                                        ctx->Color._ClampFragmentColor;
      rasterizer.half_pixel_center = 1;
      rasterizer.bottom_edge_rule = 1;
      rasterizer.depth_clip = !ctx->Transform.DepthClamp;
      rasterizer.scissor = ctx->Scissor.EnableFlags;
      cso_set_rasterizer(cso, &rasterizer);
   }

   if (write_stencil) {
      /* Stencil writing bypasses the normal fragment pipeline to
       * disable color writing and set stencil test to always pass.
       */
      struct pipe_depth_stencil_alpha_state dsa;
      struct pipe_blend_state blend;

      /* depth/stencil */
      memset(&dsa, 0, sizeof(dsa));
      dsa.stencil[0].enabled = 1;
      dsa.stencil[0].func = PIPE_FUNC_ALWAYS;
      dsa.stencil[0].writemask = ctx->Stencil.WriteMask[0] & 0xff;
      dsa.stencil[0].zpass_op = PIPE_STENCIL_OP_REPLACE;
      if (write_depth) {
         /* writing depth+stencil: depth test always passes */
         dsa.depth.enabled = 1;
         dsa.depth.writemask = ctx->Depth.Mask;
         dsa.depth.func = PIPE_FUNC_ALWAYS;
      }
      cso_set_depth_stencil_alpha(cso, &dsa);

      /* blend (colormask) */
      memset(&blend, 0, sizeof(blend));
      cso_set_blend(cso, &blend);
   }

   /* fragment shader state: TEX lookup program */
   cso_set_fragment_shader_handle(cso, driver_fp);

   /* vertex shader state: position + texcoord pass-through */
   cso_set_vertex_shader_handle(cso, driver_vp);

   /* geometry shader state: disabled */
   cso_set_geometry_shader_handle(cso, NULL);

   /* texture sampling state: */
   {
      struct pipe_sampler_state sampler;
      memset(&sampler, 0, sizeof(sampler));
      sampler.wrap_s = PIPE_TEX_WRAP_CLAMP;
      sampler.wrap_t = PIPE_TEX_WRAP_CLAMP;
      sampler.wrap_r = PIPE_TEX_WRAP_CLAMP;
      sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
      sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
      sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
      sampler.normalized_coords = normalized;

      cso_single_sampler(cso, PIPE_SHADER_FRAGMENT, 0, &sampler);
      if (num_sampler_view > 1) {
         cso_single_sampler(cso, PIPE_SHADER_FRAGMENT, 1, &sampler);
      }
      cso_single_sampler_done(cso, PIPE_SHADER_FRAGMENT);
   }

   /* viewport state: viewport matching window dims */
   {
      const float w = (float) ctx->DrawBuffer->Width;
      const float h = (float) ctx->DrawBuffer->Height;
      struct pipe_viewport_state vp;
      vp.scale[0] =  0.5f * w;
      vp.scale[1] = -0.5f * h;
      vp.scale[2] = 0.5f;
      vp.scale[3] = 1.0f;
      vp.translate[0] = 0.5f * w;
      vp.translate[1] = 0.5f * h;
      vp.translate[2] = 0.5f;
      vp.translate[3] = 0.0f;
      cso_set_viewport(cso, &vp);
   }

   cso_set_vertex_elements(cso, 3, st->velems_util_draw);
   cso_set_stream_outputs(st->cso_context, 0, NULL, NULL);

   /* texture state: */
   cso_set_sampler_views(cso, PIPE_SHADER_FRAGMENT, num_sampler_view, sv);

   /* Compute Gallium window coords (y=0=top) with pixel zoom.
    * Recall that these coords are transformed by the current
    * vertex shader and viewport transformation.
    */
   if (st_fb_orientation(ctx->DrawBuffer) == Y_0_BOTTOM) {
      y = ctx->DrawBuffer->Height - (int) (y + height * ctx->Pixel.ZoomY);
      invertTex = !invertTex;
   }

   x0 = (GLfloat) x;
   x1 = x + width * ctx->Pixel.ZoomX;
   y0 = (GLfloat) y;
   y1 = y + height * ctx->Pixel.ZoomY;

   /* convert Z from [0,1] to [-1,-1] to match viewport Z scale/bias */
   z = z * 2.0f - 1.0f;

   draw_quad(ctx, x0, y0, z, x1, y1, color, invertTex,
             normalized ? ((GLfloat) width / sv[0]->texture->width0) : (GLfloat)width,
             normalized ? ((GLfloat) height / sv[0]->texture->height0) : (GLfloat)height);

   /* restore state */
   cso_restore_rasterizer(cso);
   cso_restore_viewport(cso);
   cso_restore_samplers(cso, PIPE_SHADER_FRAGMENT);
   cso_restore_sampler_views(cso, PIPE_SHADER_FRAGMENT);
   cso_restore_fragment_shader(cso);
   cso_restore_vertex_shader(cso);
   cso_restore_geometry_shader(cso);
   cso_restore_vertex_elements(cso);
   cso_restore_aux_vertex_buffer_slot(cso);
   cso_restore_stream_outputs(cso);
   if (write_stencil) {
      cso_restore_depth_stencil_alpha(cso);
      cso_restore_blend(cso);
   }
}


/**
 * Software fallback to do glDrawPixels(GL_STENCIL_INDEX) when we
 * can't use a fragment shader to write stencil values.
 */
static void
draw_stencil_pixels(struct gl_context *ctx, GLint x, GLint y,
                    GLsizei width, GLsizei height, GLenum format, GLenum type,
                    const struct gl_pixelstore_attrib *unpack,
                    const GLvoid *pixels)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct st_renderbuffer *strb;
   enum pipe_transfer_usage usage;
   struct pipe_transfer *pt;
   const GLboolean zoom = ctx->Pixel.ZoomX != 1.0 || ctx->Pixel.ZoomY != 1.0;
   ubyte *stmap;
   struct gl_pixelstore_attrib clippedUnpack = *unpack;
   GLubyte *sValues;
   GLuint *zValues;

   if (!zoom) {
      if (!_mesa_clip_drawpixels(ctx, &x, &y, &width, &height,
                                 &clippedUnpack)) {
         /* totally clipped */
         return;
      }
   }

   strb = st_renderbuffer(ctx->DrawBuffer->
                          Attachment[BUFFER_STENCIL].Renderbuffer);

   if (st_fb_orientation(ctx->DrawBuffer) == Y_0_TOP) {
      y = ctx->DrawBuffer->Height - y - height;
   }

   if (format == GL_STENCIL_INDEX && 
       _mesa_is_format_packed_depth_stencil(strb->Base.Format)) {
      /* writing stencil to a combined depth+stencil buffer */
      usage = PIPE_TRANSFER_READ_WRITE;
   }
   else {
      usage = PIPE_TRANSFER_WRITE;
   }

   stmap = pipe_transfer_map(pipe, strb->texture,
                             strb->surface->u.tex.level,
                             strb->surface->u.tex.first_layer,
                             usage, x, y,
                             width, height, &pt);

   pixels = _mesa_map_pbo_source(ctx, &clippedUnpack, pixels);
   assert(pixels);

   sValues = malloc(width * sizeof(GLubyte));
   zValues = malloc(width * sizeof(GLuint));

   if (sValues && zValues) {
      GLint row;
      for (row = 0; row < height; row++) {
         GLfloat *zValuesFloat = (GLfloat*)zValues;
         GLenum destType = GL_UNSIGNED_BYTE;
         const GLvoid *source = _mesa_image_address2d(&clippedUnpack, pixels,
                                                      width, height,
                                                      format, type,
                                                      row, 0);
         _mesa_unpack_stencil_span(ctx, width, destType, sValues,
                                   type, source, &clippedUnpack,
                                   ctx->_ImageTransferState);

         if (format == GL_DEPTH_STENCIL) {
            GLenum ztype =
               pt->resource->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT ?
               GL_FLOAT : GL_UNSIGNED_INT;

            _mesa_unpack_depth_span(ctx, width, ztype, zValues,
                                    (1 << 24) - 1, type, source,
                                    &clippedUnpack);
         }

         if (zoom) {
            _mesa_problem(ctx, "Gallium glDrawPixels(GL_STENCIL) with "
                          "zoom not complete");
         }

         {
            GLint spanY;

            if (st_fb_orientation(ctx->DrawBuffer) == Y_0_TOP) {
               spanY = height - row - 1;
            }
            else {
               spanY = row;
            }

            /* now pack the stencil (and Z) values in the dest format */
            switch (pt->resource->format) {
            case PIPE_FORMAT_S8_UINT:
               {
                  ubyte *dest = stmap + spanY * pt->stride;
                  assert(usage == PIPE_TRANSFER_WRITE);
                  memcpy(dest, sValues, width);
               }
               break;
            case PIPE_FORMAT_Z24_UNORM_S8_UINT:
               if (format == GL_DEPTH_STENCIL) {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLint k;
                  assert(usage == PIPE_TRANSFER_WRITE);
                  for (k = 0; k < width; k++) {
                     dest[k] = zValues[k] | (sValues[k] << 24);
                  }
               }
               else {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLint k;
                  assert(usage == PIPE_TRANSFER_READ_WRITE);
                  for (k = 0; k < width; k++) {
                     dest[k] = (dest[k] & 0xffffff) | (sValues[k] << 24);
                  }
               }
               break;
            case PIPE_FORMAT_S8_UINT_Z24_UNORM:
               if (format == GL_DEPTH_STENCIL) {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLint k;
                  assert(usage == PIPE_TRANSFER_WRITE);
                  for (k = 0; k < width; k++) {
                     dest[k] = (zValues[k] << 8) | (sValues[k] & 0xff);
                  }
               }
               else {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLint k;
                  assert(usage == PIPE_TRANSFER_READ_WRITE);
                  for (k = 0; k < width; k++) {
                     dest[k] = (dest[k] & 0xffffff00) | (sValues[k] & 0xff);
                  }
               }
               break;
            case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
               if (format == GL_DEPTH_STENCIL) {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLfloat *destf = (GLfloat*)dest;
                  GLint k;
                  assert(usage == PIPE_TRANSFER_WRITE);
                  for (k = 0; k < width; k++) {
                     destf[k*2] = zValuesFloat[k];
                     dest[k*2+1] = sValues[k] & 0xff;
                  }
               }
               else {
                  uint *dest = (uint *) (stmap + spanY * pt->stride);
                  GLint k;
                  assert(usage == PIPE_TRANSFER_READ_WRITE);
                  for (k = 0; k < width; k++) {
                     dest[k*2+1] = sValues[k] & 0xff;
                  }
               }
               break;
            default:
               assert(0);
            }
         }
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glDrawPixels()");
   }

   free(sValues);
   free(zValues);

   _mesa_unmap_pbo_source(ctx, &clippedUnpack);

   /* unmap the stencil buffer */
   pipe_transfer_unmap(pipe, pt);
}


/**
 * Get fragment program variant for a glDrawPixels or glCopyPixels
 * command for RGBA data.
 */
static struct st_fp_variant *
get_color_fp_variant(struct st_context *st)
{
   struct gl_context *ctx = st->ctx;
   struct st_fp_variant_key key;
   struct st_fp_variant *fpv;

   memset(&key, 0, sizeof(key));

   key.st = st;
   key.drawpixels = 1;
   key.scaleAndBias = (ctx->Pixel.RedBias != 0.0 ||
                       ctx->Pixel.RedScale != 1.0 ||
                       ctx->Pixel.GreenBias != 0.0 ||
                       ctx->Pixel.GreenScale != 1.0 ||
                       ctx->Pixel.BlueBias != 0.0 ||
                       ctx->Pixel.BlueScale != 1.0 ||
                       ctx->Pixel.AlphaBias != 0.0 ||
                       ctx->Pixel.AlphaScale != 1.0);
   key.pixelMaps = ctx->Pixel.MapColorFlag;
   key.clamp_color = st->clamp_frag_color_in_shader &&
                     st->ctx->Color._ClampFragmentColor;

   fpv = st_get_fp_variant(st, st->fp, &key);

   return fpv;
}


/**
 * Get fragment program variant for a glDrawPixels or glCopyPixels
 * command for depth/stencil data.
 */
static struct st_fp_variant *
get_depth_stencil_fp_variant(struct st_context *st, GLboolean write_depth,
                             GLboolean write_stencil)
{
   struct st_fp_variant_key key;
   struct st_fp_variant *fpv;

   memset(&key, 0, sizeof(key));

   key.st = st;
   key.drawpixels = 1;
   key.drawpixels_z = write_depth;
   key.drawpixels_stencil = write_stencil;

   fpv = st_get_fp_variant(st, st->fp, &key);

   return fpv;
}


/**
 * Clamp glDrawPixels width and height to the maximum texture size.
 */
static void
clamp_size(struct pipe_context *pipe, GLsizei *width, GLsizei *height,
           struct gl_pixelstore_attrib *unpack)
{
   const int maxSize =
      1 << (pipe->screen->get_param(pipe->screen,
                                    PIPE_CAP_MAX_TEXTURE_2D_LEVELS) - 1);

   if (*width > maxSize) {
      if (unpack->RowLength == 0)
         unpack->RowLength = *width;
      *width = maxSize;
   }
   if (*height > maxSize) {
      *height = maxSize;
   }
}


/**
 * Called via ctx->Driver.DrawPixels()
 */
static void
st_DrawPixels(struct gl_context *ctx, GLint x, GLint y,
              GLsizei width, GLsizei height,
              GLenum format, GLenum type,
              const struct gl_pixelstore_attrib *unpack, const GLvoid *pixels)
{
   void *driver_vp, *driver_fp;
   struct st_context *st = st_context(ctx);
   const GLfloat *color;
   struct pipe_context *pipe = st->pipe;
   GLboolean write_stencil = GL_FALSE, write_depth = GL_FALSE;
   struct pipe_sampler_view *sv[2];
   int num_sampler_view = 1;
   struct st_fp_variant *fpv;
   struct gl_pixelstore_attrib clippedUnpack;

   /* Mesa state should be up to date by now */
   assert(ctx->NewState == 0x0);

   st_validate_state(st);

   /* Limit the size of the glDrawPixels to the max texture size.
    * Strictly speaking, that's not correct but since we don't handle
    * larger images yet, this is better than crashing.
    */
   clippedUnpack = *unpack;
   unpack = &clippedUnpack;
   clamp_size(st->pipe, &width, &height, &clippedUnpack);

   if (format == GL_DEPTH_STENCIL)
      write_stencil = write_depth = GL_TRUE;
   else if (format == GL_STENCIL_INDEX)
      write_stencil = GL_TRUE;
   else if (format == GL_DEPTH_COMPONENT)
      write_depth = GL_TRUE;

   if (write_stencil &&
       !pipe->screen->get_param(pipe->screen, PIPE_CAP_SHADER_STENCIL_EXPORT)) {
      /* software fallback */
      draw_stencil_pixels(ctx, x, y, width, height, format, type,
                          unpack, pixels);
      return;
   }

   /*
    * Get vertex/fragment shaders
    */
   if (write_depth || write_stencil) {
      fpv = get_depth_stencil_fp_variant(st, write_depth, write_stencil);

      driver_fp = fpv->driver_shader;

      driver_vp = make_passthrough_vertex_shader(st, GL_TRUE);

      color = ctx->Current.RasterColor;
   }
   else {
      fpv = get_color_fp_variant(st);

      driver_fp = fpv->driver_shader;

      driver_vp = make_passthrough_vertex_shader(st, GL_FALSE);

      color = NULL;
      if (st->pixel_xfer.pixelmap_enabled) {
	  sv[1] = st->pixel_xfer.pixelmap_sampler_view;
	  num_sampler_view++;
      }
   }

   /* update fragment program constants */
   st_upload_constants(st, fpv->parameters, PIPE_SHADER_FRAGMENT);

   /* draw with textured quad */
   {
      struct pipe_resource *pt
         = make_texture(st, width, height, format, type, unpack, pixels);
      if (pt) {
         sv[0] = st_create_texture_sampler_view(st->pipe, pt);

         if (sv[0]) {
            /* Create a second sampler view to read stencil.
             * The stencil is written using the shader stencil export
             * functionality. */
            if (write_stencil) {
               enum pipe_format stencil_format =
                     util_format_stencil_only(pt->format);

               sv[1] = st_create_texture_sampler_view_format(st->pipe, pt,
                                                             stencil_format);
               num_sampler_view++;
            }

            draw_textured_quad(ctx, x, y, ctx->Current.RasterPos[2],
                               width, height,
                               ctx->Pixel.ZoomX, ctx->Pixel.ZoomY,
                               sv,
                               num_sampler_view,
                               driver_vp,
                               driver_fp,
                               color, GL_FALSE, write_depth, write_stencil);
            pipe_sampler_view_reference(&sv[0], NULL);
            if (num_sampler_view > 1)
               pipe_sampler_view_reference(&sv[1], NULL);
         }
         pipe_resource_reference(&pt, NULL);
      }
   }
}



/**
 * Software fallback for glCopyPixels(GL_STENCIL).
 */
static void
copy_stencil_pixels(struct gl_context *ctx, GLint srcx, GLint srcy,
                    GLsizei width, GLsizei height,
                    GLint dstx, GLint dsty)
{
   struct st_renderbuffer *rbDraw;
   struct pipe_context *pipe = st_context(ctx)->pipe;
   enum pipe_transfer_usage usage;
   struct pipe_transfer *ptDraw;
   ubyte *drawMap;
   ubyte *buffer;
   int i;

   buffer = malloc(width * height * sizeof(ubyte));
   if (!buffer) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCopyPixels(stencil)");
      return;
   }

   /* Get the dest renderbuffer */
   rbDraw = st_renderbuffer(ctx->DrawBuffer->
                            Attachment[BUFFER_STENCIL].Renderbuffer);

   /* this will do stencil pixel transfer ops */
   _mesa_readpixels(ctx, srcx, srcy, width, height,
                    GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                    &ctx->DefaultPacking, buffer);

   if (0) {
      /* debug code: dump stencil values */
      GLint row, col;
      for (row = 0; row < height; row++) {
         printf("%3d: ", row);
         for (col = 0; col < width; col++) {
            printf("%02x ", buffer[col + row * width]);
         }
         printf("\n");
      }
   }

   if (_mesa_is_format_packed_depth_stencil(rbDraw->Base.Format))
      usage = PIPE_TRANSFER_READ_WRITE;
   else
      usage = PIPE_TRANSFER_WRITE;

   if (st_fb_orientation(ctx->DrawBuffer) == Y_0_TOP) {
      dsty = rbDraw->Base.Height - dsty - height;
   }

   assert(util_format_get_blockwidth(rbDraw->texture->format) == 1);
   assert(util_format_get_blockheight(rbDraw->texture->format) == 1);

   /* map the stencil buffer */
   drawMap = pipe_transfer_map(pipe,
                               rbDraw->texture,
                               rbDraw->surface->u.tex.level,
                               rbDraw->surface->u.tex.first_layer,
                               usage, dstx, dsty,
                               width, height, &ptDraw);

   /* draw */
   /* XXX PixelZoom not handled yet */
   for (i = 0; i < height; i++) {
      ubyte *dst;
      const ubyte *src;
      int y;

      y = i;

      if (st_fb_orientation(ctx->DrawBuffer) == Y_0_TOP) {
         y = height - y - 1;
      }

      dst = drawMap + y * ptDraw->stride;
      src = buffer + i * width;

      _mesa_pack_ubyte_stencil_row(rbDraw->Base.Format, width, src, dst);
   }

   free(buffer);

   /* unmap the stencil buffer */
   pipe_transfer_unmap(pipe, ptDraw);
}


/**
 * Return renderbuffer to use for reading color pixels for glCopyPixels
 */
static struct st_renderbuffer *
st_get_color_read_renderbuffer(struct gl_context *ctx)
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct st_renderbuffer *strb =
      st_renderbuffer(fb->_ColorReadBuffer);

   return strb;
}


/**
 * \return TRUE if two regions overlap, FALSE otherwise
 */
static boolean
regions_overlap(int srcX0, int srcY0,
                int srcX1, int srcY1,
                int dstX0, int dstY0,
                int dstX1, int dstY1)
{
   if (MAX2(srcX0, srcX1) < MIN2(dstX0, dstX1))
      return FALSE; /* src completely left of dst */

   if (MAX2(dstX0, dstX1) < MIN2(srcX0, srcX1))
      return FALSE; /* dst completely left of src */

   if (MAX2(srcY0, srcY1) < MIN2(dstY0, dstY1))
      return FALSE; /* src completely above dst */

   if (MAX2(dstY0, dstY1) < MIN2(srcY0, srcY1))
      return FALSE; /* dst completely above src */

   return TRUE; /* some overlap */
}


/**
 * Try to do a glCopyPixels for simple cases with a blit by calling
 * pipe->blit().
 *
 * We can do this when we're copying color pixels (depth/stencil
 * eventually) with no pixel zoom, no pixel transfer ops, no
 * per-fragment ops, and the src/dest regions don't overlap.
 */
static GLboolean
blit_copy_pixels(struct gl_context *ctx, GLint srcx, GLint srcy,
                 GLsizei width, GLsizei height,
                 GLint dstx, GLint dsty, GLenum type)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct pipe_screen *screen = pipe->screen;
   struct gl_pixelstore_attrib pack, unpack;
   GLint readX, readY, readW, readH, drawX, drawY, drawW, drawH;

   if (type == GL_COLOR &&
       ctx->Pixel.ZoomX == 1.0 &&
       ctx->Pixel.ZoomY == 1.0 &&
       ctx->_ImageTransferState == 0x0 &&
       !ctx->Color.BlendEnabled &&
       !ctx->Color.AlphaEnabled &&
       !ctx->Depth.Test &&
       !ctx->Fog.Enabled &&
       !ctx->Stencil.Enabled &&
       !ctx->FragmentProgram.Enabled &&
       !ctx->VertexProgram.Enabled &&
       !ctx->_Shader->CurrentProgram[MESA_SHADER_FRAGMENT] &&
       ctx->DrawBuffer->_NumColorDrawBuffers == 1 &&
       !ctx->Query.CondRenderQuery &&
       !ctx->Query.CurrentOcclusionObject) {
      struct st_renderbuffer *rbRead, *rbDraw;

      /*
       * Clip the read region against the src buffer bounds.
       * We'll still allocate a temporary buffer/texture for the original
       * src region size but we'll only read the region which is on-screen.
       * This may mean that we draw garbage pixels into the dest region, but
       * that's expected.
       */
      readX = srcx;
      readY = srcy;
      readW = width;
      readH = height;
      pack = ctx->DefaultPacking;
      if (!_mesa_clip_readpixels(ctx, &readX, &readY, &readW, &readH, &pack))
         return GL_TRUE; /* all done */

      /* clip against dest buffer bounds and scissor box */
      drawX = dstx + pack.SkipPixels;
      drawY = dsty + pack.SkipRows;
      unpack = pack;
      if (!_mesa_clip_drawpixels(ctx, &drawX, &drawY, &readW, &readH, &unpack))
         return GL_TRUE; /* all done */

      readX = readX - pack.SkipPixels + unpack.SkipPixels;
      readY = readY - pack.SkipRows + unpack.SkipRows;

      drawW = readW;
      drawH = readH;

      rbRead = st_get_color_read_renderbuffer(ctx);
      rbDraw = st_renderbuffer(ctx->DrawBuffer->_ColorDrawBuffers[0]);

      /* Flip src/dst position depending on the orientation of buffers. */
      if (st_fb_orientation(ctx->ReadBuffer) == Y_0_TOP) {
         readY = rbRead->Base.Height - readY;
         readH = -readH;
      }

      if (st_fb_orientation(ctx->DrawBuffer) == Y_0_TOP) {
         /* We can't flip the destination for pipe->blit, so we only adjust
          * its position and flip the source.
          */
         drawY = rbDraw->Base.Height - drawY - drawH;
         readY += readH;
         readH = -readH;
      }

      if (rbRead != rbDraw ||
          !regions_overlap(readX, readY, readX + readW, readY + readH,
                           drawX, drawY, drawX + drawW, drawY + drawH)) {
         struct pipe_blit_info blit;

         memset(&blit, 0, sizeof(blit));
         blit.src.resource = rbRead->texture;
         blit.src.level = rbRead->surface->u.tex.level;
         blit.src.format = rbRead->texture->format;
         blit.src.box.x = readX;
         blit.src.box.y = readY;
         blit.src.box.z = rbRead->surface->u.tex.first_layer;
         blit.src.box.width = readW;
         blit.src.box.height = readH;
         blit.src.box.depth = 1;
         blit.dst.resource = rbDraw->texture;
         blit.dst.level = rbDraw->surface->u.tex.level;
         blit.dst.format = rbDraw->texture->format;
         blit.dst.box.x = drawX;
         blit.dst.box.y = drawY;
         blit.dst.box.z = rbDraw->surface->u.tex.first_layer;
         blit.dst.box.width = drawW;
         blit.dst.box.height = drawH;
         blit.dst.box.depth = 1;
         blit.mask = PIPE_MASK_RGBA;
         blit.filter = PIPE_TEX_FILTER_NEAREST;

         if (screen->is_format_supported(screen, blit.src.format,
                                         blit.src.resource->target,
                                         blit.src.resource->nr_samples,
                                         PIPE_BIND_SAMPLER_VIEW) &&
             screen->is_format_supported(screen, blit.dst.format,
                                         blit.dst.resource->target,
                                         blit.dst.resource->nr_samples,
                                         PIPE_BIND_RENDER_TARGET)) {
            pipe->blit(pipe, &blit);
            return GL_TRUE;
         }
      }
   }

   return GL_FALSE;
}


static void
st_CopyPixels(struct gl_context *ctx, GLint srcx, GLint srcy,
              GLsizei width, GLsizei height,
              GLint dstx, GLint dsty, GLenum type)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct pipe_screen *screen = pipe->screen;
   struct st_renderbuffer *rbRead;
   void *driver_vp, *driver_fp;
   struct pipe_resource *pt;
   struct pipe_sampler_view *sv[2];
   int num_sampler_view = 1;
   GLfloat *color;
   enum pipe_format srcFormat;
   unsigned srcBind;
   GLboolean invertTex = GL_FALSE;
   GLint readX, readY, readW, readH;
   struct gl_pixelstore_attrib pack = ctx->DefaultPacking;
   struct st_fp_variant *fpv;

   st_validate_state(st);

   if (type == GL_DEPTH_STENCIL) {
      /* XXX make this more efficient */
      st_CopyPixels(ctx, srcx, srcy, width, height, dstx, dsty, GL_STENCIL);
      st_CopyPixels(ctx, srcx, srcy, width, height, dstx, dsty, GL_DEPTH);
      return;
   }

   if (type == GL_STENCIL) {
      /* can't use texturing to do stencil */
      copy_stencil_pixels(ctx, srcx, srcy, width, height, dstx, dsty);
      return;
   }

   if (blit_copy_pixels(ctx, srcx, srcy, width, height, dstx, dsty, type))
      return;

   /*
    * The subsequent code implements glCopyPixels by copying the source
    * pixels into a temporary texture that's then applied to a textured quad.
    * When we draw the textured quad, all the usual per-fragment operations
    * are handled.
    */


   /*
    * Get vertex/fragment shaders
    */
   if (type == GL_COLOR) {
      rbRead = st_get_color_read_renderbuffer(ctx);
      color = NULL;

      fpv = get_color_fp_variant(st);
      driver_fp = fpv->driver_shader;

      driver_vp = make_passthrough_vertex_shader(st, GL_FALSE);

      if (st->pixel_xfer.pixelmap_enabled) {
         sv[1] = st->pixel_xfer.pixelmap_sampler_view;
         num_sampler_view++;
      }
   }
   else {
      assert(type == GL_DEPTH);
      rbRead = st_renderbuffer(ctx->ReadBuffer->
                               Attachment[BUFFER_DEPTH].Renderbuffer);
      color = ctx->Current.Attrib[VERT_ATTRIB_COLOR0];

      fpv = get_depth_stencil_fp_variant(st, GL_TRUE, GL_FALSE);
      driver_fp = fpv->driver_shader;

      driver_vp = make_passthrough_vertex_shader(st, GL_TRUE);
   }

   /* update fragment program constants */
   st_upload_constants(st, fpv->parameters, PIPE_SHADER_FRAGMENT);

   /* Choose the format for the temporary texture. */
   srcFormat = rbRead->texture->format;
   srcBind = PIPE_BIND_SAMPLER_VIEW |
      (type == GL_COLOR ? PIPE_BIND_RENDER_TARGET : PIPE_BIND_DEPTH_STENCIL);

   if (!screen->is_format_supported(screen, srcFormat, st->internal_target, 0,
                                    srcBind)) {
      /* srcFormat is non-renderable. Find a compatible renderable format. */
      if (type == GL_DEPTH) {
         srcFormat = st_choose_format(st, GL_DEPTH_COMPONENT, GL_NONE,
                                      GL_NONE, st->internal_target, 0,
                                      srcBind, FALSE);
      }
      else {
         assert(type == GL_COLOR);

         if (util_format_is_float(srcFormat)) {
            srcFormat = st_choose_format(st, GL_RGBA32F, GL_NONE,
                                         GL_NONE, st->internal_target, 0,
                                         srcBind, FALSE);
         }
         else if (util_format_is_pure_sint(srcFormat)) {
            srcFormat = st_choose_format(st, GL_RGBA32I, GL_NONE,
                                         GL_NONE, st->internal_target, 0,
                                         srcBind, FALSE);
         }
         else if (util_format_is_pure_uint(srcFormat)) {
            srcFormat = st_choose_format(st, GL_RGBA32UI, GL_NONE,
                                         GL_NONE, st->internal_target, 0,
                                         srcBind, FALSE);
         }
         else if (util_format_is_snorm(srcFormat)) {
            srcFormat = st_choose_format(st, GL_RGBA16_SNORM, GL_NONE,
                                         GL_NONE, st->internal_target, 0,
                                         srcBind, FALSE);
         }
         else {
            srcFormat = st_choose_format(st, GL_RGBA, GL_NONE,
                                         GL_NONE, st->internal_target, 0,
                                         srcBind, FALSE);
         }
      }

      if (srcFormat == PIPE_FORMAT_NONE) {
         assert(0 && "cannot choose a format for src of CopyPixels");
         return;
      }
   }

   /* Invert src region if needed */
   if (st_fb_orientation(ctx->ReadBuffer) == Y_0_TOP) {
      srcy = ctx->ReadBuffer->Height - srcy - height;
      invertTex = !invertTex;
   }

   /* Clip the read region against the src buffer bounds.
    * We'll still allocate a temporary buffer/texture for the original
    * src region size but we'll only read the region which is on-screen.
    * This may mean that we draw garbage pixels into the dest region, but
    * that's expected.
    */
   readX = srcx;
   readY = srcy;
   readW = width;
   readH = height;
   if (!_mesa_clip_readpixels(ctx, &readX, &readY, &readW, &readH, &pack)) {
      /* The source region is completely out of bounds.  Do nothing.
       * The GL spec says "Results of copies from outside the window,
       * or from regions of the window that are not exposed, are
       * hardware dependent and undefined."
       */
      return;
   }

   readW = MAX2(0, readW);
   readH = MAX2(0, readH);

   /* Allocate the temporary texture. */
   pt = alloc_texture(st, width, height, srcFormat, srcBind);
   if (!pt)
      return;

   sv[0] = st_create_texture_sampler_view(st->pipe, pt);
   if (!sv[0]) {
      pipe_resource_reference(&pt, NULL);
      return;
   }

   /* Copy the src region to the temporary texture. */
   {
      struct pipe_blit_info blit;

      memset(&blit, 0, sizeof(blit));
      blit.src.resource = rbRead->texture;
      blit.src.level = rbRead->surface->u.tex.level;
      blit.src.format = rbRead->texture->format;
      blit.src.box.x = readX;
      blit.src.box.y = readY;
      blit.src.box.z = rbRead->surface->u.tex.first_layer;
      blit.src.box.width = readW;
      blit.src.box.height = readH;
      blit.src.box.depth = 1;
      blit.dst.resource = pt;
      blit.dst.level = 0;
      blit.dst.format = pt->format;
      blit.dst.box.x = pack.SkipPixels;
      blit.dst.box.y = pack.SkipRows;
      blit.dst.box.z = 0;
      blit.dst.box.width = readW;
      blit.dst.box.height = readH;
      blit.dst.box.depth = 1;
      blit.mask = util_format_get_mask(pt->format) & ~PIPE_MASK_S;
      blit.filter = PIPE_TEX_FILTER_NEAREST;

      pipe->blit(pipe, &blit);
   }

   /* OK, the texture 'pt' contains the src image/pixels.  Now draw a
    * textured quad with that texture.
    */
   draw_textured_quad(ctx, dstx, dsty, ctx->Current.RasterPos[2],
                      width, height, ctx->Pixel.ZoomX, ctx->Pixel.ZoomY,
                      sv,
                      num_sampler_view,
                      driver_vp, 
                      driver_fp,
                      color, invertTex, GL_FALSE, GL_FALSE);

   pipe_resource_reference(&pt, NULL);
   pipe_sampler_view_reference(&sv[0], NULL);
}



void st_init_drawpixels_functions(struct dd_function_table *functions)
{
   functions->DrawPixels = st_DrawPixels;
   functions->CopyPixels = st_CopyPixels;
}


void
st_destroy_drawpix(struct st_context *st)
{
   GLuint i;

   for (i = 0; i < Elements(st->drawpix.shaders); i++) {
      if (st->drawpix.shaders[i])
         _mesa_reference_fragprog(st->ctx, &st->drawpix.shaders[i], NULL);
   }

   st_reference_fragprog(st, &st->pixel_xfer.combined_prog, NULL);
   if (st->drawpix.vert_shaders[0])
      cso_delete_vertex_shader(st->cso_context, st->drawpix.vert_shaders[0]);
   if (st->drawpix.vert_shaders[1])
      cso_delete_vertex_shader(st->cso_context, st->drawpix.vert_shaders[1]);
}
