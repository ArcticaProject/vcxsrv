/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be
 used in advertising or publicity pertaining to distribution
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.

 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#ifndef _XKBPATH_H_
#define _XKBPATH_H_ 1

extern Bool XkbInitIncludePath(void);

extern void XkbClearIncludePath(void);

extern void XkbAddDefaultDirectoriesToPath(void);

extern Bool XkbAddDirectoryToPath(const char *  /* dir */
    );

extern char *XkbDirectoryForInclude(unsigned    /* type */
    );

extern FILE *XkbFindFileInPath(char * /* name */ ,
                               unsigned /* type */ ,
                               char **  /* pathRtrn */
    );

extern void *XkbAddFileToCache(char * /* name */ ,
                               unsigned /* type */ ,
                               char * /* path */ ,
                               void *   /* data */
    );

extern void *XkbFindFileInCache(char * /* name */ ,
                                unsigned /* type */ ,
                                char ** /* pathRtrn */
    );

extern Bool XkbParseIncludeMap(char ** /* str_inout */ ,
                               char ** /* file_rtrn */ ,
                               char ** /* map_rtrn */ ,
                               char * /* nextop_rtrn */ ,
                               char **  /* extra_data */
    );

#endif /* _XKBPATH_H_ */
