#include <list.h>
#include <sched.h>
#include <malloc.h>
#include <proc.h>
#include <switch.h>

extern struct list plist;
struct process *cur_process;

//bool need_sched = false;
bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);

struct process* get_next_proc(void)
{
	bool found = false;
	struct process *next;
    
	next = cur_process;

	list_sort(&plist, more_prio, NULL);

	while(!found)
	{
		if(&next->all_elem == list_back(&plist))
		{
			next = list_entry(list_front(&plist), struct process, all_elem);
		}
		else
		{
			next = list_entry(next->all_elem.next, struct process, all_elem);
		}

		if(next->state == PROC_RUN)
			found = true;
	}

	return next;
}

void schedule(void)
{              
	struct process *cur;
	struct process *next;
	bool found = false;

	proc_wake();

	next = get_next_proc();
	cur = cur_process;

	cur_process = next;

	switch_process(cur, next);
	cur_process->time_slice = 0;
}
