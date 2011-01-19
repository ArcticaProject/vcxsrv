/*
 * Copyright © 2006 Keith Packard
 * Copyright © 2005 Patrick Lam
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <ctype.h>

#define ENDIAN_TEST 0x01020304
#define MACHINE_SIGNATURE_SIZE 1024

static char *
FcCacheMachineSignature (void)
{
    static char buf[MACHINE_SIGNATURE_SIZE];
    int32_t magic = ENDIAN_TEST;
    char * m = (char *)&magic;

    sprintf (buf, "%01x%01x%01x%01x_"
	     "%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_"
	     "%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x_%02x",
	     m[0], m[1], m[2], m[3],
	     (unsigned int)sizeof (FcAlign),
	     (unsigned int)sizeof (char),
	     (unsigned int)sizeof (char *),
	     (unsigned int)sizeof (int),
	     (unsigned int)sizeof (intptr_t),
	     (unsigned int)sizeof (FcPattern),
	     (unsigned int)sizeof (FcPatternEltPtr),
	     (unsigned int)sizeof (struct  FcPatternElt *),
	     (unsigned int)sizeof (FcPatternElt),
	     (unsigned int)sizeof (FcObject),
	     (unsigned int)sizeof (FcValueListPtr),
	     (unsigned int)sizeof (FcValue),
	     (unsigned int)sizeof (FcValueBinding),
	     (unsigned int)sizeof (struct  FcValueList *),
	     (unsigned int)sizeof (FcStrSet *), /* For FcLangSet */
	     (unsigned int)sizeof (FcCharSet),
	     (unsigned int)sizeof (FcCharLeaf **),
	     (unsigned int)sizeof (FcChar16 *),
	     (unsigned int)sizeof (FcChar16),
	     (unsigned int)sizeof (FcCharLeaf),
	     (unsigned int)sizeof (FcChar32),
	     (unsigned int)sizeof (FcCache));

    return buf;
}

int
main (int argc, char **argv)
{
    static char		line[1024];
    char		*signature;
    int			signature_length;
    char		*space;
    char		*arch = NULL;
    int			lineno = 0;
    
    if (argc != 2)
	fprintf (stderr, "Usage: %s <architecture>|auto < fcarch.tmpl.h > fcarch.h\n",
		 argv[0]);
    arch = argv[1];
    /*
     * Scan the input until the marker is found
     */
    
    while (fgets (line, sizeof (line), stdin))
    {
	lineno++;
	if (!strncmp (line, "@@@", 3))
	    break;
	fputs (line, stdout);
    }
    signature = FcCacheMachineSignature();
    signature_length = strlen (signature);
    
    if (strcmp (arch, "auto") == 0)
    {
	arch = NULL;
	/*
	 * Search for signature
	 */
	while (fgets (line, sizeof (line), stdin)) 
	{
	    lineno++;
	    /*
	     * skip comments
	     */
	    if (!strncmp (line, "@@@", 3))
		continue;
	    space = line;
	    while (*space && !isspace (*space))
		space++;
	    if (!space)
	    {
		fprintf (stderr, "%s: malformed input on line %d\n",
			 argv[0], lineno);
		exit (1);
	    }
	    *space++ = '\0';
	    while (isspace (*space))
		space++;
	    if (!strncmp (space, signature, signature_length))
	    {
		arch = line;
		break;
	    }
	}
    }
    if (!arch)
    {
	fprintf (stderr, "%s: unknown signature \"%s\"\n", argv[0], signature);
	fprintf (stderr, "\tPlease update fcarch.tmpl.h and rebuild\n");
	exit (1);
    }
    printf ("#define FC_ARCHITECTURE \"%s\"\n", arch);
    fflush (stdout);
    exit (ferror (stdout));
}

