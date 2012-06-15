/*
 * fontconfig/src/fcstr.c
 *
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

FcChar8 *
FcStrCopy (const FcChar8 *s)
{
    int     len;
    FcChar8 *r;

    if (!s)
	return 0;
    len = strlen ((char *) s) + 1;
    r = (FcChar8 *) malloc (len);
    if (!r)
	return 0;
    FcMemAlloc (FC_MEM_STRING, len);
    memcpy (r, s, len);
    return r;
}

FcChar8 *
FcStrPlus (const FcChar8 *s1, const FcChar8 *s2)
{
    int	    s1l = strlen ((char *) s1);
    int	    s2l = strlen ((char *) s2);
    int	    l = s1l + s2l + 1;
    FcChar8 *s = malloc (l);

    if (!s)
	return 0;
    FcMemAlloc (FC_MEM_STRING, l);
    memcpy (s, s1, s1l);
    memcpy (s + s1l, s2, s2l + 1);
    return s;
}

void
FcStrFree (FcChar8 *s)
{
    FcMemFree (FC_MEM_STRING, strlen ((char *) s) + 1);
    free (s);
}


#include "../fc-case/fccase.h"

#define FcCaseFoldUpperCount(cf) \
    ((cf)->method == FC_CASE_FOLD_FULL ? 1 : (cf)->count)

#define FC_STR_CANON_BUF_LEN	1024

typedef struct _FcCaseWalker {
    const FcChar8   *read;
    const FcChar8   *src;
    FcChar8	    utf8[FC_MAX_CASE_FOLD_CHARS + 1];
} FcCaseWalker;

static void
FcStrCaseWalkerInit (const FcChar8 *src, FcCaseWalker *w)
{
    w->src = src;
    w->read = 0;
}

static FcChar8
FcStrCaseWalkerLong (FcCaseWalker *w, FcChar8 r)
{
    FcChar32	ucs4;
    int		slen;
    int		len = strlen((char*)w->src);

    slen = FcUtf8ToUcs4 (w->src - 1, &ucs4, len + 1);
    if (slen <= 0)
	return r;
    if (FC_MIN_FOLD_CHAR <= ucs4 && ucs4 <= FC_MAX_FOLD_CHAR)
    {
	int min = 0;
	int max = FC_NUM_CASE_FOLD;

	while (min <= max)
	{
	    int		mid = (min + max) >> 1;
	    FcChar32    low = fcCaseFold[mid].upper;
	    FcChar32    high = low + FcCaseFoldUpperCount (&fcCaseFold[mid]);
	
	    if (high <= ucs4)
		min = mid + 1;
	    else if (ucs4 < low)
		max = mid - 1;
	    else
	    {
		const FcCaseFold    *fold = &fcCaseFold[mid];
		int		    dlen;
		
		switch (fold->method) {
		case  FC_CASE_FOLD_EVEN_ODD:
		    if ((ucs4 & 1) != (fold->upper & 1))
			return r;
		    /* fall through ... */
		default:
		    dlen = FcUcs4ToUtf8 (ucs4 + fold->offset, w->utf8);
		    break;
		case FC_CASE_FOLD_FULL:
		    dlen = fold->count;
		    memcpy (w->utf8, fcCaseFoldChars + fold->offset, dlen);
		    break;
		}
		
		/* consume rest of src utf-8 bytes */
		w->src += slen - 1;
		
		/* read from temp buffer */
		w->utf8[dlen] = '\0';
		w->read = w->utf8;
		return *w->read++;
	    }
	}
    }
    return r;
}

static FcChar8
FcStrCaseWalkerNext (FcCaseWalker *w)
{
    FcChar8	r;

    if (w->read)
    {
	if ((r = *w->read++))
	    return r;
	w->read = 0;
    }
    r = *w->src++;

    if ((r & 0xc0) == 0xc0)
	return FcStrCaseWalkerLong (w, r);
    if ('A' <= r && r <= 'Z')
        r = r - 'A' + 'a';
    return r;
}

