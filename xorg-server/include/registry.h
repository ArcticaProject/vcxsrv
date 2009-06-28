/***********************************************************

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef DIX_REGISTRY_H
#define DIX_REGISTRY_H

/*
 * Result returned from any unsuccessful lookup
 */
#define XREGISTRY_UNKNOWN "<unknown>"

#ifdef XREGISTRY

#include "resource.h"
#include "extnsionst.h"

/* Internal string registry - for auditing, debugging, security, etc. */

/*
 * Registration functions.  The name string is not copied, so it must
 * not be a stack variable.
 */
void RegisterResourceName(RESTYPE type, char *name);
void RegisterExtensionNames(ExtensionEntry *ext);

/*
 * Lookup functions.  The returned string must not be modified or freed.
 */
const char *LookupMajorName(int major);
const char *LookupRequestName(int major, int minor);
const char *LookupEventName(int event);
const char *LookupErrorName(int error);
const char *LookupResourceName(RESTYPE rtype);

/*
 * Setup and teardown
 */
void dixResetRegistry(void);

#else /* XREGISTRY */

/* Define calls away when the registry is not being built. */

#define RegisterResourceName(a, b) { ; }
#define RegisterExtensionNames(a) { ; }

#define LookupMajorName(a) XREGISTRY_UNKNOWN
#define LookupRequestName(a, b) XREGISTRY_UNKNOWN
#define LookupEventName(a) XREGISTRY_UNKNOWN
#define LookupErrorName(a) XREGISTRY_UNKNOWN
#define LookupResourceName(a) XREGISTRY_UNKNOWN

#define dixResetRegistry() { ; }

#endif /* XREGISTRY */
#endif /* DIX_REGISTRY_H */
