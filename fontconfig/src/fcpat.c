/*
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
#include "fcftint.h"

/* Objects MT-safe for readonly access. */

FcPattern *
FcPatternCreate (void)
{
    FcPattern	*p;

    p = (FcPattern *) malloc (sizeof (FcPattern));
    if (!p)
	return 0;
    memset (p, 0, sizeof (FcPattern));
    p->num = 0;
    p->size = 0;
    p->elts_offset = FcPtrToOffset (p, NULL);
    FcRefInit (&p->ref, 1);
    return p;
}

void
FcValueDestroy (FcValue v)
{
    switch ((int) v.type) {
    case FcTypeString:
	FcFree (v.u.s);
	break;
    case FcTypeMatrix:
	FcMatrixFree ((FcMatrix *) v.u.m);
	break;
    case FcTypeCharSet:
	FcCharSetDestroy ((FcCharSet *) v.u.c);
	break;
    case FcTypeLangSet:
	FcLangSetDestroy ((FcLangSet *) v.u.l);
	break;
    case FcTypeRange:
	FcRangeDestroy ((FcRange *) v.u.r);
	break;
    default:
	break;
    }
}

FcValue
FcValueCanonicalize (const FcValue *v)
{
    FcValue new;

    switch ((int) v->type)
    {
    case FcTypeString:
	new.u.s = FcValueString(v);
	new.type = FcTypeString;
	break;
    case FcTypeCharSet:
	new.u.c = FcValueCharSet(v);
	new.type = FcTypeCharSet;
	break;
    case FcTypeLangSet:
	new.u.l = FcValueLangSet(v);
	new.type = FcTypeLangSet;
	break;
    case FcTypeRange:
	new.u.r = FcValueRange(v);
	new.type = FcTypeRange;
	break;
    default:
	new = *v;
	break;
    }
    return new;
}

FcValue
FcValueSave (FcValue v)
{
    switch ((int) v.type) {
    case FcTypeString:
	v.u.s = FcStrdup (v.u.s);
	if (!v.u.s)
	    v.type = FcTypeVoid;
	break;
    case FcTypeMatrix:
	v.u.m = FcMatrixCopy (v.u.m);
	if (!v.u.m)
	    v.type = FcTypeVoid;
	break;
    case FcTypeCharSet:
	v.u.c = FcCharSetCopy ((FcCharSet *) v.u.c);
	if (!v.u.c)
	    v.type = FcTypeVoid;
	break;
    case FcTypeLangSet:
	v.u.l = FcLangSetCopy (v.u.l);
	if (!v.u.l)
	    v.type = FcTypeVoid;
	break;
    case FcTypeRange:
	v.u.r = FcRangeCopy (v.u.r);
	if (!v.u.r)
	    v.type = FcTypeVoid;
	break;
    default:
	break;
    }
    return v;
}

FcValueListPtr
FcValueListCreate (void)
{
    return calloc (1, sizeof (FcValueList));
}

void
FcValueListDestroy (FcValueListPtr l)
{
    FcValueListPtr next;
    for (; l; l = next)
    {
	switch ((int) l->value.type) {
	case FcTypeString:
	    FcFree (l->value.u.s);
	    break;
	case FcTypeMatrix:
	    FcMatrixFree ((FcMatrix *)l->value.u.m);
	    break;
	case FcTypeCharSet:
	    FcCharSetDestroy
		((FcCharSet *) (l->value.u.c));
	    break;
	case FcTypeLangSet:
	    FcLangSetDestroy
		((FcLangSet *) (l->value.u.l));
	    break;
	case FcTypeRange:
	    FcRangeDestroy ((FcRange *) (l->value.u.r));
	    break;
	default:
	    break;
	}
	next = FcValueListNext(l);
	free(l);
    }
}

FcValueListPtr
FcValueListPrepend (FcValueListPtr vallist,
		    FcValue        value,
		    FcValueBinding binding)
{
    FcValueListPtr new;

    if (value.type == FcTypeVoid)
	return vallist;
    new = FcValueListCreate ();
    if (!new)
	return vallist;

    new->value = FcValueSave (value);
    new->binding = binding;
    new->next = vallist;

    return new;
}

