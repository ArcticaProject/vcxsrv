/* Token-reader for Bison's input parser,
   Copyright (C) 1984, 1986, 1989 Free Software Foundation, Inc.

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


/* 
   lex() is the entry point.  It is called from reader.c.
   It returns one of the token-type codes defined in lex.h.
   When an identifier is seen, the code IDENTIFIER is returned
   and the name is looked up in the symbol table using symtab.c;
   symval is set to a pointer to the entry found.  */

#include <stdio.h>
#include <ctype.h>
#include "system.h"
#include "files.h"
#include "symtab.h"
#include "lex.h"
#include "new.h"


extern int lineno;
extern int translations;

int parse_percent_token();

extern void fatal(const char*);
extern void fatals(const char*,void*);
extern void fatals(const char*,void*,void*);
extern void fatals(const char*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*,void*);

/* Buffer for storing the current token.  */
char *token_buffer;

/* Allocated size of token_buffer, not including space for terminator.  */
static int maxtoken;

bucket *symval;
int numval;

static int unlexed;		/* these two describe a token to be reread */
static bucket *unlexed_symval;	/* by the next call to lex */


void
init_lex()
{
  maxtoken = 100;
  token_buffer = NEW2 (maxtoken + 1, char);
  unlexed = -1;
}


static char *
grow_token_buffer (char* p)
{
  int offset = p - token_buffer;
  maxtoken *= 2;
  token_buffer = (char *) xrealloc(token_buffer, maxtoken + 1);
  return token_buffer + offset;
}


int
skip_white_space()
{
  register int c;
  register int inside;

  c = getc(finput);

  for (;;)
    {
      int cplus_comment;

      switch (c)
	{
	case '/':
	  c = getc(finput);
	  if (c != '*' && c != '/')
	    fatals("unexpected `/%c' found", (void*) c);
	  cplus_comment = (c == '/');

	  c = getc(finput);

	  inside = 1;
	  while (inside)
	    {
	      if (!cplus_comment && c == '*')
		{
		  while (c == '*')
		    c = getc(finput);

		  if (c == '/')
		    {
		      inside = 0;
		      c = getc(finput);
		    }
		}
	      else if (c == '\n')
		{
		  lineno++;
		  if (cplus_comment)
		    inside = 0;
		  c = getc(finput);
		}
	      else if (c == EOF)
		fatal("unterminated comment");
	      else
		c = getc(finput);
	    }

	  break;

	case '\n':
	  lineno++;

	case ' ':
	case '\t':
	case '\f':
	  c = getc(finput);
	  break;

	default:
	  return (c);
	}
    }
}


void
unlex(int token)
{
  unlexed = token;
  unlexed_symval = symval;
}



