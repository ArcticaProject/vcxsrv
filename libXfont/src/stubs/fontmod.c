#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef LOADABLEFONTS
#include "stubs.h"
#include <X11/fonts/fontmod.h>

#ifdef __SUNPRO_C
#pragma weak FontModuleList
#endif

weak FontModule *FontModuleList;
#endif /* LOADABLEFONTS */
