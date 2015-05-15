/*
 * Copyright © 2014 Connor Abbott
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
 *    Connor Abbott (cwabbott0@gmail.com)
 *
 */

#pragma once

#include "util/hash_table.h"
#include "../list.h"
#include "GL/gl.h" /* GLenum */
#include "util/list.h"
#include "util/ralloc.h"
#include "util/set.h"
#include "util/bitset.h"
#include "nir_types.h"
#include "glsl/shader_enums.h"
#include <stdio.h>

#include "nir_opcodes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gl_program;
struct gl_shader_program;

#define NIR_FALSE 0u
#define NIR_TRUE (~0u)

/** Defines a cast function
 *
 * This macro defines a cast function from in_type to out_type where
 * out_type is some structure type that contains a field of type out_type.
 *
 * Note that you have to be a bit careful as the generated cast function
 * destroys constness.
 */
#define NIR_DEFINE_CAST(name, in_type, out_type, field)  \
static inline out_type *                                 \
name(const in_type *parent)                              \
{                                                        \
   return exec_node_data(out_type, parent, field);       \
}

struct nir_function_overload;
struct nir_function;
struct nir_shader;
struct nir_instr;


/**
 * Description of built-in state associated with a uniform
 *
 * \sa nir_variable::state_slots
 */
typedef struct {
   int tokens[5];
   int swizzle;
} nir_state_slot;

typedef enum {
   nir_var_shader_in,
   nir_var_shader_out,
   nir_var_global,
   nir_var_local,
   nir_var_uniform,
   nir_var_system_value
} nir_variable_mode;

/**
 * Data stored in an nir_constant
 */
union nir_constant_data {
   unsigned u[16];
   int i[16];
   float f[16];
   bool b[16];
};

typedef struct nir_constant {
   /**
    * Value of the constant.
    *
    * The field used to back the values supplied by the constant is determined
    * by the type associated with the \c nir_variable.  Constants may be
    * scalars, vectors, or matrices.
    */
   union nir_constant_data value;

   /* Array elements / Structure Fields */
   struct nir_constant **elements;
} nir_constant;

/**
 * \brief Layout qualifiers for gl_FragDepth.
 *
 * The AMD/ARB_conservative_depth extensions allow gl_FragDepth to be redeclared
 * with a layout qualifier.
 */
typedef enum {
    nir_depth_layout_none, /**< No depth layout is specified. */
    nir_depth_layout_any,
    nir_depth_layout_greater,
    nir_depth_layout_less,
    nir_depth_layout_unchanged
} nir_depth_layout;

/**
 * Either a uniform, global variable, shader input, or shader output. Based on
 * ir_variable - it should be easy to translate between the two.
 */

typedef struct {
   struct exec_node node;

   /**
    * Declared type of the variable
    */
   const struct glsl_type *type;

   /**
    * Declared name of the variable
    */
   char *name;

   /**
    * For variables which satisfy the is_interface_instance() predicate, this
    * points to an array of integers such that if the ith member of the
    * interface block is an array, max_ifc_array_access[i] is the maximum
    * array element of that member that has been accessed.  If the ith member
    * of the interface block is not an array, max_ifc_array_access[i] is
    * unused.
    *
    * For variables whose type is not an interface block, this pointer is
    * NULL.
    */
   unsigned *max_ifc_array_access;

   struct nir_variable_data {

      /**
       * Is the variable read-only?
       *
       * This is set for variables declared as \c const, shader inputs,
       * and uniforms.
       */
      unsigned read_only:1;
      unsigned centroid:1;
      unsigned sample:1;
      unsigned invariant:1;

      /**
       * Storage class of the variable.
       *
       * \sa nir_variable_mode
       */
      nir_variable_mode mode:4;

      /**
       * Interpolation mode for shader inputs / outputs
       *
       * \sa glsl_interp_qualifier
       */
      unsigned interpolation:2;

      /**
       * \name ARB_fragment_coord_conventions
       * @{
       */
      unsigned origin_upper_left:1;
      unsigned pixel_center_integer:1;
      /*@}*/

      /**
       * Was the location explicitly set in the shader?
       *
       * If the location is explicitly set in the shader, it \b cannot be changed
       * by the linker or by the API (e.g., calls to \c glBindAttribLocation have
       * no effect).
       */
      unsigned explicit_location:1;
      unsigned explicit_index:1;

      /**
       * Was an initial binding explicitly set in the shader?
       *
       * If so, constant_initializer contains an integer nir_constant
       * representing the initial binding point.
       */
      unsigned explicit_binding:1;

      /**
       * Does this variable have an initializer?
       *
       * This is used by the linker to cross-validiate initializers of global
       * variables.
       */
      unsigned has_initializer:1;

      /**
       * Is this variable a generic output or input that has not yet been matched
       * up to a variable in another stage of the pipeline?
       *
       * This is used by the linker as scratch storage while assigning locations
       * to generic inputs and outputs.
       */
      unsigned is_unmatched_generic_inout:1;

      /**
       * If non-zero, then this variable may be packed along with other variables
       * into a single varying slot, so this offset should be applied when
       * accessing components.  For example, an offset of 1 means that the x
       * component of this variable is actually stored in component y of the
       * location specified by \c location.
       */
      unsigned location_frac:2;

      /**
       * Non-zero if this variable was created by lowering a named interface
       * block which was not an array.
       *
       * Note that this variable and \c from_named_ifc_block_array will never
       * both be non-zero.
       */
      unsigned from_named_ifc_block_nonarray:1;

      /**
       * Non-zero if this variable was created by lowering a named interface
       * block which was an array.
       *
       * Note that this variable and \c from_named_ifc_block_nonarray will never
       * both be non-zero.
       */
      unsigned from_named_ifc_block_array:1;

      /**
       * \brief Layout qualifier for gl_FragDepth.
       *
       * This is not equal to \c ir_depth_layout_none if and only if this
       * variable is \c gl_FragDepth and a layout qualifier is specified.
       */
      nir_depth_layout depth_layout;

      /**
       * Storage location of the base of this variable
       *
       * The precise meaning of this field depends on the nature of the variable.
       *
       *   - Vertex shader input: one of the values from \c gl_vert_attrib.
       *   - Vertex shader output: one of the values from \c gl_varying_slot.
       *   - Geometry shader input: one of the values from \c gl_varying_slot.
       *   - Geometry shader output: one of the values from \c gl_varying_slot.
       *   - Fragment shader input: one of the values from \c gl_varying_slot.
       *   - Fragment shader output: one of the values from \c gl_frag_result.
       *   - Uniforms: Per-stage uniform slot number for default uniform block.
       *   - Uniforms: Index within the uniform block definition for UBO members.
       *   - Other: This field is not currently used.
       *
       * If the variable is a uniform, shader input, or shader output, and the
       * slot has not been assigned, the value will be -1.
       */
      int location;

      /**
       * The actual location of the variable in the IR. Only valid for inputs
       * and outputs.
       */
      unsigned int driver_location;

      /**
       * output index for dual source blending.
       */
      int index;

      /**
       * Initial binding point for a sampler or UBO.
       *
       * For array types, this represents the binding point for the first element.
       */
      int binding;

      /**
       * Location an atomic counter is stored at.
       */
      struct {
         unsigned buffer_index;
         unsigned offset;
      } atomic;

      /**
       * ARB_shader_image_load_store qualifiers.
       */
      struct {
         bool read_only; /**< "readonly" qualifier. */
         bool write_only; /**< "writeonly" qualifier. */
         bool coherent;
         bool _volatile;
         bool restrict_flag;

         /** Image internal format if specified explicitly, otherwise GL_NONE. */
         GLenum format;
      } image;

      /**
       * Highest element accessed with a constant expression array index
       *
       * Not used for non-array variables.
       */
      unsigned max_array_access;

   } data;

   /**
    * Built-in state that backs this uniform
    *
    * Once set at variable creation, \c state_slots must remain invariant.
    * This is because, ideally, this array would be shared by all clones of
    * this variable in the IR tree.  In other words, we'd really like for it
    * to be a fly-weight.
    *
    * If the variable is not a uniform, \c num_state_slots will be zero and
    * \c state_slots will be \c NULL.
    */
   /*@{*/
   unsigned num_state_slots;    /**< Number of state slots used */
   nir_state_slot *state_slots;  /**< State descriptors. */
   /*@}*/

   /**
    * Constant expression assigned in the initializer of the variable
    */
   nir_constant *constant_initializer;

   /**
    * For variables that are in an interface block or are an instance of an
    * interface block, this is the \c GLSL_TYPE_INTERFACE type for that block.
    *
    * \sa ir_variable::location
    */
   const struct glsl_type *interface_type;
} nir_variable;

