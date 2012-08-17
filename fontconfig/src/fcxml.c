/*
 * fontconfig/src/fcxml.c
 *
 * Copyright © 2002 Keith Packard
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
#include <fcntl.h>
#include <stdarg.h>
#include <dirent.h>

#ifdef ENABLE_LIBXML2

#include <libxml/parser.h>

#define XML_Char			xmlChar
#define XML_Parser			xmlParserCtxtPtr
#define XML_ParserFree			xmlFreeParserCtxt
#define XML_GetCurrentLineNumber	xmlSAX2GetLineNumber
#define XML_GetErrorCode		xmlCtxtGetLastError
#define XML_ErrorString(Error)		(Error)->message

#else /* ENABLE_LIBXML2 */

#ifndef HAVE_XMLPARSE_H
#define HAVE_XMLPARSE_H 0
#endif

#if HAVE_XMLPARSE_H
#include <xmlparse.h>
#else
#include <expat.h>
#endif

#endif /* ENABLE_LIBXML2 */

#ifdef _WIN32
#include <mbstring.h>
#endif

static void
FcExprDestroy (FcExpr *e);

void
FcTestDestroy (FcTest *test)
{
    if (test->next)
	FcTestDestroy (test->next);
    FcExprDestroy (test->expr);
    FcMemFree (FC_MEM_TEST, sizeof (FcTest));
    free (test);
}

static FcExpr *
FcExprCreateInteger (FcConfig *config, int i)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpInteger;
	e->u.ival = i;
    }
    return e;
}

static FcExpr *
FcExprCreateDouble (FcConfig *config, double d)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpDouble;
	e->u.dval = d;
    }
    return e;
}

static FcExpr *
FcExprCreateString (FcConfig *config, const FcChar8 *s)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpString;
	e->u.sval = FcSharedStr (s);
    }
    return e;
}

static FcExpr *
FcExprCreateMatrix (FcConfig *config, const FcMatrix *m)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpMatrix;
	e->u.mval = FcMatrixCopy (m);
    }
    return e;
}

static FcExpr *
FcExprCreateBool (FcConfig *config, FcBool b)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpBool;
	e->u.bval = b;
    }
    return e;
}

static FcExpr *
FcExprCreateCharSet (FcConfig *config, FcCharSet *charset)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpCharSet;
	e->u.cval = FcCharSetCopy (charset);
    }
    return e;
}

static FcExpr *
FcExprCreateLangSet (FcConfig *config, FcLangSet *langset)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpLangSet;
	e->u.lval = FcLangSetCopy (langset);
    }
    return e;
}

static FcExpr *
FcExprCreateField (FcConfig *config, const char *field)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpField;
	e->u.object = FcObjectFromName (field);
    }
    return e;
}

static FcExpr *
FcExprCreateConst (FcConfig *config, const FcChar8 *constant)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = FcOpConst;
	e->u.constant = FcSharedStr (constant);
    }
    return e;
}

static FcExpr *
FcExprCreateOp (FcConfig *config, FcExpr *left, FcOp op, FcExpr *right)
{
    FcExpr *e = FcConfigAllocExpr (config);
    if (e)
    {
	e->op = op;
	e->u.tree.left = left;
	e->u.tree.right = right;
    }
    return e;
}

static void
FcExprDestroy (FcExpr *e)
{
    if (!e)
	return;
    switch (FC_OP_GET_OP (e->op)) {
    case FcOpInteger:
	break;
    case FcOpDouble:
	break;
    case FcOpString:
	FcSharedStrFree (e->u.sval);
	break;
    case FcOpMatrix:
	FcMatrixFree (e->u.mval);
	break;
    case FcOpRange:
	break;
    case FcOpCharSet:
	FcCharSetDestroy (e->u.cval);
	break;
    case FcOpLangSet:
	FcLangSetDestroy (e->u.lval);
	break;
    case FcOpBool:
	break;
    case FcOpField:
	break;
    case FcOpConst:
	FcSharedStrFree (e->u.constant);
	break;
    case FcOpAssign:
    case FcOpAssignReplace:
    case FcOpPrepend:
    case FcOpPrependFirst:
    case FcOpAppend:
    case FcOpAppendLast:
	break;
    case FcOpOr:
    case FcOpAnd:
    case FcOpEqual:
    case FcOpNotEqual:
    case FcOpLess:
    case FcOpLessEqual:
    case FcOpMore:
    case FcOpMoreEqual:
    case FcOpContains:
    case FcOpListing:
    case FcOpNotContains:
    case FcOpPlus:
    case FcOpMinus:
    case FcOpTimes:
    case FcOpDivide:
    case FcOpQuest:
    case FcOpComma:
	FcExprDestroy (e->u.tree.right);
	/* fall through */
    case FcOpNot:
    case FcOpFloor:
    case FcOpCeil:
    case FcOpRound:
    case FcOpTrunc:
	FcExprDestroy (e->u.tree.left);
	break;
    case FcOpNil:
    case FcOpInvalid:
	break;
    }

    e->op = FcOpNil;
}

void
FcEditDestroy (FcEdit *e)
{
    if (e->next)
	FcEditDestroy (e->next);
    if (e->expr)
	FcExprDestroy (e->expr);
    free (e);
}

typedef enum _FcElement {
    FcElementNone,
    FcElementFontconfig,
    FcElementDir,
    FcElementCacheDir,
    FcElementCache,
    FcElementInclude,
    FcElementConfig,
    FcElementMatch,
    FcElementAlias,
	
    FcElementBlank,
    FcElementRescan,

    FcElementPrefer,
    FcElementAccept,
    FcElementDefault,
    FcElementFamily,

    FcElementSelectfont,
    FcElementAcceptfont,
    FcElementRejectfont,
    FcElementGlob,
    FcElementPattern,
    FcElementPatelt,

    FcElementTest,
    FcElementEdit,
    FcElementInt,
    FcElementDouble,
    FcElementString,
    FcElementMatrix,
    FcElementRange,
    FcElementBool,
    FcElementCharSet,
    FcElementLangSet,
    FcElementName,
    FcElementConst,
    FcElementOr,
    FcElementAnd,
    FcElementEq,
    FcElementNotEq,
    FcElementLess,
    FcElementLessEq,
    FcElementMore,
    FcElementMoreEq,
    FcElementContains,
    FcElementNotContains,
    FcElementPlus,
    FcElementMinus,
    FcElementTimes,
    FcElementDivide,
    FcElementNot,
    FcElementIf,
    FcElementFloor,
    FcElementCeil,
    FcElementRound,
    FcElementTrunc,
    FcElementUnknown
} FcElement;

static const struct {
    const char  name[16];
    FcElement   element;
} fcElementMap[] = {
    { "fontconfig",	FcElementFontconfig },
    { "dir",		FcElementDir },
    { "cachedir",	FcElementCacheDir },
    { "cache",		FcElementCache },
    { "include",	FcElementInclude },
    { "config",		FcElementConfig },
    { "match",		FcElementMatch },
    { "alias",		FcElementAlias },

    { "blank",		FcElementBlank },
    { "rescan",		FcElementRescan },

    { "prefer",		FcElementPrefer },
    { "accept",		FcElementAccept },
    { "default",	FcElementDefault },
    { "family",		FcElementFamily },

    { "selectfont",	FcElementSelectfont },
    { "acceptfont",	FcElementAcceptfont },
    { "rejectfont",	FcElementRejectfont },
    { "glob",		FcElementGlob },
    { "pattern",	FcElementPattern },
    { "patelt",		FcElementPatelt },

    { "test",		FcElementTest },
    { "edit",		FcElementEdit },
    { "int",		FcElementInt },
    { "double",		FcElementDouble },
    { "string",		FcElementString },
    { "matrix",		FcElementMatrix },
    { "range",		FcElementRange },
    { "bool",		FcElementBool },
    { "charset",	FcElementCharSet },
    { "langset",	FcElementLangSet },
    { "name",		FcElementName },
    { "const",		FcElementConst },
    { "or",		FcElementOr },
    { "and",		FcElementAnd },
    { "eq",		FcElementEq },
    { "not_eq",		FcElementNotEq },
    { "less",		FcElementLess },
    { "less_eq",	FcElementLessEq },
    { "more",		FcElementMore },
    { "more_eq",	FcElementMoreEq },
    { "contains",	FcElementContains },
    { "not_contains",	FcElementNotContains },
    { "plus",		FcElementPlus },
    { "minus",		FcElementMinus },
    { "times",		FcElementTimes },
    { "divide",		FcElementDivide },
    { "not",		FcElementNot },
    { "if",		FcElementIf },
    { "floor",		FcElementFloor },
    { "ceil",		FcElementCeil },
    { "round",		FcElementRound },
    { "trunc",		FcElementTrunc },
};
#define NUM_ELEMENT_MAPS (int) (sizeof fcElementMap / sizeof fcElementMap[0])

static FcElement
FcElementMap (const XML_Char *name)
{

    int	    i;
    for (i = 0; i < NUM_ELEMENT_MAPS; i++)
	if (!strcmp ((char *) name, fcElementMap[i].name))
	    return fcElementMap[i].element;
    return FcElementUnknown;
}

typedef struct _FcPStack {
    struct _FcPStack   *prev;
    FcElement		element;
    FcChar8		**attr;
    FcStrBuf		str;
    FcChar8            *attr_buf_static[16];
} FcPStack;

