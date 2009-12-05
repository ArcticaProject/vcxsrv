#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <stdio.h>

static __inline void __assert(int Cond)
{
#ifdef _DEBUG
  if (!Cond)
  {
    printf("assertion occured.\n");
    __asm int 3;
    while (1);
  }
#endif
}

#define assert(Cond) __assert((int)(Cond))

#endif
 