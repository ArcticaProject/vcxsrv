/*
 * Copyright © 2014 Intel Corporation
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
 *
 * Authors:
 *    Jason Ekstrand (jason@jlekstrand.net)
 *
 */

#include "nir.h"
#include "nir_array.h"

struct locals_to_regs_state {
   nir_shader *shader;
   nir_function_impl *impl;

   /* A hash table mapping derefs to registers */
   struct hash_table *regs_table;

   /* A growing array of derefs that we have encountered.  There is exactly
    * one element of this array per element in the hash table.  This is
    * used to make adding register initialization code deterministic.
    */
   nir_array derefs_array;
};

/* The following two functions implement a hash and equality check for
 * variable dreferences.  When the hash or equality function encounters an
 * array, it ignores the offset and whether it is direct or indirect
 * entirely.
 */
static uint32_t
hash_deref(const void *void_deref)
{
   uint32_t hash = _mesa_fnv32_1a_offset_bias;

   const nir_deref_var *deref_var = void_deref;
   hash = _mesa_fnv32_1a_accumulate(hash, deref_var->var);

   for (const nir_deref *deref = deref_var->deref.child;
        deref; deref = deref->child) {
      if (deref->deref_type == nir_deref_type_struct) {
         const nir_deref_struct *deref_struct = nir_deref_as_struct(deref);
         hash = _mesa_fnv32_1a_accumulate(hash, deref_struct->index);
      }
   }

   return hash;
}

static bool
derefs_equal(const void *void_a, const void *void_b)
{
   const nir_deref_var *a_var = void_a;
   const nir_deref_var *b_var = void_b;

   if (a_var->var != b_var->var)
      return false;

   for (const nir_deref *a = a_var->deref.child, *b = b_var->deref.child;
        a != NULL; a = a->child, b = b->child) {
      if (a->deref_type != b->deref_type)
         return false;

      if (a->deref_type == nir_deref_type_struct) {
         if (nir_deref_as_struct(a)->index != nir_deref_as_struct(b)->index)
            return false;
      }
      /* Do nothing for arrays.  They're all the same. */

      assert((a->child == NULL) == (b->child == NULL));
      if((a->child == NULL) != (b->child == NULL))
         return false;
   }

   return true;
}

static nir_register *
get_reg_for_deref(nir_deref_var *deref, struct locals_to_regs_state *state)
{
   uint32_t hash = hash_deref(deref);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(state->regs_table, hash, deref);
   if (entry)
      return entry->data;

   unsigned array_size = 1;
   nir_deref *tail = &deref->deref;
   while (tail->child) {
      if (tail->child->deref_type == nir_deref_type_array)
         array_size *= glsl_get_length(tail->type);
      tail = tail->child;
   }

   assert(glsl_type_is_vector(tail->type) || glsl_type_is_scalar(tail->type));

   nir_register *reg = nir_local_reg_create(state->impl);
   reg->num_components = glsl_get_vector_elements(tail->type);
   reg->num_array_elems = array_size > 1 ? array_size : 0;

   _mesa_hash_table_insert_pre_hashed(state->regs_table, hash, deref, reg);
   nir_array_add(&state->derefs_array, nir_deref_var *, deref);

   return reg;
}

static nir_src
get_deref_reg_src(nir_deref_var *deref, nir_instr *instr,
                  struct locals_to_regs_state *state)
{
   nir_src src;

   src.is_ssa = false;
   src.reg.reg = get_reg_for_deref(deref, state);
   src.reg.base_offset = 0;
   src.reg.indirect = NULL;

   /* It is possible for a user to create a shader that has an array with a
    * single element and then proceed to access it indirectly.  Indirectly
    * accessing a non-array register is not allowed in NIR.  In order to
    * handle this case we just convert it to a direct reference.
    */
   if (src.reg.reg->num_array_elems == 0)
      return src;

