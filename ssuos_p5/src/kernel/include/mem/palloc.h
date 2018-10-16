#ifndef __PALLOC_H__
#define __PALLOC_H__

#include <ssulib.h>

#define FREE__ 	0x001
#define HEAP__ 	0x010
#define STACK__	0x020

#define PAGE_POOL_SIZE 1024

void init_palloc (void);
uint32_t *palloc_get_page (uint32_t);
uint32_t *palloc_get_multiple (uint32_t, size_t );
void palloc_free_page (void *);
void palloc_free_multiple (void *, size_t page_cnt);
void palloc_pf_test(void);
uint32_t *va_to_ra(uint32_t *va);
uint32_t *ra_to_va(uint32_t *ra);



#endif /* palloc.h */

