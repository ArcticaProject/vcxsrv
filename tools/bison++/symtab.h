/* Definitions for symtab.c and callers, part of bison,
   Copyright (C) 1984, 1989 Free Software Foundation, Inc.

This file is part of Bison, the GNU Compiler Compiler.

Bison is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Bison is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bison; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#define	TABSIZE	1009


/*  symbol classes  */

#define SUNKNOWN 0
#define STOKEN	 1
#define SNTERM	 2


typedef
  struct bucket
    {
      struct bucket *link;
      struct bucket *next;
      char *tag;
      char *type_name;
      short value;
      short prec;
      short assoc;
      short user_token_number;
      char internalClass;
    }
  bucket;


extern bucket **symtab;
extern bucket *firstsymbol;

extern bucket *getsym(char*);