   nir_deref *tail = &deref->deref;
   while (tail->child != NULL) {
      const struct glsl_type *parent_type = tail->type;
      tail = tail->child;

      if (tail->deref_type != nir_deref_type_array)
         continue;

      nir_deref_array *deref_array = nir_deref_as_array(tail);

      src.reg.base_offset *= glsl_get_length(parent_type);
      src.reg.base_offset += deref_array->base_offset;

      if (src.reg.indirect) {
         nir_load_const_instr *load_const =
            nir_load_const_instr_create(state->shader, 1);
         load_const->value.u[0] = glsl_get_length(parent_type);
         nir_instr_insert_before(instr, &load_const->instr);

         nir_alu_instr *mul = nir_alu_instr_create(state->shader, nir_op_imul);
         mul->src[0].src = *src.reg.indirect;
         mul->src[1].src.is_ssa = true;
         mul->src[1].src.ssa = &load_const->def;
         mul->dest.write_mask = 1;
         nir_ssa_dest_init(&mul->instr, &mul->dest.dest, 1, NULL);
         nir_instr_insert_before(instr, &mul->instr);

         src.reg.indirect->is_ssa = true;
         src.reg.indirect->ssa = &mul->dest.dest.ssa;
      }

      if (deref_array->deref_array_type == nir_deref_array_type_indirect) {
         if (src.reg.indirect == NULL) {
            src.reg.indirect = ralloc(state->shader, nir_src);
            nir_src_copy(src.reg.indirect, &deref_array->indirect,
                         state->shader);
         } else {
            nir_alu_instr *add = nir_alu_instr_create(state->shader,
                                                      nir_op_iadd);
            add->src[0].src = *src.reg.indirect;
            nir_src_copy(&add->src[1].src, &deref_array->indirect,
                         state->shader);
            add->dest.write_mask = 1;
            nir_ssa_dest_init(&add->instr, &add->dest.dest, 1, NULL);
            nir_instr_insert_before(instr, &add->instr);

            src.reg.indirect->is_ssa = true;
            src.reg.indirect->ssa = &add->dest.dest.ssa;
         }
      }
   }

   return src;
}

static bool
lower_locals_to_regs_block(nir_block *block, void *void_state)
{
   struct locals_to_regs_state *state = void_state;

   nir_foreach_instr_safe(block, instr) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      switch (intrin->intrinsic) {
      case nir_intrinsic_load_var: {
         if (intrin->variables[0]->var->data.mode != nir_var_local)
            continue;

         nir_alu_instr *mov = nir_alu_instr_create(state->shader, nir_op_imov);
         mov->src[0].src = get_deref_reg_src(intrin->variables[0],
                                             &intrin->instr, state);
         mov->dest.write_mask = (1 << intrin->num_components) - 1;
         if (intrin->dest.is_ssa) {
            nir_ssa_dest_init(&mov->instr, &mov->dest.dest,
                              intrin->num_components, NULL);
            nir_ssa_def_rewrite_uses(&intrin->dest.ssa,
                                     nir_src_for_ssa(&mov->dest.dest.ssa),
                                     state->shader);
         } else {
            nir_dest_copy(&mov->dest.dest, &intrin->dest, state->shader);
         }
         nir_instr_insert_before(&intrin->instr, &mov->instr);

         nir_instr_remove(&intrin->instr);
         break;
      }

      case nir_intrinsic_store_var: {
         if (intrin->variables[0]->var->data.mode != nir_var_local)
            continue;

         nir_src reg_src = get_deref_reg_src(intrin->variables[0],
                                             &intrin->instr, state);

         nir_alu_instr *mov = nir_alu_instr_create(state->shader, nir_op_imov);
         nir_src_copy(&mov->src[0].src, &intrin->src[0], state->shader);
         mov->dest.write_mask = (1 << intrin->num_components) - 1;
         mov->dest.dest.is_ssa = false;
         mov->dest.dest.reg.reg = reg_src.reg.reg;
         mov->dest.dest.reg.base_offset = reg_src.reg.base_offset;
         mov->dest.dest.reg.indirect = reg_src.reg.indirect;

         nir_instr_insert_before(&intrin->instr, &mov->instr);

         nir_instr_remove(&intrin->instr);
         break;
      }

      case nir_intrinsic_copy_var:
         unreachable("There should be no copies whatsoever at this point");
         break;

      default:
         continue;
      }
   }

   return true;
}

