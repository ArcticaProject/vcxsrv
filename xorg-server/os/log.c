/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
Copyright 1994 Quarterdeck Office Systems.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital and
Quarterdeck not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

DIGITAL AND QUARTERDECK DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

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


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xos.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>	/* for malloc() */
#include <errno.h>

#include "input.h"
#include "site.h"
#include "opaque.h"

#ifdef WIN32
#include <process.h>
#define getpid(x) _getpid(x)
#endif

#ifdef XF86BIGFONT
#include "xf86bigfontsrv.h"
#endif

#ifdef DDXOSVERRORF
void (*OsVendorVErrorFProc)(const char *, va_list args) = NULL;
#endif

static FILE *logFile = NULL;
static Bool logFlush = FALSE;
static Bool logSync = FALSE;
static int logVerbosity = DEFAULT_LOG_VERBOSITY;
static int logFileVerbosity = DEFAULT_LOG_FILE_VERBOSITY;

/* Buffer to information logged before the log file is opened. */
static char *saveBuffer = NULL;
static int bufferSize = 0, bufferUnused = 0, bufferPos = 0;
static Bool needBuffer = TRUE;

#ifdef __APPLE__
#include <AvailabilityMacros.h>

static char __crashreporter_info_buff__[4096] = {0};
static const char *__crashreporter_info__ = &__crashreporter_info_buff__[0];
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
// This is actually a toolchain requirement, but I'm not sure the correct check,        
// but it should be fine to just only include it for Leopard and later.  This line
// just tells the linker to never strip this symbol (such as for space optimization)
asm (".desc ___crashreporter_info__, 0x10");
#endif
#endif

/* Prefix strings for log messages. */
#ifndef X_UNKNOWN_STRING
#define X_UNKNOWN_STRING		"(\?\?)"
#endif
#ifndef X_PROBE_STRING
#define X_PROBE_STRING			"(--)"
#endif
#ifndef X_CONFIG_STRING
#define X_CONFIG_STRING			"(**)"
#endif
#ifndef X_DEFAULT_STRING
#define X_DEFAULT_STRING		"(==)"
#endif
#ifndef X_CMDLINE_STRING
#define X_CMDLINE_STRING		"(++)"
#endif
#ifndef X_NOTICE_STRING
#define X_NOTICE_STRING			"(!!)"
#endif
#ifndef X_ERROR_STRING
#define X_ERROR_STRING			"(EE)"
#endif
#ifndef X_WARNING_STRING
#define X_WARNING_STRING		"(WW)"
#endif
#ifndef X_INFO_STRING
#define X_INFO_STRING			"(II)"
#endif
#ifndef X_NOT_IMPLEMENTED_STRING
#define X_NOT_IMPLEMENTED_STRING	"(NI)"
#endif

/*
 * LogInit is called to start logging to a file.  It is also called (with
 * NULL arguments) when logging to a file is not wanted.  It must always be
 * called, otherwise log messages will continue to accumulate in a buffer.
 *
 * %s, if present in the fname or backup strings, is expanded to the display
 * string.
 */

const char *
LogInit(const char *fname, const char *backup)
{
    char *logFileName = NULL;

    if (fname && *fname) {
	/* malloc() can't be used yet. */
	logFileName = malloc(strlen(fname) + strlen(display) + 1);
	if (!logFileName)
	    FatalError("Cannot allocate space for the log file name\n");
	sprintf(logFileName, fname, display);

	if (backup && *backup) {
	    struct stat buf;

	    if (!stat(logFileName, &buf) && S_ISREG(buf.st_mode)) {
		char *suffix;
		char *oldLog;

		oldLog = malloc(strlen(logFileName) + strlen(backup) +
				strlen(display) + 1);
		suffix = malloc(strlen(backup) + strlen(display) + 1);
		if (!oldLog || !suffix)
		    FatalError("Cannot allocate space for the log file name\n");
		sprintf(suffix, backup, display);
		sprintf(oldLog, "%s%s", logFileName, suffix);
		free(suffix);
		if (rename(logFileName, oldLog) == -1) {
		    FatalError("Cannot move old log file \"%s\" to \"%s\"\n",
			       logFileName, oldLog);
		}
		free(oldLog);
	    }
	}
	if ((logFile = fopen(logFileName, "w")) == NULL)
	    FatalError("Cannot open log file \"%s\"\n", logFileName);
	setvbuf(logFile, NULL, _IONBF, 0);

	/* Flush saved log information. */
	if (saveBuffer && bufferSize > 0) {
	    fwrite(saveBuffer, bufferPos, 1, logFile);
	    fflush(logFile);
#ifndef WIN32
	    fsync(fileno(logFile));
#endif
	}
    }

    /*
     * Unconditionally free the buffer, and flag that the buffer is no longer
     * needed.
     */
    if (saveBuffer && bufferSize > 0) {
	free(saveBuffer);	/* Must be free(), not free() */
	saveBuffer = NULL;
	bufferSize = 0;
    }
    needBuffer = FALSE;

    return logFileName;
}