FcValueListPtr
FcValueListAppend (FcValueListPtr vallist,
		   FcValue        value,
		   FcValueBinding binding)
{
    FcValueListPtr new, last;

    if (value.type == FcTypeVoid)
	return vallist;
    new = FcValueListCreate ();
    if (!new)
	return vallist;

    new->value = FcValueSave (value);
    new->binding = binding;
    new->next = NULL;

    if (vallist)
    {
	for (last = vallist; FcValueListNext (last); last = FcValueListNext (last));

	last->next = new;
    }
    else
	vallist = new;

    return vallist;
}

FcValueListPtr
FcValueListDuplicate(FcValueListPtr orig)
{
    FcValueListPtr new = NULL, l, t = NULL;
    FcValue v;

    for (l = orig; l != NULL; l = FcValueListNext (l))
    {
	if (!new)
	{
	    t = new = FcValueListCreate();
	}
	else
	{
	    t->next = FcValueListCreate();
	    t = FcValueListNext (t);
	}
	v = FcValueCanonicalize (&l->value);
	t->value = FcValueSave (v);
	t->binding = l->binding;
	t->next = NULL;
    }

    return new;
}

FcBool
FcValueEqual (FcValue va, FcValue vb)
{
    if (va.type != vb.type)
    {
	if (va.type == FcTypeInteger)
	{
	    va.type = FcTypeDouble;
	    va.u.d = va.u.i;
	}
	if (vb.type == FcTypeInteger)
	{
	    vb.type = FcTypeDouble;
	    vb.u.d = vb.u.i;
	}
	if (va.type != vb.type)
	    return FcFalse;
    }
    switch (va.type) {
    case FcTypeUnknown:
	return FcFalse;	/* don't know how to compare this object */
    case FcTypeVoid:
	return FcTrue;
    case FcTypeInteger:
	return va.u.i == vb.u.i;
    case FcTypeDouble:
	return va.u.d == vb.u.d;
    case FcTypeString:
	return FcStrCmpIgnoreCase (va.u.s, vb.u.s) == 0;
    case FcTypeBool:
	return va.u.b == vb.u.b;
    case FcTypeMatrix:
	return FcMatrixEqual (va.u.m, vb.u.m);
    case FcTypeCharSet:
	return FcCharSetEqual (va.u.c, vb.u.c);
    case FcTypeFTFace:
	return va.u.f == vb.u.f;
    case FcTypeLangSet:
	return FcLangSetEqual (va.u.l, vb.u.l);
    case FcTypeRange:
	return FcRangeIsInRange (va.u.r, vb.u.r);
    }
    return FcFalse;
}

static FcChar32
FcDoubleHash (double d)
{
    if (d < 0)
	d = -d;
    if (d > 0xffffffff)
	d = 0xffffffff;
    return (FcChar32) d;
}

FcChar32
FcStringHash (const FcChar8 *s)
{
    FcChar8	c;
    FcChar32	h = 0;

    if (s)
	while ((c = *s++))
	    h = ((h << 1) | (h >> 31)) ^ c;
    return h;
}

static FcChar32
FcValueHash (const FcValue *v)
{
    switch (v->type) {
    case FcTypeUnknown:
    case FcTypeVoid:
	return 0;
    case FcTypeInteger:
	return (FcChar32) v->u.i;
    case FcTypeDouble:
	return FcDoubleHash (v->u.d);
    case FcTypeString:
	return FcStringHash (FcValueString(v));
    case FcTypeBool:
	return (FcChar32) v->u.b;
    case FcTypeMatrix:
	return (FcDoubleHash (v->u.m->xx) ^
		FcDoubleHash (v->u.m->xy) ^
		FcDoubleHash (v->u.m->yx) ^
		FcDoubleHash (v->u.m->yy));
    case FcTypeCharSet:
	return (FcChar32) FcValueCharSet(v)->num;
    case FcTypeFTFace:
	return FcStringHash ((const FcChar8 *) ((FT_Face) v->u.f)->family_name) ^
	       FcStringHash ((const FcChar8 *) ((FT_Face) v->u.f)->style_name);
    case FcTypeLangSet:
	return FcLangSetHash (FcValueLangSet(v));
    case FcTypeRange:
	return FcRangeHash (v->u.r);
    }
    return 0;
}

static FcBool
FcValueListEqual (FcValueListPtr la, FcValueListPtr lb)
{
    if (la == lb)
	return FcTrue;

    while (la && lb)
    {
	if (!FcValueEqual (la->value, lb->value))
	    return FcFalse;
	la = FcValueListNext(la);
	lb = FcValueListNext(lb);
    }
    if (la || lb)
	return FcFalse;
    return FcTrue;
}

