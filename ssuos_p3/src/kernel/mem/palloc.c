#include <mem/palloc.h>
#include <bitmap.h>
#include <type.h>
#include <round.h>
#include <mem/mm.h>
#include <synch.h>
#include <device/console.h>
#include <mem/paging.h>

/* Page allocator.  Hands out memory in page-size (or
   page-multiple) chunks.  

   System memory is managed by "pools" called the mem  pools.
   pool check free page and allocate page to process 
   using bitmap.
   */

/* A memory pool. */
struct pool
{
	struct lock lock;                   /* Mutual exclusion. */
	struct bitmap *used_map;            /* Bitmap of free pages. */
	uint32_t *base;                      /* Base of pool. */
};

static struct pool mem_pool;

static void init_pool (struct pool *, void *base, size_t page_cnt,
		const char *name);

/* Initializes the page allocator. */
	void
init_palloc (void) 
{
	/* Free memory starts at 1 MB and runs to the end of RAM. */
	uint32_t *free_start = (uint32_t*)KERNEL_ADDR;
	uint32_t *free_end = (uint32_t*)mem_size();
	size_t free_pages = ((unsigned int)free_end - (unsigned int)free_start) / PAGE_SIZE;

	init_pool (&mem_pool, free_start, free_pages, "memory pool"); 
}

/* Obtains and returns a group of PAGE_CNT contiguous free pages.
   */
	uint32_t *
palloc_get_multiple (size_t page_cnt)
{
	struct pool *pool = &mem_pool;  
	void *pages;
	size_t page_idx;

	if (page_cnt == 0)
		return NULL;

	lock_acquire (&pool->lock);  
	page_idx = bitmap_scan_and_flip (pool->used_map, 0, page_cnt, false);
	lock_release (&pool->lock);  

	if (page_idx != BITMAP_ERROR)
		pages = pool->base + PAGE_SIZE * page_idx/sizeof(uint32_t);
	else 
		pages = NULL;

	if (pages != NULL) 
	{
		memset (pages, 0, PAGE_SIZE * page_cnt);
	}

	return (uint32_t*)pages; 
}

/* Obtains a single free page and returns its address.
   */
	uint32_t *
palloc_get_page (void) 
{
	return palloc_get_multiple (1);
}

/* Frees the PAGE_CNT pages starting at PAGES. */
	void
palloc_free_multiple (void *pages, size_t page_cnt) 
{
	struct pool *pool;
	size_t page_idx;

	if (pages == NULL || page_cnt == 0)
		return;

	pool = &mem_pool;

	page_idx = pg_no (pages) - pg_no (pool->base); 

	bitmap_set_multiple (pool->used_map, page_idx, page_cnt, false);
}

/* Frees the page at PAGE. */
	void
palloc_free_page (void *page) 
{
	palloc_free_multiple (page, 1);
}

/* Initializes pool P as starting at START and ending at END,
   naming it NAME for debugging purposes. */
	static void
init_pool (struct pool *p, void *base, size_t page_cnt, const char *name) 
{
	/* We'll put the pool's used_map at its base.
	   Calculate the space needed for the bitmap
	   and subtract it from the pool's size. */
	size_t bm_pages = DIV_ROUND_UP (bitmap_buf_size (page_cnt), PAGE_SIZE);
	if (bm_pages > page_cnt) 
		printk("Not enough memory in %s for bitmap.", name);
	page_cnt -= bm_pages;  

	printk ("%u pages available in %s.\n", page_cnt, name);

	/* Initialize the pool. */
	lock_init (&p->lock);  
	p->used_map = bitmap_create_in_buf (page_cnt, base, bm_pages * PAGE_SIZE); 
	p->base = base + bm_pages * PAGE_SIZE; 
}


