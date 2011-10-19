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

#ifdef __clang__
#pragma clang diagnostic ignored "-Wformat-nonliteral"
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
static const char *__crashreporter_info__ __attribute__((__used__)) = &__crashreporter_info_buff__[0];
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
#ifndef X_NONE_STRING
#define X_NONE_STRING                   ""
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
	if (asprintf(&logFileName, fname, display) == -1)
	    FatalError("Cannot allocate space for the log file name\n");

	if (backup && *backup) {
	    struct stat buf;

	    if (!stat(logFileName, &buf) && S_ISREG(buf.st_mode)) {
		char *suffix;
		char *oldLog;

		if ((asprintf(&suffix, backup, display) == -1) ||
		    (asprintf(&oldLog, "%s%s", logFileName, suffix) == -1))
		    FatalError("Cannot allocate space for the log file name\n");
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
LogClose(enum ExitCode error)
{
    if (logFile) {
	ErrorF("Server terminated %s (%d). Closing log file.\n",
		(error == EXIT_NO_ERROR) ? "successfully" : "with error", error);
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
	    if (len > bufferUnused) {
		bufferSize += 1024;
		bufferUnused += 1024;
		saveBuffer = realloc(saveBuffer, bufferSize);
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

/* Returns the Message Type string to prepend to a logging message, or NULL
 * if the message will be dropped due to insufficient verbosity. */
static const char *
LogMessageTypeVerbString(MessageType type, int verb)
{
    if (type == X_ERROR)
	verb = 0;

    if (logVerbosity < verb && logFileVerbosity < verb)
	return NULL;

    switch (type) {
    case X_PROBED:
	return X_PROBE_STRING;
    case X_CONFIG:
	return X_CONFIG_STRING;
    case X_DEFAULT:
	return X_DEFAULT_STRING;
    case X_CMDLINE:
	return X_CMDLINE_STRING;
    case X_NOTICE:
	return X_NOTICE_STRING;
    case X_ERROR:
	return X_ERROR_STRING;
    case X_WARNING:
	return X_WARNING_STRING;
    case X_INFO:
	return X_INFO_STRING;
    case X_NOT_IMPLEMENTED:
	return X_NOT_IMPLEMENTED_STRING;
    case X_UNKNOWN:
	return X_UNKNOWN_STRING;
    case X_NONE:
	return X_NONE_STRING;
    default:
	return X_UNKNOWN_STRING;
    }
}

void
LogVMessageVerb(MessageType type, int verb, const char *format, va_list args)
{
    const char *type_str;
    char tmpFormat[1024];
    char *new_format;

    type_str = LogMessageTypeVerbString(type, verb);
    if (!type_str)
	return;

    /* if type_str is not "", prepend it and ' ', to format */
    if (type_str[0] == '\0')
        new_format = format;
    else {
        new_format = tmpFormat;
        snprintf(tmpFormat, sizeof(tmpFormat), "%s %s", type_str, format);
    }

    LogVWrite(verb, new_format, args);
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
LogVHdrMessageVerb(MessageType type, int verb, const char *msg_format,
		   va_list msg_args, const char *hdr_format, va_list hdr_args)
{
    const char *type_str;
    char tmpFormat[1024];
    char *tmpFormat_end = &tmpFormat[sizeof(tmpFormat)];
    char *p;
    int left;

    type_str = LogMessageTypeVerbString(type, verb);
    if (!type_str)
	return;

    /* if type_str != "", copy it and ' ' to tmpFormat; set p after ' ' */
    p = tmpFormat;
    if (type_str[0] != '\0')
	p += snprintf(tmpFormat, sizeof(tmpFormat), "%s ", type_str);

    /* append as much of hdr as fits after type_str (if there was one) */
    left = tmpFormat_end - p;
    if (left > 1)
	p += vsnprintf(p, left, hdr_format, hdr_args);

    /* append as much of msg_format as will fit after hdr */
    left = tmpFormat_end - p;
    if (left > 1)
	snprintf(p, left, "%s", msg_format);

    LogVWrite(verb, tmpFormat, msg_args);
}

void
LogHdrMessageVerb(MessageType type, int verb, const char *msg_format,
		  va_list msg_args, const char *hdr_format, ...)
{
    va_list hdr_args;

    va_start(hdr_args, hdr_format);
    LogVHdrMessageVerb(type, verb, msg_format, msg_args, hdr_format, hdr_args);
    va_end(hdr_args);
}

void
LogHdrMessage(MessageType type, const char *msg_format, va_list msg_args,
	      const char *hdr_format, ...)
{
    va_list hdr_args;

    va_start(hdr_args, hdr_format);
    LogVHdrMessageVerb(type, 1, msg_format, msg_args, hdr_format, hdr_args);
    va_end(hdr_args);
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
    AbortDDX(EXIT_ERR_ABORT);
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
    {
        va_list args2;
        va_copy(args2, args);
        (void)vsnprintf(__crashreporter_info_buff__, sizeof(__crashreporter_info_buff__), f, args2);
        va_end(args2);
    }
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

