#! /usr/bin/env python
#
# Copyright (C) 2014 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Jason Ekstrand (jason@jlekstrand.net)

import nir_algebraic

# Convenience variables
a = 'a'
b = 'b'
c = 'c'
d = 'd'

# Written in the form (<search>, <replace>) where <search> is an expression
# and <replace> is either an expression or a value.  An expression is
# defined as a tuple of the form (<op>, <src0>, <src1>, <src2>, <src3>)
# where each source is either an expression or a value.  A value can be
# either a numeric constant or a string representing a variable name.
#
# Variable names are specified as "[#]name[@type]" where "#" inicates that
# the given variable will only match constants and the type indicates that
# the given variable will only match values from ALU instructions with the
# given output type.
#
# For constants, you have to be careful to make sure that it is the right
# type because python is unaware of the source and destination types of the
# opcodes.

optimizations = [
   (('fneg', ('fneg', a)), a),
   (('ineg', ('ineg', a)), a),
   (('fabs', ('fabs', a)), ('fabs', a)),
   (('fabs', ('fneg', a)), ('fabs', a)),
   (('iabs', ('iabs', a)), ('iabs', a)),
   (('iabs', ('ineg', a)), ('iabs', a)),
   (('fadd', a, 0.0), a),
   (('iadd', a, 0), a),
   (('fadd', ('fmul', a, b), ('fmul', a, c)), ('fmul', a, ('fadd', b, c))),
   (('iadd', ('imul', a, b), ('imul', a, c)), ('imul', a, ('iadd', b, c))),
   (('fadd', ('fneg', a), a), 0.0),
   (('iadd', ('ineg', a), a), 0),
   (('fmul', a, 0.0), 0.0),
   (('imul', a, 0), 0),
   (('fmul', a, 1.0), a),
   (('imul', a, 1), a),
   (('fmul', a, -1.0), ('fneg', a)),
   (('imul', a, -1), ('ineg', a)),
   (('ffma', 0.0, a, b), b),
   (('ffma', a, 0.0, b), b),
   (('ffma', a, b, 0.0), ('fmul', a, b)),
   (('ffma', a, 1.0, b), ('fadd', a, b)),
   (('ffma', 1.0, a, b), ('fadd', a, b)),
   (('flrp', a, b, 0.0), a),
   (('flrp', a, b, 1.0), b),
   (('flrp', a, a, b), a),
   (('flrp', 0.0, a, b), ('fmul', a, b)),
   (('flrp', a, b, c), ('fadd', ('fmul', c, ('fsub', b, a)), a), 'options->lower_flrp'),
   (('fadd', ('fmul', a, ('fadd', 1.0, ('fneg', c))), ('fmul', b, c)), ('flrp', a, b, c), '!options->lower_flrp'),
   (('fadd', a, ('fmul', c, ('fadd', b, ('fneg', a)))), ('flrp', a, b, c), '!options->lower_flrp'),
   (('ffma', a, b, c), ('fadd', ('fmul', a, b), c), 'options->lower_ffma'),
   (('fadd', ('fmul', a, b), c), ('ffma', a, b, c), '!options->lower_ffma'),
   # Comparison simplifications
   (('inot', ('flt', a, b)), ('fge', a, b)),
   (('inot', ('fge', a, b)), ('flt', a, b)),
   (('inot', ('feq', a, b)), ('fne', a, b)),
   (('inot', ('fne', a, b)), ('feq', a, b)),
   (('inot', ('ilt', a, b)), ('ige', a, b)),
   (('inot', ('ige', a, b)), ('ilt', a, b)),
   (('inot', ('ieq', a, b)), ('ine', a, b)),
   (('inot', ('ine', a, b)), ('ieq', a, b)),
   (('fge', ('fneg', ('fabs', a)), 0.0), ('feq', a, 0.0)),
   (('bcsel', ('flt', a, b), a, b), ('fmin', a, b)),
   (('bcsel', ('flt', a, b), b, a), ('fmax', a, b)),
   (('bcsel', ('inot', 'a@bool'), b, c), ('bcsel', a, c, b)),
   (('bcsel', a, ('bcsel', a, b, c), d), ('bcsel', a, b, d)),
   (('fmin', a, a), a),
   (('fmax', a, a), a),
   (('imin', a, a), a),
   (('imax', a, a), a),
   (('umin', a, a), a),
   (('umax', a, a), a),
   (('fmin', ('fmax', a, 0.0), 1.0), ('fsat', a), '!options->lower_fsat'),
   (('fsat', a), ('fmin', ('fmax', a, 0.0), 1.0), 'options->lower_fsat'),
   (('fsat', ('fsat', a)), ('fsat', a)),
   (('fmin', ('fmax', ('fmin', ('fmax', a, 0.0), 1.0), 0.0), 1.0), ('fmin', ('fmax', a, 0.0), 1.0)),
   (('ior', ('flt', a, b), ('flt', a, c)), ('flt', a, ('fmax', b, c))),
   (('ior', ('flt', a, c), ('flt', b, c)), ('flt', ('fmin', a, b), c)),
   (('ior', ('fge', a, b), ('fge', a, c)), ('fge', a, ('fmin', b, c))),
   (('ior', ('fge', a, c), ('fge', b, c)), ('fge', ('fmax', a, b), c)),
   (('slt', a, b), ('b2f', ('flt', a, b)), 'options->lower_scmp'),
   (('sge', a, b), ('b2f', ('fge', a, b)), 'options->lower_scmp'),
   (('seq', a, b), ('b2f', ('feq', a, b)), 'options->lower_scmp'),
   (('sne', a, b), ('b2f', ('fne', a, b)), 'options->lower_scmp'),
   # Emulating booleans
   (('imul', ('b2i', a), ('b2i', b)), ('b2i', ('iand', a, b))),
   (('fmul', ('b2f', a), ('b2f', b)), ('b2f', ('iand', a, b))),
   (('fsat', ('fadd', ('b2f', a), ('b2f', b))), ('b2f', ('ior', a, b))),
   (('iand', 'a@bool', 1.0), ('b2f', a)),
   (('flt', ('fneg', ('b2f', a)), 0), a), # Generated by TGSI KILL_IF.
   (('flt', ('fsub', 0.0, ('b2f', a)), 0), a), # Generated by TGSI KILL_IF.
   # Comparison with the same args.  Note that these are not done for
   # the float versions because NaN always returns false on float
   # inequalities.
   (('ilt', a, a), False),
   (('ige', a, a), True),
   (('ieq', a, a), True),
   (('ine', a, a), False),
   (('ult', a, a), False),
   (('uge', a, a), True),
   # Logical and bit operations
   (('fand', a, 0.0), 0.0),
   (('iand', a, a), a),
   (('iand', a, 0), 0),
   (('ior', a, a), a),
   (('ior', a, 0), a),
   (('fxor', a, a), 0.0),
   (('ixor', a, a), 0),
   (('inot', ('inot', a)), a),
   # DeMorgan's Laws
   (('iand', ('inot', a), ('inot', b)), ('inot', ('ior',  a, b))),
   (('ior',  ('inot', a), ('inot', b)), ('inot', ('iand', a, b))),
   # Shift optimizations
   (('ishl', 0, a), 0),
   (('ishl', a, 0), a),
   (('ishr', 0, a), 0),
   (('ishr', a, 0), a),
   (('ushr', 0, a), 0),
   (('ushr', a, 0), a),
   # Exponential/logarithmic identities
   (('fexp2', ('flog2', a)), a), # 2^lg2(a) = a
   (('flog2', ('fexp2', a)), a), # lg2(2^a) = a
   (('fpow', a, b), ('fexp2', ('fmul', ('flog2', a), b)), 'options->lower_fpow'), # a^b = 2^(lg2(a)*b)
   (('fexp2', ('fmul', ('flog2', a), b)), ('fpow', a, b), '!options->lower_fpow'), # 2^(lg2(a)*b) = a^b
   (('fpow', a, 1.0), a),
   (('fpow', a, 2.0), ('fmul', a, a)),
   (('fpow', a, 4.0), ('fmul', ('fmul', a, a), ('fmul', a, a))),
   (('fpow', 2.0, a), ('fexp2', a)),
   (('fsqrt', ('fexp2', a)), ('fexp2', ('fmul', 0.5, a))),
   (('frcp', ('fexp2', a)), ('fexp2', ('fneg', a))),
   (('frsq', ('fexp2', a)), ('fexp2', ('fmul', -0.5, a))),
   (('flog2', ('fsqrt', a)), ('fmul', 0.5, ('flog2', a))),
   (('flog2', ('frcp', a)), ('fneg', ('flog2', a))),
   (('flog2', ('frsq', a)), ('fmul', -0.5, ('flog2', a))),
   (('flog2', ('fpow', a, b)), ('fmul', b, ('flog2', a))),
   (('fadd', ('flog2', a), ('flog2', b)), ('flog2', ('fmul', a, b))),
   (('fadd', ('flog2', a), ('fneg', ('flog2', b))), ('flog2', ('fdiv', a, b))),
   (('fmul', ('fexp2', a), ('fexp2', b)), ('fexp2', ('fadd', a, b))),
   # Division and reciprocal
   (('fdiv', 1.0, a), ('frcp', a)),
   (('frcp', ('frcp', a)), a),
   (('frcp', ('fsqrt', a)), ('frsq', a)),
   (('fsqrt', a), ('frcp', ('frsq', a)), 'options->lower_fsqrt'),
   (('frcp', ('frsq', a)), ('fsqrt', a), '!options->lower_fsqrt'),
   # Boolean simplifications
   (('ine', 'a@bool', 0), 'a'),
   (('ieq', 'a@bool', 0), ('inot', 'a')),
   (('bcsel', a, True, False), ('ine', a, 0)),
   (('bcsel', a, False, True), ('ieq', a, 0)),
   (('bcsel', True, b, c), b),
   (('bcsel', False, b, c), c),
   # The result of this should be hit by constant propagation and, in the
   # next round of opt_algebraic, get picked up by one of the above two.
   (('bcsel', '#a', b, c), ('bcsel', ('ine', 'a', 0), b, c)),

   (('bcsel', a, b, b), b),
   (('fcsel', a, b, b), b),

   # Conversions
   (('i2b', ('b2i', a)), a),
   (('f2i', ('ftrunc', a)), ('f2i', a)),
   (('f2u', ('ftrunc', a)), ('f2u', a)),

   # Subtracts
   (('fsub', a, ('fsub', 0.0, b)), ('fadd', a, b)),
   (('isub', a, ('isub', 0, b)), ('iadd', a, b)),
   (('fsub', a, b), ('fadd', a, ('fneg', b)), 'options->lower_sub'),
   (('isub', a, b), ('iadd', a, ('ineg', b)), 'options->lower_sub'),
   (('fneg', a), ('fsub', 0.0, a), 'options->lower_negate'),
   (('ineg', a), ('isub', 0, a), 'options->lower_negate'),
   (('fadd', a, ('fsub', 0.0, b)), ('fsub', a, b)),
   (('iadd', a, ('isub', 0, b)), ('isub', a, b)),
   (('fabs', ('fsub', 0.0, a)), ('fabs', a)),
   (('iabs', ('isub', 0, a)), ('iabs', a)),
]

