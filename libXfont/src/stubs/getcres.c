#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak GetClientResolutions
#endif

weak FontResolutionPtr
GetClientResolutions(int *num)
{
  return (FontResolutionPtr) 0;
}