typedef enum _FcVStackTag {
    FcVStackNone,

    FcVStackString,
    FcVStackFamily,
    FcVStackField,
    FcVStackConstant,
    FcVStackGlob,
    FcVStackPattern,

    FcVStackPrefer,
    FcVStackAccept,
    FcVStackDefault,

    FcVStackInteger,
    FcVStackDouble,
    FcVStackMatrix,
    FcVStackRange,
    FcVStackBool,
    FcVStackCharSet,
    FcVStackLangSet,

    FcVStackTest,
    FcVStackExpr,
    FcVStackEdit
} FcVStackTag;

typedef struct _FcVStack {
    struct _FcVStack	*prev;
    FcPStack		*pstack;	/* related parse element */
    FcVStackTag		tag;
    union {
	FcChar8		*string;

	int		integer;
	double		_double;
	FcMatrix	*matrix;
	FcRange		range;
	FcBool		bool_;
	FcCharSet	*charset;
	FcLangSet	*langset;

	FcTest		*test;
	FcQual		qual;
	FcOp		op;
	FcExpr		*expr;
	FcEdit		*edit;

	FcPattern	*pattern;
    } u;
} FcVStack;

typedef struct _FcConfigParse {
    FcPStack	    *pstack;
    FcVStack	    *vstack;
    FcBool	    error;
    const FcChar8   *name;
    FcConfig	    *config;
    XML_Parser	    parser;
    int             pstack_static_used;
    FcPStack        pstack_static[8];
    int             vstack_static_used;
    FcVStack        vstack_static[64];
} FcConfigParse;

typedef enum _FcConfigSeverity {
    FcSevereInfo, FcSevereWarning, FcSevereError
} FcConfigSeverity;

static void
FcConfigMessage (FcConfigParse *parse, FcConfigSeverity severe, const char *fmt, ...)
{
    const char	*s = "unknown";
    va_list	args;

    va_start (args, fmt);

    switch (severe) {
    case FcSevereInfo: s = "info"; break;
    case FcSevereWarning: s = "warning"; break;
    case FcSevereError: s = "error"; break;
    }
    if (parse)
    {
	if (parse->name)
	    fprintf (stderr, "Fontconfig %s: \"%s\", line %d: ", s,
		     parse->name, (int)XML_GetCurrentLineNumber (parse->parser));
	else
	    fprintf (stderr, "Fontconfig %s: line %d: ", s,
		     (int)XML_GetCurrentLineNumber (parse->parser));
	if (severe >= FcSevereError)
	    parse->error = FcTrue;
    }
    else
	fprintf (stderr, "Fontconfig %s: ", s);
    vfprintf (stderr, fmt, args);
    fprintf (stderr, "\n");
    va_end (args);
}


static const char *
FcTypeName (FcType type)
{
    switch (type) {
    case FcTypeVoid:
	return "void";
    case FcTypeInteger:
    case FcTypeDouble:
	return "number";
    case FcTypeString:
	return "string";
    case FcTypeBool:
	return "bool";
    case FcTypeMatrix:
	return "matrix";
    case FcTypeCharSet:
	return "charset";
    case FcTypeFTFace:
	return "FT_Face";
    case FcTypeLangSet:
	return "langset";
    default:
	return "unknown";
    }
}

static void
FcTypecheckValue (FcConfigParse *parse, FcType value, FcType type)
{
    if (value == FcTypeInteger)
	value = FcTypeDouble;
    if (type == FcTypeInteger)
	type = FcTypeDouble;
    if (value != type)
    {
	if ((value == FcTypeLangSet && type == FcTypeString) ||
	    (value == FcTypeString && type == FcTypeLangSet))
	    return;
	if (type == (FcType) -1)
	    return;
	FcConfigMessage (parse, FcSevereWarning, "saw %s, expected %s",
			 FcTypeName (value), FcTypeName (type));
    }
}

static void
FcTypecheckExpr (FcConfigParse *parse, FcExpr *expr, FcType type)
{
    const FcObjectType	*o;
    const FcConstant	*c;

    /* If parsing the expression failed, some nodes may be NULL */
    if (!expr)
	return;

    switch (FC_OP_GET_OP (expr->op)) {
    case FcOpInteger:
    case FcOpDouble:
	FcTypecheckValue (parse, FcTypeDouble, type);
	break;
    case FcOpString:
	FcTypecheckValue (parse, FcTypeString, type);
	break;
    case FcOpMatrix:
	FcTypecheckValue (parse, FcTypeMatrix, type);
	break;
    case FcOpBool:
	FcTypecheckValue (parse, FcTypeBool, type);
	break;
    case FcOpCharSet:
	FcTypecheckValue (parse, FcTypeCharSet, type);
	break;
    case FcOpLangSet:
	FcTypecheckValue (parse, FcTypeLangSet, type);
	break;
    case FcOpNil:
	break;
    case FcOpField:
	o = FcNameGetObjectType (FcObjectName (expr->u.object));
	if (o)
	    FcTypecheckValue (parse, o->type, type);
	break;
    case FcOpConst:
	c = FcNameGetConstant (expr->u.constant);
	if (c)
	{
	    o = FcNameGetObjectType (c->object);
	    if (o)
		FcTypecheckValue (parse, o->type, type);
	}
        else
            FcConfigMessage (parse, FcSevereWarning,
                             "invalid constant used : %s",
                             expr->u.constant);
	break;
    case FcOpQuest:
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeBool);
	FcTypecheckExpr (parse, expr->u.tree.right->u.tree.left, type);
	FcTypecheckExpr (parse, expr->u.tree.right->u.tree.right, type);
	break;
    case FcOpAssign:
    case FcOpAssignReplace:
	break;
    case FcOpEqual:
    case FcOpNotEqual:
    case FcOpLess:
    case FcOpLessEqual:
    case FcOpMore:
    case FcOpMoreEqual:
    case FcOpContains:
    case FcOpNotContains:
    case FcOpListing:
	FcTypecheckValue (parse, FcTypeBool, type);
	break;
    case FcOpComma:
    case FcOpOr:
    case FcOpAnd:
    case FcOpPlus:
    case FcOpMinus:
    case FcOpTimes:
    case FcOpDivide:
	FcTypecheckExpr (parse, expr->u.tree.left, type);
	FcTypecheckExpr (parse, expr->u.tree.right, type);
	break;
    case FcOpNot:
	FcTypecheckValue (parse, FcTypeBool, type);
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeBool);
	break;
    case FcOpFloor:
    case FcOpCeil:
    case FcOpRound:
    case FcOpTrunc:
	FcTypecheckValue (parse, FcTypeDouble, type);
	FcTypecheckExpr (parse, expr->u.tree.left, FcTypeDouble);
	break;
    default:
	break;
    }
}

static FcTest *
FcTestCreate (FcConfigParse *parse,
	      FcMatchKind   kind,
	      FcQual	    qual,
	      const FcChar8 *field,
	      FcOp	    compare,
	      FcExpr	    *expr)
{
    FcTest	*test = (FcTest *) malloc (sizeof (FcTest));

    if (test)
    {
	const FcObjectType	*o;
	
	FcMemAlloc (FC_MEM_TEST, sizeof (FcTest));
	test->next = 0;
	test->kind = kind;
	test->qual = qual;
	test->object = FcObjectFromName ((const char *) field);
	test->op = compare;
	test->expr = expr;
	o = FcNameGetObjectType (FcObjectName (test->object));
	if (o)
	    FcTypecheckExpr (parse, expr, o->type);
    }
    return test;
}

static FcEdit *
FcEditCreate (FcConfigParse	*parse,
	      FcObject		object,
	      FcOp		op,
	      FcExpr		*expr,
	      FcValueBinding	binding)
{
    FcEdit *e = (FcEdit *) malloc (sizeof (FcEdit));

    if (e)
    {
	const FcObjectType	*o;

	e->next = 0;
	e->object = object;
	e->op = op;
	e->expr = expr;
	e->binding = binding;
	o = FcNameGetObjectType (FcObjectName (e->object));
	if (o)
	    FcTypecheckExpr (parse, expr, o->type);
    }
    return e;
}

static FcVStack *
FcVStackCreateAndPush (FcConfigParse *parse)
{
    FcVStack    *new;

    if (parse->vstack_static_used < sizeof (parse->vstack_static) / sizeof (parse->vstack_static[0]))
	new = &parse->vstack_static[parse->vstack_static_used++];
    else
    {
	new = malloc (sizeof (FcVStack));
	if (!new)
	    return 0;
	FcMemAlloc (FC_MEM_VSTACK, sizeof (FcVStack));
    }
    new->tag = FcVStackNone;
    new->prev = 0;

    new->prev = parse->vstack;
    new->pstack = parse->pstack ? parse->pstack->prev : 0;
    parse->vstack = new;

    return new;
}

static FcBool
FcVStackPushString (FcConfigParse *parse, FcVStackTag tag, FcChar8 *string)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.string = string;
    vstack->tag = tag;
    return FcTrue;
}

static FcBool
FcVStackPushInteger (FcConfigParse *parse, int integer)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.integer = integer;
    vstack->tag = FcVStackInteger;
    return FcTrue;
}

static FcBool
FcVStackPushDouble (FcConfigParse *parse, double _double)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u._double = _double;
    vstack->tag = FcVStackDouble;
    return FcTrue;
}

static FcBool
FcVStackPushMatrix (FcConfigParse *parse, FcMatrix *matrix)
{
    FcVStack    *vstack;
    matrix = FcMatrixCopy (matrix);
    if (!matrix)
	return FcFalse;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.matrix = matrix;
    vstack->tag = FcVStackMatrix;
    return FcTrue;
}

