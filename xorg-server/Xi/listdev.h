/************************************************************

Copyright 1996 by Thomas E. Dickey <dickey@clark.net>

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the above listed
copyright holder(s) not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef LISTDEV_H
#define LISTDEV_H 1

#define VPC	20	/* Max # valuators per chunk */

int SProcXListInputDevices(ClientPtr	/* client */
    );

int ProcXListInputDevices(ClientPtr	/* client */
    );

void SRepXListInputDevices(ClientPtr /* client */ ,
			   int /* size */ ,
			   xListInputDevicesReply *	/* rep */
    );

void
CopySwapClasses(ClientPtr /* client */,
                DeviceIntPtr /* dev */,
                CARD8* /* num_classes */,
                char** /* classbuf */);

void
SizeDeviceInfo(DeviceIntPtr /* dev */,
               int* /* namesize */,
               int* /* size */);

#endif /* LISTDEV_H */
