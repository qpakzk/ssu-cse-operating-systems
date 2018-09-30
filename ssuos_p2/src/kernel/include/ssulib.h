#ifndef __SSULIB_H__
#define __SSULIB_H__

#include <type.h>
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)

/* GCC predefines the types we need for ptrdiff_t and size_t,
   so that we don't have to guess. */
/* typedef __PTRDIFF_TYPE__ ptrdiff_t; */
/* typedef __SIZE_TYPE__ size_t; */

void delay(int times);

#endif
