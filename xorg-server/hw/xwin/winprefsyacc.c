
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "winprefsyacc.y"

/*
 * Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 * Copyright (C) Colin Harrison 2005-2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from the XFree86 Project.
 *
 * Authors:     Earle F. Philhower, III
 *              Colin Harrison
 */
/* $XFree86: $ */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "winprefs.h"

/* The following give better error messages in bison at the cost of a few KB */
#define YYERROR_VERBOSE 1

/* YYLTYPE_IS_TRIVIAL and YYENABLE_NLS defined to suppress warnings */
#define YYLTYPE_IS_TRIVIAL 1
#define YYENABLE_NLS 0

/* The global pref settings */
WINPREFS pref;

/* The working menu */  
static MENUPARSED menu;

/* Functions for parsing the tokens into out structure */
/* Defined at the end section of this file */

static void SetIconDirectory (char *path);
static void SetDefaultIcon (char *fname);
static void SetRootMenu (char *menu);
static void SetDefaultSysMenu (char *menu, int pos);
static void SetTrayIcon (char *fname);

static void OpenMenu(char *menuname);
static void AddMenuLine(char *name, MENUCOMMANDTYPE cmd, char *param);
static void CloseMenu(void);

static void OpenIcons(void);
static void AddIconLine(char *matchstr, char *iconfile);
static void CloseIcons(void);

static void OpenStyles(void);
static void AddStyleLine(char *matchstr, unsigned long style);
static void CloseStyles(void);

static void OpenSysMenu(void);
static void AddSysMenuLine(char *matchstr, char *menuname, int pos);
static void CloseSysMenu(void);

static int yyerror (char *s);

extern void ErrorF (const char* /*f*/, ...);
extern char *yytext;
extern int yylex(void);



/* Line 189 of yacc.c  */
#line 162 "winprefsyacc.c"

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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NEWLINE = 258,
     MENU = 259,
     LB = 260,
     RB = 261,
     ICONDIRECTORY = 262,
     DEFAULTICON = 263,
     ICONS = 264,
     STYLES = 265,
     TOPMOST = 266,
     MAXIMIZE = 267,
     MINIMIZE = 268,
     BOTTOM = 269,
     NOTITLE = 270,
     OUTLINE = 271,
     NOFRAME = 272,
     DEFAULTSYSMENU = 273,
     SYSMENU = 274,
     ROOTMENU = 275,
     SEPARATOR = 276,
     ATSTART = 277,
     ATEND = 278,
     EXEC = 279,
     ALWAYSONTOP = 280,
     DEBUG = 281,
     RELOAD = 282,
     TRAYICON = 283,
     FORCEEXIT = 284,
     SILENTEXIT = 285,
     STRING = 286
   };
#endif
/* Tokens.  */
#define NEWLINE 258
#define MENU 259
#define LB 260
#define RB 261
#define ICONDIRECTORY 262
#define DEFAULTICON 263
#define ICONS 264
#define STYLES 265
#define TOPMOST 266
#define MAXIMIZE 267
#define MINIMIZE 268
#define BOTTOM 269
#define NOTITLE 270
#define OUTLINE 271
#define NOFRAME 272
#define DEFAULTSYSMENU 273
#define SYSMENU 274
#define ROOTMENU 275
#define SEPARATOR 276
#define ATSTART 277
#define ATEND 278
#define EXEC 279
#define ALWAYSONTOP 280
#define DEBUG 281
#define RELOAD 282
#define TRAYICON 283
#define FORCEEXIT 284
#define SILENTEXIT 285
#define STRING 286




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 89 "winprefsyacc.y"

  char *sVal;
  unsigned long uVal;
  int iVal;



