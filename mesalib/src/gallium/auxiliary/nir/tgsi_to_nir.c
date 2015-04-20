/*
 * Copyright © 2014-2015 Broadcom
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/ralloc.h"
#include "glsl/nir/nir.h"
#include "glsl/nir/nir_builder.h"
#include "glsl/list.h"
#include "glsl/shader_enums.h"

#include "nir/tgsi_to_nir.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_scan.h"

#define SWIZ(X, Y, Z, W) (unsigned[4]){      \
      TGSI_SWIZZLE_##X,                      \
      TGSI_SWIZZLE_##Y,                      \
      TGSI_SWIZZLE_##Z,                      \
      TGSI_SWIZZLE_##W,                      \
   }

struct ttn_reg_info {
   /** nir register containing this TGSI index. */
   nir_register *reg;
   nir_variable *var;
   /** Offset (in vec4s) from the start of var for this TGSI index. */
   int offset;
};

struct ttn_compile {
   union tgsi_full_token *token;
   nir_builder build;
   struct tgsi_shader_info *scan;

   struct ttn_reg_info *output_regs;
   struct ttn_reg_info *temp_regs;
   nir_ssa_def **imm_defs;

   nir_register *addr_reg;

   /**
    * Stack of cf_node_lists where instructions should be pushed as we pop
    * back out of the control flow stack.
    *
    * For each IF/ELSE/ENDIF block, if_stack[if_stack_pos] has where the else
    * instructions should be placed, and if_stack[if_stack_pos - 1] has where
    * the next instructions outside of the if/then/else block go.
    */
   struct exec_list **if_stack;
   unsigned if_stack_pos;

   /**
    * Stack of cf_node_lists where instructions should be pushed as we pop
    * back out of the control flow stack.
    *
    * loop_stack[loop_stack_pos - 1] contains the cf_node_list for the outside
    * of the loop.
    */
   struct exec_list **loop_stack;
   unsigned loop_stack_pos;

   /* How many TGSI_FILE_IMMEDIATE vec4s have been parsed so far. */
   unsigned next_imm;
};

#define ttn_swizzle(b, src, x, y, z, w) \
   nir_swizzle(b, src, SWIZ(x, y, z, w), 4, false)
#define ttn_channel(b, src, swiz) \
   nir_swizzle(b, src, SWIZ(swiz, swiz, swiz, swiz), 1, false)

static nir_ssa_def *
ttn_src_for_dest(nir_builder *b, nir_alu_dest *dest)
{
   nir_alu_src src;
   memset(&src, 0, sizeof(src));

   if (dest->dest.is_ssa)
      src.src = nir_src_for_ssa(&dest->dest.ssa);
   else {
      assert(!dest->dest.reg.indirect);
      src.src = nir_src_for_reg(dest->dest.reg.reg);
      src.src.reg.base_offset = dest->dest.reg.base_offset;
   }

   for (int i = 0; i < 4; i++)
      src.swizzle[i] = i;

   return nir_fmov_alu(b, src, 4);
}

static void
ttn_emit_declaration(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_declaration *decl = &c->token->FullDeclaration;
   unsigned array_size = decl->Range.Last - decl->Range.First + 1;
   unsigned file = decl->Declaration.File;
   unsigned i;

   if (file == TGSI_FILE_TEMPORARY) {
      if (decl->Declaration.Array) {
         /* for arrays, we create variables instead of registers: */
         nir_variable *var = rzalloc(b->shader, nir_variable);

         var->type = glsl_array_type(glsl_vec4_type(), array_size);
         var->data.mode = nir_var_global;
         var->name = ralloc_asprintf(var, "arr_%d", decl->Array.ArrayID);

         exec_list_push_tail(&b->shader->globals, &var->node);

         for (i = 0; i < array_size; i++) {
            /* point all the matching slots to the same var,
             * with appropriate offset set, mostly just so
             * we know what to do when tgsi does a non-indirect
             * access
             */
            c->temp_regs[decl->Range.First + i].reg = NULL;
            c->temp_regs[decl->Range.First + i].var = var;
            c->temp_regs[decl->Range.First + i].offset = i;
         }
      } else {
         for (i = 0; i < array_size; i++) {
            nir_register *reg = nir_local_reg_create(b->impl);
            reg->num_components = 4;
            c->temp_regs[decl->Range.First + i].reg = reg;
            c->temp_regs[decl->Range.First + i].var = NULL;
            c->temp_regs[decl->Range.First + i].offset = 0;
         }
      }
   } else if (file == TGSI_FILE_ADDRESS) {
      c->addr_reg = nir_local_reg_create(b->impl);
      c->addr_reg->num_components = 4;
   } else if (file == TGSI_FILE_SYSTEM_VALUE) {
      /* Nothing to record for system values. */
   } else if (file == TGSI_FILE_SAMPLER) {
      /* Nothing to record for samplers. */
   } else {
      nir_variable *var;
      assert(file == TGSI_FILE_INPUT ||
             file == TGSI_FILE_OUTPUT ||
             file == TGSI_FILE_CONSTANT);

      /* nothing to do for UBOs: */
      if ((file == TGSI_FILE_CONSTANT) && decl->Declaration.Dimension)
         return;

      var = rzalloc(b->shader, nir_variable);
      var->data.driver_location = decl->Range.First;

      var->type = glsl_vec4_type();
      if (array_size > 1)
         var->type = glsl_array_type(var->type, array_size);

      switch (file) {
      case TGSI_FILE_INPUT:
         var->data.read_only = true;
         var->data.mode = nir_var_shader_in;
         var->name = ralloc_asprintf(var, "in_%d", decl->Range.First);

         /* We should probably translate to a VERT_ATTRIB_* or VARYING_SLOT_*
          * instead, but nothing in NIR core is looking at the value
          * currently, and this is less change to drivers.
          */
         var->data.location = decl->Semantic.Name;
         var->data.index = decl->Semantic.Index;

         /* We definitely need to translate the interpolation field, because
          * nir_print will decode it.
          */
         switch (decl->Interp.Interpolate) {
         case TGSI_INTERPOLATE_CONSTANT:
            var->data.interpolation = INTERP_QUALIFIER_FLAT;
            break;
         case TGSI_INTERPOLATE_LINEAR:
            var->data.interpolation = INTERP_QUALIFIER_NOPERSPECTIVE;
            break;
         case TGSI_INTERPOLATE_PERSPECTIVE:
            var->data.interpolation = INTERP_QUALIFIER_SMOOTH;
            break;
         }

         exec_list_push_tail(&b->shader->inputs, &var->node);
         break;
      case TGSI_FILE_OUTPUT: {
         /* Since we can't load from outputs in the IR, we make temporaries
          * for the outputs and emit stores to the real outputs at the end of
          * the shader.
          */
         nir_register *reg = nir_local_reg_create(b->impl);
         reg->num_components = 4;
         if (array_size > 1)
            reg->num_array_elems = array_size;

         var->data.mode = nir_var_shader_out;
         var->name = ralloc_asprintf(var, "out_%d", decl->Range.First);

         var->data.location = decl->Semantic.Name;
         var->data.index = decl->Semantic.Index;

         for (i = 0; i < array_size; i++) {
            c->output_regs[decl->Range.First + i].offset = i;
            c->output_regs[decl->Range.First + i].reg = reg;
         }

         exec_list_push_tail(&b->shader->outputs, &var->node);
      }
         break;
      case TGSI_FILE_CONSTANT:
         var->data.mode = nir_var_uniform;
         var->name = ralloc_asprintf(var, "uniform_%d", decl->Range.First);

         exec_list_push_tail(&b->shader->uniforms, &var->node);
         break;
      default:
         unreachable("bad declaration file");
         return;
      }

   }
}

