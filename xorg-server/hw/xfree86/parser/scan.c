/* 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */


/* View/edit this file with tab stops set to 4 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <X11/Xfuncproto.h>

#if defined(_POSIX_SOURCE)
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif /* _POSIX_SOURCE */

#if !defined(PATH_MAX)
#if defined(MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif /* MAXPATHLEN */
#endif /* !PATH_MAX */

#if !defined(MAXHOSTNAMELEN)
#define MAXHOSTNAMELEN 32
#endif /* !MAXHOSTNAMELEN */

#include "Configint.h"
#include "xf86tokens.h"

#define CONFIG_BUF_LEN     1024

static int StringToToken (char *, xf86ConfigSymTabRec *);

static FILE *configFile = NULL;
static const char **builtinConfig = NULL;
static int builtinIndex = 0;
static int configPos = 0;		/* current readers position */
static int configLineNo = 0;	/* linenumber */
static char *configBuf, *configRBuf;	/* buffer for lines */
static char *configPath;		/* path to config file */
static char *configSection = NULL;	/* name of current section being parsed */
static int pushToken = LOCK_TOKEN;
static int eol_seen = 0;		/* private state to handle comments */
LexRec val;

/* 
 * xf86strToUL --
 *
 *  A portable, but restricted, version of strtoul().  It only understands
 *  hex, octal, and decimal.  But it's good enough for our needs.
 */
static unsigned int
xf86strToUL (char *str)
{
	int base = 10;
	char *p = str;
	unsigned int tot = 0;

	if (*p == '0')
	{
		p++;
		if ((*p == 'x') || (*p == 'X'))
		{
			p++;
			base = 16;
		}
		else
			base = 8;
	}
	while (*p)
	{
		if ((*p >= '0') && (*p <= ((base == 8) ? '7' : '9')))
		{
			tot = tot * base + (*p - '0');
		}
		else if ((base == 16) && (*p >= 'a') && (*p <= 'f'))
		{
			tot = tot * base + 10 + (*p - 'a');
		}
		else if ((base == 16) && (*p >= 'A') && (*p <= 'F'))
		{
			tot = tot * base + 10 + (*p - 'A');
		}
		else
		{
			return (tot);
		}
		p++;
	}
	return (tot);
}

/*
 * xf86getNextLine --
 *
 *  read from the configFile FILE stream until we encounter a new
 *  line; this is effectively just a big wrapper for fgets(3).
 *
 *  xf86getToken() assumes that we will read up to the next
 *  newline; we need to grow configBuf and configRBuf as needed to
 *  support that.
 */

static char*
xf86getNextLine(void)
{
	static int configBufLen = CONFIG_BUF_LEN;
	char *tmpConfigBuf, *tmpConfigRBuf;
	int c, i, pos = 0, eolFound = 0;
	char *ret = NULL;

	/*
	 * reallocate the string if it was grown last time (i.e., is no
	 * longer CONFIG_BUF_LEN); we malloc the new strings first, so
	 * that if either of the mallocs fail, we can fall back on the
	 * existing buffer allocations
	 */

	if (configBufLen != CONFIG_BUF_LEN) {

		tmpConfigBuf = malloc(CONFIG_BUF_LEN);
		tmpConfigRBuf = malloc(CONFIG_BUF_LEN);

		if (!tmpConfigBuf || !tmpConfigRBuf) {

			/*
			 * at least one of the mallocs failed; keep the old buffers
			 * and free any partial allocations
			 */

			free(tmpConfigBuf);
			free(tmpConfigRBuf);

		} else {

			/*
			 * malloc succeeded; free the old buffers and use the new
			 * buffers
			 */

			configBufLen = CONFIG_BUF_LEN;

			free(configBuf);
			free(configRBuf);

			configBuf = tmpConfigBuf;
			configRBuf = tmpConfigRBuf;
		}
	}

	/* read in another block of chars */

	do {
		ret = fgets(configBuf + pos, configBufLen - pos - 1, configFile);

		if (!ret) break;

		/* search for EOL in the new block of chars */

		for (i = pos; i < (configBufLen - 1); i++) {
			c = configBuf[i];

			if (c == '\0') break;

			if ((c == '\n') || (c == '\r')) {
				eolFound = 1;
				break;
			}
		}

		/*
		 * if we didn't find EOL, then grow the string and
		 * read in more
		 */

		if (!eolFound) {

			tmpConfigBuf = realloc(configBuf, configBufLen + CONFIG_BUF_LEN);
			tmpConfigRBuf = realloc(configRBuf, configBufLen + CONFIG_BUF_LEN);

			if (!tmpConfigBuf || !tmpConfigRBuf) {

				/*
				 * at least one of the reallocations failed; use the
				 * new allocation that succeeded, but we have to
				 * fallback to the previous configBufLen size and use
				 * the string we have, even though we don't have an
				 * EOL
				 */

				if (tmpConfigBuf) configBuf = tmpConfigBuf;
				if (tmpConfigRBuf) configRBuf = tmpConfigRBuf;

				break;

			} else {

				/* reallocation succeeded */

				configBuf = tmpConfigBuf;
				configRBuf = tmpConfigRBuf;
				pos = i;
				configBufLen += CONFIG_BUF_LEN;
			}
		}

	} while (!eolFound);

	return (ret);
}

