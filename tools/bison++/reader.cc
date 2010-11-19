/* Input parser for bison
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


/* read in the grammar specification and record it in the format described in gram.h.
  All guards are copied into the fguard file and all actions into faction,
  in each case forming the body of a C function (yyguard or yyaction)
  which contains a switch statement to decide which guard or action to execute.

The entry point is reader().  */

#include <stdio.h>
#include <ctype.h>
#include "system.h"
#include "files.h"
#include "new.h"
#include "symtab.h"
#include "lex.h"
#include "gram.h"
#include "machine.h"

extern bool bison_compability;

/* Number of slots allocated (but not necessarily used yet) in `rline'  */
int rline_allocated;

extern char *program_name;
extern int definesflag;
extern int nolinesflag;
extern bucket *symval;
extern int numval;
extern int failure;
extern int expected_conflicts;
extern char *token_buffer;



extern void init_lex();
extern void tabinit();
extern void output_headers();
extern void output_trailers();
extern void free_symtab();
extern void open_extra_files();
extern void fatal(const char*);
extern void fatals(const char*,void*);
extern void fatals(const char*,void*,void*);
extern void fatals(const char*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*,void*);

extern void unlex(int);
extern void done(int);

extern int skip_white_space();
extern int parse_percent_token();
extern int lex();

void read_declarations();
void copy_definition();
void parse_token_decl(int,int);
void parse_start_decl();
void parse_type_decl();
void parse_assoc_decl(int);
void parse_union_decl();
void parse_expect_decl();
void readgram();
void record_rule_line();
void packsymbols();
void output_token_defines();
void packgram();
int read_signed_integer(FILE*);
int get_type();

typedef
  struct symbol_list
    {
      struct symbol_list *next;
      bucket *sym;
      bucket *ruleprec;
    }
  symbol_list;

void copy_action(symbol_list*,int);


int lineno;
symbol_list *grammar;
int start_flag;
bucket *startval;
char **tags;

/* Nonzero if components of semantic values are used, implying
   they must be unions.  */
static int value_components_used;

static int typed;  /* nonzero if %union has been seen.  */

static int lastprec;  /* incremented for each %left, %right or %nonassoc seen */

static int gensym_count;  /* incremented for each generated symbol */

bucket *errtoken;

/* Nonzero if any action or guard uses the @n construct.  */
int yylsp_needed;

extern char *version_string;

extern void output_before_read();
extern  void output_about_token();
void set_parser_name(char*);
void cputc(int);
void hputc(int);
void copy_header_definition();
void parse_name_declaration();
void parse_define();
void read_a_name();

extern FILE *finput;
extern int lineno;


void
copy_a_definition (void (*do_put)(int))
{
  register int c;
  register int match;
  register int ended;
  register int after_percent;  /* -1 while reading a character if prev char was % */
  int cplus_comment;

  after_percent = 0;

  c = getc(finput);

  for (;;)
    {
      switch (c)
	{
	case '\n':
	  (*do_put)(c);
	  lineno++;
	  break;

	case '%':
          after_percent = -1;
	  break;
	      
	case '\'':
	case '"':
	  match = c;
	  (*do_put)(c);
	  c = getc(finput);

	  while (c != match)
	    {
	      if (c == EOF || c == '\n')
		fatal("unterminated string");

	      (*do_put)(c);
	      
	      if (c == '\\')
		{
		  c = getc(finput);
		  if (c == EOF)
		    fatal("unterminated string");
		  (*do_put)(c);
		  if (c == '\n')
		    lineno++;
		}

	      c = getc(finput);
	    }

	  (*do_put)(c);
	  break;

	case '/':
	  (*do_put)(c);
	  c = getc(finput);
	  if (c != '*' && c != '/')
	    continue;

	  cplus_comment = (c == '/');
	  (*do_put)(c);
	  c = getc(finput);

	  ended = 0;
	  while (!ended)
	    {
	      if (!cplus_comment && c == '*')
		{
		  while (c == '*')
		    {
		      (*do_put)(c);
		      c = getc(finput);
		    }

		  if (c == '/')
		    {
		      (*do_put)(c);
		      ended = 1;
		    }
		}
	      else if (c == '\n')
		{
		  lineno++;
		  (*do_put)(c);
		  if (cplus_comment)
		    ended = 1;
		  else
		    c = getc(finput);
		}
	      //else if (c == EOF)
	      //fatal("unterminated comment in `%{' definition");
	      else
		{
		  (*do_put)(c);
		  c = getc(finput);
		}
	    }

	  break;

	case EOF:
	  //fatal("unterminated `%{' definition");

	default:
	  (*do_put)(c);
	}

      c = getc(finput);

      if (after_percent)
	{
	  if (c == '}')
	    return;
          (*do_put)('%');
	}
      after_percent = 0;

    }

}




