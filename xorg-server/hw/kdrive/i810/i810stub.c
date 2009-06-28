/* COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2000, 2001 Nokia Home Communications

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group */

/* Stub functions for the i810 KDrive driver
   Author: Pontus Lidman <pontus.lidman@nokia.com>
*/

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kxv.h"
#include "i810.h"
#include "klinux.h"

static const int i810Cards[]={
    PCI_CHIP_I810,
    PCI_CHIP_I810_DC100,
    PCI_CHIP_I810_E
};

#define numI810Cards (sizeof(i810Cards) / sizeof(i810Cards[0]))

void
InitCard (char *name)
{
    KdCardAttr	attr;
    int		i;

    for (i = 0; i < numI810Cards; i++)
	if (LinuxFindPci (0x8086, i810Cards[i], 0, &attr))
	    KdCardInfoAdd (&i810Funcs, &attr, (void *) i810Cards[i]);
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
}

int
ddxProcessArgument (int argc, char **argv, int i)
{
    int	ret;
    
    ret = KdProcessArgument(argc, argv, i);
    return ret;
}