typedef struct {
   struct exec_node node;

   unsigned num_components; /** < number of vector components */
   unsigned num_array_elems; /** < size of array (0 for no array) */

   /** generic register index. */
   unsigned index;

   /** only for debug purposes, can be NULL */
   const char *name;

   /** whether this register is local (per-function) or global (per-shader) */
   bool is_global;

   /**
    * If this flag is set to true, then accessing channels >= num_components
    * is well-defined, and simply spills over to the next array element. This
    * is useful for backends that can do per-component accessing, in
    * particular scalar backends. By setting this flag and making
    * num_components equal to 1, structures can be packed tightly into
    * registers and then registers can be accessed per-component to get to
    * each structure member, even if it crosses vec4 boundaries.
    */
   bool is_packed;

   /**
    * If this pointer is non-NULL then this register has exactly one
    * definition and that definition dominates all of its uses.  This is
    * set by the out-of-SSA pass so that backends can get SSA-like
    * information even once they have gone out of SSA.
    */
   struct nir_instr *parent_instr;

   /** set of nir_instr's where this register is used (read from) */
   struct list_head uses;

   /** set of nir_instr's where this register is defined (written to) */
   struct list_head defs;

   /** set of nir_if's where this register is used as a condition */
   struct list_head if_uses;
} nir_register;

typedef enum {
   nir_instr_type_alu,
   nir_instr_type_call,
   nir_instr_type_tex,
   nir_instr_type_intrinsic,
   nir_instr_type_load_const,
   nir_instr_type_jump,
   nir_instr_type_ssa_undef,
   nir_instr_type_phi,
   nir_instr_type_parallel_copy,
} nir_instr_type;

typedef struct nir_instr {
   struct exec_node node;
   nir_instr_type type;
   struct nir_block *block;

   /* A temporary for optimization and analysis passes to use for storing
    * flags.  For instance, DCE uses this to store the "dead/live" info.
    */
   uint8_t pass_flags;
} nir_instr;

static inline nir_instr *
nir_instr_next(nir_instr *instr)
{
   struct exec_node *next = exec_node_get_next(&instr->node);
   if (exec_node_is_tail_sentinel(next))
      return NULL;
   else
      return exec_node_data(nir_instr, next, node);
}

static inline nir_instr *
nir_instr_prev(nir_instr *instr)
{
   struct exec_node *prev = exec_node_get_prev(&instr->node);
   if (exec_node_is_head_sentinel(prev))
      return NULL;
   else
      return exec_node_data(nir_instr, prev, node);
}

typedef struct {
   /** for debugging only, can be NULL */
   const char* name;

   /** generic SSA definition index. */
   unsigned index;

   /** Index into the live_in and live_out bitfields */
   unsigned live_index;

   nir_instr *parent_instr;

   /** set of nir_instr's where this register is used (read from) */
   struct list_head uses;

   /** set of nir_if's where this register is used as a condition */
   struct list_head if_uses;

   uint8_t num_components;
} nir_ssa_def;

struct nir_src;

typedef struct {
   nir_register *reg;
   struct nir_src *indirect; /** < NULL for no indirect offset */
   unsigned base_offset;

   /* TODO use-def chain goes here */
} nir_reg_src;

typedef struct {
   nir_instr *parent_instr;
   struct list_head def_link;

   nir_register *reg;
   struct nir_src *indirect; /** < NULL for no indirect offset */
   unsigned base_offset;

   /* TODO def-use chain goes here */
} nir_reg_dest;

struct nir_if;

typedef struct nir_src {
   union {
      nir_instr *parent_instr;
      struct nir_if *parent_if;
   };

   struct list_head use_link;

   union {
      nir_reg_src reg;
      nir_ssa_def *ssa;
   };

   bool is_ssa;
} nir_src;

#define NIR_SRC_INIT (nir_src) { { NULL } }

#define nir_foreach_use(reg_or_ssa_def, src) \
   list_for_each_entry(nir_src, src, &(reg_or_ssa_def)->uses, use_link)

