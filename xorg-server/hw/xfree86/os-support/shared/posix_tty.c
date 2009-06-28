/*
 * Copyright 1993-2003 by The XFree86 Project, Inc.
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
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 */
/*
 *
 * Copyright (c) 1997  Metro Link Incorporated
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
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

static int 
GetBaud (int baudrate)
{
#ifdef B300
	if (baudrate == 300)
		return B300;
#endif
#ifdef B1200
	if (baudrate == 1200)
		return B1200;
#endif
#ifdef B2400
	if (baudrate == 2400)
		return B2400;
#endif
#ifdef B4800
	if (baudrate == 4800)
		return B4800;
#endif
#ifdef B9600
	if (baudrate == 9600)
		return B9600;
#endif
#ifdef B19200
	if (baudrate == 19200)
		return B19200;
#endif
#ifdef B38400
	if (baudrate == 38400)
		return B38400;
#endif
#ifdef B57600
	if (baudrate == 57600)
		return B57600;
#endif
#ifdef B115200
	if (baudrate == 115200)
		return B115200;
#endif
#ifdef B230400
	if (baudrate == 230400)
		return B230400;
#endif
#ifdef B460800
	if (baudrate == 460800)
		return B460800;
#endif
	return (0);
}

_X_EXPORT int
xf86OpenSerial (pointer options)
{
#ifdef Lynx
	struct sgttyb ms_sgtty;
#endif
	struct termios t;
	int fd, i;
	char *dev;

	dev = xf86SetStrOption (options, "Device", NULL);
	if (!dev)
	{
		xf86Msg (X_ERROR, "xf86OpenSerial: No Device specified.\n");
		return (-1);
	}

	SYSCALL (fd = open (dev, O_RDWR | O_NONBLOCK));
	if (fd == -1)
	{
		xf86Msg (X_ERROR,
			 "xf86OpenSerial: Cannot open device %s\n\t%s.\n",
			 dev, strerror (errno));
		xfree(dev);
		return (-1);
	}

	if (!isatty (fd))
	{
#if 1
		/* Allow non-tty devices to be opened. */
		xfree(dev);
		return (fd);
#else
		xf86Msg (X_WARNING,
			 "xf86OpenSerial: Specified device %s is not a tty\n",
			 dev);
		SYSCALL (close (fd));
		errno = EINVAL;
		xfree(dev);
		return (-1);
#endif
	}

#ifdef Lynx
	/* LynxOS does not assert DTR without this */
	ioctl (fd, TIOCGETP, (char *) &ms_sgtty);
	ioctl (fd, TIOCSDTR, (char *) &ms_sgtty);
#endif

	/* set up default port parameters */
	SYSCALL (tcgetattr (fd, &t));
	t.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR\
						|IGNCR|ICRNL|IXON);
	t.c_oflag &= ~OPOST;
	t.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	t.c_cflag &= ~(CSIZE|PARENB);
	t.c_cflag |= CS8|CLOCAL;

	cfsetispeed (&t, B9600);
	cfsetospeed (&t, B9600);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

	SYSCALL (tcsetattr (fd, TCSANOW, &t));

	if (xf86SetSerial (fd, options) == -1)
	{
		SYSCALL (close (fd));
		xfree(dev);
		return (-1);
	}

	SYSCALL (i = fcntl (fd, F_GETFL, 0));
	if (i == -1)
	{
		SYSCALL (close (fd));
		xfree(dev);
		return (-1);
	}
	i &= ~O_NONBLOCK;
	SYSCALL (i = fcntl (fd, F_SETFL, i));
	if (i == -1)
	{
		SYSCALL (close (fd));
		xfree(dev);
		return (-1);
	}
	xfree(dev);
	return (fd);
}

