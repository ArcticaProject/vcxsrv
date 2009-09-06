
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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
     SILENTEXIT = 284,
     STRING = 285
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
#define SILENTEXIT 284
#define STRING 285




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 89 "winprefsyacc.y"

  char *sVal;
  unsigned long uVal;
  int iVal;



/* Line 1676 of yacc.c  */
#line 120 "winprefsyacc.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


