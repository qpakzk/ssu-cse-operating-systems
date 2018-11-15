#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <ssulib.h>
#include <string.h>
#include <interrupt.h>
#include <proc/sched.h>
#include <device/console.h>
#include <device/io.h>
#include <syscall.h>
#include <mem/paging.h>
#include <mem/palloc.h>
#include <filesys/fs.h>
#include <filesys/file.h>

#define STACK_SIZE	512
#define PROC_NUM_MAX 16

struct list p_list;		// All Porcess List
struct list r_list;		// Run Porcess List
struct list s_list;		// Sleep Process List
struct list d_list;		// Deleted Process List

struct process procs[PROC_NUM_MAX];
struct process *cur_process;
int pid_num_max;


uint32_t process_stack_ofs;

//values for pid
static int lock_pid_simple; //1 : lock, 0 : unlock
static int lately_pid;		//init vaule = -1

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
bool less_time_sleep(const struct list_elem *a, const struct list_elem *b,void *aux);
pid_t getValidPid(int *idx);

void proc_start(void);				//프로세스 시작시 실행
void proc_end(void);				//프로세스 종료시 실행

void init_proc()
{
	process_stack_ofs = offsetof (struct process, stack);

	lock_pid_simple = 0;
	lately_pid = -1;

	list_init(&p_list);
	list_init(&r_list);
	list_init(&s_list);
	list_init(&d_list);

	int i;
	for (i = 0; i < PROC_NUM_MAX; i++)
	{
		memset (&procs[i], 0, 77);
		procs[i].pid = i;
		procs[i].state = PROC_UNUSED;
		procs[i].parent = NULL;
	}

	pid_t pid = getValidPid(&i);
    cur_process = &procs[i];

    cur_process->pid = pid;
    cur_process->parent = NULL;
    cur_process->state = PROC_RUN;
	cur_process->priority = 0;
	cur_process->stack = 0;
	cur_process->pd = (void*)read_cr3();

	list_push_back(&p_list, &cur_process->elem_all);
	list_push_back(&r_list, &cur_process->elem_stat);
}

pid_t getValidPid(int *idx) {

	pid_t pid = -1;
	int i;

	while(lock_pid_simple)
		;

	lock_pid_simple++;

	// find unuse process pid and return it
	for(i = 0; i < PROC_NUM_MAX; i++)
	{
		int tmp = i + lately_pid + 1;// % PROC_NUM_MAX;
		if(procs[tmp % PROC_NUM_MAX].state == PROC_UNUSED) { // find out valid state;
			pid = lately_pid + 1;
			*idx = tmp % PROC_NUM_MAX;
			break;
		}
	}

	if(pid != -1)
	{
		lately_pid = pid;	
	}

	lock_pid_simple = 0;

	return pid;
}

pid_t proc_create(proc_func func, struct proc_option *opt, void* aux, void* aux2)
{
	struct process *p;
	int idx;

	enum intr_level old_level = intr_disable();

	pid_t pid = getValidPid(&idx);
	p = &procs[idx];

	p->pid = pid;
	p->state = PROC_RUN;

	if(opt != NULL)
		p->priority = opt->priority;   
	else
		p->priority = (unsigned char)0;

	p->time_used = 0;
	p->time_sched= 0;
	p->parent = cur_process;
	p->simple_lock = 0;
	p->child_pid = -1;
	p->pd = pd_create(pid);
	
	int i;
	for(i=0;i<NR_FILEDES;i++)
		p->file[i] = NULL;

	//init stack
    int *top = (int*)palloc_get_page();
	int stack = (int)top;
	top = (int*)stack + STACK_SIZE - 1;

	*(--top) = (int)aux2;		//argument for func
	*(--top) = (int)aux; 
	*(--top) = (int)proc_end;	//return address from func
	*(--top) = (int)func;		//return address from proc_start
	*(--top) = (int)proc_start; //return address from switch_process


	*(--top) = (int)((int*)stack + STACK_SIZE - 1); //ebp
	*(--top) = 1; //eax
	*(--top) = 2; //ebx
	*(--top) = 3; //ecx
	*(--top) = 4; //edx
	*(--top) = 5; //esi
	*(--top) = 6; //edi

	p->stack = top;
	p->elem_all.prev = NULL;
	p->elem_all.next = NULL;
	p->elem_stat.prev = NULL;
	p->elem_stat.next = NULL;

	/* P7 */
	p->rootdir = p->parent->rootdir;
	p->cwd = p->parent->cwd;

	list_push_back(&p_list, &p->elem_all);
	list_push_back(&r_list, &p->elem_stat);

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

	list_remove(&cur_process->elem_stat);

	cur_process->state = PROC_ZOMBIE;	//change state
	list_push_back(&d_list, &cur_process->elem_stat);

}