# Add optimizations to handle the case where the result of a ternary is
# compared to a constant.  This way we can take things like
#
# (a ? 0 : 1) > 0
#
# and turn it into
#
# a ? (0 > 0) : (1 > 0)
#
# which constant folding will eat for lunch.  The resulting ternary will
# further get cleaned up by the boolean reductions above and we will be
# left with just the original variable "a".
for op in ['flt', 'fge', 'feq', 'fne',
           'ilt', 'ige', 'ieq', 'ine', 'ult', 'uge']:
   optimizations += [
      ((op, ('bcsel', 'a', '#b', '#c'), '#d'),
       ('bcsel', 'a', (op, 'b', 'd'), (op, 'c', 'd'))),
      ((op, '#d', ('bcsel', a, '#b', '#c')),
       ('bcsel', 'a', (op, 'd', 'b'), (op, 'd', 'c'))),
   ]

# This section contains "late" optimizations that should be run after the
# regular optimizations have finished.  Optimizations should go here if
# they help code generation but do not necessarily produce code that is
# more easily optimizable.
late_optimizations = [
   (('flt', ('fadd', a, b), 0.0), ('flt', a, ('fneg', b))),
   (('fge', ('fadd', a, b), 0.0), ('fge', a, ('fneg', b))),
   (('feq', ('fadd', a, b), 0.0), ('feq', a, ('fneg', b))),
   (('fne', ('fadd', a, b), 0.0), ('fne', a, ('fneg', b))),
]

print nir_algebraic.AlgebraicPass("nir_opt_algebraic", optimizations).render()
print nir_algebraic.AlgebraicPass("nir_opt_algebraic_late",
                                  late_optimizations).render()
