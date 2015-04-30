/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include "main/core.h" /* for Elements, MAX2 */
#include "glsl_parser_extras.h"
#include "glsl_types.h"
#include "program/hash_table.h"


mtx_t glsl_type::mutex = _MTX_INITIALIZER_NP;
hash_table *glsl_type::array_types = NULL;
hash_table *glsl_type::record_types = NULL;
hash_table *glsl_type::interface_types = NULL;
void *glsl_type::mem_ctx = NULL;

void
glsl_type::init_ralloc_type_ctx(void)
{
   if (glsl_type::mem_ctx == NULL) {
      glsl_type::mem_ctx = ralloc_autofree_context();
      assert(glsl_type::mem_ctx != NULL);
   }
}

glsl_type::glsl_type(GLenum gl_type,
		     glsl_base_type base_type, unsigned vector_elements,
		     unsigned matrix_columns, const char *name) :
   gl_type(gl_type),
   base_type(base_type),
   sampler_dimensionality(0), sampler_shadow(0), sampler_array(0),
   sampler_type(0), interface_packing(0),
   vector_elements(vector_elements), matrix_columns(matrix_columns),
   length(0)
{
   mtx_lock(&glsl_type::mutex);

   init_ralloc_type_ctx();
   assert(name != NULL);
   this->name = ralloc_strdup(this->mem_ctx, name);

   mtx_unlock(&glsl_type::mutex);

   /* Neither dimension is zero or both dimensions are zero.
    */
   assert((vector_elements == 0) == (matrix_columns == 0));
   memset(& fields, 0, sizeof(fields));
}

glsl_type::glsl_type(GLenum gl_type, glsl_base_type base_type,
		     enum glsl_sampler_dim dim, bool shadow, bool array,
		     unsigned type, const char *name) :
   gl_type(gl_type),
   base_type(base_type),
   sampler_dimensionality(dim), sampler_shadow(shadow),
   sampler_array(array), sampler_type(type), interface_packing(0),
   length(0)
{
   mtx_lock(&glsl_type::mutex);

   init_ralloc_type_ctx();
   assert(name != NULL);
   this->name = ralloc_strdup(this->mem_ctx, name);

   mtx_unlock(&glsl_type::mutex);

   memset(& fields, 0, sizeof(fields));

   if (base_type == GLSL_TYPE_SAMPLER) {
      /* Samplers take no storage whatsoever. */
      matrix_columns = vector_elements = 0;
   } else {
      matrix_columns = vector_elements = 1;
   }
}

glsl_type::glsl_type(const glsl_struct_field *fields, unsigned num_fields,
		     const char *name) :
   gl_type(0),
   base_type(GLSL_TYPE_STRUCT),
   sampler_dimensionality(0), sampler_shadow(0), sampler_array(0),
   sampler_type(0), interface_packing(0),
   vector_elements(0), matrix_columns(0),
   length(num_fields)
{
   unsigned int i;

   mtx_lock(&glsl_type::mutex);

   init_ralloc_type_ctx();
   assert(name != NULL);
   this->name = ralloc_strdup(this->mem_ctx, name);
   this->fields.structure = ralloc_array(this->mem_ctx,
					 glsl_struct_field, length);

   for (i = 0; i < length; i++) {
      this->fields.structure[i].type = fields[i].type;
      this->fields.structure[i].name = ralloc_strdup(this->fields.structure,
						     fields[i].name);
      this->fields.structure[i].location = fields[i].location;
      this->fields.structure[i].interpolation = fields[i].interpolation;
      this->fields.structure[i].centroid = fields[i].centroid;
      this->fields.structure[i].sample = fields[i].sample;
      this->fields.structure[i].matrix_layout = fields[i].matrix_layout;
   }

   mtx_unlock(&glsl_type::mutex);
}

glsl_type::glsl_type(const glsl_struct_field *fields, unsigned num_fields,
		     enum glsl_interface_packing packing, const char *name) :
   gl_type(0),
   base_type(GLSL_TYPE_INTERFACE),
   sampler_dimensionality(0), sampler_shadow(0), sampler_array(0),
   sampler_type(0), interface_packing((unsigned) packing),
   vector_elements(0), matrix_columns(0),
   length(num_fields)
{
   unsigned int i;

   mtx_lock(&glsl_type::mutex);

   init_ralloc_type_ctx();
   assert(name != NULL);
   this->name = ralloc_strdup(this->mem_ctx, name);
   this->fields.structure = ralloc_array(this->mem_ctx,
					 glsl_struct_field, length);
   for (i = 0; i < length; i++) {
      this->fields.structure[i].type = fields[i].type;
      this->fields.structure[i].name = ralloc_strdup(this->fields.structure,
						     fields[i].name);
      this->fields.structure[i].location = fields[i].location;
      this->fields.structure[i].interpolation = fields[i].interpolation;
      this->fields.structure[i].centroid = fields[i].centroid;
      this->fields.structure[i].sample = fields[i].sample;
      this->fields.structure[i].matrix_layout = fields[i].matrix_layout;
   }

   mtx_unlock(&glsl_type::mutex);
}