static void
ttn_emit_immediate(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_immediate *tgsi_imm = &c->token->FullImmediate;
   nir_load_const_instr *load_const;
   int i;

   load_const = nir_load_const_instr_create(b->shader, 4);
   c->imm_defs[c->next_imm] = &load_const->def;
   c->next_imm++;

   for (i = 0; i < 4; i++)
      load_const->value.u[i] = tgsi_imm->u[i].Uint;

   nir_instr_insert_after_cf_list(b->cf_node_list, &load_const->instr);
}

static nir_src
ttn_src_for_indirect(struct ttn_compile *c, struct tgsi_ind_register *indirect);

/* generate either a constant or indirect deref chain for accessing an
 * array variable.
 */
static nir_deref_var *
ttn_array_deref(struct ttn_compile *c, nir_intrinsic_instr *instr,
                nir_variable *var, unsigned offset,
                struct tgsi_ind_register *indirect)
{
   nir_deref_var *deref = nir_deref_var_create(instr, var);
   nir_deref_array *arr = nir_deref_array_create(deref);

   arr->base_offset = offset;
   arr->deref.type = glsl_get_array_element(var->type);

   if (indirect) {
      arr->deref_array_type = nir_deref_array_type_indirect;
      arr->indirect = ttn_src_for_indirect(c, indirect);
   } else {
      arr->deref_array_type = nir_deref_array_type_direct;
   }

   deref->deref.child = &arr->deref;

   return deref;
}

static nir_src
ttn_src_for_file_and_index(struct ttn_compile *c, unsigned file, unsigned index,
                           struct tgsi_ind_register *indirect,
                           struct tgsi_dimension *dim,
                           struct tgsi_ind_register *dimind)
{
   nir_builder *b = &c->build;
   nir_src src;

   memset(&src, 0, sizeof(src));

   switch (file) {
   case TGSI_FILE_TEMPORARY:
      if (c->temp_regs[index].var) {
         unsigned offset = c->temp_regs[index].offset;
         nir_variable *var = c->temp_regs[index].var;
         nir_intrinsic_instr *load;

         load = nir_intrinsic_instr_create(b->shader,
                                           nir_intrinsic_load_var);
         load->num_components = 4;
         load->variables[0] = ttn_array_deref(c, load, var, offset, indirect);

         nir_ssa_dest_init(&load->instr, &load->dest, 4, NULL);
         nir_instr_insert_after_cf_list(b->cf_node_list, &load->instr);

         src = nir_src_for_ssa(&load->dest.ssa);

      } else {
         assert(!indirect);
         src.reg.reg = c->temp_regs[index].reg;
      }
      assert(!dim);
      break;

   case TGSI_FILE_ADDRESS:
      src.reg.reg = c->addr_reg;
      assert(!dim);
      break;

   case TGSI_FILE_IMMEDIATE:
      src = nir_src_for_ssa(c->imm_defs[index]);
      assert(!indirect);
      assert(!dim);
      break;

   case TGSI_FILE_SYSTEM_VALUE: {
      nir_intrinsic_instr *load;
      nir_intrinsic_op op;
      unsigned ncomp = 1;

      assert(!indirect);
      assert(!dim);

      switch (c->scan->system_value_semantic_name[index]) {
      case TGSI_SEMANTIC_VERTEXID_NOBASE:
         op = nir_intrinsic_load_vertex_id_zero_base;
         break;
      case TGSI_SEMANTIC_VERTEXID:
         op = nir_intrinsic_load_vertex_id;
         break;
      case TGSI_SEMANTIC_BASEVERTEX:
         op = nir_intrinsic_load_base_vertex;
         break;
      case TGSI_SEMANTIC_INSTANCEID:
         op = nir_intrinsic_load_instance_id;
         break;
      default:
         unreachable("bad system value");
      }

      load = nir_intrinsic_instr_create(b->shader, op);
      load->num_components = ncomp;

      nir_ssa_dest_init(&load->instr, &load->dest, ncomp, NULL);
      nir_instr_insert_after_cf_list(b->cf_node_list, &load->instr);

      src = nir_src_for_ssa(&load->dest.ssa);
      break;
   }

   case TGSI_FILE_INPUT:
   case TGSI_FILE_CONSTANT: {
      nir_intrinsic_instr *load;
      nir_intrinsic_op op;
      unsigned srcn = 0;

      switch (file) {
      case TGSI_FILE_INPUT:
         op = indirect ? nir_intrinsic_load_input_indirect :
                         nir_intrinsic_load_input;
         assert(!dim);
         break;
      case TGSI_FILE_CONSTANT:
         if (dim) {
            op = indirect ? nir_intrinsic_load_ubo_indirect :
                            nir_intrinsic_load_ubo;
            /* convert index from vec4 to byte: */
            index *= 16;
         } else {
            op = indirect ? nir_intrinsic_load_uniform_indirect :
                            nir_intrinsic_load_uniform;
         }
         break;
      default:
         unreachable("No other load files supported");
         break;
      }

      load = nir_intrinsic_instr_create(b->shader, op);

      load->num_components = 4;
      load->const_index[0] = index;
      load->const_index[1] = 1;
      if (dim) {
         if (dimind) {
            load->src[srcn] =
               ttn_src_for_file_and_index(c, dimind->File, dimind->Index,
                                          NULL, NULL, NULL);
         } else {
            /* UBOs start at index 1 in TGSI: */
            load->src[srcn] =
               nir_src_for_ssa(nir_imm_int(b, dim->Index - 1));
         }
         srcn++;
      }
      if (indirect) {
         load->src[srcn] = ttn_src_for_indirect(c, indirect);
         if (dim) {
            assert(load->src[srcn].is_ssa);
            /* we also need to covert vec4 to byte here too: */
            load->src[srcn] =
               nir_src_for_ssa(nir_ishl(b, load->src[srcn].ssa,
                                        nir_imm_int(b, 4)));
         }
         srcn++;
      }
      nir_ssa_dest_init(&load->instr, &load->dest, 4, NULL);
      nir_instr_insert_after_cf_list(b->cf_node_list, &load->instr);

      src = nir_src_for_ssa(&load->dest.ssa);
      break;
   }

   default:
      unreachable("bad src file");
   }


   return src;
}

static nir_src
ttn_src_for_indirect(struct ttn_compile *c, struct tgsi_ind_register *indirect)
{
   nir_builder *b = &c->build;
   nir_alu_src src;
   memset(&src, 0, sizeof(src));
   for (int i = 0; i < 4; i++)
      src.swizzle[i] = indirect->Swizzle;
   src.src = ttn_src_for_file_and_index(c,
                                        indirect->File,
                                        indirect->Index,
                                        NULL, NULL, NULL);
   return nir_src_for_ssa(nir_imov_alu(b, src, 1));
}

