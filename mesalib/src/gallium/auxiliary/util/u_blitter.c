/**************************************************************************
 *
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Blitter utility to facilitate acceleration of the clear, clear_render_target,
 * clear_depth_stencil, resource_copy_region, and blit functions.
 *
 * @author Marek Olšák
 */

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_state.h"

#include "util/u_format.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_blitter.h"
#include "util/u_draw_quad.h"
#include "util/u_sampler.h"
#include "util/u_simple_shaders.h"
#include "util/u_surface.h"
#include "util/u_texture.h"
#include "util/u_upload_mgr.h"

#define INVALID_PTR ((void*)~0)

struct blitter_context_priv
{
   struct blitter_context base;

   struct u_upload_mgr *upload;

   float vertices[4][2][4];   /**< {pos, color} or {pos, texcoord} */

   /* Templates for various state objects. */

   /* Constant state objects. */
   /* Vertex shaders. */
   void *vs; /**< Vertex shader which passes {pos, generic} to the output.*/
   void *vs_pos_only; /**< Vertex shader which passes pos to the output.*/

   /* Fragment shaders. */
   void *fs_empty;
   void *fs_write_one_cbuf;
   void *fs_write_all_cbufs;

   /* FS which outputs a color from a texture,
      where the index is PIPE_TEXTURE_* to be sampled. */
   void *fs_texfetch_col[PIPE_MAX_TEXTURE_TYPES];