static FcBool
FcVStackPushRange (FcConfigParse *parse, FcRange *range)
{
    FcVStack	*vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.range.begin = range->begin;
    vstack->u.range.end = range->end;
    vstack->tag = FcVStackRange;
    return FcTrue;
}

static FcBool
FcVStackPushBool (FcConfigParse *parse, FcBool bool_)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.bool_ = bool_;
    vstack->tag = FcVStackBool;
    return FcTrue;
}

static FcBool
FcVStackPushCharSet (FcConfigParse *parse, FcCharSet *charset)
{
    FcVStack	*vstack;
    if (!charset)
	return FcFalse;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.charset = charset;
    vstack->tag = FcVStackCharSet;
    return FcTrue;
}

static FcBool
FcVStackPushLangSet (FcConfigParse *parse, FcLangSet *langset)
{
    FcVStack	*vstack;
    if (!langset)
	return FcFalse;
    vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.langset = langset;
    vstack->tag = FcVStackLangSet;
    return FcTrue;
}

static FcBool
FcVStackPushTest (FcConfigParse *parse, FcTest *test)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.test = test;
    vstack->tag = FcVStackTest;
    return FcTrue;
}

static FcBool
FcVStackPushExpr (FcConfigParse *parse, FcVStackTag tag, FcExpr *expr)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.expr = expr;
    vstack->tag = tag;
    return FcTrue;
}

static FcBool
FcVStackPushEdit (FcConfigParse *parse, FcEdit *edit)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.edit = edit;
    vstack->tag = FcVStackEdit;
    return FcTrue;
}

static FcBool
FcVStackPushPattern (FcConfigParse *parse, FcPattern *pattern)
{
    FcVStack    *vstack = FcVStackCreateAndPush (parse);
    if (!vstack)
	return FcFalse;
    vstack->u.pattern = pattern;
    vstack->tag = FcVStackPattern;
    return FcTrue;
}

static FcVStack *
FcVStackFetch (FcConfigParse *parse, int off)
{
    FcVStack    *vstack;

    for (vstack = parse->vstack; vstack && off-- > 0; vstack = vstack->prev);
    return vstack;
}

static FcVStack *
FcVStackPeek (FcConfigParse *parse)
{
    FcVStack	*vstack = parse->vstack;

    return vstack && vstack->pstack == parse->pstack ? vstack : 0;
}

static void
FcVStackPopAndDestroy (FcConfigParse *parse)
{
    FcVStack	*vstack = parse->vstack;

    if (!vstack || vstack->pstack != parse->pstack)
	return;

    parse->vstack = vstack->prev;

    switch (vstack->tag) {
    case FcVStackNone:
	break;
    case FcVStackFamily:
	break;
    case FcVStackString:
    case FcVStackField:
    case FcVStackConstant:
    case FcVStackGlob:
	FcStrFree (vstack->u.string);
	break;
    case FcVStackPattern:
	FcPatternDestroy (vstack->u.pattern);
	break;
    case FcVStackInteger:
    case FcVStackDouble:
	break;
    case FcVStackMatrix:
	FcMatrixFree (vstack->u.matrix);
	break;
    case FcVStackRange:
    case FcVStackBool:
	break;
    case FcVStackCharSet:
	FcCharSetDestroy (vstack->u.charset);
	break;
    case FcVStackLangSet:
	FcLangSetDestroy (vstack->u.langset);
	break;
    case FcVStackTest:
	FcTestDestroy (vstack->u.test);
	break;
    case FcVStackExpr:
    case FcVStackPrefer:
    case FcVStackAccept:
    case FcVStackDefault:
	FcExprDestroy (vstack->u.expr);
	break;
    case FcVStackEdit:
	FcEditDestroy (vstack->u.edit);
	break;
    }

    if (vstack == &parse->vstack_static[parse->vstack_static_used - 1])
	parse->vstack_static_used--;
    else
    {
	FcMemFree (FC_MEM_VSTACK, sizeof (FcVStack));
	free (vstack);
    }
}

static void
FcVStackClear (FcConfigParse *parse)
{
    while (FcVStackPeek (parse))
	FcVStackPopAndDestroy (parse);
}

static int
FcVStackElements (FcConfigParse *parse)
{
    int		h = 0;
    FcVStack	*vstack = parse->vstack;
    while (vstack && vstack->pstack == parse->pstack)
    {
	h++;
	vstack = vstack->prev;
    }
    return h;
}

static FcChar8 **
FcConfigSaveAttr (const XML_Char **attr, FcChar8 **buf, int size_bytes)
{
    int		slen;
    int		i;
    FcChar8	**new;
    FcChar8	*s;

    if (!attr)
	return 0;
    slen = 0;
    for (i = 0; attr[i]; i++)
	slen += strlen ((char *) attr[i]) + 1;
    if (i == 0)
	return 0;
    slen += (i + 1) * sizeof (FcChar8 *);
    if (slen <= size_bytes)
	new = buf;
    else
    {
	new = malloc (slen);
	if (!new)
	{
	    FcConfigMessage (0, FcSevereError, "out of memory");
	    return 0;
	}
	FcMemAlloc (FC_MEM_ATTR, 1);    /* size is too expensive */
    }
    s = (FcChar8 *) (new + (i + 1));
    for (i = 0; attr[i]; i++)
    {
	new[i] = s;
	strcpy ((char *) s, (char *) attr[i]);
	s += strlen ((char *) s) + 1;
    }
    new[i] = 0;
    return new;
}

static FcBool
FcPStackPush (FcConfigParse *parse, FcElement element, const XML_Char **attr)
{
    FcPStack   *new;

    if (parse->pstack_static_used < sizeof (parse->pstack_static) / sizeof (parse->pstack_static[0]))
	new = &parse->pstack_static[parse->pstack_static_used++];
    else
    {
	new = malloc (sizeof (FcPStack));
	if (!new)
	    return FcFalse;
	FcMemAlloc (FC_MEM_PSTACK, sizeof (FcPStack));
    }

    new->prev = parse->pstack;
    new->element = element;
    new->attr = FcConfigSaveAttr (attr, new->attr_buf_static, sizeof (new->attr_buf_static));
    FcStrBufInit (&new->str, 0, 0);
    parse->pstack = new;
    return FcTrue;
}

static FcBool
FcPStackPop (FcConfigParse *parse)
{
    FcPStack   *old;

    if (!parse->pstack)
    {
	FcConfigMessage (parse, FcSevereError, "mismatching element");
	return FcFalse;
    }
    FcVStackClear (parse);
    old = parse->pstack;
    parse->pstack = old->prev;
    FcStrBufDestroy (&old->str);
    if (old->attr && old->attr != old->attr_buf_static)
    {
	FcMemFree (FC_MEM_ATTR, 1); /* size is to expensive */
	free (old->attr);
    }

    if (old == &parse->pstack_static[parse->pstack_static_used - 1])
	parse->pstack_static_used--;
    else
    {
	FcMemFree (FC_MEM_PSTACK, sizeof (FcPStack));
	free (old);
    }
    return FcTrue;
}

static FcBool
FcConfigInit (FcConfigParse *parse, const FcChar8 *name, FcConfig *config, XML_Parser parser)
{
    parse->pstack = 0;
    parse->pstack_static_used = 0;
    parse->vstack = 0;
    parse->vstack_static_used = 0;
    parse->error = FcFalse;
    parse->name = name;
    parse->config = config;
    parse->parser = parser;
    return FcTrue;
}

static void
FcConfigCleanup (FcConfigParse	*parse)
{
    while (parse->pstack)
	FcPStackPop (parse);
}

static const FcChar8 *
FcConfigGetAttribute (FcConfigParse *parse, const char *attr)
{
    FcChar8 **attrs;
    if (!parse->pstack)
	return 0;

    attrs = parse->pstack->attr;
    if (!attrs)
        return 0;

    while (*attrs)
    {
	if (!strcmp ((char *) *attrs, attr))
	    return attrs[1];
	attrs += 2;
    }
    return 0;
}

static void
FcStartElement(void *userData, const XML_Char *name, const XML_Char **attr)
{
    FcConfigParse   *parse = userData;
    FcElement	    element;

    element = FcElementMap (name);
    if (element == FcElementUnknown)
	FcConfigMessage (parse, FcSevereWarning, "unknown element \"%s\"", name);

    if (!FcPStackPush (parse, element, attr))
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    return;
}

static void
FcParseBlank (FcConfigParse *parse)
{
    int		n = FcVStackElements (parse);
    FcChar32	i;
    while (n-- > 0)
    {
	FcVStack    *v = FcVStackFetch (parse, n);
	if (!parse->config->blanks)
	{
	    parse->config->blanks = FcBlanksCreate ();
	    if (!parse->config->blanks)
		goto bail;
	}
	switch (v->tag) {
	case FcVStackInteger:
	    if (!FcBlanksAdd (parse->config->blanks, v->u.integer))
		goto bail;
	    break;
	case FcVStackRange:
	    if (v->u.range.begin <= v->u.range.end)
	    {
	      for (i = v->u.range.begin; i <= v->u.range.end; i++)
	      {
		  if (!FcBlanksAdd (parse->config->blanks, i))
		      goto bail;
	      }
	    }
	    break;
	default:
	    FcConfigMessage (parse, FcSevereError, "invalid element in blank");
	    break;
	}
    }
    return;
  bail:
    FcConfigMessage (parse, FcSevereError, "out of memory");
}