void proc_end(void)
{
	proc_free();
	schedule();
	printk("never reach\n");
	return;	//never reach
}

void proc_wake(void)
{
	struct process* p;
	unsigned long long t = get_ticks();

    while(!list_empty(&s_list))
	{
		p = list_entry(list_front(&s_list), struct process, elem_stat);
		if(p->time_sleep > t)
			break;
		//proc_unblock(p);
		p->state = PROC_RUN;
		list_remove(&p->elem_stat);
	}
}

void proc_sleep(unsigned ticks)
{
	unsigned long cur_ticks = get_ticks();
	cur_process->time_sleep =  ticks + cur_ticks;
	cur_process->state = PROC_STOP;
	list_insert_ordered(&s_list, &cur_process->elem_stat,
			less_time_sleep, NULL);
	schedule();
}

void proc_block(void)
{
	cur_process->state = PROC_BLOCK;
	schedule();	
}

void proc_unblock(struct process* proc)
{
	enum intr_level old_level;

	old_level = intr_disable();

	list_push_back(&r_list, &proc->elem_stat);
	proc->state = PROC_RUN;

	intr_set_level(old_level);
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
	cur_process -> priority = 200;
	while(1)
	{
		schedule();
	}
}

void kernel2_proc(void* aux)
{
	cur_process -> priority = 200;
	while(1)
	{
		schedule();
	}
}

void ps_proc(void* aux)
{
	int i;
	for(i = 0; i<PROC_NUM_MAX; i++)
	{
		struct process *p = &procs[i];

		if(p->state == PROC_UNUSED)
			continue;

		printk("pid %d ppid ", p->pid);

		if(p->parent != NULL)
			printk("%d", p->parent->pid);
		else
			printk("non");

		printk(" state %d prio %d using time %d sched time %d\n",
				p->state, p->priority, p->time_used, p->time_sched);

	}
	exit(1);
}

extern const char* VERSION;
extern const char* AUTHOR;
extern const char* MODIFIER;
void uname_proc(void* aux)
{
	printk("SSUOS %s\nmade by %s\nmodefied by %s\n", VERSION, AUTHOR, MODIFIER);	

}

void print_pid(void* aux) {

	while(1) {
		printk("pid = %d ", cur_process->pid);
		printk("prio = %d ", cur_process->priority);
		printk("time = %d ", cur_process->time_slice);
		printk("ticks = %d ", get_ticks());
		printk("in %s\n", aux);

#define SLEEP_FREQ 100
		proc_sleep(cur_process->pid * cur_process->pid * SLEEP_FREQ);
	}
}

void open_proc(void *aux)
{
	char* name = (char*)aux;
	open(name,0);
}

void write_proc(void *aux)
{
	char *name = (char *)aux;
	int fd;
	fd = open(name,O_WRONLY);
	write(fd,"ssuos:oslab",11);
}

void ls_proc(void *aux)
{
	list_segment(cur_process->cwd);
}

void cat_proc(void *aux)
{
	char buf[21] = {0};
	int fd;
	int length;
	fd = open((char *)aux,O_RDONLY);
	if (fd < 0) return;
	length = read(fd, buf, 20);
	printk("%s\n",buf);
}

void lseek_proc(void *aux , void *filename)
{

	char buf[BUFSIZ] = {0};
	int fd;
	fd = open(filename, O_RDWR);
	if (fd < 0 )return;
	write(fd , "ssuos ",6);
	//if you add lseek() system call , remove the '//'
//	printk ("%d \n ", lseek(fd, -3, SEEK_CUR));  
	write(fd, "world",5);
//	lseek(fd, 0, SEEK_SET);
	read(fd , buf, 8);
	printk("%s\n", buf);
	//lseek(fd, -9, SEEK_END);
	read(fd, buf, 9);
	printk("%s\n", buf);

	//ssuworld 가 정확히 출력되어야 함
	// 옵션에 대한 시나리오 및 검증할 코드 아래에 추가
	// 각 옵션에 대해 파일 크기 및 내용이 정확하게 채워지는지 보여야 함
	/*   option  */
}