/* 
 * xf86getToken --
 *      Read next Token from the config file. Handle the global variable
 *      pushToken.
 */
int
xf86getToken (xf86ConfigSymTabRec * tab)
{
	int c, i;

	/* 
	 * First check whether pushToken has a different value than LOCK_TOKEN.
	 * In this case rBuf[] contains a valid STRING/TOKEN/NUMBER. But in the
	 * oth * case the next token must be read from the input.
	 */
	if (pushToken == EOF_TOKEN)
		return (EOF_TOKEN);
	else if (pushToken == LOCK_TOKEN)
	{
		/*
		 * eol_seen is only set for the first token after a newline.
		 */
		eol_seen = 0;

		c = configBuf[configPos];

		/* 
		 * Get start of next Token. EOF is handled,
		 * whitespaces are skipped. 
		 */

again:
		if (!c)
		{
			char *ret;
			if (configFile)
				ret = xf86getNextLine();
			else {
				if (builtinConfig[builtinIndex] == NULL)
					ret = NULL;
				else {
					ret = strncpy(configBuf, builtinConfig[builtinIndex],
							CONFIG_BUF_LEN);
					builtinIndex++;
				}
			}
			if (ret == NULL)
			{
				return (pushToken = EOF_TOKEN);
			}
			configLineNo++;
			configPos = 0;
			eol_seen = 1;
		}

		i = 0;
		for (;;) {
			c = configBuf[configPos++];
			configRBuf[i++] = c;
			switch (c) {
				case ' ':
				case '\t':
				case '\r':
					continue;
				case '\n':
					i = 0;
					continue;
			}
			break;
		}
		if (c == '\0')
			goto again;

		if (c == '#')
		{
			do
			{
				configRBuf[i++] = (c = configBuf[configPos++]);
			}
			while ((c != '\n') && (c != '\r') && (c != '\0'));
			configRBuf[i] = '\0';
			/* XXX no private copy.
			 * Use xf86addComment when setting a comment.
			 */
			val.str = configRBuf;
			return (COMMENT);
		}

		/* GJA -- handle '-' and ','  * Be careful: "-hsync" is a keyword. */
		else if ((c == ',') && !isalpha (configBuf[configPos]))
		{
			return COMMA;
		}
		else if ((c == '-') && !isalpha (configBuf[configPos]))
		{
			return DASH;
		}

		/* 
		 * Numbers are returned immediately ...
		 */
		if (isdigit (c))
		{
			int base;

			if (c == '0')
				if ((configBuf[configPos] == 'x') ||
					(configBuf[configPos] == 'X'))
                                {
					base = 16;
                                        val.numType = PARSE_HEX;
                                }
				else
                                {
					base = 8;
                                        val.numType = PARSE_OCTAL;
                                }
			else
                        {
				base = 10;
                                val.numType = PARSE_DECIMAL;
                        }

			configRBuf[0] = c;
			i = 1;
			while (isdigit (c = configBuf[configPos++]) ||
				   (c == '.') || (c == 'x') || (c == 'X') ||
				   ((base == 16) && (((c >= 'a') && (c <= 'f')) ||
									 ((c >= 'A') && (c <= 'F')))))
				configRBuf[i++] = c;
			configPos--;		/* GJA -- one too far */
			configRBuf[i] = '\0';
			val.num = xf86strToUL (configRBuf);
			val.realnum = atof (configRBuf);
			return (NUMBER);
		}

		/* 
		 * All Strings START with a \" ...
		 */
		else if (c == '\"')
		{
			i = -1;
			do
			{
				configRBuf[++i] = (c = configBuf[configPos++]);
			}
			while ((c != '\"') && (c != '\n') && (c != '\r') && (c != '\0'));
			configRBuf[i] = '\0';
			val.str = malloc (strlen (configRBuf) + 1);
			strcpy (val.str, configRBuf);	/* private copy ! */
			return (STRING);
		}

		/* 
		 * ... and now we MUST have a valid token.  The search is
		 * handled later along with the pushed tokens.
		 */
		else
		{
			configRBuf[0] = c;
			i = 0;
			do
			{
				configRBuf[++i] = (c = configBuf[configPos++]);
			}
			while ((c != ' ') && (c != '\t') && (c != '\n') && (c != '\r') && (c != '\0') && (c != '#'));
			--configPos;
			configRBuf[i] = '\0';
			i = 0;
		}

	}
	else
	{

		/* 
		 * Here we deal with pushed tokens. Reinitialize pushToken again. If
		 * the pushed token was NUMBER || STRING return them again ...
		 */
		int temp = pushToken;
		pushToken = LOCK_TOKEN;

		if (temp == COMMA || temp == DASH)
			return (temp);
		if (temp == NUMBER || temp == STRING)
			return (temp);
	}

	/* 
	 * Joop, at last we have to lookup the token ...
	 */
	if (tab)
	{
		i = 0;
		while (tab[i].token != -1)
			if (xf86nameCompare (configRBuf, tab[i].name) == 0)
				return (tab[i].token);
			else
				i++;
	}

	return (ERROR_TOKEN);		/* Error catcher */
}