static nir_alu_dest
ttn_get_dest(struct ttn_compile *c, struct tgsi_full_dst_register *tgsi_fdst)
{
   struct tgsi_dst_register *tgsi_dst = &tgsi_fdst->Register;
   nir_alu_dest dest;
   unsigned index = tgsi_dst->Index;

   memset(&dest, 0, sizeof(dest));

   if (tgsi_dst->File == TGSI_FILE_TEMPORARY) {
      if (c->temp_regs[index].var) {
          nir_builder *b = &c->build;
          nir_intrinsic_instr *load;
          struct tgsi_ind_register *indirect =
                tgsi_dst->Indirect ? &tgsi_fdst->Indirect : NULL;
          nir_register *reg;

         /* this works, because TGSI will give us a base offset
          * (in case of indirect index) that points back into
          * the array.  Access can be direct or indirect, we
          * don't really care.  Just create a one-shot dst reg
          * that will get store_var'd back into the array var
          * at the end of ttn_emit_instruction()
          */
         reg = nir_local_reg_create(c->build.impl);
         reg->num_components = 4;
         dest.dest.reg.reg = reg;
         dest.dest.reg.base_offset = 0;

         /* since the alu op might not write to all components
          * of the temporary, we must first do a load_var to
          * get the previous array elements into the register.
          * This is one area that NIR could use a bit of
          * improvement (or opt pass to clean up the mess
          * once things are scalarized)
          */

         load = nir_intrinsic_instr_create(c->build.shader,
                                           nir_intrinsic_load_var);
         load->num_components = 4;
         load->variables[0] =
               ttn_array_deref(c, load, c->temp_regs[index].var,
                               c->temp_regs[index].offset,
                               indirect);

         load->dest = nir_dest_for_reg(reg);

         nir_instr_insert_after_cf_list(b->cf_node_list, &load->instr);
      } else {
         assert(!tgsi_dst->Indirect);
         dest.dest.reg.reg = c->temp_regs[index].reg;
         dest.dest.reg.base_offset = c->temp_regs[index].offset;
      }
   } else if (tgsi_dst->File == TGSI_FILE_OUTPUT) {
      dest.dest.reg.reg = c->output_regs[index].reg;
      dest.dest.reg.base_offset = c->output_regs[index].offset;
   } else if (tgsi_dst->File == TGSI_FILE_ADDRESS) {
      assert(index == 0);
      dest.dest.reg.reg = c->addr_reg;
   }

   dest.write_mask = tgsi_dst->WriteMask;
   dest.saturate = false;

   if (tgsi_dst->Indirect && (tgsi_dst->File != TGSI_FILE_TEMPORARY)) {
      nir_src *indirect = ralloc(c->build.shader, nir_src);
      *indirect = ttn_src_for_indirect(c, &tgsi_fdst->Indirect);
      dest.dest.reg.indirect = indirect;
   }

   return dest;
}

static nir_variable *
ttn_get_var(struct ttn_compile *c, struct tgsi_full_dst_register *tgsi_fdst)
{
   struct tgsi_dst_register *tgsi_dst = &tgsi_fdst->Register;
   unsigned index = tgsi_dst->Index;

   if (tgsi_dst->File == TGSI_FILE_TEMPORARY) {
      /* we should not have an indirect when there is no var! */
      if (!c->temp_regs[index].var)
         assert(!tgsi_dst->Indirect);
      return c->temp_regs[index].var;
   }

   return NULL;
}

static nir_ssa_def *
ttn_get_src(struct ttn_compile *c, struct tgsi_full_src_register *tgsi_fsrc)
{
   nir_builder *b = &c->build;
   struct tgsi_src_register *tgsi_src = &tgsi_fsrc->Register;
   unsigned tgsi_opcode = c->token->FullInstruction.Instruction.Opcode;
   unsigned tgsi_src_type = tgsi_opcode_infer_src_type(tgsi_opcode);
   bool src_is_float = !(tgsi_src_type == TGSI_TYPE_SIGNED ||
                         tgsi_src_type == TGSI_TYPE_UNSIGNED);
   nir_alu_src src;

   memset(&src, 0, sizeof(src));

   if (tgsi_src->File == TGSI_FILE_NULL) {
      return nir_imm_float(b, 0.0);
   } else if (tgsi_src->File == TGSI_FILE_SAMPLER) {
      /* Only the index of the sampler gets used in texturing, and it will
       * handle looking that up on its own instead of using the nir_alu_src.
       */
      assert(!tgsi_src->Indirect);
      return NULL;
   } else {
      struct tgsi_ind_register *ind = NULL;
      struct tgsi_dimension *dim = NULL;
      struct tgsi_ind_register *dimind = NULL;
      if (tgsi_src->Indirect)
         ind = &tgsi_fsrc->Indirect;
      if (tgsi_src->Dimension) {
         dim = &tgsi_fsrc->Dimension;
         if (dim->Indirect)
            dimind = &tgsi_fsrc->DimIndirect;
      }
      src.src = ttn_src_for_file_and_index(c,
                                           tgsi_src->File,
                                           tgsi_src->Index,
                                           ind, dim, dimind);
   }

   src.swizzle[0] = tgsi_src->SwizzleX;
   src.swizzle[1] = tgsi_src->SwizzleY;
   src.swizzle[2] = tgsi_src->SwizzleZ;
   src.swizzle[3] = tgsi_src->SwizzleW;

   nir_ssa_def *def = nir_fmov_alu(b, src, 4);

   if (tgsi_src->Absolute) {
      if (src_is_float)
         def = nir_fabs(b, def);
      else
         def = nir_iabs(b, def);
   }

   if (tgsi_src->Negate) {
      if (src_is_float)
         def = nir_fneg(b, def);
      else
         def = nir_ineg(b, def);
   }

   return def;
}

static void
ttn_alu(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   unsigned num_srcs = nir_op_infos[op].num_inputs;
   nir_alu_instr *instr = nir_alu_instr_create(b->shader, op);
   unsigned i;

   for (i = 0; i < num_srcs; i++)
      instr->src[i].src = nir_src_for_ssa(src[i]);

   instr->dest = dest;
   nir_instr_insert_after_cf_list(b->cf_node_list, &instr->instr);
}

static void
ttn_move_dest_masked(nir_builder *b, nir_alu_dest dest,
                     nir_ssa_def *def, unsigned write_mask)
{
   if (!(dest.write_mask & write_mask))
      return;

   nir_alu_instr *mov = nir_alu_instr_create(b->shader, nir_op_imov);
   mov->dest = dest;
   mov->dest.write_mask &= write_mask;
   mov->src[0].src = nir_src_for_ssa(def);
   for (unsigned i = def->num_components; i < 4; i++)
      mov->src[0].swizzle[i] = def->num_components - 1;
   nir_instr_insert_after_cf_list(b->cf_node_list, &mov->instr);
}

static void
ttn_move_dest(nir_builder *b, nir_alu_dest dest, nir_ssa_def *def)
{
   ttn_move_dest_masked(b, dest, def, TGSI_WRITEMASK_XYZW);
}

static void
ttn_arl(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_f2i(b, nir_ffloor(b, src[0])));
}

/* EXP - Approximate Exponential Base 2
 *  dst.x = 2^{\lfloor src.x\rfloor}
 *  dst.y = src.x - \lfloor src.x\rfloor
 *  dst.z = 2^{src.x}
 *  dst.w = 1.0
 */