typedef struct
{
	char* cmd;
	unsigned char type;	//0 : 직접실행, 1 : fork 함수실행
	void* func;
} CmdList;

void shell_proc(void* aux)
{
	//<<<HW>>>
	CmdList cmdlist[] = {
		{"shutdown", 0, shutdown}
		//{"ps", 1, ps_proc},
		//{"uname", 1, uname_proc}
		//{"write", 1, write_proc}
		,{"ls", 1, ls_proc}
		,{"touch",1,open_proc}
		,{"test",1, lseek_proc}
		,{"cat",1,cat_proc}
	};
#define CMDNUM 5
#define TOKNUM 3
	char buf[BUFSIZ];
	char token[TOKNUM][BUFSIZ];
	int token_num;
	struct direntry cwde;
	int fd;
	while(1)
	{
		proc_func *func;
		int i,j, len;
		/* cwd 출력*/
		if(cur_process->cwd == cur_process->rootdir)
			printk("~");
		else
			printk("%s", cwde.de_name);

		printk("> ");

		for(i=0;i<BUFSIZ;i++) 
		{
			buf[i] = 0;
			for(j=0;j<TOKNUM;j++)
				token[j][i] = 0;
		}
		
		while(getkbd(buf,BUFSIZ))
		{
			; 
		}
		
		for(i=0;buf[i] != '\n'; i++); 
		for(i--; buf[i] == ' '; i--)
			buf[i] = 0;

//		for( i = 0 ; buf[i] == ' ' ; i++)
//			buf[i] = 0;

		token_num = getToken(buf,token,TOKNUM);


		if( strcmp(token[0], "exit") == 0)
			break;

		if( strncmp(token[0], "list", BUFSIZ) == 0)
		{
			for(i = 0; i < CMDNUM; i++)
				printk("%s\n", cmdlist[i].cmd);
			continue;
		}

		if( strncmp(token[0], "cd", BUFSIZ) == 0)
		{
			if(change_dir(cur_process->cwd, token[1]) == 0)
			{
				get_curde(cur_process->cwd, &cwde);
			}
			continue;
		}
		
		for(i = 0; i < CMDNUM; i++)
		{
			if( strncmp(cmdlist[i].cmd, token[0], BUFSIZ) == 0)
				break;
		}

		if(i == CMDNUM)
		{
			printk("Unknown command %s\n", buf);
			continue;
		}

		if(cmdlist[i].type == 0)
		{
			void (*func)(void*);
			func = cmdlist[i].func;
			func((void*)0x9);
		}
		else if(cmdlist[i].type == 1)
		{
			cur_process->simple_lock = 1;
			int pid = fork(cmdlist[i].func, token[1], token[2]);

			while(cur_process->simple_lock)
				;
		}
		else
		{
			printk("Unknown type\n");
			continue;
		}
	}
}

void login_proc(void* aux)
{
	int i,fd;
	char id[30];
	char password[30];
	char buf[30];
	char buf2[] = "ssuos:oslab";

	cur_process -> priority = 100;
	
	//fd = open("passwd",O_RDWR);
	//write(fd,buf2,11);
	//fd = open("passwd",O_RDWR);
	//read(fd,buf,30);	
	while(1)
	{
		for(i=0;i<30;i++) {
			id[i] = 0;
			password[i] = 0;
		}
		printk("id : ");
		while(getkbd(id,BUFSIZ));
	    
		printk("password : ");
	    while(getkbd(password,BUFSIZ));
		
		if(id[6] != 0 || strncmp(id,buf2,5) != 0) {printk("%s\n",id); continue;}
		if(password[6] != 0 || strncmp(password,buf2+6,5) != 0) {printk("%s\n",password); continue;}
		shell_proc(NULL);
	}

}

void idle(void* aux)
{
	proc_create(kernel1_proc, NULL, NULL, NULL);
	proc_create(kernel2_proc, NULL, NULL, NULL);
	proc_create(login_proc, NULL, NULL, NULL);

	while(1) {  
		if(cur_process->pid != 0) {
			printk("error : idle process's pid != 0\n", cur_process->pid);
			while(1);
		}

		while( !list_empty(&d_list) )
		{
			struct list_elem *e = list_pop_front(&d_list);
			struct process *p = list_entry(e, struct process, elem_stat);
			p->state = PROC_UNUSED;
			list_remove( &p->elem_all);
		}

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

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printk ("  %s\n", buff);

            // Output the offset.
            printk ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printk (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printk ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printk ("  %s\n", buff);
}