static nir_block *
compute_reg_usedef_lca(nir_register *reg)
{
   nir_block *lca = NULL;

   list_for_each_entry(nir_dest, def_dest, &reg->defs, reg.def_link)
      lca = nir_dominance_lca(lca, def_dest->reg.parent_instr->block);

   list_for_each_entry(nir_src, use_src, &reg->uses, use_link)
      lca = nir_dominance_lca(lca, use_src->parent_instr->block);

   list_for_each_entry(nir_src, use_src, &reg->if_uses, use_link) {
      nir_cf_node *prev_node = nir_cf_node_prev(&use_src->parent_if->cf_node);
      assert(prev_node->type == nir_cf_node_block);
      lca = nir_dominance_lca(lca, nir_cf_node_as_block(prev_node));
   }

   return lca;
}

static void
insert_constant_initializer(nir_deref_var *deref_head, nir_deref *deref_tail,
                            nir_block *block,
                            struct locals_to_regs_state *state)
{
   if (deref_tail->child) {
      switch (deref_tail->child->deref_type) {
      case nir_deref_type_array: {
         unsigned array_elems = glsl_get_length(deref_tail->type);

         nir_deref_array arr_deref;
         arr_deref.deref = *deref_tail->child;
         arr_deref.deref_array_type = nir_deref_array_type_direct;

         nir_deref *old_child = deref_tail->child;
         deref_tail->child = &arr_deref.deref;
         for (unsigned i = 0; i < array_elems; i++) {
            arr_deref.base_offset = i;
            insert_constant_initializer(deref_head, &arr_deref.deref,
                                        block, state);
         }
         deref_tail->child = old_child;
         return;
      }

      case nir_deref_type_struct:
         insert_constant_initializer(deref_head, deref_tail->child,
                                     block, state);
         return;

      default:
         unreachable("Invalid deref child type");
      }
   }

   assert(deref_tail->child == NULL);

   nir_load_const_instr *load =
      nir_deref_get_const_initializer_load(state->shader, deref_head);
   nir_instr_insert_before_block(block, &load->instr);

   nir_src reg_src = get_deref_reg_src(deref_head, &load->instr, state);

   nir_alu_instr *mov = nir_alu_instr_create(state->shader, nir_op_imov);
   mov->src[0].src = nir_src_for_ssa(&load->def);
   mov->dest.write_mask = (1 << load->def.num_components) - 1;
   mov->dest.dest.is_ssa = false;
   mov->dest.dest.reg.reg = reg_src.reg.reg;
   mov->dest.dest.reg.base_offset = reg_src.reg.base_offset;
   mov->dest.dest.reg.indirect = reg_src.reg.indirect;

   nir_instr_insert_after(&load->instr, &mov->instr);
}

static void
nir_lower_locals_to_regs_impl(nir_function_impl *impl)
{
   struct locals_to_regs_state state;

   state.shader = impl->overload->function->shader;
   state.impl = impl;
   state.regs_table = _mesa_hash_table_create(NULL, hash_deref, derefs_equal);
   nir_array_init(&state.derefs_array, NULL);

   nir_metadata_require(impl, nir_metadata_dominance);

   nir_foreach_block(impl, lower_locals_to_regs_block, &state);

   nir_array_foreach(&state.derefs_array, nir_deref_var *, deref_ptr) {
      nir_deref_var *deref = *deref_ptr;
      struct hash_entry *deref_entry =
         _mesa_hash_table_search(state.regs_table, deref);
      assert(deref_entry && deref_entry->key == deref);
      nir_register *reg = (nir_register *)deref_entry->data;

      if (deref->var->constant_initializer == NULL)
         continue;

      nir_block *usedef_lca = compute_reg_usedef_lca(reg);

      insert_constant_initializer(deref, &deref->deref, usedef_lca, &state);
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                               nir_metadata_dominance);

   nir_array_fini(&state.derefs_array);
   _mesa_hash_table_destroy(state.regs_table, NULL);
}

void
nir_lower_locals_to_regs(nir_shader *shader)
{
   nir_foreach_overload(shader, overload) {
      if (overload->impl)
         nir_lower_locals_to_regs_impl(overload->impl);
   }
}