   /* FS which outputs a depth from a texture,
      where the index is PIPE_TEXTURE_* to be sampled. */
   void *fs_texfetch_depth[PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_depthstencil[PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_stencil[PIPE_MAX_TEXTURE_TYPES];

   /* FS which outputs one sample from a multisample texture. */
   void *fs_texfetch_col_msaa[PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_depth_msaa[PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_depthstencil_msaa[PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_stencil_msaa[PIPE_MAX_TEXTURE_TYPES];

   /* Blend state. */
   void *blend[PIPE_MASK_RGBA+1]; /**< blend state with writemask */

   /* Depth stencil alpha state. */
   void *dsa_write_depth_stencil;
   void *dsa_write_depth_keep_stencil;
   void *dsa_keep_depth_stencil;
   void *dsa_keep_depth_write_stencil;

   /* Vertex elements states. */
   void *velem_state;
   void *velem_state_readbuf[4]; /**< X, XY, XYZ, XYZW */

   /* Sampler state. */
   void *sampler_state;
   void *sampler_state_linear;
   void *sampler_state_rect;
   void *sampler_state_rect_linear;

   /* Rasterizer state. */
   void *rs_state, *rs_state_scissor, *rs_discard_state;

   /* Viewport state. */
   struct pipe_viewport_state viewport;

   /* Destination surface dimensions. */
   unsigned dst_width;
   unsigned dst_height;

   boolean has_geometry_shader;
   boolean has_stream_out;
   boolean has_stencil_export;
   boolean has_texture_multisample;

   /* The Draw module overrides these functions.
    * Always create the blitter before Draw. */
   void   (*bind_fs_state)(struct pipe_context *, void *);
   void   (*delete_fs_state)(struct pipe_context *, void *);
};

static struct pipe_surface *
util_blitter_get_next_surface_layer(struct pipe_context *pipe,
                                    struct pipe_surface *surf);

struct blitter_context *util_blitter_create(struct pipe_context *pipe)
{
   struct blitter_context_priv *ctx;
   struct pipe_blend_state blend;
   struct pipe_depth_stencil_alpha_state dsa;
   struct pipe_rasterizer_state rs_state;
   struct pipe_sampler_state sampler_state;
   struct pipe_vertex_element velem[2];
   unsigned i;

   ctx = CALLOC_STRUCT(blitter_context_priv);
   if (!ctx)
      return NULL;

   ctx->base.pipe = pipe;
   ctx->base.draw_rectangle = util_blitter_draw_rectangle;
   ctx->base.get_next_surface_layer = util_blitter_get_next_surface_layer;

   ctx->bind_fs_state = pipe->bind_fs_state;
   ctx->delete_fs_state = pipe->delete_fs_state;

   /* init state objects for them to be considered invalid */
   ctx->base.saved_blend_state = INVALID_PTR;
   ctx->base.saved_dsa_state = INVALID_PTR;
   ctx->base.saved_rs_state = INVALID_PTR;
   ctx->base.saved_fs = INVALID_PTR;
   ctx->base.saved_vs = INVALID_PTR;
   ctx->base.saved_gs = INVALID_PTR;
   ctx->base.saved_velem_state = INVALID_PTR;
   ctx->base.saved_fb_state.nr_cbufs = ~0;
   ctx->base.saved_num_sampler_views = ~0;
   ctx->base.saved_num_sampler_states = ~0;
   ctx->base.saved_num_so_targets = ~0;

   ctx->has_geometry_shader =
      pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_GEOMETRY,
                                     PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0;
   ctx->has_stream_out =
      pipe->screen->get_param(pipe->screen,
                              PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS) != 0;

   ctx->has_stencil_export =
         pipe->screen->get_param(pipe->screen,
                                 PIPE_CAP_SHADER_STENCIL_EXPORT);

   ctx->has_texture_multisample =
      pipe->screen->get_param(pipe->screen, PIPE_CAP_TEXTURE_MULTISAMPLE);

   /* blend state objects */
   memset(&blend, 0, sizeof(blend));

   for (i = 0; i <= PIPE_MASK_RGBA; i++) {
      blend.rt[0].colormask = i;
      ctx->blend[i] = pipe->create_blend_state(pipe, &blend);
   }

   /* depth stencil alpha state objects */
   memset(&dsa, 0, sizeof(dsa));
   ctx->dsa_keep_depth_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.depth.enabled = 1;
   dsa.depth.writemask = 1;
   dsa.depth.func = PIPE_FUNC_ALWAYS;
   ctx->dsa_write_depth_keep_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.stencil[0].enabled = 1;
   dsa.stencil[0].func = PIPE_FUNC_ALWAYS;
   dsa.stencil[0].fail_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].zpass_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].zfail_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].valuemask = 0xff;
   dsa.stencil[0].writemask = 0xff;
   ctx->dsa_write_depth_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.depth.enabled = 0;
   dsa.depth.writemask = 0;
   ctx->dsa_keep_depth_write_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   /* sampler state */
   memset(&sampler_state, 0, sizeof(sampler_state));
   sampler_state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.normalized_coords = 1;
   ctx->sampler_state = pipe->create_sampler_state(pipe, &sampler_state);
   sampler_state.normalized_coords = 0;
   ctx->sampler_state_rect = pipe->create_sampler_state(pipe, &sampler_state);

   sampler_state.min_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler_state.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler_state.normalized_coords = 1;
   ctx->sampler_state_linear = pipe->create_sampler_state(pipe, &sampler_state);
   sampler_state.normalized_coords = 0;
   ctx->sampler_state_rect_linear = pipe->create_sampler_state(pipe, &sampler_state);

   /* rasterizer state */
   memset(&rs_state, 0, sizeof(rs_state));
   rs_state.cull_face = PIPE_FACE_NONE;
   rs_state.half_pixel_center = 1;
   rs_state.bottom_edge_rule = 1;
   rs_state.flatshade = 1;
   rs_state.depth_clip = 1;
   ctx->rs_state = pipe->create_rasterizer_state(pipe, &rs_state);

   rs_state.scissor = 1;
   ctx->rs_state_scissor = pipe->create_rasterizer_state(pipe, &rs_state);

   if (ctx->has_stream_out) {
      rs_state.scissor = 0;
      rs_state.rasterizer_discard = 1;
      ctx->rs_discard_state = pipe->create_rasterizer_state(pipe, &rs_state);
   }

   ctx->base.vb_slot = 0; /* 0 for now */

   /* vertex elements states */
   memset(&velem[0], 0, sizeof(velem[0]) * 2);
   for (i = 0; i < 2; i++) {
      velem[i].src_offset = i * 4 * sizeof(float);
      velem[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
      velem[i].vertex_buffer_index = ctx->base.vb_slot;
   }
   ctx->velem_state = pipe->create_vertex_elements_state(pipe, 2, &velem[0]);

   if (ctx->has_stream_out) {
      static enum pipe_format formats[4] = {
         PIPE_FORMAT_R32_UINT,
         PIPE_FORMAT_R32G32_UINT,
         PIPE_FORMAT_R32G32B32_UINT,
         PIPE_FORMAT_R32G32B32A32_UINT
      };

      for (i = 0; i < 4; i++) {
         velem[0].src_format = formats[i];
         velem[0].vertex_buffer_index = ctx->base.vb_slot;
         ctx->velem_state_readbuf[i] =
               pipe->create_vertex_elements_state(pipe, 1, &velem[0]);
      }
   }

   /* Fragment shaders are created on-demand, except these.
    * The interpolation must be constant for integer texture clearing to work.
    */
   ctx->fs_empty = util_make_empty_fragment_shader(pipe);
   ctx->fs_write_one_cbuf =
      util_make_fragment_passthrough_shader(pipe, TGSI_SEMANTIC_GENERIC,
                                            TGSI_INTERPOLATE_CONSTANT, FALSE);
   ctx->fs_write_all_cbufs =
      util_make_fragment_passthrough_shader(pipe, TGSI_SEMANTIC_GENERIC,
                                            TGSI_INTERPOLATE_CONSTANT, TRUE);

   /* vertex shaders */
   {
      const uint semantic_names[] = { TGSI_SEMANTIC_POSITION,
                                      TGSI_SEMANTIC_GENERIC };
      const uint semantic_indices[] = { 0, 0 };
      ctx->vs =
         util_make_vertex_passthrough_shader(pipe, 2, semantic_names,
                                             semantic_indices);
   }
   if (ctx->has_stream_out) {
      struct pipe_stream_output_info so;
      const uint semantic_names[] = { TGSI_SEMANTIC_POSITION };
      const uint semantic_indices[] = { 0 };

      memset(&so, 0, sizeof(so));
      so.num_outputs = 1;
      so.output[0].num_components = 1;
      so.stride[0] = 1;

      ctx->vs_pos_only =
         util_make_vertex_passthrough_shader_with_so(pipe, 1, semantic_names,
                                                     semantic_indices, &so);
   }

   /* set invariant vertex coordinates */
   for (i = 0; i < 4; i++)
      ctx->vertices[i][0][3] = 1; /*v.w*/

   ctx->upload = u_upload_create(pipe, 65536, 4, PIPE_BIND_VERTEX_BUFFER);

   return &ctx->base;
}

void util_blitter_destroy(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = blitter->pipe;
   int i;

   for (i = 0; i <= PIPE_MASK_RGBA; i++) {
      pipe->delete_blend_state(pipe, ctx->blend[i]);
   }
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe,
                                          ctx->dsa_write_depth_keep_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);

   pipe->delete_rasterizer_state(pipe, ctx->rs_state);
   pipe->delete_rasterizer_state(pipe, ctx->rs_state_scissor);
   if (ctx->rs_discard_state)
      pipe->delete_rasterizer_state(pipe, ctx->rs_discard_state);
   pipe->delete_vs_state(pipe, ctx->vs);
   if (ctx->vs_pos_only)
      pipe->delete_vs_state(pipe, ctx->vs_pos_only);
   pipe->delete_vertex_elements_state(pipe, ctx->velem_state);
   for (i = 0; i < 4; i++) {
      if (ctx->velem_state_readbuf[i]) {
         pipe->delete_vertex_elements_state(pipe, ctx->velem_state_readbuf[i]);
      }
   }

   for (i = 0; i < PIPE_MAX_TEXTURE_TYPES; i++) {
      if (ctx->fs_texfetch_col[i])
         ctx->delete_fs_state(pipe, ctx->fs_texfetch_col[i]);
      if (ctx->fs_texfetch_depth[i])
         ctx->delete_fs_state(pipe, ctx->fs_texfetch_depth[i]);
      if (ctx->fs_texfetch_depthstencil[i])
         ctx->delete_fs_state(pipe, ctx->fs_texfetch_depthstencil[i]);
      if (ctx->fs_texfetch_stencil[i])
         ctx->delete_fs_state(pipe, ctx->fs_texfetch_stencil[i]);
   }
   ctx->delete_fs_state(pipe, ctx->fs_empty);
   ctx->delete_fs_state(pipe, ctx->fs_write_one_cbuf);
   ctx->delete_fs_state(pipe, ctx->fs_write_all_cbufs);

   pipe->delete_sampler_state(pipe, ctx->sampler_state_rect_linear);
   pipe->delete_sampler_state(pipe, ctx->sampler_state_rect);
   pipe->delete_sampler_state(pipe, ctx->sampler_state_linear);
   pipe->delete_sampler_state(pipe, ctx->sampler_state);
   u_upload_destroy(ctx->upload);
   FREE(ctx);
}

void util_blitter_set_texture_multisample(struct blitter_context *blitter,
                                          boolean supported)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   ctx->has_texture_multisample = supported;
}

static void blitter_set_running_flag(struct blitter_context_priv *ctx)
{
   if (ctx->base.running) {
      _debug_printf("u_blitter:%i: Caught recursion. This is a driver bug.\n",
                    __LINE__);
   }
   ctx->base.running = TRUE;
}

static void blitter_unset_running_flag(struct blitter_context_priv *ctx)
{
   if (!ctx->base.running) {
      _debug_printf("u_blitter:%i: Caught recursion. This is a driver bug.\n",
                    __LINE__);
   }
   ctx->base.running = FALSE;
}

static void blitter_check_saved_vertex_states(struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_velem_state != INVALID_PTR);
   assert(ctx->base.saved_vs != INVALID_PTR);
   assert(!ctx->has_geometry_shader || ctx->base.saved_gs != INVALID_PTR);
   assert(!ctx->has_stream_out || ctx->base.saved_num_so_targets != ~0);
   assert(ctx->base.saved_rs_state != INVALID_PTR);
}