static FcChar8
FcStrCaseWalkerNextIgnoreBlanks (FcCaseWalker *w)
{
    FcChar8	r;

    if (w->read)
    {
	if ((r = *w->read++))
	    return r;
	w->read = 0;
    }
    do
    {
	r = *w->src++;
    } while (r == ' ');

    if ((r & 0xc0) == 0xc0)
	return FcStrCaseWalkerLong (w, r);
    if ('A' <= r && r <= 'Z')
        r = r - 'A' + 'a';
    return r;
}

FcChar8 *
FcStrDowncase (const FcChar8 *s)
{
    FcCaseWalker    w;
    int		    len = 0;
    FcChar8	    *dst, *d;

    FcStrCaseWalkerInit (s, &w);
    while (FcStrCaseWalkerNext (&w))
	len++;
    d = dst = malloc (len + 1);
    if (!d)
	return 0;
    FcMemAlloc (FC_MEM_STRING, len + 1);
    FcStrCaseWalkerInit (s, &w);
    while ((*d++ = FcStrCaseWalkerNext (&w)));
    return dst;
}

int
FcStrCmpIgnoreCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker    w1, w2;
    FcChar8	    c1, c2;

    if (s1 == s2) return 0;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;)
    {
	c1 = FcStrCaseWalkerNext (&w1);
	c2 = FcStrCaseWalkerNext (&w2);
	if (!c1 || (c1 != c2))
	    break;
    }
    return (int) c1 - (int) c2;
}

int
FcStrCmpIgnoreBlanksAndCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker    w1, w2;
    FcChar8	    c1, c2;

    if (s1 == s2) return 0;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;)
    {
	c1 = FcStrCaseWalkerNextIgnoreBlanks (&w1);
	c2 = FcStrCaseWalkerNextIgnoreBlanks (&w2);
	if (!c1 || (c1 != c2))
	    break;
    }
    return (int) c1 - (int) c2;
}

int
FcStrCmp (const FcChar8 *s1, const FcChar8 *s2)
{
    FcChar8 c1, c2;

    if (s1 == s2)
	return 0;
    for (;;)
    {
	c1 = *s1++;
	c2 = *s2++;
	if (!c1 || c1 != c2)
	    break;
    }
    return (int) c1 - (int) c2;
}

#ifdef USE_REGEX
static FcBool
_FcStrRegexCmp (const FcChar8 *s, const FcChar8 *regex, int cflags, int eflags)
{
    int ret = -1;
    regex_t reg;

    if ((ret = regcomp (&reg, (const char *)regex, cflags)) != 0)
    {
	if (FcDebug () & FC_DBG_MATCHV)
	{
	    char buf[512];

	    regerror (ret, &reg, buf, 512);
	    printf("Regexp compile error: %s\n", buf);
	}
	return FcFalse;
    }
    ret = regexec (&reg, (const char *)s, 0, NULL, eflags);
    if (ret != 0)
    {
	if (FcDebug () & FC_DBG_MATCHV)
	{
	    char buf[512];

	    regerror (ret, &reg, buf, 512);
	    printf("Regexp exec error: %s\n", buf);
	}
    }
    regfree (&reg);

    return ret == 0 ? FcTrue : FcFalse;
}
#else
#  define _FcStrRegexCmp(_s_, _regex_, _cflags_, _eflags_)	(FcFalse)
#endif

FcBool
FcStrRegexCmp (const FcChar8 *s, const FcChar8 *regex)
{
	return _FcStrRegexCmp (s, regex, REG_EXTENDED | REG_NOSUB, 0);
}

FcBool
FcStrRegexCmpIgnoreCase (const FcChar8 *s, const FcChar8 *regex)
{
	return _FcStrRegexCmp (s, regex, REG_EXTENDED | REG_NOSUB | REG_ICASE, 0);
}