static void
ttn_exp(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_ssa_def *srcx = ttn_channel(b, src[0], X);

   ttn_move_dest_masked(b, dest, nir_fexp2(b, nir_ffloor(b, srcx)),
                        TGSI_WRITEMASK_X);
   ttn_move_dest_masked(b, dest, nir_fsub(b, srcx, nir_ffloor(b, srcx)),
                        TGSI_WRITEMASK_Y);
   ttn_move_dest_masked(b, dest, nir_fexp2(b, srcx), TGSI_WRITEMASK_Z);
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_W);
}

/* LOG - Approximate Logarithm Base 2
 *  dst.x = \lfloor\log_2{|src.x|}\rfloor
 *  dst.y = \frac{|src.x|}{2^{\lfloor\log_2{|src.x|}\rfloor}}
 *  dst.z = \log_2{|src.x|}
 *  dst.w = 1.0
 */
static void
ttn_log(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_ssa_def *abs_srcx = nir_fabs(b, ttn_channel(b, src[0], X));
   nir_ssa_def *log2 = nir_flog2(b, abs_srcx);

   ttn_move_dest_masked(b, dest, nir_ffloor(b, log2), TGSI_WRITEMASK_X);
   ttn_move_dest_masked(b, dest,
                        nir_fdiv(b, abs_srcx, nir_fexp2(b, nir_ffloor(b, log2))),
                        TGSI_WRITEMASK_Y);
   ttn_move_dest_masked(b, dest, nir_flog2(b, abs_srcx), TGSI_WRITEMASK_Z);
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_W);
}

/* DST - Distance Vector
 *   dst.x = 1.0
 *   dst.y = src0.y \times src1.y
 *   dst.z = src0.z
 *   dst.w = src1.w
 */
static void
ttn_dst(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_X);
   ttn_move_dest_masked(b, dest, nir_fmul(b, src[0], src[1]), TGSI_WRITEMASK_Y);
   ttn_move_dest_masked(b, dest, nir_fmov(b, src[0]), TGSI_WRITEMASK_Z);
   ttn_move_dest_masked(b, dest, nir_fmov(b, src[1]), TGSI_WRITEMASK_W);
}

/* LIT - Light Coefficients
 *  dst.x = 1.0
 *  dst.y = max(src.x, 0.0)
 *  dst.z = (src.x > 0.0) ? max(src.y, 0.0)^{clamp(src.w, -128.0, 128.0))} : 0
 *  dst.w = 1.0
 */
static void
ttn_lit(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_XW);

   ttn_move_dest_masked(b, dest, nir_fmax(b, ttn_channel(b, src[0], X),
                                          nir_imm_float(b, 0.0)), TGSI_WRITEMASK_Y);

   if (dest.write_mask & TGSI_WRITEMASK_Z) {
      nir_ssa_def *src0_y = ttn_channel(b, src[0], Y);
      nir_ssa_def *wclamp = nir_fmax(b, nir_fmin(b, ttn_channel(b, src[0], W),
                                                 nir_imm_float(b, 128.0)),
                                     nir_imm_float(b, -128.0));
      nir_ssa_def *pow = nir_fpow(b, nir_fmax(b, src0_y, nir_imm_float(b, 0.0)),
                                  wclamp);

      ttn_move_dest_masked(b, dest,
                           nir_bcsel(b,
                                     nir_fge(b,
                                             nir_imm_float(b, 0.0),
                                             ttn_channel(b, src[0], X)),
                                     nir_imm_float(b, 0.0),
                                     pow),
                           TGSI_WRITEMASK_Z);
   }
}

/* SCS - Sine Cosine
 *   dst.x = \cos{src.x}
 *   dst.y = \sin{src.x}
 *   dst.z = 0.0
 *   dst.w = 1.0
 */
static void
ttn_scs(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest_masked(b, dest, nir_fcos(b, ttn_channel(b, src[0], X)),
                        TGSI_WRITEMASK_X);
   ttn_move_dest_masked(b, dest, nir_fsin(b, ttn_channel(b, src[0], X)),
                        TGSI_WRITEMASK_Y);
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 0.0), TGSI_WRITEMASK_Z);
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_W);
}

static void
ttn_sle(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_sge(b, src[1], src[0]));
}

static void
ttn_sgt(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_slt(b, src[1], src[0]));
}

static void
ttn_clamp(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_fmin(b, nir_fmax(b, src[0], src[1]), src[2]));
}

static void
ttn_xpd(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest_masked(b, dest,
                        nir_fsub(b,
                                 nir_fmul(b,
                                          ttn_swizzle(b, src[0], Y, Z, X, X),
                                          ttn_swizzle(b, src[1], Z, X, Y, X)),
                                 nir_fmul(b,
                                          ttn_swizzle(b, src[1], Y, Z, X, X),
                                          ttn_swizzle(b, src[0], Z, X, Y, X))),
                        TGSI_WRITEMASK_XYZ);
   ttn_move_dest_masked(b, dest, nir_imm_float(b, 1.0), TGSI_WRITEMASK_W);
}

static void
ttn_dp2a(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest,
                 ttn_channel(b, nir_fadd(b, nir_fdot2(b, src[0], src[1]),
                                         src[2]),
                             X));
}

static void
ttn_dp2(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_fdot2(b, src[0], src[1]));
}

static void
ttn_dp3(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_fdot3(b, src[0], src[1]));
}

static void
ttn_dp4(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_fdot4(b, src[0], src[1]));
}

static void
ttn_dph(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_fadd(b, nir_fdot3(b, src[0], src[1]),
                                   ttn_channel(b, src[1], W)));
}

static void
ttn_umad(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_iadd(b, nir_imul(b, src[0], src[1]), src[2]));
}

static void
ttn_arr(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_ffloor(b, nir_fadd(b, src[0], nir_imm_float(b, 0.5))));
}

static void
ttn_cmp(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_bcsel(b,
                                    nir_flt(b, src[0], nir_imm_float(b, 0.0)),
                                    src[1], src[2]));
}

static void
ttn_ucmp(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   ttn_move_dest(b, dest, nir_bcsel(b,
                                    nir_ine(b, src[0], nir_imm_int(b, 0)),
                                    src[1], src[2]));
}

static void
ttn_kill(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_intrinsic_instr *discard =
      nir_intrinsic_instr_create(b->shader, nir_intrinsic_discard);
   nir_instr_insert_after_cf_list(b->cf_node_list, &discard->instr);
}

static void
ttn_kill_if(nir_builder *b, nir_op op, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_ssa_def *cmp = nir_bany4(b, nir_flt(b, src[0], nir_imm_float(b, 0.0)));
   nir_intrinsic_instr *discard =
      nir_intrinsic_instr_create(b->shader, nir_intrinsic_discard_if);
   discard->src[0] = nir_src_for_ssa(cmp);
   nir_instr_insert_after_cf_list(b->cf_node_list, &discard->instr);
}

static void
ttn_if(struct ttn_compile *c, nir_ssa_def *src, bool is_uint)
{
   nir_builder *b = &c->build;

   /* Save the outside-of-the-if-statement node list. */
   c->if_stack[c->if_stack_pos] = b->cf_node_list;
   c->if_stack_pos++;

   src = ttn_channel(b, src, X);

   nir_if *if_stmt = nir_if_create(b->shader);
   if (is_uint) {
      if_stmt->condition = nir_src_for_ssa(nir_ine(b, src, nir_imm_int(b, 0)));
   } else {
      if_stmt->condition = nir_src_for_ssa(nir_fne(b, src, nir_imm_int(b, 0)));
   }
   nir_cf_node_insert_end(b->cf_node_list, &if_stmt->cf_node);

   nir_builder_insert_after_cf_list(b, &if_stmt->then_list);

   c->if_stack[c->if_stack_pos] = &if_stmt->else_list;
   c->if_stack_pos++;
}