int
xf86getSubToken (char **comment)
{
	int token;

	for (;;) {
		token = xf86getToken(NULL);
		if (token == COMMENT) {
			if (comment)
				*comment = xf86addComment(*comment, val.str);
		}
		else
			return (token);
	}
	/*NOTREACHED*/
}

int
xf86getSubTokenWithTab (char **comment, xf86ConfigSymTabRec *tab)
{
	int token;

	for (;;) {
		token = xf86getToken(tab);
		if (token == COMMENT) {
			if (comment)
				*comment = xf86addComment(*comment, val.str);
		}
		else
			return (token);
	}
	/*NOTREACHED*/
}

void
xf86unGetToken (int token)
{
	pushToken = token;
}

char *
xf86tokenString (void)
{
	return configRBuf;
}

int
xf86pathIsAbsolute(const char *path)
{
	if (path && path[0] == '/')
		return 1;
	return 0;
}

/* A path is "safe" if it is relative and if it contains no ".." elements. */
int
xf86pathIsSafe(const char *path)
{
	if (xf86pathIsAbsolute(path))
		return 0;

	/* Compare with ".." */
	if (!strcmp(path, ".."))
		return 0;

	/* Look for leading "../" */
	if (!strncmp(path, "../", 3))
		return 0;

	/* Look for trailing "/.." */
	if ((strlen(path) > 3) && !strcmp(path + strlen(path) - 3, "/.."))
		return 0;

	/* Look for "/../" */
	if (strstr(path, "/../"))
		return 0;

	return 1;
}

/*
 * This function substitutes the following escape sequences:
 *
 *    %A    cmdline argument as an absolute path (must be absolute to match)
 *    %R    cmdline argument as a relative path
 *    %S    cmdline argument as a "safe" path (relative, and no ".." elements)
 *    %X    default config file name ("xorg.conf")
 *    %H    hostname
 *    %E    config file environment ($XORGCONFIG) as an absolute path
 *    %F    config file environment ($XORGCONFIG) as a relative path
 *    %G    config file environment ($XORGCONFIG) as a safe path
 *    %P    projroot
 *    %M    major version number
 *    %%    %
 */