_X_EXPORT int
xf86SetSerial (int fd, pointer options)
{
	struct termios t;
	int val;
	const char *s;
	int baud, r;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

	SYSCALL (tcgetattr (fd, &t));

	if ((val = xf86SetIntOption (options, "BaudRate", 0)))
	{
		if ((baud = GetBaud (val)))
		{
			cfsetispeed (&t, baud);
			cfsetospeed (&t, baud);
		}
		else
		{
			xf86Msg (X_ERROR,
				 "Invalid Option BaudRate value: %d\n", val);
			return (-1);
		}
	}

	if ((val = xf86SetIntOption (options, "StopBits", 0)))
	{
		switch (val)
		{
		case 1:
			t.c_cflag &= ~(CSTOPB);
			break;
		case 2:
			t.c_cflag |= CSTOPB;
			break;
		default:
			xf86Msg (X_ERROR,
				 "Invalid Option StopBits value: %d\n", val);
			return (-1);
			break;
		}
	}

	if ((val = xf86SetIntOption (options, "DataBits", 0)))
	{
		switch (val)
		{
		case 5:
			t.c_cflag &= ~(CSIZE);
			t.c_cflag |= CS5;
			break;
		case 6:
			t.c_cflag &= ~(CSIZE);
			t.c_cflag |= CS6;
			break;
		case 7:
			t.c_cflag &= ~(CSIZE);
			t.c_cflag |= CS7;
			break;
		case 8:
			t.c_cflag &= ~(CSIZE);
			t.c_cflag |= CS8;
			break;
		default:
			xf86Msg (X_ERROR,
				 "Invalid Option DataBits value: %d\n", val);
			return (-1);
			break;
		}
	}

	if ((s = xf86SetStrOption (options, "Parity", NULL)))
	{
		if (xf86NameCmp (s, "Odd") == 0)
		{
			t.c_cflag |= PARENB | PARODD;
		}
		else if (xf86NameCmp (s, "Even") == 0)
		{
			t.c_cflag |= PARENB;
			t.c_cflag &= ~(PARODD);
		}
		else if (xf86NameCmp (s, "None") == 0)
		{
			t.c_cflag &= ~(PARENB);
		}
		else
		{
			xf86Msg (X_ERROR, "Invalid Option Parity value: %s\n",
				 s);
			return (-1);
		}
	}

	if ((val = xf86SetIntOption (options, "Vmin", -1)) != -1)
	{
		t.c_cc[VMIN] = val;
	}
	if ((val = xf86SetIntOption (options, "Vtime", -1)) != -1)
	{
		t.c_cc[VTIME] = val;
	}

	if ((s = xf86SetStrOption (options, "FlowControl", NULL)))
	{
		xf86MarkOptionUsedByName (options, "FlowControl");
		if (xf86NameCmp (s, "Xoff") == 0)
		{
			t.c_iflag |= IXOFF;
		}
		else if (xf86NameCmp (s, "Xon") == 0)
		{
			t.c_iflag |= IXON;
		}
		else if (xf86NameCmp (s, "XonXoff") == 0)
		{
			t.c_iflag |= IXON|IXOFF;
		}
		else if (xf86NameCmp (s, "None") == 0)
		{
			t.c_iflag &= ~(IXON | IXOFF);
		}
		else
		{
			xf86Msg (X_ERROR,
				 "Invalid Option FlowControl value: %s\n", s);
			return (-1);
		}
	}

	if ((xf86SetBoolOption (options, "ClearDTR", FALSE)))
	{
#ifdef CLEARDTR_SUPPORT
# if !defined(Lynx) || defined(TIOCMBIC)
		val = TIOCM_DTR;
		SYSCALL (ioctl(fd, TIOCMBIC, &val));
# else
		SYSCALL (ioctl(fd, TIOCCDTR, NULL));
# endif
#else
		xf86Msg (X_WARNING,
			 "Option ClearDTR not supported on this OS\n");
			return (-1);
#endif
		xf86MarkOptionUsedByName (options, "ClearDTR");
	}

	if ((xf86SetBoolOption (options, "ClearRTS", FALSE)))
	{
#ifdef CLEARRTS_SUPPORT
		val = TIOCM_RTS;
		SYSCALL (ioctl(fd, TIOCMBIC, &val));
#else
		xf86Msg (X_WARNING,
			 "Option ClearRTS not supported on this OS\n");
			return (-1);
#endif
		xf86MarkOptionUsedByName (options, "ClearRTS");
	}

	SYSCALL (r = tcsetattr (fd, TCSANOW, &t));
	return (r);
}

_X_EXPORT int
xf86SetSerialSpeed (int fd, int speed)
{
	struct termios t;
	int baud, r;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

	SYSCALL (tcgetattr (fd, &t));

	if ((baud = GetBaud (speed)))
	{
		cfsetispeed (&t, baud);
		cfsetospeed (&t, baud);
	}
	else
	{
		xf86Msg (X_ERROR,
			 "Invalid Option BaudRate value: %d\n", speed);
		return (-1);
	}

	SYSCALL (r = tcsetattr (fd, TCSANOW, &t));
	return (r);
}

_X_EXPORT int
xf86ReadSerial (int fd, void *buf, int count)
{
	int r;
#ifdef DEBUG
	int i;
#endif
	SYSCALL (r = read (fd, buf, count));
#ifdef DEBUG
	ErrorF("ReadingSerial: 0x%x",
	       (unsigned char)*(((unsigned char *)buf)));
	for (i = 1; i < r; i++)
	    ErrorF(", 0x%x",(unsigned char)*(((unsigned char *)buf) + i));
	ErrorF("\n");
#endif
	return (r);
}

_X_EXPORT int
xf86WriteSerial (int fd, const void *buf, int count)
{
	int r;
#ifdef DEBUG
	int i;

	ErrorF("WritingSerial: 0x%x",(unsigned char)*(((unsigned char *)buf)));
	for (i = 1; i < count; i++)
	    ErrorF(", 0x%x",(unsigned char)*(((unsigned char *)buf) + i));
	ErrorF("\n");
#endif
	SYSCALL (r = write (fd, buf, count));
	return (r);
}

_X_EXPORT int
xf86CloseSerial (int fd)
{
	int r;

	SYSCALL (r = close (fd));
	return (r);
}

