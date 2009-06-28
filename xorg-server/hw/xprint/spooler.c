/*
Copyright (c) 2003-2004 Roland Mainz <roland.mainz@nrubsig.org>
Copyright (c) 2004      Sun Microsystems, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#ifdef __hpux
#include <sys/sysmacros.h>
#endif

#include "spooler.h"

/*
 * The string LIST_QUEUES_* is fed to a shell to generate an ordered
 * list of available printers on the system. These string definitions
 * are taken from the file PrintSubSys.C within the code for the 
 * dtprintinfo program.
 */
#define LIST_QUEUES_AIX4 \
  "lsallq | grep -v '^bsh$' | sort | uniq"

#define LIST_QUEUES_HPUX \
  "LANG=C lpstat -v | " \
  "awk '" \
  " $2 == \"for\" " \
  "   { " \
  "      x = match($3, /:/); " \
  "      print substr($3, 1, x-1)" \
  "   }' | sort | uniq"

#define LIST_QUEUES_OSF \
  "LANG=C lpstat -v | " \
  "nawk '" \
  " $2 == \"for\"    " \
  "   { print $4 }' " \
  "   | sort | uniq"

#define LIST_QUEUES_UXP \
  "LANG=C lpstat -v |" \
  "nawk '" \
  " $4 == \"for\" " \
  "   { " \
  "      x = match($5, /:/); " \
  "      print substr($5, 1, x-1)" \
  "   }' | sort | uniq"

/* Support both normal and LPRng output of "lpc status" */
#define LIST_QUEUES_BSD \
  "PATH=\"${PATH}:/usr/bin:/usr/sbin:/bin:/sbin\"\n" \
  "export PATH\n" \
  \
  "which_tool()\n" \
  "{\n" \
  "  echo \"${PATH}\" | tr \":\" \"\n\" | while read i ; do ls -1ad \"${i}/${1}\" 2>/dev/null ; done\n" \
  "}\n" \
  \
  "(\n" \
  "WHICH_LPC=\"`which_tool lpc`\"\n" \
  \
  "if [ \"`which_tool nawk`\" != \"\" ] ; then\n" \
  "    NAWK=\"nawk\"\n" \
  "else\n" \
  "    NAWK=\"awk\"\n" \
  "fi\n" \
  \
  "[ \"${WHICH_LPC}\"    != \"\" ] && (LANG=C lpc status    | ${NAWK} '/^[^ ]*:$/    && !/@/   && !/ / { print $1 }' | sed -e /:/s///)\n" \
  "[ \"${WHICH_LPC}\"    != \"\" ] && (LANG=C lpc -a status | ${NAWK} '/^[^ ]*@[^ ]/ && !/:$/          { split( $1, name, \"@\" ); print name[1]; }')\n" \
  ") | egrep -v -i \" |^all$\" | sort | uniq"

#define LIST_QUEUES_SYSV \
  "PATH=\"${PATH}:/usr/bin:/usr/sbin:/bin:/sbin\"\n" \
  "export PATH\n" \
  \
  "which_tool()\n" \
  "{\n" \
  "  echo \"${PATH}\" | tr \":\" \"\n\" | while read i ; do ls -1ad \"${i}/${1}\" 2>/dev/null ; done\n" \
  "}\n" \
  \
  "(\n" \
  "WHICH_LPSTAT=\"`which_tool lpstat`\"\n" \
  \
  "if [ \"`which_tool nawk`\" != \"\" ] ; then\n" \
  "    NAWK=\"nawk\"\n" \
  "else\n" \
  "    NAWK=\"awk\"\n" \
  "fi\n" \
  \
  "[ \"${WHICH_LPSTAT}\" != \"\" ] && (LANG=C lpstat -v     | ${NAWK} ' $2 == \"for\" { x = match($3, /:/); print substr($3, 1, x-1)   }')\n" \
  ") | egrep -v -i \" |^all$\" | sort | uniq"