#ifndef XCONFIGFILE
#define XCONFIGFILE	"xorg.conf"
#endif
#ifndef PROJECTROOT
#define PROJECTROOT	"/usr/X11R6"
#endif
#ifndef XCONFENV
#define XCONFENV	"XORGCONFIG"
#endif
#define XFREE86CFGFILE "XF86Config"
#ifndef XF86_VERSION_MAJOR
#ifdef XVERSION
#if XVERSION > 40000000
#define XF86_VERSION_MAJOR	(XVERSION / 10000000)
#else
#define XF86_VERSION_MAJOR	(XVERSION / 1000)
#endif
#else
#define XF86_VERSION_MAJOR	4
#endif
#endif

#define BAIL_OUT		do {									\
							free(result);				\
							return NULL;						\
						} while (0)

#define CHECK_LENGTH	do {									\
							if (l > PATH_MAX) {					\
								BAIL_OUT;						\
							}									\
						} while (0)

#define APPEND_STR(s)	do {									\
							if (strlen(s) + l > PATH_MAX) {		\
								BAIL_OUT;						\
							} else {							\
								strcpy(result + l, s);			\
								l += strlen(s);					\
							}									\
						} while (0)

static char *
DoSubstitution(const char *template, const char *cmdline, const char *projroot,
				int *cmdlineUsed, int *envUsed, char *XConfigFile)
{
	char *result;
	int i, l;
	static const char *env = NULL;
	static char *hostname = NULL;
	static char majorvers[3] = "";

	if (!template)
		return NULL;

	if (cmdlineUsed)
		*cmdlineUsed = 0;
	if (envUsed)
		*envUsed = 0;

	result = malloc(PATH_MAX + 1);
	l = 0;
	for (i = 0; template[i]; i++) {
		if (template[i] != '%') {
			result[l++] = template[i];
			CHECK_LENGTH;
		} else {
			switch (template[++i]) {
			case 'A':
				if (cmdline && xf86pathIsAbsolute(cmdline)) {
					APPEND_STR(cmdline);
					if (cmdlineUsed)
						*cmdlineUsed = 1;
				} else
					BAIL_OUT;
				break;
			case 'R':
				if (cmdline && !xf86pathIsAbsolute(cmdline)) {
					APPEND_STR(cmdline);
					if (cmdlineUsed)
						*cmdlineUsed = 1;
				} else 
					BAIL_OUT;
				break;
			case 'S':
				if (cmdline && xf86pathIsSafe(cmdline)) {
					APPEND_STR(cmdline);
					if (cmdlineUsed)
						*cmdlineUsed = 1;
				} else 
					BAIL_OUT;
				break;
			case 'X':
				APPEND_STR(XConfigFile);
				break;
			case 'H':
				if (!hostname) {
					if ((hostname = malloc(MAXHOSTNAMELEN + 1))) {
						if (gethostname(hostname, MAXHOSTNAMELEN) == 0) {
							hostname[MAXHOSTNAMELEN] = '\0';
						} else {
							free(hostname);
							hostname = NULL;
						}
					}
				}
				if (hostname)
					APPEND_STR(hostname);
				break;
			case 'E':
				if (!env)
					env = getenv(XCONFENV);
				if (env && xf86pathIsAbsolute(env)) {
					APPEND_STR(env);
					if (envUsed)
						*envUsed = 1;
				} else
					BAIL_OUT;
				break;
			case 'F':
				if (!env)
					env = getenv(XCONFENV);
				if (env && !xf86pathIsAbsolute(env)) {
					APPEND_STR(env);
					if (envUsed)
						*envUsed = 1;
				} else
					BAIL_OUT;
				break;
			case 'G':
				if (!env)
					env = getenv(XCONFENV);
				if (env && xf86pathIsSafe(env)) {
					APPEND_STR(env);
					if (envUsed)
						*envUsed = 1;
				} else
					BAIL_OUT;
				break;
			case 'P':
				if (projroot && xf86pathIsAbsolute(projroot))
					APPEND_STR(projroot);
				else
					BAIL_OUT;
				break;
			case 'M':
				if (!majorvers[0]) {
					if (XF86_VERSION_MAJOR < 0 || XF86_VERSION_MAJOR > 99) {
						fprintf(stderr, "XF86_VERSION_MAJOR is out of range\n");
						BAIL_OUT;
					} else
						sprintf(majorvers, "%d", XF86_VERSION_MAJOR);
				}
				APPEND_STR(majorvers);
				break;
			case '%':
				result[l++] = '%';
				CHECK_LENGTH;
				break;
			default:
				fprintf(stderr, "invalid escape %%%c found in path template\n",
						template[i]);
				BAIL_OUT;
				break;
			}
		}
	}
#ifdef DEBUG
	fprintf(stderr, "Converted `%s' to `%s'\n", template, result);
#endif
	return result;
}