void
reader()
{
  start_flag = 0;
  startval = NULL;  /* start symbol not specified yet. */

#if 0
  translations = 0;  /* initially assume token number translation not needed.  */
#endif
  /* Nowadays translations is always set to 1,
     since we give `error' a user-token-number
     to satisfy the Posix demand for YYERRCODE==256.  */
  translations = 1;

  nsyms = 1;
  nvars = 0;
  nrules = 0;
  nitems = 0;
  rline_allocated = 10;
  rline = NEW2(rline_allocated, short);

  typed = 0;
  lastprec = 0;

  gensym_count = 0;

  semantic_parser = 0;
  pure_parser = 0;
  yylsp_needed = 0;

  grammar = NULL;

  init_lex();
  lineno = 1;

  /* initialize the symbol table.  */
  tabinit();
  /* construct the error token */
  errtoken = getsym("error");
  errtoken->internalClass = STOKEN;
  errtoken->user_token_number = 256; /* Value specified by posix.  */
  /* construct a token that represents all undefined literal tokens. */
  /* it is always token number 2.  */
  getsym("$illegal.")->internalClass = STOKEN;
  /* Read the declaration section.  Copy %{ ... %} groups to ftable and fdefines file.
     Also notice any %token, %left, etc. found there.  */
  output_before_read();

  read_declarations();
  output_headers();
  /* read in the grammar, build grammar in list form.  write out guards and actions.  */
  readgram();
  /* write closing delimiters for actions and guards.  */
  output_trailers();
  /* assign the symbols their symbol numbers.
     Write #defines for the token symbols into fdefines if requested.  */
  packsymbols();
  /* convert the grammar into the format described in gram.h.  */
  packgram();
  /* free the symbol table data structure
     since symbols are now all referred to by symbol number.  */
  free_symtab();
}



/* read from finput until %% is seen.  Discard the %%.
Handle any % declarations,
and copy the contents of any %{ ... %} groups to ftable.  */

void
read_declarations ()
{
  register int c;
  register int tok;

  for (;;)
    {
      c = skip_white_space();

      if (c == '%')
	{
	  tok = parse_percent_token();

	  switch (tok)
	    {
	    case TWO_PERCENTS:
	      return;

	    case PERCENT_LEFT_CURLY:
	      copy_definition();
	      break;
	    case PERCENT_LEFT_CURLY_HEADER:
	      copy_header_definition();
	      break;

	    case TOKEN:
	      parse_token_decl (STOKEN, SNTERM);
	      break;
	
	    case NTERM:
	      parse_token_decl (SNTERM, STOKEN);
	      break;
	
	    case TYPE:
	      parse_type_decl();
	      break;
	
	    case START:
	      parse_start_decl();
	      break;
	
	    case UNION:
	      parse_union_decl();
	      break;
	
	    case EXPECT:
	      parse_expect_decl();
	      break;
	
	    case LEFT:
	      parse_assoc_decl(LEFT_ASSOC);
	      break;

	    case RIGHT:
	      parse_assoc_decl(RIGHT_ASSOC);
	      break;

	    case NONASSOC:
	      parse_assoc_decl(NON_ASSOC);
	      break;

	    case SEMANTIC_PARSER:
	      if (semantic_parser == 0)
		{
		  semantic_parser = 1;
		  open_extra_files();
		  fprintf(stderr,
                "%semantic_parser no more supported in this version of bison !!! \n errors will be done ! use classic bison, use simple parser, or addapt this version to semantic parser\n");
		}
	      break;

	    case PURE_PARSER:
	      pure_parser = 1;
	      break;

           
	    case PARSER_NAME:
              parse_name_declaration();
	      break;
           
	    case DEFINE_SYM:
              parse_define();
	      break;
           

	    default:
	      fatal("junk after `%%' in definition section");
	    }
	}
      else if (c == EOF)
        fatal("no input grammar");
      else if (c >= 040 && c <= 0177)
	fatals ("unknown character `%c' in declaration section", (void*) c);
      else
	fatals ("unknown character with code 0x%x in declaration section", (void*) c);
    }
}


/* copy the contents of a %{ ... %} into the definitions file.
The %{ has already been read.  Return after reading the %}.  */

void
copy_definition ()
{
 if (!nolinesflag)
    fprintf(ftable, "#line %d \"%s\"\n", lineno, quoted_filename(infile));

 copy_a_definition (cputc);
}

void
copy_header_definition ()
{
  if (!nolinesflag)
    {fprintf(ftable, "#line %d \"%s\"\n", lineno, quoted_filename(infile));
     if(definesflag) 
      fprintf(fdefines, "#line %d \"%s\"\n", lineno, quoted_filename(infile));
    }
 copy_a_definition (hputc);
}
void 
hputc(int c)
{
 putc(c,ftable);
 if(definesflag) putc(c,fdefines);
}

void 
cputc(int c)
{
 putc(c,ftable);
}




/* parse what comes after %token or %nterm.
For %token, what_is is STOKEN and what_is_not is SNTERM.
For %nterm, the arguments are reversed.  */

void
parse_token_decl (int what_is, int  what_is_not)
{
  register int token = 0;
  register int prev;
     char *internalTypename = 0;
  int k;

/*   start_lineno = lineno; JF */

  for (;;)
    {
      if(ungetc(skip_white_space(), finput) == '%')
	return;

/*      if (lineno != start_lineno)
	return; JF */

      /* we have not passed a newline, so the token now starting is in this declaration */
      prev = token;

      token = lex();
      if (token == COMMA)
	continue;
      if (token == TYPENAME)
	{
	  k = strlen(token_buffer);
	  internalTypename = NEW2(k + 1, char);
	  strcpy(internalTypename, token_buffer);
	  value_components_used = 1;
	}
      else if (token == IDENTIFIER)
	{
	  int oldclass = symval->internalClass;

	  if (symval->internalClass == what_is_not)
	    fatals("symbol %s redefined", (void*) symval->tag);
	  symval->internalClass = what_is;
	  if (what_is == SNTERM && oldclass != SNTERM)
	    symval->value = nvars++;

	  if (internalTypename)
	    {
	      if (symval->type_name == NULL)
		symval->type_name = internalTypename;
	      else
		fatals("type redeclaration for %s",(void*)  symval->tag);
	    }
	}
      else if (prev == IDENTIFIER && token == NUMBER)
        {
	  symval->user_token_number = numval;
	  translations = 1;
        }
      else
	fatal("invalid text in %token or %nterm declaration");
    }

}