void
LogClose(void)
{
    if (logFile) {
	fclose(logFile);
	logFile = NULL;
    }
}

Bool
LogSetParameter(LogParameter param, int value)
{
    switch (param) {
    case XLOG_FLUSH:
	logFlush = value ? TRUE : FALSE;
	return TRUE;
    case XLOG_SYNC:
	logSync = value ? TRUE : FALSE;
	return TRUE;
    case XLOG_VERBOSITY:
	logVerbosity = value;
	return TRUE;
    case XLOG_FILE_VERBOSITY:
	logFileVerbosity = value;
	return TRUE;
    default:
	return FALSE;
    }
}

/* This function does the actual log message writes. */

void
LogVWrite(int verb, const char *f, va_list args)
{
    static char tmpBuffer[1024];
    int len = 0;
    static Bool newline = TRUE;

    if (newline) {
	sprintf(tmpBuffer, "[%10.3f] ", GetTimeInMillis() / 1000.0);
	len = strlen(tmpBuffer);
	if (logFile)
	    fwrite(tmpBuffer, len, 1, logFile);
    }

    /*
     * Since a va_list can only be processed once, write the string to a
     * buffer, and then write the buffer out to the appropriate output
     * stream(s).
     */
    if (verb < 0 || logFileVerbosity >= verb || logVerbosity >= verb) {
	vsnprintf(tmpBuffer, sizeof(tmpBuffer), f, args);
	len = strlen(tmpBuffer);
    }
    newline = (tmpBuffer[len-1] == '\n');
    if ((verb < 0 || logVerbosity >= verb) && len > 0)
	fwrite(tmpBuffer, len, 1, stderr);
    if ((verb < 0 || logFileVerbosity >= verb) && len > 0) {
	if (logFile) {
	    fwrite(tmpBuffer, len, 1, logFile);
	    if (logFlush) {
		fflush(logFile);
#ifndef WIN32
		if (logSync)
		    fsync(fileno(logFile));
#endif
	    }
	} else if (needBuffer) {
	    /*
	     * Note, this code is used before OsInit() has been called, so
	     * malloc() and friends can't be used.
	     */
	    if (len > bufferUnused) {
		bufferSize += 1024;
		bufferUnused += 1024;
		if (saveBuffer)
		    saveBuffer = realloc(saveBuffer, bufferSize);
		else
		    saveBuffer = malloc(bufferSize);
		if (!saveBuffer)
		    FatalError("realloc() failed while saving log messages\n");
	    }
	    bufferUnused -= len;
	    memcpy(saveBuffer + bufferPos, tmpBuffer, len);
	    bufferPos += len;
	}
    }
}

void
LogWrite(int verb, const char *f, ...)
{
    va_list args;

    va_start(args, f);
    LogVWrite(verb, f, args);
    va_end(args);
}

void
LogVMessageVerb(MessageType type, int verb, const char *format, va_list args)
{
    const char *s  = X_UNKNOWN_STRING;
    char tmpBuf[1024];

    /* Ignore verbosity for X_ERROR */
    if (logVerbosity >= verb || logFileVerbosity >= verb || type == X_ERROR) {
	switch (type) {
	case X_PROBED:
	    s = X_PROBE_STRING;
	    break;
	case X_CONFIG:
	    s = X_CONFIG_STRING;
	    break;
	case X_DEFAULT:
	    s = X_DEFAULT_STRING;
	    break;
	case X_CMDLINE:
	    s = X_CMDLINE_STRING;
	    break;
	case X_NOTICE:
	    s = X_NOTICE_STRING;
	    break;
	case X_ERROR:
	    s = X_ERROR_STRING;
	    if (verb > 0)
		verb = 0;
	    break;
	case X_WARNING:
	    s = X_WARNING_STRING;
	    break;
	case X_INFO:
	    s = X_INFO_STRING;
	    break;
	case X_NOT_IMPLEMENTED:
	    s = X_NOT_IMPLEMENTED_STRING;
	    break;
	case X_UNKNOWN:
	    s = X_UNKNOWN_STRING;
	    break;
	case X_NONE:
	    s = NULL;
	    break;
	}

        /* if s is not NULL we need a space before format */
        snprintf(tmpBuf, sizeof(tmpBuf), "%s%s%s", s ? s : "",
                                                   s ? " " : "",
                                                   format);
        LogVWrite(verb, tmpBuf, args);
    }
}

/* Log message with verbosity level specified. */
void
LogMessageVerb(MessageType type, int verb, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    LogVMessageVerb(type, verb, format, ap);
    va_end(ap);
}

/* Log a message with the standard verbosity level of 1. */
void
LogMessage(MessageType type, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    LogVMessageVerb(type, 1, format, ap);
    va_end(ap);
}

void
AbortServer(void) _X_NORETURN;

void
AbortServer(void)
{
#ifdef XF86BIGFONT
    XF86BigfontCleanup();
#endif
    CloseWellKnownConnections();
    OsCleanup(TRUE);
    CloseDownDevices();
    AbortDDX();
    fflush(stderr);
    if (CoreDump)
	OsAbort();
    exit (1);
}

