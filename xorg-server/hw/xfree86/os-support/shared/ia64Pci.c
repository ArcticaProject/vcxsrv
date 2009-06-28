/*
 * Copyright (C) 2002-2003 The XFree86 Project, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 */

/*
 * This file contains the glue needed to support various IA-64 chipsets.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/pci.h>

#include "compiler.h"
#include "Pci.h"

/*
 * We use special in/out routines here since Altix platforms require the
 * use of the sysfs legacy_io interface.  The legacy_io file maps to the I/O
 * space of a given PCI domain; reads and writes are used to do port I/O.
 * The file descriptor for the file is stored in the upper bits of the
 * value passed in by the caller, and is created and populated by
 * xf86MapLegacyIO.
 *
 * If the legacy_io interface doesn't exist, we fall back to the glibc in/out
 * routines, which are prefixed by an underscore (e.g. _outb).
 */
static int ia64_port_to_fd(unsigned long port)
{
    return (port >> 24) & 0xffffffff;
}

_X_EXPORT void outb(unsigned long port, unsigned char val)
{
    int fd = ia64_port_to_fd(port);

    if (!fd) {
	_outb(val, port & 0xffff);
	goto out;
    }
    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	goto out;
    }
    if (write(fd, &val, 1) != 1) {
	ErrorF("I/O write failed\n");
	goto out;
    }
 out:
    return;
}

_X_EXPORT void outw(unsigned long port, unsigned short val)
{
    int fd = ia64_port_to_fd(port);

    if (!fd) {
	_outw(val, port & 0xffff);
	goto out;
    }
    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	goto out;
    }
    if (write(fd, &val, 2) != 2) {
	ErrorF("I/O write failed\n");
	goto out;
    }
 out:
    return;
}

_X_EXPORT void outl(unsigned long port, unsigned int val)
{
    int fd = ia64_port_to_fd(port);

    if (!fd) {
	_outl(val, port & 0xffff);
	goto out;
    }
    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	goto out;
    }
    if (write(fd, &val, 4) != 4) {
	ErrorF("I/O write failed\n");
	goto out;
    }
 out:
    return;
}

_X_EXPORT unsigned int inb(unsigned long port)
{
    int fd = ia64_port_to_fd(port);
    unsigned char val;

    if (!fd)
	return _inb(port & 0xffff);

    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	val = -1;
	goto out;
    }
    if (read(fd, &val, 1) != 1) {
	ErrorF("I/O read failed\n");
	val = -1;
	goto out;
    }
 out:
    return val;
}

_X_EXPORT unsigned int inw(unsigned long port)
{
    int fd = ia64_port_to_fd(port);
    unsigned short val;

    if (!fd)
	return _inw(port & 0xffff);

    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	val = -1;
	goto out;
    }
    if (read(fd, &val, 2) != 2) {
	ErrorF("I/O read failed\n");
	val = -1;
	goto out;
    }
 out:
    return val;
}

_X_EXPORT unsigned int inl(unsigned long port)
{
    int fd = ia64_port_to_fd(port);
    unsigned int val;

    if (!fd)
	return _inl(port & 0xffff);

    if (lseek(fd, port & 0xffff, SEEK_SET) == -1) {
	ErrorF("I/O lseek failed\n");
	val = -1;
	goto out;
    }
    if (read(fd, &val, 4) != 4) {
	ErrorF("I/O read failed\n");
	val = -1;
	goto out;
    }
 out:
    return val;
}

