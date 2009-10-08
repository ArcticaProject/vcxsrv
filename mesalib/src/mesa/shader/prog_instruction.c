/*
 * Mesa 3-D graphics library
 * Version:  7.3
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 1999-2009  VMware, Inc.  All Rights Reserved.
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


#include "main/glheader.h"
#include "main/imports.h"
#include "main/mtypes.h"
#include "prog_instruction.h"


/**
 * Initialize program instruction fields to defaults.
 * \param inst  first instruction to initialize
 * \param count  number of instructions to initialize
 */
void
_mesa_init_instructions(struct prog_instruction *inst, GLuint count)
{
   GLuint i;

   _mesa_bzero(inst, count * sizeof(struct prog_instruction));

   for (i = 0; i < count; i++) {
      inst[i].SrcReg[0].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[0].Swizzle = SWIZZLE_NOOP;
      inst[i].SrcReg[1].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[1].Swizzle = SWIZZLE_NOOP;
      inst[i].SrcReg[2].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[2].Swizzle = SWIZZLE_NOOP;

      inst[i].DstReg.File = PROGRAM_UNDEFINED;
      inst[i].DstReg.WriteMask = WRITEMASK_XYZW;
      inst[i].DstReg.CondMask = COND_TR;
      inst[i].DstReg.CondSwizzle = SWIZZLE_NOOP;

      inst[i].SaturateMode = SATURATE_OFF;
      inst[i].Precision = FLOAT32;
   }
}


/**
 * Allocate an array of program instructions.
 * \param numInst  number of instructions
 * \return pointer to instruction memory
 */
struct prog_instruction *
_mesa_alloc_instructions(GLuint numInst)
{
   return (struct prog_instruction *)
      _mesa_calloc(numInst * sizeof(struct prog_instruction));
}


/**
 * Reallocate memory storing an array of program instructions.
 * This is used when we need to append additional instructions onto an
 * program.
 * \param oldInst  pointer to first of old/src instructions
 * \param numOldInst  number of instructions at <oldInst>
 * \param numNewInst  desired size of new instruction array.
 * \return  pointer to start of new instruction array.
 */
struct prog_instruction *
_mesa_realloc_instructions(struct prog_instruction *oldInst,
                           GLuint numOldInst, GLuint numNewInst)
{
   struct prog_instruction *newInst;

   newInst = (struct prog_instruction *)
      _mesa_realloc(oldInst,
                    numOldInst * sizeof(struct prog_instruction),
                    numNewInst * sizeof(struct prog_instruction));

   return newInst;
}


/**
 * Copy an array of program instructions.
 * \param dest  pointer to destination.
 * \param src  pointer to source.
 * \param n  number of instructions to copy.
 * \return pointer to destination.
 */
struct prog_instruction *
_mesa_copy_instructions(struct prog_instruction *dest,
                        const struct prog_instruction *src, GLuint n)
{
   GLuint i;
   _mesa_memcpy(dest, src, n * sizeof(struct prog_instruction));
   for (i = 0; i < n; i++) {
      if (src[i].Comment)
         dest[i].Comment = _mesa_strdup(src[i].Comment);
   }
   return dest;
}


/**
 * Free an array of instructions
 */
void
_mesa_free_instructions(struct prog_instruction *inst, GLuint count)
{
   GLuint i;
   for (i = 0; i < count; i++) {
      if (inst[i].Data)
         _mesa_free(inst[i].Data);
      if (inst[i].Comment)
         _mesa_free((char *) inst[i].Comment);
   }
   _mesa_free(inst);
}


/**
 * Basic info about each instruction
 */
struct instruction_info
{
   gl_inst_opcode Opcode;
   const char *Name;
   GLuint NumSrcRegs;
   GLuint NumDstRegs;
};

/**
 * Instruction info
 * \note Opcode should equal array index!
 */
