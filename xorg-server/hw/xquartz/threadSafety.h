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

#ifndef _XQ_THREAD_SAFETY_H_
#define _XQ_THREAD_SAFETY_H_

#define DEBUG_THREADS 1

#include <pthread.h>

extern pthread_t APPKIT_THREAD_ID;
extern pthread_t SERVER_THREAD_ID;

/* Dump the call stack */
void spewCallStack(void);

/* Print message to ErrorF if we're in the wrong thread */
void _threadSafetyAssert(pthread_t tid, const char *file, const char *fun, int line);

/* Get a string that identifies our thread nicely */
const char *threadSafetyID(pthread_t tid);

#define threadSafetyAssert(tid) _threadSafetyAssert(tid, __FILE__, __FUNCTION__, __LINE__)

#ifdef DEBUG_THREADS
#define TA_APPKIT() threadSafetyAssert(APPKIT_THREAD_ID)
#define TA_SERVER() threadSafetyAssert(SERVER_THREAD_ID)
#else
#define TA_SERVER() 
#define TA_APPKIT() 
#endif

#endif _XQ_THREAD_SAFETY_H_
