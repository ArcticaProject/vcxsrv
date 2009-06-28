#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak XpClientIsBitmapClient
#pragma weak XpClientIsPrintClient
#endif

weak Bool
XpClientIsBitmapClient(ClientPtr client)
{
    return True;
}

weak Bool
XpClientIsPrintClient(ClientPtr client, FontPathElementPtr fpe)
{
    return False;
}
