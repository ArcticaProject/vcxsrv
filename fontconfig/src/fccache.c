/*
 * Copyright © 2000 Keith Packard
 * Copyright © 2005 Patrick Lam
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "fcint.h"
#include "fcarch.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
#  include <unistd.h>
#  include <sys/mman.h>
#elif defined(_WIN32)
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif


struct MD5Context {
        FcChar32 buf[4];
        FcChar32 bits[2];
        unsigned char in[64];
};

static void MD5Init(struct MD5Context *ctx);
static void MD5Update(struct MD5Context *ctx, const unsigned char *buf, unsigned len);
static void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
static void MD5Transform(FcChar32 buf[4], FcChar32 in[16]);

#define CACHEBASE_LEN (1 + 32 + 1 + sizeof (FC_ARCHITECTURE) + sizeof (FC_CACHE_SUFFIX))

#ifdef _WIN32

#include <windows.h>

#ifdef __GNUC__
typedef long long INT64;
#define EPOCH_OFFSET 11644473600ll
#else
#define EPOCH_OFFSET 11644473600i64
typedef __int64 INT64;
#endif

/* Workaround for problems in the stat() in the Microsoft C library:
 *
 * 1) stat() uses FindFirstFile() to get the file
 * attributes. Unfortunately this API doesn't return correct values
 * for modification time of a directory until some time after a file
 * or subdirectory has been added to the directory. (This causes
 * run-test.sh to fail, for instance.) GetFileAttributesEx() is
 * better, it returns the updated timestamp right away.
 *
 * 2) stat() does some strange things related to backward
 * compatibility with the local time timestamps on FAT volumes and
 * daylight saving time. This causes problems after the switches
 * to/from daylight saving time. See
 * http://bugzilla.gnome.org/show_bug.cgi?id=154968 , especially
 * comment #30, and http://www.codeproject.com/datetime/dstbugs.asp .
 * We don't need any of that, FAT and Win9x are as good as dead. So
 * just use the UTC timestamps from NTFS, converted to the Unix epoch.
 */

int
FcStat (const FcChar8 *file, struct stat *statb)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    char full_path_name[MAX_PATH];
    char *basename;
    DWORD rc;

    if (!GetFileAttributesEx (file, GetFileExInfoStandard, &wfad))
	return -1;

    statb->st_dev = 0;

    /* Calculate a pseudo inode number as a hash of the full path name.
     * Call GetLongPathName() to get the spelling of the path name as it
     * is on disk.
     */
    rc = GetFullPathName (file, sizeof (full_path_name), full_path_name, &basename);
    if (rc == 0 || rc > sizeof (full_path_name))
	return -1;

    rc = GetLongPathName (full_path_name, full_path_name, sizeof (full_path_name));
    statb->st_ino = FcStringHash (full_path_name);

    statb->st_mode = _S_IREAD | _S_IWRITE;
    statb->st_mode |= (statb->st_mode >> 3) | (statb->st_mode >> 6);

    if (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	statb->st_mode |= _S_IFDIR;
    else
	statb->st_mode |= _S_IFREG;

    statb->st_nlink = 1;
    statb->st_uid = statb->st_gid = 0;
    statb->st_rdev = 0;

    if (wfad.nFileSizeHigh > 0)
	return -1;
    statb->st_size = wfad.nFileSizeLow;

    statb->st_atime = (*(INT64 *)&wfad.ftLastAccessTime)/10000000 - EPOCH_OFFSET;
    statb->st_mtime = (*(INT64 *)&wfad.ftLastWriteTime)/10000000 - EPOCH_OFFSET;
    statb->st_ctime = statb->st_mtime;

    return 0;
}

#else

int
FcStat (const FcChar8 *file, struct stat *statb)
{
  return stat ((char *) file, statb);
}

#endif

static const char bin2hex[] = { '0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'a', 'b',
				'c', 'd', 'e', 'f' };