static FcChar32
FcValueListHash (FcValueListPtr l)
{
    FcChar32	hash = 0;

    for (; l; l = FcValueListNext(l))
    {
	hash = ((hash << 1) | (hash >> 31)) ^ FcValueHash (&l->value);
    }
    return hash;
}

void
FcPatternDestroy (FcPattern *p)
{
    int		    i;
    FcPatternElt    *elts;

    if (!p)
	return;

    if (FcRefIsConst (&p->ref))
    {
	FcCacheObjectDereference (p);
	return;
    }
	
    if (FcRefDec (&p->ref) != 1)
	return;

    elts = FcPatternElts (p);
    for (i = 0; i < p->num; i++)
	FcValueListDestroy (FcPatternEltValues(&elts[i]));

    free (elts);
    free (p);
}

static int
FcPatternObjectPosition (const FcPattern *p, FcObject object)
{
    int	    low, high, mid, c;
    FcPatternElt    *elts = FcPatternElts(p);

    low = 0;
    high = p->num - 1;
    c = 1;
    mid = 0;
    while (low <= high)
    {
	mid = (low + high) >> 1;
	c = elts[mid].object - object;
	if (c == 0)
	    return mid;
	if (c < 0)
	    low = mid + 1;
	else
	    high = mid - 1;
    }
    if (c < 0)
	mid++;
    return -(mid + 1);
}

int
FcPatternPosition (const FcPattern *p, const char *object)
{
    return FcPatternObjectPosition (p, FcObjectFromName (object));
}

FcPatternElt *
FcPatternObjectFindElt (const FcPattern *p, FcObject object)
{
    int	    i = FcPatternObjectPosition (p, object);
    if (i < 0)
	return 0;
    return &FcPatternElts(p)[i];
}

FcPatternElt *
FcPatternObjectInsertElt (FcPattern *p, FcObject object)
{
    int		    i;
    FcPatternElt   *e;

    i = FcPatternObjectPosition (p, object);
    if (i < 0)
    {
	i = -i - 1;

	/* reallocate array */
	if (p->num + 1 >= p->size)
	{
	    int s = p->size + 16;
	    if (p->size)
	    {
		FcPatternElt *e0 = FcPatternElts(p);
		e = (FcPatternElt *) realloc (e0, s * sizeof (FcPatternElt));
		if (!e) /* maybe it was mmapped */
		{
		    e = malloc(s * sizeof (FcPatternElt));
		    if (e)
			memcpy(e, e0, p->num * sizeof (FcPatternElt));
		}
	    }
	    else
		e = (FcPatternElt *) malloc (s * sizeof (FcPatternElt));
	    if (!e)
		return FcFalse;
	    p->elts_offset = FcPtrToOffset (p, e);
	    while (p->size < s)
	    {
		e[p->size].object = 0;
		e[p->size].values = NULL;
		p->size++;
	    }
	}
	
	e = FcPatternElts(p);
	/* move elts up */
	memmove (e + i + 1,
		 e + i,
		 sizeof (FcPatternElt) *
		 (p->num - i));
		
	/* bump count */
	p->num++;
	
	e[i].object = object;
	e[i].values = NULL;
    }

    return FcPatternElts(p) + i;
}

FcBool
FcPatternEqual (const FcPattern *pa, const FcPattern *pb)
{
    int	i;
    FcPatternElt   *pae, *pbe;

    if (pa == pb)
	return FcTrue;

    if (pa->num != pb->num)
	return FcFalse;
    pae = FcPatternElts(pa);
    pbe = FcPatternElts(pb);
    for (i = 0; i < pa->num; i++)
    {
	if (pae[i].object != pbe[i].object)
	    return FcFalse;
	if (!FcValueListEqual (FcPatternEltValues(&pae[i]),
			       FcPatternEltValues(&pbe[i])))
	    return FcFalse;
    }
    return FcTrue;
}

FcChar32
FcPatternHash (const FcPattern *p)
{
    int		i;
    FcChar32	h = 0;
    FcPatternElt    *pe = FcPatternElts(p);

    for (i = 0; i < p->num; i++)
    {
	h = (((h << 1) | (h >> 31)) ^
	     pe[i].object ^
	     FcValueListHash (FcPatternEltValues(&pe[i])));
    }
    return h;
}

