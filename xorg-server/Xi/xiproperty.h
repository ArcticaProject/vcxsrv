/*
 * Copyright © 2008 Peter Hutterer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Peter Hutterer
 */

#ifndef XIPROPERTY_C
#define XIPROPERTY_C

int ProcXListDeviceProperties     (ClientPtr client);
int ProcXChangeDeviceProperty     (ClientPtr client);
int ProcXDeleteDeviceProperty     (ClientPtr client);
int ProcXGetDeviceProperty        (ClientPtr client);

/* request swapping */
int SProcXListDeviceProperties    (ClientPtr client);
int SProcXChangeDeviceProperty    (ClientPtr client);
int SProcXDeleteDeviceProperty    (ClientPtr client);
int SProcXGetDeviceProperty       (ClientPtr client);

/* reply swapping */
void SRepXListDeviceProperties(ClientPtr client, int size,
                               xListDevicePropertiesReply *rep);
void SRepXGetDeviceProperty(ClientPtr client, int size,
                            xGetDevicePropertyReply *rep);

void XIInitKnownProperties(void);

#endif /* XIPROPERTY_C */