/* parse what comes after %start */

void
parse_start_decl ()
{
  if (start_flag)
    fatal("multiple %start declarations");
  start_flag = 1;
  if (lex() != IDENTIFIER)
    fatal("invalid %start declaration");
  startval = symval;
}



/* read in a %type declaration and record its information for get_type_name to access */

void
parse_type_decl ()
{
  register int k;
  register char *name;
/*   register int start_lineno; JF */

  if (lex() != TYPENAME)
    fatal("ill-formed %type declaration");

  k = strlen(token_buffer);
  name = NEW2(k + 1, char);
  strcpy(name, token_buffer);

/*   start_lineno = lineno; */

  for (;;)
    {
      register int t;

      if(ungetc(skip_white_space(), finput) == '%')
	return;

/*       if (lineno != start_lineno)
	return; JF */

      /* we have not passed a newline, so the token now starting is in this declaration */

      t = lex();

      switch (t)
	{

	case COMMA:
	case SEMICOLON:
	  break;

	case IDENTIFIER:
	  if (symval->type_name == NULL)
	    symval->type_name = name;
	  else
	    fatals("type redeclaration for %s", (void*) symval->tag);

	  break;

	default:
	  fatal("invalid %type declaration");
	}
    }
}



/* read in a %left, %right or %nonassoc declaration and record its information.  */
/* assoc is either LEFT_ASSOC, RIGHT_ASSOC or NON_ASSOC.  */

void
parse_assoc_decl (int assoc)
{
  register int k;
  register char *name = NULL;
/*  register int start_lineno; JF */
  register int prev = 0;	/* JF added = 0 to keep lint happy */

  lastprec++;  /* Assign a new precedence level, never 0.  */

/*   start_lineno = lineno; */

  for (;;)
    {
      register int t;

      if(ungetc(skip_white_space(), finput) == '%')
	return;

      /* if (lineno != start_lineno)
	return; JF */

      /* we have not passed a newline, so the token now starting is in this declaration */

      t = lex();

      switch (t)
	{

	case TYPENAME:
	  k = strlen(token_buffer);
	  name = NEW2(k + 1, char);
	  strcpy(name, token_buffer);
	  break;

	case COMMA:
	  break;

	case IDENTIFIER:
	  if (symval->prec != 0)
	    fatals("redefining precedence of %s", (void*) symval->tag);
	  symval->prec = lastprec;
	  symval->assoc = assoc;
	  if (symval->internalClass == SNTERM)
	    fatals("symbol %s redefined", (void*) symval->tag);
	  symval->internalClass = STOKEN;
	  if (name)
	    { /* record the type, if one is specified */
	      if (symval->type_name == NULL)
		symval->type_name = name;
	      else
		fatals("type redeclaration for %s", (void*) symval->tag);
	    }
	  break;

	case NUMBER:
	  if (prev == IDENTIFIER)
            {
	      symval->user_token_number = numval;
	      translations = 1;
            }
          else	  
	    fatal("invalid text in association declaration");
	  break;

	case SEMICOLON:
	  return;

	default:
	  fatal("malformatted association declaration");
	}

      prev = t;

    }
}



/* copy the union declaration into ftable (and fdefines),
   where it is made into the
   definition of YYSTYPE, the type of elements of the parser value stack.  */

void
parse_union_decl()
{
  register int c;
  register int count;
  register int in_comment;
  int cplus_comment;

  if (typed)
    fatal("multiple %union declarations");

  typed = 1;

  if (!nolinesflag)
    fprintf(ftable, "\n#line %d \"%s\"\n", lineno, quoted_filename(infile));
  else
    fprintf(ftable, "\n");
  fprintf(ftable, "typedef union");
  if (definesflag)
   {
    if (!nolinesflag)
     fprintf(fdefines, "\n#line %d \"%s\"\n", lineno, quoted_filename(infile));
    else
     fprintf(fdefines, "\n");
    fprintf(fdefines, "typedef union");
   }


  count = 0;
  in_comment = 0;

  c = getc(finput);

  while (c != EOF)
    {
      hputc(c);
      switch (c)
	{
	case '\n':
	  lineno++;
	  break;

	case '/':
	  c = getc(finput);
	  if (c != '*' && c != '/')
	    ungetc(c, finput);
	  else
	    {
	      hputc(c);
	      cplus_comment = (c == '/');
	      in_comment = 1;
	      c = getc(finput);
	      while (in_comment)
		{
		  hputc(c);
		  if (c == '\n')
		    {
		      lineno++;
		      if (cplus_comment)
			{
			  in_comment = 0;
			  break;
			}
		    }
		  if (c == EOF)
		    fatal("unterminated comment");

		  if (!cplus_comment && c == '*')
		    {
		      c = getc(finput);
		      if (c == '/')
			{
			  hputc('/');
			  in_comment = 0;
			}
		    }
		  else
		    c = getc(finput);
		}
	    }
	  break;


	case '{':
	  count++;
	  break;

	case '}':
	  if (count == 0)
	    fatal ("unmatched close-brace (`}')");
	  count--;
	  if (count == 0)
	    { 
	      set_parser_name(NULL); /* if undef, use default */
	      fprintf(ftable, 
               " yy_%s_stype;\n#define YY_%s_STYPE yy_%s_stype\n",
               parser_name,parser_name,parser_name);
	      if(bison_compability==true)
		{
	      fprintf(ftable, 
		      "#ifndef YY_USE_CLASS\n#define YYSTYPE yy_%s_stype\n#endif\n",parser_name);
		  
		}
	      if (definesflag)
		{
		  fprintf(fdefines,  
			  " yy_%s_stype;\n#define YY_%s_STYPE yy_%s_stype\n",
			  parser_name,parser_name,parser_name);
		  if(bison_compability==true)
		    {
		      fprintf(fdefines, 
			      "#ifndef YY_USE_CLASS\n#define YYSTYPE yy_%s_stype\n#endif\n",parser_name);
		      
		    }
		}
	      /* JF don't choke on trailing semi */
	      c=skip_white_space();
	      if(c!=';') ungetc(c,finput);
	      return;
	    }
	}

      c = getc(finput);
    }
}