static void
ttn_else(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   nir_builder_insert_after_cf_list(b, c->if_stack[c->if_stack_pos - 1]);
}

static void
ttn_endif(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   c->if_stack_pos -= 2;
   nir_builder_insert_after_cf_list(b, c->if_stack[c->if_stack_pos]);
}

static void
ttn_bgnloop(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   /* Save the outside-of-the-loop node list. */
   c->loop_stack[c->loop_stack_pos] = b->cf_node_list;
   c->loop_stack_pos++;

   nir_loop *loop = nir_loop_create(b->shader);
   nir_cf_node_insert_end(b->cf_node_list, &loop->cf_node);

   nir_builder_insert_after_cf_list(b, &loop->body);
}

static void
ttn_cont(nir_builder *b)
{
   nir_jump_instr *instr = nir_jump_instr_create(b->shader, nir_jump_continue);
   nir_instr_insert_after_cf_list(b->cf_node_list, &instr->instr);
}

static void
ttn_brk(nir_builder *b)
{
   nir_jump_instr *instr = nir_jump_instr_create(b->shader, nir_jump_break);
   nir_instr_insert_after_cf_list(b->cf_node_list, &instr->instr);
}

static void
ttn_endloop(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   c->loop_stack_pos--;
   nir_builder_insert_after_cf_list(b, c->loop_stack[c->loop_stack_pos]);
}

static void
setup_texture_info(nir_tex_instr *instr, unsigned texture)
{
   switch (texture) {
   case TGSI_TEXTURE_1D:
      instr->sampler_dim = GLSL_SAMPLER_DIM_1D;
      break;
   case TGSI_TEXTURE_1D_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_1D;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_SHADOW1D:
      instr->sampler_dim = GLSL_SAMPLER_DIM_1D;
      instr->is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_1D;
      instr->is_shadow = true;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_2D:
      instr->sampler_dim = GLSL_SAMPLER_DIM_2D;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_2D;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_2D_MSAA:
      instr->sampler_dim = GLSL_SAMPLER_DIM_MS;
      break;
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      instr->sampler_dim = GLSL_SAMPLER_DIM_MS;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_SHADOW2D:
      instr->sampler_dim = GLSL_SAMPLER_DIM_2D;
      instr->is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_2D;
      instr->is_shadow = true;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_3D:
      instr->sampler_dim = GLSL_SAMPLER_DIM_3D;
      break;
   case TGSI_TEXTURE_CUBE:
      instr->sampler_dim = GLSL_SAMPLER_DIM_CUBE;
      break;
   case TGSI_TEXTURE_CUBE_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_CUBE;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_SHADOWCUBE:
      instr->sampler_dim = GLSL_SAMPLER_DIM_CUBE;
      instr->is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      instr->sampler_dim = GLSL_SAMPLER_DIM_CUBE;
      instr->is_shadow = true;
      instr->is_array = true;
      break;
   case TGSI_TEXTURE_RECT:
      instr->sampler_dim = GLSL_SAMPLER_DIM_RECT;
      break;
   case TGSI_TEXTURE_SHADOWRECT:
      instr->sampler_dim = GLSL_SAMPLER_DIM_RECT;
      instr->is_shadow = true;
      break;
   default:
      fprintf(stderr, "Unknown TGSI texture target %d\n", texture);
      abort();
   }
}

static void
ttn_tex(struct ttn_compile *c, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   nir_tex_instr *instr;
   nir_texop op;
   unsigned num_srcs, samp = 1, i;

   switch (tgsi_inst->Instruction.Opcode) {
   case TGSI_OPCODE_TEX:
      op = nir_texop_tex;
      num_srcs = 1;
      break;
   case TGSI_OPCODE_TXP:
      op = nir_texop_tex;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXB:
      op = nir_texop_txb;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXL:
      op = nir_texop_txl;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXL2:
      op = nir_texop_txl;
      num_srcs = 2;
      samp = 2;
      break;
   case TGSI_OPCODE_TXF:
      op = nir_texop_txf;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXD:
      op = nir_texop_txd;
      num_srcs = 3;
      samp = 3;
      break;

   default:
      fprintf(stderr, "unknown TGSI tex op %d\n", tgsi_inst->Instruction.Opcode);
      abort();
   }

   if (tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW1D ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW1D_ARRAY ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW2D ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW2D_ARRAY ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWRECT ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWCUBE ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
      num_srcs++;
   }

   num_srcs += tgsi_inst->Texture.NumOffsets;

   instr = nir_tex_instr_create(b->shader, num_srcs);
   instr->op = op;

   setup_texture_info(instr, tgsi_inst->Texture.Texture);

   switch (instr->sampler_dim) {
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_BUF:
      instr->coord_components = 1;
      break;
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_RECT:
   case GLSL_SAMPLER_DIM_EXTERNAL:
   case GLSL_SAMPLER_DIM_MS:
      instr->coord_components = 2;
      break;
   case GLSL_SAMPLER_DIM_3D:
   case GLSL_SAMPLER_DIM_CUBE:
      instr->coord_components = 3;
      break;
   }

   if (instr->is_array)
      instr->coord_components++;

   assert(tgsi_inst->Src[samp].Register.File == TGSI_FILE_SAMPLER);
   instr->sampler_index = tgsi_inst->Src[samp].Register.Index;

   unsigned src_number = 0;

   instr->src[src_number].src =
      nir_src_for_ssa(nir_swizzle(b, src[0], SWIZ(X, Y, Z, W),
                                  instr->coord_components, false));
   instr->src[src_number].src_type = nir_tex_src_coord;
   src_number++;

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXP) {
      instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      instr->src[src_number].src_type = nir_tex_src_projector;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXB) {
      instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      instr->src[src_number].src_type = nir_tex_src_bias;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXL) {
      instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      instr->src[src_number].src_type = nir_tex_src_lod;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXL2) {
      instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[1], X));
      instr->src[src_number].src_type = nir_tex_src_lod;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXF) {
      instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      instr->src[src_number].src_type = nir_tex_src_lod;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXD) {
      instr->src[src_number].src =
         nir_src_for_ssa(nir_swizzle(b, src[1], SWIZ(X, Y, Z, W),
              instr->coord_components, false));
      instr->src[src_number].src_type = nir_tex_src_ddx;
      src_number++;
      instr->src[src_number].src =
         nir_src_for_ssa(nir_swizzle(b, src[2], SWIZ(X, Y, Z, W),
              instr->coord_components, false));
      instr->src[src_number].src_type = nir_tex_src_ddy;
      src_number++;
   }

   if (instr->is_shadow) {
      if (instr->coord_components < 3)
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], Z));
      else
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));

      instr->src[src_number].src_type = nir_tex_src_comparitor;
      src_number++;
   }

   for (i = 0; i < tgsi_inst->Texture.NumOffsets; i++) {
      struct tgsi_texture_offset *tex_offset = &tgsi_inst->TexOffsets[i];
      /* since TexOffset ins't using tgsi_full_src_register we get to
       * do some extra gymnastics:
       */
      nir_alu_src src;

      memset(&src, 0, sizeof(src));

      src.src = ttn_src_for_file_and_index(c,
                                           tex_offset->File,
                                           tex_offset->Index,
                                           NULL, NULL, NULL);

      src.swizzle[0] = tex_offset->SwizzleX;
      src.swizzle[1] = tex_offset->SwizzleY;
      src.swizzle[2] = tex_offset->SwizzleZ;
      src.swizzle[3] = TGSI_SWIZZLE_W;

      instr->src[src_number].src_type = nir_tex_src_offset;
      instr->src[src_number].src = nir_src_for_ssa(
         nir_fmov_alu(b, src, nir_tex_instr_src_size(instr, src_number)));
      src_number++;
   }

   assert(src_number == num_srcs);

   nir_ssa_dest_init(&instr->instr, &instr->dest, 4, NULL);
   nir_instr_insert_after_cf_list(b->cf_node_list, &instr->instr);

   /* Resolve the writemask on the texture op. */
   ttn_move_dest(b, dest, &instr->dest.ssa);
}