static void
FcParseRescan (FcConfigParse *parse)
{
    int	    n = FcVStackElements (parse);
    while (n-- > 0)
    {
	FcVStack    *v = FcVStackFetch (parse, n);
	if (v->tag != FcVStackInteger)
	    FcConfigMessage (parse, FcSevereWarning, "non-integer rescan");
	else
	    parse->config->rescanInterval = v->u.integer;
    }
}

static void
FcParseInt (FcConfigParse *parse)
{
    FcChar8 *s, *end;
    int	    l;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    end = 0;
    l = (int) strtol ((char *) s, (char **)&end, 0);
    if (end != s + strlen ((char *) s))
	FcConfigMessage (parse, FcSevereError, "\"%s\": not a valid integer", s);
    else
	FcVStackPushInteger (parse, l);
    FcStrBufDestroy (&parse->pstack->str);
}

/*
 * idea copied from glib g_ascii_strtod with
 * permission of the author (Alexander Larsson)
 */

#include <locale.h>

static double
FcStrtod (char *s, char **end)
{
    struct lconv    *locale_data;
    char	    *dot;
    double	    v;

    /*
     * Have to swap the decimal point to match the current locale
     * if that locale doesn't use 0x2e
     */
    if ((dot = strchr (s, 0x2e)) &&
	(locale_data = localeconv ()) &&
	(locale_data->decimal_point[0] != 0x2e ||
	 locale_data->decimal_point[1] != 0))
    {
	char	buf[128];
	int	slen = strlen (s);
	int	dlen = strlen (locale_data->decimal_point);
	
	if (slen + dlen > (int) sizeof (buf))
	{
	    if (end)
		*end = s;
	    v = 0;
	}
	else
	{
	    char	*buf_end;
	    /* mantissa */
	    strncpy (buf, s, dot - s);
	    /* decimal point */
	    strcpy (buf + (dot - s), locale_data->decimal_point);
	    /* rest of number */
	    strcpy (buf + (dot - s) + dlen, dot + 1);
	    buf_end = 0;
	    v = strtod (buf, &buf_end);
	    if (buf_end) {
		buf_end = s + (buf_end - buf);
		if (buf_end > dot)
		    buf_end -= dlen - 1;
	    }
	    if (end)
		*end = buf_end;
	}
    }
    else
	v = strtod (s, end);
    return v;
}

static void
FcParseDouble (FcConfigParse *parse)
{
    FcChar8 *s, *end;
    double  d;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    end = 0;
    d = FcStrtod ((char *) s, (char **)&end);
    if (end != s + strlen ((char *) s))
	FcConfigMessage (parse, FcSevereError, "\"%s\": not a valid double", s);
    else
	FcVStackPushDouble (parse, d);
    FcStrBufDestroy (&parse->pstack->str);
}

static void
FcParseString (FcConfigParse *parse, FcVStackTag tag)
{
    FcChar8 *s;

    if (!parse->pstack)
	return;
    s = FcStrBufDone (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    if (!FcVStackPushString (parse, tag, s))
	FcStrFree (s);
}

static void
FcParseMatrix (FcConfigParse *parse)
{
    FcVStack	*vstack;
    enum { m_done, m_xx, m_xy, m_yx, m_yy } matrix_state = m_yy;
    FcMatrix	m;

    while ((vstack = FcVStackPeek (parse)))
    {
	double	v;
	switch (vstack->tag) {
	case FcVStackInteger:
	    v = vstack->u.integer;
	    break;
	case FcVStackDouble:
	    v = vstack->u._double;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereError, "non-double matrix element");
	    v = 1.0;
	    break;
	}
	switch (matrix_state) {
	case m_xx: m.xx = v; break;
	case m_xy: m.xy = v; break;
	case m_yx: m.yx = v; break;
	case m_yy: m.yy = v; break;
	default: break;
	}
	FcVStackPopAndDestroy (parse);
	matrix_state--;
    }
    if (matrix_state != m_done)
	FcConfigMessage (parse, FcSevereError, "wrong number of matrix elements");
    else
	FcVStackPushMatrix (parse, &m);
}

static void
FcParseRange (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcRange	r;
    FcChar32	n;
    int		count = 1;

    while ((vstack = FcVStackPeek (parse)))
    {
	if (count < 0)
	{
	    FcConfigMessage (parse, FcSevereError, "too many elements in range");
	    return;
	}
	switch (vstack->tag) {
	case FcVStackInteger:
	    n = vstack->u.integer;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereError, "invalid element in range");
	    n = 0;
	    break;
	}
	if (count == 1)
	    r.end = n;
	else
	    r.begin = n;
	count--;
	FcVStackPopAndDestroy (parse);
    }
    if (count < 0)
    {
	if (r.begin > r.end)
	{
	    FcConfigMessage (parse, FcSevereError, "invalid range");
	    return;
	}
	FcVStackPushRange (parse, &r);
    }
    else
	FcConfigMessage (parse, FcSevereError, "invalid range");
}

static FcBool
FcConfigLexBool (FcConfigParse *parse, const FcChar8 *bool_)
{
    FcBool  result = FcFalse;

    if (!FcNameBool (bool_, &result))
	FcConfigMessage (parse, FcSevereWarning, "\"%s\" is not known boolean",
			 bool_);
    return result;
}

static void
FcParseBool (FcConfigParse *parse)
{
    FcChar8 *s;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    FcVStackPushBool (parse, FcConfigLexBool (parse, s));
    FcStrBufDestroy (&parse->pstack->str);
}

static void
FcParseCharSet (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcCharSet	*charset = FcCharSetCreate ();
    FcChar32	i;
    int n = 0;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackInteger:
	    if (!FcCharSetAddChar (charset, vstack->u.integer))
	    {
		FcConfigMessage (parse, FcSevereWarning, "invalid character: 0x%04x", vstack->u.integer);
	    }
	    else
		n++;
	    break;
	case FcVStackRange:
	    if (vstack->u.range.begin <= vstack->u.range.end)
	    {
	      for (i = vstack->u.range.begin; i <= vstack->u.range.end; i++)
	      {
		  if (!FcCharSetAddChar (charset, i))
		  {
		      FcConfigMessage (parse, FcSevereWarning, "invalid character: 0x%04x", i);
		  }
		  else
		      n++;
	      }
	    }
	    break;
	default:
		FcConfigMessage (parse, FcSevereError, "invalid element in charset");
		break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (n > 0)
	    FcVStackPushCharSet (parse, charset);
    else
	    FcCharSetDestroy (charset);
}

static void
FcParseLangSet (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcLangSet	*langset = FcLangSetCreate ();
    int n = 0;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackString:
	    if (!FcLangSetAdd (langset, vstack->u.string))
	    {
		FcConfigMessage (parse, FcSevereWarning, "invalid langset: %s", vstack->u.string);
	    }
	    else
		n++;
	    break;
	default:
		FcConfigMessage (parse, FcSevereError, "invalid element in langset");
		break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (n > 0)
	    FcVStackPushLangSet (parse, langset);
    else
	    FcLangSetDestroy (langset);
}

static FcBool
FcConfigLexBinding (FcConfigParse   *parse,
		    const FcChar8   *binding_string,
		    FcValueBinding  *binding_ret)
{
    FcValueBinding binding;

    if (!binding_string)
	binding = FcValueBindingWeak;
    else
    {
	if (!strcmp ((char *) binding_string, "weak"))
	    binding = FcValueBindingWeak;
	else if (!strcmp ((char *) binding_string, "strong"))
	    binding = FcValueBindingStrong;
	else if (!strcmp ((char *) binding_string, "same"))
	    binding = FcValueBindingSame;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid binding \"%s\"", binding_string);
	    return FcFalse;
	}
    }
    *binding_ret = binding;
    return FcTrue;
}

static void
FcParseFamilies (FcConfigParse *parse, FcVStackTag tag)
{
    FcVStack	*vstack;
    FcExpr	*left, *expr = 0, *new;

    while ((vstack = FcVStackPeek (parse)))
    {
	if (vstack->tag != FcVStackFamily)
	{
	    FcConfigMessage (parse, FcSevereWarning, "non-family");
	    FcVStackPopAndDestroy (parse);
	    continue;
	}
	left = vstack->u.expr;
	vstack->tag = FcVStackNone;
	FcVStackPopAndDestroy (parse);
	if (expr)
	{
	    new = FcExprCreateOp (parse->config, left, FcOpComma, expr);
	    if (!new)
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcExprDestroy (left);
		FcExprDestroy (expr);
		break;
	    }
	    expr = new;
	}
	else
	    expr = left;
    }
    if (expr)
    {
	if (!FcVStackPushExpr (parse, tag, expr))
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
            FcExprDestroy (expr);
	}
    }
}

static void
FcParseFamily (FcConfigParse *parse)
{
    FcChar8 *s;
    FcExpr  *expr;

    if (!parse->pstack)
	return;
    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    expr = FcExprCreateString (parse->config, s);
    FcStrBufDestroy (&parse->pstack->str);
    if (expr)
	FcVStackPushExpr (parse, FcVStackFamily, expr);
}