static const struct instruction_info InstInfo[MAX_OPCODE] = {
   { OPCODE_NOP,    "NOP",     0, 0 },
   { OPCODE_ABS,    "ABS",     1, 1 },
   { OPCODE_ADD,    "ADD",     2, 1 },
   { OPCODE_AND,    "AND",     2, 1 },
   { OPCODE_ARA,    "ARA",     1, 1 },
   { OPCODE_ARL,    "ARL",     1, 1 },
   { OPCODE_ARL_NV, "ARL_NV",  1, 1 },
   { OPCODE_ARR,    "ARL",     1, 1 },
   { OPCODE_BGNLOOP,"BGNLOOP", 0, 0 },
   { OPCODE_BGNSUB, "BGNSUB",  0, 0 },
   { OPCODE_BRA,    "BRA",     0, 0 },
   { OPCODE_BRK,    "BRK",     0, 0 },
   { OPCODE_CAL,    "CAL",     0, 0 },
   { OPCODE_CMP,    "CMP",     3, 1 },
   { OPCODE_CONT,   "CONT",    0, 0 },
   { OPCODE_COS,    "COS",     1, 1 },
   { OPCODE_DDX,    "DDX",     1, 1 },
   { OPCODE_DDY,    "DDY",     1, 1 },
   { OPCODE_DP2,    "DP2",     2, 1 },
   { OPCODE_DP2A,   "DP2A",    3, 1 },
   { OPCODE_DP3,    "DP3",     2, 1 },
   { OPCODE_DP4,    "DP4",     2, 1 },
   { OPCODE_DPH,    "DPH",     2, 1 },
   { OPCODE_DST,    "DST",     2, 1 },
   { OPCODE_ELSE,   "ELSE",    0, 0 },
   { OPCODE_END,    "END",     0, 0 },
   { OPCODE_ENDIF,  "ENDIF",   0, 0 },
   { OPCODE_ENDLOOP,"ENDLOOP", 0, 0 },
   { OPCODE_ENDSUB, "ENDSUB",  0, 0 },
   { OPCODE_EX2,    "EX2",     1, 1 },
   { OPCODE_EXP,    "EXP",     1, 1 },
   { OPCODE_FLR,    "FLR",     1, 1 },
   { OPCODE_FRC,    "FRC",     1, 1 },
   { OPCODE_IF,     "IF",      1, 0 },
   { OPCODE_KIL,    "KIL",     1, 0 },
   { OPCODE_KIL_NV, "KIL_NV",  0, 0 },
   { OPCODE_LG2,    "LG2",     1, 1 },
   { OPCODE_LIT,    "LIT",     1, 1 },
   { OPCODE_LOG,    "LOG",     1, 1 },
   { OPCODE_LRP,    "LRP",     3, 1 },
   { OPCODE_MAD,    "MAD",     3, 1 },
   { OPCODE_MAX,    "MAX",     2, 1 },
   { OPCODE_MIN,    "MIN",     2, 1 },
   { OPCODE_MOV,    "MOV",     1, 1 },
   { OPCODE_MUL,    "MUL",     2, 1 },
   { OPCODE_NOISE1, "NOISE1",  1, 1 },
   { OPCODE_NOISE2, "NOISE2",  1, 1 },
   { OPCODE_NOISE3, "NOISE3",  1, 1 },
   { OPCODE_NOISE4, "NOISE4",  1, 1 },
   { OPCODE_NOT,    "NOT",     1, 1 },
   { OPCODE_NRM3,   "NRM3",    1, 1 },
   { OPCODE_NRM4,   "NRM4",    1, 1 },
   { OPCODE_OR,     "OR",      2, 1 },
   { OPCODE_PK2H,   "PK2H",    1, 1 },
   { OPCODE_PK2US,  "PK2US",   1, 1 },
   { OPCODE_PK4B,   "PK4B",    1, 1 },
   { OPCODE_PK4UB,  "PK4UB",   1, 1 },
   { OPCODE_POW,    "POW",     2, 1 },
   { OPCODE_POPA,   "POPA",    0, 0 },
   { OPCODE_PRINT,  "PRINT",   1, 0 },
   { OPCODE_PUSHA,  "PUSHA",   0, 0 },
   { OPCODE_RCC,    "RCC",     1, 1 },
   { OPCODE_RCP,    "RCP",     1, 1 },
   { OPCODE_RET,    "RET",     0, 0 },
   { OPCODE_RFL,    "RFL",     1, 1 },
   { OPCODE_RSQ,    "RSQ",     1, 1 },
   { OPCODE_SCS,    "SCS",     1, 1 },
   { OPCODE_SEQ,    "SEQ",     2, 1 },
   { OPCODE_SFL,    "SFL",     0, 1 },
   { OPCODE_SGE,    "SGE",     2, 1 },
   { OPCODE_SGT,    "SGT",     2, 1 },
   { OPCODE_SIN,    "SIN",     1, 1 },
   { OPCODE_SLE,    "SLE",     2, 1 },
   { OPCODE_SLT,    "SLT",     2, 1 },
   { OPCODE_SNE,    "SNE",     2, 1 },
   { OPCODE_SSG,    "SSG",     1, 1 },
   { OPCODE_STR,    "STR",     0, 1 },
   { OPCODE_SUB,    "SUB",     2, 1 },
   { OPCODE_SWZ,    "SWZ",     1, 1 },
   { OPCODE_TEX,    "TEX",     1, 1 },
   { OPCODE_TXB,    "TXB",     1, 1 },
   { OPCODE_TXD,    "TXD",     3, 1 },
   { OPCODE_TXL,    "TXL",     1, 1 },
   { OPCODE_TXP,    "TXP",     1, 1 },
   { OPCODE_TXP_NV, "TXP_NV",  1, 1 },
   { OPCODE_TRUNC,  "TRUNC",   1, 1 },
   { OPCODE_UP2H,   "UP2H",    1, 1 },
   { OPCODE_UP2US,  "UP2US",   1, 1 },
   { OPCODE_UP4B,   "UP4B",    1, 1 },
   { OPCODE_UP4UB,  "UP4UB",   1, 1 },
   { OPCODE_X2D,    "X2D",     3, 1 },
   { OPCODE_XOR,    "XOR",     2, 1 },
   { OPCODE_XPD,    "XPD",     2, 1 }
};