bool
glsl_type::contains_sampler() const
{
   if (this->is_array()) {
      return this->fields.array->contains_sampler();
   } else if (this->is_record()) {
      for (unsigned int i = 0; i < this->length; i++) {
	 if (this->fields.structure[i].type->contains_sampler())
	    return true;
      }
      return false;
   } else {
      return this->is_sampler();
   }
}


bool
glsl_type::contains_integer() const
{
   if (this->is_array()) {
      return this->fields.array->contains_integer();
   } else if (this->is_record()) {
      for (unsigned int i = 0; i < this->length; i++) {
	 if (this->fields.structure[i].type->contains_integer())
	    return true;
      }
      return false;
   } else {
      return this->is_integer();
   }
}

bool
glsl_type::contains_double() const
{
   if (this->is_array()) {
      return this->fields.array->contains_double();
   } else if (this->is_record()) {
      for (unsigned int i = 0; i < this->length; i++) {
	 if (this->fields.structure[i].type->contains_double())
	    return true;
      }
      return false;
   } else {
      return this->is_double();
   }
}

bool
glsl_type::contains_opaque() const {
   switch (base_type) {
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
   case GLSL_TYPE_ATOMIC_UINT:
      return true;
   case GLSL_TYPE_ARRAY:
      return element_type()->contains_opaque();
   case GLSL_TYPE_STRUCT:
      for (unsigned int i = 0; i < length; i++) {
         if (fields.structure[i].type->contains_opaque())
            return true;
      }
      return false;
   default:
      return false;
   }
}

gl_texture_index
glsl_type::sampler_index() const
{
   const glsl_type *const t = (this->is_array()) ? this->fields.array : this;

   assert(t->is_sampler());

   switch (t->sampler_dimensionality) {
   case GLSL_SAMPLER_DIM_1D:
      return (t->sampler_array) ? TEXTURE_1D_ARRAY_INDEX : TEXTURE_1D_INDEX;
   case GLSL_SAMPLER_DIM_2D:
      return (t->sampler_array) ? TEXTURE_2D_ARRAY_INDEX : TEXTURE_2D_INDEX;
   case GLSL_SAMPLER_DIM_3D:
      return TEXTURE_3D_INDEX;
   case GLSL_SAMPLER_DIM_CUBE:
      return (t->sampler_array) ? TEXTURE_CUBE_ARRAY_INDEX : TEXTURE_CUBE_INDEX;
   case GLSL_SAMPLER_DIM_RECT:
      return TEXTURE_RECT_INDEX;
   case GLSL_SAMPLER_DIM_BUF:
      return TEXTURE_BUFFER_INDEX;
   case GLSL_SAMPLER_DIM_EXTERNAL:
      return TEXTURE_EXTERNAL_INDEX;
   case GLSL_SAMPLER_DIM_MS:
      return (t->sampler_array) ? TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX : TEXTURE_2D_MULTISAMPLE_INDEX;
   default:
      assert(!"Should not get here.");
      return TEXTURE_BUFFER_INDEX;
   }
}

bool
glsl_type::contains_image() const
{
   if (this->is_array()) {
      return this->fields.array->contains_image();
   } else if (this->is_record()) {
      for (unsigned int i = 0; i < this->length; i++) {
	 if (this->fields.structure[i].type->contains_image())
	    return true;
      }
      return false;
   } else {
      return this->is_image();
   }
}

const glsl_type *glsl_type::get_base_type() const
{
   switch (base_type) {
   case GLSL_TYPE_UINT:
      return uint_type;
   case GLSL_TYPE_INT:
      return int_type;
   case GLSL_TYPE_FLOAT:
      return float_type;
   case GLSL_TYPE_DOUBLE:
      return double_type;
   case GLSL_TYPE_BOOL:
      return bool_type;
   default:
      return error_type;
   }
}


const glsl_type *glsl_type::get_scalar_type() const
{
   const glsl_type *type = this;

   /* Handle arrays */
   while (type->base_type == GLSL_TYPE_ARRAY)
      type = type->fields.array;

   /* Handle vectors and matrices */
   switch (type->base_type) {
   case GLSL_TYPE_UINT:
      return uint_type;
   case GLSL_TYPE_INT:
      return int_type;
   case GLSL_TYPE_FLOAT:
      return float_type;
   case GLSL_TYPE_DOUBLE:
      return double_type;
   case GLSL_TYPE_BOOL:
      return bool_type;
   default:
      /* Handle everything else */
      return type;
   }
}


void
_mesa_glsl_release_types(void)
{
   mtx_lock(&glsl_type::mutex);

   if (glsl_type::array_types != NULL) {
      hash_table_dtor(glsl_type::array_types);
      glsl_type::array_types = NULL;
   }

   if (glsl_type::record_types != NULL) {
      hash_table_dtor(glsl_type::record_types);
      glsl_type::record_types = NULL;
   }

   mtx_unlock(&glsl_type::mutex);
}