FcBool
FcPatternEqualSubset (const FcPattern *pai, const FcPattern *pbi, const FcObjectSet *os)
{
    FcPatternElt    *ea, *eb;
    int		    i;

    for (i = 0; i < os->nobject; i++)
    {
	FcObject    object = FcObjectFromName (os->objects[i]);
	ea = FcPatternObjectFindElt (pai, object);
	eb = FcPatternObjectFindElt (pbi, object);
	if (ea)
	{
	    if (!eb)
		return FcFalse;
	    if (!FcValueListEqual (FcPatternEltValues(ea), FcPatternEltValues(eb)))
		return FcFalse;
	}
	else
	{
	    if (eb)
		return FcFalse;
	}
    }
    return FcTrue;
}

FcBool
FcPatternObjectListAdd (FcPattern	*p,
			FcObject	object,
			FcValueListPtr	list,
			FcBool		append)
{
    FcPatternElt   *e;
    FcValueListPtr l, *prev;

    if (FcRefIsConst (&p->ref))
	goto bail0;

    /*
     * Make sure the stored type is valid for built-in objects
     */
    for (l = list; l != NULL; l = FcValueListNext (l))
    {
	if (!FcObjectValidType (object, l->value.type))
	{
	    fprintf (stderr,
		     "Fontconfig warning: FcPattern object %s does not accept value", FcObjectName (object));
	    FcValuePrintFile (stderr, l->value);
	    fprintf (stderr, "\n");
	    goto bail0;
	}
    }

    e = FcPatternObjectInsertElt (p, object);
    if (!e)
	goto bail0;

    if (append)
    {
	for (prev = &e->values; *prev; prev = &(*prev)->next)
	    ;
	*prev = list;
    }
    else
    {
	for (prev = &list; *prev; prev = &(*prev)->next)
	    ;
	*prev = e->values;
	e->values = list;
    }

    return FcTrue;

bail0:
    return FcFalse;
}

FcBool
FcPatternObjectAddWithBinding  (FcPattern	*p,
				FcObject	object,
				FcValue		value,
				FcValueBinding  binding,
				FcBool		append)
{
    FcPatternElt   *e;
    FcValueListPtr new, *prev;

    if (FcRefIsConst (&p->ref))
	goto bail0;

    new = FcValueListCreate ();
    if (!new)
	goto bail0;

    value = FcValueSave (value);
    if (value.type == FcTypeVoid)
	goto bail1;

    /*
     * Make sure the stored type is valid for built-in objects
     */
    if (!FcObjectValidType (object, value.type))
    {
	fprintf (stderr,
		 "Fontconfig warning: FcPattern object %s does not accept value",
		 FcObjectName (object));
	FcValuePrintFile (stderr, value);
	fprintf (stderr, "\n");
	goto bail1;
    }

    new->value = value;
    new->binding = binding;
    new->next = NULL;

    e = FcPatternObjectInsertElt (p, object);
    if (!e)
	goto bail2;

    if (append)
    {
	for (prev = &e->values; *prev; prev = &(*prev)->next)
	    ;
	*prev = new;
    }
    else
    {
	new->next = e->values;
	e->values = new;
    }

    return FcTrue;

bail2:
    FcValueDestroy (value);
bail1:
    free (new);
bail0:
    return FcFalse;
}

FcBool
FcPatternObjectAdd (FcPattern *p, FcObject object, FcValue value, FcBool append)
{
    return FcPatternObjectAddWithBinding (p, object,
					  value, FcValueBindingStrong, append);
}

FcBool
FcPatternAdd (FcPattern *p, const char *object, FcValue value, FcBool append)
{
    return FcPatternObjectAddWithBinding (p, FcObjectFromName (object),
					  value, FcValueBindingStrong, append);
}

FcBool
FcPatternAddWeak  (FcPattern *p, const char *object, FcValue value, FcBool append)
{
    return FcPatternObjectAddWithBinding (p, FcObjectFromName (object),
					  value, FcValueBindingWeak, append);
}