/*
 * Return a hash value for a string
 */

FcChar32
FcStrHashIgnoreCase (const FcChar8 *s)
{
    FcChar32	    h = 0;
    FcCaseWalker    w;
    FcChar8	    c;

    FcStrCaseWalkerInit (s, &w);
    while ((c = FcStrCaseWalkerNext (&w)))
	h = ((h << 3) ^ (h >> 3)) ^ c;
    return h;
}

/*
 * Is the head of s1 equal to s2?
 */

static FcBool
FcStrIsAtIgnoreBlanksAndCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker    w1, w2;
    FcChar8	    c1, c2;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;)
    {
	c1 = FcStrCaseWalkerNextIgnoreBlanks (&w1);
	c2 = FcStrCaseWalkerNextIgnoreBlanks (&w2);
	if (!c1 || (c1 != c2))
	    break;
    }
    return c1 == c2 || !c2;
}

/*
 * Does s1 contain an instance of s2 (ignoring blanks and case)?
 */

const FcChar8 *
FcStrContainsIgnoreBlanksAndCase (const FcChar8 *s1, const FcChar8 *s2)
{
    while (*s1)
    {
	if (FcStrIsAtIgnoreBlanksAndCase (s1, s2))
	    return s1;
	s1++;
    }
    return 0;
}

static FcBool
FcCharIsPunct (const FcChar8 c)
{
    if (c < '0')
	return FcTrue;
    if (c <= '9')
	return FcFalse;
    if (c < 'A')
	return FcTrue;
    if (c <= 'Z')
	return FcFalse;
    if (c < 'a')
	return FcTrue;
    if (c <= 'z')
	return FcFalse;
    if (c <= '~')
	return FcTrue;
    return FcFalse;
}

/*
 * Is the head of s1 equal to s2?
 */

static FcBool
FcStrIsAtIgnoreCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker    w1, w2;
    FcChar8	    c1, c2;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;)
    {
	c1 = FcStrCaseWalkerNext (&w1);
	c2 = FcStrCaseWalkerNext (&w2);
	if (!c1 || (c1 != c2))
	    break;
    }
    return c1 == c2 || !c2;
}

/*
 * Does s1 contain an instance of s2 (ignoring blanks and case)?
 */

const FcChar8 *
FcStrContainsIgnoreCase (const FcChar8 *s1, const FcChar8 *s2)
{
    while (*s1)
    {
	if (FcStrIsAtIgnoreCase (s1, s2))
	    return s1;
	s1++;
    }
    return 0;
}

/*
 * Does s1 contain an instance of s2 on a word boundary (ignoring case)?
 */

const FcChar8 *
FcStrContainsWord (const FcChar8 *s1, const FcChar8 *s2)
{
    FcBool  wordStart = FcTrue;
    int	    s1len = strlen ((char *) s1);
    int	    s2len = strlen ((char *) s2);
	
    while (s1len >= s2len)
    {
	if (wordStart &&
	    FcStrIsAtIgnoreCase (s1, s2) &&
	    (s1len == s2len || FcCharIsPunct (s1[s2len])))
	{
	    return s1;
	}
	wordStart = FcFalse;
	if (FcCharIsPunct (*s1))
	    wordStart = FcTrue;
	s1++;
	s1len--;
    }
    return 0;
}

const FcChar8 *
FcStrStrIgnoreCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker    w1, w2;
    FcChar8	    c1, c2;
    const FcChar8   *cur;

    if (!s1 || !s2)
	return 0;

    if (s1 == s2)
	return s1;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    c2 = FcStrCaseWalkerNext (&w2);

    for (;;)
    {
	cur = w1.src;
	c1 = FcStrCaseWalkerNext (&w1);
	if (!c1)
	    break;
	if (c1 == c2)
	{
	    FcCaseWalker    w1t = w1;
	    FcCaseWalker    w2t = w2;
	    FcChar8	    c1t, c2t;

	    for (;;)
	    {
		c1t = FcStrCaseWalkerNext (&w1t);
		c2t = FcStrCaseWalkerNext (&w2t);

		if (!c2t)
		    return cur;
		if (c2t != c1t)
		    break;
	    }
	}
    }
    return 0;
}