glsl_type::glsl_type(const glsl_type *array, unsigned length) :
   base_type(GLSL_TYPE_ARRAY),
   sampler_dimensionality(0), sampler_shadow(0), sampler_array(0),
   sampler_type(0), interface_packing(0),
   vector_elements(0), matrix_columns(0),
   length(length), name(NULL)
{
   this->fields.array = array;
   /* Inherit the gl type of the base. The GL type is used for
    * uniform/statevar handling in Mesa and the arrayness of the type
    * is represented by the size rather than the type.
    */
   this->gl_type = array->gl_type;

   /* Allow a maximum of 10 characters for the array size.  This is enough
    * for 32-bits of ~0.  The extra 3 are for the '[', ']', and terminating
    * NUL.
    */
   const unsigned name_length = strlen(array->name) + 10 + 3;

   mtx_lock(&glsl_type::mutex);
   char *const n = (char *) ralloc_size(this->mem_ctx, name_length);
   mtx_unlock(&glsl_type::mutex);

   if (length == 0)
      snprintf(n, name_length, "%s[]", array->name);
   else {
      /* insert outermost dimensions in the correct spot
       * otherwise the dimension order will be backwards
       */
      const char *pos = strchr(array->name, '[');
      if (pos) {
         int idx = pos - array->name;
         snprintf(n, idx+1, "%s", array->name);
         snprintf(n + idx, name_length - idx, "[%u]%s",
                  length, array->name + idx);
      } else {
         snprintf(n, name_length, "%s[%u]", array->name, length);
      }
   }

   this->name = n;
}


const glsl_type *
glsl_type::vec(unsigned components)
{
   if (components == 0 || components > 4)
      return error_type;

   static const glsl_type *const ts[] = {
      float_type, vec2_type, vec3_type, vec4_type
   };
   return ts[components - 1];
}

const glsl_type *
glsl_type::dvec(unsigned components)
{
   if (components == 0 || components > 4)
      return error_type;

   static const glsl_type *const ts[] = {
      double_type, dvec2_type, dvec3_type, dvec4_type
   };
   return ts[components - 1];
}

const glsl_type *
glsl_type::ivec(unsigned components)
{
   if (components == 0 || components > 4)
      return error_type;

   static const glsl_type *const ts[] = {
      int_type, ivec2_type, ivec3_type, ivec4_type
   };
   return ts[components - 1];
}


const glsl_type *
glsl_type::uvec(unsigned components)
{
   if (components == 0 || components > 4)
      return error_type;

   static const glsl_type *const ts[] = {
      uint_type, uvec2_type, uvec3_type, uvec4_type
   };
   return ts[components - 1];
}


const glsl_type *
glsl_type::bvec(unsigned components)
{
   if (components == 0 || components > 4)
      return error_type;

   static const glsl_type *const ts[] = {
      bool_type, bvec2_type, bvec3_type, bvec4_type
   };
   return ts[components - 1];
}


const glsl_type *
glsl_type::get_instance(unsigned base_type, unsigned rows, unsigned columns)
{
   if (base_type == GLSL_TYPE_VOID)
      return void_type;

   if ((rows < 1) || (rows > 4) || (columns < 1) || (columns > 4))
      return error_type;

   /* Treat GLSL vectors as Nx1 matrices.
    */
   if (columns == 1) {
      switch (base_type) {
      case GLSL_TYPE_UINT:
	 return uvec(rows);
      case GLSL_TYPE_INT:
	 return ivec(rows);
      case GLSL_TYPE_FLOAT:
	 return vec(rows);
      case GLSL_TYPE_DOUBLE:
	 return dvec(rows);
      case GLSL_TYPE_BOOL:
	 return bvec(rows);
      default:
	 return error_type;
      }
   } else {
      if ((base_type != GLSL_TYPE_FLOAT && base_type != GLSL_TYPE_DOUBLE) || (rows == 1))
	 return error_type;

      /* GLSL matrix types are named mat{COLUMNS}x{ROWS}.  Only the following
       * combinations are valid:
       *
       *   1 2 3 4
       * 1
       * 2   x x x
       * 3   x x x
       * 4   x x x
       */
#define IDX(c,r) (((c-1)*3) + (r-1))

      if (base_type == GLSL_TYPE_DOUBLE) {
         switch (IDX(columns, rows)) {
         case IDX(2,2): return dmat2_type;
         case IDX(2,3): return dmat2x3_type;
         case IDX(2,4): return dmat2x4_type;
         case IDX(3,2): return dmat3x2_type;
         case IDX(3,3): return dmat3_type;
         case IDX(3,4): return dmat3x4_type;
         case IDX(4,2): return dmat4x2_type;
         case IDX(4,3): return dmat4x3_type;
         case IDX(4,4): return dmat4_type;
         default: return error_type;
         }
      } else {
         switch (IDX(columns, rows)) {
         case IDX(2,2): return mat2_type;
         case IDX(2,3): return mat2x3_type;
         case IDX(2,4): return mat2x4_type;
         case IDX(3,2): return mat3x2_type;
         case IDX(3,3): return mat3_type;
         case IDX(3,4): return mat3x4_type;
         case IDX(4,2): return mat4x2_type;
         case IDX(4,3): return mat4x3_type;
         case IDX(4,4): return mat4_type;
         default: return error_type;
         }
      }
   }

   assert(!"Should not get here.");
   return error_type;
}