/**
 * Return the number of src registers for the given instruction/opcode.
 */
GLuint
_mesa_num_inst_src_regs(gl_inst_opcode opcode)
{
   ASSERT(opcode < MAX_OPCODE);
   ASSERT(opcode == InstInfo[opcode].Opcode);
   ASSERT(OPCODE_XPD == InstInfo[OPCODE_XPD].Opcode);
   return InstInfo[opcode].NumSrcRegs;
}


/**
 * Return the number of dst registers for the given instruction/opcode.
 */
GLuint
_mesa_num_inst_dst_regs(gl_inst_opcode opcode)
{
   ASSERT(opcode < MAX_OPCODE);
   ASSERT(opcode == InstInfo[opcode].Opcode);
   ASSERT(OPCODE_XPD == InstInfo[OPCODE_XPD].Opcode);
   return InstInfo[opcode].NumDstRegs;
}


GLboolean
_mesa_is_tex_instruction(gl_inst_opcode opcode)
{
   return (opcode == OPCODE_TEX ||
           opcode == OPCODE_TXB ||
           opcode == OPCODE_TXD ||
           opcode == OPCODE_TXL ||
           opcode == OPCODE_TXP);
}


/**
 * Check if there's a potential src/dst register data dependency when
 * using SOA execution.
 * Example:
 *   MOV T, T.yxwz;
 * This would expand into:
 *   MOV t0, t1;
 *   MOV t1, t0;
 *   MOV t2, t3;
 *   MOV t3, t2;
 * The second instruction will have the wrong value for t0 if executed as-is.
 */
GLboolean
_mesa_check_soa_dependencies(const struct prog_instruction *inst)
{
   GLuint i, chan;

   if (inst->DstReg.WriteMask == WRITEMASK_X ||
       inst->DstReg.WriteMask == WRITEMASK_Y ||
       inst->DstReg.WriteMask == WRITEMASK_Z ||
       inst->DstReg.WriteMask == WRITEMASK_W ||
       inst->DstReg.WriteMask == 0x0) {
      /* no chance of data dependency */
      return GL_FALSE;
   }

   /* loop over src regs */
   for (i = 0; i < 3; i++) {
      if (inst->SrcReg[i].File == inst->DstReg.File &&
          inst->SrcReg[i].Index == inst->DstReg.Index) {
         /* loop over dest channels */
         GLuint channelsWritten = 0x0;
         for (chan = 0; chan < 4; chan++) {
            if (inst->DstReg.WriteMask & (1 << chan)) {
               /* check if we're reading a channel that's been written */
               GLuint swizzle = GET_SWZ(inst->SrcReg[i].Swizzle, chan);
               if (swizzle <= SWIZZLE_W &&
                   (channelsWritten & (1 << swizzle))) {
                  return GL_TRUE;
               }

               channelsWritten |= (1 << chan);
            }
         }
      }
   }
   return GL_FALSE;
}


/**
 * Return string name for given program opcode.
 */
const char *
_mesa_opcode_string(gl_inst_opcode opcode)
{
   if (opcode < MAX_OPCODE)
      return InstInfo[opcode].Name;
   else {
      static char s[20];
      _mesa_snprintf(s, sizeof(s), "OP%u", opcode);
      return s;
   }
}

