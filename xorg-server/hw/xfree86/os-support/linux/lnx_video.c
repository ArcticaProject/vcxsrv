/*
 * Copyright 1992 by Orest Zborowski <obz@Kodak.com>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Orest Zborowski and David Wexelblat
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  Orest Zborowski
 * and David Wexelblat make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OREST ZBOROWSKI AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OREST ZBOROWSKI OR DAVID WEXELBLAT BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <errno.h>
#include <string.h>

#include <X11/X.h>
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

static Bool ExtendedEnabled = FALSE;

#ifdef __ia64__

#include "compiler.h"
#include <sys/io.h>

#elif !defined(__powerpc__) && \
      !defined(__mc68000__) && \
      !defined(__sparc__) && \
      !defined(__mips__) && \
      !defined(__nds32__) && \
      !defined(__arm__) && \
      !defined(__aarch64__) && \
      !defined(__arc__) && \
      !defined(__xtensa__)

/*
 * Due to conflicts with "compiler.h", don't rely on <sys/io.h> to declare
 * these.
 */
extern int ioperm(unsigned long __from, unsigned long __num, int __turn_on);
extern int iopl(int __level);

#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
    pVidMem->initialised = TRUE;
}

/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

#if defined(__powerpc__)
volatile unsigned char *ioBase = NULL;

#ifndef __NR_pciconfig_iobase
#define __NR_pciconfig_iobase	200
#endif

static Bool
hwEnableIO(void)
{
    int fd;
    unsigned int ioBase_phys = syscall(__NR_pciconfig_iobase, 2, 0, 0);

    fd = open("/dev/mem", O_RDWR);
    if (ioBase == NULL) {
        ioBase = (volatile unsigned char *) mmap(0, 0x20000,
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_SHARED, fd, ioBase_phys);
    }
    close(fd);

    return ioBase != MAP_FAILED;
}

static void
hwDisableIO(void)
{
    munmap(ioBase, 0x20000);
    ioBase = NULL;
}

#elif defined(__i386__) || defined(__x86_64__) || defined(__ia64__) || \
      defined(__alpha__)

static Bool
hwEnableIO(void)
{
    if (ioperm(0, 1024, 1) || iopl(3)) {
        ErrorF("xf86EnableIOPorts: failed to set IOPL for I/O (%s)\n",
               strerror(errno));
        return FALSE;
    }
#if !defined(__alpha__)
    /* XXX: this is actually not trapping anything because of iopl(3)
     * above */
    ioperm(0x40, 4, 0);         /* trap access to the timer chip */
    ioperm(0x60, 4, 0);         /* trap access to the keyboard controller */
#endif

    return TRUE;
}

static void
hwDisableIO(void)
{
    iopl(0);
    ioperm(0, 1024, 0);
}

#else /* non-IO architectures */

#define hwEnableIO() TRUE
#define hwDisableIO() do {} while (0)

#endif

Bool
xf86EnableIO(void)
{
    if (ExtendedEnabled)
        return TRUE;

    ExtendedEnabled = hwEnableIO();

    return ExtendedEnabled;
}

void
xf86DisableIO(void)
{
    if (!ExtendedEnabled)
        return;

    hwDisableIO();

    ExtendedEnabled = FALSE;
}

#if defined (__alpha__)

extern int readDense8(void *Base, register unsigned long Offset);
extern int readDense16(void *Base, register unsigned long Offset);
extern int readDense32(void *Base, register unsigned long Offset);
extern void
 writeDense8(int Value, void *Base, register unsigned long Offset);
extern void
 writeDense16(int Value, void *Base, register unsigned long Offset);
extern void
 writeDense32(int Value, void *Base, register unsigned long Offset);

void (*xf86WriteMmio8) (int Value, void *Base, unsigned long Offset)
    = writeDense8;
void (*xf86WriteMmio16) (int Value, void *Base, unsigned long Offset)
    = writeDense16;
void (*xf86WriteMmio32) (int Value, void *Base, unsigned long Offset)
    = writeDense32;
int (*xf86ReadMmio8) (void *Base, unsigned long Offset)
    = readDense8;
int (*xf86ReadMmio16) (void *Base, unsigned long Offset)
    = readDense16;
int (*xf86ReadMmio32) (void *Base, unsigned long Offset)
    = readDense32;

#endif                          /* __alpha__ */