const glsl_type *
glsl_type::get_sampler_instance(enum glsl_sampler_dim dim,
                                bool shadow,
                                bool array,
                                glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_FLOAT:
      switch (dim) {
      case GLSL_SAMPLER_DIM_1D:
         if (shadow)
            return (array ? sampler1DArrayShadow_type : sampler1DShadow_type);
         else
            return (array ? sampler1DArray_type : sampler1D_type);
      case GLSL_SAMPLER_DIM_2D:
         if (shadow)
            return (array ? sampler2DArrayShadow_type : sampler2DShadow_type);
         else
            return (array ? sampler2DArray_type : sampler2D_type);
      case GLSL_SAMPLER_DIM_3D:
         if (shadow || array)
            return error_type;
         else
            return sampler3D_type;
      case GLSL_SAMPLER_DIM_CUBE:
         if (shadow)
            return (array ? samplerCubeArrayShadow_type : samplerCubeShadow_type);
         else
            return (array ? samplerCubeArray_type : samplerCube_type);
      case GLSL_SAMPLER_DIM_RECT:
         if (array)
            return error_type;
         if (shadow)
            return sampler2DRectShadow_type;
         else
            return sampler2DRect_type;
      case GLSL_SAMPLER_DIM_BUF:
         if (shadow || array)
            return error_type;
         else
            return samplerBuffer_type;
      case GLSL_SAMPLER_DIM_MS:
         if (shadow)
            return error_type;
         return (array ? sampler2DMSArray_type : sampler2DMS_type);
      case GLSL_SAMPLER_DIM_EXTERNAL:
         if (shadow || array)
            return error_type;
         else
            return samplerExternalOES_type;
      }
   case GLSL_TYPE_INT:
      if (shadow)
         return error_type;
      switch (dim) {
      case GLSL_SAMPLER_DIM_1D:
         return (array ? isampler1DArray_type : isampler1D_type);
      case GLSL_SAMPLER_DIM_2D:
         return (array ? isampler2DArray_type : isampler2D_type);
      case GLSL_SAMPLER_DIM_3D:
         if (array)
            return error_type;
         return isampler3D_type;
      case GLSL_SAMPLER_DIM_CUBE:
         return (array ? isamplerCubeArray_type : isamplerCube_type);
      case GLSL_SAMPLER_DIM_RECT:
         if (array)
            return error_type;
         return isampler2DRect_type;
      case GLSL_SAMPLER_DIM_BUF:
         if (array)
            return error_type;
         return isamplerBuffer_type;
      case GLSL_SAMPLER_DIM_MS:
         return (array ? isampler2DMSArray_type : isampler2DMS_type);
      case GLSL_SAMPLER_DIM_EXTERNAL:
         return error_type;
      }
   case GLSL_TYPE_UINT:
      if (shadow)
         return error_type;
      switch (dim) {
      case GLSL_SAMPLER_DIM_1D:
         return (array ? usampler1DArray_type : usampler1D_type);
      case GLSL_SAMPLER_DIM_2D:
         return (array ? usampler2DArray_type : usampler2D_type);
      case GLSL_SAMPLER_DIM_3D:
         if (array)
            return error_type;
         return usampler3D_type;
      case GLSL_SAMPLER_DIM_CUBE:
         return (array ? usamplerCubeArray_type : usamplerCube_type);
      case GLSL_SAMPLER_DIM_RECT:
         if (array)
            return error_type;
         return usampler2DRect_type;
      case GLSL_SAMPLER_DIM_BUF:
         if (array)
            return error_type;
         return usamplerBuffer_type;
      case GLSL_SAMPLER_DIM_MS:
         return (array ? usampler2DMSArray_type : usampler2DMS_type);
      case GLSL_SAMPLER_DIM_EXTERNAL:
         return error_type;
      }
   default:
      return error_type;
   }

   unreachable("switch statement above should be complete");
}

const glsl_type *
glsl_type::get_array_instance(const glsl_type *base, unsigned array_size)
{
   /* Generate a name using the base type pointer in the key.  This is
    * done because the name of the base type may not be unique across
    * shaders.  For example, two shaders may have different record types
    * named 'foo'.
    */
   char key[128];
   snprintf(key, sizeof(key), "%p[%u]", (void *) base, array_size);

   mtx_lock(&glsl_type::mutex);

   if (array_types == NULL) {
      array_types = hash_table_ctor(64, hash_table_string_hash,
				    hash_table_string_compare);
   }

   const glsl_type *t = (glsl_type *) hash_table_find(array_types, key);

   if (t == NULL) {
      mtx_unlock(&glsl_type::mutex);
      t = new glsl_type(base, array_size);
      mtx_lock(&glsl_type::mutex);

      hash_table_insert(array_types, (void *) t, ralloc_strdup(mem_ctx, key));
   }

   assert(t->base_type == GLSL_TYPE_ARRAY);
   assert(t->length == array_size);
   assert(t->fields.array == base);

   mtx_unlock(&glsl_type::mutex);

   return t;
}


