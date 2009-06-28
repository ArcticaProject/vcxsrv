/*
 * Copyright 2001,2005 by Kean Johnston <jkj@sco.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Kean Johnston not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Kean Johnston makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * KEAN JOHNSTON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#include "X.h"

#include "compiler.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86OSpriv.h"
#include "xf86_OSlib.h"


/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

static Bool IOEnabled = FALSE;

_X_EXPORT Bool
xf86EnableIO(void)
{
  if (IOEnabled)
    return TRUE;

  if (sysi86(SI86IOPL, 3) < 0)
    FatalError("Failed to set IOPL for extended I/O\n");
  IOEnabled = TRUE;
  return TRUE;
}

_X_EXPORT void
xf86DisableIO(void)
{
  if (!IOEnabled)
    return;

  sysi86(SI86IOPL, 0);
  IOEnabled = FALSE;
}

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

_X_EXPORT Bool
xf86DisableInterrupts(void)
{
  if (!IOEnabled) {
    if (sysi86(SI86IOPL, 3) < 0)
      return FALSE;
  }

#ifdef __GNUC__
  __asm__ __volatile__("cli");
#else 
  asm("cli");
#endif /* __GNUC__ */

  if (!IOEnabled) {
    sysi86(SI86IOPL, 0);
  }

  return(TRUE);
}

_X_EXPORT void
xf86EnableInterrupts(void)
{
  if (!IOEnabled) {
    if (sysi86(SI86IOPL, 3) < 0)
      return;
  }

#ifdef __GNUC__
  __asm__ __volatile__("sti");
#else 
  asm("sti");
#endif /* __GNUC__ */

  if (!IOEnabled) {
    sysi86(SI86IOPL, 0);
  }
}

