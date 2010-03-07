/*
 * Mesa 3-D graphics library
 * Version:  7.3
 *
 * Copyright (C) 2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file slang_link.c
 * GLSL linker
 * \author Brian Paul
 */

#include "main/imports.h"
#include "main/context.h"
#include "main/hash.h"
#include "main/macros.h"
#include "shader/program.h"
#include "shader/prog_instruction.h"
#include "shader/prog_parameter.h"
#include "shader/prog_print.h"
#include "shader/prog_statevars.h"
#include "shader/prog_uniform.h"
#include "shader/shader_api.h"
#include "slang_builtin.h"
#include "slang_link.h"


/** cast wrapper */
static struct gl_vertex_program *
vertex_program(struct gl_program *prog)
{
   assert(prog->Target == GL_VERTEX_PROGRAM_ARB);
   return (struct gl_vertex_program *) prog;
}


/** cast wrapper */
static struct gl_fragment_program *
fragment_program(struct gl_program *prog)
{
   assert(prog->Target == GL_FRAGMENT_PROGRAM_ARB);
   return (struct gl_fragment_program *) prog;
}


/**
 * Record a linking error.
 */
static void
link_error(struct gl_shader_program *shProg, const char *msg)
{
   if (shProg->InfoLog) {
      _mesa_free(shProg->InfoLog);
   }
   shProg->InfoLog = _mesa_strdup(msg);
   shProg->LinkStatus = GL_FALSE;
}



/**
 * Check if the given bit is either set or clear in both bitfields.
 */
static GLboolean
bits_agree(GLbitfield flags1, GLbitfield flags2, GLbitfield bit)
{
   return (flags1 & bit) == (flags2 & bit);
}


/**
 * Linking varying vars involves rearranging varying vars so that the
 * vertex program's output varyings matches the order of the fragment
 * program's input varyings.
 * We'll then rewrite instructions to replace PROGRAM_VARYING with either
 * PROGRAM_INPUT or PROGRAM_OUTPUT depending on whether it's a vertex or
 * fragment shader.
 * This is also where we set program Input/OutputFlags to indicate
 * which inputs are centroid-sampled, invariant, etc.
 */
static GLboolean
link_varying_vars(GLcontext *ctx,
                  struct gl_shader_program *shProg, struct gl_program *prog)
{
   GLuint *map, i, firstVarying, newFile;
   GLbitfield *inOutFlags;

   map = (GLuint *) _mesa_malloc(prog->Varying->NumParameters * sizeof(GLuint));
   if (!map)
      return GL_FALSE;

   /* Varying variables are treated like other vertex program outputs
    * (and like other fragment program inputs).  The position of the
    * first varying differs for vertex/fragment programs...
    * Also, replace File=PROGRAM_VARYING with File=PROGRAM_INPUT/OUTPUT.
    */
   if (prog->Target == GL_VERTEX_PROGRAM_ARB) {
      firstVarying = VERT_RESULT_VAR0;
      newFile = PROGRAM_OUTPUT;
      inOutFlags = prog->OutputFlags;
   }
   else {
      assert(prog->Target == GL_FRAGMENT_PROGRAM_ARB);
      firstVarying = FRAG_ATTRIB_VAR0;
      newFile = PROGRAM_INPUT;
      inOutFlags = prog->InputFlags;
   }

