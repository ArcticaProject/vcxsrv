/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
     DEFAULTSYSMENU = 265,
     SYSMENU = 266,
     ROOTMENU = 267,
     SEPARATOR = 268,
     ATSTART = 269,
     ATEND = 270,
     EXEC = 271,
     ALWAYSONTOP = 272,
     DEBUG = 273,
     RELOAD = 274,
     TRAYICON = 275,
     SILENTEXIT = 276,
     STRING = 277
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
#define DEFAULTSYSMENU 265
#define SYSMENU 266
#define ROOTMENU 267
#define SEPARATOR 268
#define ATSTART 269
#define ATEND 270
#define EXEC 271
#define ALWAYSONTOP 272
#define DEBUG 273
#define RELOAD 274
#define TRAYICON 275
#define SILENTEXIT 276
#define STRING 277




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 79 "winprefsyacc.y"
{
  char *sVal;
  int iVal;
}
/* Line 1489 of yacc.c.  */
#line 98 "winprefsyacc.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

