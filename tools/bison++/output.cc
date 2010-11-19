/* Output the generated parsing program for bison,
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


/* functions to output parsing data to various files.  Entries are:

  output_headers ()

Output constant strings to the beginning of certain files.

  output_trailers()

Output constant strings to the ends of certain files.

  output ()

Output the parsing tables and the parser code to ftable.

The parser tables consist of these tables.
Starred ones needed only for the semantic parser.

yytranslate = vector mapping yylex's token numbers into bison's token numbers.

yytname = vector of string-names indexed by bison token number

yyrline = vector of line-numbers of all rules.  For yydebug printouts.

yyrhs = vector of items of all rules.
        This is exactly what ritems contains.  For yydebug and for semantic
	parser.

yyprhs[r] = index in yyrhs of first item for rule r.

yyr1[r] = symbol number of symbol that rule r derives.

yyr2[r] = number of symbols composing right hand side of rule r.

* yystos[s] = the symbol number of the symbol that leads to state s.

yydefact[s] = default rule to reduce with in state s,
	      when yytable doesn't specify something else to do.
	      Zero means the default is an error.

yydefgoto[i] = default state to go to after a reduction of a rule that
	       generates variable ntokens + i, except when yytable
	       specifies something else to do.

yypact[s] = index in yytable of the portion describing state s.
            The lookahead token's type is used to index that portion
            to find out what to do.

	    If the value in yytable is positive,
	    we shift the token and go to that state.

	    If the value is negative, it is minus a rule number to reduce by.

	    If the value is zero, the default action from yydefact[s] is used.

yypgoto[i] = the index in yytable of the portion describing 
             what to do after reducing a rule that derives variable i + ntokens.
             This portion is indexed by the parser state number
	     as of before the text for this nonterminal was read.
	     The value from yytable is the state to go to.

yytable = a vector filled with portions for different uses,
          found via yypact and yypgoto.

yycheck = a vector indexed in parallel with yytable.
	  It indicates, in a roundabout way, the bounds of the
	  portion you are trying to examine.

	  Suppose that the portion of yytable starts at index p
	  and the index to be examined within the portion is i.
	  Then if yycheck[p+i] != i, i is outside the bounds
	  of what is actually allocated, and the default
	  (from yydefact or yydefgoto) should be used.
	  Otherwise, yytable[p+i] should be used.

YYFINAL = the state number of the termination state.
YYFLAG = most negative short int.  Used to flag ??
YYNTBASE = ntokens.

*/

#include <stdio.h>
#include "system.h"
#include "machine.h"
#include "new.h"
#include "files.h"
#include "gram.h"
#include "state.h"
#include "symtab.h"


extern bool bison_compability;
extern int debugflag;
extern int nolinesflag;
extern int definesflag;
char *quoted_filename(char* f);
extern char **tags;
extern int tokensetsize;
extern int final_state;
extern core **state_table;
extern shifts **shift_table;
extern errs **err_table;
extern reductions **reduction_table;
extern short *accessing_symbol;
extern unsigned *LA;
extern short *LAruleno;
extern short *lookaheads;
extern char *consistent;
extern short *goto_map;
extern short *from_state;
extern short *to_state;

extern char* xmalloc(unsigned);


void output_token_translations();
void output_gram();
void output_stos();
void output_rule_data();
void output_defines();
void output_actions();
void token_actions();
void save_row(int);
void goto_actions();
void save_column(int,int);
void sort_actions();
void pack_table();
void output_base();
void output_table();
void output_check();
void output_parser();
void output_program();
void free_itemset();
void free_shifts();
void free_reductions();
void free_itemsets();
int action_row(int);
int default_goto(int);
int matching_state(int);
int pack_vector(int);

extern void berror(char*);
extern void fatal(const char*);
extern void fatals(const char*,void*);
extern void fatals(const char*,void*,void*);
extern void fatals(const char*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*);
extern void fatals(const char*,void*,void*,void*,void*,void*);

static int nvectors;
static int nentries;
static short **froms;
static short **tos;
static short *tally;
static short *width;
static short *actrow;
static short *state_count;
static short *order;
static short *base;
static short *pos;
static short *table;
static short *check;
static int lowzero;
static int high;

void output_section(FILE* fin,FILE* fout);
void output_token_defines_fmt(FILE* file,char* fmt,int notrans);
extern bucket *errtoken;
void output_token_defines(FILE* file);
void output_token_const_def(FILE* file);
void output_token_const_decl(FILE* file);
void output_token_enum(FILE* file);
extern int line_fparser;
extern int line_fhskel;
extern char *parser_fname;
extern char *hskel_fname;
extern char *version_string;




