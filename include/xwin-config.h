/* include/xwin-config.h.  Generated from xwin-config.h.in by configure.  */
/*
 * xwin-config.h.in
 *
 * This file has all defines used in the xwin ddx
 *
 */
#include <dix-config.h>

/* Winsock networking */
#define HAS_WINSOCK

/* Cygwin has /dev/windows for signaling new win32 messages */
// let's should this for now #define HAS_DEVWINDOWS 1

/* Switch on debug messages */ 
/* #undef CYGDEBUG */
/* #undef CYGWINDOWING_DEBUG */
/* #undef CYGMULTIWINDOW_DEBUG */

/* Define to 1 if unsigned long is 64 bits. */
/* #undef _XSERVER64 */

/* Do we require our own snprintf? */
/* #undef NEED_SNPRINTF */

/* Vendor web address for support */
#define __VENDORDWEBSUPPORT__ "http://www.hc-consult.be/"

/* Default log location */
#define DEFAULT_LOGDIR "."

/* Location of system.XWinrc */
#define SYSCONFDIR "."

