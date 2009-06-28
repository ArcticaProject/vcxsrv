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
/* $XFree86: xc/programs/mkfontscale/list.h,v 1.2 2003/06/20 15:49:52 eich Exp $ */

#ifndef _MKS_LIST_H_
#define _MKS_LIST_H_ 1

char *dsprintf(char *f, ...);

typedef struct _List {
    char *value;
    struct _List *next;
} ListRec, *ListPtr;

int listMember(char *elt, ListPtr list);
ListPtr listCons(char *car, ListPtr cdr);
ListPtr listAdjoin(char *car, ListPtr cdr);
ListPtr listConsF(ListPtr cdr, char *f, ...);
ListPtr listAdjoinF(ListPtr cdr, char *f, ...);
int listLength(ListPtr list);
ListPtr appendList(ListPtr first, ListPtr second);
ListPtr makeList(char **a, int n, ListPtr old, int begin);
ListPtr reverseList(ListPtr old);
void destroyList(ListPtr old);
void deepDestroyList(ListPtr old);

#endif /* _MKS_LIST_H_ */
