/*
  Copyright (c) 2002 by Juliusz Chroboczek

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef _MKS_LIST_H_
#define _MKS_LIST_H_ 1

#include <X11/Xfuncproto.h> /* for _X_ATTRIBUTE_PRINTF */

char *dsprintf(const char *f, ...) _X_ATTRIBUTE_PRINTF(1,2);

typedef struct _List {
    char *value;
    struct _List *next;
} ListRec, *ListPtr;

int listMember(const char *elt, ListPtr list);
ListPtr listCons(char *car, ListPtr cdr);
ListPtr listAdjoin(char *car, ListPtr cdr);
ListPtr listConsF(ListPtr cdr, const char *f, ...) _X_ATTRIBUTE_PRINTF(2,3);
ListPtr listAdjoinF(ListPtr cdr, const char *f, ...) _X_ATTRIBUTE_PRINTF(2,3);
int listLength(ListPtr list);
ListPtr appendList(ListPtr first, ListPtr second);
ListPtr makeList(char **a, int n, ListPtr old, int begin);
ListPtr reverseList(ListPtr old);
ListPtr sortList(ListPtr old);
void destroyList(ListPtr old);
void deepDestroyList(ListPtr old);

#endif /* _MKS_LIST_H_ */
