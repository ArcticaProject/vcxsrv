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

#ifndef VMOD_H
#define VMOD_H 1

typedef struct _VModInfo
{
    XkbDescPtr xkb;
    unsigned defined;
    unsigned available;
    unsigned newlyDefined;
    int errorCount;
} VModInfo;

extern void InitVModInfo(VModInfo * /* info */ ,
                         XkbDescPtr     /* xkb */
    );

extern void ClearVModInfo(VModInfo * /* info */ ,
                          XkbDescPtr    /* xkb */
    );

extern Bool HandleVModDef(VModDef * /* stmt */ ,
                          unsigned /* mergeMode */ ,
                          VModInfo *    /* info */
    );

extern Bool ApplyVModDefs(VModInfo * /* info */ ,
                          XkbDescPtr    /* xkb */
    );

extern int LookupVModIndex(XPointer /* priv */ ,
                           Atom /* elem */ ,
                           Atom /* field */ ,
                           unsigned /* type */ ,
                           ExprResult * /* val_rtrn */
    );

extern int LookupVModMask(XPointer /* priv */ ,
                          Atom /* elem */ ,
                          Atom /* field */ ,
                          unsigned /* type */ ,
                          ExprResult *  /* val_rtrn */
    );

extern int FindKeypadVMod(XkbDescPtr    /* xkb */
    );

extern Bool ResolveVirtualModifier(ExprDef * /* def */ ,
                                   ExprResult * /* value_rtrn */ ,
                                   VModInfo *   /* info */
    );

#endif /* VMOD_H */