const FcChar8 *
FcStrStr (const FcChar8 *s1, const FcChar8 *s2)
{
    FcChar8 c1, c2;
    const FcChar8 * p = s1;
    const FcChar8 * b = s2;

    if (!s1 || !s2)
	return 0;

    if (s1 == s2)
	return s1;

again:
    c2 = *s2++;

    if (!c2)
	return 0;

    for (;;)
    {
	p = s1;
	c1 = *s1++;
	if (!c1 || c1 == c2)
	    break;
    }

    if (c1 != c2)
	return 0;

    for (;;)
    {
	c1 = *s1;
	c2 = *s2;
	if (c1 && c2 && c1 != c2)
	{
	    s1 = p + 1;
	    s2 = b;
	    goto again;
	}
	if (!c2)
	    return p;
	if (!c1)
	    return 0;
	++ s1;
	++ s2;
    }
    /* never reached. */
}

int
FcUtf8ToUcs4 (const FcChar8 *src_orig,
	      FcChar32	    *dst,
	      int	    len)
{
    const FcChar8   *src = src_orig;
    FcChar8	    s;
    int		    extra;
    FcChar32	    result;

    if (len == 0)
	return 0;

    s = *src++;
    len--;

    if (!(s & 0x80))
    {
	result = s;
	extra = 0;
    }
    else if (!(s & 0x40))
    {
	return -1;
    }
    else if (!(s & 0x20))
    {
	result = s & 0x1f;
	extra = 1;
    }
    else if (!(s & 0x10))
    {
	result = s & 0xf;
	extra = 2;
    }
    else if (!(s & 0x08))
    {
	result = s & 0x07;
	extra = 3;
    }
    else if (!(s & 0x04))
    {
	result = s & 0x03;
	extra = 4;
    }
    else if ( ! (s & 0x02))
    {
	result = s & 0x01;
	extra = 5;
    }
    else
    {
	return -1;
    }
    if (extra > len)
	return -1;

    while (extra--)
    {
	result <<= 6;
	s = *src++;
	
	if ((s & 0xc0) != 0x80)
	    return -1;
	
	result |= s & 0x3f;
    }
    *dst = result;
    return src - src_orig;
}

FcBool
FcUtf8Len (const FcChar8    *string,
	   int		    len,
	   int		    *nchar,
	   int		    *wchar)
{
    int		n;
    int		clen;
    FcChar32	c;
    FcChar32	max;

    n = 0;
    max = 0;
    while (len)
    {
	clen = FcUtf8ToUcs4 (string, &c, len);
	if (clen <= 0)	/* malformed UTF8 string */
	    return FcFalse;
	if (c > max)
	    max = c;
	string += clen;
	len -= clen;
	n++;
    }
    *nchar = n;
    if (max >= 0x10000)
	*wchar = 4;
    else if (max > 0x100)
	*wchar = 2;
    else
	*wchar = 1;
    return FcTrue;
}

int
FcUcs4ToUtf8 (FcChar32	ucs4,
	      FcChar8	dest[FC_UTF8_MAX_LEN])
{
    int	bits;
    FcChar8 *d = dest;

    if      (ucs4 <       0x80) {  *d++=  ucs4;                         bits= -6; }
    else if (ucs4 <      0x800) {  *d++= ((ucs4 >>  6) & 0x1F) | 0xC0;  bits=  0; }
    else if (ucs4 <    0x10000) {  *d++= ((ucs4 >> 12) & 0x0F) | 0xE0;  bits=  6; }
    else if (ucs4 <   0x200000) {  *d++= ((ucs4 >> 18) & 0x07) | 0xF0;  bits= 12; }
    else if (ucs4 <  0x4000000) {  *d++= ((ucs4 >> 24) & 0x03) | 0xF8;  bits= 18; }
    else if (ucs4 < 0x80000000) {  *d++= ((ucs4 >> 30) & 0x01) | 0xFC;  bits= 24; }
    else return 0;

    for ( ; bits >= 0; bits-= 6) {
	*d++= ((ucs4 >> bits) & 0x3F) | 0x80;
    }
    return d - dest;
}

