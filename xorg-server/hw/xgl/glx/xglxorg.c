/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: David Reveman <davidr@novell.com>
 *          Matthias Hopf <mhopf@suse.de>
 */

#include "xglx.h"

#ifndef NXGLXORG

#include <X11/Xauth.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>

typedef void (*sighandler_t) (int);

#define XORG_DIE_TIMEOUT 3
#define XORG_DEV_RANDOM  "/dev/urandom"

static char xorgAuthBuf[256];
static char *xorgAuthTempl = "/tmp/.Xgl-auth-XXXXXX";
static char *xorgAuth      = NULL;

static char *xorgProgs[] = { "/usr/bin/Xorg", "/usr/X11R6/bin/Xorg" };
static char *xorgProg    = NULL;

static char *xorgDisplay   = ":93";
static char *xorgTerminate = "-terminate";

static pid_t   xorgPid = 0;
static int     receivedUsr1 = 0;
static jmp_buf jumpbuf;

static Bool waitAndExit = FALSE;

static char **xorgArgv = 0;
static int  nXorgArgv  = 0;

typedef struct _xglxArg *xglxArgPtr;

typedef int (*xglxProcessArgumentProc) (xglxArgPtr, int, char **, int);

typedef struct _xglxArg {
    xglxProcessArgumentProc processArgument;
    const char		    *name;
    const char		    *usage;
} xglxArgRec;

static int
xglxAddXorgArguments (char **argv,
		      int  n)
{
    char **newArgv;
    int  i;

    newArgv = xrealloc (xorgArgv, sizeof (char *) * (nXorgArgv + n));
    if (!newArgv)
	return 0;

    for (i = 0; i < n; i++)
	newArgv[nXorgArgv + i] = argv[i];

    xorgArgv   = newArgv;
    nXorgArgv += n;

    return n;
}

static int
xglxProcessCommonXorgArgument (xglxArgPtr pArg,
			       int	  n,
			       int	  argc,
			       char	  **argv,
			       int	  i)
{
    if (strcmp (argv[i], pArg->name) == 0)
    {
	if (i + n - 1 < argc)
	    return xglxAddXorgArguments (&argv[i], n);
    }

    return 0;
}

#define PROCESS_COMMON_XORG_ARGUMENT_IMP(args)				  \
    static int								  \
    xglxProcess ## args ## CommonXorgArgument (xglxArgPtr pArg,		  \
					       int	  argc,		  \
					       char	  **argv,	  \
					       int	  i)		  \
    {									  \
	return xglxProcessCommonXorgArgument (pArg, args, argc, argv, i); \
    }

PROCESS_COMMON_XORG_ARGUMENT_IMP (1)
PROCESS_COMMON_XORG_ARGUMENT_IMP (2)

static int
xglxProcessXorgVTArgument (xglxArgPtr pArg,
			   int	      argc,
			   char	      **argv,
			   int	      i)
{
    if (argv[i][0] == 'v' && argv[i][1] == 't' &&
	strspn (&argv[i][2], "0123456789") == strlen (&argv[i][2]))
	return xglxAddXorgArguments (&argv[i], 1);

    return 0;
}

static int
xglxProcessXorgAcArgument (xglxArgPtr pArg,
			   int	      argc,
			   char	      **argv,
			   int	      i)
{
    static char *ac = "-ac";

    if (strcmp (argv[i], pArg->name) == 0)
    {
	if (xglxAddXorgArguments (&ac, 1))
	    return 1;
    }

    return 0;
}

static int
xglxProcessXorgVersionArgument (xglxArgPtr pArg,
				int	   argc,
				char	   **argv,
				int	   i)
{
    static char *version = "-version";

    if (strcmp (argv[i], pArg->name) == 0)
    {
	if (xglxAddXorgArguments (&version, 1))
	{
	    waitAndExit = TRUE;
	    return 1;
	}
    }

    return 0;
}

static int
xglxProcessXorgProgArgument (xglxArgPtr pArg,
			     int	argc,
			     char	**argv,
			     int	i)
{
    if (strcmp (argv[i], pArg->name) == 0)
    {
	if (i + 1 < argc)
	{
	    xorgProg = argv[i + 1];
	    return 2;
	}
    }

    return 0;
}

static int
xglxProcessXorgDisplayArgument (xglxArgPtr pArg,
				int	   argc,
				char	   **argv,
				int	   i)
{
    if (strcmp (argv[i], pArg->name) == 0)
    {
	if (i + 1 < argc)
	{
	    xorgDisplay = argv[i + 1];
	    return 2;
	}
    }

    return 0;
}