static void
FcParseAlias (FcConfigParse *parse)
{
    FcExpr	*family = 0, *accept = 0, *prefer = 0, *def = 0, *new = 0;
    FcEdit	*edit = 0, *next;
    FcVStack	*vstack;
    FcTest	*test = NULL;
    FcValueBinding  binding;

    if (!FcConfigLexBinding (parse, FcConfigGetAttribute (parse, "binding"), &binding))
	return;
    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackFamily:
	    if (family)
	    {
		FcConfigMessage (parse, FcSevereWarning, "Having multiple <family> in <alias> isn't supported and may not work as expected");
		new = FcExprCreateOp (parse->config, vstack->u.expr, FcOpComma, family);
		if (!new)
		    FcConfigMessage (parse, FcSevereError, "out of memory");
		else
		    family = new;
	    }
	    else
		new = vstack->u.expr;
	    if (new)
	    {
		family = new;
		vstack->tag = FcVStackNone;
	    }
	    break;
	case FcVStackPrefer:
	    if (prefer)
		FcExprDestroy (prefer);
	    prefer = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackAccept:
	    if (accept)
		FcExprDestroy (accept);
	    accept = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackDefault:
	    if (def)
		FcExprDestroy (def);
	    def = vstack->u.expr;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackTest:
	    vstack->u.test->next = test;
	    test = vstack->u.test;
	    vstack->tag = FcVStackNone;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "bad alias");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (!family)
    {
	FcConfigMessage (parse, FcSevereError, "missing family in alias");
	if (prefer)
	    FcExprDestroy (prefer);
	if (accept)
	    FcExprDestroy (accept);
	if (def)
	    FcExprDestroy (def);
	return;
    }
    if (prefer)
    {
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpPrepend,
			     prefer,
			     binding);
	if (edit)
	    edit->next = 0;
	else
	    FcExprDestroy (prefer);
    }
    if (accept)
    {
	next = edit;
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpAppend,
			     accept,
			     binding);
	if (edit)
	    edit->next = next;
	else
	    FcExprDestroy (accept);
    }
    if (def)
    {
	next = edit;
	edit = FcEditCreate (parse,
			     FC_FAMILY_OBJECT,
			     FcOpAppendLast,
			     def,
			     binding);
	if (edit)
	    edit->next = next;
	else
	    FcExprDestroy (def);
    }
    if (edit)
    {
	FcTest *t = FcTestCreate (parse, FcMatchPattern,
				  FcQualAny,
				  (FcChar8 *) FC_FAMILY,
				  FC_OP (FcOpEqual, FcOpFlagIgnoreBlanks),
				  family);
	if (test)
	{
	    FcTest *p = test;

	    while (p->next)
		p = p->next;
	    p->next = t;
	}
	else
	    test = t;
	if (test)
	    if (!FcConfigAddEdit (parse->config, test, edit, FcMatchPattern))
		FcTestDestroy (test);
    }
    else
	FcExprDestroy (family);
}

static FcExpr *
FcPopExpr (FcConfigParse *parse)
{
    FcVStack	*vstack = FcVStackPeek (parse);
    FcExpr	*expr = 0;
    if (!vstack)
	return 0;
    switch (vstack->tag) {
    case FcVStackNone:
	break;
    case FcVStackString:
    case FcVStackFamily:
	expr = FcExprCreateString (parse->config, vstack->u.string);
	break;
    case FcVStackField:
	expr = FcExprCreateField (parse->config, (char *) vstack->u.string);
	break;
    case FcVStackConstant:
	expr = FcExprCreateConst (parse->config, vstack->u.string);
	break;
    case FcVStackGlob:
	/* XXX: What's the correct action here? (CDW) */
	break;
    case FcVStackPrefer:
    case FcVStackAccept:
    case FcVStackDefault:
	expr = vstack->u.expr;
	vstack->tag = FcVStackNone;
	break;
    case FcVStackInteger:
	expr = FcExprCreateInteger (parse->config, vstack->u.integer);
	break;
    case FcVStackDouble:
	expr = FcExprCreateDouble (parse->config, vstack->u._double);
	break;
    case FcVStackMatrix:
	expr = FcExprCreateMatrix (parse->config, vstack->u.matrix);
	break;
    case FcVStackRange:
	break;
    case FcVStackBool:
	expr = FcExprCreateBool (parse->config, vstack->u.bool_);
	break;
    case FcVStackCharSet:
	expr = FcExprCreateCharSet (parse->config, vstack->u.charset);
	break;
    case FcVStackLangSet:
	expr = FcExprCreateLangSet (parse->config, vstack->u.langset);
	break;
    case FcVStackTest:
	break;
    case FcVStackExpr:
	expr = vstack->u.expr;
	vstack->tag = FcVStackNone;
	break;
    case FcVStackEdit:
	break;
    default:
	break;
    }
    FcVStackPopAndDestroy (parse);
    return expr;
}

/*
 * This builds a tree of binary operations.  Note
 * that every operator is defined so that if only
 * a single operand is contained, the value of the
 * whole expression is the value of the operand.
 *
 * This code reduces in that case to returning that
 * operand.
 */
static FcExpr *
FcPopBinary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *left, *expr = 0, *new;

    while ((left = FcPopExpr (parse)))
    {
	if (expr)
	{
	    new = FcExprCreateOp (parse->config, left, op, expr);
	    if (!new)
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcExprDestroy (left);
		FcExprDestroy (expr);
		return 0;
	    }
	    expr = new;
	}
	else
	    expr = left;
    }
    return expr;
}

static void
FcParseBinary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *expr = FcPopBinary (parse, op);
    if (expr)
	FcVStackPushExpr (parse, FcVStackExpr, expr);
}

/*
 * This builds a a unary operator, it consumes only
 * a single operand
 */

static FcExpr *
FcPopUnary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *operand, *new = 0;

    if ((operand = FcPopExpr (parse)))
    {
	new = FcExprCreateOp (parse->config, operand, op, 0);
	if (!new)
	{
	    FcExprDestroy (operand);
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	}
    }
    return new;
}

static void
FcParseUnary (FcConfigParse *parse, FcOp op)
{
    FcExpr  *expr = FcPopUnary (parse, op);
    if (expr)
	FcVStackPushExpr (parse, FcVStackExpr, expr);
}

static void
FcParseDir (FcConfigParse *parse)
{
    const FcChar8 *attr, *data;
    FcChar8 *prefix = NULL;
#ifdef _WIN32
    FcChar8         buffer[1000];
#endif

    attr = FcConfigGetAttribute (parse, "prefix");
    if (attr && FcStrCmp (attr, (const FcChar8 *)"xdg") == 0)
	prefix = FcConfigXdgDataHome ();
    data = FcStrBufDoneStatic (&parse->pstack->str);
    if (!data)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	goto bail;
    }
    if (prefix)
    {
	size_t plen = strlen ((const char *)prefix);
	size_t dlen = strlen ((const char *)data);

	FcMemFree (FC_MEM_STRING, plen + 1);
	prefix = realloc (prefix, plen + 1 + dlen + 1);
	if (!prefix)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	FcMemAlloc (FC_MEM_STRING, plen + 1 + dlen + 1);
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], data, dlen);
	prefix[plen + 1 + dlen] = 0;
	data = prefix;
    }
#ifdef _WIN32
    if (strcmp ((const char *) data, "CUSTOMFONTDIR") == 0)
    {
	char *p;
	data = buffer;
	if (!GetModuleFileName (NULL, (LPCH) buffer, sizeof (buffer) - 20))
	{
	    FcConfigMessage (parse, FcSevereError, "GetModuleFileName failed");
	    goto bail;
	}
	/*
	 * Must use the multi-byte aware function to search
	 * for backslash because East Asian double-byte code
	 * pages have characters with backslash as the second
	 * byte.
	 */
	p = _mbsrchr (data, '\\');
	if (p) *p = '\0';
	strcat (data, "\\fonts");
    }
    else if (strcmp ((const char *) data, "APPSHAREFONTDIR") == 0)
    {
	char *p;
	data = buffer;
	if (!GetModuleFileName (NULL, (LPCH) buffer, sizeof (buffer) - 20))
	{
	    FcConfigMessage (parse, FcSevereError, "GetModuleFileName failed");
	    goto bail;
	}
	p = _mbsrchr (data, '\\');
	if (p) *p = '\0';
	strcat (data, "\\..\\share\\fonts");
    }
    else if (strcmp ((const char *) data, "WINDOWSFONTDIR") == 0)
    {
	int rc;
	data = buffer;
	rc = pGetSystemWindowsDirectory ((LPSTR) buffer, sizeof (buffer) - 20);
	if (rc == 0 || rc > sizeof (buffer) - 20)
	{
	    FcConfigMessage (parse, FcSevereError, "GetSystemWindowsDirectory failed");
	    goto bail;
	}
	if (data [strlen ((const char *) data) - 1] != '\\')
	    strcat (data, "\\");
	strcat (data, "fonts");
    }
#endif
    if (strlen ((char *) data) == 0)
	FcConfigMessage (parse, FcSevereWarning, "empty font directory name ignored");
    else if (!FcStrUsesHome (data) || FcConfigHome ())
    {
	if (!FcConfigAddDir (parse->config, data))
	    FcConfigMessage (parse, FcSevereError, "out of memory; cannot add directory %s", data);
    }
    FcStrBufDestroy (&parse->pstack->str);

  bail:
    if (prefix)
	FcStrFree (prefix);
}

static void
FcParseCacheDir (FcConfigParse *parse)
{
    const FcChar8 *attr;
    FcChar8 *prefix = NULL, *data;

    attr = FcConfigGetAttribute (parse, "prefix");
    if (attr && FcStrCmp (attr, (const FcChar8 *)"xdg") == 0)
	prefix = FcConfigXdgCacheHome ();
    data = FcStrBufDone (&parse->pstack->str);
    if (!data)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	goto bail;
    }
    if (prefix)
    {
	size_t plen = strlen ((const char *)prefix);
	size_t dlen = strlen ((const char *)data);

	FcMemFree (FC_MEM_STRING, plen + 1);
	prefix = realloc (prefix, plen + 1 + dlen + 1);
	if (!prefix)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	FcMemAlloc (FC_MEM_STRING, plen + 1 + dlen + 1);
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], data, dlen);
	prefix[plen + 1 + dlen] = 0;
	FcStrFree (data);
	data = prefix;
    }