bool
glsl_type::record_compare(const glsl_type *b) const
{
   if (this->length != b->length)
      return false;

   if (this->interface_packing != b->interface_packing)
      return false;

   /* From the GLSL 4.20 specification (Sec 4.2):
    *
    *     "Structures must have the same name, sequence of type names, and
    *     type definitions, and field names to be considered the same type."
    *
    * GLSL ES behaves the same (Ver 1.00 Sec 4.2.4, Ver 3.00 Sec 4.2.5).
    *
    * Note that we cannot force type name check when comparing unnamed
    * structure types, these have a unique name assigned during parsing.
    */
   if (!this->is_anonymous() && !b->is_anonymous())
      if (strcmp(this->name, b->name) != 0)
         return false;

   for (unsigned i = 0; i < this->length; i++) {
      if (this->fields.structure[i].type != b->fields.structure[i].type)
	 return false;
      if (strcmp(this->fields.structure[i].name,
		 b->fields.structure[i].name) != 0)
	 return false;
      if (this->fields.structure[i].matrix_layout
         != b->fields.structure[i].matrix_layout)
        return false;
      if (this->fields.structure[i].location
          != b->fields.structure[i].location)
         return false;
      if (this->fields.structure[i].interpolation
          != b->fields.structure[i].interpolation)
         return false;
      if (this->fields.structure[i].centroid
          != b->fields.structure[i].centroid)
         return false;
      if (this->fields.structure[i].sample
          != b->fields.structure[i].sample)
         return false;
   }

   return true;
}


int
glsl_type::record_key_compare(const void *a, const void *b)
{
   const glsl_type *const key1 = (glsl_type *) a;
   const glsl_type *const key2 = (glsl_type *) b;

   /* Return zero is the types match (there is zero difference) or non-zero
    * otherwise.
    */
   if (strcmp(key1->name, key2->name) != 0)
      return 1;

   return !key1->record_compare(key2);
}


/**
 * Generate an integer hash value for a glsl_type structure type.
 */
unsigned
glsl_type::record_key_hash(const void *a)
{
   const glsl_type *const key = (glsl_type *) a;
   uintptr_t hash = key->length;
   unsigned retval;

   for (unsigned i = 0; i < key->length; i++) {
      /* casting pointer to uintptr_t */
      hash = (hash * 13 ) + (uintptr_t) key->fields.structure[i].type;
   }

   if (sizeof(hash) == 8)
      retval = (hash & 0xffffffff) ^ ((uint64_t) hash >> 32);
   else
      retval = hash;

   return retval;
}


const glsl_type *
glsl_type::get_record_instance(const glsl_struct_field *fields,
			       unsigned num_fields,
			       const char *name)
{
   const glsl_type key(fields, num_fields, name);

   mtx_lock(&glsl_type::mutex);

   if (record_types == NULL) {
      record_types = hash_table_ctor(64, record_key_hash, record_key_compare);
   }

   const glsl_type *t = (glsl_type *) hash_table_find(record_types, & key);
   if (t == NULL) {
      mtx_unlock(&glsl_type::mutex);
      t = new glsl_type(fields, num_fields, name);
      mtx_lock(&glsl_type::mutex);

      hash_table_insert(record_types, (void *) t, t);
   }

   assert(t->base_type == GLSL_TYPE_STRUCT);
   assert(t->length == num_fields);
   assert(strcmp(t->name, name) == 0);

   mtx_unlock(&glsl_type::mutex);

   return t;
}


const glsl_type *
glsl_type::get_interface_instance(const glsl_struct_field *fields,
				  unsigned num_fields,
				  enum glsl_interface_packing packing,
				  const char *block_name)
{
   const glsl_type key(fields, num_fields, packing, block_name);

   mtx_lock(&glsl_type::mutex);

   if (interface_types == NULL) {
      interface_types = hash_table_ctor(64, record_key_hash, record_key_compare);
   }

   const glsl_type *t = (glsl_type *) hash_table_find(interface_types, & key);
   if (t == NULL) {
      mtx_unlock(&glsl_type::mutex);
      t = new glsl_type(fields, num_fields, packing, block_name);
      mtx_lock(&glsl_type::mutex);

      hash_table_insert(interface_types, (void *) t, t);
   }

   assert(t->base_type == GLSL_TYPE_INTERFACE);
   assert(t->length == num_fields);
   assert(strcmp(t->name, block_name) == 0);

   mtx_unlock(&glsl_type::mutex);

   return t;
}


