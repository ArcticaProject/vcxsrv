/*
 * Copyright 2002 through 2004 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "compiler.h"
#include "xf86_OSproc.h"

#include <errno.h>

static char *MyName;
static int Port = -1, Index = -1;
static unsigned int Value;

static void
inb_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "inb [-i <index>] <port>\n");
}

static void
inw_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "inw [-i <index>] <port>\n");
}

static void
inl_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "inl [-i <index>] <port>\n");
}


static void
outb_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "outb [-i <index>] <port> <value>\n");
}

static void
outw_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "outw [-i <index>] <port> <value>\n");
}

static void
outl_usage
(
#ifdef __STDC__
    void
#endif
)
{
	fprintf(stderr, "outl [-i <index>] <port> <value>\n");
}

static int
#ifdef __STDC__
parse_number
(
    const char *String,
    void (* Usage)(void),
    unsigned int Maximum)
#else
parse_number(String, Usage, Maximum)
    const char *String;
    void (* Usage)();
    unsigned int Maximum;
#endif
{
	char *BadString = (char *)0;
	unsigned int Number = strtoul(String, &BadString, 0);
	if ((Number > Maximum) || errno ||
	    (BadString && *BadString))
	{
		(*Usage)();
		exit(1);
	}

	return (Number);
}

static void
#ifdef __STDC__
input_parse
(
    int argc,
    char **argv,
    void (* Usage)(void))
#else
input_parse(argc, argv, Usage)
    int argc;
    char **argv;
    void (* Usage)();
#endif
{
	if ((argc < 2) || (argc > 4))
	{
		(*Usage)();
		exit(1);
	}

	for(;  (++argv, --argc);  )
	{
		if ((Index < 0) &&
		    (argv[0][0] == '-') &&
		    (argv[0][1] == 'i'))
		{
			if ((++argv[0], *(++argv[0])) || (++argv, --argc))
				Index = parse_number(argv[0], Usage, 0xFFU);
			else
			{
				(*Usage)();
				exit(1);
			}
		}
		else if (Port < 0)
		{
			Port = parse_number(argv[0], Usage, 0xFFFFU);
		}
		else
		{
			(*Usage)();
			exit(1);
		}
	}
}

static void
#ifdef __STDC__
output_parse
(
    int argc,
    char **argv,
    void (* Usage)(void),
    unsigned int Maximum
)
#else
output_parse(argc, argv, Usage, Maximum)
    int argc;
    char **argv;
    void (* Usage)();
    unsigned int Maximum;
#endif
{
	char ValueSpecified = 0;

	if ((argc < 3) || (argc > 5))
	{
		(*Usage)();
		exit(1);
	}

	for (;  (++argv, --argc);  )
	{
		if ((Index < 0) &&
		    (argv[0][0] == '-') &&
		    (argv[0][1] == 'i'))
		{
			if ((++argv[0], *(++argv[0])) || (++argv, --argc))
				Index = parse_number(argv[0], Usage, 0xFFU);
			else
			{
				(*Usage)();
				exit(1);
			}
		}
		else if (Port < 0)
		{
			Port = parse_number(argv[0], Usage, 0xFFFFU);
		}
		else if (!ValueSpecified)
		{
			Value = parse_number(argv[0], Usage, Maximum);
			ValueSpecified = 1;
		}
		else
		{
			(*Usage)();
			exit(1);
		}
	}

	if (!ValueSpecified)
	{
		(*Usage)();
		exit(1);
	}
}

static void
#ifdef __STDC__
do_inb
(
    int argc,
    char **argv
)
#else
do_inb(argc, argv)
    int argc;
    char **argv;
#endif
{
	input_parse(argc, argv, inb_usage);

	xf86EnableIO();

	if (Index >= 0)
	{
		if (Port == 0x03C0U)
		{	/* Attribute Controller is different */
			unsigned short gens1;

			gens1 = ((inb(0x03CCU) & 0x01U) << 5) + 0x03BA;
			(void) inb(gens1);
			Index = (Index & 0x1FU) | 0x20U;
		}
		outb(Port, Index);
		Port++;
	}
	Value = inb(Port);

	xf86DisableIO();

	printf("0x%02X\n", Value);
}