   for (i = 0; i < prog->Varying->NumParameters; i++) {
      /* see if this varying is in the linked varying list */
      const struct gl_program_parameter *var = prog->Varying->Parameters + i;
      GLint j = _mesa_lookup_parameter_index(shProg->Varying, -1, var->Name);
      if (j >= 0) {
         /* varying is already in list, do some error checking */
         const struct gl_program_parameter *v =
            &shProg->Varying->Parameters[j];
         if (var->Size != v->Size) {
            link_error(shProg, "mismatched varying variable types");
            _mesa_free(map);
            return GL_FALSE;
         }
         if (!bits_agree(var->Flags, v->Flags, PROG_PARAM_BIT_CENTROID)) {
            char msg[100];
            _mesa_snprintf(msg, sizeof(msg),
                           "centroid modifier mismatch for '%s'", var->Name);
            link_error(shProg, msg);
            _mesa_free(map);
            return GL_FALSE;
         }
         if (!bits_agree(var->Flags, v->Flags, PROG_PARAM_BIT_INVARIANT)) {
            char msg[100];
            _mesa_snprintf(msg, sizeof(msg),
                           "invariant modifier mismatch for '%s'", var->Name);
            link_error(shProg, msg);
            _mesa_free(map);
            return GL_FALSE;
         }
      }
      else {
         /* not already in linked list */
         j = _mesa_add_varying(shProg->Varying, var->Name, var->Size,
                               var->Flags);
      }

      if (shProg->Varying->NumParameters > ctx->Const.MaxVarying) {
         link_error(shProg, "Too many varying variables");
         _mesa_free(map);
         return GL_FALSE;
      }

      /* Map varying[i] to varying[j].
       * Note: the loop here takes care of arrays or large (sz>4) vars.
       */
      {
         GLint sz = var->Size;
         while (sz > 0) {
            inOutFlags[firstVarying + j] = var->Flags;
            /*printf("Link varying from %d to %d\n", i, j);*/
            map[i++] = j++;
            sz -= 4;
         }
         i--; /* go back one */
      }
   }


   /* OK, now scan the program/shader instructions looking for varying vars,
    * replacing the old index with the new index.
    */
   for (i = 0; i < prog->NumInstructions; i++) {
      struct prog_instruction *inst = prog->Instructions + i;
      GLuint j;

      if (inst->DstReg.File == PROGRAM_VARYING) {
         inst->DstReg.File = newFile;
         inst->DstReg.Index = map[ inst->DstReg.Index ] + firstVarying;
      }

      for (j = 0; j < 3; j++) {
         if (inst->SrcReg[j].File == PROGRAM_VARYING) {
            inst->SrcReg[j].File = newFile;
            inst->SrcReg[j].Index = map[ inst->SrcReg[j].Index ] + firstVarying;
         }
      }
   }

   _mesa_free(map);

   /* these will get recomputed before linking is completed */
   prog->InputsRead = 0x0;
   prog->OutputsWritten = 0x0;

   return GL_TRUE;
}


/**
 * Build the shProg->Uniforms list.
 * This is basically a list/index of all uniforms found in either/both of
 * the vertex and fragment shaders.
 *
 * About uniforms:
 * Each uniform has two indexes, one that points into the vertex
 * program's parameter array and another that points into the fragment
 * program's parameter array.  When the user changes a uniform's value
 * we have to change the value in the vertex and/or fragment program's
 * parameter array.
 *
 * This function will be called twice to set up the two uniform->parameter
 * mappings.
 *
 * If a uniform is only present in the vertex program OR fragment program
 * then the fragment/vertex parameter index, respectively, will be -1.
 */
