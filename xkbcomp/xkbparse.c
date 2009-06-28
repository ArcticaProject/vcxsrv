/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END_OF_FILE = 0,
     ERROR_TOK = 255,
     XKB_KEYMAP = 1,
     XKB_KEYCODES = 2,
     XKB_TYPES = 3,
     XKB_SYMBOLS = 4,
     XKB_COMPATMAP = 5,
     XKB_GEOMETRY = 6,
     XKB_SEMANTICS = 7,
     XKB_LAYOUT = 8,
     INCLUDE = 10,
     OVERRIDE = 11,
     AUGMENT = 12,
     REPLACE = 13,
     ALTERNATE = 14,
     VIRTUAL_MODS = 20,
     TYPE = 21,
     INTERPRET = 22,
     ACTION_TOK = 23,
     KEY = 24,
     ALIAS = 25,
     GROUP = 26,
     MODIFIER_MAP = 27,
     INDICATOR = 28,
     SHAPE = 29,
     KEYS = 30,
     ROW = 31,
     SECTION = 32,
     OVERLAY = 33,
     TEXT = 34,
     OUTLINE = 35,
     SOLID = 36,
     LOGO = 37,
     VIRTUAL = 38,
     EQUALS = 40,
     PLUS = 41,
     MINUS = 42,
     DIVIDE = 43,
     TIMES = 44,
     OBRACE = 45,
     CBRACE = 46,
     OPAREN = 47,
     CPAREN = 48,
     OBRACKET = 49,
     CBRACKET = 50,
     DOT = 51,
     COMMA = 52,
     SEMI = 53,
     EXCLAM = 54,
     INVERT = 55,
     STRING = 60,
     INTEGER = 61,
     FLOAT = 62,
     IDENT = 63,
     KEYNAME = 64,
     PARTIAL = 70,
     DEFAULT = 71,
     HIDDEN = 72,
     ALPHANUMERIC_KEYS = 73,
     MODIFIER_KEYS = 74,
     KEYPAD_KEYS = 75,
     FUNCTION_KEYS = 76,
     ALTERNATE_GROUP = 77
   };
#endif
/* Tokens.  */
#define END_OF_FILE 0
#define ERROR_TOK 255
#define XKB_KEYMAP 1
#define XKB_KEYCODES 2
#define XKB_TYPES 3
#define XKB_SYMBOLS 4
#define XKB_COMPATMAP 5
#define XKB_GEOMETRY 6
#define XKB_SEMANTICS 7
#define XKB_LAYOUT 8
#define INCLUDE 10
#define OVERRIDE 11
#define AUGMENT 12
#define REPLACE 13
#define ALTERNATE 14
#define VIRTUAL_MODS 20
#define TYPE 21
#define INTERPRET 22
#define ACTION_TOK 23
#define KEY 24
#define ALIAS 25
#define GROUP 26
#define MODIFIER_MAP 27
#define INDICATOR 28
#define SHAPE 29
#define KEYS 30
#define ROW 31
#define SECTION 32
#define OVERLAY 33
#define TEXT 34
#define OUTLINE 35
#define SOLID 36
#define LOGO 37
#define VIRTUAL 38
#define EQUALS 40
#define PLUS 41
#define MINUS 42
#define DIVIDE 43
#define TIMES 44
#define OBRACE 45
#define CBRACE 46
#define OPAREN 47
#define CPAREN 48
#define OBRACKET 49
#define CBRACKET 50
#define DOT 51
#define COMMA 52
#define SEMI 53
#define EXCLAM 54
#define INVERT 55
#define STRING 60
#define INTEGER 61
#define FLOAT 62
#define IDENT 63
#define KEYNAME 64
#define PARTIAL 70
#define DEFAULT 71
#define HIDDEN 72
#define ALPHANUMERIC_KEYS 73
#define MODIFIER_KEYS 74
#define KEYPAD_KEYS 75
#define FUNCTION_KEYS 76
#define ALTERNATE_GROUP 77




/* Copy the first part of user declarations.  */
#line 93 "xkbparse.y"

