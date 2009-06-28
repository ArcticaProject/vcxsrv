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

/* Modified for FreeBSD by David Dawes <dawes@XFree86.org> */


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <machine/joystick.h>
#include <fcntl.h>

#include "misc.h"
#include "xf86.h"

#define JS_RETURN sizeof(struct joystick)

/***********************************************************************
 *
 * xf86JoystickOn --
 *
 * open the device and init timeout according to the device value.
 *
 ***********************************************************************
 */

int
xf86JoystickOn(char * name, int *timeout, int *centerX, int *centerY)
{
  int   status;
  int   changed = 0;
  int   timeinmicros;
  struct joystick	js;
  
#ifdef DEBUG
  ErrorF("xf86JoystickOn: %s\n", name);
#endif
  
  if ((status = open(name, O_RDWR | O_NDELAY, 0)) < 0)
    {
      xf86Msg(X_WARNING, "xf86JoystickOn: Cannot open joystick '%s' (%s)\n",
	      name, strerror(errno));
      return -1;
    }

  if (*timeout <= 0) {
    /* Use the current setting */
    ioctl(status, JOY_GETTIMEOUT, (char *)&timeinmicros);
    *timeout = timeinmicros / 1000;
    if (*timeout == 0)
      *timeout = 1;
    changed = 1;
  }
  /* Maximum allowed timeout in the FreeBSD driver is 10ms */
  if (*timeout > 10) {
    *timeout = 10;
    changed = 1;
  }
  
  if (changed)
    xf86Msg(X_PROBED, "Joystick: timeout value = %d\n", *timeout);

  timeinmicros = *timeout * 1000;

  /* Assume the joystick is centred when this is called */
  read(status, &js, JS_RETURN);
  if (*centerX < 0) {
    *centerX = js.x;
    xf86Msg(X_PROBED, "Joystick: CenterX set to %d\n", *centerX);
  }
  if (*centerY < 0) {
    *centerY = js.y;
    xf86Msg(X_PROBED, "Joystick: CenterY set to %d\n", *centerY);
  }

  return status;
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
  struct joystick	js;
  int                   status;
  
  status = read(fd, &js, JS_RETURN);
 
  if (status != JS_RETURN)
    {
      Error("Joystick read");      
      return 0;
    }
  
  *x = js.x;
  *y = js.y;
  *buttons = js.b1 | (js.b2 << 1);
#ifdef DEBUG
  ErrorF("xf86JoystickGetState: x = %d, y = %d, buttons = %d\n", *x, *y,
	 *buttons);
#endif
  
  return 1;
}

/*
 * Entry point for XFree86 Loader
 */
void
bsd_jstkModuleInit(pointer *data, INT32 *magic)
{
    *magic = MAGIC_DONE;
    *data = NULL;
}
/* end of bsd_jstk.c */
