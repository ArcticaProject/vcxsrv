/*
 *
 * Copyright © 2004 Franco Catrin
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Franco Catrin not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Franco Catrin makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * FRANCO CATRIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL FRANCO CATRIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "neomagic.h"

extern struct NeoChipInfo neoChips[];

void
InitCard (char *name)
{
    KdCardAttr attr;
    struct NeoChipInfo *chip;

    for (chip = neoChips; chip->name != NULL; ++chip) {
        int j = 0;
        while (LinuxFindPci(chip->vendor, chip->device, j++, &attr)) {
            KdCardInfoAdd(&neoFuncs, &attr, 0);
        }
    }
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
    int ret;

    if (!(ret = vesaProcessArgument (argc, argv, i)))
    ret = KdProcessArgument(argc, argv, i);
    return ret;
}