/* TGSI_OPCODE_TXQ is actually two distinct operations:
 *
 *     dst.x = texture\_width(unit, lod)
 *     dst.y = texture\_height(unit, lod)
 *     dst.z = texture\_depth(unit, lod)
 *     dst.w = texture\_levels(unit)
 *
 * dst.xyz map to NIR txs opcode, and dst.w maps to query_levels
 */
static void
ttn_txq(struct ttn_compile *c, nir_alu_dest dest, nir_ssa_def **src)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   nir_tex_instr *txs, *qlv;

   txs = nir_tex_instr_create(b->shader, 1);
   txs->op = nir_texop_txs;
   setup_texture_info(txs, tgsi_inst->Texture.Texture);

   qlv = nir_tex_instr_create(b->shader, 0);
   qlv->op = nir_texop_query_levels;
   setup_texture_info(qlv, tgsi_inst->Texture.Texture);

   assert(tgsi_inst->Src[1].Register.File == TGSI_FILE_SAMPLER);
   txs->sampler_index = tgsi_inst->Src[1].Register.Index;
   qlv->sampler_index = tgsi_inst->Src[1].Register.Index;

   /* only single src, the lod: */
   txs->src[0].src = nir_src_for_ssa(ttn_channel(b, src[0], X));
   txs->src[0].src_type = nir_tex_src_lod;

   nir_ssa_dest_init(&txs->instr, &txs->dest, 3, NULL);
   nir_instr_insert_after_cf_list(b->cf_node_list, &txs->instr);

   nir_ssa_dest_init(&qlv->instr, &qlv->dest, 1, NULL);
   nir_instr_insert_after_cf_list(b->cf_node_list, &qlv->instr);

   ttn_move_dest_masked(b, dest, &txs->dest.ssa, TGSI_WRITEMASK_XYZ);
   ttn_move_dest_masked(b, dest, &qlv->dest.ssa, TGSI_WRITEMASK_W);
}