FcBool
FcPatternObjectDel (FcPattern *p, FcObject object)
{
    FcPatternElt   *e;

    e = FcPatternObjectFindElt (p, object);
    if (!e)
	return FcFalse;

    /* destroy value */
    FcValueListDestroy (e->values);

    /* shuffle existing ones down */
    memmove (e, e+1,
	     (FcPatternElts(p) + p->num - (e + 1)) *
	     sizeof (FcPatternElt));
    p->num--;
    e = FcPatternElts(p) + p->num;
    e->object = 0;
    e->values = NULL;
    return FcTrue;
}

FcBool
FcPatternDel (FcPattern *p, const char *object)
{
    return FcPatternObjectDel (p, FcObjectFromName (object));
}

FcBool
FcPatternRemove (FcPattern *p, const char *object, int id)
{
    FcPatternElt    *e;
    FcValueListPtr  *prev, l;

    e = FcPatternObjectFindElt (p, FcObjectFromName (object));
    if (!e)
	return FcFalse;
    for (prev = &e->values; (l = *prev); prev = &l->next)
    {
	if (!id)
	{
	    *prev = l->next;
	    l->next = NULL;
	    FcValueListDestroy (l);
	    if (!e->values)
		FcPatternDel (p, object);
	    return FcTrue;
	}
	id--;
    }
    return FcFalse;
}

FcBool
FcPatternObjectAddInteger (FcPattern *p, FcObject object, int i)
{
    FcValue	v;

    v.type = FcTypeInteger;
    v.u.i = i;
    return FcPatternObjectAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddInteger (FcPattern *p, const char *object, int i)
{
    return FcPatternObjectAddInteger (p, FcObjectFromName (object), i);
}

FcBool
FcPatternObjectAddDouble (FcPattern *p, FcObject object, double d)
{
    FcValue	v;

    v.type = FcTypeDouble;
    v.u.d = d;
    return FcPatternObjectAdd (p, object, v, FcTrue);
}


FcBool
FcPatternAddDouble (FcPattern *p, const char *object, double d)
{
    return FcPatternObjectAddDouble (p, FcObjectFromName (object), d);
}

FcBool
FcPatternObjectAddString (FcPattern *p, FcObject object, const FcChar8 *s)
{
    FcValue	v;

    if (!s)
    {
	v.type = FcTypeVoid;
	v.u.s = 0;
	return FcPatternObjectAdd (p, object, v, FcTrue);
    }

    v.type = FcTypeString;
    v.u.s = s;
    return FcPatternObjectAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddString (FcPattern *p, const char *object, const FcChar8 *s)
{
    return FcPatternObjectAddString (p, FcObjectFromName (object), s);
}

FcBool
FcPatternAddMatrix (FcPattern *p, const char *object, const FcMatrix *s)
{
    FcValue	v;

    v.type = FcTypeMatrix;
    v.u.m = s;
    return FcPatternAdd (p, object, v, FcTrue);
}


FcBool
FcPatternObjectAddBool (FcPattern *p, FcObject object, FcBool b)
{
    FcValue	v;

    v.type = FcTypeBool;
    v.u.b = b;
    return FcPatternObjectAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddBool (FcPattern *p, const char *object, FcBool b)
{
    return FcPatternObjectAddBool (p, FcObjectFromName (object), b);
}

FcBool
FcPatternAddCharSet (FcPattern *p, const char *object, const FcCharSet *c)
{
    FcValue	v;

    v.type = FcTypeCharSet;
    v.u.c = (FcCharSet *)c;
    return FcPatternAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddFTFace (FcPattern *p, const char *object, const FT_Face f)
{
    FcValue	v;

    v.type = FcTypeFTFace;
    v.u.f = (void *) f;
    return FcPatternAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddLangSet (FcPattern *p, const char *object, const FcLangSet *ls)
{
    FcValue	v;

    v.type = FcTypeLangSet;
    v.u.l = (FcLangSet *)ls;
    return FcPatternAdd (p, object, v, FcTrue);
}

FcBool
FcPatternObjectAddRange (FcPattern *p, FcObject object, const FcRange *r)
{
    FcValue v;

    v.type = FcTypeRange;
    v.u.r = (FcRange *)r;
    return FcPatternObjectAdd (p, object, v, FcTrue);
}

FcBool
FcPatternAddRange (FcPattern *p, const char *object, const FcRange *r)
{
    return FcPatternObjectAddRange (p, FcObjectFromName (object), r);
}

FcResult
FcPatternObjectGet (const FcPattern *p, FcObject object, int id, FcValue *v)
{
    FcPatternElt   *e;
    FcValueListPtr l;

    if (!p)
	return FcResultNoMatch;
    e = FcPatternObjectFindElt (p, object);
    if (!e)
	return FcResultNoMatch;
    for (l = FcPatternEltValues(e); l; l = FcValueListNext(l))
    {
	if (!id)
	{
	    *v = FcValueCanonicalize(&l->value);
	    return FcResultMatch;
	}
	id--;
    }
    return FcResultNoId;
}

FcResult
FcPatternGet (const FcPattern *p, const char *object, int id, FcValue *v)
{
    return FcPatternObjectGet (p, FcObjectFromName (object), id, v);
}

FcResult
FcPatternObjectGetInteger (const FcPattern *p, FcObject object, int id, int *i)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternObjectGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    switch ((int) v.type) {
    case FcTypeDouble:
	*i = (int) v.u.d;
	break;
    case FcTypeInteger:
	*i = v.u.i;
	break;
    default:
        return FcResultTypeMismatch;
    }
    return FcResultMatch;
}

FcResult
FcPatternGetInteger (const FcPattern *p, const char *object, int id, int *i)
{
    return FcPatternObjectGetInteger (p, FcObjectFromName (object), id, i);
}


FcResult
FcPatternObjectGetDouble (const FcPattern *p, FcObject object, int id, double *d)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternObjectGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    switch ((int) v.type) {
    case FcTypeDouble:
	*d = v.u.d;
	break;
    case FcTypeInteger:
	*d = (double) v.u.i;
	break;
    default:
        return FcResultTypeMismatch;
    }
    return FcResultMatch;
}

FcResult
FcPatternGetDouble (const FcPattern *p, const char *object, int id, double *d)
{
    return FcPatternObjectGetDouble (p, FcObjectFromName (object), id, d);
}

FcResult
FcPatternObjectGetString (const FcPattern *p, FcObject object, int id, FcChar8 ** s)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternObjectGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeString)
        return FcResultTypeMismatch;

    *s = (FcChar8 *) v.u.s;
    return FcResultMatch;
}

FcResult
FcPatternGetString (const FcPattern *p, const char *object, int id, FcChar8 ** s)
{
    return FcPatternObjectGetString (p, FcObjectFromName (object), id, s);
}

FcResult
FcPatternGetMatrix(const FcPattern *p, const char *object, int id, FcMatrix **m)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeMatrix)
        return FcResultTypeMismatch;
    *m = (FcMatrix *)v.u.m;
    return FcResultMatch;
}