/* parse the declaration %expect N which says to expect N
   shift-reduce conflicts.  */

void
parse_expect_decl()
{
  register int c;
  register int count;
  char buffer[20];

  c = getc(finput);
  while (c == ' ' || c == '\t')
    c = getc(finput);

  count = 0;
  while (c >= '0' && c <= '9')
    {
      if (count < 20)
	buffer[count++] = c;
      c = getc(finput);
    }
  buffer[count] = 0;

  ungetc (c, finput);

  expected_conflicts = atoi (buffer);
}

/* that's all of parsing the declaration section */

/* Get the data type (alternative in the union) of the value for symbol n in rule rule.  */

char *
get_type_name(int n, symbol_list* rule)
{
  static char *msg = "invalid $ value";

  register int i;
  register symbol_list *rp;

  if (n < 0)
    fatal(msg);

  rp = rule;
  i = 0;

  while (i < n)
    {
      rp = rp->next;
      if (rp == NULL || rp->sym == NULL)
	fatal(msg);
      i++;
    }

  return (rp->sym->type_name);
}



/* after %guard is seen in the input file,
copy the actual guard into the guards file.
If the guard is followed by an action, copy that into the actions file.
stack_offset is the number of values in the current rule so far,
which says where to find $0 with respect to the top of the stack,
for the simple parser in which the stack is not popped until after the guard is run.  */

void
copy_guard(symbol_list* rule, int stack_offset)
{
  register int c;
  register int n;
  register int count;
  register int match;
  register int ended;
  register char *type_name;
  int brace_flag = 0;
  int cplus_comment;

  /* offset is always 0 if parser has already popped the stack pointer */
  if (semantic_parser) stack_offset = 0;

  fprintf(fguard, "\ncase %d:\n", nrules);
  if (!nolinesflag)
    fprintf(fguard, "#line %d \"%s\"\n", lineno, quoted_filename(infile));
  putc('{', fguard);

  count = 0;
  c = getc(finput);

  while (brace_flag ? (count > 0) : (c != ';'))
    {
      switch (c)
	{
	case '\n':
	  putc(c, fguard);
	  lineno++;
	  break;

	case '{':
	  putc(c, fguard);
	  brace_flag = 1;
	  count++;
	  break;

	case '}':
	  putc(c, fguard);
	  if (count > 0)
	    count--;
	  else
	    fatal("unmatched right brace ('}')");
          break;

	case '\'':
	case '"':
	  match = c;
	  putc(c, fguard);
	  c = getc(finput);

	  while (c != match)
	    {
	      if (c == EOF || c == '\n')
		fatal("unterminated string");

	      putc(c, fguard);
	      
	      if (c == '\\')
		{
		  c = getc(finput);
		  if (c == EOF)
		    fatal("unterminated string");
		  putc(c, fguard);
		  if (c == '\n')
		    lineno++;
		}

	      c = getc(finput);
	    }

	  putc(c, fguard);
	  break;

	case '/':
	  putc(c, fguard);
	  c = getc(finput);
	  if (c != '*' && c != '/')
	    continue;

	  cplus_comment = (c == '/');
	  putc(c, fguard);
	  c = getc(finput);

	  ended = 0;
	  while (!ended)
	    {
	      if (!cplus_comment && c == '*')
		{
		  while (c == '*')
		    {
		      putc(c, fguard);
		      c = getc(finput);
		    }

		  if (c == '/')
		    {
		      putc(c, fguard);
		      ended = 1;
		    }
		}
	      else if (c == '\n')
		{
		  lineno++;
		  putc(c, fguard);
		  if (cplus_comment)
		    ended = 1;
		  else
		    c = getc(finput);
		}
	      else if (c == EOF)
		fatal("unterminated comment");
	      else
		{
		  putc(c, fguard);
		  c = getc(finput);
		}
	    }

	  break;

	case '$':
	  c = getc(finput);
	  type_name = NULL;

	  if (c == '<')
	    {
	      register char *cp = token_buffer;

	      while ((c = getc(finput)) != '>' && c > 0)
		*cp++ = c;
	      *cp = 0;
	      type_name = token_buffer;

	      c = getc(finput);
	    }

	  if (c == '$')
	    {
	      fprintf(fguard, "yyval");
	      if (!type_name) type_name = rule->sym->type_name;
	      if (type_name)
		fprintf(fguard, ".%s", type_name);
	      if(!type_name && typed)	/* JF */
		fprintf(stderr,"%s:%d:  warning:  $$ of '%s' has no declared type.\n",infile,lineno,rule->sym->tag);
	    }

	  else if (isdigit(c) || c == '-')
	    {
	      ungetc (c, finput);
	      n = read_signed_integer(finput);
	      c = getc(finput);

	      if (!type_name && n > 0)
		type_name = get_type_name(n, rule);

	      fprintf(fguard, "yyvsp[%d]", n - stack_offset);
	      if (type_name)
		fprintf(fguard, ".%s", type_name);
	      if(!type_name && typed)	/* JF */
		fprintf(stderr,"%s:%d:  warning:  $%d of '%s' has no declared type.\n",infile,lineno,n,rule->sym->tag);
	      continue;
	    }
	  else
	    fatals("$%c is invalid",(void*) c);	/* JF changed style */

	  break;

	case '@':
	  c = getc(finput);
	  if (isdigit(c) || c == '-')
	    {
	      ungetc (c, finput);
	      n = read_signed_integer(finput);
	      c = getc(finput);
	    }
	  else
	    fatals("@%c is invalid",(void*) c);	/* JF changed style */

	  fprintf(fguard, "yylsp[%d]", n - stack_offset);
	  yylsp_needed = 1;

	  continue;

	case EOF:
	  fatal("unterminated %guard clause");

	default:
	  putc(c, fguard);
	}

      if (c != '}' || count != 0)
	c = getc(finput);
    }

  c = skip_white_space();

  fprintf(fguard, ";\n    break;}");
  if (c == '{')
    copy_action(rule, stack_offset);
  else if (c == '=')
    {
      c = getc(finput);
      if (c == '{')
	copy_action(rule, stack_offset);
    }
  else
    ungetc(c, finput);
}



