/* Based on src/fontfile/gunzip.c
   written by Mark Eichin <eichin@kitten.gen.ma.us> September 1996.
   intended for inclusion in X11 public releases. */

/* Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */ 


#include "config.h"

#include <X11/fonts/fontmisc.h>
#include <X11/fonts/bufio.h>
#include <bzlib.h>

typedef struct _xzip_buf {
    bz_stream z;
    int zstat;
    BufChar b[BUFFILESIZE];
    BufChar b_in[BUFFILESIZE];
    BufFilePtr f;
} xzip_buf;

static int BufBzip2FileClose ( BufFilePtr f, int flag );
static int BufBzip2FileFill ( BufFilePtr f );
static int BufBzip2FileSkip ( BufFilePtr f, int c );

_X_HIDDEN BufFilePtr
BufFilePushBZIP2 (BufFilePtr f)
{
    xzip_buf *x;

    x = malloc (sizeof (xzip_buf));
    if (!x) return NULL;

    bzero(&(x->z), sizeof(bz_stream));
    x->f = f;

    x->zstat = BZ2_bzDecompressInit(&(x->z),
				    0,	/* verbosity: 0 silent, 4 max */
				    0);	/* 0: go faster, 1: use less memory */
    if (x->zstat != BZ_OK) {
	free(x);
	return NULL;
    }

    /* now that the history buffer is allocated, we provide the data buffer */
    x->z.next_out = (char *) x->b;
    x->z.avail_out = BUFFILESIZE;
    x->z.next_in = (char *) x->b_in;
    x->z.avail_in = 0;

    return BufFileCreate((char *)x,
			 BufBzip2FileFill,
			 NULL,
			 BufBzip2FileSkip,
			 BufBzip2FileClose);
}

static int 
BufBzip2FileClose(BufFilePtr f, int flag)
{
    xzip_buf *x = (xzip_buf *)f->private;
    BZ2_bzDecompressEnd (&(x->z));
    BufFileClose (x->f, flag);
    free (x);
    return 1;
}

/* here's the real work. 
   -- we need to put stuff in f.buffer, update f.left and f.bufp,
   then return the first byte (or BUFFILEEOF).
   -- to do this, we need to get stuff into avail_in, and next_in, 
   and call BZ2_bzDecompress appropriately.
   -- we may also need to add CRC maintenance - if BZ2_bzDecompress tells us
   BZ_STREAM_END, we then have 4bytes CRC and 4bytes length...
*/
static int 
BufBzip2FileFill (BufFilePtr f)
{
    xzip_buf *x = (xzip_buf *)f->private;

    /* we only get called when left == 0... */
    /* but just in case, deal */
    if (f->left >= 0) {
	f->left--;
	return *(f->bufp++);
    }
    /* did we run out last time? */
    switch (x->zstat) {
    case BZ_OK:
	break;
    case BZ_STREAM_END:
    case BZ_DATA_ERROR:
    case BZ_DATA_ERROR_MAGIC:
	f->left = 0;
	return BUFFILEEOF;
    default:
	return BUFFILEEOF;
    }
    /* now we work to consume what we can */
    /* let libbz2 know what we can handle */
    x->z.next_out = (char *) x->b;
    x->z.avail_out = BUFFILESIZE;

    /* and try to consume all of it */
    while (x->z.avail_out > 0) {
	/* if we don't have anything to work from... */
	if (x->z.avail_in == 0) {
	    /* ... fill the z buf from underlying file */
	    int i, c;
	    for (i = 0; i < sizeof(x->b_in); i++) {
		c = BufFileGet(x->f);
		if (c == BUFFILEEOF) break;
		x->b_in[i] = c;
	    }
	    x->z.avail_in += i;
	    x->z.next_in = (char *) x->b_in;
	}
	/* so now we have some output space and some input data */
	x->zstat = BZ2_bzDecompress(&(x->z));
	/* the inflation output happens in the f buffer directly... */
	if (x->zstat == BZ_STREAM_END) {
	    /* deal with EOF, crc */
	    break;
	}
	if (x->zstat != BZ_OK) {
	    break;
	}
    }
    f->bufp = x->b;
    f->left = BUFFILESIZE - x->z.avail_out;  

    if (f->left >= 0) {
	f->left--;
	return *(f->bufp++);
    } else {
	return BUFFILEEOF;
    }
}

/* there should be a BufCommonSkip... */
static int 
BufBzip2FileSkip (BufFilePtr f, int c)
{
    /* BufFileRawSkip returns the count unchanged.
       BufCompressedSkip returns 0.
       That means it probably never gets called... */
    int retval = c;
    while(c--) {
	int get = BufFileGet(f);
	if (get == BUFFILEEOF) return get;
    }
    return retval;
}