#define LIST_QUEUES_SOLARIS "LANG=C lpget -k description " \
  "`lpstat -v " \
    "| nawk '$2 == \"for\" { x = match($3, /:/); print substr($3, 1,x-1) }' " \
    "| sort -u` " \
  "| nawk -F: ' NF == 2 { name=$1 } " \
             " NF == 1 { sub(\"^.*description\\( - undefined|=\\)\",\"\"); " \
                        " printf \"%s\txp-printerattr.descriptor=%s\\n\",  name, $1 } '"

#define LIST_QUEUES_OTHER \
  "LANG=C lpstat -v | " \
  "nawk '" \
  " $2 == \"for\" " \
  "   { " \
  "      x = match($3, /:/); " \
  "      print substr($3, 1, x-1)" \
  "   }' | sort | uniq"
  
#define DEFAULT_SPOOL_COMMAND_HPUX      "/usr/bin/lp -d %printer-name% -o raw -n %copy-count% -t %job-name% %options%"
#define DEFAULT_SPOOL_COMMAND_BSD       "/usr/bin/lpr -P %printer-name% -#%copy-count% -T %job-name% %options%"
#define DEFAULT_SPOOL_COMMAND_SYSV      "/usr/bin/lp -d %printer-name% -n %copy-count% -t %job-name% %options%"
#define DEFAULT_SPOOL_COMMAND_SOLARIS   "/usr/bin/lp -d %printer-name% -n %copy-count% -t %job-name% %options%"
#define DEFAULT_SPOOL_COMMAND_OTHER     "/usr/bin/lp -d %printer-name% -n %copy-count% -t %job-name% %options%"


/* List of spooler types and the commands used to enumerate
 * print queues and submit print jobs */
XpSpoolerType xpstm[] =
{
  /* OS-specific spoolers */
  { "aix",      LIST_QUEUES_AIX4,       DEFAULT_SPOOL_COMMAND_OTHER      },
  { "aix4",     LIST_QUEUES_AIX4,       DEFAULT_SPOOL_COMMAND_OTHER      },
  { "bsd",      LIST_QUEUES_BSD,        DEFAULT_SPOOL_COMMAND_BSD        },
  { "osf",      LIST_QUEUES_OSF,        DEFAULT_SPOOL_COMMAND_OTHER      },
  { "solaris",  LIST_QUEUES_SOLARIS,    DEFAULT_SPOOL_COMMAND_SOLARIS    },
  { "sysv",     LIST_QUEUES_SYSV,       DEFAULT_SPOOL_COMMAND_SYSV       },
  { "uxp",      LIST_QUEUES_UXP,        DEFAULT_SPOOL_COMMAND_OTHER      },
  /* crossplatform spoolers */
  { "cups",     LIST_QUEUES_SYSV,       DEFAULT_SPOOL_COMMAND_SYSV       },
  { "lprng",    LIST_QUEUES_BSD,        DEFAULT_SPOOL_COMMAND_BSD        },
  /* misc */
  { "other",    LIST_QUEUES_OTHER,      DEFAULT_SPOOL_COMMAND_OTHER      },
  { "none",     NULL,                   NULL                             },
  { NULL,       NULL,                   NULL                             }
};

/* Used by Init.c and attributes.c */
XpSpoolerTypePtr spooler_type = NULL;

XpSpoolerTypePtr XpSpoolerNameToXpSpoolerType(char *name)
{
  XpSpoolerTypePtr curr = xpstm;
  
  while( curr->name != NULL )
  {
    if( !strcasecmp(name, curr->name) )
      return curr;
      
    curr++;
  }
  
  return NULL;
}

static char *spooler_namelist = NULL;

char *XpGetSpoolerTypeNameList(void)
{
  if( spooler_namelist )
    return spooler_namelist;
  
  return XPDEFAULTSPOOLERNAMELIST;
}

void XpSetSpoolerTypeNameList(char *namelist)
{
  spooler_namelist = namelist;
}