static GLboolean
link_uniform_vars(GLcontext *ctx,
                  struct gl_shader_program *shProg,
                  struct gl_program *prog,
                  GLuint *numSamplers)
{
   GLuint samplerMap[200]; /* max number of samplers declared, not used */
   GLuint i;

   for (i = 0; i < prog->Parameters->NumParameters; i++) {
      const struct gl_program_parameter *p = prog->Parameters->Parameters + i;

      /*
       * XXX FIX NEEDED HERE
       * We should also be adding a uniform if p->Type == PROGRAM_STATE_VAR.
       * For example, modelview matrix, light pos, etc.
       * Also, we need to update the state-var name-generator code to
       * generate GLSL-style names, like "gl_LightSource[0].position".
       * Furthermore, we'll need to fix the state-var's size/datatype info.
       */

      if ((p->Type == PROGRAM_UNIFORM || p->Type == PROGRAM_SAMPLER)
          && p->Used) {
         /* add this uniform, indexing into the target's Parameters list */
         struct gl_uniform *uniform =
            _mesa_append_uniform(shProg->Uniforms, p->Name, prog->Target, i);
         if (uniform)
            uniform->Initialized = p->Initialized;
      }

      /* The samplerMap[] table we build here is used to remap/re-index
       * sampler references by TEX instructions.
       */
      if (p->Type == PROGRAM_SAMPLER && p->Used) {
         /* Allocate a new sampler index */
         GLuint oldSampNum = (GLuint) prog->Parameters->ParameterValues[i][0];
         GLuint newSampNum = *numSamplers;
         if (newSampNum >= ctx->Const.MaxTextureImageUnits) {
            char s[100];
            _mesa_sprintf(s, "Too many texture samplers (%u, max is %u)",
                          newSampNum, ctx->Const.MaxTextureImageUnits);
            link_error(shProg, s);
            return GL_FALSE;
         }
         /* save old->new mapping in the table */
         if (oldSampNum < Elements(samplerMap))
            samplerMap[oldSampNum] = newSampNum;
         /* update parameter's sampler index */
         prog->Parameters->ParameterValues[i][0] = (GLfloat) newSampNum;
         (*numSamplers)++;
      }
   }

   /* OK, now scan the program/shader instructions looking for texture
    * instructions using sampler vars.  Replace old sampler indexes with
    * new ones.
    */
   prog->SamplersUsed = 0x0;
   for (i = 0; i < prog->NumInstructions; i++) {
      struct prog_instruction *inst = prog->Instructions + i;
      if (_mesa_is_tex_instruction(inst->Opcode)) {
         /* here, inst->TexSrcUnit is really the sampler unit */
         const GLint oldSampNum = inst->TexSrcUnit;

#if 0
         printf("====== remap sampler from %d to %d\n",
                inst->TexSrcUnit, samplerMap[ inst->TexSrcUnit ]);
#endif

         if (oldSampNum < Elements(samplerMap)) {
            const GLuint newSampNum = samplerMap[oldSampNum];
            inst->TexSrcUnit = newSampNum;
            prog->SamplerTargets[newSampNum] = inst->TexSrcTarget;
            prog->SamplersUsed |= (1 << newSampNum);
            if (inst->TexShadow) {
               prog->ShadowSamplers |= (1 << newSampNum);
            }
         }
      }
   }

   return GL_TRUE;
}


/**
 * Resolve binding of generic vertex attributes.
 * For example, if the vertex shader declared "attribute vec4 foobar" we'll
 * allocate a generic vertex attribute for "foobar" and plug that value into
 * the vertex program instructions.
 * But if the user called glBindAttributeLocation(), those bindings will
 * have priority.
 */
static GLboolean
_slang_resolve_attributes(struct gl_shader_program *shProg,
                          const struct gl_program *origProg,
                          struct gl_program *linkedProg)
{
   GLint attribMap[MAX_VERTEX_GENERIC_ATTRIBS];
   GLuint i, j;
   GLbitfield usedAttributes; /* generics only, not legacy attributes */
   GLbitfield inputsRead = 0x0;

   assert(origProg != linkedProg);
   assert(origProg->Target == GL_VERTEX_PROGRAM_ARB);
   assert(linkedProg->Target == GL_VERTEX_PROGRAM_ARB);

   if (!shProg->Attributes)
      shProg->Attributes = _mesa_new_parameter_list();

   if (linkedProg->Attributes) {
      _mesa_free_parameter_list(linkedProg->Attributes);
   }
   linkedProg->Attributes = _mesa_new_parameter_list();


   /* Build a bitmask indicating which attribute indexes have been
    * explicitly bound by the user with glBindAttributeLocation().
    */
   usedAttributes = 0x0;
   for (i = 0; i < shProg->Attributes->NumParameters; i++) {
      GLint attr = shProg->Attributes->Parameters[i].StateIndexes[0];
      usedAttributes |= (1 << attr);
   }

