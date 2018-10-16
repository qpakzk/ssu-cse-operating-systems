#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>

uint32_t *PID0_PAGE_DIR;

intr_handler_func pf_handler;

uint32_t scale_up(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0)
		base = base & mask + size;
	return base;
}

uint32_t scale_down(uint32_t base, uint32_t size)
{
	uint32_t mask = ~(size-1);
	if(base & mask != 0)
		base = base & mask - size;
	return base;
}

void init_paging()
{
	uint32_t *page_dir = palloc_get_page(HEAP__);
	uint32_t *page_tbl = palloc_get_page(HEAP__);
//	page_dir = VH_TO_RH(page_dir);
	page_dir = va_to_ra(page_dir);
//	page_tbl = VH_TO_RH(page_tbl);
	page_tbl = va_to_ra(page_tbl);
	PID0_PAGE_DIR = page_dir;

	int NUM_PT, NUM_PE;
	uint32_t cr0_paging_set;
	int i;

	NUM_PE = mem_size() / PAGE_SIZE;
	NUM_PT = NUM_PE / 1024;

	printk("-PE=%d, PT=%d\n", NUM_PE, NUM_PT);
	printk("-page dir=%x page tbl=%x\n", page_dir, page_tbl);

	//페이지 디렉토리 구성
	page_dir[0] = (uint32_t)page_tbl | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;
	
	NUM_PE = RKERNEL_HEAP_START / PAGE_SIZE;
	//페이지 테이블 구성
	for ( i = 0; i < NUM_PE; i++ ) {
		page_tbl[i] = (PAGE_SIZE * i)
			| PAGE_FLAG_RW
			| PAGE_FLAG_PRESENT;
		//writable & present
	}


	//CR0레지스터 설정
	cr0_paging_set = read_cr0() | CR0_FLAG_PG;		// PG bit set

	//컨트롤 레지스터 저장
	write_cr3( (unsigned)page_dir );		// cr3 레지스터에 PDE 시작주소 저장
	write_cr0( cr0_paging_set );          // PG bit를 설정한 값을 cr0 레지스터에 저장

	reg_handler(14, pf_handler);
}

void memcpy_hard(void* from, void* to, uint32_t len)
{
	write_cr0( read_cr0() & ~CR0_FLAG_PG);
	memcpy(from, to, len);
	write_cr0( read_cr0() | CR0_FLAG_PG);
}

uint32_t* get_cur_pd()
{
	return (uint32_t*)read_cr3();
}

uint32_t pde_idx_addr(uint32_t* addr)
{
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PDE) >> PAGE_OFFSET_PDE;
	return ret;
}

uint32_t pte_idx_addr(uint32_t* addr)
{
	uint32_t ret = ((uint32_t)addr & PAGE_MASK_PTE) >> PAGE_OFFSET_PTE;
	return ret;
}

uint32_t* pt_pde(uint32_t pde)
{
	uint32_t * ret = (uint32_t*)(pde & PAGE_MASK_BASE);
	return ret;
}

uint32_t* pt_addr(uint32_t* addr)
{
	uint32_t idx = pde_idx_addr(addr);
	uint32_t* pd = get_cur_pd();
	return pt_pde(pd[idx]);
}

uint32_t* pg_addr(uint32_t* addr)
{
	uint32_t *pt = pt_addr(addr);
	uint32_t idx = pte_idx_addr(addr);
	uint32_t *ret = (uint32_t*)(pt[idx] & PAGE_MASK_BASE);
	return ret;
}

/*
    page table 복사
*/
void  pt_copy(uint32_t *pd, uint32_t *dest_pd, uint32_t idx)
{
	uint32_t *pt = pt_pde(pd[idx]);
	uint32_t *new_pt;
	uint32_t i;

//	pt = RH_TO_VH(pt);
	pt = ra_to_va(pt);
    new_pt = palloc_get_page(HEAP__);
 
    for(i = 0; i<1024; i++)
    {
      	if(pt[i] & PAGE_FLAG_PRESENT)
    	{
//            new_pt = VH_TO_RH(new_pt);
			new_pt = va_to_ra(new_pt);
            dest_pd[idx] = (uint32_t)new_pt | (pd[idx] & ~PAGE_MASK_BASE & ~    PAGE_FLAG_ACCESS);
//            new_pt = RH_TO_VH(new_pt);
			new_pt = ra_to_va(new_pt);
            new_pt[i] = pt[i];
        }
    }
}

/*
    page directory 복사. 
*/
void pd_copy(uint32_t* from, uint32_t* to)
{
	uint32_t i;

	for(i = 0; i < 1024; i++)
	{
		if(from[i] & PAGE_FLAG_PRESENT)
			pt_copy(from, to, i);
	}
}

uint32_t* pd_create (pid_t pid)
{
	uint32_t *pd = palloc_get_page(HEAP__);

//	pd_copy(RH_TO_VH((uint32_t*)read_cr3()), pd);
	pd_copy(ra_to_va((uint32_t*)read_cr3()), pd);

    //pd = VH_TO_RH(pd);
	pd = va_to_ra(pd);

	return pd;
}
void child_stack_reset(pid_t pid){// (2)
		
}

void pf_handler(struct intr_frame *iframe)
{
	void *fault_addr;

	asm ("movl %%cr2, %0" : "=r" (fault_addr));

	printk("Page fault : %X\n",fault_addr);
#ifdef SCREEN_SCROLL
	refreshScreen();
#endif

	uint32_t pdi, pti;
    uint32_t *pta;
    uint32_t *pda = (uint32_t*)read_cr3();

    pdi = pde_idx_addr(fault_addr);
    pti = pte_idx_addr(fault_addr);

    if(pda == PID0_PAGE_DIR){
        write_cr0( read_cr0() & ~CR0_FLAG_PG);
        pta = pt_pde(pda[pdi]);
        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else{
        //pda = RH_TO_VH(pda);
		pda = ra_to_va(pda);

        pta = pt_pde(pda[pdi]);
    }

    if(pta == NULL){
        write_cr0( read_cr0() & ~CR0_FLAG_PG);

        pta = palloc_get_page(HEAP__);
//        pta = VH_TO_RH(pta);
		pta = va_to_ra(pta);
        memset(pta,0,PAGE_SIZE);
        
        pda[pdi] = (uint32_t)pta | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

		fault_addr = (uint32_t*)((uint32_t)fault_addr & PAGE_MASK_BASE);

//	        fault_addr = VH_TO_RH(fault_addr);
		fault_addr = va_to_ra(fault_addr);

        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;

//        pta = RH_TO_VH(pta);
		pta = ra_to_va(pta);
        pdi = pde_idx_addr(pta);
        pti = pte_idx_addr(pta);

        uint32_t *tmp_pta = pt_pde(pda[pdi]);
        //tmp_pta[pti] = (uint32_t)VH_TO_RH(pta) | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;
        tmp_pta[pti] = (uint32_t)va_to_ra(pta) | PAGE_FLAG_RW | PAGE_FLAG_PRESENT;

        write_cr0( read_cr0() | CR0_FLAG_PG);
    }
    else{
//        pta = RH_TO_VH(pta);
		pta = ra_to_va(pta);
		fault_addr = (uint32_t*)((uint32_t)fault_addr & PAGE_MASK_BASE);
	       // fault_addr = VH_TO_RH(fault_addr);
		fault_addr = va_to_ra(fault_addr);
        pta[pti] = (uint32_t)fault_addr | PAGE_FLAG_RW  | PAGE_FLAG_PRESENT;
    }
}
