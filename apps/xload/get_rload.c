/* $XFree86: xc/programs/xload/get_rload.c,v 1.4 2002/01/07 20:38:31 dawes Exp $ */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "xload.h"

/* Not all OS supports get_rload
   steal the STUB idea from get_load
 */
#if defined(QNX4) || defined(__CYGWIN__)
#define RLOADSTUB
#endif

#ifdef RLOADSTUB
void GetRLoadPoint( w, closure, call_data )
     Widget   w;              /* unused */
     XtPointer  closure;        /* unused */
     XtPointer  call_data;      /* pointer to (double) return value */

{
  *(double *)call_data = 1.0;
}
#else  /* RLOADSTUB */

#include <protocols/rwhod.h>
#ifndef _PATH_RWHODIR
#define _PATH_RWHODIR "/var/spool/rwho"
#endif

typedef struct _XLoadResources {
  Boolean show_label;
  Boolean use_lights;
  String remote;
} XLoadResources;

extern XLoadResources resources ;

#define WHDRSIZE        ((int)(sizeof (buf) - sizeof (buf.wd_we)))

void GetRLoadPoint( w, closure, call_data )
     Widget   w;              /* unused */
     XtPointer  closure;        /* unused */
     XtPointer  call_data;      /* pointer to (double) return value */

{
  int f;
  static char *fname = NULL;
  static struct whod buf;
  int cc;

  *(double *)call_data = 0.0; /* to be on the safe side */

  if (fname == NULL) {
    if ((fname = malloc(strlen(_PATH_RWHODIR)+strlen("/whod.")+strlen(resources.remote)+1)) == NULL) {
      fprintf(stderr,"GetRLoadPoint: malloc() failed\n");
      exit(1);
    }
    strcpy(fname,_PATH_RWHODIR);
    strcat(fname,"/whod.");
    strcat(fname,resources.remote);
  }
  if ((f = open(fname, O_RDONLY, 0)) < 0)
    return;

  cc = read(f, &buf, sizeof(buf));
  close(f);
  if (cc < WHDRSIZE)
    return;

  *(double *)call_data = buf.wd_loadav[0] / 100.0;
}

#endif  /* RLOADSTUB */