#define nir_foreach_use_safe(reg_or_ssa_def, src) \
   list_for_each_entry_safe(nir_src, src, &(reg_or_ssa_def)->uses, use_link)

#define nir_foreach_if_use(reg_or_ssa_def, src) \
   list_for_each_entry(nir_src, src, &(reg_or_ssa_def)->if_uses, use_link)

#define nir_foreach_if_use_safe(reg_or_ssa_def, src) \
   list_for_each_entry_safe(nir_src, src, &(reg_or_ssa_def)->if_uses, use_link)

typedef struct {
   union {
      nir_reg_dest reg;
      nir_ssa_def ssa;
   };

   bool is_ssa;
} nir_dest;

#define NIR_DEST_INIT (nir_dest) { { { NULL } } }

#define nir_foreach_def(reg, dest) \
   list_for_each_entry(nir_dest, dest, &(reg)->defs, reg.def_link)

#define nir_foreach_def_safe(reg, dest) \
   list_for_each_entry_safe(nir_dest, dest, &(reg)->defs, reg.def_link)

static inline nir_src
nir_src_for_ssa(nir_ssa_def *def)
{
   nir_src src = NIR_SRC_INIT;

   src.is_ssa = true;
   src.ssa = def;

   return src;
}

static inline nir_src
nir_src_for_reg(nir_register *reg)
{
   nir_src src = NIR_SRC_INIT;

   src.is_ssa = false;
   src.reg.reg = reg;
   src.reg.indirect = NULL;
   src.reg.base_offset = 0;

   return src;
}

static inline nir_instr *
nir_src_get_parent_instr(const nir_src *src)
{
   if (src->is_ssa) {
      return src->ssa->parent_instr;
   } else {
      return src->reg.reg->parent_instr;
   }
}

static inline nir_dest
nir_dest_for_reg(nir_register *reg)
{
   nir_dest dest = NIR_DEST_INIT;

   dest.reg.reg = reg;

   return dest;
}

void nir_src_copy(nir_src *dest, const nir_src *src, void *mem_ctx);
void nir_dest_copy(nir_dest *dest, const nir_dest *src, void *mem_ctx);

typedef struct {
   nir_src src;

   /**
    * \name input modifiers
    */
   /*@{*/
   /**
    * For inputs interpreted as floating point, flips the sign bit. For
    * inputs interpreted as integers, performs the two's complement negation.
    */
   bool negate;

   /**
    * Clears the sign bit for floating point values, and computes the integer
    * absolute value for integers. Note that the negate modifier acts after
    * the absolute value modifier, therefore if both are set then all inputs
    * will become negative.
    */
   bool abs;
   /*@}*/

   /**
    * For each input component, says which component of the register it is
    * chosen from. Note that which elements of the swizzle are used and which
    * are ignored are based on the write mask for most opcodes - for example,
    * a statement like "foo.xzw = bar.zyx" would have a writemask of 1101b and
    * a swizzle of {2, x, 1, 0} where x means "don't care."
    */
   uint8_t swizzle[4];
} nir_alu_src;

typedef struct {
   nir_dest dest;

   /**
    * \name saturate output modifier
    *
    * Only valid for opcodes that output floating-point numbers. Clamps the
    * output to between 0.0 and 1.0 inclusive.
    */

   bool saturate;

   unsigned write_mask : 4; /* ignored if dest.is_ssa is true */
} nir_alu_dest;

void nir_alu_src_copy(nir_alu_src *dest, const nir_alu_src *src, void *mem_ctx);
void nir_alu_dest_copy(nir_alu_dest *dest, const nir_alu_dest *src,
                       void *mem_ctx);

typedef enum {
   nir_type_invalid = 0, /* Not a valid type */
   nir_type_float,
   nir_type_int,
   nir_type_unsigned,
   nir_type_bool
} nir_alu_type;

typedef enum {
   NIR_OP_IS_COMMUTATIVE = (1 << 0),
   NIR_OP_IS_ASSOCIATIVE = (1 << 1),
} nir_op_algebraic_property;

typedef struct {
   const char *name;

   unsigned num_inputs;

   /**
    * The number of components in the output
    *
    * If non-zero, this is the size of the output and input sizes are
    * explicitly given; swizzle and writemask are still in effect, but if
    * the output component is masked out, then the input component may
    * still be in use.
    *
    * If zero, the opcode acts in the standard, per-component manner; the
    * operation is performed on each component (except the ones that are
    * masked out) with the input being taken from the input swizzle for
    * that component.
    *
    * The size of some of the inputs may be given (i.e. non-zero) even
    * though output_size is zero; in that case, the inputs with a zero
    * size act per-component, while the inputs with non-zero size don't.
    */
   unsigned output_size;

   /**
    * The type of vector that the instruction outputs. Note that the
    * staurate modifier is only allowed on outputs with the float type.
    */

   nir_alu_type output_type;

   /**
    * The number of components in each input
    */
   unsigned input_sizes[4];

   /**
    * The type of vector that each input takes. Note that negate and
    * absolute value are only allowed on inputs with int or float type and
    * behave differently on the two.
    */
   nir_alu_type input_types[4];

   nir_op_algebraic_property algebraic_properties;
} nir_op_info;

extern const nir_op_info nir_op_infos[nir_num_opcodes];

typedef struct nir_alu_instr {
   nir_instr instr;
   nir_op op;
   nir_alu_dest dest;
   nir_alu_src src[];
} nir_alu_instr;

/* is this source channel used? */
static inline bool
nir_alu_instr_channel_used(nir_alu_instr *instr, unsigned src, unsigned channel)
{
   if (nir_op_infos[instr->op].input_sizes[src] > 0)
      return channel < nir_op_infos[instr->op].input_sizes[src];

   return (instr->dest.write_mask >> channel) & 1;
}

/*
 * For instructions whose destinations are SSA, get the number of channels
 * used for a source
 */
static inline unsigned
nir_ssa_alu_instr_src_components(nir_alu_instr *instr, unsigned src)
{
   assert(instr->dest.dest.is_ssa);

   if (nir_op_infos[instr->op].input_sizes[src] > 0)
      return nir_op_infos[instr->op].input_sizes[src];

   return instr->dest.dest.ssa.num_components;
}

typedef enum {
   nir_deref_type_var,
   nir_deref_type_array,
   nir_deref_type_struct
} nir_deref_type;