static void blitter_restore_vertex_states(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;

   /* Vertex buffer. */
   pipe->set_vertex_buffers(pipe, ctx->base.vb_slot, 1,
                            &ctx->base.saved_vertex_buffer);
   pipe_resource_reference(&ctx->base.saved_vertex_buffer.buffer, NULL);

   /* Vertex elements. */
   pipe->bind_vertex_elements_state(pipe, ctx->base.saved_velem_state);
   ctx->base.saved_velem_state = INVALID_PTR;

   /* Vertex shader. */
   pipe->bind_vs_state(pipe, ctx->base.saved_vs);
   ctx->base.saved_vs = INVALID_PTR;

   /* Geometry shader. */
   if (ctx->has_geometry_shader) {
      pipe->bind_gs_state(pipe, ctx->base.saved_gs);
      ctx->base.saved_gs = INVALID_PTR;
   }

   /* Stream outputs. */
   if (ctx->has_stream_out) {
      pipe->set_stream_output_targets(pipe,
                                      ctx->base.saved_num_so_targets,
                                      ctx->base.saved_so_targets, ~0);

      for (i = 0; i < ctx->base.saved_num_so_targets; i++)
         pipe_so_target_reference(&ctx->base.saved_so_targets[i], NULL);

      ctx->base.saved_num_so_targets = ~0;
   }

   /* Rasterizer. */
   pipe->bind_rasterizer_state(pipe, ctx->base.saved_rs_state);
   ctx->base.saved_rs_state = INVALID_PTR;
}

static void blitter_check_saved_fragment_states(struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_fs != INVALID_PTR);
   assert(ctx->base.saved_dsa_state != INVALID_PTR);
   assert(ctx->base.saved_blend_state != INVALID_PTR);
}

static void blitter_restore_fragment_states(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   /* Fragment shader. */
   ctx->bind_fs_state(pipe, ctx->base.saved_fs);
   ctx->base.saved_fs = INVALID_PTR;

   /* Depth, stencil, alpha. */
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->base.saved_dsa_state);
   ctx->base.saved_dsa_state = INVALID_PTR;

   /* Blend state. */
   pipe->bind_blend_state(pipe, ctx->base.saved_blend_state);
   ctx->base.saved_blend_state = INVALID_PTR;

   /* Sample mask. */
   if (ctx->base.is_sample_mask_saved) {
      pipe->set_sample_mask(pipe, ctx->base.saved_sample_mask);
      ctx->base.is_sample_mask_saved = FALSE;
   }

   /* Miscellaneous states. */
   /* XXX check whether these are saved and whether they need to be restored
    * (depending on the operation) */
   pipe->set_stencil_ref(pipe, &ctx->base.saved_stencil_ref);
   pipe->set_viewport_states(pipe, 0, 1, &ctx->base.saved_viewport);
}

static void blitter_check_saved_fb_state(struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_fb_state.nr_cbufs != ~0);
}

static void blitter_disable_render_cond(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->base.saved_render_cond_query) {
      pipe->render_condition(pipe, NULL, FALSE, 0);
   }
}

static void blitter_restore_render_cond(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->base.saved_render_cond_query) {
      pipe->render_condition(pipe, ctx->base.saved_render_cond_query,
                             ctx->base.saved_render_cond_cond,
                             ctx->base.saved_render_cond_mode);
      ctx->base.saved_render_cond_query = NULL;
   }
}

static void blitter_restore_fb_state(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   pipe->set_framebuffer_state(pipe, &ctx->base.saved_fb_state);
   util_unreference_framebuffer_state(&ctx->base.saved_fb_state);
}

static void blitter_check_saved_textures(struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_num_sampler_states != ~0);
   assert(ctx->base.saved_num_sampler_views != ~0);
}

static void blitter_restore_textures(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;

   /* Fragment sampler states. */
   pipe->bind_fragment_sampler_states(pipe,
                                      ctx->base.saved_num_sampler_states,
                                      ctx->base.saved_sampler_states);
   ctx->base.saved_num_sampler_states = ~0;

   /* Fragment sampler views. */
   pipe->set_fragment_sampler_views(pipe,
                                    ctx->base.saved_num_sampler_views,
                                    ctx->base.saved_sampler_views);

   for (i = 0; i < ctx->base.saved_num_sampler_views; i++)
      pipe_sampler_view_reference(&ctx->base.saved_sampler_views[i], NULL);

   ctx->base.saved_num_sampler_views = ~0;
}

static void blitter_set_rectangle(struct blitter_context_priv *ctx,
                                  int x1, int y1, int x2, int y2,
                                  float depth)
{
   int i;

   /* set vertex positions */
   ctx->vertices[0][0][0] = (float)x1 / ctx->dst_width * 2.0f - 1.0f; /*v0.x*/
   ctx->vertices[0][0][1] = (float)y1 / ctx->dst_height * 2.0f - 1.0f; /*v0.y*/

   ctx->vertices[1][0][0] = (float)x2 / ctx->dst_width * 2.0f - 1.0f; /*v1.x*/
   ctx->vertices[1][0][1] = (float)y1 / ctx->dst_height * 2.0f - 1.0f; /*v1.y*/

   ctx->vertices[2][0][0] = (float)x2 / ctx->dst_width * 2.0f - 1.0f; /*v2.x*/
   ctx->vertices[2][0][1] = (float)y2 / ctx->dst_height * 2.0f - 1.0f; /*v2.y*/

   ctx->vertices[3][0][0] = (float)x1 / ctx->dst_width * 2.0f - 1.0f; /*v3.x*/
   ctx->vertices[3][0][1] = (float)y2 / ctx->dst_height * 2.0f - 1.0f; /*v3.y*/

   for (i = 0; i < 4; i++)
      ctx->vertices[i][0][2] = depth; /*z*/

   /* viewport */
   ctx->viewport.scale[0] = 0.5f * ctx->dst_width;
   ctx->viewport.scale[1] = 0.5f * ctx->dst_height;
   ctx->viewport.scale[2] = 1.0f;
   ctx->viewport.scale[3] = 1.0f;
   ctx->viewport.translate[0] = 0.5f * ctx->dst_width;
   ctx->viewport.translate[1] = 0.5f * ctx->dst_height;
   ctx->viewport.translate[2] = 0.0f;
   ctx->viewport.translate[3] = 0.0f;
   ctx->base.pipe->set_viewport_states(ctx->base.pipe, 0, 1, &ctx->viewport);
}

static void blitter_set_clear_color(struct blitter_context_priv *ctx,
                                    const union pipe_color_union *color)
{
   int i;

   if (color) {
      for (i = 0; i < 4; i++) {
         uint32_t *uiverts = (uint32_t *)ctx->vertices[i][1];
         uiverts[0] = color->ui[0];
         uiverts[1] = color->ui[1];
         uiverts[2] = color->ui[2];
         uiverts[3] = color->ui[3];
      }
   } else {
      for (i = 0; i < 4; i++) {
         ctx->vertices[i][1][0] = 0;
         ctx->vertices[i][1][1] = 0;
         ctx->vertices[i][1][2] = 0;
         ctx->vertices[i][1][3] = 0;
      }
   }
}

static void get_texcoords(struct pipe_sampler_view *src,
                          unsigned src_width0, unsigned src_height0,
                          int x1, int y1, int x2, int y2,
                          float out[4])
{
   struct pipe_resource *tex = src->texture;
   unsigned level = src->u.tex.first_level;
   boolean normalized = tex->target != PIPE_TEXTURE_RECT &&
                        tex->nr_samples <= 1;

   if (normalized) {
      out[0] = x1 / (float)u_minify(src_width0,  level);
      out[1] = y1 / (float)u_minify(src_height0, level);
      out[2] = x2 / (float)u_minify(src_width0,  level);
      out[3] = y2 / (float)u_minify(src_height0, level);
   } else {
      out[0] = (float) x1;
      out[1] = (float) y1;
      out[2] = (float) x2;
      out[3] = (float) y2;
   }
}