int
lex()
{
  register int c;
  register char *p;

  if (unlexed >= 0)
    {
      symval = unlexed_symval;
      c = unlexed;
      unlexed = -1;
      return (c);
    }

  c = skip_white_space();

  switch (c)
    {
    case EOF:
      return (ENDFILE);

    case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
    case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
    case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
    case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
    case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
    case 'Z':
    case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    case 'p':  case 'q':  case 'r':  case 's':  case 't':
    case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
    case 'z':
    case '.':  case '_':
      p = token_buffer;
      while (isalnum(c) || c == '_' || c == '.')
	{
	  if (p == token_buffer + maxtoken)
	    p = grow_token_buffer(p);

	  *p++ = c;
	  c = getc(finput);
	}

      *p = 0;
      ungetc(c, finput);
      symval = getsym(token_buffer);
      return (IDENTIFIER);

    case '0':  case '1':  case '2':  case '3':  case '4':
    case '5':  case '6':  case '7':  case '8':  case '9':
      {
	numval = 0;

	while (isdigit(c))
	  {
	    numval = numval*10 + c - '0';
	    c = getc(finput);
	  }
	ungetc(c, finput);
	return (NUMBER);
      }

    case '\'':
      translations = -1;

      /* parse the literal token and compute character code in  code  */

      c = getc(finput);
      {
	register int code = 0;

	if (c == '\\')
	  {
	    c = getc(finput);

	    if (c <= '7' && c >= '0')
	      {
		while (c <= '7' && c >= '0')
		  {
		    code = (code * 8) + (c - '0');
		    c = getc(finput);
		    if (code >= 256 || code < 0)
		      fatals("malformatted literal token `\\%03o'", (void*) code);
		  }
	      }
	    else
	      {
		if (c == 't')
		  code = '\t';
		else if (c == 'n')
		  code = '\n';
		else if (c == 'a')
		  code = '\007';
		else if (c == 'r')
		  code = '\r';
		else if (c == 'f')
		  code = '\f';
		else if (c == 'b')
		  code = '\b';
		else if (c == 'v')
		  code = 013;
		else if (c == 'x')
		  {
		    c = getc(finput);
		    while ((c <= '9' && c >= '0')
			   || (c >= 'a' && c <= 'z')
			   || (c >= 'A' && c <= 'Z'))
		      {
			code *= 16;
			if (c <= '9' && c >= '0')
			  code += c - '0';
			else if (c >= 'a' && c <= 'z')
			  code += c - 'a' + 10;
			else if (c >= 'A' && c <= 'Z')
			  code += c - 'A' + 10;
			if (code >= 256 || code<0)/* JF this said if(c>=128) */
			  fatals("malformatted literal token `\\x%x'",(void*) code);
			c = getc(finput);
		      }
		    ungetc(c, finput);
		  }
		else if (c == '\\')
		  code = '\\';
		else if (c == '\'')
		  code = '\'';
		else if (c == '\"')	/* JF this is a good idea */
		  code = '\"';
		else
		  {
		    if (c >= 040 && c <= 0177)
		      fatals ("unknown escape sequence `\\%c'", (void*) c);
		    else
		      fatals ("unknown escape sequence: `\\' followed by char code 0x%x", (void*) c);
		  }

		c = getc(finput);
	      }
	  }
	else
	  {
	    code = c;
	    c = getc(finput);
	  }
	if (c != '\'')
	  fatal("multicharacter literal tokens not supported");

	/* now fill token_buffer with the canonical name for this character
	   as a literal token.  Do not use what the user typed,
	   so that '\012' and '\n' can be interchangeable.  */

	p = token_buffer;
	*p++ = '\'';
	if (code == '\\')
	  {
	    *p++ = '\\';
	    *p++ = '\\';
	  }
	else if (code == '\'')
	  {
	    *p++ = '\\';
	    *p++ = '\'';
	  }
	else if (code >= 040 && code != 0177)
	  *p++ = code;
	else if (code == '\t')
	  {
	    *p++ = '\\';
	    *p++ = 't';
	  }
	else if (code == '\n')
	  {
	    *p++ = '\\';
	    *p++ = 'n';
	  }
	else if (code == '\r')
	  {
	    *p++ = '\\';
	    *p++ = 'r';
	  }
	else if (code == '\v')
	  {
	    *p++ = '\\';
	    *p++ = 'v';
	  }
	else if (code == '\b')
	  {
	    *p++ = '\\';
	    *p++ = 'b';
	  }
	else if (code == '\f')
	  {
	    *p++ = '\\';
	    *p++ = 'f';
	  }
        else
	  {
	    *p++ = code / 0100 + '0';
	    *p++ = ((code / 010) & 07) + '0';
	    *p++ = (code & 07) + '0';
	  }
	*p++ = '\'';
	*p = 0;
	symval = getsym(token_buffer);
	symval->internalClass = STOKEN;
	if (! symval->user_token_number)
	  symval->user_token_number = code;
	return (IDENTIFIER);
      }

    case ',':
      return (COMMA);

    case ':':
      return (COLON);

    case ';':
      return (SEMICOLON);

    case '|':
      return (BAR);

    case '{':
      return (LEFT_CURLY);

    case '=':
      do
	{
	  c = getc(finput);
	  if (c == '\n') lineno++;
	}
      while(c==' ' || c=='\n' || c=='\t');

      if (c == '{')
      	return(LEFT_CURLY);
      else
	{
	  ungetc(c, finput);
	  return(ILLEGAL);
	}

    case '<':
      p = token_buffer;
      c = getc(finput);
      while (c != '>')
	{
	  if (c == '\n' || c == EOF)
	    fatal("unterminated type name");

	  if (p == token_buffer + maxtoken)
	    p = grow_token_buffer(p);

	  *p++ = c;
	  c = getc(finput);
	}
      *p = 0;
      return (TYPENAME);
	    

    case '%':
      return (parse_percent_token());

    default:
      return (ILLEGAL);
    }
}


/* parse a token which starts with %.  Assumes the % has already been read and discarded.  */

int
parse_percent_token ()
{
  register int c;
  register char *p;

  p = token_buffer;
  c = getc(finput);

  switch (c)
    {
    case '%':
      return (TWO_PERCENTS);

    case '{':
      return (PERCENT_LEFT_CURLY);

    case '<':
      return (LEFT);

    case '>':
      return (RIGHT);

    case '2':
      return (NONASSOC);

    case '0':
      return (TOKEN);

    case '=':
      return (PREC);
    }
  if (!isalpha(c))
    return (ILLEGAL);

  while (isalpha(c) || c == '_')
    {
      if (p == token_buffer + maxtoken)
	p = grow_token_buffer(p);

      *p++ = c;
      c = getc(finput);
    }

  ungetc(c, finput);

  *p = 0;

  if (strcmp(token_buffer, "token") == 0
      ||
      strcmp(token_buffer, "term") == 0)
    return (TOKEN);
  else if (strcmp(token_buffer, "nterm") == 0)
    return (NTERM);
  else if (strcmp(token_buffer, "type") == 0)
    return (TYPE);
  else if (strcmp(token_buffer, "guard") == 0)
    return (GUARD);
  else if (strcmp(token_buffer, "union") == 0)
    return (UNION);
  else if (strcmp(token_buffer, "expect") == 0)
    return (EXPECT);
  else if (strcmp(token_buffer, "start") == 0)
    return (START);
  else if (strcmp(token_buffer, "left") == 0)
    return (LEFT);
  else if (strcmp(token_buffer, "right") == 0)
    return (RIGHT);
  else if (strcmp(token_buffer, "nonassoc") == 0
	   ||
	   strcmp(token_buffer, "binary") == 0)
    return (NONASSOC);
  else if (strcmp(token_buffer, "semantic_parser") == 0)
    return (SEMANTIC_PARSER);
  else if (strcmp(token_buffer, "pure_parser") == 0)
    return (PURE_PARSER);
  else if (strcmp(token_buffer, "prec") == 0)
    return (PREC);
  else if (strcmp(token_buffer, "name") == 0)
    return (PARSER_NAME);
  else if (strcmp(token_buffer, "define") == 0)
    return (DEFINE_SYM);
  else if (strcmp(token_buffer, "header") == 0)
    { c=getc(finput);
      if(c=='{')
          return (PERCENT_LEFT_CURLY_HEADER);
      else return (ILLEGAL);
    }
  else return (ILLEGAL);
}
