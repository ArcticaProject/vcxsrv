#ifndef __STDDEF_H__
#define __STDDEF_H__

#define offsetof(st, m) ((size_t) ( (char *)&((st *)(0))->m - (char *)0 ))

#endif
