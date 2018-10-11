#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <ssulib.h>
#include <interrupt.h>
#include <proc/sched.h>
#include <syscall.h>
#include <mem/palloc.h>
#include <device/console.h>


#define STACK_SIZE 512

struct list plist;				
struct list level_que[QUE_LV_MAX];
struct list slist;				
struct list dlist;				
struct process procs[PROC_NUM_MAX]; 
struct process *cur_process;
struct process *idle_process; 
int pid_num_max;
uint32_t process_stack_ofs; 
static int lock_pid_simple; 
static int lately_pid; 

bool more_prio(const struct list_elem *a, const struct list_elem *b, void *aux);
bool less_time_sleep(const struct list_elem *a, const struct list_elem *b, void *aux);
pid_t getValidPid(int *idx);
void proc_start(void);
void proc_end(void);


void kernel1_proc(void *aux);
void kernel2_proc(void *aux);
void kernel3_proc(void *aux);
void kernel4_proc(void *aux);
void kernel5_proc(void *aux);
void kernel6_proc(void *aux);
void kernel7_proc(void *aux);
void kernel8_proc(void *aux);
void kernel9_proc(void *aux);
void kernel10_proc(void *aux);

void init_proc()
{
	int i, j;
	process_stack_ofs = offsetof (struct process, stack); 

	lock_pid_simple = 0;
	lately_pid = -1;

	list_init(&plist);

	for (i = 0; i < 3 ; i++) {
			list_init(&level_que[i]);
			printk("create level_que[%d]", i);
	}

	list_init(&slist);
	list_init(&dlist);

	for (i = 0; i < PROC_NUM_MAX; i++)
	{
		procs[i].pid = i;
		procs[i].state = PROC_UNUSED;
		procs[i].parent = NULL;
	}

	pid_t pid = getValidPid(&i);
	cur_process = &procs[0];
	idle_process = &procs[0]; //procs 0 is always idle

	cur_process->pid = pid;
	cur_process->parent = NULL;
	cur_process->state = PROC_RUN;

	cur_process -> nice = 0;
	cur_process -> rt_priority = 0;
	cur_process -> priority = cur_process -> nice + cur_process -> rt_priority;

	cur_process->stack = 0;
	cur_process->pd = (void*)read_cr3();
	cur_process -> elem_all.prev = NULL;
	cur_process -> elem_all.next = NULL;
	cur_process -> elem_stat.prev = NULL;
	cur_process -> elem_stat.next = NULL;

	cur_process->que_level = 0 ;
	list_push_back(&plist, &cur_process->elem_all);
	list_push_back(&level_que[0], &cur_process->elem_stat);

}

pid_t getValidPid(int *idx) {
	pid_t pid = -1;
	int i;

	while(lock_pid_simple);

	lock_pid_simple++;

	for(i = 0; i < PROC_NUM_MAX; i++)
	{
		int tmp = i + lately_pid + 1;
		if(procs[tmp % PROC_NUM_MAX].state == PROC_UNUSED) { 
			pid = lately_pid + 1;
			*idx = tmp % PROC_NUM_MAX;
			break;
		}
	}

	if(pid != -1)
		lately_pid = pid;	

	lock_pid_simple = 0;

	return pid;
}

pid_t proc_create(proc_func func, struct proc_option *opt, void* aux)
{
	struct process *p;
	int idx;
	int i,j;

	enum intr_level old_level = intr_disable();

	pid_t pid = getValidPid(&idx);
	p = &procs[pid];
	p->pid = pid;
	p->state = PROC_RUN;

	if(opt != NULL) {
		p -> nice = opt -> nice; // not use.
		p -> rt_priority = opt -> rt_priority;   //not use.
	}
	else {
		p -> nice = 20;
		p -> rt_priority = (unsigned char)45;
	}

	p -> priority = p -> nice + p -> rt_priority; //not use.
	p -> que_level = 1; 
	p -> old_proc = 0;
	p->time_used = 0;
	p->time_slice = 0;
	p->parent = cur_process;
	p->simple_lock = 0;
	p->child_pid = -1;
	p->pd = pd_create(p->pid);

	//init stack
	int *top = (int*)palloc_get_page();
	int stack = (int)top;
	top = (int*)stack + STACK_SIZE - 1;

	*(--top) = (int)aux;		//argument for func
	*(--top) = (int)proc_end;	//return address from func
	*(--top) = (int)func;		//return address from proc_start
	*(--top) = (int)proc_start; //return address from switch_process

	//process call stack : 
	//switch_process > proc_start > func(aux) > proc_end

	*(--top) = (int)((int*)stack + STACK_SIZE - 1); //ebp
	*(--top) = 1; //eax
	*(--top) = 2; //ebx
	*(--top) = 3; //ecx
	*(--top) = 4; //edx
	*(--top) = 5; //esi
	*(--top) = 6; //edi

	p -> stack = top;
	p -> elem_all.prev = NULL;
	p -> elem_all.next = NULL;
	p -> elem_stat.prev = NULL;
	p -> elem_stat.next = NULL;

	list_push_back(&plist, &p->elem_all);
	 //TODO : when create first , input the level 1 queue

	intr_set_level (old_level);
	return p->pid;
}