/* Line 214 of yacc.c  */
#line 268 "winprefsyacc.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 280 "winprefsyacc.c"

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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   98

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  33
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNRULES -- Number of states.  */
#define YYNSTATES  121

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    12,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    35,    37,
      39,    43,    47,    52,    56,    60,    64,    69,    75,    81,
      86,    88,    91,    92,   100,   105,   107,   110,   111,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   139,
     142,   147,   149,   152,   153,   160,   161,   163,   165,   171,
     173,   176,   177,   185,   188,   191
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      33,     0,    -1,    -1,    33,    34,    -1,     3,    -1,    36,
      -1,    -1,     3,    35,    -1,    40,    -1,    41,    -1,    44,
      -1,    48,    -1,    55,    -1,    60,    -1,    38,    -1,    39,
      -1,    64,    -1,    37,    -1,    62,    -1,    63,    -1,    28,
      31,     3,    -1,    20,    31,     3,    -1,    18,    31,    57,
       3,    -1,     8,    31,     3,    -1,     7,    31,     3,    -1,
      21,     3,    35,    -1,    31,    25,     3,    35,    -1,    31,
      24,    31,     3,    35,    -1,    31,     4,    31,     3,    35,
      -1,    31,    27,     3,    35,    -1,    42,    -1,    42,    43,
      -1,    -1,     4,    31,     5,    45,    35,    43,     6,    -1,
      31,    31,     3,    35,    -1,    46,    -1,    46,    47,    -1,
      -1,     9,     5,    49,    35,    47,     6,    -1,    11,    -1,
      12,    -1,    13,    -1,    14,    -1,    15,    -1,    16,    -1,
      17,    -1,    50,    -1,    51,    -1,    50,    51,    -1,    51,
      50,    -1,    31,    52,     3,    35,    -1,    53,    -1,    53,
      54,    -1,    -1,    10,     5,    56,    35,    54,     6,    -1,
      -1,    22,    -1,    23,    -1,    31,    31,    57,     3,    35,
      -1,    58,    -1,    58,    59,    -1,    -1,    19,     5,     3,
      61,    35,    59,     6,    -1,    29,     3,    -1,    30,     3,
      -1,    26,    31,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   132,   132,   133,   136,   137,   141,   142,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     159,   162,   165,   168,   171,   174,   175,   176,   177,   178,
     181,   182,   185,   185,   188,   191,   192,   195,   195,   198,
     199,   200,   201,   204,   205,   206,   209,   210,   211,   212,
     215,   218,   219,   222,   222,   225,   226,   227,   230,   233,
     234,   237,   237,   240,   243,   246
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NEWLINE", "MENU", "LB", "RB",
  "ICONDIRECTORY", "DEFAULTICON", "ICONS", "STYLES", "TOPMOST", "MAXIMIZE",
  "MINIMIZE", "BOTTOM", "NOTITLE", "OUTLINE", "NOFRAME", "DEFAULTSYSMENU",
  "SYSMENU", "ROOTMENU", "SEPARATOR", "ATSTART", "ATEND", "EXEC",
  "ALWAYSONTOP", "DEBUG", "RELOAD", "TRAYICON", "FORCEEXIT", "SILENTEXIT",
  "STRING", "$accept", "input", "line", "newline_or_nada", "command",
  "trayicon", "rootmenu", "defaultsysmenu", "defaulticon", "icondirectory",
  "menuline", "menulist", "menu", "$@1", "iconline", "iconlist", "icons",
  "$@2", "group1", "group2", "stylecombo", "styleline", "stylelist",
  "styles", "$@3", "atspot", "sysmenuline", "sysmenulist", "sysmenu",
  "$@4", "forceexit", "silentexit", "debug", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    33,    34,    34,    35,    35,    36,    36,
      36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
      37,    38,    39,    40,    41,    42,    42,    42,    42,    42,
      43,    43,    45,    44,    46,    47,    47,    49,    48,    50,
      50,    50,    50,    51,    51,    51,    52,    52,    52,    52,
      53,    54,    54,    56,    55,    57,    57,    57,    58,    59,
      59,    61,    60,    62,    63,    64
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     3,     3,     3,     4,     5,     5,     4,
       1,     2,     0,     7,     4,     1,     2,     0,     6,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     1,     2,     0,     6,     0,     1,     1,     5,     1,
       2,     0,     7,     2,     2,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     5,    17,    14,
      15,     8,     9,    10,    11,    12,    13,    18,    19,    16,
       0,     0,     0,    37,    53,    55,     0,     0,     0,     0,
      63,    64,    32,    24,    23,     6,     6,    56,    57,     0,
      61,    21,    65,    20,     6,     6,     0,     0,    22,     6,
       0,     7,     0,    35,     0,     0,    51,     0,     0,     0,
       0,    30,     0,     0,    36,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    52,    54,     0,    59,
       0,     6,     0,     0,     0,     0,    31,    33,     6,    48,
      49,     6,    55,    60,    62,    25,     0,     0,     6,     6,
      34,    50,     0,     6,     6,    26,    29,     6,    28,    27,
      58
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    16,    56,    17,    18,    19,    20,    21,    22,
      71,    72,    23,    54,    63,    64,    24,    45,    83,    84,
      85,    66,    67,    25,    46,    49,    89,    90,    26,    59,
      27,    28,    29
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -47
static const yytype_int8 yypact[] =
{
     -47,     7,   -47,   -47,    -1,     0,     1,    18,    29,    15,
      42,    17,    19,    20,    46,    50,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
      49,    53,    54,   -47,   -47,     6,    55,    56,    57,    58,
     -47,   -47,   -47,   -47,   -47,    61,    61,   -47,   -47,    62,
     -47,   -47,   -47,   -47,    61,    61,    35,    38,   -47,    61,
     -19,   -47,    39,    35,    66,    27,    38,    67,    43,    72,
      -3,   -19,    70,    74,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,     3,    -8,    75,   -47,   -47,    48,    43,
      76,    61,    52,    59,    77,    78,   -47,   -47,    61,   -47,
     -47,    61,     6,   -47,   -47,   -47,    81,    82,    61,    61,
     -47,   -47,    83,    61,    61,   -47,   -47,    61,   -47,   -47,
     -47
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -47,   -47,   -47,   -46,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,    16,   -47,   -47,   -47,    25,   -47,   -47,     5,     8,
     -47,   -47,    26,   -47,   -47,    -9,   -47,     9,   -47,   -47,
     -47,   -47,   -47
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      57,    92,    69,    76,    77,    78,    79,     2,    60,    61,
       3,     4,    70,    68,     5,     6,     7,     8,    80,    81,
      82,    93,    94,    33,    95,     9,    10,    11,    47,    48,
      30,    31,    32,    12,    34,    13,    14,    15,    76,    77,
      78,    79,    80,    81,    82,   105,    35,    36,    37,    40,
      38,    39,   110,    41,    42,   111,    43,    44,    50,    51,
      52,    53,   115,   116,    55,    58,    62,   118,   119,    65,
      73,   120,    75,    87,    88,    91,    97,    98,   101,   102,
     108,   109,   104,   106,   113,   114,   117,    96,    74,   100,
     107,    99,    86,   112,     0,     0,     0,     0,   103
};