/* 
 * xf86openConfigFile --
 *
 * This function take a config file search path (optional), a command-line
 * specified file name (optional) and the ProjectRoot path (optional) and
 * locates and opens a config file based on that information.  If a
 * command-line file name is specified, then this function fails if none
 * of the located files.
 *
 * The return value is a pointer to the actual name of the file that was
 * opened.  When no file is found, the return value is NULL.
 *
 * The escape sequences allowed in the search path are defined above.
 *  
 */

#ifndef DEFAULT_CONF_PATH
#define DEFAULT_CONF_PATH	"/etc/X11/%S," \
							"%P/etc/X11/%S," \
							"/etc/X11/%G," \
							"%P/etc/X11/%G," \
							"/etc/X11/%X-%M," \
							"/etc/X11/%X," \
							"/etc/%X," \
							"%P/etc/X11/%X.%H," \
							"%P/etc/X11/%X-%M," \
							"%P/etc/X11/%X," \
							"%P/lib/X11/%X.%H," \
							"%P/lib/X11/%X-%M," \
							"%P/lib/X11/%X"
#endif

const char *
xf86openConfigFile(const char *path, const char *cmdline, const char *projroot)
{
	char *pathcopy;
	const char *template;
	int cmdlineUsed = 0;

	configFile = NULL;
	configPos = 0;		/* current readers position */
	configLineNo = 0;	/* linenumber */
	pushToken = LOCK_TOKEN;

	if (!path || !path[0])
		path = DEFAULT_CONF_PATH;
	pathcopy = malloc(strlen(path) + 1);
	strcpy(pathcopy, path);
	if (!projroot || !projroot[0])
		projroot = PROJECTROOT;

	template = strtok(pathcopy, ",");

	/* First, search for a config file. */
	while (template && !configFile) {
		if ((configPath = DoSubstitution(template, cmdline, projroot,
						 &cmdlineUsed, NULL,
						 XCONFIGFILE))) {
			if ((configFile = fopen(configPath, "r")) != 0) {
				if (cmdline && !cmdlineUsed) {
					fclose(configFile);
					configFile = NULL;
				}
			}
		}
		if (configPath && !configFile) {
			free(configPath);
			configPath = NULL;
		}
		template = strtok(NULL, ",");
	}
	
	/* Then search for fallback */
	if (!configFile) {
	    strcpy(pathcopy, path);
	    template = strtok(pathcopy, ",");

	    while (template && !configFile) {
		if ((configPath = DoSubstitution(template, cmdline, projroot,
						 &cmdlineUsed, NULL,
						 XFREE86CFGFILE))) {
		    if ((configFile = fopen(configPath, "r")) != 0) {
			if (cmdline && !cmdlineUsed) {
			    fclose(configFile);
			    configFile = NULL;
			}
		    }
		}
		if (configPath && !configFile) {
		    free(configPath);
		    configPath = NULL;
		}
		template = strtok(NULL, ",");
	    }
	}
	
	free(pathcopy);
	if (!configFile) {

		return NULL;
	}

	configBuf = malloc (CONFIG_BUF_LEN);
	configRBuf = malloc (CONFIG_BUF_LEN);
	configBuf[0] = '\0';		/* sanity ... */

	return configPath;
}