void* getEIP()
{
    return __builtin_return_address(0);
}

void  proc_start(void)
{
	intr_enable ();
	return;
}

void proc_free(void)
{
	uint32_t pt = *(uint32_t*)cur_process->pd;
	cur_process->parent->child_pid = cur_process->pid;
	cur_process->parent->simple_lock = 0;

	cur_process->state = PROC_ZOMBIE;
	list_push_back(&dlist, &cur_process->elem_stat); 

	palloc_free_page(cur_process->stack); 
	palloc_free_page((void*)pt);
	palloc_free_page(cur_process->pd);

	list_remove(&cur_process->elem_stat);
	list_remove(&cur_process->elem_all);
}

void proc_end(void)
{
	proc_free();
	schedule();
	return;
}

void proc_wake(void)
{
	struct process* p;
	int que_level;
	int old_level;
	unsigned long long t = get_ticks();
	struct list_elem *e;
		for(e = list_begin (&slist); e != list_end (&slist);
		e = list_next (e))
	{
		struct process* p = list_entry(e, struct process, elem_stat);
	}
    while(!list_empty(&slist))
	{
			p = list_entry(list_front(&slist), struct process, elem_stat); //pop the sleep list of sleeping proc
		if(p->time_sleep > t)
			break;

		list_remove(&p->elem_stat);
		que_level = 1;
		p->que_level = 1;
		list_push_back(&level_que[que_level], &p->elem_stat);
		p->state = PROC_RUN;
	}
	
}

void proc_sleep(unsigned ticks)
{
	unsigned long cur_ticks = get_ticks();

	cur_process->time_sleep =  ticks + cur_ticks;
	cur_process->state = PROC_STOP;
	cur_process->time_slice = 0;
	struct list_elem *e;
	
	list_remove(&cur_process->elem_stat); //remove queue list

	list_insert_ordered(&slist, &cur_process->elem_stat,
			less_time_sleep, NULL); //order by less time sleep 

	list_sort (&slist,less_time_sleep, NULL);

	schedule();
}

void proc_block(void) //io 
{
	cur_process->state = PROC_BLOCK;
	schedule();	
}

void proc_unblock(struct process* proc)
{
	enum intr_level old_level;
	list_push_back(&level_que[proc->que_level],&proc->elem_stat);
	proc->state = PROC_RUN;
}     

bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, elem_stat);
	struct process *p2 = list_entry(b, struct process, elem_stat);

	return p1->time_sleep < p2->time_sleep;
}

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, elem_stat);
	struct process *p2 = list_entry(b, struct process, elem_stat);
	
	return p1->priority > p2->priority;
}

void kernel1_proc(void* aux)
{
	int passed = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 80) && (!passed)) {
		
			proc_sleep(60);
			passed = 1;
			
		}
		if (cur_process -> time_used >= 200)
			proc_end();	
	}
}

void kernel2_proc(void* aux)
{
	int passed = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 30) && (!passed)) {
			proc_sleep(30);
			passed = 1;
			
		}

		if (cur_process -> time_used >= 120) {
			proc_end();
		}
	}
}

void kernel3_proc(void* aux)
{
	int passed = 0;


	while(1)
	{
		if ((cur_process -> time_used >= 100) && (!passed)) {
			proc_sleep(200);
			passed = 1;
		
		}

		if (cur_process -> time_used >= 300) {
			proc_end();
		}
	}
}