FcResult
FcPatternGetBool(const FcPattern *p, const char *object, int id, FcBool *b)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeBool)
        return FcResultTypeMismatch;
    *b = v.u.b;
    return FcResultMatch;
}

FcResult
FcPatternGetCharSet(const FcPattern *p, const char *object, int id, FcCharSet **c)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeCharSet)
        return FcResultTypeMismatch;
    *c = (FcCharSet *)v.u.c;
    return FcResultMatch;
}

FcResult
FcPatternGetFTFace(const FcPattern *p, const char *object, int id, FT_Face *f)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeFTFace)
	return FcResultTypeMismatch;
    *f = (FT_Face) v.u.f;
    return FcResultMatch;
}

FcResult
FcPatternGetLangSet(const FcPattern *p, const char *object, int id, FcLangSet **ls)
{
    FcValue	v;
    FcResult	r;

    r = FcPatternGet (p, object, id, &v);
    if (r != FcResultMatch)
	return r;
    if (v.type != FcTypeLangSet)
        return FcResultTypeMismatch;
    *ls = (FcLangSet *)v.u.l;
    return FcResultMatch;
}

FcResult
FcPatternObjectGetRange (const FcPattern *p, FcObject object, int id, FcRange **r)
{
    FcValue	v;
    FcResult	res;

    res = FcPatternObjectGet (p, object, id, &v);
    if (res != FcResultMatch)
	return res;
    switch ((int)v.type) {
    case FcTypeRange:
	*r = (FcRange *)v.u.r;
	break;
    default:
	return FcResultTypeMismatch;
    }
    return FcResultMatch;
}

FcResult
FcPatternGetRange (const FcPattern *p, const char *object, int id, FcRange **r)
{
    return FcPatternObjectGetRange (p, FcObjectFromName (object), id, r);
}

