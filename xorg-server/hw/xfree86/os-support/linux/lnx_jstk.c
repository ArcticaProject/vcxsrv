/*
 * Copyright 1995 by Frederic Lepied, France. <fred@sugix.frmug.fr.net>       
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


static const char rcs_id[] = "Id: lnx_jstk.c,v 1.1 1995/12/20 14:06:09 lepied Exp";

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define inline __inline__
#include <linux/joystick.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "xf86.h"

#if !defined(JSIOCGTIMELIMIT)
/* make 2.1.x joystick.h backward compatable */
#define JSIOCGTIMELIMIT		JS_GET_TIMELIMIT
#define JSIOCSTIMELIMIT		JS_SET_TIMELIMIT
#define js_status		JS_DATA_TYPE
#endif


/***********************************************************************
 *
 * xf86JoystickOn --
 *
 * open the device and init timeout according to the device value.
 *
 ***********************************************************************
 */

int
xf86JoystickOn(char *name, int *timeout, int *centerX, int *centerY)
{
  int			fd;
  struct js_status	js;
    
#ifdef DEBUG
  ErrorF("xf86JoystickOn %s\n", name);
#endif

  if ((fd = open(name, O_RDWR | O_NDELAY, 0)) < 0)
    {
      xf86Msg(X_WARNING, "Cannot open joystick '%s' (%s)\n", name,
		strerror(errno));
      return -1;
    }

  if (*timeout == 0) {
    if (ioctl (fd, JSIOCGTIMELIMIT, timeout) == -1) {
      Error("joystick JSIOCGTIMELIMIT ioctl");
    }
    else {
      xf86Msg(X_CONFIG, "Joystick: timeout value = %d\n", *timeout);
    }
  }
  else {
    if (ioctl(fd, JSIOCSTIMELIMIT, timeout) == -1) {
      Error("joystick JSIOCSTIMELIMIT ioctl");
    }
  }

  /* Assume the joystick is centred when this is called */
  read(fd, &js, JS_RETURN);
  if (*centerX < 0) {
    *centerX = js.x;
    xf86Msg(X_CONFIG, "Joystick: CenterX set to %d\n", *centerX);
  }
  if (*centerY < 0) {
    *centerY = js.y;
    xf86Msg(X_CONFIG, "Joystick: CenterY set to %d\n", *centerY);
  }

  return fd;
}

/***********************************************************************
 *
 * xf86JoystickInit --
 *
 * called when X device is initialized.
 *
 ***********************************************************************
 */

void
xf86JoystickInit()
{
	return;
}

/***********************************************************************
 *
 * xf86JoystickOff --
 *
 * close the handle.
 *
 ***********************************************************************
 */

int
xf86JoystickOff(int *fd, int doclose)
{
  int   oldfd;
  
  if (((oldfd = *fd) >= 0) && doclose) {
    close(*fd);
    *fd = -1;
  }
  return oldfd;
}

/***********************************************************************
 *
 * xf86JoystickGetState --
 *
 * return the state of buttons and the position of the joystick.
 *
 ***********************************************************************
 */

int
xf86JoystickGetState(int fd, int *x, int *y, int *buttons)
{
  struct js_status	js;
  int			status;
  
  status = read(fd, &js, JS_RETURN);
 
  if (status != JS_RETURN)
    {
      Error("Joystick read");      
      return 0;
    }
  
  *x = js.x;
  *y = js.y;
  *buttons = js.buttons;
  
  return 1;
}

/*
 * Entry point for XFree86 Loader
 */
void
linux_jstkModuleInit(pointer *data, INT32 *magic)
{
    *magic = MAGIC_DONE;
    *data = NULL;
}

/* end of lnx_jstk.c */