void kernel4_proc(void* aux)
{
	int passed1=0, passed2 = 0 ;

	while(1)
	{
		if ((cur_process -> time_used >= 30) && (!passed1)){
			proc_sleep(250);
			passed1 = 1 ;
		
		}
		
		if ((cur_process -> time_used >= 80) && (!passed2)){
			proc_sleep(300);
			passed2 =1;
			
		}

		if (cur_process -> time_used >=300){
			proc_end();
		}
	}
 
}
void kernel5_proc(void* aux)
{
	int passed1=0, passed2 = 0 ;
	while(1)
	{
		if ((cur_process -> time_used >= 20) && (!passed1)){
			proc_sleep(350);
			passed1 = 1;
			
		}
		if ((cur_process -> time_used >= 100) && (!passed2)){
			proc_sleep(50);
			passed2 = 1;
		
		}
		if (cur_process -> time_used >=400)
			proc_end();
	}
}

void kernel6_proc(void* aux)
{
	int passed1=0, passed2 = 0, passed3 = 0, passed4 = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 10) && (!passed1)){
			proc_sleep(10);
			passed1 = 1;
			
		}
		if ((cur_process -> time_used >= 30) && (!passed2)){
			proc_sleep(20);
			passed2 = 1;
			
		}
		if ((cur_process -> time_used >= 100) && (!passed3)){
			proc_sleep(200);
			passed3 = 1;
		
		}
		if ((cur_process -> time_used >= 150) && (!passed4)){
			proc_sleep(10);
			passed4 = 1;
			
		}

		if (cur_process -> time_used >=200)
			proc_end();
	}
}

void kernel7_proc(void* aux)
{
	int passed1=0, passed2 = 0, passed3 = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 20) && (!passed1)){
			proc_sleep(20);
			passed1 = 1;
			
		}
		if ((cur_process -> time_used >= 40) && (!passed2)){
			proc_sleep(200);
			passed2 = 1;
			
		}
		if ((cur_process -> time_used >= 100) && (!passed3)){
			proc_sleep(10);
			passed3 = 1;
			
		}
		if (cur_process -> time_used >=500)
			proc_end();
	}
}

void kernel8_proc(void* aux)
{
	int passed = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 20) && (!passed)){
			proc_sleep(200);
			passed = 1;
			
		}

		if (cur_process -> time_used >=600)
			proc_end();
	}
}

void kernel9_proc(void* aux)
{
	int passed = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 30) && (!passed)){
			proc_sleep(20);
			passed = 1;
			
		}

		if (cur_process -> time_used >=750)
			proc_end();
	}
}

void kernel10_proc(void* aux)
{
	int passed = 0;
	while(1)
	{
		if ((cur_process -> time_used >= 200) && (!passed)){
			proc_sleep(100);
			passed = 1;
			
		}

		if (cur_process -> time_used >=800)
			proc_end();
	}
}

void idle(void* aux)
{
	proc_create(kernel1_proc, NULL, NULL);
	proc_create(kernel2_proc, NULL, NULL);
	proc_create(kernel3_proc, NULL, NULL);
	proc_create(kernel4_proc, NULL, NULL);
	proc_create(kernel5_proc, NULL, NULL);
	proc_create(kernel6_proc, NULL, NULL);
	proc_create(kernel7_proc, NULL, NULL);
	proc_create(kernel8_proc, NULL, NULL);
	proc_create(kernel9_proc, NULL, NULL);
	proc_create(kernel10_proc, NULL, NULL);
	printk("create proc done\n");
	struct list_elem *ele;
	while(1) {  
		schedule();    
	}
}



void proc_print_data()
{
	int a, b, c, d, bp, si, di, sp;

	//eax ebx ecx edx
	__asm__ __volatile("mov %%eax ,%0": "=m"(a));

	__asm__ __volatile("mov %ebx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(b));
	
	__asm__ __volatile("mov %ecx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(c));
	
	__asm__ __volatile("mov %edx ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(d));
	
	//ebp esi edi esp
	__asm__ __volatile("mov %ebp ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(bp));

	__asm__ __volatile("mov %esi ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(si));

	__asm__ __volatile("mov %edi ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(di));

	__asm__ __volatile("mov %esp ,%eax");
	__asm__ __volatile("mov %%eax ,%0": "=m"(sp));

	printk(	"\neax %o ebx %o ecx %o edx %o"\
			"\nebp %o esi %o edi %o esp %o\n"\
			, a, b, c, d, bp, si, di, sp);
}

void hexDump (void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (len == 0) {
        printk("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printk("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printk ("  %s\n", buff);

            printk ("  %04x ", i);
        }

        printk (" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printk ("   ");
        i++;
    }

    printk ("  %s\n", buff);
}