#define GetUtf16(src,endian) \
    ((FcChar16) ((src)[endian == FcEndianBig ? 0 : 1] << 8) | \
     (FcChar16) ((src)[endian == FcEndianBig ? 1 : 0]))

int
FcUtf16ToUcs4 (const FcChar8	*src_orig,
	       FcEndian		endian,
	       FcChar32		*dst,
	       int		len)	/* in bytes */
{
    const FcChar8   *src = src_orig;
    FcChar16	    a, b;
    FcChar32	    result;

    if (len < 2)
	return 0;

    a = GetUtf16 (src, endian); src += 2; len -= 2;

    /*
     * Check for surrogate
     */
    if ((a & 0xfc00) == 0xd800)
    {
	if (len < 2)
	    return 0;
	b = GetUtf16 (src, endian); src += 2; len -= 2;
	/*
	 * Check for invalid surrogate sequence
	 */
	if ((b & 0xfc00) != 0xdc00)
	    return 0;
	result = ((((FcChar32) a & 0x3ff) << 10) |
		  ((FcChar32) b & 0x3ff)) + 0x10000;
    }
    else
	result = a;
    *dst = result;
    return src - src_orig;
}

FcBool
FcUtf16Len (const FcChar8   *string,
	    FcEndian	    endian,
	    int		    len,	/* in bytes */
	    int		    *nchar,
	    int		    *wchar)
{
    int		n;
    int		clen;
    FcChar32	c;
    FcChar32	max;

    n = 0;
    max = 0;
    while (len)
    {
	clen = FcUtf16ToUcs4 (string, endian, &c, len);
	if (clen <= 0)	/* malformed UTF8 string */
	    return FcFalse;
	if (c > max)
	    max = c;
	string += clen;
	len -= clen;
	n++;
    }
    *nchar = n;
    if (max >= 0x10000)
	*wchar = 4;
    else if (max > 0x100)
	*wchar = 2;
    else
	*wchar = 1;
    return FcTrue;
}

void
FcStrBufInit (FcStrBuf *buf, FcChar8 *init, int size)
{
    if (init)
    {
	buf->buf = init;
	buf->size = size;
    } else
    {
	buf->buf = buf->buf_static;
	buf->size = sizeof (buf->buf_static);
    }
    buf->allocated = FcFalse;
    buf->failed = FcFalse;
    buf->len = 0;
}

void
FcStrBufDestroy (FcStrBuf *buf)
{
    if (buf->allocated)
    {
	FcMemFree (FC_MEM_STRBUF, buf->size);
	free (buf->buf);
	FcStrBufInit (buf, 0, 0);
    }
}

FcChar8 *
FcStrBufDone (FcStrBuf *buf)
{
    FcChar8 *ret;

    if (buf->failed)
	ret = NULL;
    else
	ret = malloc (buf->len + 1);
    if (ret)
    {
	FcMemAlloc (FC_MEM_STRING, buf->len + 1);
	memcpy (ret, buf->buf, buf->len);
	ret[buf->len] = '\0';
    }
    FcStrBufDestroy (buf);
    return ret;
}

FcChar8 *
FcStrBufDoneStatic (FcStrBuf *buf)
{
    FcStrBufChar (buf, '\0');

    if (buf->failed)
	return NULL;

    return buf->buf;
}