typedef struct nir_deref {
   nir_deref_type deref_type;
   struct nir_deref *child;
   const struct glsl_type *type;
} nir_deref;

typedef struct {
   nir_deref deref;

   nir_variable *var;
} nir_deref_var;

/* This enum describes how the array is referenced.  If the deref is
 * direct then the base_offset is used.  If the deref is indirect then then
 * offset is given by base_offset + indirect.  If the deref is a wildcard
 * then the deref refers to all of the elements of the array at the same
 * time.  Wildcard dereferences are only ever allowed in copy_var
 * intrinsics and the source and destination derefs must have matching
 * wildcards.
 */
typedef enum {
   nir_deref_array_type_direct,
   nir_deref_array_type_indirect,
   nir_deref_array_type_wildcard,
} nir_deref_array_type;

typedef struct {
   nir_deref deref;

   nir_deref_array_type deref_array_type;
   unsigned base_offset;
   nir_src indirect;
} nir_deref_array;

typedef struct {
   nir_deref deref;

   unsigned index;
} nir_deref_struct;

NIR_DEFINE_CAST(nir_deref_as_var, nir_deref, nir_deref_var, deref)
NIR_DEFINE_CAST(nir_deref_as_array, nir_deref, nir_deref_array, deref)
NIR_DEFINE_CAST(nir_deref_as_struct, nir_deref, nir_deref_struct, deref)

typedef struct {
   nir_instr instr;

   unsigned num_params;
   nir_deref_var **params;
   nir_deref_var *return_deref;

   struct nir_function_overload *callee;
} nir_call_instr;

#define INTRINSIC(name, num_srcs, src_components, has_dest, dest_components, \
                  num_variables, num_indices, flags) \
   nir_intrinsic_##name,

#define LAST_INTRINSIC(name) nir_last_intrinsic = nir_intrinsic_##name,

typedef enum {
#include "nir_intrinsics.h"
   nir_num_intrinsics = nir_last_intrinsic + 1
} nir_intrinsic_op;

#undef INTRINSIC
#undef LAST_INTRINSIC

/** Represents an intrinsic
 *
 * An intrinsic is an instruction type for handling things that are
 * more-or-less regular operations but don't just consume and produce SSA
 * values like ALU operations do.  Intrinsics are not for things that have
 * special semantic meaning such as phi nodes and parallel copies.
 * Examples of intrinsics include variable load/store operations, system
 * value loads, and the like.  Even though texturing more-or-less falls
 * under this category, texturing is its own instruction type because
 * trying to represent texturing with intrinsics would lead to a
 * combinatorial explosion of intrinsic opcodes.
 *
 * By having a single instruction type for handling a lot of different
 * cases, optimization passes can look for intrinsics and, for the most
 * part, completely ignore them.  Each intrinsic type also has a few
 * possible flags that govern whether or not they can be reordered or
 * eliminated.  That way passes like dead code elimination can still work
 * on intrisics without understanding the meaning of each.
 *
 * Each intrinsic has some number of constant indices, some number of
 * variables, and some number of sources.  What these sources, variables,
 * and indices mean depends on the intrinsic and is documented with the
 * intrinsic declaration in nir_intrinsics.h.  Intrinsics and texture
 * instructions are the only types of instruction that can operate on
 * variables.
 */
typedef struct {
   nir_instr instr;

   nir_intrinsic_op intrinsic;

   nir_dest dest;

   /** number of components if this is a vectorized intrinsic
    *
    * Similarly to ALU operations, some intrinsics are vectorized.
    * An intrinsic is vectorized if nir_intrinsic_infos.dest_components == 0.
    * For vectorized intrinsics, the num_components field specifies the
    * number of destination components and the number of source components
    * for all sources with nir_intrinsic_infos.src_components[i] == 0.
    */
   uint8_t num_components;

   int const_index[3];

   nir_deref_var *variables[2];

   nir_src src[];
} nir_intrinsic_instr;

/**
 * \name NIR intrinsics semantic flags
 *
 * information about what the compiler can do with the intrinsics.
 *
 * \sa nir_intrinsic_info::flags
 */
typedef enum {
   /**
    * whether the intrinsic can be safely eliminated if none of its output
    * value is not being used.
    */
   NIR_INTRINSIC_CAN_ELIMINATE = (1 << 0),

   /**
    * Whether the intrinsic can be reordered with respect to any other
    * intrinsic, i.e. whether the only reordering dependencies of the
    * intrinsic are due to the register reads/writes.
    */
   NIR_INTRINSIC_CAN_REORDER = (1 << 1),
} nir_intrinsic_semantic_flag;

#define NIR_INTRINSIC_MAX_INPUTS 4

typedef struct {
   const char *name;

   unsigned num_srcs; /** < number of register/SSA inputs */

   /** number of components of each input register
    *
    * If this value is 0, the number of components is given by the
    * num_components field of nir_intrinsic_instr.
    */
   unsigned src_components[NIR_INTRINSIC_MAX_INPUTS];

   bool has_dest;

   /** number of components of the output register
    *
    * If this value is 0, the number of components is given by the
    * num_components field of nir_intrinsic_instr.
    */
   unsigned dest_components;

   /** the number of inputs/outputs that are variables */
   unsigned num_variables;

   /** the number of constant indices used by the intrinsic */
   unsigned num_indices;

   /** semantic flags for calls to this intrinsic */
   nir_intrinsic_semantic_flag flags;
} nir_intrinsic_info;

extern const nir_intrinsic_info nir_intrinsic_infos[nir_num_intrinsics];

/**
 * \group texture information
 *
 * This gives semantic information about textures which is useful to the
 * frontend, the backend, and lowering passes, but not the optimizer.
 */

typedef enum {
   nir_tex_src_coord,
   nir_tex_src_projector,
   nir_tex_src_comparitor, /* shadow comparitor */
   nir_tex_src_offset,
   nir_tex_src_bias,
   nir_tex_src_lod,
   nir_tex_src_ms_index, /* MSAA sample index */
   nir_tex_src_ddx,
   nir_tex_src_ddy,
   nir_tex_src_sampler_offset, /* < dynamically uniform indirect offset */
   nir_num_tex_src_types
} nir_tex_src_type;

typedef struct {
   nir_src src;
   nir_tex_src_type src_type;
} nir_tex_src;