static int
xglxProcessXorgWaitExitArgument (xglxArgPtr pArg,
				 int	    argc,
				 char	    **argv,
				 int	    i)
{
    if (xglxProcessCommonXorgArgument (pArg, 1, argc, argv, i))
    {
	waitAndExit = TRUE;
	return 1;
    }

    return 0;
}

#define ARG(processArgument, name, usage) \
    { processArgument, name, usage }

#define XORG_ARG(name, args)				     \
    ARG (xglxProcess ## args ## CommonXorgArgument, name, 0)

#define XORG_UARG(name, usage, args)				 \
    ARG (xglxProcess ## args ## CommonXorgArgument, name, usage)

xglxArgRec xorgUid0Args[] = {
    XORG_UARG ("-modulepath", " paths      specify the module search path", 2),
    XORG_UARG ("-logfile", " file          specify a log file name", 2),
    ARG (xglxProcessXorgWaitExitArgument, "-configure",
	 "             probe for devices and write an Xorg config")
};

xglxArgRec xorgUidArgs[] = {
    XORG_UARG ("-config",
	       " file           specify configuration file, relative to the\n"
	       "                       Xorg config search path, "
	       "only root can use absolute", 2)
};

xglxArgRec xorgArgs[] = {
    ARG (xglxProcessXorgWaitExitArgument, "-probeonly",
	 "             probe for devices, then exit"),
    XORG_UARG ("-verbose", " [n]           verbose startup messages", 2),
    XORG_UARG ("-logverbose", " [n]        verbose log messages", 2),
    XORG_UARG ("-quiet", "                 minimal startup messages", 1),
    XORG_UARG ("-depth", " n               set colour depth. Default: 8", 2),
    XORG_UARG ("-gamma",
	       " f               set gamma value (0.1 < f < 10.0) "
	       "Default: 1.0", 2),
    XORG_UARG ("-rgamma", " f              set gamma value for red phase", 2),
    XORG_UARG ("-ggamma", " f              set gamma value for green phase",
	       2),
    XORG_UARG ("-bgamma", " f              set gamma value for blue phase", 2),
    XORG_UARG ("-layout",
	       " name           specify the ServerLayout section name", 2),
    XORG_UARG ("-screen",
	       " name           specify the Screen section name", 2),
    XORG_UARG ("-keyboard",
	       " name         specify the core keyboard InputDevice name", 2),
    XORG_UARG ("-pointer",
	       " name          specify the core pointer InputDevice name", 2),
    XORG_UARG ("-nosilk", "                disable Silken Mouse", 1),
    XORG_UARG ("-disableModInDev",
	       "       disable dynamic modification of input device settings",
	       1),
    XORG_UARG ("-allowMouseOpenFail",
	       "    start server even if the mouse can't be initialized", 1),
    XORG_UARG ("-bestRefresh",
	       "           choose modes with the best refresh rate", 1),
    XORG_UARG ("-ignoreABI",
	       "             make module ABI mismatches non-fatal", 1),
    XORG_UARG ("-isolateDevice",
	       " bus_id  restrict device resets to bus_id (PCI only)", 2),
    ARG (xglxProcessXorgVTArgument, "vtXX",
	 "                   use the specified VT number"),
    XORG_UARG ("-keeptty",
	       "               don't detach controlling tty "
	       "(for debugging only)", 1),
    XORG_UARG ("-novtswitch", "            don't immediately switch to new VT",
	       1),
    XORG_UARG ("-sharevts", "              share VTs with another X server",
	       1),
    ARG (xglxProcessXorgAcArgument, "-xorgAc",
	 "                disable access control restrictions"),
    ARG (xglxProcessXorgProgArgument, "-xorgProgram",
	 "           server program"),
    ARG (xglxProcessXorgDisplayArgument, "-xorgDisplay",
	 "           server display"),
    ARG (xglxProcessXorgVersionArgument, "-xorgVersion",
	 "           show the server version")
};

xglxArgRec sharedArgs[] = {
    XORG_ARG ("-br", 1)
};

void
xglxUseXorgMsg (void)
{
    int i;

    ErrorF ("\nXorg usage:\n");

    if (getuid () == 0)
    {
	for (i = 0; i < sizeof (xorgUid0Args) / sizeof (xglxArgRec); i++)
	    ErrorF ("%s%s\n", xorgUid0Args[i].name, xorgUid0Args[i].usage);
    }
    else
    {
	for (i = 0; i < sizeof (xorgUidArgs) / sizeof (xglxArgRec); i++)
	    ErrorF ("%s%s\n", xorgUidArgs[i].name, xorgUidArgs[i].usage);
    }

    for (i = 0; i < sizeof (xorgArgs) / sizeof (xglxArgRec); i++)
	ErrorF ("%s%s\n", xorgArgs[i].name, xorgArgs[i].usage);
}

int
xglxProcessXorgArgument (int  argc,
			 char **argv,
			 int  i)
{
    int skip, j;

    if (nXorgArgv == 0)
    {
	if (!xglxAddXorgArguments (&xorgProg, 1))
	    return 0;
    }

    if (getuid () == 0)
    {
	for (j = 0; j < sizeof (xorgUid0Args) / sizeof (xglxArgRec); j++)
	{
	    skip = (*xorgUid0Args[j].processArgument) (&xorgUid0Args[j],
						       argc, argv, i);
	    if (skip)
		return skip;
	}
    }
    else
    {
	for (j = 0; j < sizeof (xorgUidArgs) / sizeof (xglxArgRec); j++)
	{
	    skip = (*xorgUidArgs[j].processArgument) (&xorgUidArgs[j],
						      argc, argv, i);
	    if (skip)
		return skip;
	}
    }

    for (j = 0; j < sizeof (xorgArgs) / sizeof (xorgArgs[0]); j++)
    {
	skip = (*xorgArgs[j].processArgument) (&xorgArgs[j], argc, argv, i);
	if (skip)
	    return skip;
    }

    for (j = 0; j < sizeof (sharedArgs) / sizeof (sharedArgs[0]); j++)
    {
	skip = (*sharedArgs[j].processArgument) (&sharedArgs[j], argc, argv, i);
	if (skip)
	    return 0;
    }

    return 0;
}

static void
sigAlarm (int sig)
{
    ErrorF ("%s won't die, killing it\n", basename (xorgProg));

    kill (xorgPid, SIGKILL);
    if (xorgPid)
	while (waitpid (xorgPid, NULL, 0) == -1 && errno == EINTR);
}

void
xglxAbortXorg (void)
{
    sighandler_t oldSigAlarm;
    unsigned int oldAlarm;
    int          status = 0;
    char	 *name;

    if (!xorgPid)
	return;

    name = basename (xorgProg);

    oldAlarm    = alarm (0);
    oldSigAlarm = signal (SIGALRM, sigAlarm);

    kill (xorgPid, SIGTERM);

    alarm (XORG_DIE_TIMEOUT);
    while (waitpid (xorgPid, &status, 0) == -1 && errno == EINTR);
    alarm (0);

    signal (SIGALRM, oldSigAlarm);
    alarm (oldAlarm);

    if (WIFEXITED (status))
    {
	if (WEXITSTATUS (status))
	    ErrorF ("%s died, exit status %d\n", name, WEXITSTATUS (status));
    }
    else if (WIFSIGNALED (status))
	ErrorF ("%s died, signal %d\n", name, WTERMSIG (status));
    else
	ErrorF ("%s died, dubious exit\n", name);

    if (xorgAuth)
	unlink (xorgAuth);
}

static void
sigUsr1Waiting (int sig)
{
    signal (sig, sigUsr1Waiting);
    receivedUsr1++;
}

static void
sigUsr1Jump (int sig)
{

#ifdef HAVE_SIGPROCMASK
    sigset_t set;
#endif

    signal (sig, sigUsr1Waiting);

#ifdef HAVE_SIGPROCMASK
    sigemptyset (&set);
    sigaddset (&set, SIGUSR1);
    sigprocmask (SIG_UNBLOCK, &set, NULL);
#endif

    longjmp (jumpbuf, 1);
}

#define AUTH_DATA_LEN 16 /* bytes of authorization data */

static Bool
xglxSetupAuth (char *name, int authFd)
{
    Xauth   auth;
    int	    randomFd;
    ssize_t bytes, size;
    char    authHost[256];
    char    authData[AUTH_DATA_LEN];
    FILE    *file;

    auth.family = FamilyLocal;

    gethostname (authHost, sizeof (authHost));

    auth.address	= authHost;
    auth.address_length = strlen (authHost);

    auth.number	= strrchr (xorgDisplay, ':');
    if (!auth.number)
    {
	ErrorF ("Bad Xorg display name: %s\n", xorgDisplay);
	return FALSE;
    }

    auth.number++;

    auth.number_length = strlen (auth.number);
    if (!auth.number_length)
    {
	ErrorF ("Bad Xorg display name: %s\n", xorgDisplay);
	return FALSE;
    }

    auth.name	     = "MIT-MAGIC-COOKIE-1";
    auth.name_length = strlen (auth.name);

    randomFd = open (XORG_DEV_RANDOM, O_RDONLY);
    if (randomFd == -1)
    {
	ErrorF ("Failed to open " XORG_DEV_RANDOM "\n");
	return FALSE;
    }

    bytes = 0;
    do {
	size = read (randomFd, authData + bytes, AUTH_DATA_LEN - bytes);
	if (size <= 0)
	    break;

	bytes += size;
    } while (bytes != AUTH_DATA_LEN);

    close (randomFd);

    if (bytes != AUTH_DATA_LEN)
    {
	ErrorF ("Failed to read %d random bytes from " XORG_DEV_RANDOM "\n",
		AUTH_DATA_LEN);
	return FALSE;
    }

    auth.data	     = authData;
    auth.data_length = AUTH_DATA_LEN;

    file = fdopen (authFd, "w");
    if (!file)
    {
	ErrorF ("Failed to open authorization file: %s\n", name);
	close (authFd);
	return FALSE;
    }

    XauWriteAuth (file, &auth);
    fclose (file);

    return TRUE;
}

char *
xglxInitXorg (void)
{
    sighandler_t oldSigUsr1;
    pid_t	 pid;
    char	 *name;
    char	 *auth[] = { "-auth", xorgAuthBuf, "-nolisten", "tcp" };
    char	 *saver[] = { "-dpms", "-v", "-s", "0" };
    char	 *endArg = NULL;
    int		 authFd;
    int		 mask;

    if (xorgPid)
	return xorgDisplay;

    if (!xorgProg)
    {
	struct stat buf;
	int i;

	for (i = 0; i < sizeof (xorgProgs) / sizeof (char *); i++)
	{
	    if (stat (xorgProgs[i], &buf) == 0)
	    {
		xorgProg = xorgProgs[i];
		break;
	    }
	}

	if (!xorgProg)
	    FatalError ("Can't find Xorg executable\n");
    }

    strcpy (xorgAuthBuf, xorgAuthTempl);
    mask = umask (0077);
    authFd = mkstemp (xorgAuthBuf);
    umask (mask);
    if (authFd == -1)
	FatalError ("Failed to generate unique authorization file\n");

    xorgAuth = xorgAuthBuf;

    if (nXorgArgv == 0)
    {
	if (!xglxAddXorgArguments (&xorgProg, 1))
	    return 0;
    }
    else
    {
	xorgArgv[0] = xorgProg;
    }

    if (!xglxAddXorgArguments (auth, sizeof (auth) / sizeof (char *)))
	return 0;

    if (!xglxAddXorgArguments (saver, sizeof (saver) / sizeof (char *)))
	return 0;

    if (!xglxAddXorgArguments (&xorgDisplay, 1))
	return 0;

    if (!xglxAddXorgArguments (&xorgTerminate, 1))
	return 0;

    if (!xglxAddXorgArguments (&endArg, 1))
	return 0;

    name = basename (xorgProg);

    if (!xglxSetupAuth (xorgAuth, authFd))
	FatalError ("Failed to set up authorization: %s\n", xorgAuth);

    oldSigUsr1 = signal (SIGUSR1, sigUsr1Waiting);

    pid = fork ();

    switch (pid) {
    case -1:
	perror ("fork");
	FatalError ("fork");
	break;
    case 0:
	signal (SIGUSR1, SIG_IGN);
	execv (xorgArgv[0], xorgArgv);
	perror (xorgArgv[0]);
	exit (2);
	break;
    default:
	xorgPid = pid;
	break;
    }

    for (;;)
    {
	int status;

	signal (SIGUSR1, sigUsr1Waiting);
	if (setjmp (jumpbuf) && !waitAndExit)
	    break;

	signal (SIGUSR1, sigUsr1Jump);
	if (receivedUsr1 && !waitAndExit)
	    break;

	if (waitpid (xorgPid, &status, 0) != -1)
	{
	    if (WIFEXITED (status))
	    {
		if (waitAndExit)
		{
		    if (WEXITSTATUS (status))
			FatalError ("%s died, exit status %d\n", name,
				    WEXITSTATUS (status));

		    exit (WEXITSTATUS (status));
		}
		else
		{
		    FatalError ("%s died, exit status %d\n", name,
				WEXITSTATUS (status));
		}
	    }
	    else if (WIFSIGNALED (status))
		FatalError ("%s died, signal %d\n", name, WTERMSIG (status));
	    else
		FatalError ("%s died, dubious exit\n", name);
	}
    }

    signal (SIGUSR1, oldSigUsr1);

    setenv ("XAUTHORITY", xorgAuth, 1);

    return xorgDisplay;
}

#endif