#ifdef DEBUG
#define	YYDEBUG 1
#endif
#define	DEBUG_VAR parseDebug
#include "parseutils.h"
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 111 "xkbparse.y"
{
	int		 ival;
	unsigned	 uval;
	char		*str;
	Atom	 	sval;
	ParseCommon	*any;
	ExprDef		*expr;
	VarDef		*var;
	VModDef		*vmod;
	InterpDef	*interp;
	KeyTypeDef	*keyType;
	SymbolsDef	*syms;
	ModMapDef	*modMask;
	GroupCompatDef	*groupCompat;
	IndicatorMapDef	*ledMap;
	IndicatorNameDef *ledName;
	KeycodeDef	*keyName;
	KeyAliasDef	*keyAlias;
	ShapeDef	*shape;
	SectionDef	*section;
	RowDef		*row;
	KeyDef		*key;
	OverlayDef	*overlay;
	OverlayKeyDef	*olKey;
	OutlineDef	*outline;
	DoodadDef	*doodad;
	XkbFile		*file;
}
/* Line 187 of yacc.c.  */
#line 268 "xkbparse.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 281 "xkbparse.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  18
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   706

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  73
/* YYNRULES -- Number of rules.  */
#define YYNRULES  184
/* YYNRULES -- Number of states.  */
#define YYNSTATES  335

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   257

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     4,     5,     6,     7,     8,     9,    10,    11,     2,
      12,    13,    14,    15,    16,     2,     2,     2,     2,     2,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     2,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     2,     2,     2,     2,
      52,    53,    54,    55,    56,     2,     2,     2,     2,     2,
      57,    58,    59,    60,    61,    62,    63,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     1,     2
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    12,    14,    22,    24,
      26,    28,    31,    33,    41,    46,    48,    50,    52,    54,
      56,    58,    59,    62,    64,    66,    68,    70,    72,    74,
      76,    78,    80,    83,    84,    87,    90,    93,    96,    99,
     102,   105,   108,   111,   114,   117,   120,   123,   126,   129,
     134,   137,   141,   146,   152,   156,   160,   162,   164,   168,
     175,   179,   181,   184,   186,   193,   200,   204,   206,   207,
     211,   215,   217,   220,   222,   226,   230,   236,   243,   250,
     256,   263,   270,   277,   284,   287,   289,   295,   297,   299,
     301,   303,   306,   308,   314,   316,   320,   322,   324,   328,
     335,   339,   341,   345,   349,   351,   355,   361,   365,   369,
     371,   377,   384,   386,   388,   390,   392,   394,   396,   398,
     400,   402,   404,   406,   408,   410,   412,   414,   416,   418,
     420,   421,   423,   425,   427,   429,   431,   433,   434,   438,
     440,   444,   448,   452,   456,   460,   462,   465,   468,   471,
     474,   476,   481,   483,   487,   491,   493,   498,   500,   504,
     509,   516,   518,   520,   522,   524,   526,   527,   531,   533,
     535,   537,   539,   542,   544,   546,   548,   550,   552,   554,
     556,   558,   560,   562,   563
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      66,     0,    -1,    67,    -1,    70,    -1,    72,    -1,    67,
      68,    -1,    68,    -1,    74,    69,   136,    41,    70,    42,
      49,    -1,     4,    -1,    10,    -1,    11,    -1,    70,    71,
      -1,    71,    -1,    74,    73,   136,    41,    77,    42,    49,
      -1,    74,    73,   136,    77,    -1,     5,    -1,     6,    -1,
       8,    -1,     7,    -1,     9,    -1,    75,    -1,    -1,    75,
      76,    -1,    76,    -1,    57,    -1,    58,    -1,    59,    -1,
      60,    -1,    61,    -1,    62,    -1,    63,    -1,    64,    -1,
      77,    78,    -1,    -1,   116,    79,    -1,   116,    82,    -1,
     116,    85,    -1,   116,    80,    -1,   116,    81,    -1,   116,
      88,    -1,   116,    89,    -1,   116,    94,    -1,   116,    93,
      -1,   116,    95,    -1,   116,    96,    -1,   116,    97,    -1,
     116,    98,    -1,   116,   112,    -1,   117,    52,    -1,   124,
      36,   120,    49,    -1,   134,    49,    -1,    50,   134,    49,
      -1,   133,    36,   120,    49,    -1,    22,   133,    36,   133,
      49,    -1,    17,    83,    49,    -1,    83,    48,    84,    -1,
      84,    -1,   134,    -1,   134,    36,   120,    -1,    19,    86,
      41,    87,    42,    49,    -1,   128,    37,   120,    -1,   128,
      -1,    87,    79,    -1,    79,    -1,    18,   135,    41,    87,
      42,    49,    -1,    21,   133,    41,    90,    42,    49,    -1,
      90,    48,    91,    -1,    91,    -1,    -1,   124,    36,   120,
      -1,   124,    36,    92,    -1,   134,    -1,    50,   134,    -1,
      92,    -1,    45,   126,    46,    -1,    45,   122,    46,    -1,
      23,   132,    36,   120,    49,    -1,    24,   134,    41,   119,
      42,    49,    -1,    25,   135,    41,    87,    42,    49,    -1,
      25,   132,    36,   120,    49,    -1,    35,    25,   132,    36,
     120,    49,    -1,    26,   135,    41,   108,    42,    49,    -1,
      26,   135,    41,   110,    42,    49,    -1,    29,   135,    41,
      99,    42,    49,    -1,    99,   100,    -1,   100,    -1,    28,
      41,   101,    42,    49,    -1,    79,    -1,   112,    -1,    95,
      -1,   105,    -1,   101,   102,    -1,   102,    -1,    27,    41,
     103,    42,    49,    -1,    79,    -1,   103,    48,   104,    -1,
     104,    -1,   133,    -1,    41,   119,    42,    -1,    30,   135,
      41,   106,    42,    49,    -1,   106,    48,   107,    -1,   107,
      -1,   133,    36,   133,    -1,   108,    48,   109,    -1,   109,
      -1,    41,   110,    42,    -1,   134,    36,    41,   110,    42,
      -1,   134,    36,   120,    -1,   110,    48,   111,    -1,   111,
      -1,    45,   129,    48,   129,    46,    -1,   113,   135,    41,
      87,    42,    49,    -1,    31,    -1,    32,    -1,    33,    -1,
      34,    -1,   134,    -1,   115,    -1,    20,    -1,    19,    -1,
      18,    -1,    21,    -1,    23,    -1,    24,    -1,    25,    -1,
      26,    -1,    28,    -1,    29,    -1,    31,    -1,   117,    -1,
      -1,    12,    -1,    14,    -1,    13,    -1,    15,    -1,    16,
      -1,   119,    -1,    -1,   119,    48,   120,    -1,   120,    -1,
     120,    39,   120,    -1,   120,    37,   120,    -1,   120,    38,
     120,    -1,   120,    40,   120,    -1,   124,    36,   120,    -1,
     121,    -1,    38,   121,    -1,    37,   121,    -1,    50,   121,
      -1,    51,   121,    -1,   124,    -1,   114,    43,   118,    44,
      -1,   125,    -1,    43,   120,    44,    -1,   122,    48,   123,
      -1,   123,    -1,   114,    43,   118,    44,    -1,   114,    -1,
     114,    47,   114,    -1,   114,    45,   120,    46,    -1,   114,
      47,   114,    45,   120,    46,    -1,   135,    -1,   132,    -1,
     131,    -1,   133,    -1,   127,    -1,    -1,   127,    48,   128,
      -1,   128,    -1,    55,    -1,    29,    -1,   132,    -1,    38,
     130,    -1,   130,    -1,    54,    -1,    53,    -1,    54,    -1,
      53,    -1,    56,    -1,    55,    -1,    58,    -1,    52,    -1,
     137,    -1,    -1,    52,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   169,   169,   171,   173,   177,   179,   183,   189,   190,
     191,   194,   196,   200,   206,   211,   212,   213,   214,   215,
     218,   219,   222,   223,   226,   227,   228,   229,   230,   231,
     232,   233,   236,   238,   241,   246,   251,   256,   261,   266,
     271,   276,   281,   286,   291,   296,   301,   306,   311,   323,
     325,   327,   331,   342,   352,   356,   358,   362,   364,   368,
     377,   379,   383,   385,   389,   395,   401,   403,   405,   408,
     410,   412,   414,   416,   420,   422,   426,   430,   434,   438,
     440,   444,   446,   454,   458,   460,   464,   466,   468,   470,
     472,   476,   478,   482,   484,   488,   490,   494,   496,   500,
     504,   509,   513,   517,   519,   523,   525,   527,   531,   533,
     537,   547,   551,   552,   553,   554,   557,   558,   561,   563,
     565,   567,   569,   571,   573,   575,   577,   579,   581,   585,
     586,   589,   590,   591,   592,   593,   596,   597,   600,   602,
     606,   608,   610,   612,   614,   616,   620,   622,   624,   626,
     628,   630,   632,   634,   638,   640,   644,   648,   655,   663,
     672,   683,   690,   697,   704,   715,   716,   719,   721,   725,
     739,   743,   750,   751,   754,   755,   758,   761,   764,   767,
     768,   771,   774,   775,   778
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "END_OF_FILE", "error", "$undefined", "ERROR_TOK", "XKB_KEYMAP",
  "XKB_KEYCODES", "XKB_TYPES", "XKB_SYMBOLS", "XKB_COMPATMAP",
  "XKB_GEOMETRY", "XKB_SEMANTICS", "XKB_LAYOUT", "INCLUDE", "OVERRIDE",
  "AUGMENT", "REPLACE", "ALTERNATE", "VIRTUAL_MODS", "TYPE", "INTERPRET",
  "ACTION_TOK", "KEY", "ALIAS", "GROUP", "MODIFIER_MAP", "INDICATOR",
  "SHAPE", "KEYS", "ROW", "SECTION", "OVERLAY", "TEXT", "OUTLINE", "SOLID",
  "LOGO", "VIRTUAL", "EQUALS", "PLUS", "MINUS", "DIVIDE", "TIMES",
  "OBRACE", "CBRACE", "OPAREN", "CPAREN", "OBRACKET", "CBRACKET", "DOT",
  "COMMA", "SEMI", "EXCLAM", "INVERT", "STRING", "INTEGER", "FLOAT",
  "IDENT", "KEYNAME", "PARTIAL", "DEFAULT", "HIDDEN", "ALPHANUMERIC_KEYS",
  "MODIFIER_KEYS", "KEYPAD_KEYS", "FUNCTION_KEYS", "ALTERNATE_GROUP",
  "$accept", "XkbFile", "XkbCompMapList", "XkbCompositeMap",
  "XkbCompositeType", "XkbMapConfigList", "XkbMapConfig", "XkbConfig",
  "FileType", "OptFlags", "Flags", "Flag", "DeclList", "Decl", "VarDecl",
  "KeyNameDecl", "KeyAliasDecl", "VModDecl", "VModDefList", "VModDef",
  "InterpretDecl", "InterpretMatch", "VarDeclList", "KeyTypeDecl",
  "SymbolsDecl", "SymbolsBody", "SymbolsVarDecl", "ArrayInit",
  "GroupCompatDecl", "ModMapDecl", "IndicatorMapDecl", "IndicatorNameDecl",
  "ShapeDecl", "SectionDecl", "SectionBody", "SectionBodyItem", "RowBody",
  "RowBodyItem", "Keys", "Key", "OverlayDecl", "OverlayKeyList",
  "OverlayKey", "OutlineList", "OutlineInList", "CoordList", "Coord",
  "DoodadDecl", "DoodadType", "FieldSpec", "Element", "OptMergeMode",
  "MergeMode", "OptExprList", "ExprList", "Expr", "Term", "ActionList",
  "Action", "Lhs", "Terminal", "OptKeySymList", "KeySymList", "KeySym",
  "SignedNumber", "Number", "Float", "Integer", "KeyName", "Ident",
  "String", "OptMapName", "MapName", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   255,     1,     2,     3,     4,     5,     6,
       7,     8,    10,    11,    12,    13,    14,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    60,    61,    62,    63,    64,    70,    71,    72,
      73,    74,    75,    76,    77
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    66,    66,    67,    67,    68,    69,    69,
      69,    70,    70,    71,    72,    73,    73,    73,    73,    73,
      74,    74,    75,    75,    76,    76,    76,    76,    76,    76,
      76,    76,    77,    77,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    79,
      79,    79,    80,    81,    82,    83,    83,    84,    84,    85,
      86,    86,    87,    87,    88,    89,    90,    90,    90,    91,
      91,    91,    91,    91,    92,    92,    93,    94,    95,    96,
      96,    97,    97,    98,    99,    99,   100,   100,   100,   100,
     100,   101,   101,   102,   102,   103,   103,   104,   104,   105,
     106,   106,   107,   108,   108,   109,   109,   109,   110,   110,
     111,   112,   113,   113,   113,   113,   114,   114,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   116,
     116,   117,   117,   117,   117,   117,   118,   118,   119,   119,
     120,   120,   120,   120,   120,   120,   121,   121,   121,   121,
     121,   121,   121,   121,   122,   122,   123,   124,   124,   124,
     124,   125,   125,   125,   125,   126,   126,   127,   127,   128,
     128,   128,   129,   129,   130,   130,   131,   132,   133,   134,
     134,   135,   136,   136,   137
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     7,     1,     1,
       1,     2,     1,     7,     4,     1,     1,     1,     1,     1,
       1,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       2,     3,     4,     5,     3,     3,     1,     1,     3,     6,
       3,     1,     2,     1,     6,     6,     3,     1,     0,     3,
       3,     1,     2,     1,     3,     3,     5,     6,     6,     5,
       6,     6,     6,     6,     2,     1,     5,     1,     1,     1,
       1,     2,     1,     5,     1,     3,     1,     1,     3,     6,
       3,     1,     3,     3,     1,     3,     5,     3,     3,     1,
       5,     6,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     3,     1,
       3,     3,     3,     3,     3,     1,     2,     2,     2,     2,
       1,     4,     1,     3,     3,     1,     4,     1,     3,     4,
       6,     1,     1,     1,     1,     1,     0,     3,     1,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      21,    24,    25,    26,    27,    28,    29,    30,    31,     0,
      21,     6,    21,    12,     4,     0,    20,    23,     1,     5,
       0,    11,     0,     8,    15,    16,    18,    17,    19,     9,
      10,   183,   183,    22,   183,   184,     0,   182,    33,     0,
      21,    33,   130,    21,   130,   131,   133,   132,   134,   135,
      32,     0,   129,     0,     0,     0,   120,   119,   118,   121,
       0,   122,   123,   124,   125,   126,   127,   128,   113,   114,
     115,     0,     0,   179,   178,   180,    34,    37,    38,    35,
      36,    39,    40,    42,    41,    43,    44,    45,    46,    47,
       0,   157,   117,     0,     0,   116,    48,     7,    13,     0,
      56,    57,   181,     0,   170,   177,   169,     0,    61,   171,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,    54,     0,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,   120,   119,   121,   122,   123,   124,   125,
     127,   128,     0,     0,     0,     0,     0,   176,   157,     0,
     145,   150,   152,   163,   162,   164,   116,   161,   158,     0,
       0,    55,    58,    63,     0,     0,    60,   166,     0,     0,
      67,    73,     0,   116,     0,     0,     0,   139,     0,     0,
       0,     0,     0,   104,     0,   109,     0,   124,   126,     0,
      87,    89,     0,    85,    90,    88,     0,     0,   147,   150,
     146,     0,   148,   149,   137,     0,     0,     0,     0,   159,
       0,     0,    49,    52,     0,    62,     0,   170,   169,     0,
       0,   155,     0,   165,   168,    72,     0,     0,     0,    53,
      76,     0,     0,    79,     0,     0,     0,   175,   174,     0,
     173,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,   153,     0,   136,   141,   142,   140,   143,   144,
       0,    64,    59,   137,    75,     0,    74,     0,    65,    66,
      70,    69,    77,   138,    78,   105,   172,     0,    81,   103,
      82,   108,     0,   107,     0,    94,     0,    92,     0,    83,
      80,   111,   151,   160,     0,   154,   167,     0,     0,     0,
       0,    91,     0,   101,     0,   156,   110,   106,     0,     0,
      96,    97,    86,     0,     0,     0,     0,     0,     0,    99,
     100,   102,    98,    93,    95
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     9,    10,    11,    31,    12,    13,    14,    32,    22,
      16,    17,    42,    50,   173,    77,    78,    79,    99,   100,
      80,   107,   174,    81,    82,   179,   180,   181,    83,    84,
     201,    86,    87,    88,   202,   203,   296,   297,   319,   320,
     204,   312,   313,   192,   193,   194,   195,   205,    90,   158,
      92,    51,    52,   263,   264,   187,   160,   230,   231,   161,
     162,   232,   233,   108,   249,   250,   163,   164,   165,   166,
     167,    36,    37
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -188
static const yytype_int16 yypact[] =
{
     628,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    12,
       4,  -188,    58,  -188,  -188,   695,   628,  -188,  -188,  -188,
     121,  -188,   408,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,    -9,    -9,  -188,    -9,  -188,    19,  -188,    45,    45,
     628,  -188,   174,   620,   137,  -188,  -188,  -188,  -188,  -188,
    -188,   317,    39,   -15,    50,     1,    59,   -18,  -188,    72,
      72,   106,     1,   -27,    59,  -188,    59,    62,  -188,  -188,
    -188,   117,     1,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
      59,    51,  -188,   125,   146,   134,  -188,  -188,  -188,   132,
    -188,   163,  -188,   168,  -188,  -188,  -188,   173,   179,  -188,
     178,   186,   190,   194,   206,   204,   208,   221,   106,   216,
     225,   265,   640,   265,   265,  -188,     1,  -188,   265,   599,
     599,   265,   470,    72,   265,   265,   265,   599,   -26,    21,
     239,  -188,   599,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,   265,   265,   265,   265,   265,  -188,   159,   232,
    -188,   240,  -188,  -188,  -188,  -188,  -188,  -188,   223,    98,
     156,  -188,   260,  -188,   485,   513,   260,   617,     1,    30,
    -188,  -188,   243,    32,   231,   192,    64,   260,   199,   528,
     236,    35,   127,  -188,   128,  -188,   251,    59,   270,    59,
    -188,  -188,   422,  -188,  -188,  -188,   265,   556,  -188,  -188,
    -188,   342,  -188,  -188,   265,   265,   265,   265,   265,  -188,
     265,   265,  -188,  -188,   263,  -188,   264,   249,   271,   281,
      61,  -188,   279,   278,  -188,  -188,   280,   470,   340,  -188,
    -188,   283,   265,  -188,   284,   129,    16,  -188,  -188,   282,
    -188,   298,   -34,   308,   236,   381,   576,   287,   321,  -188,
     215,   325,  -188,   332,   336,   158,   158,  -188,  -188,   260,
     316,  -188,  -188,   265,  -188,   640,  -188,   -18,  -188,  -188,
    -188,   260,  -188,   260,  -188,  -188,  -188,    35,  -188,  -188,
    -188,  -188,   236,   260,   346,  -188,   442,  -188,    72,  -188,
    -188,  -188,  -188,  -188,   344,  -188,  -188,   343,   143,   -28,
     348,  -188,   176,  -188,   367,  -188,  -188,  -188,   265,   198,
    -188,  -188,  -188,   359,    72,    72,   202,   362,   -28,  -188,
    -188,  -188,  -188,  -188,  -188
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -188,  -188,  -188,   410,  -188,   383,    -7,  -188,   399,    38,
    -188,   409,   385,  -188,   -35,  -188,  -188,  -188,  -188,   301,
    -188,  -188,   -47,  -188,  -188,  -188,   191,   200,  -188,  -188,
     378,  -188,  -188,  -188,  -188,   228,  -188,   148,  -188,   130,
    -188,  -188,   133,  -188,   197,  -187,   205,   423,  -188,    26,
    -188,  -188,  -188,   203,  -134,    89,   104,  -188,   207,   -29,
    -188,  -188,  -188,  -175,   188,   233,  -188,   -43,   -51,   -45,
     -33,   109,  -188
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -180
static const yytype_int16 yytable[] =
{
      94,   186,   234,   245,    -2,    21,    95,   190,   110,   111,
     101,   104,    18,   318,   109,   190,    76,   113,   112,   191,
     114,    73,    93,   103,    75,   102,   105,   119,    74,    73,
     115,   116,    75,   117,    97,   105,    21,   106,    15,   143,
     144,    58,   145,    35,   146,   147,   197,   149,    20,   198,
     150,   199,    67,    68,    69,    70,    73,   120,    -3,    75,
      40,     1,     2,     3,     4,     5,     6,     7,     8,   247,
     248,    72,   236,   246,   -71,   140,    73,    91,   237,    75,
     -71,   101,   184,   175,    95,    95,    41,   183,   247,   248,
     189,    96,    95,   196,    95,   207,   121,    95,   122,    98,
      93,    93,   306,   182,   200,   308,   241,   274,    93,   275,
      93,   102,   242,    93,  -112,     1,     2,     3,     4,     5,
       6,     7,     8,   209,   209,    23,   209,   209,    74,    95,
      95,    29,    30,   235,   109,   215,   216,   217,   218,   225,
     225,    38,   118,    39,    95,    93,    93,   222,   168,    45,
      46,    47,    48,    49,   225,    91,    91,    95,    91,   105,
      93,   123,    95,    91,   115,    91,   257,   200,    91,   251,
     253,   285,   225,    93,   -14,   252,   254,   254,    93,    54,
     126,   127,   124,   125,   326,   317,    45,    46,    47,    48,
      49,   254,   183,   215,   216,   217,   218,   217,   218,   128,
      91,    91,   214,   229,   121,   223,   122,   196,   182,   129,
     159,    95,   169,   170,   130,    91,   131,   172,   323,   132,
     176,   295,   133,   185,   324,   188,   134,    93,    91,   215,
     216,   217,   218,    91,   109,   135,   215,   216,   217,   218,
     327,   240,   136,   211,   332,   137,   328,   314,   243,   138,
     242,    95,   215,   216,   217,   218,   208,   210,   321,   212,
     213,   295,   139,    91,   300,   141,   142,    93,   221,   215,
     216,   217,   218,   314,   331,   206,   220,   321,   219,   238,
     239,   191,    91,   143,   144,    58,   145,   255,   146,   147,
     148,   149,  -127,    65,   150,   260,   151,   215,   216,   217,
     218,   229,   152,   153,   265,   266,   267,   268,   154,   269,
     270,   256,   271,   272,  -179,   155,   156,   102,   105,   157,
      73,    74,    91,    75,   273,   276,   277,   281,   298,   278,
     287,   283,   282,   284,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   293,    65,    66,   288,    67,    68,
      69,    70,    71,   215,   216,   217,   218,   290,   143,   144,
      58,   145,   303,   146,   147,   148,   149,    72,    65,   150,
     299,   151,    73,    74,   301,    75,   302,   152,   153,   215,
     216,   217,   218,   154,   242,   177,   262,   309,   315,   316,
     155,   156,   102,   105,   157,    73,    74,   322,    75,   143,
     144,    58,   145,   325,   146,   147,   148,   149,   329,    65,
     150,   333,   151,    24,    25,    26,    27,    28,   152,   153,
      19,    34,   292,    43,   154,    33,    44,   171,   279,    85,
     259,   155,   156,   102,   105,   157,    73,    74,   280,    75,
     143,   144,    58,   145,   311,   146,   147,   197,   149,   289,
     198,   150,   199,    67,    68,    69,    70,   330,   334,   291,
     143,   144,    58,   145,   258,   146,   147,   148,   149,   294,
      65,   150,    72,   151,    89,   307,   304,    73,     0,   286,
      75,     0,   305,     0,   310,     0,     0,     0,   143,   144,
      58,   145,    72,   146,   147,   148,   149,    73,    65,   150,
      75,   151,     0,   143,   144,    58,   145,     0,   146,   147,
     148,   149,     0,    65,   150,   177,   151,     0,     0,     0,
     178,     0,     0,     0,     0,    73,     0,   224,    75,     0,
       0,   143,   144,    58,   145,    72,   146,   147,   148,   149,
      73,    65,   150,    75,   151,     0,   143,   144,    58,   145,
       0,   146,   147,   148,   149,   226,    65,   150,     0,   151,
       0,     0,     0,    72,     0,     0,     0,     0,    73,     0,
     244,    75,     0,     0,   143,   144,    58,   145,    72,   146,
     147,   148,   149,    73,    65,   150,    75,   151,     0,     0,
       0,     0,     0,     0,   143,   144,    58,   145,   261,   146,
     147,   148,   149,   294,    65,   150,    72,   151,     0,     0,
       0,    73,     0,     0,    75,     0,     0,   143,   144,    58,
     145,     0,   146,   147,   148,   149,    72,    65,   150,     0,
     151,    73,     0,     0,    75,   143,   144,    58,   145,     0,
     146,   147,   148,   149,     0,    65,   227,     0,   151,    72,
       0,     0,     0,     0,    73,     0,     0,    75,   143,   144,
      58,   145,    53,   146,   147,   148,   149,     0,    65,   150,
     105,   151,   228,     0,     0,    75,     0,     1,     2,     3,
       4,     5,     6,     7,     8,     1,     2,     3,     4,     5,
       6,     7,     8,     0,     0,    73,     0,     0,    75,    23,
      24,    25,    26,    27,    28,    29,    30
};

static const yytype_int16 yycheck[] =
{
      51,   135,   177,   190,     0,    12,    51,    41,    59,    60,
      55,    29,     0,    41,    57,    41,    51,    62,    61,    45,
      63,    55,    51,    56,    58,    52,    53,    72,    56,    55,
      63,    64,    58,    66,    49,    53,    43,    55,     0,    18,
      19,    20,    21,    52,    23,    24,    25,    26,    10,    28,
      29,    30,    31,    32,    33,    34,    55,    90,     0,    58,
      41,    57,    58,    59,    60,    61,    62,    63,    64,    53,
      54,    50,    42,    38,    42,   118,    55,    51,    48,    58,
      48,   126,   133,   130,   129,   130,    41,   132,    53,    54,
     137,    52,   137,   138,   139,   142,    45,   142,    47,    49,
     129,   130,   277,   132,   139,   292,    42,    46,   137,    48,
     139,    52,    48,   142,    52,    57,    58,    59,    60,    61,
      62,    63,    64,   152,   153,     4,   155,   156,    56,   174,
     175,    10,    11,   178,   177,    37,    38,    39,    40,   174,
     175,    32,    25,    34,   189,   174,   175,    49,   122,    12,
      13,    14,    15,    16,   189,   129,   130,   202,   132,    53,
     189,    36,   207,   137,   197,   139,   199,   202,   142,    42,
      42,    42,   207,   202,     0,    48,    48,    48,   207,    42,
      48,    49,    36,    49,   318,    42,    12,    13,    14,    15,
      16,    48,   237,    37,    38,    39,    40,    39,    40,    36,
     174,   175,    43,   177,    45,    49,    47,   252,   237,    41,
     121,   256,   123,   124,    41,   189,    37,   128,    42,    41,
     131,   256,    36,   134,    48,   136,    36,   256,   202,    37,
      38,    39,    40,   207,   277,    41,    37,    38,    39,    40,
      42,    49,    36,   154,    42,    41,    48,   298,    49,    41,
      48,   296,    37,    38,    39,    40,   152,   153,   309,   155,
     156,   296,    41,   237,    49,    49,    41,   296,    45,    37,
      38,    39,    40,   324,   325,    36,    36,   328,    46,    36,
      49,    45,   256,    18,    19,    20,    21,    36,    23,    24,
      25,    26,    43,    28,    29,   206,    31,    37,    38,    39,
      40,   275,    37,    38,   215,   216,   217,   218,    43,   220,
     221,    41,    49,    49,    43,    50,    51,    52,    53,    54,
      55,    56,   296,    58,    43,    46,    48,   238,    41,    49,
      48,   242,    49,    49,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   255,    28,    29,    49,    31,    32,
      33,    34,    35,    37,    38,    39,    40,    49,    18,    19,
      20,    21,    46,    23,    24,    25,    26,    50,    28,    29,
      49,    31,    55,    56,    49,    58,    44,    37,    38,    37,
      38,    39,    40,    43,    48,    45,    44,    41,    44,    46,
      50,    51,    52,    53,    54,    55,    56,    49,    58,    18,
      19,    20,    21,    36,    23,    24,    25,    26,    49,    28,
      29,    49,    31,     5,     6,     7,     8,     9,    37,    38,
      10,    22,    41,    40,    43,    16,    41,   126,   237,    51,
     202,    50,    51,    52,    53,    54,    55,    56,   238,    58,
      18,    19,    20,    21,   296,    23,    24,    25,    26,   252,
      28,    29,    30,    31,    32,    33,    34,   324,   328,   254,
      18,    19,    20,    21,    42,    23,    24,    25,    26,    27,
      28,    29,    50,    31,    51,   287,   273,    55,    -1,   246,
      58,    -1,   275,    -1,    42,    -1,    -1,    -1,    18,    19,
      20,    21,    50,    23,    24,    25,    26,    55,    28,    29,
      58,    31,    -1,    18,    19,    20,    21,    -1,    23,    24,
      25,    26,    -1,    28,    29,    45,    31,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    55,    -1,    42,    58,    -1,
      -1,    18,    19,    20,    21,    50,    23,    24,    25,    26,
      55,    28,    29,    58,    31,    -1,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    42,    28,    29,    -1,    31,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,
      42,    58,    -1,    -1,    18,    19,    20,    21,    50,    23,
      24,    25,    26,    55,    28,    29,    58,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    42,    23,
      24,    25,    26,    27,    28,    29,    50,    31,    -1,    -1,
      -1,    55,    -1,    -1,    58,    -1,    -1,    18,    19,    20,
      21,    -1,    23,    24,    25,    26,    50,    28,    29,    -1,
      31,    55,    -1,    -1,    58,    18,    19,    20,    21,    -1,
      23,    24,    25,    26,    -1,    28,    29,    -1,    31,    50,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    58,    18,    19,
      20,    21,    42,    23,    24,    25,    26,    -1,    28,    29,
      53,    31,    55,    -1,    -1,    58,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    55,    -1,    -1,    58,     4,
       5,     6,     7,     8,     9,    10,    11
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    58,    59,    60,    61,    62,    63,    64,    66,
      67,    68,    70,    71,    72,    74,    75,    76,     0,    68,
      74,    71,    74,     4,     5,     6,     7,     8,     9,    10,
      11,    69,    73,    76,    73,    52,   136,   137,   136,   136,
      41,    41,    77,    70,    77,    12,    13,    14,    15,    16,
      78,   116,   117,    42,    42,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    28,    29,    31,    32,    33,
      34,    35,    50,    55,    56,    58,    79,    80,    81,    82,
      85,    88,    89,    93,    94,    95,    96,    97,    98,   112,
     113,   114,   115,   124,   133,   134,    52,    49,    49,    83,
      84,   134,    52,   135,    29,    53,    55,    86,   128,   132,
     133,   133,   132,   134,   132,   135,   135,   135,    25,   134,
     135,    45,    47,    36,    36,    49,    48,    49,    36,    41,
      41,    37,    41,    36,    36,    41,    36,    41,    41,    41,
     132,    49,    41,    18,    19,    21,    23,    24,    25,    26,
      29,    31,    37,    38,    43,    50,    51,    54,   114,   120,
     121,   124,   125,   131,   132,   133,   134,   135,   114,   120,
     120,    84,   120,    79,    87,    87,   120,    45,    50,    90,
      91,    92,   124,   134,   133,   120,   119,   120,   120,    87,
      41,    45,   108,   109,   110,   111,   134,    25,    28,    30,
      79,    95,    99,   100,   105,   112,    36,    87,   121,   124,
     121,   120,   121,   121,    43,    37,    38,    39,    40,    46,
      36,    45,    49,    49,    42,    79,    42,    29,    55,   114,
     122,   123,   126,   127,   128,   134,    42,    48,    36,    49,
      49,    42,    48,    49,    42,   110,    38,    53,    54,   129,
     130,    42,    48,    42,    48,    36,    41,   135,    42,   100,
     120,    42,    44,   118,   119,   120,   120,   120,   120,   120,
     120,    49,    49,    43,    46,    48,    46,    48,    49,    91,
      92,   120,    49,   120,    49,    42,   130,    48,    49,   109,
      49,   111,    41,   120,    27,    79,   101,   102,    41,    49,
      49,    49,    44,    46,   118,   123,   128,   129,   110,    41,
      42,   102,   106,   107,   133,    44,    46,    42,    41,   103,
     104,   133,    49,    42,    48,    36,   119,    42,    48,    49,
     107,   133,    42,    49,   104
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 170 "xkbparse.y"
    { (yyval.file)= rtrnValue= (yyvsp[(1) - (1)].file); }
    break;

  case 3:
#line 172 "xkbparse.y"
    { (yyval.file)= rtrnValue= (yyvsp[(1) - (1)].file);  }
    break;

  case 4:
#line 174 "xkbparse.y"
    { (yyval.file)= rtrnValue= (yyvsp[(1) - (1)].file); }
    break;

  case 5:
#line 178 "xkbparse.y"
    { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[(1) - (2)].file)->common,&(yyvsp[(2) - (2)].file)->common); }
    break;

  case 6:
#line 180 "xkbparse.y"
    { (yyval.file)= (yyvsp[(1) - (1)].file); }
    break;

  case 7:
#line 186 "xkbparse.y"
    { (yyval.file)= CreateXKBFile((yyvsp[(2) - (7)].uval),(yyvsp[(3) - (7)].str),&(yyvsp[(5) - (7)].file)->common,(yyvsp[(1) - (7)].uval)); }
    break;

  case 8:
#line 189 "xkbparse.y"
    { (yyval.uval)= XkmKeymapFile; }
    break;

  case 9:
#line 190 "xkbparse.y"
    { (yyval.uval)= XkmSemanticsFile; }
    break;

  case 10:
#line 191 "xkbparse.y"
    { (yyval.uval)= XkmLayoutFile; }
    break;

  case 11:
#line 195 "xkbparse.y"
    { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[(1) - (2)].file)->common,&(yyvsp[(2) - (2)].file)->common); }
    break;

  case 12:
#line 197 "xkbparse.y"
    { (yyval.file)= (yyvsp[(1) - (1)].file); }
    break;

  case 13:
#line 203 "xkbparse.y"
    { (yyval.file)= CreateXKBFile((yyvsp[(2) - (7)].uval),(yyvsp[(3) - (7)].str),(yyvsp[(5) - (7)].any),(yyvsp[(1) - (7)].uval)); }
    break;

  case 14:
#line 207 "xkbparse.y"
    { (yyval.file)= CreateXKBFile((yyvsp[(2) - (4)].uval),(yyvsp[(3) - (4)].str),(yyvsp[(4) - (4)].any),(yyvsp[(1) - (4)].uval)); }
    break;

  case 15:
#line 211 "xkbparse.y"
    { (yyval.uval)= XkmKeyNamesIndex; }
    break;

  case 16:
#line 212 "xkbparse.y"
    { (yyval.uval)= XkmTypesIndex; }
    break;

  case 17:
#line 213 "xkbparse.y"
    { (yyval.uval)= XkmCompatMapIndex; }
    break;

  case 18:
#line 214 "xkbparse.y"
    { (yyval.uval)= XkmSymbolsIndex; }
    break;

  case 19:
#line 215 "xkbparse.y"
    { (yyval.uval)= XkmGeometryIndex; }
    break;

  case 20:
#line 218 "xkbparse.y"
    { (yyval.uval)= (yyvsp[(1) - (1)].uval); }
    break;

  case 21:
#line 219 "xkbparse.y"
    { (yyval.uval)= 0; }
    break;

  case 22:
#line 222 "xkbparse.y"
    { (yyval.uval)= (((yyvsp[(1) - (2)].uval))|((yyvsp[(2) - (2)].uval))); }
    break;

  case 23:
#line 223 "xkbparse.y"
    { (yyval.uval)= (yyvsp[(1) - (1)].uval); }
    break;

  case 24:
#line 226 "xkbparse.y"
    { (yyval.uval)= XkbLC_Partial; }
    break;

  case 25:
#line 227 "xkbparse.y"
    { (yyval.uval)= XkbLC_Default; }
    break;

  case 26:
#line 228 "xkbparse.y"
    { (yyval.uval)= XkbLC_Hidden; }
    break;

  case 27:
#line 229 "xkbparse.y"
    { (yyval.uval)= XkbLC_AlphanumericKeys; }
    break;

  case 28:
#line 230 "xkbparse.y"
    { (yyval.uval)= XkbLC_ModifierKeys; }
    break;

  case 29:
#line 231 "xkbparse.y"
    { (yyval.uval)= XkbLC_KeypadKeys; }
    break;

  case 30:
#line 232 "xkbparse.y"
    { (yyval.uval)= XkbLC_FunctionKeys; }
    break;

  case 31:
#line 233 "xkbparse.y"
    { (yyval.uval)= XkbLC_AlternateGroup; }
    break;

  case 32:
#line 237 "xkbparse.y"
    { (yyval.any)= AppendStmt((yyvsp[(1) - (2)].any),(yyvsp[(2) - (2)].any)); }
    break;

  case 33:
#line 238 "xkbparse.y"
    { (yyval.any)= NULL; }
    break;

  case 34:
#line 242 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].var)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].var)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].var)->common;
			}
    break;

  case 35:
#line 247 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].vmod)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].vmod)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].vmod)->common;
			}
    break;

  case 36:
#line 252 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].interp)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].interp)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].interp)->common;
			}
    break;

  case 37:
#line 257 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].keyName)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].keyName)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].keyName)->common;
			}
    break;

  case 38:
#line 262 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].keyAlias)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].keyAlias)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].keyAlias)->common;
			}
    break;

  case 39:
#line 267 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].keyType)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].keyType)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].keyType)->common;
			}
    break;

  case 40:
#line 272 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].syms)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].syms)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].syms)->common;
			}
    break;

  case 41:
#line 277 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].modMask)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].modMask)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].modMask)->common;
			}
    break;

  case 42:
#line 282 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].groupCompat)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].groupCompat)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].groupCompat)->common;
			}
    break;

  case 43:
#line 287 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].ledMap)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].ledMap)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].ledMap)->common;
			}
    break;

  case 44:
#line 292 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].ledName)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].ledName)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].ledName)->common;
			}
    break;

  case 45:
#line 297 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].shape)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].shape)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].shape)->common;
			}
    break;

  case 46:
#line 302 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].section)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].section)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].section)->common;
			}
    break;

  case 47:
#line 307 "xkbparse.y"
    {
			    (yyvsp[(2) - (2)].doodad)->merge= StmtSetMerge(&(yyvsp[(2) - (2)].doodad)->common,(yyvsp[(1) - (2)].uval));
			    (yyval.any)= &(yyvsp[(2) - (2)].doodad)->common;
			}
    break;

  case 48:
#line 312 "xkbparse.y"
    {
			    if ((yyvsp[(1) - (2)].uval)==MergeAltForm) {
				yyerror("cannot use 'alternate' to include other maps");
				(yyval.any)= &IncludeCreate(scanStr,MergeDefault)->common;
			    }
			    else {
				(yyval.any)= &IncludeCreate(scanStr,(yyvsp[(1) - (2)].uval))->common;
			    }
                        }
    break;

  case 49:
#line 324 "xkbparse.y"
    { (yyval.var)= VarCreate((yyvsp[(1) - (4)].expr),(yyvsp[(3) - (4)].expr)); }
    break;

  case 50:
#line 326 "xkbparse.y"
    { (yyval.var)= BoolVarCreate((yyvsp[(1) - (2)].sval),1); }
    break;

  case 51:
#line 328 "xkbparse.y"
    { (yyval.var)= BoolVarCreate((yyvsp[(2) - (3)].sval),0); }
    break;

  case 52:
#line 332 "xkbparse.y"
    {
			    KeycodeDef *def;

			    def= KeycodeCreate((yyvsp[(1) - (4)].str),(yyvsp[(3) - (4)].expr));
			    if ((yyvsp[(1) - (4)].str))
				free((yyvsp[(1) - (4)].str));
			    (yyval.keyName)= def;
			}
    break;

  case 53:
#line 343 "xkbparse.y"
    { 
			    KeyAliasDef	*def;
			    def= KeyAliasCreate((yyvsp[(2) - (5)].str),(yyvsp[(4) - (5)].str)); 
			    if ((yyvsp[(2) - (5)].str))	free((yyvsp[(2) - (5)].str));	
			    if ((yyvsp[(4) - (5)].str))	free((yyvsp[(4) - (5)].str));	
			    (yyval.keyAlias)= def;
			}
    break;

  case 54:
#line 353 "xkbparse.y"
    { (yyval.vmod)= (yyvsp[(2) - (3)].vmod); }
    break;

  case 55:
#line 357 "xkbparse.y"
    { (yyval.vmod)= (VModDef *)AppendStmt(&(yyvsp[(1) - (3)].vmod)->common,&(yyvsp[(3) - (3)].vmod)->common); }
    break;

  case 56:
#line 359 "xkbparse.y"
    { (yyval.vmod)= (yyvsp[(1) - (1)].vmod); }
    break;

  case 57:
#line 363 "xkbparse.y"
    { (yyval.vmod)= VModCreate((yyvsp[(1) - (1)].sval),NULL); }
    break;

  case 58:
#line 365 "xkbparse.y"
    { (yyval.vmod)= VModCreate((yyvsp[(1) - (3)].sval),(yyvsp[(3) - (3)].expr)); }
    break;

  case 59:
#line 371 "xkbparse.y"
    {
			    (yyvsp[(2) - (6)].interp)->def= (yyvsp[(4) - (6)].var);
			    (yyval.interp)= (yyvsp[(2) - (6)].interp);
			}
    break;

  case 60:
#line 378 "xkbparse.y"
    { (yyval.interp)= InterpCreate((KeySym)(yyvsp[(1) - (3)].uval),(yyvsp[(3) - (3)].expr)); }
    break;

  case 61:
#line 380 "xkbparse.y"
    { (yyval.interp)= InterpCreate((KeySym)(yyvsp[(1) - (1)].uval),NULL); }
    break;

  case 62:
#line 384 "xkbparse.y"
    { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[(1) - (2)].var)->common,&(yyvsp[(2) - (2)].var)->common); }
    break;

  case 63:
#line 386 "xkbparse.y"
    { (yyval.var)= (yyvsp[(1) - (1)].var); }
    break;

  case 64:
#line 392 "xkbparse.y"
    { (yyval.keyType)= KeyTypeCreate((yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].var)); }
    break;

  case 65:
#line 398 "xkbparse.y"
    { (yyval.syms)= SymbolsCreate((yyvsp[(2) - (6)].str),(ExprDef *)(yyvsp[(4) - (6)].var)); }
    break;

  case 66:
#line 402 "xkbparse.y"
    { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[(1) - (3)].var)->common,&(yyvsp[(3) - (3)].var)->common); }
    break;

  case 67:
#line 404 "xkbparse.y"
    { (yyval.var)= (yyvsp[(1) - (1)].var); }
    break;

  case 68:
#line 405 "xkbparse.y"
    { (yyval.var)= NULL; }
    break;

  case 69:
#line 409 "xkbparse.y"
    { (yyval.var)= VarCreate((yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 70:
#line 411 "xkbparse.y"
    { (yyval.var)= VarCreate((yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 71:
#line 413 "xkbparse.y"
    { (yyval.var)= BoolVarCreate((yyvsp[(1) - (1)].sval),1); }
    break;

  case 72:
#line 415 "xkbparse.y"
    { (yyval.var)= BoolVarCreate((yyvsp[(2) - (2)].sval),0); }
    break;

  case 73:
#line 417 "xkbparse.y"
    { (yyval.var)= VarCreate(NULL,(yyvsp[(1) - (1)].expr)); }
    break;

  case 74:
#line 421 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(2) - (3)].expr); }
    break;

  case 75:
#line 423 "xkbparse.y"
    { (yyval.expr)= ExprCreateUnary(ExprActionList,TypeAction,(yyvsp[(2) - (3)].expr)); }
    break;

  case 76:
#line 427 "xkbparse.y"
    { (yyval.groupCompat)= GroupCompatCreate((yyvsp[(2) - (5)].ival),(yyvsp[(4) - (5)].expr)); }
    break;

  case 77:
#line 431 "xkbparse.y"
    { (yyval.modMask)= ModMapCreate((yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].expr)); }
    break;

  case 78:
#line 435 "xkbparse.y"
    { (yyval.ledMap)= IndicatorMapCreate((yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].var)); }
    break;

  case 79:
#line 439 "xkbparse.y"
    { (yyval.ledName)= IndicatorNameCreate((yyvsp[(2) - (5)].ival),(yyvsp[(4) - (5)].expr),False); }
    break;

  case 80:
#line 441 "xkbparse.y"
    { (yyval.ledName)= IndicatorNameCreate((yyvsp[(3) - (6)].ival),(yyvsp[(5) - (6)].expr),True); }
    break;

  case 81:
#line 445 "xkbparse.y"
    { (yyval.shape)= ShapeDeclCreate((yyvsp[(2) - (6)].sval),(OutlineDef *)&(yyvsp[(4) - (6)].outline)->common); }
    break;

  case 82:
#line 447 "xkbparse.y"
    { 
			    OutlineDef *outlines;
			    outlines= OutlineCreate(None,(yyvsp[(4) - (6)].expr));
			    (yyval.shape)= ShapeDeclCreate((yyvsp[(2) - (6)].sval),outlines);
			}
    break;

  case 83:
#line 455 "xkbparse.y"
    { (yyval.section)= SectionDeclCreate((yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].row)); }
    break;

  case 84:
#line 459 "xkbparse.y"
    { (yyval.row)=(RowDef *)AppendStmt(&(yyvsp[(1) - (2)].row)->common,&(yyvsp[(2) - (2)].row)->common);}
    break;

  case 85:
#line 461 "xkbparse.y"
    { (yyval.row)= (yyvsp[(1) - (1)].row); }
    break;

  case 86:
#line 465 "xkbparse.y"
    { (yyval.row)= RowDeclCreate((yyvsp[(3) - (5)].key)); }
    break;

  case 87:
#line 467 "xkbparse.y"
    { (yyval.row)= (RowDef *)(yyvsp[(1) - (1)].var); }
    break;

  case 88:
#line 469 "xkbparse.y"
    { (yyval.row)= (RowDef *)(yyvsp[(1) - (1)].doodad); }
    break;

  case 89:
#line 471 "xkbparse.y"
    { (yyval.row)= (RowDef *)(yyvsp[(1) - (1)].ledMap); }
    break;

  case 90:
#line 473 "xkbparse.y"
    { (yyval.row)= (RowDef *)(yyvsp[(1) - (1)].overlay); }
    break;

  case 91:
#line 477 "xkbparse.y"
    { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[(1) - (2)].key)->common,&(yyvsp[(2) - (2)].key)->common);}
    break;

  case 92:
#line 479 "xkbparse.y"
    { (yyval.key)= (yyvsp[(1) - (1)].key); }
    break;

  case 93:
#line 483 "xkbparse.y"
    { (yyval.key)= (yyvsp[(3) - (5)].key); }
    break;

  case 94:
#line 485 "xkbparse.y"
    { (yyval.key)= (KeyDef *)(yyvsp[(1) - (1)].var); }
    break;

  case 95:
#line 489 "xkbparse.y"
    { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[(1) - (3)].key)->common,&(yyvsp[(3) - (3)].key)->common);}
    break;

  case 96:
#line 491 "xkbparse.y"
    { (yyval.key)= (yyvsp[(1) - (1)].key); }
    break;

  case 97:
#line 495 "xkbparse.y"
    { (yyval.key)= KeyDeclCreate((yyvsp[(1) - (1)].str),NULL); }
    break;

  case 98:
#line 497 "xkbparse.y"
    { (yyval.key)= KeyDeclCreate(NULL,(yyvsp[(2) - (3)].expr)); }
    break;

  case 99:
#line 501 "xkbparse.y"
    { (yyval.overlay)= OverlayDeclCreate((yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].olKey)); }
    break;

  case 100:
#line 505 "xkbparse.y"
    { 
			    (yyval.olKey)= (OverlayKeyDef *)
				AppendStmt(&(yyvsp[(1) - (3)].olKey)->common,&(yyvsp[(3) - (3)].olKey)->common);
			}
    break;

  case 101:
#line 510 "xkbparse.y"
    { (yyval.olKey)= (yyvsp[(1) - (1)].olKey); }
    break;

  case 102:
#line 514 "xkbparse.y"
    { (yyval.olKey)= OverlayKeyCreate((yyvsp[(1) - (3)].str),(yyvsp[(3) - (3)].str)); }
    break;

  case 103:
#line 518 "xkbparse.y"
    { (yyval.outline)=(OutlineDef *)AppendStmt(&(yyvsp[(1) - (3)].outline)->common,&(yyvsp[(3) - (3)].outline)->common);}
    break;

  case 104:
#line 520 "xkbparse.y"
    { (yyval.outline)= (yyvsp[(1) - (1)].outline); }
    break;

  case 105:
#line 524 "xkbparse.y"
    { (yyval.outline)= OutlineCreate(None,(yyvsp[(2) - (3)].expr)); }
    break;

  case 106:
#line 526 "xkbparse.y"
    { (yyval.outline)= OutlineCreate((yyvsp[(1) - (5)].sval),(yyvsp[(4) - (5)].expr)); }
    break;

  case 107:
#line 528 "xkbparse.y"
    { (yyval.outline)= OutlineCreate((yyvsp[(1) - (3)].sval),(yyvsp[(3) - (3)].expr)); }
    break;

  case 108:
#line 532 "xkbparse.y"
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[(1) - (3)].expr)->common,&(yyvsp[(3) - (3)].expr)->common); }
    break;

  case 109:
#line 534 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 110:
#line 538 "xkbparse.y"
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprCoord,TypeUnknown);
			    expr->value.coord.x= (yyvsp[(2) - (5)].ival);
			    expr->value.coord.y= (yyvsp[(4) - (5)].ival);
			    (yyval.expr)= expr;
			}
    break;

  case 111:
#line 548 "xkbparse.y"
    { (yyval.doodad)= DoodadCreate((yyvsp[(1) - (6)].uval),(yyvsp[(2) - (6)].sval),(yyvsp[(4) - (6)].var)); }
    break;

  case 112:
#line 551 "xkbparse.y"
    { (yyval.uval)= XkbTextDoodad; }
    break;

  case 113:
#line 552 "xkbparse.y"
    { (yyval.uval)= XkbOutlineDoodad; }
    break;

  case 114:
#line 553 "xkbparse.y"
    { (yyval.uval)= XkbSolidDoodad; }
    break;

  case 115:
#line 554 "xkbparse.y"
    { (yyval.uval)= XkbLogoDoodad; }
    break;

  case 116:
#line 557 "xkbparse.y"
    { (yyval.sval)= (yyvsp[(1) - (1)].sval); }
    break;

  case 117:
#line 558 "xkbparse.y"
    { (yyval.sval)= (yyvsp[(1) - (1)].sval); }
    break;

  case 118:
#line 562 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"action",False); }
    break;

  case 119:
#line 564 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"interpret",False); }
    break;

  case 120:
#line 566 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"type",False); }
    break;

  case 121:
#line 568 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"key",False); }
    break;

  case 122:
#line 570 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"group",False); }
    break;

  case 123:
#line 572 "xkbparse.y"
    {(yyval.sval)=XkbInternAtom(NULL,"modifier_map",False);}
    break;

  case 124:
#line 574 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"indicator",False); }
    break;

  case 125:
#line 576 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"shape",False); }
    break;

  case 126:
#line 578 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"row",False); }
    break;

  case 127:
#line 580 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"section",False); }
    break;

  case 128:
#line 582 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"text",False); }
    break;

  case 129:
#line 585 "xkbparse.y"
    { (yyval.uval)= (yyvsp[(1) - (1)].uval); }
    break;

  case 130:
#line 586 "xkbparse.y"
    { (yyval.uval)= MergeDefault; }
    break;

  case 131:
#line 589 "xkbparse.y"
    { (yyval.uval)= MergeDefault; }
    break;

  case 132:
#line 590 "xkbparse.y"
    { (yyval.uval)= MergeAugment; }
    break;

  case 133:
#line 591 "xkbparse.y"
    { (yyval.uval)= MergeOverride; }
    break;

  case 134:
#line 592 "xkbparse.y"
    { (yyval.uval)= MergeReplace; }
    break;

  case 135:
#line 593 "xkbparse.y"
    { (yyval.uval)= MergeAltForm; }
    break;

  case 136:
