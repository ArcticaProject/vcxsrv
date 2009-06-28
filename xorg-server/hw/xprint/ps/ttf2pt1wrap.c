/*
 * Wrapper to add missing symbol to externally supplied code
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef Lynx
extern int optind;
extern char *optarg;
#endif

#include "ttf2pt1.c"