   /* If gl_Vertex is used, that actually counts against the limit
    * on generic vertex attributes.  This avoids the ambiguity of
    * whether glVertexAttrib4fv(0, v) sets legacy attribute 0 (vert pos)
    * or generic attribute[0].  If gl_Vertex is used, we want the former.
    */
   if (origProg->InputsRead & VERT_BIT_POS) {
      usedAttributes |= 0x1;
   }

   /* initialize the generic attribute map entries to -1 */
   for (i = 0; i < MAX_VERTEX_GENERIC_ATTRIBS; i++) {
      attribMap[i] = -1;
   }

   /*
    * Scan program for generic attribute references
    */
   for (i = 0; i < linkedProg->NumInstructions; i++) {
      struct prog_instruction *inst = linkedProg->Instructions + i;
      for (j = 0; j < 3; j++) {
         if (inst->SrcReg[j].File == PROGRAM_INPUT) {
            inputsRead |= (1 << inst->SrcReg[j].Index);
         }

         if (inst->SrcReg[j].File == PROGRAM_INPUT &&
             inst->SrcReg[j].Index >= VERT_ATTRIB_GENERIC0) {
            /*
             * OK, we've found a generic vertex attribute reference.
             */
            const GLint k = inst->SrcReg[j].Index - VERT_ATTRIB_GENERIC0;

            GLint attr = attribMap[k];

            if (attr < 0) {
               /* Need to figure out attribute mapping now.
                */
               const char *name = origProg->Attributes->Parameters[k].Name;
               const GLint size = origProg->Attributes->Parameters[k].Size;
               const GLenum type =origProg->Attributes->Parameters[k].DataType;
               GLint index;

               /* See if there's a user-defined attribute binding for
                * this name.
                */
               index = _mesa_lookup_parameter_index(shProg->Attributes,
                                                    -1, name);
               if (index >= 0) {
                  /* Found a user-defined binding */
                  attr = shProg->Attributes->Parameters[index].StateIndexes[0];
               }
               else {
                  /* No user-defined binding, choose our own attribute number.
                   * Start at 1 since generic attribute 0 always aliases
                   * glVertex/position.
                   */
                  for (attr = 0; attr < MAX_VERTEX_GENERIC_ATTRIBS; attr++) {
                     if (((1 << attr) & usedAttributes) == 0)
                        break;
                  }
                  if (attr == MAX_VERTEX_GENERIC_ATTRIBS) {
                     link_error(shProg, "Too many vertex attributes");
                     return GL_FALSE;
                  }

                  /* mark this attribute as used */
                  usedAttributes |= (1 << attr);
               }

               attribMap[k] = attr;

               /* Save the final name->attrib binding so it can be queried
                * with glGetAttributeLocation().
                */
               _mesa_add_attribute(linkedProg->Attributes, name,
                                   size, type, attr);
            }

            assert(attr >= 0);

            /* update the instruction's src reg */
            inst->SrcReg[j].Index = VERT_ATTRIB_GENERIC0 + attr;
         }
      }
   }

   /* Handle pre-defined attributes here (gl_Vertex, gl_Normal, etc).
    * When the user queries the active attributes we need to include both
    * the user-defined attributes and the built-in ones.
    */
   for (i = VERT_ATTRIB_POS; i < VERT_ATTRIB_GENERIC0; i++) {
      if (inputsRead & (1 << i)) {
         _mesa_add_attribute(linkedProg->Attributes,
                             _slang_vert_attrib_name(i),
                             4, /* size in floats */
                             _slang_vert_attrib_type(i),
                             -1 /* attrib/input */);
      }
   }

   return GL_TRUE;
}


/**
 * Scan program instructions to update the program's NumTemporaries field.
 * Note: this implemenation relies on the code generator allocating
 * temps in increasing order (0, 1, 2, ... ).
 */