#line 596 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 137:
#line 597 "xkbparse.y"
    { (yyval.expr)= NULL; }
    break;

  case 138:
#line 601 "xkbparse.y"
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[(1) - (3)].expr)->common,&(yyvsp[(3) - (3)].expr)->common); }
    break;

  case 139:
#line 603 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 140:
#line 607 "xkbparse.y"
    { (yyval.expr)= ExprCreateBinary(OpDivide,(yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 141:
#line 609 "xkbparse.y"
    { (yyval.expr)= ExprCreateBinary(OpAdd,(yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 142:
#line 611 "xkbparse.y"
    { (yyval.expr)= ExprCreateBinary(OpSubtract,(yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 143:
#line 613 "xkbparse.y"
    { (yyval.expr)= ExprCreateBinary(OpMultiply,(yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 144:
#line 615 "xkbparse.y"
    { (yyval.expr)= ExprCreateBinary(OpAssign,(yyvsp[(1) - (3)].expr),(yyvsp[(3) - (3)].expr)); }
    break;

  case 145:
#line 617 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 146:
#line 621 "xkbparse.y"
    { (yyval.expr)= ExprCreateUnary(OpNegate,(yyvsp[(2) - (2)].expr)->type,(yyvsp[(2) - (2)].expr)); }
    break;

  case 147:
#line 623 "xkbparse.y"
    { (yyval.expr)= ExprCreateUnary(OpUnaryPlus,(yyvsp[(2) - (2)].expr)->type,(yyvsp[(2) - (2)].expr)); }
    break;

  case 148:
#line 625 "xkbparse.y"
    { (yyval.expr)= ExprCreateUnary(OpNot,TypeBoolean,(yyvsp[(2) - (2)].expr)); }
    break;

  case 149:
#line 627 "xkbparse.y"
    { (yyval.expr)= ExprCreateUnary(OpInvert,(yyvsp[(2) - (2)].expr)->type,(yyvsp[(2) - (2)].expr)); }
    break;

  case 150:
#line 629 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr);  }
    break;

  case 151:
#line 631 "xkbparse.y"
    { (yyval.expr)= ActionCreate((yyvsp[(1) - (4)].sval),(yyvsp[(3) - (4)].expr)); }
    break;

  case 152:
#line 633 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr);  }
    break;

  case 153:
#line 635 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(2) - (3)].expr);  }
    break;

  case 154:
#line 639 "xkbparse.y"
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[(1) - (3)].expr)->common,&(yyvsp[(3) - (3)].expr)->common); }
    break;

  case 155:
#line 641 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 156:
#line 645 "xkbparse.y"
    { (yyval.expr)= ActionCreate((yyvsp[(1) - (4)].sval),(yyvsp[(3) - (4)].expr)); }
    break;

  case 157:
#line 649 "xkbparse.y"
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprIdent,TypeUnknown);
                            expr->value.str= (yyvsp[(1) - (1)].sval);
                            (yyval.expr)= expr;
			}
    break;

  case 158:
#line 656 "xkbparse.y"
    {
                            ExprDef *expr;
                            expr= ExprCreate(ExprFieldRef,TypeUnknown);
                            expr->value.field.element= (yyvsp[(1) - (3)].sval);
                            expr->value.field.field= (yyvsp[(3) - (3)].sval);
                            (yyval.expr)= expr;
			}
    break;

  case 159:
#line 664 "xkbparse.y"
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= None;
			    expr->value.array.field= (yyvsp[(1) - (4)].sval);
			    expr->value.array.entry= (yyvsp[(3) - (4)].expr);
			    (yyval.expr)= expr;
			}
    break;

  case 160:
#line 673 "xkbparse.y"
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= (yyvsp[(1) - (6)].sval);
			    expr->value.array.field= (yyvsp[(3) - (6)].sval);
			    expr->value.array.entry= (yyvsp[(5) - (6)].expr);
			    (yyval.expr)= expr;
			}
    break;

  case 161:
#line 684 "xkbparse.y"
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeString);
                            expr->value.str= (yyvsp[(1) - (1)].sval);
                            (yyval.expr)= expr;
			}
    break;

  case 162:
#line 691 "xkbparse.y"
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeInt);
                            expr->value.ival= (yyvsp[(1) - (1)].ival);
                            (yyval.expr)= expr;
			}
    break;

  case 163:
#line 698 "xkbparse.y"
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeFloat);
			    expr->value.ival= (yyvsp[(1) - (1)].ival);
			    (yyval.expr)= expr;
			}
    break;

  case 164:
#line 705 "xkbparse.y"
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeKeyName);
			    memset(expr->value.keyName,0,5);
			    strncpy(expr->value.keyName,(yyvsp[(1) - (1)].str),4);
			    free((yyvsp[(1) - (1)].str));
			    (yyval.expr)= expr;
			}
    break;

  case 165:
#line 715 "xkbparse.y"
    { (yyval.expr)= (yyvsp[(1) - (1)].expr); }
    break;

  case 166:
#line 716 "xkbparse.y"
    { (yyval.expr)= NULL; }
    break;

  case 167:
#line 720 "xkbparse.y"
    { (yyval.expr)= AppendKeysymList((yyvsp[(1) - (3)].expr),(KeySym)(yyvsp[(3) - (3)].uval)); }
    break;

  case 168:
#line 722 "xkbparse.y"
    { (yyval.expr)= CreateKeysymList((KeySym)(yyvsp[(1) - (1)].uval)); }
    break;

  case 169:
#line 726 "xkbparse.y"
    { 
			    KeySym sym;
			    if (LookupKeysym(scanStr,&sym))
				(yyval.uval)= sym;
			    else {
				char buf[120];
				sprintf(buf,"expected keysym, got %s",
							uStringText(scanStr));
				yyerror(buf);
				yynerrs++;
				(yyval.uval)= NoSymbol;
			    }
			}
    break;

  case 170:
#line 740 "xkbparse.y"
    {
			    (yyval.uval)= XK_section;
			}
    break;

  case 171:
#line 744 "xkbparse.y"
    {
			    if ((yyvsp[(1) - (1)].ival)<10)	(yyval.uval)= (yyvsp[(1) - (1)].ival)+'0';	/* XK_0 .. XK_9 */
			    else	(yyval.uval)= (yyvsp[(1) - (1)].ival);
			}
    break;

  case 172:
#line 750 "xkbparse.y"
    { (yyval.ival)= -(yyvsp[(2) - (2)].ival); }
    break;

  case 173:
#line 751 "xkbparse.y"
    { (yyval.ival)= (yyvsp[(1) - (1)].ival); }
    break;

  case 174:
#line 754 "xkbparse.y"
    { (yyval.ival)= scanInt; }
    break;

  case 175:
#line 755 "xkbparse.y"
    { (yyval.ival)= scanInt*XkbGeomPtsPerMM; }
    break;

  case 176:
#line 758 "xkbparse.y"
    { (yyval.ival)= scanInt; }
    break;

  case 177:
#line 761 "xkbparse.y"
    { (yyval.ival)= scanInt; }
    break;

  case 178:
#line 764 "xkbparse.y"
    { (yyval.str)= scanStr; scanStr= NULL; }
    break;

  case 179:
#line 767 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,scanStr,False); }
    break;

  case 180:
#line 768 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,"default",False); }
    break;

  case 181:
#line 771 "xkbparse.y"
    { (yyval.sval)= XkbInternAtom(NULL,scanStr,False); }
    break;

  case 182:
#line 774 "xkbparse.y"
    { (yyval.str)= (yyvsp[(1) - (1)].str); }
    break;

  case 183:
#line 775 "xkbparse.y"
    { (yyval.str)= NULL; }
    break;

  case 184:
#line 778 "xkbparse.y"
    { (yyval.str)= scanStr; scanStr= NULL; }
    break;


/* Line 1267 of yacc.c.  */
#line 2929 "xkbparse.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 780 "xkbparse.y"

void
yyerror(const char *s)
{
    if (warningLevel>0) {
	(void)fprintf(stderr,"%s: line %d of %s\n",s,lineNum,
					(scanFile?scanFile:"(unknown)"));
	if ((scanStr)&&(warningLevel>3))
	    (void)fprintf(stderr,"last scanned symbol is: %s\n",scanStr);
    }
    return;
}


int
yywrap(void)
{
   return 1;
}


