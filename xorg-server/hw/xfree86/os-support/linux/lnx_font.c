/*
 * Copyright 2004, Egbert Eich
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * EGBERT EICH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
 * NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Egbert Eich shall not
 * be used in advertising or otherwise to promote the sale, use or other deal-
 *ings in this Software without prior written authorization from Egbert Eich.
 *
 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "lnx.h"

#include <sys/stat.h>
#include <string.h>
#include <errno.h>

struct {
    int width;
    int height;
    int charcount;
    unsigned char *data;
} lnxfont = { 0, 0, 0, NULL };


static Bool
getfont(int *width, int *height, 
	int *charcount, unsigned char *data)
{
    struct console_font_op op;
    struct consolefontdesc ds;
    int result;

    op.op = KD_FONT_OP_GET;
    op.width = *width;
    op.height = *height;
    op.charcount = *charcount;
    op.data = data;
    op.flags = 0;

    SYSCALL(result = ioctl(xf86Info.consoleFd, KDFONTOP, &op));
#ifdef DEBUG
    ErrorF("Console font read: h: %i count: %i\n",op.height,op.charcount);
#endif

    if (!result) {

	*width = op.width;
	*height = op.height;
	*charcount = op.charcount;
	
	return TRUE;
    } 
    
    if (errno != ENOSYS && errno != EINVAL)
	return FALSE;

    /* GIO_FONTX fallback */
    ds.charcount = *charcount;
    ds.charheight = *height;
    ds.chardata = (char *)data;
    *width = 8;
    
    SYSCALL(result = ioctl(xf86Info.consoleFd, GIO_FONTX, &ds));
    
    if (!result) {
	
	*charcount = ds.charcount;
	*height = ds.charheight;

	return TRUE;
    }

    if (errno != ENOSYS && errno != EINVAL)
	return FALSE;

    /* GIO_FONT fallback */
    if (*charcount < 256)
	return FALSE;

    SYSCALL(result = ioctl(xf86Info.consoleFd, GIO_FONT, data));

    if (!result) {
	*height = 0;
	*charcount = 512;
	return TRUE;
    }

    return FALSE;

}

#define VERSION_LEN 31

Bool
lnx_savefont(void)
{
    unsigned char *fontdata;
#if CHECK_OS_VERSION
    char kernel_version[VERSION_LEN + 1];
    int k_major, k_minor, k_release;
#endif
    int size;
    int fd;
    int width = 32, height = 32, charcount = 2048;

#ifdef DEBUG
    ErrorF("SAVE font\n");
#endif

#if CHECK_OS_VERSION
    /* Check if the kernel has full support for this */
    if ((fd = open ("/proc/sys/kernel/osrelease",O_RDONLY)) == -1) {
	close (fd);
	return TRUE;
    }
    size = read(fd, kernel_version, VERSION_LEN);
    close (fd);

    if (size < 0)
	return TRUE;

    size = sscanf(kernel_version, "%d.%d.%d",&k_major,&k_minor,&k_release);
    if (size < 3 
	|| (k_major < 2) 
	|| ((k_major == 2) 
	    && ((k_minor < 6) 
		|| ( k_minor == 6 
		     && k_release < 11))))
	return TRUE;
#endif
							    
    /* if we are in fbdev mode we don't bother saving fonts */
    if ((fd = open ("/dev/fb0",O_RDWR)) != -1) {
	close (fd);
	return TRUE;
    }

    if (!getfont(&width, &height, &charcount, NULL)) {
	xf86Msg(X_WARNING, 
		"lnx_savefont: cannot obtain font info\n");
	goto error;
    } else if (charcount == 2048) {
	xf86Msg(X_WARNING, "lnx_savefont: "
		"kernel bug: kernel doesn't report font info\n");
	return FALSE;
    }

    size = (width + 7)/8 * 32 * charcount;
    fontdata = (unsigned char *)xnfalloc(size);
    if (!fontdata) {
	xf86Msg(X_WARNING,
		"lnx_savefont: cannot allocate memory to save font\n");
	goto error;
    }

    if (!getfont(&width, &height, &charcount, fontdata)) {
	xf86Msg(X_WARNING,"lnx_savefont: cannot read font data\n");
	goto error;
    }
    lnxfont.width = width;
    lnxfont.height = height;
    lnxfont.charcount = charcount;
    lnxfont.data = fontdata;

    return TRUE;

 error:
    return FALSE;
}

static Bool
setfont(int width, int height, 
	int charcount, unsigned char *data)
{
    struct console_font_op op;
    struct consolefontdesc ds;
    int result;

    op.op = KD_FONT_OP_SET;
    op.flags = 0;
    op.charcount = charcount;
    op.width = width;
    op.height = height;
    op.data = data;

    SYSCALL(result = ioctl(xf86Info.consoleFd, KDFONTOP, &op));

    if (!result) 
	return TRUE;

    if (errno != ENOSYS && errno != EINVAL)
	return FALSE;

    /* PIO_FONTX fallback */
    if (width != 8)
	return FALSE;

    ds.charcount = charcount;
    ds.chardata = (char *)data;
    ds.charheight = height;
    SYSCALL(result = ioctl(xf86Info.consoleFd, PIO_FONTX, &ds));
    
    if (!result)
	return TRUE;

    if (errno != ENOSYS && errno != EINVAL)
	return FALSE;

    /* PIO_FONT fallback */
    SYSCALL(result = ioctl(xf86Info.consoleFd, PIO_FONT, data));
    
    if (!result)
	return TRUE;
    
    return FALSE;
}

Bool
lnx_restorefont(void)
{
    if (lnxfont.data == NULL)
	return FALSE;
#ifdef DEBUG
    ErrorF("RESTORE font\n");
#endif
#if 0
    /* must wack the height to make the kernel reprogram the VGA registers */
    if (!setfont(lnxfont.width, lnxfont.height + 1, lnxfont.charcount,
		lnxfont.data)) {
	xf86Msg(X_WARNING,"lnx_fontretore: cannot write font data\n");
	return FALSE;
    }
#endif
    if (!setfont(lnxfont.width, lnxfont.height, lnxfont.charcount,
		lnxfont.data)) {
	xf86Msg(X_WARNING,"lnx_restorefont: cannot write font data\n");
	return FALSE;
    }

    return TRUE;
}

Bool
lnx_switchaway(void)
{
    Bool ret;

    /* temporarily switch to text mode */
    ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT);
    ret = lnx_restorefont();
    ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS);
    return ret;
}

void
lnx_freefontdata(void)
{
    if (lnxfont.data == NULL)
	return;

    xfree(lnxfont.data);
    lnxfont.data = NULL;
    lnxfont.width = lnxfont.height = lnxfont.charcount = 0;
}