static const yytype_int8 yycheck[] =
{
      46,     4,    21,    11,    12,    13,    14,     0,    54,    55,
       3,     4,    31,    59,     7,     8,     9,    10,    15,    16,
      17,    24,    25,     5,    27,    18,    19,    20,    22,    23,
      31,    31,    31,    26,     5,    28,    29,    30,    11,    12,
      13,    14,    15,    16,    17,    91,    31,     5,    31,     3,
      31,    31,    98,     3,     5,   101,     3,     3,     3,     3,
       3,     3,   108,   109,     3,     3,    31,   113,   114,    31,
      31,   117,     6,     6,    31,     3,     6,     3,     3,    31,
       3,     3,     6,    31,     3,     3,     3,    71,    63,    84,
      31,    83,    66,   102,    -1,    -1,    -1,    -1,    89
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    33,     0,     3,     4,     7,     8,     9,    10,    18,
      19,    20,    26,    28,    29,    30,    34,    36,    37,    38,
      39,    40,    41,    44,    48,    55,    60,    62,    63,    64,
      31,    31,    31,     5,     5,    31,     5,    31,    31,    31,
       3,     3,     5,     3,     3,    49,    56,    22,    23,    57,
       3,     3,     3,     3,    45,     3,    35,    35,     3,    61,
      35,    35,    31,    46,    47,    31,    53,    54,    35,    21,
      31,    42,    43,    31,    47,     6,    11,    12,    13,    14,
      15,    16,    17,    50,    51,    52,    54,     6,    31,    58,
      59,     3,     4,    24,    25,    27,    43,     6,     3,    51,
      50,     3,    31,    59,     6,    35,    31,    31,     3,     3,
      35,    35,    57,     3,     3,    35,    35,     3,    35,    35,
      35
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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
        case 20:

/* Line 1455 of yacc.c  */
#line 159 "winprefsyacc.y"
    { SetTrayIcon((yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 162 "winprefsyacc.y"
    { SetRootMenu((yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 165 "winprefsyacc.y"
    { SetDefaultSysMenu((yyvsp[(2) - (4)].sVal), (yyvsp[(3) - (4)].iVal)); free((yyvsp[(2) - (4)].sVal)); }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 168 "winprefsyacc.y"
    { SetDefaultIcon((yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 171 "winprefsyacc.y"
    { SetIconDirectory((yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 174 "winprefsyacc.y"
    { AddMenuLine("-", CMD_SEPARATOR, ""); }
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 175 "winprefsyacc.y"
    { AddMenuLine((yyvsp[(1) - (4)].sVal), CMD_ALWAYSONTOP, ""); free((yyvsp[(1) - (4)].sVal)); }
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 176 "winprefsyacc.y"
    { AddMenuLine((yyvsp[(1) - (5)].sVal), CMD_EXEC, (yyvsp[(3) - (5)].sVal)); free((yyvsp[(1) - (5)].sVal)); free((yyvsp[(3) - (5)].sVal)); }
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 177 "winprefsyacc.y"
    { AddMenuLine((yyvsp[(1) - (5)].sVal), CMD_MENU, (yyvsp[(3) - (5)].sVal)); free((yyvsp[(1) - (5)].sVal)); free((yyvsp[(3) - (5)].sVal)); }
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 178 "winprefsyacc.y"
    { AddMenuLine((yyvsp[(1) - (4)].sVal), CMD_RELOAD, ""); free((yyvsp[(1) - (4)].sVal)); }
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 185 "winprefsyacc.y"
    { OpenMenu((yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 185 "winprefsyacc.y"
    {CloseMenu();}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 188 "winprefsyacc.y"
    { AddIconLine((yyvsp[(1) - (4)].sVal), (yyvsp[(2) - (4)].sVal)); free((yyvsp[(1) - (4)].sVal)); free((yyvsp[(2) - (4)].sVal)); }
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 195 "winprefsyacc.y"
    {OpenIcons();}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 195 "winprefsyacc.y"
    {CloseIcons();}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 198 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_TOPMOST; }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 199 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_MAXIMIZE; }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 200 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_MINIMIZE; }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 201 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_BOTTOM; }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 204 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_NOTITLE; }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 205 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_OUTLINE; }
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 206 "winprefsyacc.y"
    { (yyval.uVal)=STYLE_NOFRAME; }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 209 "winprefsyacc.y"
    { (yyval.uVal)=(yyvsp[(1) - (1)].uVal); }
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 210 "winprefsyacc.y"
    { (yyval.uVal)=(yyvsp[(1) - (1)].uVal); }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 211 "winprefsyacc.y"
    { (yyval.uVal)=(yyvsp[(1) - (2)].uVal)|(yyvsp[(2) - (2)].uVal); }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 212 "winprefsyacc.y"
    { (yyval.uVal)=(yyvsp[(1) - (2)].uVal)|(yyvsp[(2) - (2)].uVal); }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 215 "winprefsyacc.y"
    { AddStyleLine((yyvsp[(1) - (4)].sVal), (yyvsp[(2) - (4)].uVal)); free((yyvsp[(1) - (4)].sVal)); }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 222 "winprefsyacc.y"
    {OpenStyles();}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 222 "winprefsyacc.y"
    {CloseStyles();}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 225 "winprefsyacc.y"
    { (yyval.iVal)=AT_END; }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 226 "winprefsyacc.y"
    { (yyval.iVal)=AT_START; }
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 227 "winprefsyacc.y"
    { (yyval.iVal)=AT_END; }
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 230 "winprefsyacc.y"
    { AddSysMenuLine((yyvsp[(1) - (5)].sVal), (yyvsp[(2) - (5)].sVal), (yyvsp[(3) - (5)].iVal)); free((yyvsp[(1) - (5)].sVal)); free((yyvsp[(2) - (5)].sVal)); }
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 237 "winprefsyacc.y"
    {OpenSysMenu();}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 237 "winprefsyacc.y"
    {CloseSysMenu();}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 240 "winprefsyacc.y"
    { pref.fForceExit = TRUE; }
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 243 "winprefsyacc.y"
    { pref.fSilentExit = TRUE; }
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 246 "winprefsyacc.y"
    { ErrorF("LoadPreferences: %s\n", (yyvsp[(2) - (3)].sVal)); free((yyvsp[(2) - (3)].sVal)); }
    break;



/* Line 1455 of yacc.c  */
#line 1841 "winprefsyacc.c"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



/* Line 1675 of yacc.c  */
#line 250 "winprefsyacc.y"

/*
 * Errors in parsing abort and print log messages
 */
static int
yyerror (char *s) 
{
  extern int yylineno; /* Handled by flex internally */

  ErrorF("LoadPreferences: %s line %d\n", s, yylineno);
  return 1;
}

/* Miscellaneous functions to store TOKENs into the structure */
static void
SetIconDirectory (char *path)
{
  strncpy (pref.iconDirectory, path, PATH_MAX);
  pref.iconDirectory[PATH_MAX] = 0;
}

static void
SetDefaultIcon (char *fname)
{
  strncpy (pref.defaultIconName, fname, NAME_MAX);
  pref.defaultIconName[NAME_MAX] = 0;
}

static void
SetTrayIcon (char *fname)
{
  strncpy (pref.trayIconName, fname, NAME_MAX);
  pref.trayIconName[NAME_MAX] = 0;
}

static void
SetRootMenu (char *menu)
{
  strncpy (pref.rootMenuName, menu, MENU_MAX);
  pref.rootMenuName[MENU_MAX] = 0;
}

static void
SetDefaultSysMenu (char *menu, int pos)
{
  strncpy (pref.defaultSysMenuName, menu, MENU_MAX);
  pref.defaultSysMenuName[MENU_MAX] = 0;
  pref.defaultSysMenuPos = pos;
}

static void
OpenMenu (char *menuname)
{
  if (menu.menuItem) free(menu.menuItem);
  menu.menuItem = NULL;
  strncpy(menu.menuName, menuname, MENU_MAX);
  menu.menuName[MENU_MAX] = 0;
  menu.menuItems = 0;
}

static void
AddMenuLine (char *text, MENUCOMMANDTYPE cmd, char *param)
{
  if (menu.menuItem==NULL)
    menu.menuItem = (MENUITEM*)malloc(sizeof(MENUITEM));
  else
    menu.menuItem = (MENUITEM*)
      realloc(menu.menuItem, sizeof(MENUITEM)*(menu.menuItems+1));

  strncpy (menu.menuItem[menu.menuItems].text, text, MENU_MAX);
  menu.menuItem[menu.menuItems].text[MENU_MAX] = 0;

  menu.menuItem[menu.menuItems].cmd = cmd;

  strncpy(menu.menuItem[menu.menuItems].param, param, PARAM_MAX);
  menu.menuItem[menu.menuItems].param[PARAM_MAX] = 0;

  menu.menuItem[menu.menuItems].commandID = 0;

  menu.menuItems++;
}

static void
CloseMenu (void)
{
  if (menu.menuItem==NULL || menu.menuItems==0)
    {
      ErrorF("LoadPreferences: Empty menu detected\n");
      return;
    }
  
  if (pref.menuItems)
    pref.menu = (MENUPARSED*)
      realloc (pref.menu, (pref.menuItems+1)*sizeof(MENUPARSED));
  else
    pref.menu = (MENUPARSED*)malloc (sizeof(MENUPARSED));
  
  memcpy (pref.menu+pref.menuItems, &menu, sizeof(MENUPARSED));
  pref.menuItems++;

  memset (&menu, 0, sizeof(MENUPARSED));
}

static void 
OpenIcons (void)
{
  if (pref.icon != NULL) {
    ErrorF("LoadPreferences: Redefining icon mappings\n");
    free(pref.icon);
    pref.icon = NULL;
  }
  pref.iconItems = 0;
}

static void
AddIconLine (char *matchstr, char *iconfile)
{
  if (pref.icon==NULL)
    pref.icon = (ICONITEM*)malloc(sizeof(ICONITEM));
  else
    pref.icon = (ICONITEM*)
      realloc(pref.icon, sizeof(ICONITEM)*(pref.iconItems+1));

  strncpy(pref.icon[pref.iconItems].match, matchstr, MENU_MAX);
  pref.icon[pref.iconItems].match[MENU_MAX] = 0;

  strncpy(pref.icon[pref.iconItems].iconFile, iconfile, PATH_MAX+NAME_MAX+1);
  pref.icon[pref.iconItems].iconFile[PATH_MAX+NAME_MAX+1] = 0;

  pref.icon[pref.iconItems].hicon = 0;

  pref.iconItems++;
}

static void 
CloseIcons (void)
{
}

static void
OpenStyles (void)
{
  if (pref.style != NULL) {
    ErrorF("LoadPreferences: Redefining window style\n");
    free(pref.style);
    pref.style = NULL;
  }
  pref.styleItems = 0;
}

static void
AddStyleLine (char *matchstr, unsigned long style)
{
  if (pref.style==NULL)
    pref.style = (STYLEITEM*)malloc(sizeof(STYLEITEM));
  else
    pref.style = (STYLEITEM*)
      realloc(pref.style, sizeof(STYLEITEM)*(pref.styleItems+1));

  strncpy(pref.style[pref.styleItems].match, matchstr, MENU_MAX);
  pref.style[pref.styleItems].match[MENU_MAX] = 0;

  pref.style[pref.styleItems].type = style;

  pref.styleItems++;
}

static void
CloseStyles (void)
{
}

static void
OpenSysMenu (void)
{
  if (pref.sysMenu != NULL) {
    ErrorF("LoadPreferences: Redefining system menu\n");
    free(pref.sysMenu);
    pref.sysMenu = NULL;
  }
  pref.sysMenuItems = 0;
}

static void
AddSysMenuLine (char *matchstr, char *menuname, int pos)
{
  if (pref.sysMenu==NULL)
    pref.sysMenu = (SYSMENUITEM*)malloc(sizeof(SYSMENUITEM));
  else
    pref.sysMenu = (SYSMENUITEM*)
      realloc(pref.sysMenu, sizeof(SYSMENUITEM)*(pref.sysMenuItems+1));

  strncpy (pref.sysMenu[pref.sysMenuItems].match, matchstr, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].match[MENU_MAX] = 0;

  strncpy (pref.sysMenu[pref.sysMenuItems].menuName, menuname, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].menuName[MENU_MAX] = 0;

  pref.sysMenu[pref.sysMenuItems].menuPos = pos;

  pref.sysMenuItems++;
}

static void
CloseSysMenu (void)
{
}


