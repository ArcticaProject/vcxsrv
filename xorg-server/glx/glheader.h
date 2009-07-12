#ifndef __GLHEADER_H__
#define __GLHEADER_H__

#define STDC_HEADERS 1

#include <X11/Xwinsock.h>
#include <X11/Xwindows.h>
#include <assert.h>
#define strcasecmp _stricmp

#undef MINSHORT
#undef MAXSHORT

#define MINSHORT -32768
#define MAXSHORT 32767

#define PUBLIC

#define GL_GLEXT_PROTOTYPES

#define DRI_DRIVER_PATH "/usr/lib/dri"

#endif