typedef enum {
   nir_texop_tex,                /**< Regular texture look-up */
   nir_texop_txb,                /**< Texture look-up with LOD bias */
   nir_texop_txl,                /**< Texture look-up with explicit LOD */
   nir_texop_txd,                /**< Texture look-up with partial derivatvies */
   nir_texop_txf,                /**< Texel fetch with explicit LOD */
   nir_texop_txf_ms,                /**< Multisample texture fetch */
   nir_texop_txs,                /**< Texture size */
   nir_texop_lod,                /**< Texture lod query */
   nir_texop_tg4,                /**< Texture gather */
   nir_texop_query_levels       /**< Texture levels query */
} nir_texop;

typedef struct {
   nir_instr instr;

   enum glsl_sampler_dim sampler_dim;
   nir_alu_type dest_type;

   nir_texop op;
   nir_dest dest;
   nir_tex_src *src;
   unsigned num_srcs, coord_components;
   bool is_array, is_shadow;

   /**
    * If is_shadow is true, whether this is the old-style shadow that outputs 4
    * components or the new-style shadow that outputs 1 component.
    */
   bool is_new_style_shadow;

   /* constant offset - must be 0 if the offset source is used */
   int const_offset[4];

   /* gather component selector */
   unsigned component : 2;

   /** The sampler index
    *
    * If this texture instruction has a nir_tex_src_sampler_offset source,
    * then the sampler index is given by sampler_index + sampler_offset.
    */
   unsigned sampler_index;

   /** The size of the sampler array or 0 if it's not an array */
   unsigned sampler_array_size;

   nir_deref_var *sampler; /* if this is NULL, use sampler_index instead */
} nir_tex_instr;

static inline unsigned
nir_tex_instr_dest_size(nir_tex_instr *instr)
{
   switch (instr->op) {
   case nir_texop_txs: {
      unsigned ret;
      switch (instr->sampler_dim) {
         case GLSL_SAMPLER_DIM_1D:
         case GLSL_SAMPLER_DIM_BUF:
            ret = 1;
            break;
         case GLSL_SAMPLER_DIM_2D:
         case GLSL_SAMPLER_DIM_CUBE:
         case GLSL_SAMPLER_DIM_MS:
         case GLSL_SAMPLER_DIM_RECT:
         case GLSL_SAMPLER_DIM_EXTERNAL:
            ret = 2;
            break;
         case GLSL_SAMPLER_DIM_3D:
            ret = 3;
            break;
         default:
            unreachable("not reached");
      }
      if (instr->is_array)
         ret++;
      return ret;
   }

   case nir_texop_lod:
      return 2;

   case nir_texop_query_levels:
      return 1;

   default:
      if (instr->is_shadow && instr->is_new_style_shadow)
         return 1;

      return 4;
   }
}

static inline unsigned
nir_tex_instr_src_size(nir_tex_instr *instr, unsigned src)
{
   if (instr->src[src].src_type == nir_tex_src_coord)
      return instr->coord_components;


   if (instr->src[src].src_type == nir_tex_src_offset ||
       instr->src[src].src_type == nir_tex_src_ddx ||
       instr->src[src].src_type == nir_tex_src_ddy) {
      if (instr->is_array)
         return instr->coord_components - 1;
      else
         return instr->coord_components;
   }

   return 1;
}

static inline int
nir_tex_instr_src_index(nir_tex_instr *instr, nir_tex_src_type type)
{
   for (unsigned i = 0; i < instr->num_srcs; i++)
      if (instr->src[i].src_type == type)
         return (int) i;

   return -1;
}

typedef struct {
   union {
      float f[4];
      int32_t i[4];
      uint32_t u[4];
   };
} nir_const_value;

typedef struct {
   nir_instr instr;

   nir_const_value value;

   nir_ssa_def def;
} nir_load_const_instr;

typedef enum {
   nir_jump_return,
   nir_jump_break,
   nir_jump_continue,
} nir_jump_type;

typedef struct {
   nir_instr instr;
   nir_jump_type type;
} nir_jump_instr;

/* creates a new SSA variable in an undefined state */

typedef struct {
   nir_instr instr;
   nir_ssa_def def;
} nir_ssa_undef_instr;

typedef struct {
   struct exec_node node;

   /* The predecessor block corresponding to this source */
   struct nir_block *pred;

   nir_src src;
} nir_phi_src;

#define nir_foreach_phi_src(phi, entry) \
   foreach_list_typed(nir_phi_src, entry, node, &(phi)->srcs)

typedef struct {
   nir_instr instr;

   struct exec_list srcs; /** < list of nir_phi_src */

   nir_dest dest;
} nir_phi_instr;

typedef struct {
   struct exec_node node;
   nir_src src;
   nir_dest dest;
} nir_parallel_copy_entry;

#define nir_foreach_parallel_copy_entry(pcopy, entry) \
   foreach_list_typed(nir_parallel_copy_entry, entry, node, &(pcopy)->entries)

typedef struct {
   nir_instr instr;

   /* A list of nir_parallel_copy_entry's.  The sources of all of the
    * entries are copied to the corresponding destinations "in parallel".
    * In other words, if we have two entries: a -> b and b -> a, the values
    * get swapped.
    */
   struct exec_list entries;
} nir_parallel_copy_instr;

NIR_DEFINE_CAST(nir_instr_as_alu, nir_instr, nir_alu_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_call, nir_instr, nir_call_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_jump, nir_instr, nir_jump_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_tex, nir_instr, nir_tex_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_intrinsic, nir_instr, nir_intrinsic_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_load_const, nir_instr, nir_load_const_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_ssa_undef, nir_instr, nir_ssa_undef_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_phi, nir_instr, nir_phi_instr, instr)
NIR_DEFINE_CAST(nir_instr_as_parallel_copy, nir_instr,
                nir_parallel_copy_instr, instr)

/*
 * Control flow
 *
 * Control flow consists of a tree of control flow nodes, which include
 * if-statements and loops. The leaves of the tree are basic blocks, lists of
 * instructions that always run start-to-finish. Each basic block also keeps
 * track of its successors (blocks which may run immediately after the current
 * block) and predecessors (blocks which could have run immediately before the
 * current block). Each function also has a start block and an end block which
 * all return statements point to (which is always empty). Together, all the
 * blocks with their predecessors and successors make up the control flow
 * graph (CFG) of the function. There are helpers that modify the tree of
 * control flow nodes while modifying the CFG appropriately; these should be
 * used instead of modifying the tree directly.
 */