static FcChar8 *
FcDirCacheBasename (const FcChar8 * dir, FcChar8 cache_base[CACHEBASE_LEN])
{
    unsigned char 	hash[16];
    FcChar8		*hex_hash;
    int			cnt;
    struct MD5Context 	ctx;

    MD5Init (&ctx);
    MD5Update (&ctx, (const unsigned char *)dir, strlen ((const char *) dir));

    MD5Final (hash, &ctx);

    cache_base[0] = '/';
    hex_hash = cache_base + 1;
    for (cnt = 0; cnt < 16; ++cnt)
    {
	hex_hash[2*cnt  ] = bin2hex[hash[cnt] >> 4];
	hex_hash[2*cnt+1] = bin2hex[hash[cnt] & 0xf];
    }
    hex_hash[2*cnt] = 0;
    strcat ((char *) cache_base, "-" FC_ARCHITECTURE FC_CACHE_SUFFIX);

    return cache_base;
}

FcBool
FcDirCacheUnlink (const FcChar8 *dir, FcConfig *config)
{
    FcChar8	*cache_hashed = NULL;
    FcChar8	cache_base[CACHEBASE_LEN];
    FcStrList	*list;
    FcChar8	*cache_dir;

    FcDirCacheBasename (dir, cache_base);

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
        return FcFalse;
	
    while ((cache_dir = FcStrListNext (list)))
    {
        cache_hashed = FcStrPlus (cache_dir, cache_base);
        if (!cache_hashed)
	    break;
	(void) unlink ((char *) cache_hashed);
	FcStrFree (cache_hashed);
    }
    FcStrListDone (list);
    /* return FcFalse if something went wrong */
    if (cache_dir)
	return FcFalse;
    return FcTrue;
}

static int
FcDirCacheOpenFile (const FcChar8 *cache_file, struct stat *file_stat)
{
    int	fd;

#ifdef _WIN32
    if (FcStat (cache_file, file_stat) < 0)
        return -1;
#endif
    fd = open((char *) cache_file, O_RDONLY | O_BINARY);
    if (fd < 0)
	return fd;
#ifndef _WIN32
    if (fstat (fd, file_stat) < 0)
    {
	close (fd);
	return -1;
    }
#endif
    return fd;
}

/*
 * Look for a cache file for the specified dir. Attempt
 * to use each one we find, stopping when the callback
 * indicates success
 */
static FcBool
FcDirCacheProcess (FcConfig *config, const FcChar8 *dir,
		   FcBool (*callback) (int fd, struct stat *fd_stat,
				       struct stat *dir_stat, void *closure),
		   void *closure, FcChar8 **cache_file_ret)
{
    int		fd = -1;
    FcChar8	cache_base[CACHEBASE_LEN];
    FcStrList	*list;
    FcChar8	*cache_dir;
    struct stat file_stat, dir_stat;
    FcBool	ret = FcFalse;

    if (FcStat (dir, &dir_stat) < 0)
        return FcFalse;

    FcDirCacheBasename (dir, cache_base);

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
        return FcFalse;
	
    while ((cache_dir = FcStrListNext (list)))
    {
        FcChar8	*cache_hashed = FcStrPlus (cache_dir, cache_base);
        if (!cache_hashed)
	    break;
        fd = FcDirCacheOpenFile (cache_hashed, &file_stat);
        if (fd >= 0) {
	    ret = (*callback) (fd, &file_stat, &dir_stat, closure);
	    close (fd);
	    if (ret)
	    {
		if (cache_file_ret)
		    *cache_file_ret = cache_hashed;
		else
		    FcStrFree (cache_hashed);
		break;
	    }
	}
    	FcStrFree (cache_hashed);
    }
    FcStrListDone (list);

    return ret;
}

#define FC_CACHE_MIN_MMAP   1024

/*
 * Skip list element, make sure the 'next' pointer is the last thing
 * in the structure, it will be allocated large enough to hold all
 * of the necessary pointers
 */

typedef struct _FcCacheSkip FcCacheSkip;

struct _FcCacheSkip {
    FcCache	    *cache;
    int		    ref;
    intptr_t	    size;
    dev_t	    cache_dev;
    ino_t	    cache_ino;
    time_t	    cache_mtime;
    FcCacheSkip	    *next[1];
};

/*
 * The head of the skip list; pointers for every possible level
 * in the skip list, plus the largest level in the list
 */

#define FC_CACHE_MAX_LEVEL  16

static FcCacheSkip	*fcCacheChains[FC_CACHE_MAX_LEVEL];
static int		fcCacheMaxLevel;