#ifdef _WIN32
    if (strcmp ((const char *) data, "WINDOWSTEMPDIR_FONTCONFIG_CACHE") == 0)
    {
	int rc;
	FcStrFree (data);
	data = malloc (1000);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	FcMemAlloc (FC_MEM_STRING, 1000);
	rc = GetTempPath (800, (LPSTR) data);
	if (rc == 0 || rc > 800)
	{
	    FcConfigMessage (parse, FcSevereError, "GetTempPath failed");
	    goto bail;
	}
	if (data [strlen ((const char *) data) - 1] != '\\')
	    strcat (data, "\\");
	strcat (data, "fontconfig\\cache");
    }
    else if (strcmp ((const char *) data, "LOCAL_APPDATA_FONTCONFIG_CACHE") == 0)
    {
	char szFPath[MAX_PATH + 1];
	size_t len;

	if (!(pSHGetFolderPathA && SUCCEEDED(pSHGetFolderPathA(NULL, /* CSIDL_LOCAL_APPDATA */ 28, NULL, 0, szFPath))))
	{
	    FcConfigMessage (parse, FcSevereError, "SHGetFolderPathA failed");
	    goto bail;
	}
	strncat(szFPath, "\\fontconfig\\cache", MAX_PATH - 1 - strlen(szFPath));
	len = strlen(szFPath) + 1;
	FcStrFree (data);
	data = malloc(len);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	FcMemAlloc (FC_MEM_STRING, len);
	strncpy((char *) data, szFPath, len);
    }
#endif
    if (strlen ((char *) data) == 0)
	FcConfigMessage (parse, FcSevereWarning, "empty cache directory name ignored");
    else if (!FcStrUsesHome (data) || FcConfigHome ())
    {
	if (!FcConfigAddCacheDir (parse->config, data))
	    FcConfigMessage (parse, FcSevereError, "out of memory; cannot add cache directory %s", data);
    }
    FcStrBufDestroy (&parse->pstack->str);

  bail:
    if (data)
	FcStrFree (data);
}

static void
FcParseInclude (FcConfigParse *parse)
{
    FcChar8	    *s;
    const FcChar8   *attr;
    FcBool	    ignore_missing = FcFalse;
    FcBool	    deprecated = FcFalse;
    FcChar8	    *prefix = NULL;

    s = FcStrBufDoneStatic (&parse->pstack->str);
    if (!s)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	goto bail;
    }
    attr = FcConfigGetAttribute (parse, "ignore_missing");
    if (attr && FcConfigLexBool (parse, (FcChar8 *) attr) == FcTrue)
	ignore_missing = FcTrue;
    attr = FcConfigGetAttribute (parse, "deprecated");
    if (attr && FcConfigLexBool (parse, (FcChar8 *) attr) == FcTrue)
        deprecated = FcTrue;
    attr = FcConfigGetAttribute (parse, "prefix");
    if (attr && FcStrCmp (attr, (const FcChar8 *)"xdg") == 0)
	prefix = FcConfigXdgConfigHome ();
    if (prefix)
    {
	size_t plen = strlen ((const char *)prefix);
	size_t dlen = strlen ((const char *)s);

	FcMemFree (FC_MEM_STRING, plen + 1);
	prefix = realloc (prefix, plen + 1 + dlen + 1);
	if (!prefix)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    goto bail;
	}
	FcMemAlloc (FC_MEM_STRING, plen + 1 + dlen + 1);
	prefix[plen] = FC_DIR_SEPARATOR;
	memcpy (&prefix[plen + 1], s, dlen);
	prefix[plen + 1 + dlen] = 0;
	s = prefix;
    }
    if (!FcConfigParseAndLoad (parse->config, s, !ignore_missing))
	parse->error = FcTrue;
    else
    {
        FcChar8 *filename;

        filename = FcConfigFilename(s);
        if ((deprecated == FcTrue) && filename)
        {
            FcConfigMessage (parse, FcSevereWarning, "reading configurations from %s is deprecated.", s);
        }
        if(filename)
            FcStrFree(filename);
    }
    FcStrBufDestroy (&parse->pstack->str);

  bail:
    if (prefix)
	FcStrFree (prefix);
}

typedef struct _FcOpMap {
    char    name[16];
    FcOp    op;
} FcOpMap;

static FcOp
FcConfigLexOp (const FcChar8 *op, const FcOpMap	*map, int nmap)
{
    int	i;

    for (i = 0; i < nmap; i++)
	if (!strcmp ((char *) op, map[i].name))
	    return map[i].op;
    return FcOpInvalid;
}

static const FcOpMap fcCompareOps[] = {
    { "eq",		FcOpEqual	    },
    { "not_eq",		FcOpNotEqual	    },
    { "less",		FcOpLess	    },
    { "less_eq",	FcOpLessEqual	    },
    { "more",		FcOpMore	    },
    { "more_eq",	FcOpMoreEqual	    },
    { "contains",	FcOpContains	    },
    { "not_contains",	FcOpNotContains	    }
};

#define NUM_COMPARE_OPS	(int) (sizeof fcCompareOps / sizeof fcCompareOps[0])

static FcOp
FcConfigLexCompare (const FcChar8 *compare)
{
    return FcConfigLexOp (compare, fcCompareOps, NUM_COMPARE_OPS);
}

static void
FcParseTest (FcConfigParse *parse)
{
    const FcChar8   *kind_string;
    FcMatchKind	    kind;
    const FcChar8   *qual_string;
    FcQual	    qual;
    const FcChar8   *name;
    const FcChar8   *compare_string;
    FcOp	    compare;
    FcExpr	    *expr;
    FcTest	    *test;
    const FcChar8   *iblanks_string;
    int              flags = 0;

    kind_string = FcConfigGetAttribute (parse, "target");
    if (!kind_string)
	kind = FcMatchDefault;
    else
    {
	if (!strcmp ((char *) kind_string, "pattern"))
	    kind = FcMatchPattern;
	else if (!strcmp ((char *) kind_string, "font"))
	    kind = FcMatchFont;
	else if (!strcmp ((char *) kind_string, "scan"))
	    kind = FcMatchScan;
	else if (!strcmp ((char *) kind_string, "default"))
	    kind = FcMatchDefault;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test target \"%s\"", kind_string);
	    return;
	}
    }
    qual_string = FcConfigGetAttribute (parse, "qual");
    if (!qual_string)
	qual = FcQualAny;
    else
    {
	if (!strcmp ((char *) qual_string, "any"))
	    qual = FcQualAny;
	else if (!strcmp ((char *) qual_string, "all"))
	    qual = FcQualAll;
	else if (!strcmp ((char *) qual_string, "first"))
	    qual = FcQualFirst;
	else if (!strcmp ((char *) qual_string, "not_first"))
	    qual = FcQualNotFirst;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test qual \"%s\"", qual_string);
	    return;
	}
    }
    name = FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing test name");
	return;
    }
    compare_string = FcConfigGetAttribute (parse, "compare");
    if (!compare_string)
	compare = FcOpEqual;
    else
    {
	compare = FcConfigLexCompare (compare_string);
	if (compare == FcOpInvalid)
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid test compare \"%s\"", compare_string);
	    return;
	}
    }
    iblanks_string = FcConfigGetAttribute (parse, "ignore-blanks");
    if (iblanks_string)
    {
	FcBool f = FcFalse;

	if (!FcNameBool (iblanks_string, &f))
	{
	    FcConfigMessage (parse,
			     FcSevereWarning,
			     "invalid test ignore-blanks \"%s\"", iblanks_string);
	}
	if (f)
	    flags |= FcOpFlagIgnoreBlanks;
    }
    expr = FcPopBinary (parse, FcOpComma);
    if (!expr)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing test expression");
	return;
    }
    if (expr->op == FcOpComma)
    {
	FcConfigMessage (parse, FcSevereWarning, "Having multiple values in <test> isn't supported and may not work as expected");
    }
    test = FcTestCreate (parse, kind, qual, name, FC_OP (compare, flags), expr);
    if (!test)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
    FcVStackPushTest (parse, test);
}

static const FcOpMap fcModeOps[] = {
    { "assign",		FcOpAssign	    },
    { "assign_replace",	FcOpAssignReplace   },
    { "prepend",	FcOpPrepend	    },
    { "prepend_first",	FcOpPrependFirst    },
    { "append",		FcOpAppend	    },
    { "append_last",	FcOpAppendLast	    },
};

#define NUM_MODE_OPS (int) (sizeof fcModeOps / sizeof fcModeOps[0])

static FcOp
FcConfigLexMode (const FcChar8 *mode)
{
    return FcConfigLexOp (mode, fcModeOps, NUM_MODE_OPS);
}

static void
FcParseEdit (FcConfigParse *parse)
{
    const FcChar8   *name;
    const FcChar8   *mode_string;
    FcOp	    mode;
    FcValueBinding  binding;
    FcExpr	    *expr;
    FcEdit	    *edit;

    name = FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing edit name");
	return;
    }
    mode_string = FcConfigGetAttribute (parse, "mode");
    if (!mode_string)
	mode = FcOpAssign;
    else
    {
	mode = FcConfigLexMode (mode_string);
	if (mode == FcOpInvalid)
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid edit mode \"%s\"", mode_string);
	    return;
	}
    }
    if (!FcConfigLexBinding (parse, FcConfigGetAttribute (parse, "binding"), &binding))
	return;

    expr = FcPopBinary (parse, FcOpComma);
    edit = FcEditCreate (parse, FcObjectFromName ((char *) name),
			 mode, expr, binding);
    if (!edit)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	FcExprDestroy (expr);
	return;
    }
    if (!FcVStackPushEdit (parse, edit))
	FcEditDestroy (edit);
}

