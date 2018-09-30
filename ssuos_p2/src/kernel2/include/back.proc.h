#ifndef		__PROC_H__
#define		__PROC_H__

#include <list.h>

#define PROC_EXTRA_ARG

enum p_state {
	PROC_RUN,	
	PROC_STOP,
	PROC_ZOMBIE
};

typedef int pid_t;
#ifdef PROC_EXTRA_ARG
typedef void proc_func(void* aux);
#else
typedef void proc_func()
#endif

struct proc_option
{
	int priority;
};

struct process
{
	pid_t pid;				//process id

	void *stack;			//stack for esp

	enum p_state state;		// process state

	struct list_elem all_elem;

	int priority;	//프로세스의 스케쥴링 우선순위
	//int time_slice;	//프로세스의 수행 tick
};

void proc_init();
pid_t proc_create(proc_func func, struct proc_option *priority, void* aux)
void schedule(void);
void proc_print_data();
extern struct process *cur_process;

#ifdef PROC_EXTRA_ARG
void print_pid(void* aux);
void idle(void* aux);
#else
void print_pid(void);
void idle(void);
#endif

#endif