FcBool
FcStrBufChar (FcStrBuf *buf, FcChar8 c)
{
    if (buf->len == buf->size)
    {
	FcChar8	    *new;
	int	    size;

	if (buf->failed)
	    return FcFalse;

	if (buf->allocated)
	{
	    size = buf->size * 2;
	    FcMemFree (FC_MEM_STRBUF, buf->size);
	    new = realloc (buf->buf, size);
	}
	else
	{
	    size = buf->size + 64;
	    new = malloc (size);
	    if (new)
	    {
		buf->allocated = FcTrue;
		memcpy (new, buf->buf, buf->len);
	    }
	}
	if (!new)
	{
	    buf->failed = FcTrue;
	    return FcFalse;
	}
	FcMemAlloc (FC_MEM_STRBUF, size);
	buf->size = size;
	buf->buf = new;
    }
    buf->buf[buf->len++] = c;
    return FcTrue;
}

FcBool
FcStrBufString (FcStrBuf *buf, const FcChar8 *s)
{
    FcChar8 c;
    while ((c = *s++))
	if (!FcStrBufChar (buf, c))
	    return FcFalse;
    return FcTrue;
}

FcBool
FcStrBufData (FcStrBuf *buf, const FcChar8 *s, int len)
{
    while (len-- > 0)
	if (!FcStrBufChar (buf, *s++))
	    return FcFalse;
    return FcTrue;
}

FcBool
FcStrUsesHome (const FcChar8 *s)
{
    return *s == '~';
}

FcChar8 *
FcStrCopyFilename (const FcChar8 *s)
{
    FcChar8 *new;

    if (*s == '~')
    {
	FcChar8	*home = FcConfigHome ();
	FcChar8	*full;
	int	size;
	if (!home)
	    return NULL;
	size = strlen ((char *) home) + strlen ((char *) s);
	full = (FcChar8 *) malloc (size);
	if (!full)
	    return NULL;
	strcpy ((char *) full, (char *) home);
	strcat ((char *) full, (char *) s + 1);
	new = FcStrCanonFilename (full);
	free (full);
    }
    else
	new = FcStrCanonFilename (s);

    return new;
}

FcChar8 *
FcStrLastSlash (const FcChar8  *path)
{
    FcChar8	    *slash;

    slash = (FcChar8 *) strrchr ((const char *) path, '/');
#ifdef _WIN32
    {
        FcChar8     *backslash;

	backslash = (FcChar8 *) strrchr ((const char *) path, '\\');
	if (!slash || (backslash && backslash > slash))
	    slash = backslash;
    }
#endif

    return slash;
}

FcChar8 *
FcStrDirname (const FcChar8 *file)
{
    FcChar8 *slash;
    FcChar8 *dir;

    slash = FcStrLastSlash (file);
    if (!slash)
	return FcStrCopy ((FcChar8 *) ".");
    dir = malloc ((slash - file) + 1);
    if (!dir)
	return 0;
    FcMemAlloc (FC_MEM_STRING, (slash - file) + 1);
    strncpy ((char *) dir, (const char *) file, slash - file);
    dir[slash - file] = '\0';
    return dir;
}

FcChar8 *
FcStrBasename (const FcChar8 *file)
{
    FcChar8 *slash;

    slash = FcStrLastSlash (file);
    if (!slash)
	return FcStrCopy (file);
    return FcStrCopy (slash + 1);
}

