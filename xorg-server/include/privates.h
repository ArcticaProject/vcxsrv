/***********************************************************

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef PRIVATES_H
#define PRIVATES_H 1

#include "dix.h"
#include "resource.h"

/*****************************************************************
 * STUFF FOR PRIVATES
 *****************************************************************/

typedef int *DevPrivateKey;
struct _Private;
typedef struct _Private PrivateRec;

/*
 * Request pre-allocated private space for your driver/module.  This function
 * increases the amount of space allocated automatically when dixLookupPrivate
 * is called on a PrivateRec that does not yet have a value associated with
 * 'key'.
 *
 * This function will only increase the reserved size: if a size was previously
 * requested, then dixRequestPrivate causes later calls to dixLookupPrivate to
 * allocate the maximum of the old size and 'size'.  Requested sizes are reset
 * to 0 by dixResetPrivates, which is called before each server generation.
 *
 * If dixRequestPrivate is not called with a nonzero size for 'key', then the
 * module responsible for 'key' must manage the associated pointer itself with
 * dixSetPrivate.
 *
 * dixRequestPrivate returns FALSE if it cannot store the requested size.
 */
extern _X_EXPORT int
dixRequestPrivate(const DevPrivateKey key, unsigned size);

/*
 * Allocates space for an association of 'key' with a value in 'privates'.
 *
 * If a nonzero size was requested with dixRequestPrivate, then
 * dixAllocatePrivate also allocates the requested amount of memory and
 * associates the pointer to that memory with 'key' in 'privates'.  The
 * allocated memory is initialized to zero.  This memory can only be freed by
 * dixFreePrivates.
 *
 * If dixRequestPrivate was never called with a nonzero size, then
 * dixAllocatePrivate associates NULL with 'key' in 'privates'.
 *
 * dixAllocatePrivate returns a pointer to the value associated with 'key' in
 * 'privates', unless a memory allocation fails, in which case it returns NULL.
 */
extern _X_EXPORT pointer *
dixAllocatePrivate(PrivateRec **privates, const DevPrivateKey key);

/*
 * Look up a private pointer.
 *
 * If no value is currently associated with 'key' in 'privates', then
 * dixLookupPrivate calls dixAllocatePrivate and returns the resulting
 * associated value.
 *
 * dixLookupPrivate returns NULL if a memory allocation fails.
 */
extern _X_EXPORT pointer
dixLookupPrivate(PrivateRec **privates, const DevPrivateKey key);

/*
 * Look up the address of a private pointer.  If 'key' is not associated with a
 * value in 'privates', then dixLookupPrivateAddr calls dixAllocatePrivate and
 * returns a pointer to the resulting associated value.
 *
 * dixLookupPrivateAddr returns NULL if 'key' was not previously associated in
 * 'privates' and a memory allocation fails.
 */
extern _X_EXPORT pointer *
dixLookupPrivateAddr(PrivateRec **privates, const DevPrivateKey key);

/*
 * Associate 'val' with 'key' in 'privates' so that later calls to
 * dixLookupPrivate(privates, key) will return 'val'.
 *
 * dixSetPrivate returns FALSE if a memory allocation fails.
 */
extern _X_EXPORT int
dixSetPrivate(PrivateRec **privates, const DevPrivateKey key, pointer val);

/*
 * Register callbacks to be called on private allocation/freeing.
 * The calldata argument to the callbacks is a PrivateCallbackPtr.
 */
typedef struct _PrivateCallback {
    DevPrivateKey key;	/* private registration key */
    pointer *value;	/* address of private pointer */
} PrivateCallbackRec;

/*
 * Register a function to be called when dixAllocPrivate successfully associates
 * 'key' with a new PrivateRec.
 */
extern _X_EXPORT int
dixRegisterPrivateInitFunc(const DevPrivateKey key, 
			   CallbackProcPtr callback, pointer userdata);

/*
 * Register a function to be called when dixFreePrivates unassociates 'key' with
 * a PrivateRec.
 */
extern _X_EXPORT int
dixRegisterPrivateDeleteFunc(const DevPrivateKey key,
			     CallbackProcPtr callback, pointer userdata);

/*
 * Unassociates all keys from 'privates', calls the callbacks registered with
 * dixRegisterPrivateDeleteFunc, and frees all private data automatically
 * allocated via dixRequestPrivate.
 */
extern _X_EXPORT void
dixFreePrivates(PrivateRec *privates);

/*
 * Resets the privates subsystem.  dixResetPrivates is called from the main loop
 * before each server generation.  This function must only be called by main().
 */
extern _X_EXPORT int
dixResetPrivates(void);

/*
 * These next two functions are necessary because the position of
 * the devPrivates field varies by structure and calling code might
 * only know the resource type, not the structure definition.
 */

/*
 * Looks up the offset where the devPrivates field is located.
 * Returns -1 if no offset has been registered for the resource type.
 */
extern _X_EXPORT int
dixLookupPrivateOffset(RESTYPE type);

/*
 * Specifies the offset where the devPrivates field is located.
 * A negative value indicates no devPrivates field is available.
 */
extern _X_EXPORT int
dixRegisterPrivateOffset(RESTYPE type, int offset);

/*
 * Convenience macro for adding an offset to an object pointer
 * when making a call to one of the devPrivates functions
 */
#define DEVPRIV_AT(ptr, offset) ((PrivateRec **)((char *)ptr + offset))

#endif /* PRIVATES_H */