#define AUDIT_PREFIX "AUDIT: %s: %ld: "
#ifndef AUDIT_TIMEOUT
#define AUDIT_TIMEOUT ((CARD32)(120 * 1000)) /* 2 mn */
#endif

static int nrepeat = 0;
static int oldlen = -1;
static OsTimerPtr auditTimer = NULL;

void
FreeAuditTimer(void)
{
    if (auditTimer != NULL) {
	/* Force output of pending messages */
	TimerForce(auditTimer);
	TimerFree(auditTimer);
	auditTimer = NULL;
    }
}

static char *
AuditPrefix(void)
{
    time_t tm;
    char *autime, *s;
    char *tmpBuf;
    int len;

    time(&tm);
    autime = ctime(&tm);
    if ((s = strchr(autime, '\n')))
	*s = '\0';
    len = strlen(AUDIT_PREFIX) + strlen(autime) + 10 + 1;
    tmpBuf = malloc(len);
    if (!tmpBuf)
	return NULL;
    snprintf(tmpBuf, len, AUDIT_PREFIX, autime, (unsigned long)getpid());
    return tmpBuf;
}

void
AuditF(const char * f, ...)
{
    va_list args;

    va_start(args, f);

    VAuditF(f, args);
    va_end(args);
}

static CARD32
AuditFlush(OsTimerPtr timer, CARD32 now, pointer arg)
{
    char *prefix;

    if (nrepeat > 0) {
	prefix = AuditPrefix();
	ErrorF("%slast message repeated %d times\n",
	       prefix != NULL ? prefix : "", nrepeat);
	nrepeat = 0;
	if (prefix != NULL)
	    free(prefix);
	return AUDIT_TIMEOUT;
    } else {
	/* if the timer expires without anything to print, flush the message */
	oldlen = -1;
	return 0;
    }
}

void
VAuditF(const char *f, va_list args)
{
    char *prefix;
    char buf[1024];
    int len;
    static char oldbuf[1024];

    prefix = AuditPrefix();
    len = vsnprintf(buf, sizeof(buf), f, args);

    if (len == oldlen && strcmp(buf, oldbuf) == 0) {
	/* Message already seen */
	nrepeat++;
    } else {
	/* new message */
	if (auditTimer != NULL)
	    TimerForce(auditTimer);
	ErrorF("%s%s", prefix != NULL ? prefix : "", buf);
	strlcpy(oldbuf, buf, sizeof(oldbuf));
	oldlen = len;
	nrepeat = 0;
	auditTimer = TimerSet(auditTimer, 0, AUDIT_TIMEOUT, AuditFlush, NULL);
    }
    if (prefix != NULL)
	free(prefix);
}

void
FatalError(const char *f, ...)
{
    va_list args;
    static Bool beenhere = FALSE;

    if (beenhere)
	ErrorF("\nFatalError re-entered, aborting\n");
    else
	ErrorF("\nFatal server error:\n");

    va_start(args, f);
#ifdef __APPLE__
    (void)vsnprintf(__crashreporter_info_buff__, sizeof(__crashreporter_info_buff__), f, args);
#endif
    VErrorF(f, args);
    va_end(args);
    ErrorF("\n");
    if (!beenhere)
	OsVendorFatalError();
    if (!beenhere) {
	beenhere = TRUE;
	AbortServer();
    } else
	OsAbort();
    /*NOTREACHED*/
}

void
VErrorF(const char *f, va_list args)
{
#ifdef DDXOSVERRORF
    if (OsVendorVErrorFProc)
	OsVendorVErrorFProc(f, args);
    else
	LogVWrite(-1, f, args);
#else
    LogVWrite(-1, f, args);
#endif
}

void
ErrorF(const char * f, ...)
{
    va_list args;

    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
}

/* A perror() workalike. */

void
Error(char *str)
{
    char *err = NULL;
    int saveErrno = errno;

    if (str) {
	err = malloc(strlen(strerror(saveErrno)) + strlen(str) + 2 + 1);
	if (!err)
	    return;
	sprintf(err, "%s: ", str);
	strcat(err, strerror(saveErrno));
	LogWrite(-1, "%s", err);
	free(err);
    } else
	LogWrite(-1, "%s", strerror(saveErrno));
}

void
LogPrintMarkers(void)
{
    /* Show what the message marker symbols mean. */
    LogWrite(0, "Markers: ");
    LogMessageVerb(X_PROBED, 0, "probed, ");
    LogMessageVerb(X_CONFIG, 0, "from config file, ");
    LogMessageVerb(X_DEFAULT, 0, "default setting,\n\t");
    LogMessageVerb(X_CMDLINE, 0, "from command line, ");
    LogMessageVerb(X_NOTICE, 0, "notice, ");
    LogMessageVerb(X_INFO, 0, "informational,\n\t");
    LogMessageVerb(X_WARNING, 0, "warning, ");
    LogMessageVerb(X_ERROR, 0, "error, ");
    LogMessageVerb(X_NOT_IMPLEMENTED, 0, "not implemented, ");
    LogMessageVerb(X_UNKNOWN, 0, "unknown.\n");
}