static const nir_op op_trans[TGSI_OPCODE_LAST] = {
   [TGSI_OPCODE_ARL] = 0,
   [TGSI_OPCODE_MOV] = nir_op_fmov,
   [TGSI_OPCODE_LIT] = 0,
   [TGSI_OPCODE_RCP] = nir_op_frcp,
   [TGSI_OPCODE_RSQ] = nir_op_frsq,
   [TGSI_OPCODE_EXP] = 0,
   [TGSI_OPCODE_LOG] = 0,
   [TGSI_OPCODE_MUL] = nir_op_fmul,
   [TGSI_OPCODE_ADD] = nir_op_fadd,
   [TGSI_OPCODE_DP3] = 0,
   [TGSI_OPCODE_DP4] = 0,
   [TGSI_OPCODE_DST] = 0,
   [TGSI_OPCODE_MIN] = nir_op_fmin,
   [TGSI_OPCODE_MAX] = nir_op_fmax,
   [TGSI_OPCODE_SLT] = nir_op_slt,
   [TGSI_OPCODE_SGE] = nir_op_sge,
   [TGSI_OPCODE_MAD] = nir_op_ffma,
   [TGSI_OPCODE_SUB] = nir_op_fsub,
   [TGSI_OPCODE_LRP] = 0,
   [TGSI_OPCODE_SQRT] = nir_op_fsqrt,
   [TGSI_OPCODE_DP2A] = 0,
   [TGSI_OPCODE_FRC] = nir_op_ffract,
   [TGSI_OPCODE_CLAMP] = 0,
   [TGSI_OPCODE_FLR] = nir_op_ffloor,
   [TGSI_OPCODE_ROUND] = nir_op_fround_even,
   [TGSI_OPCODE_EX2] = nir_op_fexp2,
   [TGSI_OPCODE_LG2] = nir_op_flog2,
   [TGSI_OPCODE_POW] = nir_op_fpow,
   [TGSI_OPCODE_XPD] = 0,
   [TGSI_OPCODE_ABS] = nir_op_fabs,
   [TGSI_OPCODE_DPH] = 0,
   [TGSI_OPCODE_COS] = nir_op_fcos,
   [TGSI_OPCODE_DDX] = nir_op_fddx,
   [TGSI_OPCODE_DDY] = nir_op_fddy,
   [TGSI_OPCODE_KILL] = 0,
   [TGSI_OPCODE_PK2H] = 0, /* XXX */
   [TGSI_OPCODE_PK2US] = 0, /* XXX */
   [TGSI_OPCODE_PK4B] = 0, /* XXX */
   [TGSI_OPCODE_PK4UB] = 0, /* XXX */
   [TGSI_OPCODE_SEQ] = nir_op_seq,
   [TGSI_OPCODE_SGT] = 0,
   [TGSI_OPCODE_SIN] = nir_op_fsin,
   [TGSI_OPCODE_SLE] = 0,
   [TGSI_OPCODE_TEX] = 0,
   [TGSI_OPCODE_TXD] = 0,
   [TGSI_OPCODE_TXP] = 0,
   [TGSI_OPCODE_UP2H] = 0, /* XXX */
   [TGSI_OPCODE_UP2US] = 0, /* XXX */
   [TGSI_OPCODE_UP4B] = 0, /* XXX */
   [TGSI_OPCODE_UP4UB] = 0, /* XXX */
   [TGSI_OPCODE_ARR] = 0,

   /* No function calls, yet. */
   [TGSI_OPCODE_CAL] = 0, /* XXX */
   [TGSI_OPCODE_RET] = 0, /* XXX */

   [TGSI_OPCODE_SSG] = nir_op_fsign,
   [TGSI_OPCODE_CMP] = 0,
   [TGSI_OPCODE_SCS] = 0,
   [TGSI_OPCODE_TXB] = 0,
   [TGSI_OPCODE_DIV] = nir_op_fdiv,
   [TGSI_OPCODE_DP2] = 0,
   [TGSI_OPCODE_DP2A] = 0,
   [TGSI_OPCODE_TXL] = 0,

   [TGSI_OPCODE_BRK] = 0,
   [TGSI_OPCODE_IF] = 0,
   [TGSI_OPCODE_UIF] = 0,
   [TGSI_OPCODE_ELSE] = 0,
   [TGSI_OPCODE_ENDIF] = 0,

   [TGSI_OPCODE_DDX_FINE] = nir_op_fddx_fine,
   [TGSI_OPCODE_DDY_FINE] = nir_op_fddy_fine,

   [TGSI_OPCODE_PUSHA] = 0, /* XXX */
   [TGSI_OPCODE_POPA] = 0, /* XXX */

   [TGSI_OPCODE_CEIL] = nir_op_fceil,
   [TGSI_OPCODE_I2F] = nir_op_i2f,
   [TGSI_OPCODE_NOT] = nir_op_inot,
   [TGSI_OPCODE_TRUNC] = nir_op_ftrunc,
   [TGSI_OPCODE_SHL] = nir_op_ishl,
   [TGSI_OPCODE_AND] = nir_op_iand,
   [TGSI_OPCODE_OR] = nir_op_ior,
   [TGSI_OPCODE_MOD] = nir_op_umod,
   [TGSI_OPCODE_XOR] = nir_op_ixor,
   [TGSI_OPCODE_SAD] = 0, /* XXX */
   [TGSI_OPCODE_TXF] = 0,
   [TGSI_OPCODE_TXQ] = 0,

   [TGSI_OPCODE_CONT] = 0,

   [TGSI_OPCODE_EMIT] = 0, /* XXX */
   [TGSI_OPCODE_ENDPRIM] = 0, /* XXX */

   [TGSI_OPCODE_BGNLOOP] = 0,
   [TGSI_OPCODE_BGNSUB] = 0, /* XXX: no function calls */
   [TGSI_OPCODE_ENDLOOP] = 0,
   [TGSI_OPCODE_ENDSUB] = 0, /* XXX: no function calls */

   [TGSI_OPCODE_TXQ_LZ] = 0,
   [TGSI_OPCODE_NOP] = 0,
   [TGSI_OPCODE_FSEQ] = nir_op_feq,
   [TGSI_OPCODE_FSGE] = nir_op_fge,
   [TGSI_OPCODE_FSLT] = nir_op_flt,
   [TGSI_OPCODE_FSNE] = nir_op_fne,

   /* No control flow yet */
   [TGSI_OPCODE_CALLNZ] = 0, /* XXX */
   [TGSI_OPCODE_BREAKC] = 0, /* not emitted by glsl_to_tgsi.cpp */

   [TGSI_OPCODE_KILL_IF] = 0,

   [TGSI_OPCODE_END] = 0,

   [TGSI_OPCODE_F2I] = nir_op_f2i,
   [TGSI_OPCODE_IDIV] = nir_op_idiv,
   [TGSI_OPCODE_IMAX] = nir_op_imax,
   [TGSI_OPCODE_IMIN] = nir_op_imin,
   [TGSI_OPCODE_INEG] = nir_op_ineg,
   [TGSI_OPCODE_ISGE] = nir_op_ige,
   [TGSI_OPCODE_ISHR] = nir_op_ishr,
   [TGSI_OPCODE_ISLT] = nir_op_ilt,
   [TGSI_OPCODE_F2U] = nir_op_f2u,
   [TGSI_OPCODE_U2F] = nir_op_u2f,
   [TGSI_OPCODE_UADD] = nir_op_iadd,
   [TGSI_OPCODE_UDIV] = nir_op_udiv,
   [TGSI_OPCODE_UMAD] = 0,
   [TGSI_OPCODE_UMAX] = nir_op_umax,
   [TGSI_OPCODE_UMIN] = nir_op_umin,
   [TGSI_OPCODE_UMOD] = nir_op_umod,
   [TGSI_OPCODE_UMUL] = nir_op_imul,
   [TGSI_OPCODE_USEQ] = nir_op_ieq,
   [TGSI_OPCODE_USGE] = nir_op_uge,
   [TGSI_OPCODE_USHR] = nir_op_ushr,
   [TGSI_OPCODE_USLT] = nir_op_ult,
   [TGSI_OPCODE_USNE] = nir_op_ine,

   [TGSI_OPCODE_SWITCH] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_CASE] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_DEFAULT] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_ENDSWITCH] = 0, /* not emitted by glsl_to_tgsi.cpp */

   /* XXX: SAMPLE opcodes */

   [TGSI_OPCODE_UARL] = nir_op_imov,
   [TGSI_OPCODE_UCMP] = 0,
   [TGSI_OPCODE_IABS] = nir_op_iabs,
   [TGSI_OPCODE_ISSG] = nir_op_isign,

   /* XXX: atomics */

   [TGSI_OPCODE_TEX2] = 0,
   [TGSI_OPCODE_TXB2] = 0,
   [TGSI_OPCODE_TXL2] = 0,

   [TGSI_OPCODE_IMUL_HI] = nir_op_imul_high,
   [TGSI_OPCODE_UMUL_HI] = nir_op_umul_high,

   [TGSI_OPCODE_TG4] = 0,
   [TGSI_OPCODE_LODQ] = 0, /* XXX */

   [TGSI_OPCODE_IBFE] = nir_op_ibitfield_extract,
   [TGSI_OPCODE_UBFE] = nir_op_ubitfield_extract,
   [TGSI_OPCODE_BFI] = nir_op_bitfield_insert,
   [TGSI_OPCODE_BREV] = nir_op_bitfield_reverse,
   [TGSI_OPCODE_POPC] = nir_op_bit_count,
   [TGSI_OPCODE_LSB] = nir_op_find_lsb,
   [TGSI_OPCODE_IMSB] = nir_op_ifind_msb,
   [TGSI_OPCODE_UMSB] = nir_op_ifind_msb, /* XXX: signed vs unsigned */

   [TGSI_OPCODE_INTERP_CENTROID] = 0, /* XXX */
   [TGSI_OPCODE_INTERP_SAMPLE] = 0, /* XXX */
   [TGSI_OPCODE_INTERP_OFFSET] = 0, /* XXX */
};