static FcChar8 *
FcStrCanonAbsoluteFilename (const FcChar8 *s)
{
    FcChar8 *file;
    FcChar8 *f;
    const FcChar8 *slash;
    int size;

    size = strlen ((char *) s) + 1;
    file = malloc (size);
    if (!file)
	return NULL;
    FcMemAlloc (FC_MEM_STRING, size);
    slash = NULL;
    f = file;
    for (;;) {
	if (*s == '/' || *s == '\0')
	{
	    if (slash)
	    {
		switch (s - slash) {
		case 1:
		    f -= 1;	/* squash // and trim final / from file */
		    break;
		case 2:
		    if (!strncmp ((char *) slash, "/.", 2))
		    {
			f -= 2;	/* trim /. from file */
		    }
		    break;
		case 3:
		    if (!strncmp ((char *) slash, "/..", 3))
		    {
			f -= 3;	/* trim /.. from file */
			while (f > file) {
			    if (*--f == '/')
				break;
			}
		    }
		    break;
		}
	    }
	    slash = s;
	}
	if (!(*f++ = *s++))
	    break;
    }
    return file;
}

#ifdef _WIN32
/*
 * Convert '\\' to '/' , remove double '/'
 */
static void
FcConvertDosPath (char *str)
{
  size_t len = strlen (str);
  char *p = str;
  char *dest = str;
  char *end = str + len;
  char last = 0;

  if (*p == '\\')
    {
      *p = '/';
      p++;
      dest++;
    }
  while (p < end)
    {
      if (*p == '\\')
	*p = '/';

      if (*p != '/'
	  || last != '/')
	{
	  *dest++ = *p;
	}

      last = *p;
      p++;
    }

  *dest = 0;
}
#endif

FcChar8 *
FcStrCanonFilename (const FcChar8 *s)
{
#ifdef _WIN32
    FcChar8 full[FC_MAX_FILE_LEN + 2];
    int size = GetFullPathName ((LPCSTR) s, sizeof (full) -1,
				(LPSTR) full, NULL);

    if (size == 0)
	perror ("GetFullPathName");

    FcConvertDosPath ((char *) full);
    return FcStrCanonAbsoluteFilename (full);
#else
    if (s[0] == '/')
	return FcStrCanonAbsoluteFilename (s);
    else
    {
	FcChar8	*full;
	FcChar8 *file;

	FcChar8	cwd[FC_MAX_FILE_LEN + 2];
	if (getcwd ((char *) cwd, FC_MAX_FILE_LEN) == NULL)
	    return NULL;
	strcat ((char *) cwd, "/");
	full = FcStrPlus (cwd, s);
	file = FcStrCanonAbsoluteFilename (full);
	FcStrFree (full);
	return file;
    }
#endif
}


FcStrSet *
FcStrSetCreate (void)
{
    FcStrSet	*set = malloc (sizeof (FcStrSet));
    if (!set)
	return 0;
    FcMemAlloc (FC_MEM_STRSET, sizeof (FcStrSet));
    set->ref = 1;
    set->num = 0;
    set->size = 0;
    set->strs = 0;
    return set;
}

static FcBool
_FcStrSetAppend (FcStrSet *set, FcChar8 *s)
{
    if (FcStrSetMember (set, s))
    {
	FcStrFree (s);
	return FcTrue;
    }
    if (set->num == set->size)
    {
	FcChar8	**strs = malloc ((set->size + 2) * sizeof (FcChar8 *));

	if (!strs)
	    return FcFalse;
	FcMemAlloc (FC_MEM_STRSET, (set->size + 2) * sizeof (FcChar8 *));
	if (set->num)
	    memcpy (strs, set->strs, set->num * sizeof (FcChar8 *));
	if (set->strs)
	{
	    FcMemFree (FC_MEM_STRSET, (set->size + 1) * sizeof (FcChar8 *));
	    free (set->strs);
	}
	set->size = set->size + 1;
	set->strs = strs;
    }
    set->strs[set->num++] = s;
    set->strs[set->num] = 0;
    return FcTrue;
}

FcBool
FcStrSetMember (FcStrSet *set, const FcChar8 *s)
{
    int	i;

    for (i = 0; i < set->num; i++)
	if (!FcStrCmp (set->strs[i], s))
	    return FcTrue;
    return FcFalse;
}