typedef enum {
   nir_cf_node_block,
   nir_cf_node_if,
   nir_cf_node_loop,
   nir_cf_node_function
} nir_cf_node_type;

typedef struct nir_cf_node {
   struct exec_node node;
   nir_cf_node_type type;
   struct nir_cf_node *parent;
} nir_cf_node;

typedef struct nir_block {
   nir_cf_node cf_node;

   struct exec_list instr_list; /** < list of nir_instr */

   /** generic block index; generated by nir_index_blocks */
   unsigned index;

   /*
    * Each block can only have up to 2 successors, so we put them in a simple
    * array - no need for anything more complicated.
    */
   struct nir_block *successors[2];

   /* Set of nir_block predecessors in the CFG */
   struct set *predecessors;

   /*
    * this node's immediate dominator in the dominance tree - set to NULL for
    * the start block.
    */
   struct nir_block *imm_dom;

   /* This node's children in the dominance tree */
   unsigned num_dom_children;
   struct nir_block **dom_children;

   /* Set of nir_block's on the dominance frontier of this block */
   struct set *dom_frontier;

   /*
    * These two indices have the property that dom_{pre,post}_index for each
    * child of this block in the dominance tree will always be between
    * dom_pre_index and dom_post_index for this block, which makes testing if
    * a given block is dominated by another block an O(1) operation.
    */
   unsigned dom_pre_index, dom_post_index;

   /* live in and out for this block; used for liveness analysis */
   BITSET_WORD *live_in;
   BITSET_WORD *live_out;
} nir_block;

static inline nir_instr *
nir_block_first_instr(nir_block *block)
{
   struct exec_node *head = exec_list_get_head(&block->instr_list);
   return exec_node_data(nir_instr, head, node);
}

static inline nir_instr *
nir_block_last_instr(nir_block *block)
{
   struct exec_node *tail = exec_list_get_tail(&block->instr_list);
   return exec_node_data(nir_instr, tail, node);
}

#define nir_foreach_instr(block, instr) \
   foreach_list_typed(nir_instr, instr, node, &(block)->instr_list)
#define nir_foreach_instr_reverse(block, instr) \
   foreach_list_typed_reverse(nir_instr, instr, node, &(block)->instr_list)
#define nir_foreach_instr_safe(block, instr) \
   foreach_list_typed_safe(nir_instr, instr, node, &(block)->instr_list)

typedef struct nir_if {
   nir_cf_node cf_node;
   nir_src condition;

   struct exec_list then_list; /** < list of nir_cf_node */
   struct exec_list else_list; /** < list of nir_cf_node */
} nir_if;

static inline nir_cf_node *
nir_if_first_then_node(nir_if *if_stmt)
{
   struct exec_node *head = exec_list_get_head(&if_stmt->then_list);
   return exec_node_data(nir_cf_node, head, node);
}

static inline nir_cf_node *
nir_if_last_then_node(nir_if *if_stmt)
{
   struct exec_node *tail = exec_list_get_tail(&if_stmt->then_list);
   return exec_node_data(nir_cf_node, tail, node);
}

static inline nir_cf_node *
nir_if_first_else_node(nir_if *if_stmt)
{
   struct exec_node *head = exec_list_get_head(&if_stmt->else_list);
   return exec_node_data(nir_cf_node, head, node);
}

static inline nir_cf_node *
nir_if_last_else_node(nir_if *if_stmt)
{
   struct exec_node *tail = exec_list_get_tail(&if_stmt->else_list);
   return exec_node_data(nir_cf_node, tail, node);
}

typedef struct {
   nir_cf_node cf_node;

   struct exec_list body; /** < list of nir_cf_node */
} nir_loop;

static inline nir_cf_node *
nir_loop_first_cf_node(nir_loop *loop)
{
   return exec_node_data(nir_cf_node, exec_list_get_head(&loop->body), node);
}

static inline nir_cf_node *
nir_loop_last_cf_node(nir_loop *loop)
{
   return exec_node_data(nir_cf_node, exec_list_get_tail(&loop->body), node);
}

/**
 * Various bits of metadata that can may be created or required by
 * optimization and analysis passes
 */
typedef enum {
   nir_metadata_none = 0x0,
   nir_metadata_block_index = 0x1,
   nir_metadata_dominance = 0x2,
   nir_metadata_live_variables = 0x4,
} nir_metadata;

typedef struct {
   nir_cf_node cf_node;

   /** pointer to the overload of which this is an implementation */
   struct nir_function_overload *overload;

   struct exec_list body; /** < list of nir_cf_node */

   nir_block *start_block, *end_block;

   /** list for all local variables in the function */
   struct exec_list locals;

   /** array of variables used as parameters */
   unsigned num_params;
   nir_variable **params;

   /** variable used to hold the result of the function */
   nir_variable *return_var;

   /** list of local registers in the function */
   struct exec_list registers;

   /** next available local register index */
   unsigned reg_alloc;

   /** next available SSA value index */
   unsigned ssa_alloc;

   /* total number of basic blocks, only valid when block_index_dirty = false */
   unsigned num_blocks;

   nir_metadata valid_metadata;
} nir_function_impl;

static inline nir_cf_node *
nir_cf_node_next(nir_cf_node *node)
{
   struct exec_node *next = exec_node_get_next(&node->node);
   if (exec_node_is_tail_sentinel(next))
      return NULL;
   else
      return exec_node_data(nir_cf_node, next, node);
}

static inline nir_cf_node *
nir_cf_node_prev(nir_cf_node *node)
{
   struct exec_node *prev = exec_node_get_prev(&node->node);
   if (exec_node_is_head_sentinel(prev))
      return NULL;
   else
      return exec_node_data(nir_cf_node, prev, node);
}

static inline bool
nir_cf_node_is_first(const nir_cf_node *node)
{
   return exec_node_is_head_sentinel(node->node.prev);
}

static inline bool
nir_cf_node_is_last(const nir_cf_node *node)
{
   return exec_node_is_tail_sentinel(node->node.next);
}

NIR_DEFINE_CAST(nir_cf_node_as_block, nir_cf_node, nir_block, cf_node)
NIR_DEFINE_CAST(nir_cf_node_as_if, nir_cf_node, nir_if, cf_node)
NIR_DEFINE_CAST(nir_cf_node_as_loop, nir_cf_node, nir_loop, cf_node)
NIR_DEFINE_CAST(nir_cf_node_as_function, nir_cf_node, nir_function_impl, cf_node)

