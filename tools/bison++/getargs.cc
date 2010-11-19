/* Parse command line arguments for bison,
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


#include <stdio.h>
#include "getopt.h"
#include "system.h"
#include "files.h"

int verboseflag;
int definesflag;
int debugflag;
int nolinesflag;
char *spec_name_prefix; /* for -p.  */
char *spec_file_prefix; /* for -b. */
extern int fixed_outfiles;/* for -y */
extern char *header_name;
extern char *program_name;
extern char *version_string;

extern void fatal();

struct option longopts[] =
{
  {"debug", 0, &debugflag, 1},
  {"defines", 0, &definesflag, 1},
  {"file-prefix", 1, 0, 'b'},
  {"fixed-output-files", 0, &fixed_outfiles, 1},
  {"name-prefix", 1, 0, 'a'},
  {"no-lines", 0, &nolinesflag, 1},
  {"output-file", 1, 0, 'o'},
  {"output", 1, 0, 'o'},
  {"verbose", 0, &verboseflag, 1},
  {"version", 0, 0, 'V'},
  {"yacc", 0, &fixed_outfiles, 1},
  {"skeleton", 1, 0, 'S'},
  {"headerskeleton", 1, 0, 'H'},
  {"header-file", 1, 0, 'h'},
  {"help", 0, 0, 'u'},
  {"usage", 0, 0, 'u'},
  {0, 0, 0, 0}
};


void usage (FILE* stream)
{
	  fprintf (stderr, "\
Usage: %s [-dltvyVu] [-b file-prefix] [-p name-prefix]\n\
       [-o outfile] [-h headerfile]\n\
       [-S skeleton] [-H header-skeleton]\n\
       [--debug] [--defines] [--fixed-output-files] [--no-lines]\n\
       [--verbose] [--version] [--yacc] [--usage] [--help]\n\
       [--file-prefix=prefix] [--name-prefix=prefix]\n\
       [--skeleton=skeletonfile] [--headerskeleton=headerskeletonfile]\n\
       [--output=outfile] [--header-name=header] grammar-file\n",
		   program_name);
}



void
getargs(int argc, char** argv)
{
  register int c;

  verboseflag = 0;
  definesflag = 0;
  debugflag = 0;
  fixed_outfiles = 0;

  while ((c = getopt_long (argc, argv, "yvdltVuo:b:p:S:H:h:", longopts, (int *)0))
	 != EOF)
    {
      switch (c)
	{
	case 0:
	  /* Certain long options cause getopt_long to return 0.  */
	  break;

	case 'y':
	  fixed_outfiles = 1;
	  break;
	  
	case 'V':
	  printf("%s", version_string);
	  exit(0);
	  
	case 'v':
	  verboseflag = 1;
	  break;
	  
	case 'd':
	  definesflag = 1;
	  break;
	  
	case 'l':
	  nolinesflag = 1;
	  break;
	  
	case 't':
	  debugflag = 1;
	  break;
	  
	case 'o':
	  spec_outfile = optarg;
	  break;
	  
	case 'b':
	  spec_file_prefix = optarg;
	  break;
	  
	case 'p':
	  spec_name_prefix = optarg;
	  break;
	case 'S':
	  cparserfile = optarg;
	  break;
	case 'H':
	  hskelfile = optarg;
	  break;
	  
	case 'h':
	  header_name = optarg;
	  break;

	case 'u':
          usage(stdout);
	  exit (0);
	  
	default:
          usage(stderr);
	  exit (1);
	}
    }

  if (optind == argc)
    {
      fprintf(stderr, "%s: no grammar file given\n", program_name);
      exit(1);
    }
  if (optind < argc - 1)
    fprintf(stderr, "%s: warning: extra arguments ignored\n", program_name);

  infile = argv[optind];
}