FcPattern *
FcPatternDuplicate (const FcPattern *orig)
{
    FcPattern	    *new;
    FcPatternElt    *e;
    int		    i;
    FcValueListPtr  l;

    new = FcPatternCreate ();
    if (!new)
	goto bail0;

    e = FcPatternElts(orig);

    for (i = 0; i < orig->num; i++)
    {
	for (l = FcPatternEltValues(e + i); l; l = FcValueListNext(l))
	{
	    if (!FcPatternObjectAddWithBinding (new, e[i].object,
						FcValueCanonicalize(&l->value),
						l->binding,
						FcTrue))
		goto bail1;
	
	}
    }

    return new;

bail1:
    FcPatternDestroy (new);
bail0:
    return 0;
}

void
FcPatternReference (FcPattern *p)
{
    if (!FcRefIsConst (&p->ref))
	FcRefInc (&p->ref);
    else
	FcCacheObjectReference (p);
}

FcPattern *
FcPatternVaBuild (FcPattern *p, va_list va)
{
    FcPattern	*ret;

    FcPatternVapBuild (ret, p, va);
    return ret;
}

FcPattern *
FcPatternBuild (FcPattern *p, ...)
{
    va_list	va;

    va_start (va, p);
    FcPatternVapBuild (p, p, va);
    va_end (va);
    return p;
}

/*
 * Add all of the elements in 's' to 'p'
 */
FcBool
FcPatternAppend (FcPattern *p, FcPattern *s)
{
    int		    i;
    FcPatternElt    *e;
    FcValueListPtr  v;

    for (i = 0; i < s->num; i++)
    {
	e = FcPatternElts(s)+i;
	for (v = FcPatternEltValues(e); v; v = FcValueListNext(v))
	{
	    if (!FcPatternObjectAddWithBinding (p, e->object,
						FcValueCanonicalize(&v->value),
						v->binding, FcTrue))
		return FcFalse;
	}
    }
    return FcTrue;
}

FcPattern *
FcPatternFilter (FcPattern *p, const FcObjectSet *os)
{
    int		    i;
    FcPattern	    *ret;
    FcPatternElt    *e;
    FcValueListPtr  v;

    if (!os)
	return FcPatternDuplicate (p);

    ret = FcPatternCreate ();
    if (!ret)
	return NULL;

    for (i = 0; i < os->nobject; i++)
    {
	FcObject object = FcObjectFromName (os->objects[i]);
	e = FcPatternObjectFindElt (p, object);
	if (e)
	{
	    for (v = FcPatternEltValues(e); v; v = FcValueListNext(v))
	    {
		if (!FcPatternObjectAddWithBinding (ret, e->object,
						    FcValueCanonicalize(&v->value),
						    v->binding, FcTrue))
		    goto bail0;
	    }
	}
    }
    return ret;

bail0:
    FcPatternDestroy (ret);
    return NULL;
}


FcBool
FcPatternSerializeAlloc (FcSerialize *serialize, const FcPattern *pat)
{
    int	i;
    FcPatternElt    *elts = FcPatternElts(pat);

    if (!FcSerializeAlloc (serialize, pat, sizeof (FcPattern)))
	return FcFalse;
    if (!FcSerializeAlloc (serialize, elts, pat->num * sizeof (FcPatternElt)))
	return FcFalse;
    for (i = 0; i < pat->num; i++)
	if (!FcValueListSerializeAlloc (serialize, FcPatternEltValues(elts+i)))
	    return FcFalse;
    return FcTrue;
}

FcPattern *
FcPatternSerialize (FcSerialize *serialize, const FcPattern *pat)
{
    FcPattern	    *pat_serialized;
    FcPatternElt    *elts = FcPatternElts (pat);
    FcPatternElt    *elts_serialized;
    FcValueList	    *values_serialized;
    int		    i;

    pat_serialized = FcSerializePtr (serialize, pat);
    if (!pat_serialized)
	return NULL;
    *pat_serialized = *pat;
    pat_serialized->size = pat->num;
    FcRefSetConst (&pat_serialized->ref);

    elts_serialized = FcSerializePtr (serialize, elts);
    if (!elts_serialized)
	return NULL;

    pat_serialized->elts_offset = FcPtrToOffset (pat_serialized,
						 elts_serialized);

    for (i = 0; i < pat->num; i++)
    {
	values_serialized = FcValueListSerialize (serialize, FcPatternEltValues (elts+i));
	if (!values_serialized)
	    return NULL;
	elts_serialized[i].object = elts[i].object;
	elts_serialized[i].values = FcPtrToEncodedOffset (&elts_serialized[i],
							  values_serialized,
							  FcValueList);
    }
    if (FcDebug() & FC_DBG_CACHEV) {
	printf ("Raw pattern:\n");
	FcPatternPrint (pat);
	printf ("Serialized pattern:\n");
	FcPatternPrint (pat_serialized);
	printf ("\n");
    }
    return pat_serialized;
}