static int32_t
FcRandom(void)
{
    int32_t result;

#if HAVE_RANDOM_R
    static struct random_data fcrandbuf;
    static char statebuf[256];
    static FcBool initialized = FcFalse;

    if (initialized != FcTrue)
    {
	initstate_r(time(NULL), statebuf, 256, &fcrandbuf);
	initialized = FcTrue;
    }

    random_r(&fcrandbuf, &result);
#elif HAVE_RANDOM
    static char statebuf[256];
    char *state;
    static FcBool initialized = FcFalse;

    if (initialized != FcTrue)
    {
	state = initstate(time(NULL), statebuf, 256);
	initialized = FcTrue;
    }
    else
	state = setstate(statebuf);

    result = random();

    setstate(state);
#elif HAVE_LRAND48
    result = lrand48();
#elif HAVE_RAND_R
    static unsigned int seed = time(NULL);

    result = rand_r(&seed);
#elif HAVE_RAND
    static FcBool initialized = FcFalse;

    if (initialized != FcTrue)
    {
	srand(time(NULL));
	initialized = FcTrue;
    }
    result = rand();
#else
# error no random number generator function available.
#endif

    return result;
}

/*
 * Generate a random level number, distributed
 * so that each level is 1/4 as likely as the one before
 *
 * Note that level numbers run 1 <= level <= MAX_LEVEL
 */
static int
random_level (void)
{
    /* tricky bit -- each bit is '1' 75% of the time */
    long int	bits = FcRandom () | FcRandom ();
    int	level = 0;

    while (++level < FC_CACHE_MAX_LEVEL)
    {
	if (bits & 1)
	    break;
	bits >>= 1;
    }
    return level;
}

/*
 * Insert cache into the list
 */
static FcBool
FcCacheInsert (FcCache *cache, struct stat *cache_stat)
{
    FcCacheSkip    **update[FC_CACHE_MAX_LEVEL];
    FcCacheSkip    *s, **next;
    int		    i, level;

    /*
     * Find links along each chain
     */
    next = fcCacheChains;
    for (i = fcCacheMaxLevel; --i >= 0; )
    {
	for (; (s = next[i]); next = s->next)
	    if (s->cache > cache)
		break;
        update[i] = &next[i];
    }

    /*
     * Create new list element
     */
    level = random_level ();
    if (level > fcCacheMaxLevel)
    {
	level = fcCacheMaxLevel + 1;
	update[fcCacheMaxLevel] = &fcCacheChains[fcCacheMaxLevel];
	fcCacheMaxLevel = level;
    }

    s = malloc (sizeof (FcCacheSkip) + (level - 1) * sizeof (FcCacheSkip *));
    if (!s)
	return FcFalse;

    s->cache = cache;
    s->size = cache->size;
    s->ref = 1;
    if (cache_stat)
    {
	s->cache_dev = cache_stat->st_dev;
	s->cache_ino = cache_stat->st_ino;
	s->cache_mtime = cache_stat->st_mtime;
    }
    else
    {
	s->cache_dev = 0;
	s->cache_ino = 0;
	s->cache_mtime = 0;
    }

    /*
     * Insert into all fcCacheChains
     */
    for (i = 0; i < level; i++)
    {
	s->next[i] = *update[i];
	*update[i] = s;
    }
    return FcTrue;
}

static FcCacheSkip *
FcCacheFindByAddr (void *object)
{
    int	    i;
    FcCacheSkip    **next = fcCacheChains;
    FcCacheSkip    *s;

    /*
     * Walk chain pointers one level at a time
     */
    for (i = fcCacheMaxLevel; --i >= 0;)
	while (next[i] && (char *) object >= ((char *) next[i]->cache + next[i]->size))
	    next = next[i]->next;
    /*
     * Here we are
     */
    s = next[0];
    if (s && (char *) object < ((char *) s->cache + s->size))
	return s;
    return NULL;
}

static void
FcCacheRemove (FcCache *cache)
{
    FcCacheSkip	    **update[FC_CACHE_MAX_LEVEL];
    FcCacheSkip	    *s, **next;
    int		    i;

    /*
     * Find links along each chain
     */
    next = fcCacheChains;
    for (i = fcCacheMaxLevel; --i >= 0; )
    {
	for (; (s = next[i]); next = s->next)
	    if (s->cache >= cache)
		break;
        update[i] = &next[i];
    }
    s = next[0];
    for (i = 0; i < fcCacheMaxLevel && *update[i] == s; i++)
	*update[i] = s->next[i];
    while (fcCacheMaxLevel > 0 && fcCacheChains[fcCacheMaxLevel - 1] == NULL)
	fcCacheMaxLevel--;
    free (s);
}

