/* Open and close files for bison,
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


#ifndef XPFILE
 #define XPFILE "bison.cc"
#endif
#ifndef XPFILE1
 #define XPFILE1 "bison.hairy"
#endif
#ifndef XHFILE
 #define XHFILE "bison.h"
#endif

#include <stdio.h>
#include "system.h"
#include "files.h"
#include "new.h"
#include "gram.h"

FILE *finput = NULL;
FILE *foutput = NULL;
FILE *fdefines = NULL;
FILE *ftable = NULL;
FILE *fattrs = NULL;
FILE *fguard = NULL;
FILE *faction = NULL;
FILE *fparser = NULL;
FILE *fbisoncomp = NULL; /* outputs YY_USE_CLASS defs (i.e bison or bison++ output*/

extern bool bison_compability;
/* File name specified with -o for the output file, or 0 if no -o.  */
char *spec_outfile;

char *infile;
char *outfile;
char *defsfile;
char *tabfile;
char *attrsfile;
char *guardfile;
char *tmpattrsfile;
char *tmptabfile;
char *tmpdefsfile;
char *tmpbisoncompfile;
/* AC added */
char *hskelfile=NULL;
char *cparserfile=NULL;
FILE *fhskel=NULL;
char *parser_name="parse"; 
int parser_defined=0;
int line_fparser=1;
int line_fhskel=1;
char *parser_fname="bison.cc";
char *hskel_fname="bison.h";
char *header_name=NULL;
/* AC added end*/



extern char     *mktemp();      /* So the compiler won't complain */
extern char     *getenv();
extern void     perror();
FILE    *tryopen(char*,char*);     /* This might be a good idea */
extern void done(int);

extern char *program_name;
extern int verboseflag;
extern int definesflag;
int fixed_outfiles = 0;

static char *c_suffixes[]=
   {".tab.c",".tab.cc",".tab.cpp",".tab.cxx",".tab.C",
    ".c",".cc",".cpp",".cxx",".C",".CPP",".CXX",".CC",(char *)0};



char*
stringappend(char* string1, int end1, char* string2)
{
  register char *ostring;
  register char *cp, *cp1;
  register int i;

  cp = string2;  i = 0;
  while (*cp++) i++;

  ostring = NEW2(i+end1+1, char);

  cp = ostring;
  cp1 = string1;
  for (i = 0; i < end1; i++)
    *cp++ = *cp1++;

  cp1 = string2;
  while (*cp++ = *cp1++) ;

  return ostring;
}


/* JF this has been hacked to death.  Nowaday it sets up the file names for
   the output files, and opens the tmp files and the parser */
// Cleaned up under bison_compability run.x
void
openfiles()
{
  char *name_base;
  register char *cp;
  char *filename;
  int base_length;
  int short_base_length;

  char *tmp_base = "/tmp/b.";
  int tmp_len;

  tmp_len = strlen (tmp_base);

  if (spec_outfile)
    {
      /* -o was specified.  The precise -o name will be used for ftable.
	 For other output files, remove the ".c" or ".tab.c" suffix.  */
      name_base = spec_outfile;
      base_length = strlen (name_base);
      /* SHORT_BASE_LENGTH includes neither ".tab" nor ".c".  */
      char **suffix;
      for(suffix=c_suffixes;*suffix;suffix++)
	/* try to detect .c .cpp .tab.c ... options */
	{
	  if(strlen(name_base)>strlen(*suffix) 
	     && strcmp(name_base+base_length-strlen(*suffix),*suffix)==0)
	    { 
	      base_length -= strlen(*suffix);
	      break;
	    }
	};
      short_base_length=base_length;
    }
  else if (spec_file_prefix)
    {
      /* -b was specified.  Construct names from it.  */
      /* SHORT_BASE_LENGTH includes neither ".tab" nor ".c".  */
      short_base_length = strlen (spec_file_prefix);
      /* Count room for `.tab'.  */
      base_length = short_base_length + 4;
      name_base = (char *) xmalloc (base_length + 1);
      /* Append `.tab'.  */
      strcpy (name_base, spec_file_prefix);
      strcat (name_base, ".tab");
    }
  else
    {
      /* -o was not specified; compute output file name from input
	 or use y.tab.c, etc., if -y was specified.  */

      if(fixed_outfiles)
	{
	  name_base = (char*) malloc(sizeof(char)*10);
	  strcpy(name_base,"y.y"); 
	}
      else
	name_base = infile;

      /* BASE_LENGTH gets length of NAME_BASE, sans ".y" suffix if any.  */

      base_length = strlen (name_base);
      if (!strcmp (name_base + base_length - 2, ".y"))
	base_length -= 2;
      short_base_length = base_length;

      name_base = stringappend(name_base, short_base_length, ".tab");
      base_length = short_base_length + 4;
    }

  finput = tryopen(infile, "r");

  filename=cparserfile;
  if(filename==NULL)
    filename = getenv("BISON_SIMPLE");
  {
    if(filename)
      {   
	parser_fname=(char *)xmalloc(strlen(filename)+1);
	strcpy(parser_fname,filename);
      }
    else
      {   
	parser_fname=(char *)xmalloc(strlen(PFILE)+1);
	strcpy(parser_fname,PFILE);
      }
  }
  fparser = tryopen(parser_fname, "r");

  filename=hskelfile;
  if(filename==NULL)
    filename = getenv("BISON_SIMPLE_H");
  {
    if(filename)
      {   
	hskel_fname=(char *)xmalloc(strlen(filename)+1);
	strcpy(hskel_fname,filename);
      }
    else
      {   
	hskel_fname=(char *)xmalloc(strlen(HFILE)+1);
	strcpy(hskel_fname,HFILE);
      }
  }

  fhskel = tryopen(hskel_fname, "r");

  if (verboseflag)
    {
      if (spec_name_prefix)
	outfile = stringappend(name_base, short_base_length, ".out");
      else
	outfile = stringappend(name_base, short_base_length, ".output");
      foutput = tryopen(outfile, "w");
    }
  
  faction = tmpfile();
  fattrs = tmpfile();
  ftable = tmpfile();
  fbisoncomp = tmpfile();

  if (definesflag)
    { if(header_name)
       defsfile=header_name;
      else 
       defsfile = stringappend(name_base, base_length, ".h");

    fdefines = tmpfile();

    }


  /* These are opened by `done' or `open_extra_files', if at all */
  if (spec_outfile)
    tabfile = spec_outfile;
  else
    tabfile = stringappend(name_base, base_length, ".c");

  attrsfile = stringappend(name_base, short_base_length, ".stype.h");
  guardfile = stringappend(name_base, short_base_length, ".guard.c");
}