const glsl_type *
glsl_type::get_mul_type(const glsl_type *type_a, const glsl_type *type_b)
{
   if (type_a == type_b) {
      return type_a;
   } else if (type_a->is_matrix() && type_b->is_matrix()) {
      /* Matrix multiply.  The columns of A must match the rows of B.  Given
       * the other previously tested constraints, this means the vector type
       * of a row from A must be the same as the vector type of a column from
       * B.
       */
      if (type_a->row_type() == type_b->column_type()) {
         /* The resulting matrix has the number of columns of matrix B and
          * the number of rows of matrix A.  We get the row count of A by
          * looking at the size of a vector that makes up a column.  The
          * transpose (size of a row) is done for B.
          */
         const glsl_type *const type =
            get_instance(type_a->base_type,
                         type_a->column_type()->vector_elements,
                         type_b->row_type()->vector_elements);
         assert(type != error_type);

         return type;
      }
   } else if (type_a->is_matrix()) {
      /* A is a matrix and B is a column vector.  Columns of A must match
       * rows of B.  Given the other previously tested constraints, this
       * means the vector type of a row from A must be the same as the
       * vector the type of B.
       */
      if (type_a->row_type() == type_b) {
         /* The resulting vector has a number of elements equal to
          * the number of rows of matrix A. */
         const glsl_type *const type =
            get_instance(type_a->base_type,
                         type_a->column_type()->vector_elements,
                         1);
         assert(type != error_type);

         return type;
      }
   } else {
      assert(type_b->is_matrix());

      /* A is a row vector and B is a matrix.  Columns of A must match rows
       * of B.  Given the other previously tested constraints, this means
       * the type of A must be the same as the vector type of a column from
       * B.
       */
      if (type_a == type_b->column_type()) {
         /* The resulting vector has a number of elements equal to
          * the number of columns of matrix B. */
         const glsl_type *const type =
            get_instance(type_a->base_type,
                         type_b->row_type()->vector_elements,
                         1);
         assert(type != error_type);

         return type;
      }
   }

   return error_type;
}


const glsl_type *
glsl_type::field_type(const char *name) const
{
   if (this->base_type != GLSL_TYPE_STRUCT
       && this->base_type != GLSL_TYPE_INTERFACE)
      return error_type;

   for (unsigned i = 0; i < this->length; i++) {
      if (strcmp(name, this->fields.structure[i].name) == 0)
	 return this->fields.structure[i].type;
   }

   return error_type;
}


int
glsl_type::field_index(const char *name) const
{
   if (this->base_type != GLSL_TYPE_STRUCT
       && this->base_type != GLSL_TYPE_INTERFACE)
      return -1;

   for (unsigned i = 0; i < this->length; i++) {
      if (strcmp(name, this->fields.structure[i].name) == 0)
	 return i;
   }

   return -1;
}


unsigned
glsl_type::component_slots() const
{
   switch (this->base_type) {
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_BOOL:
      return this->components();

   case GLSL_TYPE_DOUBLE:
      return 2 * this->components();

   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE: {
      unsigned size = 0;

      for (unsigned i = 0; i < this->length; i++)
	 size += this->fields.structure[i].type->component_slots();

      return size;
   }

   case GLSL_TYPE_ARRAY:
      return this->length * this->fields.array->component_slots();

   case GLSL_TYPE_IMAGE:
      return 1;

   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_ATOMIC_UINT:
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
      break;
   }

   return 0;
}

unsigned
glsl_type::uniform_locations() const
{
   unsigned size = 0;

   switch (this->base_type) {
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_DOUBLE:
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
      return 1;

   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE:
      for (unsigned i = 0; i < this->length; i++)
         size += this->fields.structure[i].type->uniform_locations();
      return size;
   case GLSL_TYPE_ARRAY:
      return this->length * this->fields.array->uniform_locations();
   default:
      return 0;
   }
}

bool
glsl_type::can_implicitly_convert_to(const glsl_type *desired,
                                     _mesa_glsl_parse_state *state) const
{
   if (this == desired)
      return true;

   /* There is no conversion among matrix types. */
   if (this->matrix_columns > 1 || desired->matrix_columns > 1)
      return false;

   /* Vector size must match. */
   if (this->vector_elements != desired->vector_elements)
      return false;

   /* int and uint can be converted to float. */
   if (desired->is_float() && this->is_integer())
      return true;

   /* With GLSL 4.0 / ARB_gpu_shader5, int can be converted to uint.
    * Note that state may be NULL here, when resolving function calls in the
    * linker. By this time, all the state-dependent checks have already
    * happened though, so allow anything that's allowed in any shader version. */
   if ((!state || state->is_version(400, 0) || state->ARB_gpu_shader5_enable) &&
         desired->base_type == GLSL_TYPE_UINT && this->base_type == GLSL_TYPE_INT)
      return true;

   /* No implicit conversions from double. */
   if ((!state || state->has_double()) && this->is_double())
      return false;

   /* Conversions from different types to double. */
   if ((!state || state->has_double()) && desired->is_double()) {
      if (this->is_float())
         return true;
      if (this->is_integer())
         return true;
   }

   return false;
}

