/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"

#define SYM(ptr, name) { (void **) &(ptr), (name) }

typedef struct _xglHashFunc {
    xglHashTablePtr (*NewHashTable)	    (void);
    void	    (*DeleteHashTable)      (xglHashTablePtr	   pTable);
    void	    *(*HashLookup)	    (const xglHashTablePtr pTable,
					     unsigned int	   key);
    void	    (*HashInsert)	    (xglHashTablePtr	   pTable,
					     unsigned int	   key,
					     void		   *data);
    void	    (*HashRemove)	    (xglHashTablePtr	   pTable,
					     unsigned int	   key);
    unsigned int    (*HashFirstEntry)       (xglHashTablePtr	   pTable);
    unsigned int    (*HashNextEntry)	    (const xglHashTablePtr pTable,
					     unsigned int	   key);
    unsigned int    (*HashFindFreeKeyBlock) (xglHashTablePtr	   pTable,
					     unsigned int	   numKeys);
} xglHashFuncRec;

static xglHashFuncRec __hashFunc;

static void *hashHandle = 0;

Bool
xglLoadHashFuncs (void *handle)
{

#ifdef XGL_MODULAR
    xglSymbolRec sym[] = {
	SYM (__hashFunc.NewHashTable,	      "_mesa_NewHashTable"),
	SYM (__hashFunc.DeleteHashTable,      "_mesa_DeleteHashTable"),
	SYM (__hashFunc.HashLookup,	      "_mesa_HashLookup"),
	SYM (__hashFunc.HashInsert,	      "_mesa_HashInsert"),
	SYM (__hashFunc.HashRemove,	      "_mesa_HashRemove"),
	SYM (__hashFunc.HashFirstEntry,	      "_mesa_HashFirstEntry"),
	SYM (__hashFunc.HashNextEntry,	      "_mesa_HashNextEntry"),
	SYM (__hashFunc.HashFindFreeKeyBlock, "_mesa_HashFindFreeKeyBlock")
    };

    if (!xglLookupSymbols (handle, sym, sizeof (sym) / sizeof (sym[0])))
	return FALSE;

    hashHandle = handle;

    return TRUE;
#else
    return FALSE;
#endif

}

xglHashTablePtr
xglNewHashTable (void)
{
    if (!hashHandle)
	return 0;

    return (*__hashFunc.NewHashTable) ();
}

void
xglDeleteHashTable (xglHashTablePtr pTable)
{
    (*__hashFunc.DeleteHashTable) (pTable);
}

void *
xglHashLookup (const xglHashTablePtr pTable,
	       unsigned int	     key)
{
    return (*__hashFunc.HashLookup) (pTable, key);
}

void
xglHashInsert (xglHashTablePtr pTable,
	       unsigned int    key,
	       void	       *data)
{
    (*__hashFunc.HashInsert) (pTable, key, data);
}

void
xglHashRemove (xglHashTablePtr pTable,
	       unsigned int    key)
{
    (*__hashFunc.HashRemove) (pTable, key);
}

unsigned int
xglHashFirstEntry (xglHashTablePtr pTable)
{
    return (*__hashFunc.HashFirstEntry) (pTable);
}

unsigned int
xglHashNextEntry (const xglHashTablePtr pTable,
		  unsigned int		key)
{
    return (*__hashFunc.HashNextEntry) (pTable, key);
}

unsigned int
xglHashFindFreeKeyBlock (xglHashTablePtr pTable,
			 unsigned int	 numKeys)
{
    return (*__hashFunc.HashFindFreeKeyBlock) (pTable, numKeys);
}