/* Assuming that a { has just been seen, copy everything up to the matching }
into the actions file.
stack_offset is the number of values in the current rule so far,
which says where to find $0 with respect to the top of the stack.  */

void
copy_action(symbol_list* rule, int stack_offset)
{
  register int c;
  register int n;
  register int count;
  register int match;
  register int ended;
  register char *type_name;
  int cplus_comment;

  /* offset is always 0 if parser has already popped the stack pointer */
  if (semantic_parser) stack_offset = 0;

  fprintf(faction, "\ncase %d:\n", nrules);
  if (!nolinesflag)
    fprintf(faction, "#line %d \"%s\"\n", lineno, quoted_filename(infile));
  putc('{', faction);

  count = 1;
  c = getc(finput);

  while (count > 0)
    {
      while (c != '}')
        {
          switch (c)
	    {
	    case '\n':
	      putc(c, faction);
	      lineno++;
	      break;

	    case '{':
	      putc(c, faction);
	      count++;
	      break;

	    case '\'':
	    case '"':
	      match = c;
	      putc(c, faction);
	      c = getc(finput);

	      while (c != match)
		{
		  if (c == EOF || c == '\n')
		    fatal("unterminated string");

		  putc(c, faction);

		  if (c == '\\')
		    {
		      c = getc(finput);
		      if (c == EOF)
			fatal("unterminated string");
		      putc(c, faction);
		      if (c == '\n')
			lineno++;
		    }

		  c = getc(finput);
		}

	      putc(c, faction);
	      break;

	    case '/':
	      putc(c, faction);
	      c = getc(finput);
	      if (c != '*' && c != '/')
		continue;

	      cplus_comment = (c == '/');
	      putc(c, faction);
	      c = getc(finput);

	      ended = 0;
	      while (!ended)
		{
		  if (!cplus_comment && c == '*')
		    {
		      while (c == '*')
		        {
			  putc(c, faction);
			  c = getc(finput);
			}

		      if (c == '/')
			{
			  putc(c, faction);
			  ended = 1;
			}
		    }
		  else if (c == '\n')
		    {
		      lineno++;
		      putc(c, faction);
		      if (cplus_comment)
			ended = 1;
		      else
		        c = getc(finput);
		    }
		  else if (c == EOF)
		    fatal("unterminated comment");
		  else
		    {
		      putc(c, faction);
		      c = getc(finput);
		    }
		}

	      break;

	    case '$':
	      c = getc(finput);
	      type_name = NULL;

	      if (c == '<')
		{
		  register char *cp = token_buffer;

		  while ((c = getc(finput)) != '>' && c > 0)
		    *cp++ = c;
		  *cp = 0;
		  type_name = token_buffer;
		  value_components_used = 1;

		  c = getc(finput);
		}
	      if (c == '$')
		{
		  fprintf(faction, "yyval");
		  if (!type_name) type_name = get_type_name(0, rule);
		  if (type_name)
		    fprintf(faction, ".%s", type_name);
		  if(!type_name && typed)	/* JF */
		    fprintf(stderr,"%s:%d:  warning:  $$ of '%s' has no declared type.\n",infile,lineno,rule->sym->tag);
		}
	      else if (isdigit(c) || c == '-')
		{
		  ungetc (c, finput);
		  n = read_signed_integer(finput);
		  c = getc(finput);

		  if (!type_name && n > 0)
		    type_name = get_type_name(n, rule);

		  fprintf(faction, "yyvsp[%d]", n - stack_offset);
		  if (type_name)
		    fprintf(faction, ".%s", type_name);
		  if(!type_name && typed)	/* JF */
		    fprintf(stderr,"%s:%d:  warning:  $%d of '%s' has no declared type.\n",infile,lineno,n,rule->sym->tag);
		  continue;
		}
	      else
		fatals("$%c is invalid",(void*) c);	/* JF changed format */

	      break;

	    case '@':
	      c = getc(finput);
	      if (isdigit(c) || c == '-')
		{
		  ungetc (c, finput);
		  n = read_signed_integer(finput);
		  c = getc(finput);
		}
	      else
		fatal("invalid @-construct");

	      fprintf(faction, "yylsp[%d]", n - stack_offset);
	      yylsp_needed = 1;

	      continue;

	    case EOF:
	      fatal("unmatched '{'");

	    default:
	      putc(c, faction);
	    }

          c = getc(finput);
        }

      /* above loop exits when c is '}' */

      if (--count)
        {
	  putc(c, faction);
	  c = getc(finput);
	}
    }

  fprintf(faction, ";\n    break;}");
}