#define	GUARDSTR	"\n#include \"%s\"\nextern int yyerror;\n\
extern int yycost;\nextern char * yymsg;\nextern YYSTYPE yyval;\n\n\
yyguard(n, yyvsp, yylsp)\nregister int n;\nregister YYSTYPE *yyvsp;\n\
register YYLTYPE *yylsp;\n\
{\n  yyerror = 0;\nyycost = 0;\n  yymsg = 0;\nswitch (n)\n    {"

#define	ACTSTR		"\n#include \"%s\"\nextern YYSTYPE yyval;\
\nextern int yychar;\
yyaction(n, yyvsp, yylsp)\nregister int n;\nregister YYSTYPE *yyvsp;\n\
register YYLTYPE *yylsp;\n{\n  switch (n)\n{"

#define	ACTSTR_SIMPLE	"\n  switch (yyn) {\n"

void
output_before_read()
{ 
  fprintf(ftable, "\n/*  A Bison++ parser, made from %s  */\n\n", infile);
  fprintf(ftable, " /* with Bison++ version %s  */\n\n", version_string);
  /* Redefine certain symbols if -p was specified.  */
  if (spec_name_prefix)
    {
      fprintf(ftable, "#define yyparse %sparse\n",spec_name_prefix);
      fprintf(ftable, "#define yylex %slex\n",spec_name_prefix);
      fprintf(ftable, "#define yyerror %serror\n",spec_name_prefix);
      fprintf(ftable, "#define yylval %slval\n",spec_name_prefix);
      fprintf(ftable, "#define yychar %schar\n",spec_name_prefix);
      fprintf(ftable, "#define yydebug %sdebug\n",spec_name_prefix);
    }
  if(bison_compability==false)    
    fprintf(ftable,"#define YY_USE_CLASS\n");
  output_section(fparser,ftable);
  if(definesflag) output_section(fhskel,fdefines);
};

void
output_headers()
{
  if(bison_compability==false)
    {
    fprintf(fbisoncomp,"#define YY_USE_CLASS\n");
    }
  else
    {
      fprintf(fbisoncomp,"/*#define YY_USE_CLASS \n*/");
    }
  output_section(fhskel,fbisoncomp);
  if(definesflag) output_section(fhskel,fdefines);
  output_section(fparser,ftable);
  if (pure_parser)
    {
     fprintf(ftable, "#define YY_%s_PURE 1\n",parser_name);
     if(definesflag) fprintf(fdefines, "#define YY_%s_PURE 1\n\n",parser_name);
    }
  /* start writing the guard and action files, if they are needed.  */
  if (semantic_parser)
    fprintf(fguard, GUARDSTR, attrsfile);
  fprintf(faction, (semantic_parser ? ACTSTR : ACTSTR_SIMPLE), attrsfile);
  if(definesflag) output_section(fhskel,fdefines);
  output_section(fparser,ftable);


  /* Rename certain symbols if -p was specified.  */
  if (spec_name_prefix)
    {
      fprintf(ftable, "#define YY_%s_PARSE %sparse\n",
          parser_name, spec_name_prefix);
      fprintf(ftable, "#define YY_%s_LEX %slex\n",
          parser_name, spec_name_prefix);
      fprintf(ftable, "#define YY_%s_ERROR %serror\n",
          parser_name, spec_name_prefix);
      fprintf(ftable, "#define YY_%s_LVAL %slval\n",
          parser_name, spec_name_prefix);
      fprintf(ftable, "#define YY_%s_CHAR %schar\n",
          parser_name, spec_name_prefix);
      fprintf(ftable, "#define YY_%s_DEBUG %sdebug\n",
          parser_name, spec_name_prefix);
    }
  if (spec_name_prefix && definesflag)
    {
      fprintf(fdefines, "#define YY_%s_PARSE %sparse\n",
          parser_name, spec_name_prefix);
      fprintf(fdefines, "#define YY_%s_LEX %slex\n",
          parser_name, spec_name_prefix);
      fprintf(fdefines, "#define YY_%s_ERROR %serror\n",
          parser_name, spec_name_prefix);
      fprintf(fdefines, "#define YY_%s_LVAL %slval\n",
          parser_name, spec_name_prefix);
      fprintf(fdefines, "#define YY_%s_CHAR %schar\n",
          parser_name, spec_name_prefix);
      fprintf(fdefines, "#define YY_%s_DEBUG %sdebug\n",
          parser_name, spec_name_prefix);
    }

}


void
output_trailers()
{
  if(definesflag) output_section(fhskel,fdefines);
  output_section(fparser,ftable);
  /* output the definition of YYLTYPE into the fattrs and fdefines files.  */
  if(debugflag)
   {fprintf(ftable, 
    "#define YY_%s_DEBUG %d\n"
	    ,parser_name,!!debugflag);
    if (definesflag)
     fprintf(fdefines, 
      "#define YY_%s_DEBUG %d\n",
            parser_name,!!debugflag);
    }
  if(definesflag) output_section(fhskel,fdefines);
  output_section(fparser,ftable);
  /* Now we know whether we need the line-number stack.
     If we do, write its type into the .tab.h file.  */
  if (yylsp_needed)
    {
     /* fattrs winds up in the .tab.c file, before bison.simple.  */
      fprintf(ftable, "#define YYLSP_%s_NEEDED\n",parser_name);
      if (debugflag)

      if (definesflag)
	{
         fprintf(fdefines, 
             "#define YY_%s_LSP_NEEDED\n",
              parser_name);
        }
    }
  if (semantic_parser)
    {
      fprintf(fguard, "\n    }\n}\n");
      fprintf(faction, "\n    }\n}\n");
    }
  else
    fprintf(faction, "\n}\n");
}


void
output()
{
  int c;

   if (!semantic_parser)		/* JF Put out other stuff */
    {
      rewind(fattrs);
      while ((c=getc(fattrs))!=EOF)
        putc(c,ftable);
    }

  if (semantic_parser)
    fprintf(ftable, "#include \"%s\"\n", attrsfile);


  free_itemsets();
  output_defines();
  output_token_translations();
/*   if (semantic_parser) */
  /* This is now unconditional because debugging printouts can use it.  */
  output_gram();
  FREE(ritem);
  if (semantic_parser)
    output_stos();
  output_rule_data();
  output_actions();
  output_parser();
  output_program();
}


void
output_token_translations()
{
  register int i, j;
/*   register short *sp; JF unused */

  if (translations)
    {
      fprintf(ftable,
	      "\n#define YYTRANSLATE(x) ((unsigned)(x) <= %d ? yytranslate[x] : %d)\n",
	      max_user_token_number, nsyms);
    
      if (ntokens < 127)  /* play it very safe; check maximum element value.  */
        fprintf(ftable, "\nstatic const char yytranslate[] = {     0");
      else
	fprintf(ftable, "\nstatic const short yytranslate[] = {     0");
    
      j = 10;
      for (i = 1; i <= max_user_token_number; i++)
	{
	  putc(',', ftable);
    
	  if (j >= 10)
	    {
	      putc('\n', ftable);
	      j = 1;
	    }
	  else
	    {
	      j++;
	    }
    
	  fprintf(ftable, "%6d", token_translations[i]);
	}
    
      fprintf(ftable, "\n};\n");
    }
  else
    {
      fprintf(ftable, "\n#define YYTRANSLATE(x) (x)\n");
    } 
}


void
output_gram()
{
  register int i;
  register int j;
  register short *sp;

  /* With the ordinary parser,
     yyprhs and yyrhs are needed only for yydebug.  */
  if (!semantic_parser)
    fprintf(ftable, "\n#if YY_%s_DEBUG != 0",parser_name);

  fprintf(ftable, "\nstatic const short yyprhs[] = {     0");

  j = 10;
  for (i = 1; i <= nrules; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", rrhs[i]);
    }

  fprintf(ftable, "\n};\n");

  fprintf(ftable, "\nstatic const short yyrhs[] = {%6d", ritem[0]);

  j = 10;
  for (sp = ritem + 1; *sp; sp++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      if (*sp > 0)
	fprintf(ftable, "%6d", *sp);
      else
	fprintf(ftable, "     0");
    }

  fprintf(ftable, "\n};\n");
  if(!semantic_parser)
    fprintf(ftable, "\n#endif\n");

}


void
output_stos()
{
  register int i;
  register int j;

  fprintf(ftable, "\nstatic const short yystos[] = {     0");

  j = 10;
  for (i = 1; i < nstates; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", accessing_symbol[i]);
    }

  fprintf(ftable, "\n};\n");
}


void
output_rule_data()
{
  register int i;
  register int j;

  fprintf(ftable, 
     "\n#if (YY_%s_DEBUG != 0) || defined(YY_%s_ERROR_VERBOSE) \nstatic const short yyrline[] = { 0",parser_name,parser_name);

  j = 10;
  for (i = 1; i <= nrules; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", rline[i]);
    }

  /* Output the table of symbol names.  */

  fprintf(ftable,
          "\n};\n\nstatic const char * const yytname[] = {   \"%s\"",
          tags[0]);

  j = strlen (tags[0]) + 44;
  for (i = 1; i <= nsyms; i++)
    {
      register char *p;
      putc(',', ftable);
      j++;

      if (j > 75)
	{
	  putc('\n', ftable);
	  j = 0;
	}

      putc ('\"', ftable);
      j++;

      for (p = tags[i]; p && *p; p++)
	{
	  if (*p == '"' || *p == '\\')
	    {
	      fprintf(ftable, "\\%c", *p);
	      j += 2;
	    }
	  else if (*p == '\n')
	    {
	      fprintf(ftable, "\\n");
	      j += 2;
	    }
	  else if (*p == '\t')
	    {
	      fprintf(ftable, "\\t");
	      j += 2;
	    }
	  else if (*p == '\b')
	    {
	      fprintf(ftable, "\\b");
	      j += 2;
	    }
	  else if (*p < 040 || *p >= 0177)
	    {
	      fprintf(ftable, "\\%03o", *p);
	      j += 4;
	    }
	  else
	    {
	      putc(*p, ftable);
	      j++;
	    }
	}

      putc ('\"', ftable);
      j++;
    }

  fprintf(ftable, "\n};\n#endif\n\nstatic const short yyr1[] = {     0");

  j = 10;
  for (i = 1; i <= nrules; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", rlhs[i]);
    }

  FREE(rlhs + 1);

  fprintf(ftable, "\n};\n\nstatic const short yyr2[] = {     0");

  j = 10;
  for (i = 1; i < nrules; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", rrhs[i + 1] - rrhs[i] - 1);
    }

  putc(',', ftable);
  if (j >= 10)
    putc('\n', ftable);

  fprintf(ftable, "%6d\n};\n", nitems - rrhs[nrules] - 1);
  FREE(rrhs + 1);
}


void
output_defines()
{
  fprintf(ftable, "\n\n#define\tYYFINAL\t\t%d\n", final_state);
  fprintf(ftable, "#define\tYYFLAG\t\t%d\n", MINSHORT);
  fprintf(ftable, "#define\tYYNTBASE\t%d\n", ntokens);
}



/* compute and output yydefact, yydefgoto, yypact, yypgoto, yytable and yycheck.  */

void
output_actions()
{
  nvectors = nstates + nvars;

  froms = NEW2(nvectors, short *);
  tos = NEW2(nvectors, short *);
  tally = NEW2(nvectors, short);
  width = NEW2(nvectors, short);

  token_actions();
  free_shifts();
  free_reductions();
  FREE(lookaheads);
  FREE(LA);
  FREE(LAruleno);
  FREE(accessing_symbol);

  goto_actions();
  FREE(goto_map + ntokens);
  FREE(from_state);
  FREE(to_state);

  sort_actions();
  pack_table();
  output_base();
  output_table();
  output_check();
}



/* figure out the actions for the specified state, indexed by lookahead token type.

   The yydefact table is output now.  The detailed info
   is saved for putting into yytable later.  */

void
token_actions()
{
  register int i;
  register int j;
  register int k;

  actrow = NEW2(ntokens, short);

  k = action_row(0);
  fprintf(ftable, "\nstatic const short yydefact[] = {%6d", k);
  save_row(0);

  j = 10;
  for (i = 1; i < nstates; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      k = action_row(i);
      fprintf(ftable, "%6d", k);
      save_row(i);
    }

  fprintf(ftable, "\n};\n");
  FREE(actrow);
}



/* Decide what to do for each type of token if seen as the lookahead token in specified state.
   The value returned is used as the default action (yydefact) for the state.
   In addition, actrow is filled with what to do for each kind of token,
   index by symbol number, with zero meaning do the default action.
   The value MINSHORT, a very negative number, means this situation
   is an error.  The parser recognizes this value specially.

   This is where conflicts are resolved.  The loop over lookahead rules
   considered lower-numbered rules last, and the last rule considered that likes
   a token gets to handle it.  */

int
action_row(int state)
{
  register int i;
  register int j;
  register int k;
  register int m;
  register int n;
  register int count;
  register int default_rule;
  register int nreds;
  register int max;
  register int rule;
  register int shift_state;
  register int symbol;
  register unsigned mask;
  register unsigned *wordp;
  register reductions *redp;
  register shifts *shiftp;
  register errs *errp;
  int nodefault = 0;  /* set nonzero to inhibit having any default reduction */

  for (i = 0; i < ntokens; i++)
    actrow[i] = 0;

  default_rule = 0;
  nreds = 0;
  redp = reduction_table[state];

  if (redp)
    {
      nreds = redp->nreds;

      if (nreds >= 1)
	{
	  /* loop over all the rules available here which require lookahead */
	  m = lookaheads[state];
	  n = lookaheads[state + 1];

	  for (i = n - 1; i >= m; i--)
	    {
	      rule = - LAruleno[i];
	      wordp = LA + i * tokensetsize;
	      mask = 1;

	      /* and find each token which the rule finds acceptable to come next */
	      for (j = 0; j < ntokens; j++)
		{
		  /* and record this rule as the rule to use if that token follows.  */
		  if (mask & *wordp)
		    actrow[j] = rule;

		  mask <<= 1;
		  if (mask == 0)
		    {
		      mask = 1;
		      wordp++;
		    }
		}
	    }
	}
    }

  shiftp = shift_table[state];

  /* now see which tokens are allowed for shifts in this state.
     For them, record the shift as the thing to do.  So shift is preferred to reduce.  */

  if (shiftp)
    {
      k = shiftp->nshifts;

      for (i = 0; i < k; i++)
	{
	  shift_state = shiftp->internalShifts[i];
	  if (! shift_state) continue;

	  symbol = accessing_symbol[shift_state];

	  if (ISVAR(symbol))
	    break;

	  actrow[symbol] = shift_state;

	  /* do not use any default reduction if there is a shift for error */

	  if (symbol == error_token_number) nodefault = 1;
	}
    }

  errp = err_table[state];

  /* See which tokens are an explicit error in this state
     (due to %nonassoc).  For them, record MINSHORT as the action.  */

  if (errp)
    {
      k = errp->nerrs;

      for (i = 0; i < k; i++)
	{
	  symbol = errp->internalErrs[i];
	  actrow[symbol] = MINSHORT;
	}
    }

  /* now find the most common reduction and make it the default action for this state.  */

  if (nreds >= 1 && ! nodefault)
    {
      if (consistent[state])
	default_rule = redp->rules[0];
      else
	{
	  max = 0;
	  for (i = m; i < n; i++)
	    {
	      count = 0;
	      rule = - LAruleno[i];
    
	      for (j = 0; j < ntokens; j++)
		{
		  if (actrow[j] == rule)
		    count++;
		}
    
	      if (count > max)
		{
		  max = count;
		  default_rule = rule;
		}
	    }
    
	  /* actions which match the default are replaced with zero,
	     which means "use the default" */
    
	  if (max > 0)
	    {
	      for (j = 0; j < ntokens; j++)
		{
		  if (actrow[j] == default_rule)
		    actrow[j] = 0;
		}
    
	      default_rule = - default_rule;
	    }
	}
    }

  /* If have no default rule, the default is an error.
     So replace any action which says "error" with "use default".  */

  if (default_rule == 0)
    for (j = 0; j < ntokens; j++)
      {
	if (actrow[j] == MINSHORT)
	  actrow[j] = 0;
      }

  return (default_rule);
}


void
save_row(int state)
{
  register int i;
  register int count;
  register short *sp;
  register short *sp1;
  register short *sp2;

  count = 0;
  for (i = 0; i < ntokens; i++)
    {
      if (actrow[i] != 0)
	count++;
    }

  if (count == 0)
    return;

  froms[state] = sp1 = sp = NEW2(count, short);
  tos[state] = sp2 = NEW2(count, short);

  for (i = 0; i < ntokens; i++)
    {
      if (actrow[i] != 0)
	{
	  *sp1++ = i;
	  *sp2++ = actrow[i];
	}
    }

  tally[state] = count;
  width[state] = sp1[-1] - sp[0] + 1;
}



/* figure out what to do after reducing with each rule,
   depending on the saved state from before the beginning
   of parsing the data that matched this rule.

   The yydefgoto table is output now.  The detailed info
   is saved for putting into yytable later.  */

void
goto_actions()
{
  register int i;
  register int j;
  register int k;

  state_count = NEW2(nstates, short);

  k = default_goto(ntokens);
  fprintf(ftable, "\nstatic const short yydefgoto[] = {%6d", k);
  save_column(ntokens, k);

  j = 10;
  for (i = ntokens + 1; i < nsyms; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      k = default_goto(i);
      fprintf(ftable, "%6d", k);
      save_column(i, k);
    }

  fprintf(ftable, "\n};\n");
  FREE(state_count);
}



int
default_goto(int symbol)
{
  register int i;
  register int m;
  register int n;
  register int default_state;
  register int max;

  m = goto_map[symbol];
  n = goto_map[symbol + 1];

  if (m == n)
    return (-1);

  for (i = 0; i < nstates; i++)
    state_count[i] = 0;

  for (i = m; i < n; i++)
    state_count[to_state[i]]++;

  max = 0;
  default_state = -1;

  for (i = 0; i < nstates; i++)
    {
      if (state_count[i] > max)
	{
	  max = state_count[i];
	  default_state = i;
	}
    }

  return (default_state);
}


void
save_column(int symbol, int default_state)
{
  register int i;
  register int m;
  register int n;
  register short *sp;
  register short *sp1;
  register short *sp2;
  register int count;
  register int symno;

  m = goto_map[symbol];
  n = goto_map[symbol + 1];

  count = 0;
  for (i = m; i < n; i++)
    {
      if (to_state[i] != default_state)
	count++;
    }

  if (count == 0)
    return;

  symno = symbol - ntokens + nstates;

  froms[symno] = sp1 = sp = NEW2(count, short);
  tos[symno] = sp2 = NEW2(count, short);

  for (i = m; i < n; i++)
    {
      if (to_state[i] != default_state)
	{
	  *sp1++ = from_state[i];
	  *sp2++ = to_state[i];
	}
    }

  tally[symno] = count;
  width[symno] = sp1[-1] - sp[0] + 1;
}



/* the next few functions decide how to pack 
   the actions and gotos information into yytable. */

void
sort_actions()
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int w;

  order = NEW2(nvectors, short);
  nentries = 0;

  for (i = 0; i < nvectors; i++)
    {
      if (tally[i] > 0)
	{
	  t = tally[i];
	  w = width[i];
	  j = nentries - 1;

	  while (j >= 0 && (width[order[j]] < w))
	    j--;

	  while (j >= 0 && (width[order[j]] == w) && (tally[order[j]] < t))
	    j--;

	  for (k = nentries - 1; k > j; k--)
	    order[k + 1] = order[k];

	  order[j + 1] = i;
	  nentries++;
	}
    }
}


void
pack_table()
{
  register int i;
  register int place;
  register int state;

  base = NEW2(nvectors, short);
  pos = NEW2(nentries, short);
  table = NEW2(MAXTABLE, short);
  check = NEW2(MAXTABLE, short);

  lowzero = 0;
  high = 0;

  for (i = 0; i < nvectors; i++)
    base[i] = MINSHORT;

  for (i = 0; i < MAXTABLE; i++)
    check[i] = -1;

  for (i = 0; i < nentries; i++)
    {
      state = matching_state(i);

      if (state < 0)
	place = pack_vector(i);
      else
	place = base[state];

      pos[i] = place;
      base[order[i]] = place;
    }

  for (i = 0; i < nvectors; i++)
    {
      if (froms[i])
	FREE(froms[i]);
      if (tos[i])
	FREE(tos[i]);
    }

  FREE(froms);
  FREE(tos);
  FREE(pos);
}



int
matching_state(int vector)
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int w;
  register int match;
  register int prev;

  i = order[vector];
  if (i >= nstates)
    return (-1);

  t = tally[i];
  w = width[i];

  for (prev = vector - 1; prev >= 0; prev--)
    {
      j = order[prev];
      if (width[j] != w || tally[j] != t)
	return (-1);

      match = 1;
      for (k = 0; match && k < t; k++)
	{
	  if (tos[j][k] != tos[i][k] || froms[j][k] != froms[i][k])
	    match = 0;
	}

      if (match)
	return (j);
    }

  return (-1);
}



int
pack_vector(int vector)
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int loc;
  register int ok;
  register short *from;
  register short *to;

  i = order[vector];
  t = tally[i];

  if (t == 0)
    berror("pack_vector");

  from = froms[i];
  to = tos[i];

  for (j = lowzero - from[0]; j < MAXTABLE; j++)
    {
      ok = 1;

      for (k = 0; ok && k < t; k++)
	{
	  loc = j + from[k];
	  if (loc > MAXTABLE)
	    fatals("maximum table size (%d) exceeded",(void*) MAXTABLE);

	  if (table[loc] != 0)
	    ok = 0;
	}

      for (k = 0; ok && k < vector; k++)
	{
	  if (pos[k] == j)
	    ok = 0;
	}

      if (ok)
	{
	  for (k = 0; k < t; k++)
	    {
	      loc = j + from[k];
	      table[loc] = to[k];
	      check[loc] = from[k];
	    }

	  while (table[lowzero] != 0)
	    lowzero++;

	  if (loc > high)
	    high = loc;

	  return (j);
	}
    }

  berror("pack_vector");
  return 0;	/* JF keep lint happy */
}



/* the following functions output yytable, yycheck
   and the vectors whose elements index the portion starts */

void
output_base()
{
  register int i;
  register int j;

  fprintf(ftable, "\nstatic const short yypact[] = {%6d", base[0]);

  j = 10;
  for (i = 1; i < nstates; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", base[i]);
    }

  fprintf(ftable, "\n};\n\nstatic const short yypgoto[] = {%6d", base[nstates]);

  j = 10;
  for (i = nstates + 1; i < nvectors; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", base[i]);
    }

  fprintf(ftable, "\n};\n");
  FREE(base);
}


void
output_table()
{
  register int i;
  register int j;

  fprintf(ftable, "\n\n#define\tYYLAST\t\t%d\n\n", high);
  fprintf(ftable, "\nstatic const short yytable[] = {%6d", table[0]);

  j = 10;
  for (i = 1; i <= high; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", table[i]);
    }

  fprintf(ftable, "\n};\n");
  FREE(table);
}


void
output_check()
{
  register int i;
  register int j;

  fprintf(ftable, "\nstatic const short yycheck[] = {%6d", check[0]);

  j = 10;
  for (i = 1; i <= high; i++)
    {
      putc(',', ftable);

      if (j >= 10)
	{
	  putc('\n', ftable);
	  j = 1;
	}
      else
	{
	  j++;
	}

      fprintf(ftable, "%6d", check[i]);
    }

  fprintf(ftable, "\n};\n");
  FREE(check);
}



/* copy the parser code into the ftable file at the end.  */

void
 output_parser()
{
 register int c;
 output_section(fparser,ftable);
 rewind(faction);
 for(c=getc(faction);c!=EOF;c=getc(faction))
		putc(c,ftable);
 output_section(fparser,ftable);
}
void
output_section(FILE* fin,FILE* fout)
{
  register int c;
  int dummy;
  int *pcounter=&dummy;
  char *fil_name;
  fil_name="?";
  if(fin==fparser) 
   {pcounter=&line_fparser;fil_name=parser_fname;}
  else if(fin==fhskel) 
   {pcounter=&line_fhskel;fil_name=hskel_fname;}
  /* Loop over lines in the standard parser file.  */
  if (!nolinesflag)
    fprintf(fout, "\n#line %d \"%s\"\n", (*pcounter), quoted_filename(fil_name));

  while (1)
    {
      

      /* now write out the line... */
      for ( c = getc(fin); c != '\n' && c != EOF; c = getc(fin))
	  {if (c == '$')
	    {
  	    if (!nolinesflag)
   		{//something is wrong                "\n/* #line %d \"%s\" */\n#line @\n",
		  fprintf(fout, "\n #line %d \"%s\"\n", (*pcounter), quoted_filename(fil_name));
                }
	      return;
	    }
           else if(c=='@')
	    {fprintf(fout,"%s",parser_name);
	    }
	   else
	    putc(c, fout);
          }
      if (c == EOF)
	break;
      else if(c=='\n') (*pcounter)++;
      putc(c, fout);
    }
}


void
output_program()
{
  register int c;
  extern int lineno;
  int is_escaped=0,is_commented=0;
  char quoted='\0',last='\0';
  int len_match=0,i;
  char *match_open="%header{";
  char *match_close="%}";
  char *match_wait=match_open;
  if (!nolinesflag)
    fprintf(ftable, "#line %d \"%s\"\n", lineno, quoted_filename(infile));

  
  for (c = getc(finput);c != EOF;last=c,c = getc(finput))
    {
     if(!match_wait[len_match])
       {if(match_wait==match_open)
         {match_wait=match_close;
          if (!nolinesflag && definesflag)
	   fprintf(fdefines, "\n#line %d \"%s\"\n", lineno, quoted_filename(infile));

         }
        else 
         {match_wait=match_open;}
        len_match=0;
       }
     else if(c!=match_wait[len_match] || is_escaped || is_commented || quoted)
       {for(i=0;i<len_match;i++)
          {if(match_wait==match_close && definesflag)
             putc(match_wait[i],fdefines);
           putc(match_wait[i],ftable);}
         len_match=0;
       };
     if(c==match_wait[len_match] && !is_escaped && !is_commented && !quoted) 
       {len_match++;}
     else
       {if(match_wait==match_close && definesflag)
          putc(c,fdefines);
        putc(c,ftable);
       }
     if(c=='\n') lineno++;
     if(is_escaped)
       {is_escaped=0;}
     else if(c=='\\') 
         {is_escaped=1;}
     else if(is_commented==1) 
       {if(last=='*' && c=='/') 
         is_commented=0;}
     else if(is_commented==2) 
       {if(c=='\n') 
         is_commented=0;}
     else if((c=='"'|| c== '\'')) 
         {if(!quoted) quoted=c;
          else if(quoted==c) quoted='\0';
         }
     else if(quoted) {}
     else if(last=='/' && c=='*') is_commented=1;
     else if(last=='/' && c=='/') is_commented=2;
     
      
    }
}


void
free_itemsets()
{
  register core *cp,*cptmp;

  FREE(state_table);

  for (cp = first_state; cp; cp = cptmp) {
    cptmp=cp->next;
    FREE(cp);
  }
}


void
free_shifts()
{
  register shifts *sp,*sptmp;/* JF derefrenced freed ptr */

  FREE(shift_table);

  for (sp = first_shift; sp; sp = sptmp) {
    sptmp=sp->next;
    FREE(sp);
  }
}


void
free_reductions()
{
  register reductions *rp,*rptmp;/* JF fixed freed ptr */

  FREE(reduction_table);

  for (rp = first_reduction; rp; rp = rptmp) {
    rptmp=rp->next;
    FREE(rp);
  }
}
void output_token_defines();
void output_token_const_def();
void output_token_const_decl();

void output_about_token()
{ register int i;
 
  output_section(fparser,ftable);
  output_token_defines(ftable);
  output_section(fparser,ftable);
  output_token_const_decl(ftable);
  output_section(fparser,ftable);  /* new section */
  output_token_enum(ftable); /* enum */
  output_section(fparser,ftable);
  output_token_const_def(ftable);
  output_section(fparser,ftable);
  if (definesflag)
    {
      output_section(fhskel,fdefines);
      output_token_defines(fdefines);
      output_section(fhskel,fdefines);
      output_token_const_decl(fdefines);
      output_section(fhskel,fdefines);  /* new section */
      output_token_enum(fdefines); /* enum */
      output_section(fhskel,fdefines);
      if (semantic_parser)
	for (i = ntokens; i < nsyms; i++)
	  {
	    /* don't make these for dummy nonterminals made by gensym.  */
	    if (*tags[i] != '@')
	      fprintf(fdefines, "#define\tNT%s\t%d\n", tags[i], i);
	  }
    }

};
void output_token_defines(FILE* file)
{output_token_defines_fmt(file,"#define\t%s\t%d\n",0);
 if (semantic_parser)
  output_token_defines_fmt(file,"#define\tT%s\t%d\n",1);
};
void output_token_const_def(FILE* file)
{char line[256];
 sprintf(line,"const int YY_%s_CLASS::%%s=%%d;\n",parser_name);
 output_token_defines_fmt(file,line,0);
 sprintf(line,"const int YY_%s_CLASS::T%%s=%%d;\n",parser_name);
 if (semantic_parser)
  output_token_defines_fmt(file,line,1);
};
void output_token_const_decl(FILE* file)
{char line[256];
 output_token_defines_fmt(file,"static const int %s;\n",0);
 if (semantic_parser)
  output_token_defines_fmt(file,"static const int T%s;\n",1);
};
/* create a list like 
	,FIRST_TOKEN=256
	,SECOND_TOKEN=257
*/
void output_token_enum(FILE* file)
{
 output_token_defines_fmt(file,"\t,%s=%d\n",0);
 if (semantic_parser) /* just for compatibility with semantic parser */
  output_token_defines_fmt(file,"\t,T%s=%d\n",1);
};


void
output_token_defines_fmt(FILE* file,char* fmt,int notrans)
{
  bucket *bp;

  for (bp = firstsymbol; bp; bp = bp->next)
    {
      if (bp->value >= ntokens) continue;

      /* For named tokens, but not literal ones, define the name.  */
      /* The value is the user token number.  */

      if ('\'' != *tags[bp->value] && bp != errtoken)
	{
	  register char *cp = tags[bp->value];
	  register char c;

	  /* Don't #define nonliteral tokens whose names contain periods.  */

	  while ((c = *cp++) && c != '.');
	  if (!c)
	    {
              fprintf(file, fmt, tags[bp->value],
			    (translations && !notrans ? bp->user_token_number : bp->value));
	    }
	}
    }

  putc('\n', file);
}


char *quoted_filename(char* f)
{
 static char *buffer=NULL;
 static int buff_size=0;
 char *p;
 if(buff_size<strlen(f)*2+1)
  {
   if(buffer !=NULL ) free(buffer);
   buffer=xmalloc(strlen(f)*2+1);
  }
 for(p=buffer;*f;f++)
  {if(*f=='\\')
    *p++ ='\\';
   *p++ =*f;
  }
 *p++='\0';
 return buffer;
}