typedef enum {
   nir_parameter_in,
   nir_parameter_out,
   nir_parameter_inout,
} nir_parameter_type;

typedef struct {
   nir_parameter_type param_type;
   const struct glsl_type *type;
} nir_parameter;

typedef struct nir_function_overload {
   struct exec_node node;

   unsigned num_params;
   nir_parameter *params;
   const struct glsl_type *return_type;

   nir_function_impl *impl; /** < NULL if the overload is only declared yet */

   /** pointer to the function of which this is an overload */
   struct nir_function *function;
} nir_function_overload;

typedef struct nir_function {
   struct exec_node node;

   struct exec_list overload_list; /** < list of nir_function_overload */
   const char *name;
   struct nir_shader *shader;
} nir_function;

#define nir_function_first_overload(func) \
   exec_node_data(nir_function_overload, \
                  exec_list_get_head(&(func)->overload_list), node)

typedef struct nir_shader_compiler_options {
   bool lower_ffma;
   bool lower_flrp;
   bool lower_fpow;
   bool lower_fsat;
   bool lower_fsqrt;
   /** lowers fneg and ineg to fsub and isub. */
   bool lower_negate;
   /** lowers fsub and isub to fadd+fneg and iadd+ineg. */
   bool lower_sub;

   /* lower {slt,sge,seq,sne} to {flt,fge,feq,fne} + b2f: */
   bool lower_scmp;

   /**
    * Does the driver support real 32-bit integers?  (Otherwise, integers
    * are simulated by floats.)
    */
   bool native_integers;
} nir_shader_compiler_options;

typedef struct nir_shader {
   /** hash table of name -> uniform nir_variable */
   struct exec_list uniforms;

   /** hash table of name -> input nir_variable */
   struct exec_list inputs;

   /** hash table of name -> output nir_variable */
   struct exec_list outputs;

   /** Set of driver-specific options for the shader.
    *
    * The memory for the options is expected to be kept in a single static
    * copy by the driver.
    */
   const struct nir_shader_compiler_options *options;

   /** list of global variables in the shader */
   struct exec_list globals;

   /** list of system value variables in the shader */
   struct exec_list system_values;

   struct exec_list functions; /** < list of nir_function */

   /** list of global register in the shader */
   struct exec_list registers;

   /** next available global register index */
   unsigned reg_alloc;

   /**
    * the highest index a load_input_*, load_uniform_*, etc. intrinsic can
    * access plus one
    */
   unsigned num_inputs, num_uniforms, num_outputs;

   /** the number of uniforms that are only accessed directly */
   unsigned num_direct_uniforms;
} nir_shader;

#define nir_foreach_overload(shader, overload)                        \
   foreach_list_typed(nir_function, func, node, &(shader)->functions) \
      foreach_list_typed(nir_function_overload, overload, node, \
                         &(func)->overload_list)

nir_shader *nir_shader_create(void *mem_ctx,
                              const nir_shader_compiler_options *options);

/** creates a register, including assigning it an index and adding it to the list */
nir_register *nir_global_reg_create(nir_shader *shader);

nir_register *nir_local_reg_create(nir_function_impl *impl);

void nir_reg_remove(nir_register *reg);

/** creates a function and adds it to the shader's list of functions */
nir_function *nir_function_create(nir_shader *shader, const char *name);

/** creates a null function returning null */
nir_function_overload *nir_function_overload_create(nir_function *func);

nir_function_impl *nir_function_impl_create(nir_function_overload *func);

nir_block *nir_block_create(void *mem_ctx);
nir_if *nir_if_create(void *mem_ctx);
nir_loop *nir_loop_create(void *mem_ctx);

nir_function_impl *nir_cf_node_get_function(nir_cf_node *node);

/** puts a control flow node immediately after another control flow node */
void nir_cf_node_insert_after(nir_cf_node *node, nir_cf_node *after);

/** puts a control flow node immediately before another control flow node */
void nir_cf_node_insert_before(nir_cf_node *node, nir_cf_node *before);

/** puts a control flow node at the beginning of a list from an if, loop, or function */
void nir_cf_node_insert_begin(struct exec_list *list, nir_cf_node *node);

/** puts a control flow node at the end of a list from an if, loop, or function */
void nir_cf_node_insert_end(struct exec_list *list, nir_cf_node *node);

/** removes a control flow node, doing any cleanup necessary */
void nir_cf_node_remove(nir_cf_node *node);

/** requests that the given pieces of metadata be generated */
void nir_metadata_require(nir_function_impl *impl, nir_metadata required);
/** dirties all but the preserved metadata */
void nir_metadata_preserve(nir_function_impl *impl, nir_metadata preserved);

/** creates an instruction with default swizzle/writemask/etc. with NULL registers */
nir_alu_instr *nir_alu_instr_create(nir_shader *shader, nir_op op);

nir_jump_instr *nir_jump_instr_create(nir_shader *shader, nir_jump_type type);

nir_load_const_instr *nir_load_const_instr_create(nir_shader *shader,
                                                  unsigned num_components);

nir_intrinsic_instr *nir_intrinsic_instr_create(nir_shader *shader,
                                                nir_intrinsic_op op);

nir_call_instr *nir_call_instr_create(nir_shader *shader,
                                      nir_function_overload *callee);

nir_tex_instr *nir_tex_instr_create(nir_shader *shader, unsigned num_srcs);

nir_phi_instr *nir_phi_instr_create(nir_shader *shader);

nir_parallel_copy_instr *nir_parallel_copy_instr_create(nir_shader *shader);

nir_ssa_undef_instr *nir_ssa_undef_instr_create(nir_shader *shader,
                                                unsigned num_components);

nir_deref_var *nir_deref_var_create(void *mem_ctx, nir_variable *var);
nir_deref_array *nir_deref_array_create(void *mem_ctx);
nir_deref_struct *nir_deref_struct_create(void *mem_ctx, unsigned field_index);

nir_deref *nir_copy_deref(void *mem_ctx, nir_deref *deref);

nir_load_const_instr *
nir_deref_get_const_initializer_load(nir_shader *shader, nir_deref_var *deref);

void nir_instr_insert_before(nir_instr *instr, nir_instr *before);
void nir_instr_insert_after(nir_instr *instr, nir_instr *after);

