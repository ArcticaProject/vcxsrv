#include "glapi/glapi.h"

unsigned long _GLAPI_EXPORT
_glthread_GetID(void)
{
   return u_thread_self();
}