/* generate a dummy symbol, a nonterminal,
whose name cannot conflict with the user's names. */

bucket *
gensym()
{
  register bucket *sym;

  sprintf (token_buffer, "@%d", ++gensym_count);
  sym = getsym(token_buffer);
  sym->internalClass = SNTERM;
  sym->value = nvars++;
  return (sym);
}

/* Parse the input grammar into a one symbol_list structure.
Each rule is represented by a sequence of symbols: the left hand side
followed by the contents of the right hand side, followed by a null pointer
instead of a symbol to terminate the rule.
The next symbol is the lhs of the following rule.

All guards and actions are copied out to the appropriate files,
labelled by the rule number they apply to.  */

void
readgram()
{
  register int t;
  register bucket *lhs;
  register symbol_list *p;
  register symbol_list *p1;
  register bucket *bp;

  symbol_list *crule;	/* points to first symbol_list of current rule.  */
			/* its symbol is the lhs of the rule.   */
  symbol_list *crule1;  /* points to the symbol_list preceding crule.  */

  p1 = NULL;

  t = lex();

  while (t != TWO_PERCENTS && t != ENDFILE)
    {
      if (t == IDENTIFIER || t == BAR)
	{
	  register int actionflag = 0;
	  int rulelength = 0;  /* number of symbols in rhs of this rule so far  */
	  int xactions = 0;	/* JF for error checking */
	  bucket *first_rhs = 0;

	  if (t == IDENTIFIER)
	    {
	      lhs = symval;
    
	      t = lex();
	      if (t != COLON)
		fatal("ill-formed rule");
	    }

	  if (nrules == 0)
	    {
	      if (t == BAR)
		fatal("grammar starts with vertical bar");

	      if (!start_flag)
		startval = lhs;
	    }

	  /* start a new rule and record its lhs.  */

	  nrules++;
	  nitems++;

	  record_rule_line ();

	  p = NEW(symbol_list);
	  p->sym = lhs;

	  crule1 = p1;
	  if (p1)
	    p1->next = p;
	  else
	    grammar = p;

	  p1 = p;
	  crule = p;

	  /* mark the rule's lhs as a nonterminal if not already so.  */

	  if (lhs->internalClass == SUNKNOWN)
	    {
	      lhs->internalClass = SNTERM;
	      lhs->value = nvars;
	      nvars++;
	    }
	  else if (lhs->internalClass == STOKEN)
	    fatals("rule given for %s, which is a token", (void*)  lhs->tag);

	  /* read the rhs of the rule.  */

	  for (;;)
	    {
	      t = lex();

	      if (! (t == IDENTIFIER || t == LEFT_CURLY)) break;

	      /* If next token is an identifier, see if a colon follows it.
		 If one does, exit this rule now.  */
	      if (t == IDENTIFIER)
		{
		  register bucket *ssave;
		  register int t1;

		  ssave = symval;
		  t1 = lex();
		  unlex(t1);
		  symval = ssave;
		  if (t1 == COLON) break;

		  if(!first_rhs)	/* JF */
		    first_rhs = symval;
		  /* Not followed by colon =>
		     process as part of this rule's rhs.  */
		}

	      /* If we just passed an action, that action was in the middle
		 of a rule, so make a dummy rule to reduce it to a
		 non-terminal.  */
	      if (actionflag)
		{
		  register bucket *sdummy;

		  /* Since the action was written out with this rule's */
		  /* number, we must write give the new rule this number */
		  /* by inserting the new rule before it.  */

		  /* Make a dummy nonterminal, a gensym.  */
		  sdummy = gensym();

		  /* Make a new rule, whose body is empty,
		     before the current one, so that the action
		     just read can belong to it.  */
		  nrules++;
		  nitems++;
		  record_rule_line ();
		  p = NEW(symbol_list);
		  if (crule1)
		    crule1->next = p;
		  else grammar = p;
		  p->sym = sdummy;
		  crule1 = NEW(symbol_list);
		  p->next = crule1;
		  crule1->next = crule;

		  /* insert the dummy generated by that rule into this rule.  */
		  nitems++;
		  p = NEW(symbol_list);
		  p->sym = sdummy;
		  p1->next = p;
		  p1 = p;

		  actionflag = 0;
		}

	      if (t == IDENTIFIER)
		{
		  nitems++;
		  p = NEW(symbol_list);
		  p->sym = symval;
		  p1->next = p;
		  p1 = p;
		}
	      else /* handle an action.  */
		{
		  copy_action(crule, rulelength);
		  actionflag = 1;
		  xactions++;	/* JF */
		}
	      rulelength++;
	    }

	  /* Put an empty link in the list to mark the end of this rule  */
	  p = NEW(symbol_list);
	  p1->next = p;
	  p1 = p;

	  if (t == PREC)
	    {
	      t = lex();
	      crule->ruleprec = symval;
	      t = lex();
	    }
	  if (t == GUARD)
	    {
	      if (! semantic_parser)
		fatal("%guard present but %semantic_parser not specified");

	      copy_guard(crule, rulelength);
	      t = lex();
	    }
	  else if (t == LEFT_CURLY)
	    {
	      if (actionflag) fatal("two actions at end of one rule");
	      copy_action(crule, rulelength);
	      t = lex();
	    }
	  /* If $$ is being set in default way,
	     warn if any type mismatch.  */
	  else if (!xactions && first_rhs && lhs->type_name != first_rhs->type_name)
	    {
	      if (lhs->type_name == 0 || first_rhs->type_name == 0
		  || strcmp(lhs->type_name,first_rhs->type_name))
		fprintf(stderr, "%s:%d:  warning:  type clash ('%s' '%s') on default action\n",
			infile,
			lineno,
			lhs->type_name ? lhs->type_name : "",
			first_rhs->type_name ? first_rhs->type_name : "");
	    }
	  /* Warn if there is no default for $$ but we need one.  */
	  else if (!xactions && !first_rhs && lhs->type_name != 0)
	    fprintf(stderr,
		    "%s:%d:  warning: empty rule for typed nonterminal, and no action\n",
		    infile,
		    lineno);
	  if (t == SEMICOLON)
	    t = lex();
	}
      /* these things can appear as alternatives to rules.  */
      else if (t == TOKEN)
	{
	  parse_token_decl(STOKEN, SNTERM);
	  t = lex();
	}
      else if (t == NTERM)
	{
	  parse_token_decl(SNTERM, STOKEN);
	  t = lex();
	}
      else if (t == TYPE)
	{
	  t = get_type();
	}
      else if (t == UNION)
	{
	  parse_union_decl();
	  t = lex();
	}
      else if (t == EXPECT)
	{
	  parse_expect_decl();
	  t = lex();
	}
      else if (t == START)
	{
	  parse_start_decl();
	  t = lex();
	}
      else
	fatal("invalid input");
    }
  set_parser_name(NULL); /* if undef, use default */

  if (nsyms > MAXSHORT)
    fatals("too many symbols (tokens plus nonterminals); maximum %d",
	  (void*)  MAXSHORT);
  if (nrules == 0)
    fatal("no input grammar");

  if (typed == 0	/* JF put out same default YYSTYPE as YACC does */
      && !value_components_used)
    {
      /* We used to use `unsigned long' as YYSTYPE on MSDOS,
         but it seems better to be consistent.
         Most programs should declare their own type anyway.  */
      fprintf(ftable, "\
#ifndef YY_USE_CLASS\n\
# ifndef YYSTYPE\n\
#  define YYSTYPE int\n\
#  define YYSTYPE_IS_TRIVIAL 1\n\
# endif\n\
#endif\n");
      if (definesflag)
	fprintf(fdefines, "\
\
#ifndef YY_USE_CLASS\n\
# ifndef YYSTYPE\n\
#  define YYSTYPE int\n\
#  define YYSTYPE_IS_TRIVIAL 1\n\
# endif\n\
#endif\n");
    }

  /* Report any undefined symbols and consider them nonterminals.  */

  for (bp = firstsymbol; bp; bp = bp->next)
    if (bp->internalClass == SUNKNOWN)
      {
	fprintf(stderr, "symbol %s used, not defined as token, and no rules for it\n",
			bp->tag);
	failure = 1;
	bp->internalClass = SNTERM;
	bp->value = nvars++;
      }

  ntokens = nsyms - nvars;
}


