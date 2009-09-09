/*
 * Copyright (c) 1997-2002 by The XFree86 Project, Inc.
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

/*
 * This file contains declarations for private XFree86 functions and variables,
 * and definitions of private macros.
 *
 * "private" means not available to video drivers.
 */

#ifndef _XF86PRIV_H
#define _XF86PRIV_H

#include <pciaccess.h>

#include "xf86Privstr.h"
#include "propertyst.h"
#include "input.h"

/*
 * Parameters set ONLY from the command line options
 * The global state of these things is held in xf86InfoRec (when appropriate).
 */
extern _X_EXPORT const char *xf86ConfigFile;
extern _X_EXPORT  Bool xf86AllowMouseOpenFail;
#ifdef XF86VIDMODE
extern _X_EXPORT  Bool xf86VidModeDisabled;
extern _X_EXPORT  Bool xf86VidModeAllowNonLocal;
#endif 
extern _X_EXPORT  Bool xf86fpFlag;
extern _X_EXPORT  Bool xf86sFlag;
extern _X_EXPORT  Bool xf86bsEnableFlag;
extern _X_EXPORT  Bool xf86bsDisableFlag;
extern _X_EXPORT  Bool xf86silkenMouseDisableFlag;
extern _X_EXPORT  Bool xf86xkbdirFlag;
#ifdef HAVE_ACPI
extern _X_EXPORT  Bool xf86acpiDisableFlag;
#endif
extern _X_EXPORT  char *xf86LayoutName;
extern _X_EXPORT  char *xf86ScreenName;
extern _X_EXPORT  char *xf86PointerName;
extern _X_EXPORT  char *xf86KeyboardName;
extern _X_EXPORT  int xf86FbBpp;
extern _X_EXPORT  int xf86Depth;
extern _X_EXPORT  Pix24Flags xf86Pix24;
extern _X_EXPORT  rgb xf86Weight;
extern _X_EXPORT  Bool xf86FlipPixels;
extern _X_EXPORT  Gamma xf86Gamma;
extern _X_EXPORT  char *xf86ServerName;
extern _X_EXPORT  struct pci_slot_match xf86IsolateDevice;

/* Other parameters */

extern _X_EXPORT  xf86InfoRec xf86Info;
extern _X_EXPORT  const char *xf86ModulePath;
extern _X_EXPORT  MessageType xf86ModPathFrom;
extern _X_EXPORT  const char *xf86LogFile;
extern _X_EXPORT  MessageType xf86LogFileFrom;
extern _X_EXPORT  Bool xf86LogFileWasOpened;
extern _X_EXPORT  serverLayoutRec xf86ConfigLayout;

extern _X_EXPORT  DriverPtr *xf86DriverList;
extern _X_EXPORT  int xf86NumDrivers;
extern _X_EXPORT  Bool xf86Resetting;
extern _X_EXPORT  Bool xf86Initialising;
extern _X_EXPORT  int xf86NumScreens;
extern _X_EXPORT  const char *xf86VisualNames[];
extern _X_EXPORT  int xf86Verbose;                 /* verbosity level */
extern _X_EXPORT  int xf86LogVerbose;		/* log file verbosity level */
extern _X_EXPORT  Bool xorgHWAccess;

extern _X_EXPORT  RootWinPropPtr *xf86RegisteredPropertiesTable;

#ifndef DEFAULT_VERBOSE
#define DEFAULT_VERBOSE		0
#endif
#ifndef DEFAULT_LOG_VERBOSE
#define DEFAULT_LOG_VERBOSE	3
#endif
#ifndef DEFAULT_DPI
#define DEFAULT_DPI		96
#endif

/* Function Prototypes */
#ifndef _NO_XF86_PROTOTYPES

/* xf86Bus.c */

extern _X_EXPORT void xf86BusProbe(void);
extern _X_EXPORT void xf86AccessInit(void);
extern _X_EXPORT void xf86AccessEnter(void);
extern _X_EXPORT void xf86AccessLeave(void);
extern _X_EXPORT void xf86EntityInit(void);

extern _X_EXPORT void xf86FindPrimaryDevice(void);
/* new RAC */
extern _X_EXPORT void xf86PostProbe(void);
extern _X_EXPORT void xf86ClearEntityListForScreen(int scrnIndex);
extern _X_EXPORT void xf86AddDevToEntity(int entityIndex, GDevPtr dev);
extern _X_EXPORT void xf86PostScreenInit(void);

/* xf86Config.c */

extern _X_EXPORT Bool xf86PathIsSafe(const char *path);

/* xf86DefaultModes */

extern _X_EXPORT const DisplayModeRec xf86DefaultModes[];
extern _X_EXPORT const int xf86NumDefaultModes;

/* xf86Configure.c */
extern _X_EXPORT void DoConfigure(void);

/* xf86ShowOpts.c */
extern _X_EXPORT void DoShowOptions(void);

/* xf86Events.c */

extern _X_EXPORT void xf86Wakeup(pointer blockData, int err, pointer pReadmask);
extern _X_HIDDEN int xf86SigWrapper(int signo);
extern _X_EXPORT void xf86HandlePMEvents(int fd, pointer data);
extern _X_EXPORT int (*xf86PMGetEventFromOs)(int fd,pmEvent *events,int num);
extern _X_EXPORT pmWait (*xf86PMConfirmEventToOs)(int fd,pmEvent event);

/* xf86Helper.c */
extern _X_EXPORT void xf86LogInit(void);
extern _X_EXPORT void xf86CloseLog(void);

/* xf86Init.c */
extern _X_EXPORT Bool xf86LoadModules(char **list, pointer *optlist);
extern _X_EXPORT int xf86SetVerbosity(int verb);
extern _X_EXPORT int xf86SetLogVerbosity(int verb);
extern _X_EXPORT Bool xf86CallDriverProbe( struct _DriverRec * drv, Bool detect_only );

/* xf86Xinput.c */
extern _X_EXPORT EventList *xf86Events;

#endif /* _NO_XF86_PROTOTYPES */


#endif /* _XF86PRIV_H */