static FcCache *
FcCacheFindByStat (struct stat *cache_stat)
{
    FcCacheSkip	    *s;

    for (s = fcCacheChains[0]; s; s = s->next[0])
	if (s->cache_dev == cache_stat->st_dev &&
	    s->cache_ino == cache_stat->st_ino &&
	    s->cache_mtime == cache_stat->st_mtime)
	{
	    s->ref++;
	    return s->cache;
	}
    return NULL;
}

static void
FcDirCacheDispose (FcCache *cache)
{
    switch (cache->magic) {
    case FC_CACHE_MAGIC_ALLOC:
	free (cache);
	break;
    case FC_CACHE_MAGIC_MMAP:
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	munmap (cache, cache->size);
#elif defined(_WIN32)
	UnmapViewOfFile (cache);
#endif
	break;
    }
    FcCacheRemove (cache);
}

void
FcCacheObjectReference (void *object)
{
    FcCacheSkip *skip = FcCacheFindByAddr (object);

    if (skip)
	skip->ref++;
}

void
FcCacheObjectDereference (void *object)
{
    FcCacheSkip	*skip = FcCacheFindByAddr (object);

    if (skip)
    {
	skip->ref--;
	if (skip->ref <= 0)
	    FcDirCacheDispose (skip->cache);
    }
}

void
FcCacheFini (void)
{
    int		    i;

    for (i = 0; i < FC_CACHE_MAX_LEVEL; i++)
	assert (fcCacheChains[i] == NULL);
    assert (fcCacheMaxLevel == 0);
}

static FcBool
FcCacheTimeValid (FcCache *cache, struct stat *dir_stat)
{
    struct stat	dir_static;

    if (!dir_stat)
    {
	if (FcStat (FcCacheDir (cache), &dir_static) < 0)
	    return FcFalse;
	dir_stat = &dir_static;
    }
    if (FcDebug () & FC_DBG_CACHE)
	printf ("FcCacheTimeValid dir \"%s\" cache time %d dir time %d\n",
		FcCacheDir (cache), cache->mtime, (int) dir_stat->st_mtime);
    return cache->mtime == (int) dir_stat->st_mtime;
}

/*
 * Map a cache file into memory
 */
