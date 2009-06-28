/*
 * This file is an incredible crock to get the normally-inline functions
 * built into the server so that things can be debugged properly.
 *
 * Note: this doesn't work when using a compiler other than GCC.
 */

#define static /**/
#define __inline__ /**/
#undef NO_INLINE
#define DO_PROTOTYPES
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "compiler.h"
