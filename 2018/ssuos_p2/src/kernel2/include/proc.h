#ifndef		__PROC_H__
#define		__PROC_H__

#include <list.h>

#define PROC_EXTRA_ARG
#define PROC_SLEEP
#define SCHED_PREEMPTIVE
#define SCHED_PRIORITY
#define SCREEN_SCROLL

enum p_state {
	PROC_RUN,	
	PROC_STOP,
	PROC_ZOMBIE
};

typedef int pid_t;
#ifdef PROC_EXTRA_ARG
typedef void proc_func(void* aux);
#else
typedef void proc_func();
#endif

struct proc_option
{
	unsigned priority;
};

struct process
{
	pid_t pid;				//process id

	void *stack;			//stack for esp

	enum p_state state;		// process state

	struct list_elem all_elem;

	struct list_elem w_elem;
	unsigned long long time_sleep;

	unsigned priority;	//프로세스의 스케쥴링 우선순위
	unsigned time_slice;	//프로세스의 수행 tick
};

void proc_init(void);
void schedule(void);
void proc_print_data(void);
extern struct process *cur_process;
pid_t proc_create(proc_func func, struct proc_option *priority, void* aux);
void proc_start(void);
void proc_end(void);

void proc_wake(void);
void proc_sleep(unsigned ticks);
void proc_block(void);
void proc_unblock(struct process* proc);

#ifdef PROC_EXTRA_ARG
void print_pid(void* aux);
void idle(void* aux);
#else
void print_pid(void);
void idle(void);
#endif

#endif
