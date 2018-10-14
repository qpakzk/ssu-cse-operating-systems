#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>
#include <interrupt.h>
#include <device/console.h>

extern struct list level_que[QUE_LV_MAX];
extern struct list plist;
extern struct list slist;
extern struct process procs[PROC_NUM_MAX];
extern struct process *idle_process;

struct process *latest;

struct process* get_next_proc(struct list *rlist_target);
void proc_que_levelup(struct process *cur);
void proc_que_leveldown(struct process *cur);
struct process* sched_find_que(void);

int scheduling;

/*
   linux multilevelfeedback queue scheduler
   level 1 que policy is FCFS(First Come, First Served)
   level 2 que policy is RR(Round Robin).
 */

//sched_find_que find the next process from the highest queue that has proccesses.
struct process* sched_find_que(void) {
	int i,j;
	struct process * result = NULL;

	proc_wake();

	/*TODO :check the queue whether it is empty or not  
	  and find each queue for the next process.
	 */
	//레벨 1 큐가 비어있지 않으면
	if(!list_empty (&level_que[1]))
		//레벨 큐 1에서 프로세스를 가져온다.
		result = get_next_proc(&level_que[1]);
	//레벨 2 큐가 비어있지 않으면
	else if(!list_empty(&level_que[2]))
		//레벨 큐 2에서 프로세스를 가져온다.
		result = get_next_proc(&level_que[2]);
	return result;
}

struct process* get_next_proc(struct list *rlist_target) {
	struct list_elem *e;

	for(e = list_begin (rlist_target); e != list_end (rlist_target);
			e = list_next (e))
	{
		struct process* p = list_entry(e, struct process, elem_stat);

		if(p->state == PROC_RUN)
			return p;
	}
	return NULL;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;
	struct process *tmp;
	struct list_elem *ele;
	int i = 0, printed = 0;

	scheduling = 1;	
	cur = cur_process;
	/*TODO : if current process is idle_process(pid 0), schedule() choose the next process (not pid 0).
	  when context switching, you can use switch_process().  
	  if current process is not idle_process, schedule() choose the idle process(pid 0).
	  complete the schedule() code below.
	 */
	//0번 프로세스가 아닐 경우
	if ((cur -> pid) != 0) {
		//현재 프로세스를 latest에 저장
		latest = cur;
		//0번 프로세스를 다음에 실행할 프로세스로 가져온다.
		next = get_next_proc(&level_que[0]);
		
		//타임 인터럽트가 걸리는 것을 허용한다.
		scheduling = 0;
		//다음 프로세스를 현재 실행 중인 프로세스로 설정
		cur_process = next;
		
		//문맥 교환을 하는 동안 인터럽트 disable
		intr_disable();
		//다음 프로세스로 문맥 교환
		switch_process(cur, next);
		//문맥 교환 종료 후 인터럽트 enable
		intr_enable();

		return;
	}
	//0번 프로세스일 경우

	switch (latest -> que_level){
		case 1:
			//프로세스가 레벨 1 큐에 있었을 경우 
			//레벨 2 큐로 이동
			if(latest->state == PROC_RUN)
				proc_que_leveldown(latest);
			break;
		case 2:
			//프로세스가 레벨 2 큐에 있었을 경우 
			//다시 레벨 2 큐에 저장(라운드 로빈)
			if(latest->state == PROC_RUN)
			{
				list_remove(&latest->elem_stat);
				list_push_back(&level_que[2], &latest->elem_stat);
			}
			break;
	}

	proc_wake(); //wake up the processes

	//print the info of all 10 proc.
	//출력하는 동안 다른 인터럽트가 걸리는 것을 막기 위해 추가
	intr_disable();
	for (ele = list_begin(&plist); ele != list_end(&plist); ele = list_next(ele)) {
		tmp = list_entry (ele, struct process, elem_all);
		if ((tmp -> state == PROC_ZOMBIE) || 
				//(tmp -> state == PROC_BLOCK) || 
				//	(tmp -> state == PROC_STOP) ||
				(tmp -> pid == 0)) 	continue;
		if (!printed) {
			printk("#=%2d t=%3d u=%3d ", tmp -> pid, tmp -> time_slice, tmp -> time_used);
			printk("q=%3d\n", tmp->que_level);
			printed = 1;
		}
		else {
			printk(", #=%2d t=%3d u=%3d ", tmp -> pid, tmp -> time_slice, tmp->time_used);
			printk("q=%3d\n", tmp->que_level);
		}

	}
	//반복문 종료 후 인터럽트가 걸리는 것이 가능하도록 설정
	intr_enable();

	if (printed)
		printk("\n");

	// 다음에 실행할 프로세스 찾기
	if ((next = sched_find_que()) != NULL) {
		printk("Selected process : %d\n", next -> pid);
		//타임 인터럽트가 걸리는 것을 허용한다.
		scheduling = 0;
		//다음 프로세스를 현재 실행 중인 프로세스로 설정
		cur_process = next;
		
		//타임 퀀텀을 0으로 초기화
		//한 타임 퀀텀에서 실행한 tick을 계산하므로
		latest->time_slice = 0;

		//문맥 교환을 하는 동안 인터럽트 disable
		intr_disable();
		//다음 프로세스로 문맥 교환
		switch_process(cur, next);
		//문맥 교환 종료 후 인터럽트 enable
		intr_enable();
		return;
	}
	return;
}

void proc_que_levelup(struct process *cur)
{
	/*TODO : change the queue lv2 to queue lv1.*/
	//I/O 전에 있었던 레벨 큐 출력
	printk("Proc %d\'s in %d\n", cur->pid, cur->que_level);
	//레벨 1 큐라면 다시 레벨 1 큐로 돌아옴
	if(cur->que_level == 1)
		printk("Proc %d changes the queue(1->1)\n", cur->pid);
	//레벨 2 큐라면 레벨 1 큐로 올라옴
	else if(cur->que_level == 2)
		printk("Proc %d changes the queue(2->1)\n", cur->pid);
}

void proc_que_leveldown(struct process *cur)
{
	/*TODO : change the queue lv1 to queue lv2.*/
	//레벨 1 큐를 레벨 2 큘로 이동시킨다.
	cur->que_level = 2;
	printk("Proc %d changes the queue(1->2)\n", cur->pid);
	list_remove(&cur->elem_stat);
	list_push_back(&level_que[2], &cur->elem_stat);
}