static void
_slang_count_temporaries(struct gl_program *prog)
{
   GLuint i, j;
   GLint maxIndex = -1;

   for (i = 0; i < prog->NumInstructions; i++) {
      const struct prog_instruction *inst = prog->Instructions + i;
      const GLuint numSrc = _mesa_num_inst_src_regs(inst->Opcode);
      for (j = 0; j < numSrc; j++) {
         if (inst->SrcReg[j].File == PROGRAM_TEMPORARY) {
            if (maxIndex < inst->SrcReg[j].Index)
               maxIndex = inst->SrcReg[j].Index;
         }
         if (inst->DstReg.File == PROGRAM_TEMPORARY) {
            if (maxIndex < (GLint) inst->DstReg.Index)
               maxIndex = inst->DstReg.Index;
         }
      }
   }

   prog->NumTemporaries = (GLuint) (maxIndex + 1);
}


/**
 * Scan program instructions to update the program's InputsRead and
 * OutputsWritten fields.
 */
static void
_slang_update_inputs_outputs(struct gl_program *prog)
{
   GLuint i, j;
   GLuint maxAddrReg = 0;

   prog->InputsRead = 0x0;
   prog->OutputsWritten = 0x0;

   for (i = 0; i < prog->NumInstructions; i++) {
      const struct prog_instruction *inst = prog->Instructions + i;
      const GLuint numSrc = _mesa_num_inst_src_regs(inst->Opcode);
      for (j = 0; j < numSrc; j++) {
         if (inst->SrcReg[j].File == PROGRAM_INPUT) {
            prog->InputsRead |= 1 << inst->SrcReg[j].Index;
         }
         else if (inst->SrcReg[j].File == PROGRAM_ADDRESS) {
            maxAddrReg = MAX2(maxAddrReg, (GLuint) (inst->SrcReg[j].Index + 1));
         }
      }

      if (inst->DstReg.File == PROGRAM_OUTPUT) {
         prog->OutputsWritten |= BITFIELD64_BIT(inst->DstReg.Index);
         if (inst->DstReg.RelAddr) {
            /* If the output attribute is indexed with relative addressing
             * we know that it must be a varying or texcoord such as
             * gl_TexCoord[i] = v;  In this case, mark all the texcoords
             * or varying outputs as being written.  It's not an error if
             * a vertex shader writes varying vars that aren't used by the
             * fragment shader.  But it is an error for a fragment shader
             * to use varyings that are not written by the vertex shader.
             */
            if (prog->Target == GL_VERTEX_PROGRAM_ARB) {
               if (inst->DstReg.Index == VERT_RESULT_TEX0) {
                  /* mark all texcoord outputs as written */
                  const GLbitfield64 mask =
		     BITFIELD64_RANGE(VERT_RESULT_TEX0,
				      (VERT_RESULT_TEX0
				       + MAX_TEXTURE_COORD_UNITS - 1));
                  prog->OutputsWritten |= mask;
               }
               else if (inst->DstReg.Index == VERT_RESULT_VAR0) {
                  /* mark all generic varying outputs as written */
                  const GLbitfield64 mask =
		     BITFIELD64_RANGE(VERT_RESULT_VAR0,
				      (VERT_RESULT_VAR0 + MAX_VARYING - 1));
                  prog->OutputsWritten |= mask;
               }
            }
         }
      }
      else if (inst->DstReg.File == PROGRAM_ADDRESS) {
         maxAddrReg = MAX2(maxAddrReg, inst->DstReg.Index + 1);
      }
   }
   prog->NumAddressRegs = maxAddrReg;
}



/**
 * Remove extra #version directives from the concatenated source string.
 * Disable the extra ones by converting first two chars to //, a comment.
 * This is a bit of hack to work around a preprocessor bug that only
 * allows one #version directive per source.
 */