static void set_texcoords_in_vertices(const float coord[4],
                                      float *out, unsigned stride)
{
   out[0] = coord[0]; /*t0.s*/
   out[1] = coord[1]; /*t0.t*/
   out += stride;
   out[0] = coord[2]; /*t1.s*/
   out[1] = coord[1]; /*t1.t*/
   out += stride;
   out[0] = coord[2]; /*t2.s*/
   out[1] = coord[3]; /*t2.t*/
   out += stride;
   out[0] = coord[0]; /*t3.s*/
   out[1] = coord[3]; /*t3.t*/
}

static void blitter_set_texcoords(struct blitter_context_priv *ctx,
                                  struct pipe_sampler_view *src,
                                  unsigned src_width0, unsigned src_height0,
                                  unsigned layer, unsigned sample,
                                  int x1, int y1, int x2, int y2)
{
   unsigned i;
   float coord[4];
   float face_coord[4][2];

   get_texcoords(src, src_width0, src_height0, x1, y1, x2, y2, coord);

   if (src->texture->target == PIPE_TEXTURE_CUBE ||
       src->texture->target == PIPE_TEXTURE_CUBE_ARRAY) {
      set_texcoords_in_vertices(coord, &face_coord[0][0], 2);
      util_map_texcoords2d_onto_cubemap(layer % 6,
                                        /* pointer, stride in floats */
                                        &face_coord[0][0], 2,
                                        &ctx->vertices[0][1][0], 8);
   } else {
      set_texcoords_in_vertices(coord, &ctx->vertices[0][1][0], 8);
   }

   /* Set the layer. */
   switch (src->texture->target) {
   case PIPE_TEXTURE_3D:
      {
         float r = layer / (float)u_minify(src->texture->depth0,
                                           src->u.tex.first_level);
         for (i = 0; i < 4; i++)
            ctx->vertices[i][1][2] = r; /*r*/
      }
      break;

   case PIPE_TEXTURE_1D_ARRAY:
      for (i = 0; i < 4; i++)
         ctx->vertices[i][1][1] = (float) layer; /*t*/
      break;

   case PIPE_TEXTURE_2D_ARRAY:
      for (i = 0; i < 4; i++) {
         ctx->vertices[i][1][2] = (float) layer;  /*r*/
         ctx->vertices[i][1][3] = (float) sample; /*q*/
      }
      break;

   case PIPE_TEXTURE_CUBE_ARRAY:
      for (i = 0; i < 4; i++)
         ctx->vertices[i][1][3] = (float) (layer / 6); /*w*/
      break;

   case PIPE_TEXTURE_2D:
      for (i = 0; i < 4; i++) {
         ctx->vertices[i][1][3] = (float) sample; /*r*/
      }
      break;

   default:;
   }
}

static void blitter_set_dst_dimensions(struct blitter_context_priv *ctx,
                                       unsigned width, unsigned height)
{
   ctx->dst_width = width;
   ctx->dst_height = height;
}

static void *blitter_get_fs_texfetch_col(struct blitter_context_priv *ctx,
                                         enum pipe_texture_target target,
                                         unsigned nr_samples)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (nr_samples > 1) {
      void **shader = &ctx->fs_texfetch_col_msaa[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target,
                                                       nr_samples);

         *shader = util_make_fs_blit_msaa_color(pipe, tgsi_tex);
      }

      return *shader;
   } else {
      void **shader = &ctx->fs_texfetch_col[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);

         *shader =
            util_make_fragment_tex_shader(pipe, tgsi_tex,
                                          TGSI_INTERPOLATE_LINEAR);
      }

      return *shader;
   }
}

static INLINE
void *blitter_get_fs_texfetch_depth(struct blitter_context_priv *ctx,
                                    enum pipe_texture_target target,
                                    unsigned nr_samples)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (nr_samples > 1) {
      void **shader = &ctx->fs_texfetch_depth_msaa[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target,
                                                       nr_samples);

         *shader =
            util_make_fs_blit_msaa_depth(pipe, tgsi_tex);
      }

      return *shader;
   } else {
      void **shader = &ctx->fs_texfetch_depth[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);

         *shader =
            util_make_fragment_tex_shader_writedepth(pipe, tgsi_tex,
                                                     TGSI_INTERPOLATE_LINEAR);
      }

      return *shader;
   }
}

static INLINE
void *blitter_get_fs_texfetch_depthstencil(struct blitter_context_priv *ctx,
                                           enum pipe_texture_target target,
                                           unsigned nr_samples)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (nr_samples > 1) {
      void **shader = &ctx->fs_texfetch_depthstencil_msaa[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target,
                                                       nr_samples);

         *shader =
            util_make_fs_blit_msaa_depthstencil(pipe, tgsi_tex);
      }

      return *shader;
   } else {
      void **shader = &ctx->fs_texfetch_depthstencil[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);

         *shader =
            util_make_fragment_tex_shader_writedepthstencil(pipe, tgsi_tex,
                                                     TGSI_INTERPOLATE_LINEAR);
      }

      return *shader;
   }
}

static INLINE
void *blitter_get_fs_texfetch_stencil(struct blitter_context_priv *ctx,
                                      enum pipe_texture_target target,
                                      unsigned nr_samples)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (nr_samples > 1) {
      void **shader = &ctx->fs_texfetch_stencil_msaa[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target,
                                                       nr_samples);

         *shader =
            util_make_fs_blit_msaa_stencil(pipe, tgsi_tex);
      }

      return *shader;
   } else {
      void **shader = &ctx->fs_texfetch_stencil[target];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         unsigned tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);

         *shader =
            util_make_fragment_tex_shader_writestencil(pipe, tgsi_tex,
                                                       TGSI_INTERPOLATE_LINEAR);
      }

      return *shader;
   }
}

void util_blitter_cache_all_shaders(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_screen *screen = blitter->pipe->screen;
   unsigned i, target, max_samples;
   boolean has_arraytex, has_cubearraytex;

   max_samples = ctx->has_texture_multisample ? 2 : 1;
   has_arraytex = screen->get_param(screen,
                                    PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS) != 0;
   has_cubearraytex = screen->get_param(screen,
                                    PIPE_CAP_CUBE_MAP_ARRAY) != 0;

   /* It only matters if i <= 1 or > 1. */
   for (i = 1; i <= max_samples; i++) {
      for (target = PIPE_TEXTURE_1D; target < PIPE_MAX_TEXTURE_TYPES; target++) {
         if (!has_arraytex &&
             (target == PIPE_TEXTURE_1D_ARRAY ||
              target == PIPE_TEXTURE_2D_ARRAY)) {
            continue;
         }
         if (!has_cubearraytex &&
             (target == PIPE_TEXTURE_CUBE_ARRAY))
            continue;

	 if (i > 1 &&
	     (target != PIPE_TEXTURE_2D &&
	      target != PIPE_TEXTURE_2D_ARRAY))
	    continue;

         blitter_get_fs_texfetch_col(ctx, target, i);
         blitter_get_fs_texfetch_depth(ctx, target, i);
         if (ctx->has_stencil_export) {
            blitter_get_fs_texfetch_depthstencil(ctx, target, i);
            blitter_get_fs_texfetch_stencil(ctx, target, i);
         }
      }
   }
}