static FcCache *
FcDirCacheMapFd (int fd, struct stat *fd_stat, struct stat *dir_stat)
{
    FcCache	*cache;
    FcBool	allocated = FcFalse;

    if (fd_stat->st_size < sizeof (FcCache))
	return NULL;
    cache = FcCacheFindByStat (fd_stat);
    if (cache)
    {
	if (FcCacheTimeValid (cache, dir_stat))
	    return cache;
	FcDirCacheUnload (cache);
	cache = NULL;
    }

    /*
     * Lage cache files are mmap'ed, smaller cache files are read. This
     * balances the system cost of mmap against per-process memory usage.
     */
    if (fd_stat->st_size >= FC_CACHE_MIN_MMAP)
    {
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	cache = mmap (0, fd_stat->st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (cache == MAP_FAILED)
	    cache = NULL;
#elif defined(_WIN32)
	{
	    HANDLE hFileMap;

	    cache = NULL;
	    hFileMap = CreateFileMapping((HANDLE) _get_osfhandle(fd), NULL,
					 PAGE_READONLY, 0, 0, NULL);
	    if (hFileMap != NULL)
	    {
		cache = MapViewOfFile (hFileMap, FILE_MAP_READ, 0, 0,
				       fd_stat->st_size);
		CloseHandle (hFileMap);
	    }
	}
#endif
    }
    if (!cache)
    {
	cache = malloc (fd_stat->st_size);
	if (!cache)
	    return NULL;

	if (read (fd, cache, fd_stat->st_size) != fd_stat->st_size)
	{
	    free (cache);
	    return NULL;
	}
	allocated = FcTrue;
    }
    if (cache->magic != FC_CACHE_MAGIC_MMAP ||
	cache->version < FC_CACHE_CONTENT_VERSION ||
	cache->size != fd_stat->st_size ||
	!FcCacheTimeValid (cache, dir_stat) ||
	!FcCacheInsert (cache, fd_stat))
    {
	if (allocated)
	    free (cache);
	else
	{
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	    munmap (cache, fd_stat->st_size);
#elif defined(_WIN32)
	    UnmapViewOfFile (cache);
#endif
	}
	return NULL;
    }

    /* Mark allocated caches so they're freed rather than unmapped */
    if (allocated)
	cache->magic = FC_CACHE_MAGIC_ALLOC;
	
    return cache;
}

void
FcDirCacheReference (FcCache *cache, int nref)
{
    FcCacheSkip *skip = FcCacheFindByAddr (cache);

    if (skip)
	skip->ref += nref;
}

void
FcDirCacheUnload (FcCache *cache)
{
    FcCacheObjectDereference (cache);
}

static FcBool
FcDirCacheMapHelper (int fd, struct stat *fd_stat, struct stat *dir_stat, void *closure)
{
    FcCache *cache = FcDirCacheMapFd (fd, fd_stat, dir_stat);

    if (!cache)
	return FcFalse;
    *((FcCache **) closure) = cache;
    return FcTrue;
}

FcCache *
FcDirCacheLoad (const FcChar8 *dir, FcConfig *config, FcChar8 **cache_file)
{
    FcCache *cache = NULL;

    if (!FcDirCacheProcess (config, dir,
			    FcDirCacheMapHelper,
			    &cache, cache_file))
	return NULL;
    return cache;
}

FcCache *
FcDirCacheLoadFile (const FcChar8 *cache_file, struct stat *file_stat)
{
    int	fd;
    FcCache *cache;
    struct stat	my_file_stat;

    if (!file_stat)
	file_stat = &my_file_stat;
    fd = FcDirCacheOpenFile (cache_file, file_stat);
    if (fd < 0)
	return NULL;
    cache = FcDirCacheMapFd (fd, file_stat, NULL);
    close (fd);
    return cache;
}

/*
 * Validate a cache file by reading the header and checking
 * the magic number and the size field
 */
static FcBool
FcDirCacheValidateHelper (int fd, struct stat *fd_stat, struct stat *dir_stat, void *closure)
{
    FcBool  ret = FcTrue;
    FcCache	c;

    if (read (fd, &c, sizeof (FcCache)) != sizeof (FcCache))
	ret = FcFalse;
    else if (c.magic != FC_CACHE_MAGIC_MMAP)
	ret = FcFalse;
    else if (c.version < FC_CACHE_CONTENT_VERSION)
	ret = FcFalse;
    else if (fd_stat->st_size != c.size)
	ret = FcFalse;
    else if (c.mtime != (int) dir_stat->st_mtime)
	ret = FcFalse;
    return ret;
}

static FcBool
FcDirCacheValidConfig (const FcChar8 *dir, FcConfig *config)
{
    return FcDirCacheProcess (config, dir,
			      FcDirCacheValidateHelper,
			      NULL, NULL);
}

FcBool
FcDirCacheValid (const FcChar8 *dir)
{
    FcConfig	*config;

    config = FcConfigGetCurrent ();
    if (!config)
        return FcFalse;

    return FcDirCacheValidConfig (dir, config);
}

/*
 * Build a cache structure from the given contents
 */
FcCache *
FcDirCacheBuild (FcFontSet *set, const FcChar8 *dir, struct stat *dir_stat, FcStrSet *dirs)
{
    FcSerialize	*serialize = FcSerializeCreate ();
    FcCache *cache;
    int i;
    FcChar8	*dir_serialize;
    intptr_t	*dirs_serialize;
    FcFontSet	*set_serialize;

    if (!serialize)
	return NULL;
    /*
     * Space for cache structure
     */
    FcSerializeReserve (serialize, sizeof (FcCache));
    /*
     * Directory name
     */
    if (!FcStrSerializeAlloc (serialize, dir))
	goto bail1;
    /*
     * Subdirs
     */
    FcSerializeAlloc (serialize, dirs, dirs->num * sizeof (FcChar8 *));
    for (i = 0; i < dirs->num; i++)
	if (!FcStrSerializeAlloc (serialize, dirs->strs[i]))
	    goto bail1;

    /*
     * Patterns
     */
    if (!FcFontSetSerializeAlloc (serialize, set))
	goto bail1;

    /* Serialize layout complete. Now allocate space and fill it */
    cache = malloc (serialize->size);
    if (!cache)
	goto bail1;
    /* shut up valgrind */
    memset (cache, 0, serialize->size);

    serialize->linear = cache;

    cache->magic = FC_CACHE_MAGIC_ALLOC;
    cache->version = FC_CACHE_CONTENT_VERSION;
    cache->size = serialize->size;
    cache->mtime = (int) dir_stat->st_mtime;

    /*
     * Serialize directory name
     */
    dir_serialize = FcStrSerialize (serialize, dir);
    if (!dir_serialize)
	goto bail2;
    cache->dir = FcPtrToOffset (cache, dir_serialize);

    /*
     * Serialize sub dirs
     */
    dirs_serialize = FcSerializePtr (serialize, dirs);
    if (!dirs_serialize)
	goto bail2;
    cache->dirs = FcPtrToOffset (cache, dirs_serialize);
    cache->dirs_count = dirs->num;
    for (i = 0; i < dirs->num; i++)
    {
	FcChar8	*d_serialize = FcStrSerialize (serialize, dirs->strs[i]);
	if (!d_serialize)
	    goto bail2;
	dirs_serialize[i] = FcPtrToOffset (dirs_serialize, d_serialize);
    }

    /*
     * Serialize font set
     */
    set_serialize = FcFontSetSerialize (serialize, set);
    if (!set_serialize)
	goto bail2;
    cache->set = FcPtrToOffset (cache, set_serialize);

    FcSerializeDestroy (serialize);

    FcCacheInsert (cache, NULL);

    return cache;

bail2:
    free (cache);
bail1:
    FcSerializeDestroy (serialize);
    return NULL;
}


#ifdef _WIN32
#define mkdir(path,mode) _mkdir(path)
#endif

static FcBool
FcMakeDirectory (const FcChar8 *dir)
{
    FcChar8 *parent;
    FcBool  ret;

    if (strlen ((char *) dir) == 0)
	return FcFalse;

    parent = FcStrDirname (dir);
    if (!parent)
	return FcFalse;
    if (access ((char *) parent, F_OK) == 0)
	ret = mkdir ((char *) dir, 0755) == 0 && chmod ((char *) dir, 0755) == 0;
    else if (access ((char *) parent, F_OK) == -1)
	ret = FcMakeDirectory (parent) && (mkdir ((char *) dir, 0755) == 0) && chmod ((char *) dir, 0755) == 0;
    else
	ret = FcFalse;
    FcStrFree (parent);
    return ret;
}

/* write serialized state to the cache file */
FcBool
FcDirCacheWrite (FcCache *cache, FcConfig *config)
{
    FcChar8	    *dir = FcCacheDir (cache);
    FcChar8	    cache_base[CACHEBASE_LEN];
    FcChar8	    *cache_hashed;
    int 	    fd;
    FcAtomic 	    *atomic;
    FcStrList	    *list;
    FcChar8	    *cache_dir = NULL;
    FcChar8	    *test_dir;
    FcCacheSkip     *skip;
    struct stat     cache_stat;
    int		    magic;
    int		    written;

    /*
     * Write it to the first directory in the list which is writable
     */

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
	return FcFalse;
    while ((test_dir = FcStrListNext (list))) {
	if (access ((char *) test_dir, W_OK|X_OK) == 0)
	{
	    cache_dir = test_dir;
	    break;
	}
	else
	{
	    /*
	     * If the directory doesn't exist, try to create it
	     */
	    if (access ((char *) test_dir, F_OK) == -1) {
		if (FcMakeDirectory (test_dir))
		{
		    cache_dir = test_dir;
		    break;
		}
	    }
	    /*
	     * Otherwise, try making it writable
	     */
	    else if (chmod ((char *) test_dir, 0755) == 0)
	    {
		cache_dir = test_dir;
		break;
	    }
	}
    }
    FcStrListDone (list);
    if (!cache_dir)
	return FcFalse;

    FcDirCacheBasename (dir, cache_base);
    cache_hashed = FcStrPlus (cache_dir, cache_base);
    if (!cache_hashed)
        return FcFalse;

    if (FcDebug () & FC_DBG_CACHE)
        printf ("FcDirCacheWriteDir dir \"%s\" file \"%s\"\n",
		dir, cache_hashed);

    atomic = FcAtomicCreate ((FcChar8 *)cache_hashed);
    if (!atomic)
	goto bail1;

    if (!FcAtomicLock (atomic))
	goto bail3;

    fd = open((char *)FcAtomicNewFile (atomic), O_RDWR | O_CREAT | O_BINARY, 0666);
    if (fd == -1)
	goto bail4;

    /* Temporarily switch magic to MMAP while writing to file */
    magic = cache->magic;
    if (magic != FC_CACHE_MAGIC_MMAP)
	cache->magic = FC_CACHE_MAGIC_MMAP;

    /*
     * Write cache contents to file
     */
    written = write (fd, cache, cache->size);

    /* Switch magic back */
    if (magic != FC_CACHE_MAGIC_MMAP)
	cache->magic = magic;

    if (written != cache->size)
    {
	perror ("write cache");
	goto bail5;
    }

    close(fd);
    if (!FcAtomicReplaceOrig(atomic))
        goto bail4;

    /* If the file is small, update the cache chain entry such that the
     * new cache file is not read again.  If it's large, we don't do that
     * such that we reload it, using mmap, which is shared across processes.
     */
    if (cache->size < FC_CACHE_MIN_MMAP &&
	(skip = FcCacheFindByAddr (cache)) &&
	FcStat (cache_hashed, &cache_stat))
    {
	skip->cache_dev = cache_stat.st_dev;
	skip->cache_ino = cache_stat.st_ino;
	skip->cache_mtime = cache_stat.st_mtime;
    }

    FcStrFree (cache_hashed);
    FcAtomicUnlock (atomic);
    FcAtomicDestroy (atomic);
    return FcTrue;

 bail5:
    close (fd);
 bail4:
    FcAtomicUnlock (atomic);
 bail3:
    FcAtomicDestroy (atomic);
 bail1:
    FcStrFree (cache_hashed);
    return FcFalse;
}

/*
 * Hokey little macro trick to permit the definitions of C functions
 * with the same name as CPP macros
 */
#define args1(x)	    (x)
#define args2(x,y)	    (x,y)

const FcChar8 *
FcCacheDir args1(const FcCache *c)
{
    return FcCacheDir (c);
}

FcFontSet *
FcCacheCopySet args1(const FcCache *c)
{
    FcFontSet	*old = FcCacheSet (c);
    FcFontSet	*new = FcFontSetCreate ();
    int		i;

    if (!new)
	return NULL;
    for (i = 0; i < old->nfont; i++)
    {
	FcPattern   *font = FcFontSetFont (old, i);
	
	FcPatternReference (font);
	if (!FcFontSetAdd (new, font))
	{
	    FcFontSetDestroy (new);
	    return NULL;
	}
    }
    return new;
}

const FcChar8 *
FcCacheSubdir args2(const FcCache *c, int i)
{
    return FcCacheSubdir (c, i);
}

int
FcCacheNumSubdir args1(const FcCache *c)
{
    return c->dirs_count;
}

int
FcCacheNumFont args1(const FcCache *c)
{
    return FcCacheSet(c)->nfont;
}

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.	This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
/*
 * Note: this code is harmless on little-endian machines.
 */
void byteReverse(unsigned char *buf, unsigned longs)
{
    FcChar32 t;
    do {
	t = (FcChar32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
	    ((unsigned) buf[1] << 8 | buf[0]);
	*(FcChar32 *) buf = t;
	buf += 4;
    } while (--longs);
}
#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update(struct MD5Context *ctx, const unsigned char *buf, unsigned len)
{
    FcChar32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((FcChar32) len << 3)) < t)
	ctx->bits[1]++; 	/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) ctx->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (FcChar32 *) ctx->in);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy(ctx->in, buf, 64);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (FcChar32 *) ctx->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, count);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (FcChar32 *) ctx->in);

	/* Now fill the next block with 56 bytes */
	memset(ctx->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((FcChar32 *) ctx->in)[14] = ctx->bits[0];
    ((FcChar32 *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (FcChar32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(*ctx));        /* In case it's sensitive */
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void MD5Transform(FcChar32 buf[4], FcChar32 in[16])
{
    register FcChar32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
#define __fccache__
#include "fcaliastail.h"
#undef __fccache__