FcBool
FcStrSetEqual (FcStrSet *sa, FcStrSet *sb)
{
    int	i;
    if (sa->num != sb->num)
	return FcFalse;
    for (i = 0; i < sa->num; i++)
	if (!FcStrSetMember (sb, sa->strs[i]))
	    return FcFalse;
    return FcTrue;
}

FcBool
FcStrSetAdd (FcStrSet *set, const FcChar8 *s)
{
    FcChar8 *new = FcStrCopy (s);
    if (!new)
	return FcFalse;
    if (!_FcStrSetAppend (set, new))
    {
	FcStrFree (new);
	return FcFalse;
    }
    return FcTrue;
}

FcBool
FcStrSetAddFilename (FcStrSet *set, const FcChar8 *s)
{
    FcChar8 *new = FcStrCopyFilename (s);
    if (!new)
	return FcFalse;
    if (!_FcStrSetAppend (set, new))
    {
	FcStrFree (new);
	return FcFalse;
    }
    return FcTrue;
}

FcBool
FcStrSetAddLangs (FcStrSet *strs, const char *languages)
{
    const char *p = languages, *next;
    FcChar8 lang[128] = {0}, *normalized_lang;
    size_t len;
    FcBool ret = FcFalse;

    if (!languages)
	return FcFalse;

    while ((next = strchr (p, ':')))
    {
	len = next - p;
	len = FC_MIN (len, 128);
	strncpy ((char *) lang, p, len);
	lang[len] = 0;
	/* ignore an empty item */
	if (*lang)
	{
	    normalized_lang = FcLangNormalize ((const FcChar8 *) lang);
	    if (normalized_lang)
	    {
		FcStrSetAdd (strs, normalized_lang);
		free (normalized_lang);
		ret = FcTrue;
	    }
	}
	p = next + 1;
    }
    if (*p)
    {
	normalized_lang = FcLangNormalize ((const FcChar8 *) p);
	if (normalized_lang)
	{
	    FcStrSetAdd (strs, normalized_lang);
	    free (normalized_lang);
	    ret = FcTrue;
	}
    }

    return ret;
}

FcBool
FcStrSetDel (FcStrSet *set, const FcChar8 *s)
{
    int	i;

    for (i = 0; i < set->num; i++)
	if (!FcStrCmp (set->strs[i], s))
	{
	    FcStrFree (set->strs[i]);
	    /*
	     * copy remaining string pointers and trailing
	     * NULL
	     */
	    memmove (&set->strs[i], &set->strs[i+1],
		     (set->num - i) * sizeof (FcChar8 *));
	    set->num--;
	    return FcTrue;
	}
    return FcFalse;
}

void
FcStrSetDestroy (FcStrSet *set)
{
    if (--set->ref == 0)
    {
	int	i;

	for (i = 0; i < set->num; i++)
	    FcStrFree (set->strs[i]);
	if (set->strs)
	{
	    FcMemFree (FC_MEM_STRSET, (set->size + 1) * sizeof (FcChar8 *));
	    free (set->strs);
	}
	FcMemFree (FC_MEM_STRSET, sizeof (FcStrSet));
	free (set);
    }
}

FcStrList *
FcStrListCreate (FcStrSet *set)
{
    FcStrList	*list;

    list = malloc (sizeof (FcStrList));
    if (!list)
	return 0;
    FcMemAlloc (FC_MEM_STRLIST, sizeof (FcStrList));
    list->set = set;
    set->ref++;
    list->n = 0;
    return list;
}

FcChar8 *
FcStrListNext (FcStrList *list)
{
    if (list->n >= list->set->num)
	return 0;
    return list->set->strs[list->n++];
}

void
FcStrListDone (FcStrList *list)
{
    FcStrSetDestroy (list->set);
    FcMemFree (FC_MEM_STRLIST, sizeof (FcStrList));
    free (list);
}

#define __fcstr__
#include "fcaliastail.h"
#undef __fcstr__
