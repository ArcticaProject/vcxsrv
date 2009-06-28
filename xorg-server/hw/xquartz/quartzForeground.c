/* foreground.c - Push the current process into the foreground.

 This is in a separate file because of Quartz/X type conflicts.

 Copyright (c) 2007 Jeremy Huddleston
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization. */

#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

#include "quartzForeground.h"

int QuartzMoveToForeground() {
   ProcessSerialNumber psn = { 0, kCurrentProcess };
   OSStatus returnCode = TransformProcessType(& psn, kProcessTransformToForegroundApplication);
   if( returnCode == 0) {
       fprintf(stderr, "TransformProcessType: Success\n");
       SetFrontProcess(&psn);
   } else {
       fprintf(stderr, "TransformProcessType: Failure\n");
   }
   return (int)returnCode;
}
