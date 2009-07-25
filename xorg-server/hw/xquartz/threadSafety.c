/*
 * Copyright (C) 2008 Apple, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "threadSafety.h"
#include "os.h"

pthread_t APPKIT_THREAD_ID;
pthread_t SERVER_THREAD_ID;

#include <AvailabilityMacros.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
#include <execinfo.h>

void spewCallStack(void) {
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    
    for (i = 0; i < frames; ++i) {
        ErrorF("%s\n", strs[i]);
    }
    
    free(strs);
}
#else
void spewCallStack(void) {
	return;
}
#endif

void _threadSafetyAssert(pthread_t tid, const char *file, const char *fun, int line) {
    if(pthread_equal(pthread_self(), tid))
        return;
    
    /* NOOOO! */
    ErrorF("Thread Assertion Failed: self=%s, expected=%s\n%s:%s:%d\n",
           threadSafetyID(pthread_self()), threadSafetyID(tid),
           file, fun, line);
    spewCallStack();
}

const char *threadSafetyID(pthread_t tid) {
    if(pthread_equal(tid, APPKIT_THREAD_ID)) {
        return "Appkit Thread";
    } else if(pthread_equal(tid, SERVER_THREAD_ID)) {
        return "Xserver Thread";
    } else {        
        return "Unknown Thread";
    }
}