void
xf86closeConfigFile (void)
{
	free (configPath);
	configPath = NULL;
	free (configRBuf);
	configRBuf = NULL;
	free (configBuf);
	configBuf = NULL;

	if (configFile) {
		fclose (configFile);
		configFile = NULL;
	} else {
		builtinConfig = NULL;
		builtinIndex = 0;
	}
}

void
xf86setBuiltinConfig(const char *config[])
{
	builtinConfig = config;
	configPath = strdup("<builtin configuration>");
	configBuf = malloc (CONFIG_BUF_LEN);
	configRBuf = malloc (CONFIG_BUF_LEN);
	configBuf[0] = '\0';		/* sanity ... */

}

void
xf86parseError (char *format,...)
{
	va_list ap;

	ErrorF ("Parse error on line %d of section %s in file %s\n\t",
		 configLineNo, configSection, configPath);
	va_start (ap, format);
	VErrorF (format, ap);
	va_end (ap);

	ErrorF ("\n");
}

void
xf86validationError (char *format,...)
{
	va_list ap;

	ErrorF ("Data incomplete in file %s\n\t", configPath);
	va_start (ap, format);
	VErrorF (format, ap);
	va_end (ap);

	ErrorF ("\n");
}

void
xf86setSection (char *section)
{
	if (configSection)
		free(configSection);
	configSection = malloc(strlen (section) + 1);
	strcpy (configSection, section);
}

/* 
 * xf86getToken --
 *  Lookup a string if it is actually a token in disguise.
 */
int
xf86getStringToken (xf86ConfigSymTabRec * tab)
{
	return StringToToken (val.str, tab);
}

static int
StringToToken (char *str, xf86ConfigSymTabRec * tab)
{
	int i;

	for (i = 0; tab[i].token != -1; i++)
	{
		if (!xf86nameCompare (tab[i].name, str))
			return tab[i].token;
	}
	return (ERROR_TOKEN);
}


/* 
 * Compare two names.  The characters '_', ' ', and '\t' are ignored
 * in the comparison.
 */
int
xf86nameCompare (const char *s1, const char *s2)
{
	char c1, c2;

	if (!s1 || *s1 == 0) {
		if (!s2 || *s2 == 0)
			return (0);
		else
			return (1);
		}

	while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
		s1++;
	while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
		s2++;
	c1 = (isupper (*s1) ? tolower (*s1) : *s1);
	c2 = (isupper (*s2) ? tolower (*s2) : *s2);
	while (c1 == c2)
	{
		if (c1 == '\0')
			return (0);
		s1++;
		s2++;
		while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
			s1++;
		while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
			s2++;
		c1 = (isupper (*s1) ? tolower (*s1) : *s1);
		c2 = (isupper (*s2) ? tolower (*s2) : *s2);
	}
	return (c1 - c2);
}

char *
xf86addComment(char *cur, char *add)
{
	char *str;
	int len, curlen, iscomment, hasnewline = 0, endnewline;

	if (add == NULL || add[0] == '\0')
		return (cur);

	if (cur) {
		curlen = strlen(cur);
		if (curlen)
		    hasnewline = cur[curlen - 1] == '\n';
		eol_seen = 0;
	}
	else
		curlen = 0;

	str = add;
	iscomment = 0;
	while (*str) {
	    if (*str != ' ' && *str != '\t')
		break;
	    ++str;
	}
	iscomment = (*str == '#');

	len = strlen(add);
	endnewline = add[len - 1] == '\n';
	len +=  1 + iscomment + (!hasnewline) + (!endnewline) + eol_seen;

	if ((str = realloc(cur, len + curlen)) == NULL)
		return (cur);

	cur = str;

	if (eol_seen || (curlen && !hasnewline))
		cur[curlen++] = '\n';
	if (!iscomment)
		cur[curlen++] = '#';
	strcpy(cur + curlen, add);
	if (!endnewline)
		strcat(cur, "\n");

	return (cur);
}