void nir_instr_insert_before_block(nir_block *block, nir_instr *before);
void nir_instr_insert_after_block(nir_block *block, nir_instr *after);

void nir_instr_insert_before_cf(nir_cf_node *node, nir_instr *before);
void nir_instr_insert_after_cf(nir_cf_node *node, nir_instr *after);

void nir_instr_insert_before_cf_list(struct exec_list *list, nir_instr *before);
void nir_instr_insert_after_cf_list(struct exec_list *list, nir_instr *after);

void nir_instr_remove(nir_instr *instr);

typedef bool (*nir_foreach_ssa_def_cb)(nir_ssa_def *def, void *state);
typedef bool (*nir_foreach_dest_cb)(nir_dest *dest, void *state);
typedef bool (*nir_foreach_src_cb)(nir_src *src, void *state);
bool nir_foreach_ssa_def(nir_instr *instr, nir_foreach_ssa_def_cb cb,
                         void *state);
bool nir_foreach_dest(nir_instr *instr, nir_foreach_dest_cb cb, void *state);
bool nir_foreach_src(nir_instr *instr, nir_foreach_src_cb cb, void *state);

nir_const_value *nir_src_as_const_value(nir_src src);
bool nir_srcs_equal(nir_src src1, nir_src src2);
void nir_instr_rewrite_src(nir_instr *instr, nir_src *src, nir_src new_src);
void nir_instr_move_src(nir_instr *dest_instr, nir_src *dest, nir_src *src);
void nir_if_rewrite_condition(nir_if *if_stmt, nir_src new_src);

void nir_ssa_dest_init(nir_instr *instr, nir_dest *dest,
                       unsigned num_components, const char *name);
void nir_ssa_def_init(nir_instr *instr, nir_ssa_def *def,
                      unsigned num_components, const char *name);
void nir_ssa_def_rewrite_uses(nir_ssa_def *def, nir_src new_src, void *mem_ctx);

/* visits basic blocks in source-code order */
typedef bool (*nir_foreach_block_cb)(nir_block *block, void *state);
bool nir_foreach_block(nir_function_impl *impl, nir_foreach_block_cb cb,
                       void *state);
bool nir_foreach_block_reverse(nir_function_impl *impl, nir_foreach_block_cb cb,
                               void *state);

/* If the following CF node is an if, this function returns that if.
 * Otherwise, it returns NULL.
 */
nir_if *nir_block_get_following_if(nir_block *block);

void nir_index_local_regs(nir_function_impl *impl);
void nir_index_global_regs(nir_shader *shader);
void nir_index_ssa_defs(nir_function_impl *impl);

void nir_index_blocks(nir_function_impl *impl);

void nir_print_shader(nir_shader *shader, FILE *fp);
void nir_print_instr(const nir_instr *instr, FILE *fp);

#ifdef DEBUG
void nir_validate_shader(nir_shader *shader);
#else
static inline void nir_validate_shader(nir_shader *shader) { (void) shader; }
#endif /* DEBUG */

void nir_calc_dominance_impl(nir_function_impl *impl);
void nir_calc_dominance(nir_shader *shader);

nir_block *nir_dominance_lca(nir_block *b1, nir_block *b2);
bool nir_block_dominates(nir_block *parent, nir_block *child);

void nir_dump_dom_tree_impl(nir_function_impl *impl, FILE *fp);
void nir_dump_dom_tree(nir_shader *shader, FILE *fp);

void nir_dump_dom_frontier_impl(nir_function_impl *impl, FILE *fp);
void nir_dump_dom_frontier(nir_shader *shader, FILE *fp);

void nir_dump_cfg_impl(nir_function_impl *impl, FILE *fp);
void nir_dump_cfg(nir_shader *shader, FILE *fp);

void nir_split_var_copies(nir_shader *shader);

void nir_lower_var_copy_instr(nir_intrinsic_instr *copy, void *mem_ctx);
void nir_lower_var_copies(nir_shader *shader);

void nir_lower_global_vars_to_local(nir_shader *shader);

void nir_lower_locals_to_regs(nir_shader *shader);

void nir_assign_var_locations_scalar(struct exec_list *var_list,
                                     unsigned *size);
void nir_assign_var_locations_scalar_direct_first(nir_shader *shader,
                                                  struct exec_list *var_list,
                                                  unsigned *direct_size,
                                                  unsigned *size);

void nir_lower_io(nir_shader *shader);

void nir_lower_vars_to_ssa(nir_shader *shader);

void nir_remove_dead_variables(nir_shader *shader);

void nir_lower_vec_to_movs(nir_shader *shader);
void nir_lower_alu_to_scalar(nir_shader *shader);

void nir_lower_phis_to_scalar(nir_shader *shader);

void nir_lower_samplers(nir_shader *shader,
                        const struct gl_shader_program *shader_program,
                        gl_shader_stage stage);

void nir_lower_system_values(nir_shader *shader);
void nir_lower_tex_projector(nir_shader *shader);
void nir_lower_idiv(nir_shader *shader);

void nir_lower_atomics(nir_shader *shader);
void nir_lower_to_source_mods(nir_shader *shader);

void nir_normalize_cubemap_coords(nir_shader *shader);

void nir_live_variables_impl(nir_function_impl *impl);
bool nir_ssa_defs_interfere(nir_ssa_def *a, nir_ssa_def *b);

void nir_convert_to_ssa_impl(nir_function_impl *impl);
void nir_convert_to_ssa(nir_shader *shader);
void nir_convert_from_ssa(nir_shader *shader);

bool nir_opt_algebraic(nir_shader *shader);
bool nir_opt_algebraic_late(nir_shader *shader);
bool nir_opt_constant_folding(nir_shader *shader);

bool nir_opt_global_to_local(nir_shader *shader);

bool nir_copy_prop_impl(nir_function_impl *impl);
bool nir_copy_prop(nir_shader *shader);

bool nir_opt_cse(nir_shader *shader);

bool nir_opt_dce_impl(nir_function_impl *impl);
bool nir_opt_dce(nir_shader *shader);

void nir_opt_gcm(nir_shader *shader);

bool nir_opt_peephole_select(nir_shader *shader);
bool nir_opt_peephole_ffma(nir_shader *shader);

bool nir_opt_remove_phis(nir_shader *shader);

void nir_sweep(nir_shader *shader);

#ifdef __cplusplus
} /* extern "C" */
#endif