static void blitter_set_common_draw_rect_state(struct blitter_context_priv *ctx,
                                               boolean scissor)
{
   struct pipe_context *pipe = ctx->base.pipe;

   pipe->bind_rasterizer_state(pipe, scissor ? ctx->rs_state_scissor
                                             : ctx->rs_state);
   pipe->bind_vs_state(pipe, ctx->vs);
   if (ctx->has_geometry_shader)
      pipe->bind_gs_state(pipe, NULL);
   if (ctx->has_stream_out)
      pipe->set_stream_output_targets(pipe, 0, NULL, 0);
}

static void blitter_draw(struct blitter_context_priv *ctx,
                         int x1, int y1, int x2, int y2, float depth)
{
   struct pipe_resource *buf = NULL;
   unsigned offset = 0;

   blitter_set_rectangle(ctx, x1, y1, x2, y2, depth);

   u_upload_data(ctx->upload, 0, sizeof(ctx->vertices), ctx->vertices,
                 &offset, &buf);
   u_upload_unmap(ctx->upload);
   util_draw_vertex_buffer(ctx->base.pipe, NULL, buf, ctx->base.vb_slot,
                           offset, PIPE_PRIM_TRIANGLE_FAN, 4, 2);
   pipe_resource_reference(&buf, NULL);
}

void util_blitter_draw_rectangle(struct blitter_context *blitter,
                                 int x1, int y1, int x2, int y2, float depth,
                                 enum blitter_attrib_type type,
                                 const union pipe_color_union *attrib)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   switch (type) {
      case UTIL_BLITTER_ATTRIB_COLOR:
         blitter_set_clear_color(ctx, attrib);
         break;

      case UTIL_BLITTER_ATTRIB_TEXCOORD:
         set_texcoords_in_vertices(attrib->f, &ctx->vertices[0][1][0], 8);
         break;

      default:;
   }

   blitter_draw(ctx, x1, y1, x2, y2, depth);
}

static void util_blitter_clear_custom(struct blitter_context *blitter,
                                      unsigned width, unsigned height,
                                      unsigned clear_buffers,
                                      const union pipe_color_union *color,
                                      double depth, unsigned stencil,
                                      void *custom_blend, void *custom_dsa)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_stencil_ref sr = { { 0 } };

   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   if (custom_blend) {
      pipe->bind_blend_state(pipe, custom_blend);
   } else if (clear_buffers & PIPE_CLEAR_COLOR) {
      pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA]);
   } else {
      pipe->bind_blend_state(pipe, ctx->blend[0]);
   }

   if (custom_dsa) {
      pipe->bind_depth_stencil_alpha_state(pipe, custom_dsa);
   } else if ((clear_buffers & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
   } else if (clear_buffers & PIPE_CLEAR_DEPTH) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_keep_stencil);
   } else if (clear_buffers & PIPE_CLEAR_STENCIL) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);
   } else {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   }

   sr.ref_value[0] = stencil & 0xff;
   pipe->set_stencil_ref(pipe, &sr);

   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);
   ctx->bind_fs_state(pipe, ctx->fs_write_all_cbufs);
   pipe->set_sample_mask(pipe, ~0);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, width, height);
   blitter->draw_rectangle(blitter, 0, 0, width, height, (float) depth,
                           UTIL_BLITTER_ATTRIB_COLOR, color);

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}

void util_blitter_clear(struct blitter_context *blitter,
                        unsigned width, unsigned height,
                        unsigned clear_buffers,
                        const union pipe_color_union *color,
                        double depth, unsigned stencil)
{
   util_blitter_clear_custom(blitter, width, height,
                             clear_buffers, color, depth, stencil,
                             NULL, NULL);
}

void util_blitter_custom_clear_depth(struct blitter_context *blitter,
                                     unsigned width, unsigned height,
                                     double depth, void *custom_dsa)
{
    static const union pipe_color_union color;
    util_blitter_clear_custom(blitter, width, height, 0, &color, depth, 0,
                              NULL, custom_dsa);
}

void util_blitter_default_dst_texture(struct pipe_surface *dst_templ,
                                      struct pipe_resource *dst,
                                      unsigned dstlevel,
                                      unsigned dstz)
{
    memset(dst_templ, 0, sizeof(*dst_templ));
    dst_templ->format = util_format_linear(dst->format);
    dst_templ->u.tex.level = dstlevel;
    dst_templ->u.tex.first_layer = dstz;
    dst_templ->u.tex.last_layer = dstz;
}

static struct pipe_surface *
util_blitter_get_next_surface_layer(struct pipe_context *pipe,
                                    struct pipe_surface *surf)
{
   struct pipe_surface dst_templ;

   memset(&dst_templ, 0, sizeof(dst_templ));
   dst_templ.format = surf->format;
   dst_templ.u.tex.level = surf->u.tex.level;
   dst_templ.u.tex.first_layer = surf->u.tex.first_layer + 1;
   dst_templ.u.tex.last_layer = surf->u.tex.last_layer + 1;

   return pipe->create_surface(pipe, surf->texture, &dst_templ);
}

void util_blitter_default_src_texture(struct pipe_sampler_view *src_templ,
                                      struct pipe_resource *src,
                                      unsigned srclevel)
{
    memset(src_templ, 0, sizeof(*src_templ));
    src_templ->format = util_format_linear(src->format);
    src_templ->u.tex.first_level = srclevel;
    src_templ->u.tex.last_level = srclevel;
    src_templ->u.tex.first_layer = 0;
    src_templ->u.tex.last_layer =
        src->target == PIPE_TEXTURE_3D ? u_minify(src->depth0, srclevel) - 1
                                       : src->array_size - 1;
    src_templ->swizzle_r = PIPE_SWIZZLE_RED;
    src_templ->swizzle_g = PIPE_SWIZZLE_GREEN;
    src_templ->swizzle_b = PIPE_SWIZZLE_BLUE;
    src_templ->swizzle_a = PIPE_SWIZZLE_ALPHA;
}

static boolean is_blit_generic_supported(struct blitter_context *blitter,
                                         const struct pipe_resource *dst,
                                         enum pipe_format dst_format,
                                         const struct pipe_resource *src,
                                         enum pipe_format src_format,
                                         unsigned mask)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_screen *screen = ctx->base.pipe->screen;

   if (dst) {
      unsigned bind;
      const struct util_format_description *desc =
            util_format_description(dst_format);
      boolean dst_has_stencil = util_format_has_stencil(desc);

      /* Stencil export must be supported for stencil copy. */
      if ((mask & PIPE_MASK_S) && dst_has_stencil &&
          !ctx->has_stencil_export) {
         return FALSE;
      }

      if (dst_has_stencil || util_format_has_depth(desc))
         bind = PIPE_BIND_DEPTH_STENCIL;
      else
         bind = PIPE_BIND_RENDER_TARGET;

      if (!screen->is_format_supported(screen, dst_format, dst->target,
                                       dst->nr_samples, bind)) {
         return FALSE;
      }
   }

   if (src) {
      if (src->nr_samples > 1 && !ctx->has_texture_multisample) {
         return FALSE;
      }

      if (!screen->is_format_supported(screen, src_format, src->target,
                                 src->nr_samples, PIPE_BIND_SAMPLER_VIEW)) {
         return FALSE;
      }

      /* Check stencil sampler support for stencil copy. */
      if (mask & PIPE_MASK_S) {
         if (util_format_has_stencil(util_format_description(src_format))) {
            enum pipe_format stencil_format =
               util_format_stencil_only(src_format);
            assert(stencil_format != PIPE_FORMAT_NONE);

            if (stencil_format != src_format &&
                !screen->is_format_supported(screen, stencil_format,
                                             src->target, src->nr_samples,
                                             PIPE_BIND_SAMPLER_VIEW)) {
               return FALSE;
            }
         }
      }
   }

   return TRUE;
}

