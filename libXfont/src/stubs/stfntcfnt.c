#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak StoreFontClientFont
#endif

weak int
StoreFontClientFont(FontPtr pfont, Font id)
{
    return 0;
}
