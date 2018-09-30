#include <list.h>
#include <sched.h>
#include <malloc.h>
#include <proc.h>
#include <ssulib.h>
#include <interrupt.h>
#include <sched.h>
#include <console.h>
#include <io.h>

#define PROC_NUM 16
#define STACK_SIZE 512

struct list plist;
struct list wlist;

//struct process proc0;
struct process procs[PROC_NUM];
int stacks [PROC_NUM][STACK_SIZE];
int proc_num = 0;
struct process *cur_process;

uint32_t process_stack_ofs = offsetof (struct process, stack);

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux);

void proc_init()
{
	list_init(&plist);

#ifdef PROC_SLEEP
	list_init(&wlist);
#endif

	//need_sched = 0;
	proc_num = 0;
    cur_process = &procs[proc_num];

    cur_process->pid = proc_num;
    cur_process->state = PROC_RUN;
	cur_process->priority = 0;
	cur_process->stack = 0;

	list_push_back(&plist, &cur_process->all_elem);
    proc_num++;

    sti();
}

pid_t proc_create(proc_func func, struct proc_option *opt, void* aux)
{
	struct process *p;

	cli();

	p = &procs[proc_num];

	p->pid = proc_num;
	p->state = PROC_RUN;
	if(opt != NULL)
		p->priority = opt->priority;   
	else
		p->priority = 256;

	//init stack
    int *top;
    top = (int*)(&stacks[proc_num][0] + STACK_SIZE-1);

#ifdef PROC_EXTRA_ARG
	*(--top) = (int)aux;		//argument for func
#endif
	*(--top) = (int)proc_end; //return address from func
	*(--top) = (int)func; //return address from proc_start
	*(--top) = (int)proc_start; //return address from switch_process

	/*
	process call stack : 
	switch_process > proc_start > func(aux) > proc_end
	*/

	*(--top) = 1; //eax
	*(--top) = 2; //ebx
	*(--top) = 3; //ecx
	*(--top) = 4; //edx
	*(--top) = (int)(&stacks[proc_num][0] + STACK_SIZE-1); //ebp
	*(--top) = 5; //esi
	*(--top) = 6; //edi

	p->stack = top;

	list_push_back(&plist, &p->all_elem);
	proc_num++;

	sti();
	return p->pid;
}

void  proc_start(void)
{
#ifdef SCHED_PREEMPTIVE
	outb(0x20, 0x20);
	sti();
#endif
	return;
}

void proc_end(void)
{
	cur_process->state = PROC_ZOMBIE;
	schedule();
	return;
}

void 
#ifdef PROC_EXTRA_ARG
idle(void* aux)
#else
idle(void)
#endif
{
	struct proc_option opt;
	opt.priority = 1;
	proc_create(print_pid, &opt, "proc1");
	opt.priority = 2;
	proc_create(print_pid, &opt, "proc2");
	opt.priority = 3;
	proc_create(print_pid, &opt, "proc3");

	while(1)
	{  
		if(cur_process->pid != 0)
		{
			printk("error : pid != 0\n", cur_process->pid);
			while(1);
		}
		schedule();     
	}
}

void
#ifdef PROC_EXTRA_ARG
print_pid(void* aux)
#else
print_pid(void)
#endif
{
	while(1)
	{
//		printk("pid = %d prio = %d time = %d ticks = %d ",
//				cur_process->pid, cur_process->priority, cur_process->time_slice, get_ticks());
		printk("pid = %d ", cur_process->pid);
		printk("prio = %d ", cur_process->priority);
		printk("time = %d ", cur_process->time_slice);
		printk("ticks = %d ", get_ticks());
		printk("in %s\n",
#ifdef PROC_EXTRA_ARG
				aux
#else
				"print_pid"
#endif
			  );

#ifdef PROC_SLEEP
		proc_sleep(cur_process->pid * cur_process->pid * 1000);
#endif

#ifndef SCHED_PREEMPTIVE
		schedule();     
#endif
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

void proc_wake(void)
{
	struct process* p;
	unsigned long long t = get_ticks();

    while(!list_empty(&wlist))
	{
		p = list_entry(list_front(&wlist), struct process, w_elem);
		if(p->time_sleep > t)
			break;
		proc_unblock(p);
	}
}

void proc_sleep(unsigned ticks)
{
	unsigned long cur_ticks = get_ticks();
	cur_process->time_sleep =  ticks + cur_ticks;
	proc_block();
}

void proc_block(void)
{        
	cur_process->state = PROC_STOP;
	list_insert_ordered(&wlist, &cur_process->w_elem,
			less_time_sleep, NULL);
	schedule();
}

void proc_unblock(struct process* proc)
{
	proc->state = PROC_RUN;
    list_remove(&proc->w_elem);
}     

bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, w_elem);
	struct process *p2 = list_entry(b, struct process, w_elem);

    return p1->time_sleep < p2->time_sleep;
}

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux)
{
	struct process *p1 = list_entry(a, struct process, w_elem);
	struct process *p2 = list_entry(b, struct process, w_elem);
    return p1->priority > p2->priority;
}