static void
#ifdef __STDC__
do_inw
(
    int argc,
    char **argv
)
#else
do_inw(argc, argv)
    int argc;
    char **argv;
#endif
{
	input_parse(argc, argv, inw_usage);

	xf86EnableIO();

	if (Index >= 0)
	{
		outb(Port, Index);
		Port++;
	}
	Value = inw(Port);

	xf86DisableIO();

	printf("0x%04X\n", Value);
}

static void
#ifdef __STDC__
do_inl
(
    int argc,
    char **argv
)
#else
do_inl(argc, argv)
    int argc;
    char **argv;
#endif
{
	input_parse(argc, argv, inl_usage);

	xf86EnableIO();

	if (Index >= 0)
	{
		outb(Port, Index);
		Port++;
	}
	Value = inl(Port);

	xf86DisableIO();

	printf("0x%08X\n", Value);
}

static void
#ifdef __STDC__
do_outb
(
    int argc,
    char **argv
)
#else
do_outb(argc, argv)
    int argc;
    char **argv;
#endif
{
	output_parse(argc, argv, outb_usage, 0xFFU);

	xf86EnableIO();

	if (Index >= 0)
	{
		if (Port == 0x03C0U)
		{	/* Attribute controller is different */
			unsigned short gens1;

			gens1 = ((inb(0x03CCU) & 0x01U) << 5) + 0x03BA;
			(void) inb(gens1);
			outb(0x03C0U, (Index & 0x1FU) | 0x20U);
		}
		else
		{
			outb(Port, Index);
			Port++;
		}
	}
	outb(Port, Value);

	xf86DisableIO();

}

static void
#ifdef __STDC__
do_outw
(
    int argc,
    char **argv
)
#else
do_outw(argc, argv)
    int argc;
    char **argv;
#endif
{
	output_parse(argc, argv, outw_usage, 0xFFFFU);

	xf86EnableIO();

	if (Index >= 0)
	{
		outb(Port, Index);
		Port++;
	}
	outw(Port, Value);

	xf86DisableIO();

}

static void
#ifdef __STDC__
do_outl
(
    int argc,
    char **argv
)
#else
do_outl(argc, argv)
    int argc;
    char **argv;
#endif
{
	output_parse(argc, argv, outl_usage, 0xFFFFFFFFU);

	xf86EnableIO();

	if (Index >= 0)
	{
		outb(Port, Index);
		Port++;
	}
	outl(Port, Value);

	xf86DisableIO();

}

static void
usage
(
#ifdef __STDC__
    void
#endif
)
{
	inb_usage();
	inw_usage();
	inl_usage();
	outb_usage();
	outw_usage();
	outl_usage();
	exit(1);
}

int
#ifdef __STDC__
main
(
    int argc,
    char **argv
)
#else
main(argc, argv)
    int argc;
    char **argv;
#endif
{
	struct
	{
		char *Name;
#ifdef __STDC__
		void (* Function)(int, char **);
#else
		void (* Function)();
#endif
	}
	*Function_Entry, Function_Table[] =
	{
		{"inb", do_inb},
		{"inw", do_inw},
		{"inl", do_inl},
		{"outb", do_outb},
		{"outw", do_outw},
		{"outl", do_outl},
#ifdef __STDC__
		{(char *)0, (void (*)(int, char **))usage}
#else
		{(char *)0, usage}
#endif
	};

	/* Get name by which we were invoked */
	for (MyName = argv[0];  argv[0][0]; )
		if (*(argv[0]++) == '/')
			MyName = argv[0];

	/* Look up name in table and call corresponding function */
	for (Function_Entry = Function_Table;
	     Function_Entry->Name &&
		strcmp(MyName, Function_Entry->Name);
	     Function_Entry++);
	(*Function_Entry->Function)(argc, argv);

	return (0);
}