static void
FcParseMatch (FcConfigParse *parse)
{
    const FcChar8   *kind_name;
    FcMatchKind	    kind;
    FcTest	    *test = 0;
    FcEdit	    *edit = 0;
    FcVStack	    *vstack;

    kind_name = FcConfigGetAttribute (parse, "target");
    if (!kind_name)
	kind = FcMatchPattern;
    else
    {
	if (!strcmp ((char *) kind_name, "pattern"))
	    kind = FcMatchPattern;
	else if (!strcmp ((char *) kind_name, "font"))
	    kind = FcMatchFont;
	else if (!strcmp ((char *) kind_name, "scan"))
	    kind = FcMatchScan;
	else
	{
	    FcConfigMessage (parse, FcSevereWarning, "invalid match target \"%s\"", kind_name);
	    return;
	}
    }
    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackTest:
	    vstack->u.test->next = test;
	    test = vstack->u.test;
	    vstack->tag = FcVStackNone;
	    break;
	case FcVStackEdit:
	    vstack->u.edit->next = edit;
	    edit = vstack->u.edit;
	    vstack->tag = FcVStackNone;
	    if (kind == FcMatchScan && edit->object > FC_MAX_BASE_OBJECT)
	    {
		FcConfigMessage (parse, FcSevereError,
				 "<match target=\"scan\"> cannot edit user-defined object \"%s\"",
				 FcObjectName(edit->object));
	    }
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "invalid match element");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
    if (!FcConfigAddEdit (parse->config, test, edit, kind))
	FcConfigMessage (parse, FcSevereError, "out of memory");
}

static void
FcParseAcceptRejectFont (FcConfigParse *parse, FcElement element)
{
    FcVStack	*vstack;

    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackGlob:
	    if (!FcConfigGlobAdd (parse->config,
				  vstack->u.string,
				  element == FcElementAcceptfont))
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
	    }
	    break;
	case FcVStackPattern:
	    if (!FcConfigPatternsAdd (parse->config,
				      vstack->u.pattern,
				      element == FcElementAcceptfont))
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
	    }
	    else
		vstack->tag = FcVStackNone;
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "bad font selector");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }
}


static FcValue
FcPopValue (FcConfigParse *parse)
{
    FcVStack	*vstack = FcVStackPeek (parse);
    FcValue	value;

    value.type = FcTypeVoid;

    if (!vstack)
	return value;

    switch (vstack->tag) {
    case FcVStackString:
	value.u.s = FcSharedStr (vstack->u.string);
	if (value.u.s)
	    value.type = FcTypeString;
	break;
    case FcVStackConstant:
	if (FcNameConstant (vstack->u.string, &value.u.i))
	    value.type = FcTypeInteger;
	break;
    case FcVStackInteger:
	value.u.i = vstack->u.integer;
	value.type = FcTypeInteger;
	break;
    case FcVStackDouble:
	value.u.d = vstack->u._double;
	value.type = FcTypeInteger;
	break;
    case FcVStackMatrix:
	value.u.m = FcMatrixCopy (vstack->u.matrix);
	if (value.u.m)
	    value.type = FcTypeMatrix;
	break;
    case FcVStackBool:
	value.u.b = vstack->u.bool_;
	value.type = FcTypeBool;
	break;
    case FcVStackCharSet:
	value.u.c = FcCharSetCopy (vstack->u.charset);
	if (value.u.c)
	    value.type = FcTypeCharSet;
	break;
    case FcVStackLangSet:
	value.u.l = FcLangSetCopy (vstack->u.langset);
	if (value.u.l)
	    value.type = FcTypeLangSet;
	break;
    default:
	FcConfigMessage (parse, FcSevereWarning, "unknown pattern element %d",
			 vstack->tag);
	break;
    }
    FcVStackPopAndDestroy (parse);

    return value;
}

static void
FcParsePatelt (FcConfigParse *parse)
{
    FcValue	value;
    FcPattern	*pattern = FcPatternCreate ();
    const char	*name;

    if (!pattern)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }

    name = (char *) FcConfigGetAttribute (parse, "name");
    if (!name)
    {
	FcConfigMessage (parse, FcSevereWarning, "missing pattern element name");
	FcPatternDestroy (pattern);
	return;
    }

    for (;;)
    {
	value = FcPopValue (parse);
	if (value.type == FcTypeVoid)
	    break;
	if (!FcPatternAdd (pattern, name, value, FcTrue))
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
            FcValueDestroy(value);
	    break;
	}
        FcValueDestroy(value);
    }

    FcVStackPushPattern (parse, pattern);
}

static void
FcParsePattern (FcConfigParse *parse)
{
    FcVStack	*vstack;
    FcPattern	*pattern = FcPatternCreate ();

    if (!pattern)
    {
	FcConfigMessage (parse, FcSevereError, "out of memory");
	return;
    }
	
    while ((vstack = FcVStackPeek (parse)))
    {
	switch (vstack->tag) {
	case FcVStackPattern:
	    if (!FcPatternAppend (pattern, vstack->u.pattern))
	    {
		FcConfigMessage (parse, FcSevereError, "out of memory");
		FcPatternDestroy (pattern);
		return;
	    }
	    break;
	default:
	    FcConfigMessage (parse, FcSevereWarning, "unknown pattern element");
	    break;
	}
	FcVStackPopAndDestroy (parse);
    }

    FcVStackPushPattern (parse, pattern);
}

static void
FcEndElement(void *userData, const XML_Char *name)
{
    FcConfigParse   *parse = userData;
    FcChar8	    *data;

    if (!parse->pstack)
	return;
    switch (parse->pstack->element) {
    case FcElementNone:
	break;
    case FcElementFontconfig:
	break;
    case FcElementDir:
	FcParseDir (parse);
	break;
    case FcElementCacheDir:
	FcParseCacheDir (parse);
	break;
    case FcElementCache:
	data = FcStrBufDoneStatic (&parse->pstack->str);
	if (!data)
	{
	    FcConfigMessage (parse, FcSevereError, "out of memory");
	    break;
	}
	/* discard this data; no longer used */
	FcStrBufDestroy (&parse->pstack->str);
	break;
    case FcElementInclude:
	FcParseInclude (parse);
	break;
    case FcElementConfig:
	break;
    case FcElementMatch:
	FcParseMatch (parse);
	break;
    case FcElementAlias:
	FcParseAlias (parse);
	break;

    case FcElementBlank:
	FcParseBlank (parse);
	break;
    case FcElementRescan:
	FcParseRescan (parse);
	break;
	
    case FcElementPrefer:
	FcParseFamilies (parse, FcVStackPrefer);
	break;
    case FcElementAccept:
	FcParseFamilies (parse, FcVStackAccept);
	break;
    case FcElementDefault:
	FcParseFamilies (parse, FcVStackDefault);
	break;
    case FcElementFamily:
	FcParseFamily (parse);
	break;

    case FcElementTest:
	FcParseTest (parse);
	break;
    case FcElementEdit:
	FcParseEdit (parse);
	break;

    case FcElementInt:
	FcParseInt (parse);
	break;
    case FcElementDouble:
	FcParseDouble (parse);
	break;
    case FcElementString:
	FcParseString (parse, FcVStackString);
	break;
    case FcElementMatrix:
	FcParseMatrix (parse);
	break;
    case FcElementRange:
	FcParseRange (parse);
	break;
    case FcElementBool:
	FcParseBool (parse);
	break;
    case FcElementCharSet:
	FcParseCharSet (parse);
	break;
    case FcElementLangSet:
	FcParseLangSet (parse);
	break;
    case FcElementSelectfont:
	break;
    case FcElementAcceptfont:
    case FcElementRejectfont:
	FcParseAcceptRejectFont (parse, parse->pstack->element);
	break;
    case FcElementGlob:
	FcParseString (parse, FcVStackGlob);
	break;
    case FcElementPattern:
	FcParsePattern (parse);
	break;
    case FcElementPatelt:
	FcParsePatelt (parse);
	break;
    case FcElementName:
	FcParseString (parse, FcVStackField);
	break;
    case FcElementConst:
	FcParseString (parse, FcVStackConstant);
	break;
    case FcElementOr:
	FcParseBinary (parse, FcOpOr);
	break;
    case FcElementAnd:
	FcParseBinary (parse, FcOpAnd);
	break;
    case FcElementEq:
	FcParseBinary (parse, FcOpEqual);
	break;
    case FcElementNotEq:
	FcParseBinary (parse, FcOpNotEqual);
	break;
    case FcElementLess:
	FcParseBinary (parse, FcOpLess);
	break;
    case FcElementLessEq:
	FcParseBinary (parse, FcOpLessEqual);
	break;
    case FcElementMore:
	FcParseBinary (parse, FcOpMore);
	break;
    case FcElementMoreEq:
	FcParseBinary (parse, FcOpMoreEqual);
	break;
    case FcElementContains:
	FcParseBinary (parse, FcOpContains);
	break;
    case FcElementNotContains:
	FcParseBinary (parse, FcOpNotContains);
	break;
    case FcElementPlus:
	FcParseBinary (parse, FcOpPlus);
	break;
    case FcElementMinus:
	FcParseBinary (parse, FcOpMinus);
	break;
    case FcElementTimes:
	FcParseBinary (parse, FcOpTimes);
	break;
    case FcElementDivide:
	FcParseBinary (parse, FcOpDivide);
	break;
    case FcElementNot:
	FcParseUnary (parse, FcOpNot);
	break;
    case FcElementIf:
	FcParseBinary (parse, FcOpQuest);
	break;
    case FcElementFloor:
	FcParseUnary (parse, FcOpFloor);
	break;
    case FcElementCeil:
	FcParseUnary (parse, FcOpCeil);
	break;
    case FcElementRound:
	FcParseUnary (parse, FcOpRound);
	break;
    case FcElementTrunc:
	FcParseUnary (parse, FcOpTrunc);
	break;
    case FcElementUnknown:
	break;
    }
    (void) FcPStackPop (parse);
}

