/*
 * Copyright © 2010 Red Hat, Inc.
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
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include "dix-config.h"
#endif

#ifndef INPUTUTILS_H
#define INPUTUTILS_H

#include "input.h"

struct _ValuatorMask {
    int8_t      last_bit; /* highest bit set in mask */
    uint8_t     mask[(MAX_VALUATORS + 7)/8];
    int         valuators[MAX_VALUATORS]; /* valuator data */
};

/* server-internal */
extern _X_HIDDEN int valuator_mask_size(const ValuatorMask *mask);
extern _X_HIDDEN int valuator_mask_isset(const ValuatorMask *mask, int bit);
extern _X_HIDDEN void valuator_mask_unset(ValuatorMask *mask, int bit);
extern _X_HIDDEN int valuator_mask_num_valuators(const ValuatorMask *mask);
extern _X_HIDDEN void valuator_mask_copy(ValuatorMask *dest, const ValuatorMask *src);
extern _X_HIDDEN int valuator_mask_get(const ValuatorMask *mask, int valnum);

#endif
