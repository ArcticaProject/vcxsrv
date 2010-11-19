/* Allocate and clear storage for bison,
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


#include <stdio.h>

extern "C" char *calloc (unsigned,unsigned);
extern "C" char *realloc (char*,unsigned);
extern void done (int);

extern char *program_name;

char *
xmalloc (unsigned n)
{
  register char *block;

  /* Avoid uncertainty about what an arg of 0 will do.  */
  if (n == 0)
    n = 1;
  block = calloc (n, 1);
  if (block == NULL)
    {
      fprintf (stderr, "%s: memory exhausted\n", program_name);
      done (1);
    }

  return (block);
}

char *
xrealloc (char* block, unsigned n)
{
  /* Avoid uncertainty about what an arg of 0 will do.  */
  if (n == 0)
    n = 1;
  block = realloc (block, n);
  if (block == NULL)
    {
      fprintf (stderr, "%s: memory exhausted\n", program_name);
      done (1);
    }

  return (block);
}
