/**
 * Copyright © 2011 Red Hat, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdint.h>
#include "misc.h"

static void dix_version_compare(void)
{
    int rc;

    rc = version_compare(0, 0, 1, 0);
    assert(rc < 0);
    rc = version_compare(1, 0, 0, 0);
    assert(rc > 0);
    rc = version_compare(0, 0, 0, 0);
    assert(rc == 0);
    rc = version_compare(1, 0, 1, 0);
    assert(rc == 0);
    rc = version_compare(1, 0, 0, 9);
    assert(rc > 0);
    rc = version_compare(0, 9, 1, 0);
    assert(rc < 0);
    rc = version_compare(1, 0, 1, 9);
    assert(rc < 0);
    rc = version_compare(1, 9, 1, 0);
    assert(rc > 0);
    rc = version_compare(2, 0, 1, 9);
    assert(rc > 0);
    rc = version_compare(1, 9, 2, 0);
    assert(rc < 0);
}

int main(int argc, char** argv)
{
    dix_version_compare();

    return 0;
}
