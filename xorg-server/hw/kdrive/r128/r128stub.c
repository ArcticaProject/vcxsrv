/*
 * Copyright © 2003 Anders Carlsson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Anders Carlsson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Anders Carlsson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ANDERS CARLSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "r128.h"
#include "klinux.h"

void
InitCard (char *name)
{
  KdCardAttr attr;

  if (LinuxFindPci (0x1002, 0x4c46, 0, &attr))
    KdCardInfoAdd (&r128Funcs, &attr, 0);
  else if (LinuxFindPci (0x1002, 0x5046, 0, &attr))
    KdCardInfoAdd (&r128Funcs, &attr, 0);
}

void
InitOutput (ScreenInfo *pScreenInfo, int argc, char **argv)
{
    KdInitOutput (pScreenInfo, argc, argv);
}

void
InitInput (int argc, char **argv)
{
    KdOsAddInputDrivers ();
    KdInitInput ();
}

void
ddxUseMsg (void)
{
    KdUseMsg();
    vesaUseMsg();
}

int
ddxProcessArgument (int argc, char **argv, int i)
{
    int	ret;
    
    if (!(ret = vesaProcessArgument (argc, argv, i)))
	ret = KdProcessArgument(argc, argv, i);
    return ret;
}