static void
remove_extra_version_directives(GLchar *source)
{
   GLuint verCount = 0;
   while (1) {
      char *ver = _mesa_strstr(source, "#version");
      if (ver) {
         verCount++;
         if (verCount > 1) {
            ver[0] = '/';
            ver[1] = '/';
         }
         source += 8;
      }
      else {
         break;
      }
   }
}



/**
 * Return a new shader whose source code is the concatenation of
 * all the shader sources of the given type.
 */
static struct gl_shader *
concat_shaders(struct gl_shader_program *shProg, GLenum shaderType)
{
   struct gl_shader *newShader;
   const struct gl_shader *firstShader = NULL;
   GLuint shaderLengths[100];
   GLchar *source;
   GLuint totalLen = 0, len = 0;
   GLuint i;

   /* compute total size of new shader source code */
   for (i = 0; i < shProg->NumShaders; i++) {
      const struct gl_shader *shader = shProg->Shaders[i];
      if (shader->Type == shaderType) {
         shaderLengths[i] = _mesa_strlen(shader->Source);
         totalLen += shaderLengths[i];
         if (!firstShader)
            firstShader = shader;
      }
   }

   if (totalLen == 0)
      return NULL;

   source = (GLchar *) _mesa_malloc(totalLen + 1);
   if (!source)
      return NULL;

   /* concatenate shaders */
   for (i = 0; i < shProg->NumShaders; i++) {
      const struct gl_shader *shader = shProg->Shaders[i];
      if (shader->Type == shaderType) {
         _mesa_memcpy(source + len, shader->Source, shaderLengths[i]);
         len += shaderLengths[i];
      }
   }
   source[len] = '\0';
   /*
   _mesa_printf("---NEW CONCATENATED SHADER---:\n%s\n------------\n", source);
   */

   remove_extra_version_directives(source);

   newShader = CALLOC_STRUCT(gl_shader);
   newShader->Type = shaderType;
   newShader->Source = source;
   newShader->Pragmas = firstShader->Pragmas;

   return newShader;
}


/**
 * Search the shader program's list of shaders to find the one that
 * defines main().
 * This will involve shader concatenation and recompilation if needed.
 */
static struct gl_shader *
get_main_shader(GLcontext *ctx,
                struct gl_shader_program *shProg, GLenum type)
{
   struct gl_shader *shader = NULL;
   GLuint i;

   /*
    * Look for a shader that defines main() and has no unresolved references.
    */
   for (i = 0; i < shProg->NumShaders; i++) {
      shader = shProg->Shaders[i];
      if (shader->Type == type &&
          shader->Main &&
          !shader->UnresolvedRefs) {
         /* All set! */
         return shader;
      }
   }

   /*
    * There must have been unresolved references during the original
    * compilation.  Try concatenating all the shaders of the given type
    * and recompile that.
    */
   shader = concat_shaders(shProg, type);

   if (shader) {
      _slang_compile(ctx, shader);

      /* Finally, check if recompiling failed */
      if (!shader->CompileStatus ||
          !shader->Main ||
          shader->UnresolvedRefs) {
         link_error(shProg, "Unresolved symbols");
         _mesa_free_shader(ctx, shader);
         return NULL;
      }
   }

   return shader;
}


/**
 * Shader linker.  Currently:
 *
 * 1. The last attached vertex shader and fragment shader are linked.
 * 2. Varying vars in the two shaders are combined so their locations
 *    agree between the vertex and fragment stages.  They're treated as
 *    vertex program output attribs and as fragment program input attribs.
 * 3. The vertex and fragment programs are cloned and modified to update
 *    src/dst register references so they use the new, linked varying
 *    storage locations.
 */
void
_slang_link(GLcontext *ctx,
            GLhandleARB programObj,
            struct gl_shader_program *shProg)
{
   const struct gl_vertex_program *vertProg = NULL;
   const struct gl_fragment_program *fragProg = NULL;
   GLuint numSamplers = 0;
   GLuint i;

   _mesa_clear_shader_program_data(ctx, shProg);

   /* Initialize LinkStatus to "success".  Will be cleared if error. */
   shProg->LinkStatus = GL_TRUE;

