/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and The Open Group make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
static  char sccsid[] = "@(#)kbd_mode.c 7.1 87/04/13";
 */

/*
 * Copyright 1986 by Sun Microsystems, Inc.
 *
 *      kbd_mode:       set keyboard encoding mode
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#if defined(SVR4) || defined(__SVR4) || defined(__bsdi__)
#include <fcntl.h>
#ifndef __bsdi__
#include <sys/kbio.h>
#include <sys/kbd.h>
#else
#include <unistd.h>
#include </sys/sparc/dev/kbio.h>
#include </sys/sparc/dev/kbd.h>
#endif
#else
#ifndef CSRG_BASED
#include <sundev/kbio.h>
#include <sundev/kbd.h>
#else
#include <machine/kbio.h>
#include <machine/kbd.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void         die(char*);
static void	    usage(void);
static int          kbd_fd;

int
main(argc, argv)
    int    argc;
    char** argv;
{
    int    code = 0, translate, direct = -1;
    char   led;
    int    click;

    if ((kbd_fd = open("/dev/kbd", O_RDONLY, 0)) < 0) {
	die("Couldn't open /dev/kbd");
    }
    argc--; argv++;
    if (argc-- && **argv == '-') {
	code = *(++*argv);
    } else {
	usage();
    }
    switch (code) {
      case 'a':
      case 'A':
	translate = TR_ASCII;
	direct = 0;
	break;
      case 'e':
      case 'E':
	translate = TR_EVENT;
	break;
      case 'n':
      case 'N':
	translate = TR_NONE;
	break;
      case 'u':
      case 'U':
	translate = TR_UNTRANS_EVENT;
	break;
      default:
	usage();
    }
#ifdef KIOCSLED
    led = 0;
    if (ioctl(kbd_fd, KIOCSLED, &led))
	die("Couldn't set LEDs");
#endif
#ifdef KIOCCMD
    click = KBD_CMD_NOCLICK;
    if (ioctl(kbd_fd, KIOCCMD, &click))
	die("Couldn't set click");
#endif
    if (ioctl(kbd_fd, KIOCTRANS, (caddr_t) &translate))
	die("Couldn't set translation");
    if (direct != -1 && ioctl(kbd_fd, KIOCSDIRECT, (caddr_t) &direct))
	die("Couldn't set redirect");
    return 0;
}

static void
die(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static void
usage(void)
{
    int             translate;

    if (ioctl(kbd_fd, KIOCGTRANS, (caddr_t) &translate)) {
	die("Couldn't inquire current translation");
     }
    fprintf(stderr, "kbd_mode {-a | -e | -n | -u }\n");
    fprintf(stderr, "\tfor ascii, encoded (normal) SunView events,\n");
    fprintf(stderr, " \tnon-encoded, or unencoded SunView events, resp.\n");
    fprintf(stderr, "Current mode is %s.\n",
		(   translate == 0 ?    "n (non-translated bytes)"      :
		 (  translate == 1 ?    "a (ascii bytes)"               :
		  ( translate == 2 ?    "e (encoded events)"            :
		  /* translate == 3 */  "u (unencoded events)"))));
    exit(1);
}