boolean util_blitter_is_copy_supported(struct blitter_context *blitter,
                                       const struct pipe_resource *dst,
                                       const struct pipe_resource *src,
                                       unsigned mask)
{
   return is_blit_generic_supported(blitter, dst, dst->format,
                                    src, src->format, mask);
}

boolean util_blitter_is_blit_supported(struct blitter_context *blitter,
				       const struct pipe_blit_info *info)
{
   return is_blit_generic_supported(blitter,
                                    info->dst.resource, info->dst.format,
                                    info->src.resource, info->src.format,
                                    info->mask);
}

void util_blitter_copy_texture(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned dst_level,
                               unsigned dstx, unsigned dsty, unsigned dstz,
                               struct pipe_resource *src,
                               unsigned src_level,
                               const struct pipe_box *srcbox, unsigned mask,
                               boolean copy_all_samples)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_surface *dst_view, dst_templ;
   struct pipe_sampler_view src_templ, *src_view;
   struct pipe_box dstbox;

   assert(dst && src);
   assert(src->target < PIPE_MAX_TEXTURE_TYPES);

   u_box_3d(dstx, dsty, dstz, abs(srcbox->width), abs(srcbox->height),
            abs(srcbox->depth), &dstbox);

   /* Initialize the surface. */
   util_blitter_default_dst_texture(&dst_templ, dst, dst_level, dstz);
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   util_blitter_default_src_texture(&src_templ, src, src_level);
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* Copy. */
   util_blitter_blit_generic(blitter, dst_view, &dstbox,
                             src_view, srcbox, src->width0, src->height0,
                             mask, PIPE_TEX_FILTER_NEAREST, NULL,
                             copy_all_samples);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
}

void util_blitter_blit_generic(struct blitter_context *blitter,
                               struct pipe_surface *dst,
                               const struct pipe_box *dstbox,
                               struct pipe_sampler_view *src,
                               const struct pipe_box *srcbox,
                               unsigned src_width0, unsigned src_height0,
                               unsigned mask, unsigned filter,
                               const struct pipe_scissor_state *scissor,
                               boolean copy_all_samples)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   enum pipe_texture_target src_target = src->texture->target;
   unsigned src_samples = src->texture->nr_samples;
   boolean has_depth, has_stencil, has_color;
   boolean blit_stencil, blit_depth, blit_color;
   void *sampler_state;
   const struct util_format_description *src_desc =
         util_format_description(src->format);
   const struct util_format_description *dst_desc =
         util_format_description(dst->format);

   has_color = src_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS &&
               dst_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS;
   has_depth = util_format_has_depth(src_desc) &&
               util_format_has_depth(dst_desc);
   has_stencil = util_format_has_stencil(src_desc) &&
                 util_format_has_stencil(dst_desc);

   blit_color = has_color && (mask & PIPE_MASK_RGBA);
   blit_depth = has_depth && (mask & PIPE_MASK_Z);
   blit_stencil = has_stencil && (mask & PIPE_MASK_S) &&
                  ctx->has_stencil_export;

   if (!blit_stencil && !blit_depth && !blit_color) {
      return;
   }

   /* Check whether the states are properly saved. */
   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_textures(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* Initialize framebuffer state. */
   fb_state.width = dst->width;
   fb_state.height = dst->height;
   fb_state.nr_cbufs = blit_depth || blit_stencil ? 0 : 1;
   fb_state.cbufs[0] = NULL;
   fb_state.zsbuf = NULL;

   if (blit_depth || blit_stencil) {
      pipe->bind_blend_state(pipe, ctx->blend[0]);

      if (blit_depth && blit_stencil) {
         pipe->bind_depth_stencil_alpha_state(pipe,
                                              ctx->dsa_write_depth_stencil);
         ctx->bind_fs_state(pipe,
               blitter_get_fs_texfetch_depthstencil(ctx, src_target,
                                                    src_samples));
      } else if (blit_depth) {
         pipe->bind_depth_stencil_alpha_state(pipe,
                                              ctx->dsa_write_depth_keep_stencil);
         ctx->bind_fs_state(pipe,
               blitter_get_fs_texfetch_depth(ctx, src_target,
                                             src_samples));
      } else { /* is_stencil */
         pipe->bind_depth_stencil_alpha_state(pipe,
                                              ctx->dsa_keep_depth_write_stencil);
         ctx->bind_fs_state(pipe,
               blitter_get_fs_texfetch_stencil(ctx, src_target,
                                               src_samples));
      }

   } else {
      pipe->bind_blend_state(pipe, ctx->blend[mask & PIPE_MASK_RGBA]);
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
      ctx->bind_fs_state(pipe,
            blitter_get_fs_texfetch_col(ctx, src_target,
                                        src_samples));
   }

   /* Set the linear filter only for scaled color non-MSAA blits. */
   if (filter == PIPE_TEX_FILTER_LINEAR &&
       !blit_depth && !blit_stencil &&
       src_samples <= 1 &&
       (dstbox->width != abs(srcbox->width) ||
        dstbox->height != abs(srcbox->height))) {
      if (src_target == PIPE_TEXTURE_RECT) {
         sampler_state = ctx->sampler_state_rect_linear;
      } else {
         sampler_state = ctx->sampler_state_linear;
      }
   } else {
      if (src_target == PIPE_TEXTURE_RECT) {
         sampler_state = ctx->sampler_state_rect;
      } else {
         sampler_state = ctx->sampler_state;
      }
   }

   /* Set samplers. */
   if (blit_depth && blit_stencil) {
      /* Setup two samplers, one for depth and the other one for stencil. */
      struct pipe_sampler_view templ;
      struct pipe_sampler_view *views[2];
      void *samplers[2] = {sampler_state, sampler_state};

      templ = *src;
      templ.format = util_format_stencil_only(templ.format);
      assert(templ.format != PIPE_FORMAT_NONE);

      views[0] = src;
      views[1] = pipe->create_sampler_view(pipe, src->texture, &templ);

      pipe->set_fragment_sampler_views(pipe, 2, views);
      pipe->bind_fragment_sampler_states(pipe, 2, samplers);

      pipe_sampler_view_reference(&views[1], NULL);
   } else if (blit_stencil) {
      /* Set a stencil-only sampler view for it not to sample depth instead. */
      struct pipe_sampler_view templ;
      struct pipe_sampler_view *view;

      templ = *src;
      templ.format = util_format_stencil_only(templ.format);
      assert(templ.format != PIPE_FORMAT_NONE);

      view = pipe->create_sampler_view(pipe, src->texture, &templ);

      pipe->set_fragment_sampler_views(pipe, 1, &view);
      pipe->bind_fragment_sampler_states(pipe, 1, &sampler_state);

      pipe_sampler_view_reference(&view, NULL);
   } else {
      pipe->set_fragment_sampler_views(pipe, 1, &src);
      pipe->bind_fragment_sampler_states(pipe, 1, &sampler_state);
   }

   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);
   if (scissor) {
      pipe->set_scissor_states(pipe, 0, 1, scissor);
   }

   blitter_set_common_draw_rect_state(ctx, scissor != NULL);
   blitter_set_dst_dimensions(ctx, dst->width, dst->height);

   if ((src_target == PIPE_TEXTURE_1D ||
        src_target == PIPE_TEXTURE_2D ||
        src_target == PIPE_TEXTURE_RECT) &&
       src_samples <= 1) {
      /* Draw the quad with the draw_rectangle callback. */

      /* Set texture coordinates. - use a pipe color union
       * for interface purposes.
       * XXX pipe_color_union is a wrong name since we use that to set
       * texture coordinates too.
       */
      union pipe_color_union coord;
      get_texcoords(src, src_width0, src_height0, srcbox->x, srcbox->y,
                    srcbox->x+srcbox->width, srcbox->y+srcbox->height, coord.f);

      /* Set framebuffer state. */
      if (blit_depth || blit_stencil) {
         fb_state.zsbuf = dst;
      } else {
         fb_state.cbufs[0] = dst;
      }
      pipe->set_framebuffer_state(pipe, &fb_state);

      /* Draw. */
      pipe->set_sample_mask(pipe, ~0);
      blitter->draw_rectangle(blitter, dstbox->x, dstbox->y,
                              dstbox->x + dstbox->width,
                              dstbox->y + dstbox->height, 0,
                              UTIL_BLITTER_ATTRIB_TEXCOORD, &coord);
   } else {
      /* Draw the quad with the generic codepath. */
      int z;
      for (z = 0; z < dstbox->depth; z++) {
         struct pipe_surface *old;

         /* Set framebuffer state. */
         if (blit_depth || blit_stencil) {
            fb_state.zsbuf = dst;
         } else {
            fb_state.cbufs[0] = dst;
         }
         pipe->set_framebuffer_state(pipe, &fb_state);

         /* See if we need to blit a multisample or singlesample buffer. */
         if (copy_all_samples &&
             src_samples == dst->texture->nr_samples &&
             dst->texture->nr_samples > 1) {
            unsigned i, max_sample = MAX2(dst->texture->nr_samples, 1) - 1;

            for (i = 0; i <= max_sample; i++) {
               pipe->set_sample_mask(pipe, 1 << i);
               blitter_set_texcoords(ctx, src, src_width0, src_height0,
                                     srcbox->z + z,
                                     i, srcbox->x, srcbox->y,
                                     srcbox->x + srcbox->width,
                                     srcbox->y + srcbox->height);
               blitter_draw(ctx, dstbox->x, dstbox->y,
                            dstbox->x + dstbox->width,
                            dstbox->y + dstbox->height, 0);
            }
         } else {
            pipe->set_sample_mask(pipe, ~0);
            blitter_set_texcoords(ctx, src, src_width0, src_height0,
                                  srcbox->z + z, 0,
                                  srcbox->x, srcbox->y,
                                  srcbox->x + srcbox->width,
                                  srcbox->y + srcbox->height);
            blitter_draw(ctx, dstbox->x, dstbox->y,
                         dstbox->x + dstbox->width,
                         dstbox->y + dstbox->height, 0);
         }

         /* Get the next surface or (if this is the last iteration)
          * just unreference the last one. */
         old = dst;
         if (z < dstbox->depth-1) {
            dst = ctx->base.get_next_surface_layer(ctx->base.pipe, dst);
         }
         if (z) {
            pipe_surface_reference(&old, NULL);
         }
      }
   }

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_textures(ctx);
   blitter_restore_fb_state(ctx);
   if (scissor) {
      pipe->set_scissor_states(pipe, 0, 1, &ctx->base.saved_scissor);
   }
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}

