
/* sigio.c -- Support for SIGIO handler installation and removal
 * Created: Thu Jun  3 15:39:18 1999 by faith@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * Authors: Rickard E. (Rik) Faith <faith@valinux.com>
 */
/*
 * Copyright (c) 2002 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

# include <X11/X.h>
# include "xf86.h"
# include "xf86Priv.h"
# include "xf86_OSlib.h"
# include "inputstr.h"

/*
 * Linux libc5 defines FASYNC, but not O_ASYNC.  Don't know if it is
 * functional or not.
 */
#if defined(FASYNC) && !defined(O_ASYNC)
#  define O_ASYNC FASYNC
#endif

#ifdef MAX_DEVICES
/* MAX_DEVICES represents the maximimum number of input devices usable
 * at the same time plus one entry for DRM support.
 */
# define MAX_FUNCS   (MAX_DEVICES + 1)
#else
# define MAX_FUNCS 16
#endif

typedef struct _xf86SigIOFunc {
    void    (*f) (int, void *);
    int	    fd;
    void    *closure;
} Xf86SigIOFunc;

static Xf86SigIOFunc	xf86SigIOFuncs[MAX_FUNCS];
static int		xf86SigIOMax;
static int		xf86SigIOMaxFd;
static fd_set		xf86SigIOMask;

/*
 * SIGIO gives no way of discovering which fd signalled, select
 * to discover
 */
static void
xf86SIGIO (int sig)
{
    int	    i;
    fd_set  ready;
    struct timeval  to;
    int save_errno = errno;	/* do not clobber the global errno */
    int	    r;

    ready = xf86SigIOMask;
    to.tv_sec = 0;
    to.tv_usec = 0;
    SYSCALL (r = select (xf86SigIOMaxFd, &ready, 0, 0, &to));
    for (i = 0; r > 0 && i < xf86SigIOMax; i++)
	if (xf86SigIOFuncs[i].f && FD_ISSET (xf86SigIOFuncs[i].fd, &ready))
	{
	    (*xf86SigIOFuncs[i].f)(xf86SigIOFuncs[i].fd,
				   xf86SigIOFuncs[i].closure);
	    r--;
	}
    if (r > 0) {
      xf86Msg(X_ERROR, "SIGIO %d descriptors not handled\n", r);
    }
    /* restore global errno */
    errno = save_errno;
}

static int
xf86IsPipe (int fd)
{
    struct stat	buf;
    
    if (fstat (fd, &buf) < 0)
	return 0;
    return S_ISFIFO(buf.st_mode);
}

_X_EXPORT int
xf86InstallSIGIOHandler(int fd, void (*f)(int, void *), void *closure)
{
    struct sigaction sa;
    struct sigaction osa;
    int	i;
    int blocked;

    for (i = 0; i < MAX_FUNCS; i++)
    {
	if (!xf86SigIOFuncs[i].f)
	{
	    if (xf86IsPipe (fd))
		return 0;
	    blocked = xf86BlockSIGIO();
	    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_ASYNC) == -1) {
		xf86Msg(X_WARNING, "fcntl(%d, O_ASYNC): %s\n", 
			fd, strerror(errno));
		xf86UnblockSIGIO(blocked);
		return 0;
	    }
	    if (fcntl(fd, F_SETOWN, getpid()) == -1) {
		xf86Msg(X_WARNING, "fcntl(%d, F_SETOWN): %s\n", 
			fd, strerror(errno));
		xf86UnblockSIGIO(blocked);
		return 0;
	    }
	    sigemptyset(&sa.sa_mask);
	    sigaddset(&sa.sa_mask, SIGIO);
	    sa.sa_flags   = 0;
	    sa.sa_handler = xf86SIGIO;
	    sigaction(SIGIO, &sa, &osa);
	    xf86SigIOFuncs[i].fd = fd;
	    xf86SigIOFuncs[i].closure = closure;
	    xf86SigIOFuncs[i].f = f;
	    if (i >= xf86SigIOMax)
		xf86SigIOMax = i+1;
	    if (fd >= xf86SigIOMaxFd)
		xf86SigIOMaxFd = fd + 1;
	    FD_SET (fd, &xf86SigIOMask);
	    xf86UnblockSIGIO(blocked);
	    return 1;
	}
 	/* Allow overwriting of the closure and callback */
 	else if (xf86SigIOFuncs[i].fd == fd)
 	{
 	    xf86SigIOFuncs[i].closure = closure;
 	    xf86SigIOFuncs[i].f = f;
 	    return 1;
 	}
    }
    return 0;
}

_X_EXPORT int
xf86RemoveSIGIOHandler(int fd)
{
    struct sigaction sa;
    struct sigaction osa;
    int	i;
    int max;
    int maxfd;
    int ret;

    max = 0;
    maxfd = -1;
    ret = 0;
    for (i = 0; i < MAX_FUNCS; i++)
    {
	if (xf86SigIOFuncs[i].f)
	{
	    if (xf86SigIOFuncs[i].fd == fd)
	    {
		xf86SigIOFuncs[i].f = 0;
		xf86SigIOFuncs[i].fd = 0;
		xf86SigIOFuncs[i].closure = 0;
		FD_CLR (fd, &xf86SigIOMask);
		ret = 1;
	    }
	    else
	    {
		max = i + 1;
		if (xf86SigIOFuncs[i].fd >= maxfd)
		    maxfd = xf86SigIOFuncs[i].fd + 1;
	    }
	}
    }
    if (ret)
    {
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_ASYNC);
	xf86SigIOMax = max;
	xf86SigIOMaxFd = maxfd;
	if (!max)
	{
	    sigemptyset(&sa.sa_mask);
	    sigaddset(&sa.sa_mask, SIGIO);
	    sa.sa_flags   = 0;
	    sa.sa_handler = SIG_IGN;
	    sigaction(SIGIO, &sa, &osa);
	}
    }
    return ret;
}

_X_EXPORT int
xf86BlockSIGIO (void)
{
    sigset_t	set, old;
    int		ret;
    
    sigemptyset (&set);
    sigaddset (&set, SIGIO);
    sigprocmask (SIG_BLOCK, &set, &old);
    ret = sigismember (&old, SIGIO);
    return ret; 
}

_X_EXPORT void
xf86UnblockSIGIO (int wasset)
{
    sigset_t	set;

    if (!wasset)
    {
	sigemptyset (&set);
	sigaddset (&set, SIGIO);
	sigprocmask (SIG_UNBLOCK, &set, NULL);
    }
}

void
xf86AssertBlockedSIGIO (char *where)
{
    sigset_t	set, old;

    sigemptyset (&set);
    sigprocmask (SIG_BLOCK, &set, &old);
    if (!sigismember (&old, SIGIO))
	xf86Msg (X_ERROR, "SIGIO not blocked at %s\n", where);
}

/* XXX This is a quick hack for the benefit of xf86SetSilkenMouse() */

int
xf86SIGIOSupported (void)
{
    return 1;
}