void
record_rule_line ()
{
  /* Record each rule's source line number in rline table.  */

  if (nrules >= rline_allocated)
    {
      rline_allocated = nrules * 2;
      rline = (short *) xrealloc (( char*) rline,
				 rline_allocated * sizeof (short));
    }
  rline[nrules] = lineno;
}


/* read in a %type declaration and record its information for get_type_name to access */

int
get_type()
{
  register int k;
  register int t;
  register char *name;

  t = lex();

  if (t != TYPENAME)
    fatal("ill-formed %type declaration");

  k = strlen(token_buffer);
  name = NEW2(k + 1, char);
  strcpy(name, token_buffer);

  for (;;)
    {
      t = lex();

      switch (t)
	{
	case SEMICOLON:
	  return (lex());

	case COMMA:
	  break;

	case IDENTIFIER:
	  if (symval->type_name == NULL)
	    symval->type_name = name;
	  else
	    fatals("type redeclaration for %s", (void*) symval->tag);

	  break;

	default:
	  return (t);
	}
    }
}



/* assign symbol numbers, and write definition of token names into fdefines.
Set up vectors tags and sprec of names and precedences of symbols.  */

void
packsymbols()
{
  register bucket *bp;
  register int tokno = 1;
  register int i;
  register int last_user_token_number;

  /* int lossage = 0; JF set but not used */

  tags = NEW2(nsyms + 1, char *);
  tags[0] = "$";

  sprec = NEW2(nsyms, short);
  sassoc = NEW2(nsyms, short);

  max_user_token_number = 256;
  last_user_token_number = 256;

  for (bp = firstsymbol; bp; bp = bp->next)
    {
      if (bp->internalClass == SNTERM)
	{
	  bp->value += ntokens;
	}
      else
	{
	  if (translations && !(bp->user_token_number))
	    bp->user_token_number = ++last_user_token_number;
	  if (bp->user_token_number > max_user_token_number)
	    max_user_token_number = bp->user_token_number;
	  bp->value = tokno++;
	}

      tags[bp->value] = bp->tag;
      sprec[bp->value] = bp->prec;
      sassoc[bp->value] = bp->assoc;

    }

  if (translations)
    {
      register int i;

      token_translations = NEW2(max_user_token_number+1, short);

      /* initialize all entries for literal tokens to 2,
	 the internal token number for $illegal., which represents all invalid inputs.  */
      for (i = 0; i <= max_user_token_number; i++)
        token_translations[i] = 2;      
    }

  for (bp = firstsymbol; bp; bp = bp->next)
    {
      if (bp->value >= ntokens) continue;
      if (translations)
	{
	  if (token_translations[bp->user_token_number] != 2)
	    {
	    	/* JF made this a call to fatals() */
	      fatals( "tokens %s and %s both assigned number %d",
			      (void*) tags[token_translations[bp->user_token_number]],
			      (void*) bp->tag,
			      (void*) bp->user_token_number);
	    }
	  token_translations[bp->user_token_number] = bp->value;
	}
    }

  error_token_number = errtoken->value;

  
  if (startval->internalClass == SUNKNOWN)
    fatals("the start symbol %s is undefined", (void*) startval->tag);
  else if (startval->internalClass == STOKEN)
    fatals("the start symbol %s is a token", (void*) startval->tag);

  start_symbol = startval->value;
  output_about_token();
}
      