void
util_blitter_blit(struct blitter_context *blitter,
		  const struct pipe_blit_info *info)
{
   struct pipe_resource *dst = info->dst.resource;
   struct pipe_resource *src = info->src.resource;
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_surface *dst_view, dst_templ;
   struct pipe_sampler_view src_templ, *src_view;

   /* Initialize the surface. */
   util_blitter_default_dst_texture(&dst_templ, dst, info->dst.level,
                                    info->dst.box.z);
   dst_templ.format = info->dst.format;
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   util_blitter_default_src_texture(&src_templ, src, info->src.level);
   src_templ.format = info->src.format;
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* Copy. */
   util_blitter_blit_generic(blitter, dst_view, &info->dst.box,
                             src_view, &info->src.box, src->width0, src->height0,
                             info->mask, info->filter,
                             info->scissor_enable ? &info->scissor : NULL, TRUE);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
}

/* Clear a region of a color surface to a constant value. */
void util_blitter_clear_render_target(struct blitter_context *blitter,
                                      struct pipe_surface *dstsurf,
                                      const union pipe_color_union *color,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA]);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   ctx->bind_fs_state(pipe, ctx->fs_write_one_cbuf);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dstsurf;
   fb_state.zsbuf = 0;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter->draw_rectangle(blitter, dstx, dsty, dstx+width, dsty+height, 0,
                           UTIL_BLITTER_ATTRIB_COLOR, color);

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_fb_state(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}

/* Clear a region of a depth stencil surface. */
void util_blitter_clear_depth_stencil(struct blitter_context *blitter,
                                      struct pipe_surface *dstsurf,
                                      unsigned clear_flags,
                                      double depth,
                                      unsigned stencil,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   struct pipe_stencil_ref sr = { { 0 } };

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[0]);
   if ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
      sr.ref_value[0] = stencil & 0xff;
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
      pipe->set_stencil_ref(pipe, &sr);
   }
   else if (clear_flags & PIPE_CLEAR_DEPTH) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_keep_stencil);
   }
   else if (clear_flags & PIPE_CLEAR_STENCIL) {
      sr.ref_value[0] = stencil & 0xff;
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);
      pipe->set_stencil_ref(pipe, &sr);
   }
   else
      /* hmm that should be illegal probably, or make it a no-op somewhere */
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);

   ctx->bind_fs_state(pipe, ctx->fs_empty);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 0;
   fb_state.cbufs[0] = 0;
   fb_state.zsbuf = dstsurf;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter->draw_rectangle(blitter, dstx, dsty, dstx+width, dsty+height,
                           (float) depth,
                           UTIL_BLITTER_ATTRIB_NONE, NULL);

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_fb_state(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}

/* draw a rectangle across a region using a custom dsa stage - for r600g */
void util_blitter_custom_depth_stencil(struct blitter_context *blitter,
				       struct pipe_surface *zsurf,
				       struct pipe_surface *cbsurf,
				       unsigned sample_mask,
				       void *dsa_stage, float depth)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;

   assert(zsurf->texture);
   if (!zsurf->texture)
      return;

   /* check the saved state */
   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, cbsurf ? ctx->blend[PIPE_MASK_RGBA] :
                                         ctx->blend[0]);
   pipe->bind_depth_stencil_alpha_state(pipe, dsa_stage);
   ctx->bind_fs_state(pipe, ctx->fs_empty);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);

   /* set a framebuffer state */
   fb_state.width = zsurf->width;
   fb_state.height = zsurf->height;
   fb_state.nr_cbufs = 1;
   if (cbsurf) {
	   fb_state.cbufs[0] = cbsurf;
	   fb_state.nr_cbufs = 1;
   } else {
	   fb_state.cbufs[0] = NULL;
	   fb_state.nr_cbufs = 0;
   }
   fb_state.zsbuf = zsurf;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, sample_mask);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, zsurf->width, zsurf->height);
   blitter->draw_rectangle(blitter, 0, 0, zsurf->width, zsurf->height, depth,
                           UTIL_BLITTER_ATTRIB_NONE, NULL);

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_fb_state(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}