FcBool
FcValueListSerializeAlloc (FcSerialize *serialize, const FcValueList *vl)
{
    while (vl)
    {
	if (!FcSerializeAlloc (serialize, vl, sizeof (FcValueList)))
	    return FcFalse;
	switch ((int) vl->value.type) {
	case FcTypeString:
	    if (!FcStrSerializeAlloc (serialize, vl->value.u.s))
		return FcFalse;
	    break;
	case FcTypeCharSet:
	    if (!FcCharSetSerializeAlloc (serialize, vl->value.u.c))
		return FcFalse;
	    break;
	case FcTypeLangSet:
	    if (!FcLangSetSerializeAlloc (serialize, vl->value.u.l))
		return FcFalse;
	    break;
	case FcTypeRange:
	    if (!FcRangeSerializeAlloc (serialize, vl->value.u.r))
		return FcFalse;
	    break;
	default:
	    break;
	}
	vl = vl->next;
    }
    return FcTrue;
}

FcValueList *
FcValueListSerialize (FcSerialize *serialize, const FcValueList *vl)
{
    FcValueList	*vl_serialized;
    FcChar8	*s_serialized;
    FcCharSet	*c_serialized;
    FcLangSet	*l_serialized;
    FcRange	*r_serialized;
    FcValueList	*head_serialized = NULL;
    FcValueList	*prev_serialized = NULL;

    while (vl)
    {
	vl_serialized = FcSerializePtr (serialize, vl);
	if (!vl_serialized)
	    return NULL;

	if (prev_serialized)
	    prev_serialized->next = FcPtrToEncodedOffset (prev_serialized,
							  vl_serialized,
							  FcValueList);
	else
	    head_serialized = vl_serialized;
	
	vl_serialized->next = NULL;
	vl_serialized->value.type = vl->value.type;
	switch ((int) vl->value.type) {
	case FcTypeInteger:
	    vl_serialized->value.u.i = vl->value.u.i;
	    break;
	case FcTypeDouble:
	    vl_serialized->value.u.d = vl->value.u.d;
	    break;
	case FcTypeString:
	    s_serialized = FcStrSerialize (serialize, vl->value.u.s);
	    if (!s_serialized)
		return NULL;
	    vl_serialized->value.u.s = FcPtrToEncodedOffset (&vl_serialized->value,
							     s_serialized,
							     FcChar8);
	    break;
	case FcTypeBool:
	    vl_serialized->value.u.b = vl->value.u.b;
	    break;
	case FcTypeMatrix:
	    /* can't happen */
	    break;
	case FcTypeCharSet:
	    c_serialized = FcCharSetSerialize (serialize, vl->value.u.c);
	    if (!c_serialized)
		return NULL;
	    vl_serialized->value.u.c = FcPtrToEncodedOffset (&vl_serialized->value,
							     c_serialized,
							     FcCharSet);
	    break;
	case FcTypeFTFace:
	    /* can't happen */
	    break;
	case FcTypeLangSet:
	    l_serialized = FcLangSetSerialize (serialize, vl->value.u.l);
	    if (!l_serialized)
		return NULL;
	    vl_serialized->value.u.l = FcPtrToEncodedOffset (&vl_serialized->value,
							     l_serialized,
							     FcLangSet);
	    break;
	case FcTypeRange:
	    r_serialized = FcRangeSerialize (serialize, vl->value.u.r);
	    if (!r_serialized)
		return NULL;
	    vl_serialized->value.u.r = FcPtrToEncodedOffset (&vl_serialized->value,
							     r_serialized,
							     FcRange);
	    break;
	default:
	    break;
	}
	prev_serialized = vl_serialized;
	vl = vl->next;
    }
    return head_serialized;
}

#define __fcpat__
#include "fcaliastail.h"
#include "fcftaliastail.h"
#undef __fcpat__