static void
FcCharacterData (void *userData, const XML_Char *s, int len)
{
    FcConfigParse   *parse = userData;

    if (!parse->pstack)
	return;
    if (!FcStrBufData (&parse->pstack->str, (FcChar8 *) s, len))
	FcConfigMessage (parse, FcSevereError, "out of memory");
}

static void
FcStartDoctypeDecl (void	    *userData,
		    const XML_Char  *doctypeName,
		    const XML_Char  *sysid,
		    const XML_Char  *pubid,
		    int		    has_internal_subset)
{
    FcConfigParse   *parse = userData;

    if (strcmp ((char *) doctypeName, "fontconfig") != 0)
	FcConfigMessage (parse, FcSevereError, "invalid doctype \"%s\"", doctypeName);
}

#ifdef ENABLE_LIBXML2

static void
FcInternalSubsetDecl (void            *userData,
		      const XML_Char  *doctypeName,
		      const XML_Char  *sysid,
		      const XML_Char  *pubid)
{
    FcStartDoctypeDecl (userData, doctypeName, sysid, pubid, 1);
}

static void
FcExternalSubsetDecl (void            *userData,
		      const XML_Char  *doctypeName,
		      const XML_Char  *sysid,
		      const XML_Char  *pubid)
{
    FcStartDoctypeDecl (userData, doctypeName, sysid, pubid, 0);
}

#else /* ENABLE_LIBXML2 */

static void
FcEndDoctypeDecl (void *userData)
{
}

#endif /* ENABLE_LIBXML2 */

static int
FcSortCmpStr (const void *a, const void *b)
{
    const FcChar8    *as = *((FcChar8 **) a);
    const FcChar8    *bs = *((FcChar8 **) b);
    return FcStrCmp (as, bs);
}

static FcBool
FcConfigParseAndLoadDir (FcConfig	*config,
			 const FcChar8	*name,
			 const FcChar8	*dir,
			 FcBool		complain)
{
    DIR		    *d;
    struct dirent   *e;
    FcBool	    ret = FcTrue;
    FcChar8	    *file;
    FcChar8	    *base;
    FcStrSet	    *files;

    d = opendir ((char *) dir);
    if (!d)
    {
	if (complain)
	    FcConfigMessage (0, FcSevereError, "Cannot open config dir \"%s\"",
			     name);
	ret = FcFalse;
	goto bail0;
    }
    /* freed below */
    file = (FcChar8 *) malloc (strlen ((char *) dir) + 1 + FC_MAX_FILE_LEN + 1);
    if (!file)
    {
	ret = FcFalse;
	goto bail1;
    }

    strcpy ((char *) file, (char *) dir);
    strcat ((char *) file, "/");
    base = file + strlen ((char *) file);

    files = FcStrSetCreate ();
    if (!files)
    {
	ret = FcFalse;
	goto bail2;
    }

    if (FcDebug () & FC_DBG_CONFIG)
	printf ("\tScanning config dir %s\n", dir);
	
    while (ret && (e = readdir (d)))
    {
	int d_len;
#define TAIL	    ".conf"
#define TAIL_LEN    5
	/*
	 * Add all files of the form [0-9]*.conf
	 */
	if ('0' <= e->d_name[0] && e->d_name[0] <= '9' &&
	    (d_len = strlen (e->d_name)) < FC_MAX_FILE_LEN &&
	    d_len > TAIL_LEN &&
	    strcmp (e->d_name + d_len - TAIL_LEN, TAIL) == 0)
	{
	    strcpy ((char *) base, (char *) e->d_name);
	    if (!FcStrSetAdd (files, file))
	    {
		ret = FcFalse;
		goto bail3;
	    }
	}
    }
    if (ret)
    {
	int i;
	qsort (files->strs, files->num, sizeof (FcChar8 *),
	       (int (*)(const void *, const void *)) FcSortCmpStr);
	for (i = 0; ret && i < files->num; i++)
	    ret = FcConfigParseAndLoad (config, files->strs[i], complain);
    }
bail3:
    FcStrSetDestroy (files);
bail2:
    free (file);
bail1:
    closedir (d);
bail0:
    return ret || !complain;
}

#ifdef _WIN32
pfnGetSystemWindowsDirectory pGetSystemWindowsDirectory = NULL;
pfnSHGetFolderPathA pSHGetFolderPathA = NULL;
#endif

FcBool
FcConfigParseAndLoad (FcConfig	    *config,
		      const FcChar8 *name,
		      FcBool	    complain)
{

    XML_Parser	    p;
    FcChar8	    *filename;
    int		    fd;
    int		    len;
    FcConfigParse   parse;
    FcBool	    error = FcTrue;

#ifdef ENABLE_LIBXML2
    xmlSAXHandler   sax;
    char            buf[BUFSIZ];
#else
    void	    *buf;
#endif

#ifdef _WIN32
    if (!pGetSystemWindowsDirectory)
    {
        HMODULE hk32 = GetModuleHandleA("kernel32.dll");
        if (!(pGetSystemWindowsDirectory = (pfnGetSystemWindowsDirectory) GetProcAddress(hk32, "GetSystemWindowsDirectoryA")))
            pGetSystemWindowsDirectory = (pfnGetSystemWindowsDirectory) GetWindowsDirectory;
    }
    if (!pSHGetFolderPathA)
    {
        HMODULE hSh = LoadLibraryA("shfolder.dll");
        /* the check is done later, because there is no provided fallback */
        if (hSh)
            pSHGetFolderPathA = (pfnSHGetFolderPathA) GetProcAddress(hSh, "SHGetFolderPathA");
    }
#endif

    filename = FcConfigFilename (name);
    if (!filename)
	goto bail0;

    if (FcStrSetMember (config->configFiles, filename))
    {
        FcStrFree (filename);
        return FcTrue;
    }

    if (!FcStrSetAdd (config->configFiles, filename))
    {
	FcStrFree (filename);
	goto bail0;
    }

    if (FcFileIsDir (filename))
    {
	FcBool ret = FcConfigParseAndLoadDir (config, name, filename, complain);
	FcStrFree (filename);
	return ret;
    }

    if (FcDebug () & FC_DBG_CONFIG)
	printf ("\tLoading config file %s\n", filename);

    fd = open ((char *) filename, O_RDONLY);
    if (fd == -1) {
	FcStrFree (filename);
	goto bail0;
    }

#ifdef ENABLE_LIBXML2
    memset(&sax, 0, sizeof(sax));

    sax.internalSubset = FcInternalSubsetDecl;
    sax.externalSubset = FcExternalSubsetDecl;
    sax.startElement = FcStartElement;
    sax.endElement = FcEndElement;
    sax.characters = FcCharacterData;

    p = xmlCreatePushParserCtxt (&sax, &parse, NULL, 0, (const char *) filename);
#else
    p = XML_ParserCreate ("UTF-8");
#endif
    FcStrFree (filename);

    if (!p)
	goto bail1;

    if (!FcConfigInit (&parse, name, config, p))
	goto bail2;

#ifndef ENABLE_LIBXML2

    XML_SetUserData (p, &parse);

    XML_SetDoctypeDeclHandler (p, FcStartDoctypeDecl, FcEndDoctypeDecl);
    XML_SetElementHandler (p, FcStartElement, FcEndElement);
    XML_SetCharacterDataHandler (p, FcCharacterData);
	
#endif /* ENABLE_LIBXML2 */

    do {
#ifndef ENABLE_LIBXML2
	buf = XML_GetBuffer (p, BUFSIZ);
	if (!buf)
	{
	    FcConfigMessage (&parse, FcSevereError, "cannot get parse buffer");
	    goto bail3;
	}
#endif
	len = read (fd, buf, BUFSIZ);
	if (len < 0)
	{
	    FcConfigMessage (&parse, FcSevereError, "failed reading config file");
	    goto bail3;
	}

#ifdef ENABLE_LIBXML2
	if (xmlParseChunk (p, buf, len, len == 0))
#else
	if (!XML_ParseBuffer (p, len, len == 0))
#endif
	{
	    FcConfigMessage (&parse, FcSevereError, "%s",
			   XML_ErrorString (XML_GetErrorCode (p)));
	    goto bail3;
	}
    } while (len != 0);
    error = parse.error;
bail3:
    FcConfigCleanup (&parse);
bail2:
    XML_ParserFree (p);
bail1:
    close (fd);
    fd = -1;
bail0:
    if (error && complain)
    {
	if (name)
	    FcConfigMessage (0, FcSevereError, "Cannot load config file \"%s\"", name);
	else
	    FcConfigMessage (0, FcSevereError, "Cannot load default config file");
	return FcFalse;
    }
    return FcTrue;
}
#define __fcxml__
#include "fcaliastail.h"
#undef __fcxml__