unsigned
glsl_type::std140_base_alignment(bool row_major) const
{
   unsigned N = is_double() ? 8 : 4;

   /* (1) If the member is a scalar consuming <N> basic machine units, the
    *     base alignment is <N>.
    *
    * (2) If the member is a two- or four-component vector with components
    *     consuming <N> basic machine units, the base alignment is 2<N> or
    *     4<N>, respectively.
    *
    * (3) If the member is a three-component vector with components consuming
    *     <N> basic machine units, the base alignment is 4<N>.
    */
   if (this->is_scalar() || this->is_vector()) {
      switch (this->vector_elements) {
      case 1:
	 return N;
      case 2:
	 return 2 * N;
      case 3:
      case 4:
	 return 4 * N;
      }
   }

   /* (4) If the member is an array of scalars or vectors, the base alignment
    *     and array stride are set to match the base alignment of a single
    *     array element, according to rules (1), (2), and (3), and rounded up
    *     to the base alignment of a vec4. The array may have padding at the
    *     end; the base offset of the member following the array is rounded up
    *     to the next multiple of the base alignment.
    *
    * (6) If the member is an array of <S> column-major matrices with <C>
    *     columns and <R> rows, the matrix is stored identically to a row of
    *     <S>*<C> column vectors with <R> components each, according to rule
    *     (4).
    *
    * (8) If the member is an array of <S> row-major matrices with <C> columns
    *     and <R> rows, the matrix is stored identically to a row of <S>*<R>
    *     row vectors with <C> components each, according to rule (4).
    *
    * (10) If the member is an array of <S> structures, the <S> elements of
    *      the array are laid out in order, according to rule (9).
    */
   if (this->is_array()) {
      if (this->fields.array->is_scalar() ||
	  this->fields.array->is_vector() ||
	  this->fields.array->is_matrix()) {
	 return MAX2(this->fields.array->std140_base_alignment(row_major), 16);
      } else {
	 assert(this->fields.array->is_record());
	 return this->fields.array->std140_base_alignment(row_major);
      }
   }

   /* (5) If the member is a column-major matrix with <C> columns and
    *     <R> rows, the matrix is stored identically to an array of
    *     <C> column vectors with <R> components each, according to
    *     rule (4).
    *
    * (7) If the member is a row-major matrix with <C> columns and <R>
    *     rows, the matrix is stored identically to an array of <R>
    *     row vectors with <C> components each, according to rule (4).
    */
   if (this->is_matrix()) {
      const struct glsl_type *vec_type, *array_type;
      int c = this->matrix_columns;
      int r = this->vector_elements;

      if (row_major) {
	 vec_type = get_instance(base_type, c, 1);
	 array_type = glsl_type::get_array_instance(vec_type, r);
      } else {
	 vec_type = get_instance(base_type, r, 1);
	 array_type = glsl_type::get_array_instance(vec_type, c);
      }

      return array_type->std140_base_alignment(false);
   }

   /* (9) If the member is a structure, the base alignment of the
    *     structure is <N>, where <N> is the largest base alignment
    *     value of any of its members, and rounded up to the base
    *     alignment of a vec4. The individual members of this
    *     sub-structure are then assigned offsets by applying this set
    *     of rules recursively, where the base offset of the first
    *     member of the sub-structure is equal to the aligned offset
    *     of the structure. The structure may have padding at the end;
    *     the base offset of the member following the sub-structure is
    *     rounded up to the next multiple of the base alignment of the
    *     structure.
    */
   if (this->is_record()) {
      unsigned base_alignment = 16;
      for (unsigned i = 0; i < this->length; i++) {
         bool field_row_major = row_major;
         const enum glsl_matrix_layout matrix_layout =
            glsl_matrix_layout(this->fields.structure[i].matrix_layout);
         if (matrix_layout == GLSL_MATRIX_LAYOUT_ROW_MAJOR) {
            field_row_major = true;
         } else if (matrix_layout == GLSL_MATRIX_LAYOUT_COLUMN_MAJOR) {
            field_row_major = false;
         }

	 const struct glsl_type *field_type = this->fields.structure[i].type;
	 base_alignment = MAX2(base_alignment,
			       field_type->std140_base_alignment(field_row_major));
      }
      return base_alignment;
   }

   assert(!"not reached");
   return -1;
}