   /* check that all programs compiled successfully */
   for (i = 0; i < shProg->NumShaders; i++) {
      if (!shProg->Shaders[i]->CompileStatus) {
         link_error(shProg, "linking with uncompiled shader\n");
         return;
      }
   }

   shProg->Uniforms = _mesa_new_uniform_list();
   shProg->Varying = _mesa_new_parameter_list();

   /*
    * Find the vertex and fragment shaders which define main()
    */
   {
      struct gl_shader *vertShader, *fragShader;
      vertShader = get_main_shader(ctx, shProg, GL_VERTEX_SHADER);
      fragShader = get_main_shader(ctx, shProg, GL_FRAGMENT_SHADER);
      if (vertShader)
         vertProg = vertex_program(vertShader->Program);
      if (fragShader)
         fragProg = fragment_program(fragShader->Program);
      if (!shProg->LinkStatus)
         return;
   }

#if FEATURE_es2_glsl
   /* must have both a vertex and fragment program for ES2 */
   if (!vertProg) {
      link_error(shProg, "missing vertex shader\n");
      return;
   }
   if (!fragProg) {
      link_error(shProg, "missing fragment shader\n");
      return;
   }
#endif

   /*
    * Make copies of the vertex/fragment programs now since we'll be
    * changing src/dst registers after merging the uniforms and varying vars.
    */
   _mesa_reference_vertprog(ctx, &shProg->VertexProgram, NULL);
   if (vertProg) {
      struct gl_vertex_program *linked_vprog =
         vertex_program(_mesa_clone_program(ctx, &vertProg->Base));
      shProg->VertexProgram = linked_vprog; /* refcount OK */
      /* vertex program ID not significant; just set Id for debugging purposes */
      shProg->VertexProgram->Base.Id = shProg->Name;
      ASSERT(shProg->VertexProgram->Base.RefCount == 1);
   }

   _mesa_reference_fragprog(ctx, &shProg->FragmentProgram, NULL);
   if (fragProg) {
      struct gl_fragment_program *linked_fprog = 
         fragment_program(_mesa_clone_program(ctx, &fragProg->Base));
      shProg->FragmentProgram = linked_fprog; /* refcount OK */
      /* vertex program ID not significant; just set Id for debugging purposes */
      shProg->FragmentProgram->Base.Id = shProg->Name;
      ASSERT(shProg->FragmentProgram->Base.RefCount == 1);
   }

   /* link varying vars */
   if (shProg->VertexProgram) {
      if (!link_varying_vars(ctx, shProg, &shProg->VertexProgram->Base))
         return;
   }
   if (shProg->FragmentProgram) {
      if (!link_varying_vars(ctx, shProg, &shProg->FragmentProgram->Base))
         return;
   }

   /* link uniform vars */
   if (shProg->VertexProgram) {
      if (!link_uniform_vars(ctx, shProg, &shProg->VertexProgram->Base,
                             &numSamplers)) {
         return;
      }
   }
   if (shProg->FragmentProgram) {
      if (!link_uniform_vars(ctx, shProg, &shProg->FragmentProgram->Base,
                             &numSamplers)) {
         return;
      }
   }

   /*_mesa_print_uniforms(shProg->Uniforms);*/

   if (shProg->VertexProgram) {
      if (!_slang_resolve_attributes(shProg, &vertProg->Base,
                                     &shProg->VertexProgram->Base)) {
         return;
      }
   }

   if (shProg->VertexProgram) {
      _slang_update_inputs_outputs(&shProg->VertexProgram->Base);
      _slang_count_temporaries(&shProg->VertexProgram->Base);
      if (!(shProg->VertexProgram->Base.OutputsWritten
	    & BITFIELD64_BIT(VERT_RESULT_HPOS))) {
         /* the vertex program did not compute a vertex position */
         link_error(shProg,
                    "gl_Position was not written by vertex shader\n");
         return;
      }
   }
   if (shProg->FragmentProgram) {
      _slang_count_temporaries(&shProg->FragmentProgram->Base);
      _slang_update_inputs_outputs(&shProg->FragmentProgram->Base);
   }

