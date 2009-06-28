/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
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
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 */

#include "config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/*
 * Initialization
 */
static int xpid;
Display *DPY;

/*
 * Implementation
 */
Bool
startx(void)
{
    int timeout = 8;

    if (getenv("DISPLAY") != NULL)
	/* already running Xserver */
	return (False);

    if (XF86Config_path == NULL) {
	char *home, filename[PATH_MAX];
	char commandline[PATH_MAX * 4];
	int c_pos;
	int len;
	/* 
	 * The name of the 4.0 binary is XFree86. X might also
	 * be the name of the 3.3 binary. Therefore don't change
	 * name to 'X'.
	 */
	if (XFree86_path)
	    c_pos = XmuSnprintf(commandline, sizeof(commandline),
				"%s/"__XSERVERNAME__" :8 -configure ",XFree86_path);
	else
	    c_pos = XmuSnprintf(commandline, sizeof(commandline), 
				"%s/bin/"__XSERVERNAME__" :8 -configure ", XFree86Dir);
	if (XF86Module_path && ((len = sizeof(commandline) - c_pos) > 0))
	    c_pos += XmuSnprintf(commandline + c_pos,len,
				 " -modulepath %s",XF86Module_path);
	if (XF86Font_path && ((len = sizeof(commandline) - c_pos) > 0))
	    c_pos += XmuSnprintf(commandline + c_pos,len,
				 " -fontpath %s",XF86Font_path);
	
	if (system(commandline) != 0) {
	    fprintf(stderr, "Failed to run \"X -configure\".\n");
	    exit(1);
	}

	if ((home = getenv("HOME")) == NULL)
	    home = "/";

#ifndef QNX4
	XmuSnprintf(filename, sizeof(filename), "%s/"__XCONFIGFILE__".new", home);
#else
	XmuSnprintf(filename, sizeof(filename), "//%d%s/"__XCONFIGFILE__".new",
		    getnid(), home);
#endif

	/* this memory is never released, even if the value of XF86Config_path is
	 * changed.
	 */
	XF86Config_path = XtNewString(filename);
    }

    putenv("DISPLAY=:8");

    switch (xpid = fork()) {
	case 0: {
	    char path[PATH_MAX];
	    /* Don't change to X! see above */
	    if (XFree86_path)
	        XmuSnprintf(path, sizeof(path), "%s/"__XSERVERNAME__, XFree86_path);
	    else
	        XmuSnprintf(path, sizeof(path), "%s/bin/"__XSERVERNAME__, XFree86Dir);
	    execl(path, "X", ":8", /*"+xinerama",*/ "+accessx","-allowMouseOpenFail",
		  "-xf86config", XF86Config_path, (void *)NULL);
	    exit(-127);
	}   break;
	case -1:
	    fprintf(stderr, "Cannot fork.\n");
	    exit(1);
	    break;
	default:
	    break;
    }

    while (timeout > 0) {
	int status;

	sleep(timeout -= 2);
	if (waitpid(xpid, &status, WNOHANG | WUNTRACED) == xpid)
	    break;
	else {
	    DPY = XOpenDisplay(NULL);
	    if (DPY != NULL)
		break;
	}
    }

    if (DPY == NULL) {
	fprintf(stderr, "Cannot connect to X server.\n");
	exit(1);
    }

    return (True);
}

void
endx(void)
{
    if (xpid != 0)
	kill(xpid, SIGTERM);
}