void util_blitter_copy_buffer(struct blitter_context *blitter,
                              struct pipe_resource *dst,
                              unsigned dstx,
                              struct pipe_resource *src,
                              unsigned srcx,
                              unsigned size)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_vertex_buffer vb;
   struct pipe_stream_output_target *so_target;

   if (srcx >= src->width0 ||
       dstx >= dst->width0) {
      return;
   }
   if (srcx + size > src->width0) {
      size = src->width0 - srcx;
   }
   if (dstx + size > dst->width0) {
      size = dst->width0 - dstx;
   }

   /* Drivers not capable of Stream Out should not call this function
    * in the first place. */
   assert(ctx->has_stream_out);

   /* Some alignment is required. */
   if (srcx % 4 != 0 || dstx % 4 != 0 || size % 4 != 0 ||
       !ctx->has_stream_out) {
      struct pipe_box box;
      u_box_1d(srcx, size, &box);
      util_resource_copy_region(pipe, dst, 0, dstx, 0, 0, src, 0, &box);
      return;
   }

   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_disable_render_cond(ctx);

   vb.buffer = src;
   vb.buffer_offset = srcx;
   vb.stride = 4;

   pipe->set_vertex_buffers(pipe, ctx->base.vb_slot, 1, &vb);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state_readbuf[0]);
   pipe->bind_vs_state(pipe, ctx->vs_pos_only);
   if (ctx->has_geometry_shader)
      pipe->bind_gs_state(pipe, NULL);
   pipe->bind_rasterizer_state(pipe, ctx->rs_discard_state);

   so_target = pipe->create_stream_output_target(pipe, dst, dstx, size);
   pipe->set_stream_output_targets(pipe, 1, &so_target, 0);

   util_draw_arrays(pipe, PIPE_PRIM_POINTS, 0, size / 4);

   blitter_restore_vertex_states(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
   pipe_so_target_reference(&so_target, NULL);
}

void util_blitter_clear_buffer(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned offset, unsigned size,
                               unsigned num_channels,
                               const union pipe_color_union *clear_value)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_vertex_buffer vb = {0};
   struct pipe_stream_output_target *so_target;

   assert(num_channels >= 1);
   assert(num_channels <= 4);

   /* IMPORTANT:  DON'T DO ANY BOUNDS CHECKING HERE!
    *
    * R600 uses this to initialize texture resources, so width0 might not be
    * what you think it is.
    */

   /* Streamout is required. */
   if (!ctx->has_stream_out) {
      assert(!"Streamout unsupported in util_blitter_clear_buffer()");
      return;
   }

   /* Some alignment is required. */
   if (offset % 4 != 0 || size % 4 != 0) {
      assert(!"Bad alignment in util_blitter_clear_buffer()");
      return;
   }

   u_upload_data(ctx->upload, 0, num_channels*4, clear_value,
                 &vb.buffer_offset, &vb.buffer);
   vb.stride = 0;

   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_disable_render_cond(ctx);

   pipe->set_vertex_buffers(pipe, ctx->base.vb_slot, 1, &vb);
   pipe->bind_vertex_elements_state(pipe,
                                    ctx->velem_state_readbuf[num_channels-1]);
   pipe->bind_vs_state(pipe, ctx->vs_pos_only);
   if (ctx->has_geometry_shader)
      pipe->bind_gs_state(pipe, NULL);
   pipe->bind_rasterizer_state(pipe, ctx->rs_discard_state);

   so_target = pipe->create_stream_output_target(pipe, dst, offset, size);
   pipe->set_stream_output_targets(pipe, 1, &so_target, 0);

   util_draw_arrays(pipe, PIPE_PRIM_POINTS, 0, size / 4);

   blitter_restore_vertex_states(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
   pipe_so_target_reference(&so_target, NULL);
   pipe_resource_reference(&vb.buffer, NULL);
}

/* probably radeon specific */
void util_blitter_custom_resolve_color(struct blitter_context *blitter,
				       struct pipe_resource *dst,
				       unsigned dst_level,
				       unsigned dst_layer,
				       struct pipe_resource *src,
				       unsigned src_layer,
				       unsigned sample_mask,
				       void *custom_blend,
                                       enum pipe_format format)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   struct pipe_surface *srcsurf, *dstsurf, surf_tmpl;

   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, custom_blend);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);
   ctx->bind_fs_state(pipe, ctx->fs_write_one_cbuf);
   pipe->set_sample_mask(pipe, sample_mask);

   memset(&surf_tmpl, 0, sizeof(surf_tmpl));
   surf_tmpl.format = format;
   surf_tmpl.u.tex.level = dst_level;
   surf_tmpl.u.tex.first_layer = dst_layer;
   surf_tmpl.u.tex.last_layer = dst_layer;

   dstsurf = pipe->create_surface(pipe, dst, &surf_tmpl);

   surf_tmpl.u.tex.level = 0;
   surf_tmpl.u.tex.first_layer = src_layer;
   surf_tmpl.u.tex.last_layer = src_layer;

   srcsurf = pipe->create_surface(pipe, src, &surf_tmpl);

   /* set a framebuffer state */
   fb_state.width = src->width0;
   fb_state.height = src->height0;
   fb_state.nr_cbufs = 2;
   fb_state.cbufs[0] = srcsurf;
   fb_state.cbufs[1] = dstsurf;
   fb_state.zsbuf = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, src->width0, src->height0);
   blitter->draw_rectangle(blitter, 0, 0, src->width0, src->height0,
                           0, 0, NULL);
   blitter_restore_fb_state(ctx);
   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);

   pipe_surface_reference(&srcsurf, NULL);
   pipe_surface_reference(&dstsurf, NULL);
}

void util_blitter_custom_color(struct blitter_context *blitter,
                               struct pipe_surface *dstsurf,
                               void *custom_blend)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   blitter_set_running_flag(ctx);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, custom_blend ? custom_blend
                                             : ctx->blend[PIPE_MASK_RGBA]);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   ctx->bind_fs_state(pipe, ctx->fs_write_one_cbuf);
   pipe->bind_vertex_elements_state(pipe, ctx->velem_state);
   pipe->set_sample_mask(pipe, (1ull << MAX2(1, dstsurf->texture->nr_samples)) - 1);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dstsurf;
   fb_state.zsbuf = 0;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);

   blitter_set_common_draw_rect_state(ctx, FALSE);
   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter->draw_rectangle(blitter, 0, 0, dstsurf->width, dstsurf->height,
                           0, 0, NULL);

   blitter_restore_vertex_states(ctx);
   blitter_restore_fragment_states(ctx);
   blitter_restore_fb_state(ctx);
   blitter_restore_render_cond(ctx);
   blitter_unset_running_flag(ctx);
}