_X_EXPORT int
xf86WaitForInput (int fd, int timeout)
{
	fd_set readfds;
	struct timeval to;
	int r;

	FD_ZERO(&readfds);

	if (fd >= 0) {
	    FD_SET(fd, &readfds);
	}
	
	to.tv_sec = timeout / 1000000;
	to.tv_usec = timeout % 1000000;

	if (fd >= 0) {
	    SYSCALL (r = select (FD_SETSIZE, &readfds, NULL, NULL, &to));
	}
	else {
	    SYSCALL (r = select (FD_SETSIZE, NULL, NULL, NULL, &to));
	}
	xf86ErrorFVerb (9,"select returned %d\n", r);
	return (r);
}

_X_EXPORT int
xf86SerialSendBreak (int fd, int duration)
{
	int r;

	SYSCALL (r = tcsendbreak (fd, duration));
	return (r);
	
}

_X_EXPORT int
xf86FlushInput(int fd)
{
	fd_set fds;
	struct timeval timeout;
	char c[4];

#ifdef DEBUG
	ErrorF("FlushingSerial\n");
#endif
	if (tcflush(fd, TCIFLUSH) == 0)
		return 0;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) > 0) {
		if (read(fd, &c, sizeof(c)) < 1)
		    return 0;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
	}
	return 0;
}

static struct states {
	int xf;
	int os;
} modemStates[] = {
#ifdef TIOCM_LE
	{ XF86_M_LE, TIOCM_LE },
#endif
#ifdef TIOCM_DTR
	{ XF86_M_DTR, TIOCM_DTR },
#endif
#ifdef TIOCM_RTS
	{ XF86_M_RTS, TIOCM_RTS },
#endif
#ifdef TIOCM_ST
	{ XF86_M_ST, TIOCM_ST },
#endif
#ifdef TIOCM_SR
	{ XF86_M_SR, TIOCM_SR },
#endif
#ifdef TIOCM_CTS
	{ XF86_M_CTS, TIOCM_CTS },
#endif
#ifdef TIOCM_CAR
	{ XF86_M_CAR, TIOCM_CAR },
#elif defined(TIOCM_CD)
	{ XF86_M_CAR, TIOCM_CD },
#endif
#ifdef TIOCM_RNG
	{ XF86_M_RNG, TIOCM_RNG },
#elif defined(TIOCM_RI)
	{ XF86_M_CAR, TIOCM_RI },
#endif
#ifdef TIOCM_DSR
	{ XF86_M_DSR, TIOCM_DSR },
#endif
};

static int numStates = sizeof(modemStates) / sizeof(modemStates[0]);

static int
xf2osState(int state)
{
	int i;
	int ret = 0;

	for (i = 0; i < numStates; i++)
		if (state & modemStates[i].xf)
			ret |= modemStates[i].os;
	return ret;
}

static int
os2xfState(int state)
{
	int i;
	int ret = 0;

	for (i = 0; i < numStates; i++)
		if (state & modemStates[i].os)
			ret |= modemStates[i].xf;
	return ret;
}

static int
getOsStateMask(void)
{
	int i;
	int ret = 0;
	for (i = 0; i < numStates; i++)
		ret |= modemStates[i].os;
	return ret;
}

static int osStateMask = 0;

_X_EXPORT int
xf86SetSerialModemState(int fd, int state)
{
	int ret;
	int s;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

#ifndef TIOCMGET
	return -1;
#else
	if (!osStateMask)
		osStateMask = getOsStateMask();

	state = xf2osState(state);
	SYSCALL((ret = ioctl(fd, TIOCMGET, &s)));
	if (ret < 0)
		return -1;
	s &= ~osStateMask;
	s |= state;
	SYSCALL((ret = ioctl(fd, TIOCMSET, &s)));
	if (ret < 0)
		return -1;
	else
		return 0;
#endif
}

_X_EXPORT int
xf86GetSerialModemState(int fd)
{
	int ret;
	int s;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

#ifndef TIOCMGET
	return -1;
#else
	SYSCALL((ret = ioctl(fd, TIOCMGET, &s)));
	if (ret < 0)
		return -1;
	return os2xfState(s);
#endif
}

_X_EXPORT int
xf86SerialModemSetBits(int fd, int bits)
{
	int ret;
	int s;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

#ifndef TIOCMGET
	return -1;
#else
	s = xf2osState(bits);
	SYSCALL((ret = ioctl(fd, TIOCMBIS, &s)));
	return ret;
#endif
}

_X_EXPORT int
xf86SerialModemClearBits(int fd, int bits)
{
	int ret;
	int s;

	if (fd < 0)
		return -1;

	/* Don't try to set parameters for non-tty devices. */
	if (!isatty(fd))
		return 0;

#ifndef TIOCMGET
	return -1;
#else
	s = xf2osState(bits);
	SYSCALL((ret = ioctl(fd, TIOCMBIC, &s)));
	return ret;
#endif
}
