
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/*
 * Utility functions required by libxf86_os. 
 */

_X_EXPORT pointer
Xalloc(unsigned long n)
{
    if (!n)
	n = 1;
    return malloc(n);
}

_X_EXPORT pointer
Xrealloc(pointer p, unsigned long n)
{
    if (!n)
	n = 1;
    return realloc(p, n);
}

_X_EXPORT pointer
Xcalloc(unsigned long n)
{
    pointer r;

    r = Xalloc(n);
    memset(r, 0, n);
    return r;
}

_X_EXPORT pointer
XNFalloc(unsigned long n)
{
    pointer r;

    r = Xalloc(n);
    if (!r)
	FatalError("XNFalloc failed\n");
    return r;
   
}

_X_EXPORT pointer
XNFrealloc(pointer p, unsigned long n)
{
    pointer r;

    r = Xrealloc(p, n);
    if (!r)
	FatalError("XNFrealloc failed\n");
    return r;
   
}

_X_EXPORT pointer
XNFcalloc(unsigned long n)
{
    pointer r;

    r = Xcalloc(n);
    if (!r)
	FatalError("XNFcalloc failed\n");
    return r;
   
}

_X_EXPORT void
Xfree(pointer p)
{
    free(p);
}

_X_EXPORT char *
Xstrdup(const char *s)
{
    char *sd;

    if (s == NULL)
	return NULL;

    sd = (char *)Xalloc(strlen(s) + 1);
    if (sd != NULL)
	strcpy(sd, s);
    return sd;
}

char *
XNFstrdup(const char *s)
{
    char *sd;
    size_t len;

    if (s == NULL)
	return NULL;
    
    len = strlen(s) + 1;
    sd = (char *)XNFalloc(len);
    strlcpy(sd, s, len);
    return sd;
}

