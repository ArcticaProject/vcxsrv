/*
 * Copyright 2002 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Rickard E. (Rik) Faith <faith@redhat.com>
 *
 */

#include "dmxconfig.h"
#include "dmxparse.h"
#include "dmxprint.h"
#include "dmxcompat.h"

int main(int argc, char **argv)
{
    DMXConfigEntryPtr entry;
    FILE              *str;

    if (argc != 2 && argc !=3) {
        fprintf(stderr, "Usage: vdltodmx inFile [outFile]\n");
        return 1;
    }
    if (argc == 2) {
        str = stdout;
    } else if (!(str = fopen(argv[2], "w"))) {
        fprintf(stderr, "Cannot open %s for write\n", argv[2]);
        return 2;
    }
    entry = dmxVDLRead(argv[1]);
    dmxConfigPrint(str, entry);
    return 0;
}