unsigned
glsl_type::std140_size(bool row_major) const
{
   unsigned N = is_double() ? 8 : 4;

   /* (1) If the member is a scalar consuming <N> basic machine units, the
    *     base alignment is <N>.
    *
    * (2) If the member is a two- or four-component vector with components
    *     consuming <N> basic machine units, the base alignment is 2<N> or
    *     4<N>, respectively.
    *
    * (3) If the member is a three-component vector with components consuming
    *     <N> basic machine units, the base alignment is 4<N>.
    */
   if (this->is_scalar() || this->is_vector()) {
      return this->vector_elements * N;
   }

   /* (5) If the member is a column-major matrix with <C> columns and
    *     <R> rows, the matrix is stored identically to an array of
    *     <C> column vectors with <R> components each, according to
    *     rule (4).
    *
    * (6) If the member is an array of <S> column-major matrices with <C>
    *     columns and <R> rows, the matrix is stored identically to a row of
    *     <S>*<C> column vectors with <R> components each, according to rule
    *     (4).
    *
    * (7) If the member is a row-major matrix with <C> columns and <R>
    *     rows, the matrix is stored identically to an array of <R>
    *     row vectors with <C> components each, according to rule (4).
    *
    * (8) If the member is an array of <S> row-major matrices with <C> columns
    *     and <R> rows, the matrix is stored identically to a row of <S>*<R>
    *     row vectors with <C> components each, according to rule (4).
    */
   if (this->without_array()->is_matrix()) {
      const struct glsl_type *element_type;
      const struct glsl_type *vec_type;
      unsigned int array_len;

      if (this->is_array()) {
	 element_type = this->fields.array;
	 array_len = this->length;
      } else {
	 element_type = this;
	 array_len = 1;
      }

      if (row_major) {
         vec_type = get_instance(element_type->base_type,
                                 element_type->matrix_columns, 1);

	 array_len *= element_type->vector_elements;
      } else {
	 vec_type = get_instance(element_type->base_type,
				 element_type->vector_elements, 1);
	 array_len *= element_type->matrix_columns;
      }
      const glsl_type *array_type = glsl_type::get_array_instance(vec_type,
								  array_len);

      return array_type->std140_size(false);
   }

   /* (4) If the member is an array of scalars or vectors, the base alignment
    *     and array stride are set to match the base alignment of a single
    *     array element, according to rules (1), (2), and (3), and rounded up
    *     to the base alignment of a vec4. The array may have padding at the
    *     end; the base offset of the member following the array is rounded up
    *     to the next multiple of the base alignment.
    *
    * (10) If the member is an array of <S> structures, the <S> elements of
    *      the array are laid out in order, according to rule (9).
    */
   if (this->is_array()) {
      if (this->fields.array->is_record()) {
	 return this->length * this->fields.array->std140_size(row_major);
      } else {
	 unsigned element_base_align =
	    this->fields.array->std140_base_alignment(row_major);
	 return this->length * MAX2(element_base_align, 16);
      }
   }

   /* (9) If the member is a structure, the base alignment of the
    *     structure is <N>, where <N> is the largest base alignment
    *     value of any of its members, and rounded up to the base
    *     alignment of a vec4. The individual members of this
    *     sub-structure are then assigned offsets by applying this set
    *     of rules recursively, where the base offset of the first
    *     member of the sub-structure is equal to the aligned offset
    *     of the structure. The structure may have padding at the end;
    *     the base offset of the member following the sub-structure is
    *     rounded up to the next multiple of the base alignment of the
    *     structure.
    */
   if (this->is_record()) {
      unsigned size = 0;
      unsigned max_align = 0;

      for (unsigned i = 0; i < this->length; i++) {
         bool field_row_major = row_major;
         const enum glsl_matrix_layout matrix_layout =
            glsl_matrix_layout(this->fields.structure[i].matrix_layout);
         if (matrix_layout == GLSL_MATRIX_LAYOUT_ROW_MAJOR) {
            field_row_major = true;
         } else if (matrix_layout == GLSL_MATRIX_LAYOUT_COLUMN_MAJOR) {
            field_row_major = false;
         }

	 const struct glsl_type *field_type = this->fields.structure[i].type;
	 unsigned align = field_type->std140_base_alignment(field_row_major);
	 size = glsl_align(size, align);
	 size += field_type->std140_size(field_row_major);

         max_align = MAX2(align, max_align);

         if (field_type->is_record() && (i + 1 < this->length))
            size = glsl_align(size, 16);
      }
      size = glsl_align(size, MAX2(max_align, 16));
      return size;
   }

   assert(!"not reached");
   return -1;
}


unsigned
glsl_type::count_attribute_slots() const
{
   /* From page 31 (page 37 of the PDF) of the GLSL 1.50 spec:
    *
    *     "A scalar input counts the same amount against this limit as a vec4,
    *     so applications may want to consider packing groups of four
    *     unrelated float inputs together into a vector to better utilize the
    *     capabilities of the underlying hardware. A matrix input will use up
    *     multiple locations.  The number of locations used will equal the
    *     number of columns in the matrix."
    *
    * The spec does not explicitly say how arrays are counted.  However, it
    * should be safe to assume the total number of slots consumed by an array
    * is the number of entries in the array multiplied by the number of slots
    * consumed by a single element of the array.
    *
    * The spec says nothing about how structs are counted, because vertex
    * attributes are not allowed to be (or contain) structs.  However, Mesa
    * allows varying structs, the number of varying slots taken up by a
    * varying struct is simply equal to the sum of the number of slots taken
    * up by each element.
    */
   switch (this->base_type) {
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_DOUBLE:
      return this->matrix_columns;

   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE: {
      unsigned size = 0;

      for (unsigned i = 0; i < this->length; i++)
         size += this->fields.structure[i].type->count_attribute_slots();

      return size;
   }

   case GLSL_TYPE_ARRAY:
      return this->length * this->fields.array->count_attribute_slots();

   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
   case GLSL_TYPE_ATOMIC_UINT:
   case GLSL_TYPE_VOID:
   case GLSL_TYPE_ERROR:
      break;
   }

   assert(!"Unexpected type in count_attribute_slots()");

   return 0;
}

int
glsl_type::coordinate_components() const
{
   int size;

   switch (sampler_dimensionality) {
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_BUF:
      size = 1;
      break;
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_RECT:
   case GLSL_SAMPLER_DIM_MS:
   case GLSL_SAMPLER_DIM_EXTERNAL:
      size = 2;
      break;
   case GLSL_SAMPLER_DIM_3D:
   case GLSL_SAMPLER_DIM_CUBE:
      size = 3;
      break;
   default:
      assert(!"Should not get here.");
      size = 1;
      break;
   }

   /* Array textures need an additional component for the array index, except
    * for cubemap array images that behave like a 2D array of interleaved
    * cubemap faces.
    */
   if (sampler_array &&
       !(base_type == GLSL_TYPE_IMAGE &&
         sampler_dimensionality == GLSL_SAMPLER_DIM_CUBE))
      size += 1;

   return size;
}
