#ifndef		__PROC_H__
#define		__PROC_H__

#include <list.h>

#define PROC_NUM_MAX 16

typedef int pid_t;

enum p_state {
	PROC_UNUSED,
	PROC_RUN,	
	PROC_STOP,
	PROC_ZOMBIE,
	PROC_BLOCK,
};

#pragma pack(push, 1)

struct proc_option
{
	unsigned char nice; 
	unsigned char rt_priority;

};

struct process
{
	pid_t pid;	
	void *stack;	
	void *pd;	
	enum p_state state;
	unsigned long long time_sleep;
	struct process* parent;
	int simple_lock;	
	int child_pid;
	int exit_status;
	int que_level; //(0,1,2) queue level

	unsigned char priority;		
	unsigned char rt_priority;	
	unsigned char nice;		
	
	bool old_proc;
	unsigned member;
	unsigned time_slice;	
	unsigned long long time_used;	


	struct list_elem elem_all;	
	struct list_elem elem_stat;	
};

#pragma pack(pop)

extern struct process *cur_process;
extern unsigned long proc_ticks;

typedef void proc_func(void* aux);
proc_func print_pid;	
proc_func idle;	

void schedule(void);
struct process* sched_find_set();

void init_proc(void);
void proc_free(void);
pid_t proc_create(proc_func func, struct proc_option *priority, void* aux);
void proc_wake(void);
void proc_sleep(unsigned ticks);	
void proc_block(void);
void proc_unblock(struct process* proc);
void proc_print_data(void);
#endif
