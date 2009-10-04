/* Copyright 2005-2006 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */

/*
 * Xserver dtrace provider definition
 */
#ifdef __APPLE__
#define string char *
#define pid_t uint32_t
#define zoneid_t uint32_t
#else
#include <sys/types.h>
#endif

provider Xserver {
	/* reqType, data, length, client id, request buffer */
	probe request__start(string, uint8_t, uint16_t, int, void *);
	/* reqType, data, sequence, client id, result */
	probe request__done(string, uint8_t, uint32_t, int, int);
	/* client id, client fd */
	probe client__connect(int, int);
	/* client id, client address, client pid, client zone id */
	probe client__auth(int, string, pid_t, zoneid_t);
	/* client id */
	probe client__disconnect(int);
	/* resource id, resource type, value, resource type name */
	probe resource__alloc(uint32_t, uint32_t, void *, string);
	/* resource id, resource type, value, resource type name */
	probe resource__free(uint32_t, uint32_t, void *, string);
	/* client id, event type, event* */
	probe send__event(int, uint8_t, void *);
};

#pragma D attributes Unstable/Unstable/Common provider Xserver provider
#pragma D attributes Private/Private/Unknown  provider Xserver module
#pragma D attributes Private/Private/Unknown  provider Xserver function
#pragma D attributes Unstable/Unstable/Common provider Xserver name
#pragma D attributes Unstable/Unstable/Common provider Xserver args