/* convert the rules into the representation using rrhs, rlhs and ritems.  */

void
packgram()
{
  register int itemno;
  register int ruleno;
  register symbol_list *p;
/*  register bucket *bp; JF unused */

  bucket *ruleprec;

  ritem = NEW2(nitems + 1, short);
  rlhs = NEW2(nrules, short) - 1;
  rrhs = NEW2(nrules, short) - 1;
  rprec = NEW2(nrules, short) - 1;
  rprecsym = NEW2(nrules, short) - 1;
  rassoc = NEW2(nrules, short) - 1;

  itemno = 0;
  ruleno = 1;

  p = grammar;
  while (p)
    {
      rlhs[ruleno] = p->sym->value;
      rrhs[ruleno] = itemno;
      ruleprec = p->ruleprec;

      p = p->next;
      while (p && p->sym)
	{
	  ritem[itemno++] = p->sym->value;
	  /* A rule gets by default the precedence and associativity
	     of the last token in it.  */
          if (p->sym->internalClass == STOKEN)
	    {
	      rprec[ruleno] = p->sym->prec;
	      rassoc[ruleno] = p->sym->assoc;
	    }
	  if (p) p = p->next;
	}

      /* If this rule has a %prec,
	 the specified symbol's precedence replaces the default.  */
      if (ruleprec)
	{
          rprec[ruleno] = ruleprec->prec;
          rassoc[ruleno] = ruleprec->assoc;
	  rprecsym[ruleno] = ruleprec->value;
	}

      ritem[itemno++] = -ruleno;
      ruleno++;

      if (p) p = p->next;
    }

  ritem[itemno] = 0;
}
/* Read a signed integer from STREAM and return its value.  */

int
read_signed_integer (FILE* stream)
{
  register int c = getc(stream);
  register int sign = 1;
  register int n;

  if (c == '-')
    {
      c = getc(stream);
      sign = -1;
    }
  n = 0;
  while (isdigit(c))
    {
      n = 10*n + (c - '0');
      c = getc(stream);
    }

  ungetc(c, stream);

  return n * sign;
}

void set_parser_name(char* n)
{
 if(n) /* define */
   {if(parser_defined) /* redef */
      fatals("parser name already defined as \"%s\"and redefined to \"%s\"\n",(void*) parser_name,(void*) n);
    else
      parser_defined++;
      parser_name=(char *)xmalloc(strlen(n)+1);
      strcpy(parser_name,n);
   }
 else /* use only */
   {
     if(!parser_defined) /* first use, default */
      {parser_defined++;
       fprintf(stderr,"%s:%d parser name defined to default :\"%s\"\n"
         ,infile,lineno,parser_name);
      }
     else /* next use ok*/;
   }
}

void read_a_name(char* buf,int len)
{int c,l;
 for(c = skip_white_space(),l=0;
     (isalnum(c) || c == '_');
     c=getc(finput),l++) 
      if(l<len) buf[l]=c;
 if(l>=len)
   {buf[len-1]=0;
    fprintf(stderr,"%s:%d name too long, truncated to :\"%s\"\n"
         ,infile,lineno,buf);
   }
 else
  buf[l]=0;    
     
 ungetc(c, finput);

}  ;  

void
parse_name_declaration()
{char name[65];
 read_a_name(name,sizeof(name));
 set_parser_name(name);
}
void
parse_define()
{char name[65];
 int c;
 int after_backslash;
 read_a_name(name,sizeof(name));
 set_parser_name(NULL);
 fprintf(ftable,"#define YY_%s_%s ",parser_name,name);
 if (definesflag)
   fprintf(fdefines,"#define YY_%s_%s ",parser_name,name);
 for(after_backslash=0,c=getc(finput);
     (after_backslash || c!='\n');
     c=getc(finput))
  {after_backslash=(c=='\\');
   if(c=='\n') lineno++;
   if(c==EOF)
    {fatal("unexpected EOF in %define");}
   hputc(c);
  }
  hputc('\n');lineno++;
}