/* open the output files needed only for the semantic parser.
This is done when %semantic_parser is seen in the declarations section.  */

void
open_extra_files()
{
  FILE *ftmp;
  int c;
  char *filename, *cp;

  fclose(fparser);
  filename=cparserfile;
  if(filename!=NULL)
    filename = (char *) getenv ("BISON_HAIRY");
#ifdef _MSDOS
  /* File doesn't exist in current directory; try in INIT directory.  */
  cp = getenv("INIT");
  if (filename == 0 && cp != NULL)
    {FILE *tst;
      filename = (char *)xmalloc(strlen(cp) + strlen(PFILE1) + 2);
      strcpy(filename, PFILE1);
      if((tst=fopen(filename,"r"))!=NULL)
       {fclose(tst);}
      else 
      {
      strcpy(filename, cp);
      cp = filename + strlen(filename);
      *cp++ = '/';
      strcpy(cp, PFILE1);
      }

    }
#endif /* MSDOS */
  {

    if(filename)
      {
	parser_fname=(char *)xmalloc(strlen(filename)+1);
	strcpy(parser_fname,filename);	
      }
    else
      {
	parser_fname=(char *)xmalloc(strlen(PFILE1)+1);
	strcpy(parser_fname,PFILE1);	
      }
  }
  fparser = tryopen(parser_fname, "r");


		/* JF change from inline attrs file to separate one */
  ftmp = tryopen(attrsfile, "w");
  rewind(fattrs);
  while((c=getc(fattrs))!=EOF)  /* Thank god for buffering */
    putc(c,ftmp);
  fclose(fattrs);
  fattrs=ftmp;

  fguard = tryopen(guardfile, "w");

}

	/* JF to make file opening easier.  This func tries to open file
	   NAME with mode MODE, and prints an error message if it fails. */
FILE *
tryopen(char* name, char* mode)
{
  FILE  *ptr;

  ptr = fopen(name, mode);
  if (ptr == NULL)
    {
      fprintf(stderr, "%s: ", program_name);
      perror(name);
      done(2);
    }
  return ptr;
}

void
done(int k)
{
  if (faction)
    fclose(faction);

  if (fattrs)
    fclose(fattrs);

  if (fguard)
    fclose(fguard);

  if (finput)
    fclose(finput);

  if (fparser)
    fclose(fparser);

  if (foutput)
    fclose(foutput);

	/* JF write out the output file */
  if (k == 0 && ftable)
    {
      FILE *ftmp;
      register int c;

      ftmp=tryopen(tabfile, "w");
/* avoid reloading the definitions of tab.h */
      fprintf(ftmp,"#define YY_%s_h_included\n",parser_name);
      if(bison_compability==false)
	  fprintf(ftmp,"#define YY_USE_CLASS\n");
      else
	  fprintf(ftmp,"/*#define YY_USE_CLASS \n*/");

      rewind(ftable);
      while((c=getc(ftable)) != EOF)
	putc(c,ftmp);
      fclose(ftmp);
      fclose(ftable);


      if (definesflag)
	{
	  ftmp = tryopen(defsfile, "w");
	  fprintf(ftmp,"#ifndef YY_%s_h_included\n",parser_name);
	  fprintf(ftmp,"#define YY_%s_h_included\n",parser_name);
	  if(bison_compability==false)
	    fprintf(ftmp,"#define YY_USE_CLASS\n");
	  else
	    fprintf(ftmp,"/*#define YY_USE_CLASS \n*/");
	  fflush(fdefines);
	  rewind(fdefines);
	  while((c=getc(fdefines)) != EOF)
	    putc(c,ftmp);
	  fclose(fdefines);
	  fprintf(ftmp,"#endif\n");
	  fclose(ftmp);
	}
    }

}