   /* Check that all the varying vars needed by the fragment shader are
    * actually produced by the vertex shader.
    */
   if (shProg->FragmentProgram) {
      const GLbitfield varyingRead
         = shProg->FragmentProgram->Base.InputsRead >> FRAG_ATTRIB_VAR0;
      const GLbitfield64 varyingWritten = shProg->VertexProgram ?
         shProg->VertexProgram->Base.OutputsWritten >> VERT_RESULT_VAR0 : 0x0;
      if ((varyingRead & varyingWritten) != varyingRead) {
         link_error(shProg,
          "Fragment program using varying vars not written by vertex shader\n");
         return;
      }         
   }

   /* check that gl_FragColor and gl_FragData are not both written to */
   if (shProg->FragmentProgram) {
      const GLbitfield64 outputsWritten =
	 shProg->FragmentProgram->Base.OutputsWritten;
      if ((outputsWritten & BITFIELD64_BIT(FRAG_RESULT_COLOR)) &&
          (outputsWritten >= BITFIELD64_BIT(FRAG_RESULT_DATA0))) {
         link_error(shProg, "Fragment program cannot write both gl_FragColor"
                    " and gl_FragData[].\n");
         return;
      }         
   }


   if (fragProg && shProg->FragmentProgram) {
      /* Compute initial program's TexturesUsed info */
      _mesa_update_shader_textures_used(&shProg->FragmentProgram->Base);

      /* notify driver that a new fragment program has been compiled/linked */
      ctx->Driver.ProgramStringNotify(ctx, GL_FRAGMENT_PROGRAM_ARB,
                                      &shProg->FragmentProgram->Base);
      if (ctx->Shader.Flags & GLSL_DUMP) {
         _mesa_printf("Mesa pre-link fragment program:\n");
         _mesa_print_program(&fragProg->Base);
         _mesa_print_program_parameters(ctx, &fragProg->Base);

         _mesa_printf("Mesa post-link fragment program:\n");
         _mesa_print_program(&shProg->FragmentProgram->Base);
         _mesa_print_program_parameters(ctx, &shProg->FragmentProgram->Base);
      }
   }

   if (vertProg && shProg->VertexProgram) {
      /* Compute initial program's TexturesUsed info */
      _mesa_update_shader_textures_used(&shProg->VertexProgram->Base);

      /* notify driver that a new vertex program has been compiled/linked */
      ctx->Driver.ProgramStringNotify(ctx, GL_VERTEX_PROGRAM_ARB,
                                      &shProg->VertexProgram->Base);
      if (ctx->Shader.Flags & GLSL_DUMP) {
         _mesa_printf("Mesa pre-link vertex program:\n");
         _mesa_print_program(&vertProg->Base);
         _mesa_print_program_parameters(ctx, &vertProg->Base);

         _mesa_printf("Mesa post-link vertex program:\n");
         _mesa_print_program(&shProg->VertexProgram->Base);
         _mesa_print_program_parameters(ctx, &shProg->VertexProgram->Base);
      }
   }

   /* Debug: */
   if (0) {
      if (shProg->VertexProgram)
         _mesa_postprocess_program(ctx, &shProg->VertexProgram->Base);
      if (shProg->FragmentProgram)
         _mesa_postprocess_program(ctx, &shProg->FragmentProgram->Base);
   }

   if (ctx->Shader.Flags & GLSL_DUMP) {
      _mesa_printf("Varying vars:\n");
      _mesa_print_parameter_list(shProg->Varying);
      if (shProg->InfoLog) {
         _mesa_printf("Info Log: %s\n", shProg->InfoLog);
      }
   }

   shProg->LinkStatus = (shProg->VertexProgram || shProg->FragmentProgram);
}