static void
ttn_emit_instruction(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   unsigned i;
   unsigned tgsi_op = tgsi_inst->Instruction.Opcode;
   struct tgsi_full_dst_register *tgsi_dst = &tgsi_inst->Dst[0];

   if (tgsi_op == TGSI_OPCODE_END)
      return;

   nir_ssa_def *src[TGSI_FULL_MAX_SRC_REGISTERS];
   for (i = 0; i < TGSI_FULL_MAX_SRC_REGISTERS; i++) {
      src[i] = ttn_get_src(c, &tgsi_inst->Src[i]);
   }
   nir_alu_dest dest = ttn_get_dest(c, tgsi_dst);

   switch (tgsi_op) {
   case TGSI_OPCODE_RSQ:
      ttn_move_dest(b, dest, nir_frsq(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_SQRT:
      ttn_move_dest(b, dest, nir_fsqrt(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_RCP:
      ttn_move_dest(b, dest, nir_frcp(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_EX2:
      ttn_move_dest(b, dest, nir_fexp2(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_LG2:
      ttn_move_dest(b, dest, nir_flog2(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_POW:
      ttn_move_dest(b, dest, nir_fpow(b,
                                      ttn_channel(b, src[0], X),
                                      ttn_channel(b, src[1], X)));
      break;

   case TGSI_OPCODE_COS:
      ttn_move_dest(b, dest, nir_fcos(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_SIN:
      ttn_move_dest(b, dest, nir_fsin(b, ttn_channel(b, src[0], X)));
      break;

   case TGSI_OPCODE_ARL:
      ttn_arl(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_EXP:
      ttn_exp(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_LOG:
      ttn_log(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DST:
      ttn_dst(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_LIT:
      ttn_lit(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_CLAMP:
      ttn_clamp(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_XPD:
      ttn_xpd(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DP2:
      ttn_dp2(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DP3:
      ttn_dp3(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DP4:
      ttn_dp4(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DP2A:
      ttn_dp2a(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_DPH:
      ttn_dph(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_UMAD:
      ttn_umad(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_LRP:
      ttn_move_dest(b, dest, nir_flrp(b, src[2], src[1], src[0]));
      break;

   case TGSI_OPCODE_KILL:
      ttn_kill(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_ARR:
      ttn_arr(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_CMP:
      ttn_cmp(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_UCMP:
      ttn_ucmp(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_SCS:
      ttn_scs(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_SGT:
      ttn_sgt(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_SLE:
      ttn_sle(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_KILL_IF:
      ttn_kill_if(b, op_trans[tgsi_op], dest, src);
      break;

   case TGSI_OPCODE_TEX:
   case TGSI_OPCODE_TXP:
   case TGSI_OPCODE_TXL:
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXD:
   case TGSI_OPCODE_TXL2:
   case TGSI_OPCODE_TXB2:
   case TGSI_OPCODE_TXQ_LZ:
   case TGSI_OPCODE_TXF:
   case TGSI_OPCODE_TG4:
      ttn_tex(c, dest, src);
      break;

   case TGSI_OPCODE_TXQ:
      ttn_txq(c, dest, src);
      break;

   case TGSI_OPCODE_NOP:
      break;

   case TGSI_OPCODE_IF:
      ttn_if(c, src[0], false);
      break;

   case TGSI_OPCODE_UIF:
      ttn_if(c, src[0], true);
      break;

   case TGSI_OPCODE_ELSE:
      ttn_else(c);
      break;

   case TGSI_OPCODE_ENDIF:
      ttn_endif(c);
      break;

   case TGSI_OPCODE_BGNLOOP:
      ttn_bgnloop(c);
      break;

   case TGSI_OPCODE_BRK:
      ttn_brk(b);
      break;

   case TGSI_OPCODE_CONT:
      ttn_cont(b);
      break;

   case TGSI_OPCODE_ENDLOOP:
      ttn_endloop(c);
      break;

   default:
      if (op_trans[tgsi_op] != 0 || tgsi_op == TGSI_OPCODE_MOV) {
         ttn_alu(b, op_trans[tgsi_op], dest, src);
      } else {
         fprintf(stderr, "unknown TGSI opcode: %s\n",
                 tgsi_get_opcode_name(tgsi_op));
         abort();
      }
      break;
   }

   if (tgsi_inst->Instruction.Saturate) {
      assert(tgsi_inst->Instruction.Saturate == TGSI_SAT_ZERO_ONE);
      assert(!dest.dest.is_ssa);
      ttn_move_dest(b, dest, nir_fsat(b, ttn_src_for_dest(b, &dest)));
   }

   /* if the dst has a matching var, append store_global to move
    * output from reg to var
    */
   nir_variable *var = ttn_get_var(c, tgsi_dst);
   if (var) {
      unsigned index = tgsi_dst->Register.Index;
      unsigned offset = c->temp_regs[index].offset;
      nir_intrinsic_instr *store =
         nir_intrinsic_instr_create(b->shader, nir_intrinsic_store_var);
      struct tgsi_ind_register *indirect = tgsi_dst->Register.Indirect ?
                                           &tgsi_dst->Indirect : NULL;

      store->num_components = 4;
      store->variables[0] = ttn_array_deref(c, store, var, offset, indirect);
      store->src[0] = nir_src_for_reg(dest.dest.reg.reg);

      nir_instr_insert_after_cf_list(b->cf_node_list, &store->instr);
   }
}

/**
 * Puts a NIR intrinsic to store of each TGSI_FILE_OUTPUT value to the output
 * variables at the end of the shader.
 *
 * We don't generate these incrementally as the TGSI_FILE_OUTPUT values are
 * written, because there's no output load intrinsic, which means we couldn't
 * handle writemasks.
 */
static void
ttn_add_output_stores(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   foreach_list_typed(nir_variable, var, node, &b->shader->outputs) {
      unsigned array_len = MAX2(glsl_get_length(var->type), 1);
      unsigned i;

      for (i = 0; i < array_len; i++) {
         nir_intrinsic_instr *store =
            nir_intrinsic_instr_create(b->shader, nir_intrinsic_store_output);
         store->num_components = 4;
         store->const_index[0] = var->data.driver_location + i;
         store->const_index[1] = 1;
         store->src[0].reg.reg = c->output_regs[var->data.driver_location].reg;
         nir_instr_insert_after_cf_list(b->cf_node_list, &store->instr);
      }
   }
}

struct nir_shader *
tgsi_to_nir(const void *tgsi_tokens,
            const nir_shader_compiler_options *options)
{
   struct tgsi_parse_context parser;
   struct tgsi_shader_info scan;
   struct ttn_compile *c;
   struct nir_shader *s;
   int ret;

   c = rzalloc(NULL, struct ttn_compile);
   s = nir_shader_create(NULL, options);

   nir_function *func = nir_function_create(s, "main");
   nir_function_overload *overload = nir_function_overload_create(func);
   nir_function_impl *impl = nir_function_impl_create(overload);

   nir_builder_init(&c->build, impl);
   nir_builder_insert_after_cf_list(&c->build, &impl->body);

   tgsi_scan_shader(tgsi_tokens, &scan);
   c->scan = &scan;

   s->num_inputs = scan.file_max[TGSI_FILE_INPUT] + 1;
   s->num_uniforms = scan.const_file_max[0] + 1;
   s->num_outputs = scan.file_max[TGSI_FILE_OUTPUT] + 1;

   c->output_regs = rzalloc_array(c, struct ttn_reg_info,
                                  scan.file_max[TGSI_FILE_OUTPUT] + 1);
   c->temp_regs = rzalloc_array(c, struct ttn_reg_info,
                                scan.file_max[TGSI_FILE_TEMPORARY] + 1);
   c->imm_defs = rzalloc_array(c, nir_ssa_def *,
                               scan.file_max[TGSI_FILE_IMMEDIATE] + 1);

   c->if_stack = rzalloc_array(c, struct exec_list *,
                               (scan.opcode_count[TGSI_OPCODE_IF] +
                                scan.opcode_count[TGSI_OPCODE_UIF]) * 2);
   c->loop_stack = rzalloc_array(c, struct exec_list *,
                                 scan.opcode_count[TGSI_OPCODE_BGNLOOP]);

   ret = tgsi_parse_init(&parser, tgsi_tokens);
   assert(ret == TGSI_PARSE_OK);

   while (!tgsi_parse_end_of_tokens(&parser)) {
      tgsi_parse_token(&parser);
      c->token = &parser.FullToken;

      switch (parser.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_DECLARATION:
         ttn_emit_declaration(c);
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:
         ttn_emit_instruction(c);
         break;

      case TGSI_TOKEN_TYPE_IMMEDIATE:
         ttn_emit_immediate(c);
         break;
      }
   }

   tgsi_parse_free(&parser);

   ttn_add_output_stores(c);

   ralloc_free(c);
   return s;
}
