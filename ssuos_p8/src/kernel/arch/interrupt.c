#include <interrupt.h>
#include <device/console.h>
#include <type.h>
#include <device/pit.h>
#include <proc/sched.h>
#include <device/io.h>
#include <device/kbd.h>
#include <proc/proc.h>
#include <ssulib.h>
#include <proc/switch.h>
#include <syscall.h>
#include <intr-stubs.h>

//#define INTR_MAX   256
#define TIMER_MAX 10
#define REFRESH_FPS 20
#define CODE_SEGMENT    0x08

#pragma pack(push, 1)

typedef struct _ID
{
	unsigned short LowOffset;
	unsigned short CodeSelector;
	unsigned short Type;
	unsigned short HighOffset;
} _ID;

typedef struct _IDTR
{
	unsigned short Size;
	unsigned long Address;
} _IDTR;
#pragma pack(pop)

_ID     *idt;

#ifndef ASSERT
#define ASSERT(CONDITION) ( (void*) 0)
#endif

static unsigned long ticks;

intr_handler_func *handlers[INTR_MAX];	//실제 인터럽트 핸들러 들
bool sched_on_return;		//인터럽트가 끝날 때 schedule 할 지 결정

static bool in_external_intr;   /* Are we processing an external interrupt? */

void default_handler(struct intr_frame *iframe);
void timer_handler(struct intr_frame *iframe);
void syscall_handler(struct intr_frame *iframe);

int init_intr(void)
{
	int i;

	ticks = 0;
	in_external_intr = false;
	sched_on_return = false;

	enum intr_level old_level = intr_disable();

	_IDTR   IDTR;

	__asm__ __volatile("sidt %0"::"m" (IDTR));
	printk("idtr size : %d address : 0x%x\n", IDTR.Size, IDTR.Address);
	idt = (_ID *)IDTR.Address;

	for(i = 0; i<INTR_MAX; i++)
	{
		idt[i].LowOffset = (unsigned short)((unsigned long)intr_stubs[i] & 0xffff);
		idt[i].CodeSelector = (unsigned short)CODE_SEGMENT;
		if(i == 5)
			idt[i].Type = (unsigned short)0x9f00;
		else
			idt[i].Type = (unsigned short)0x8e00;      // P=1, DPL=00, S=1
		idt[i].HighOffset = (unsigned short)(((unsigned long)intr_stubs[i] >> 16) & 0xffff);

		handlers[i] = default_handler;
	}

	reg_handler(32, timer_handler);
	printk("%s", "Timer Handler Registration\n");

	/* register syscall handler */
	reg_handler(0x30, syscall_handler);
	printk("%s", "System Call Handler Registeration\n");

	intr_set_level (old_level);
	return 1;
}

void intr_common(struct intr_frame *iframe)
{
	in_external_intr = iframe->vec_no >= 0x20 && iframe->vec_no < 0x30;
	
	//test external interrupt
	if(in_external_intr)
	{
		ASSERT (intr_get_level () == INTR_OFF);
		sched_on_return = false;
	}

	//call handler
	if(handlers[iframe->vec_no] != NULL)
		handlers[iframe->vec_no](iframe);
	else
		default_handler(iframe);

	if(in_external_intr)
	{
		ASSERT (iframe->vec_no >= 0x20 && iframe->vec_no < 0x30);

		in_external_intr = false;

		/* Acknowledge master PIC. */
		outb(0x20, 0x20);

		/* Acknowledge slave PIC if this is a slave interrupt. */
		if(iframe->vec_no >= 0x28)
			outb(0xa0, 0x20);

		if(sched_on_return)
			schedule();
	}
}

void do_sched_on_return(void)
{
	ASSERT (intr_context ());
	sched_on_return = true;
}

void reg_handler(unsigned short idx, intr_handler_func *handler) 
{
	enum intr_level old_level = intr_disable();

	handlers[idx] = handler;
	//SSUMUST : 인터럽트 / 트랩 구분

	intr_set_level (old_level);
	return;
}

/*
   intr level control
   SSUOS:CFP
   ------------------------------
*/

/* EFLAGS Register. */
#define FLAG_MBS  0x00000002    /* Must be set. */
#define FLAG_IF   0x00000200    /* Interrupt Flag. */ 

/* Returns the current interrupt status. */
enum intr_level intr_get_level (void) 
{
	uint32_t flags;

	/* Push the flags register on the processor stack, then pop the
	   value off the stack into `flags'.  See [IA32-v2b] "PUSHF"
	   and "POP" and [IA32-v3a] 5.8.1 "Masking Maskable Hardware
	   Interrupts". */
	asm volatile ("pushfl; popl %0" : "=g" (flags));

	return flags & FLAG_IF ? INTR_ON : INTR_OFF;
}

/* Enables or disables interrupts as specified by LEVEL and
   returns the previous interrupt status. */
enum intr_level intr_set_level (enum intr_level level) 
{
	return level == INTR_ON ? intr_enable () : intr_disable ();
}

/* Enables interrupts and returns the previous interrupt status. */
enum intr_level intr_enable (void) 
{
	enum intr_level old_level = intr_get_level ();
	ASSERT (!intr_context ());

	/* Enable interrupts by setting the interrupt flag.

	   See [IA32-v2b] "STI" and [IA32-v3a] 5.8.1 "Masking Maskable
	   Hardware Interrupts". */
	asm volatile ("sti");

	return old_level;
}

/* Disables interrupts and returns the previous interrupt status. */
enum intr_level intr_disable (void) 
{
	enum intr_level old_level = intr_get_level ();

	/* Disable interrupts by clearing the interrupt flag.
	   See [IA32-v2b] "CLI" and [IA32-v3a] 5.8.1 "Masking Maskable
	   Hardware Interrupts". */
	asm volatile ("cli" : : : "memory");

	return old_level;
}

/* Returns true during processing of an external interrupt
   and false at all other times. */
bool intr_context (void)
{
	return in_external_intr;
}

/* ------------------------------ SSUOS:CFP */

unsigned long get_ticks(void)
{
	return ticks;
}

void default_handler(struct intr_frame *iframe)
{
	//enum intr_level old_level = intr_disable();

	printk("\n\nint intr %d\n\n", iframe->vec_no);

	//PrintChar(Glob_x++, Glob_y, '@');
	//intr_set_level (old_level);
}

void timer_handler(struct intr_frame *iframe)
{
	ticks++;	//add tick
	cur_process->time_used++;	// 총 실행시간 추가
	cur_process->time_slice++;	// 스케줄 전까지 사용 시간 추가

	if(cur_process->time_slice >= TIMER_MAX)
		//프로세스가 시간을 다 쓰면, 인터럽트 처리 후 스케줄.
		do_sched_on_return();

#ifdef SCREEN_SCROLL
	//if time out, screen are refreshed
	static unsigned long refresh_ticks = 0;
	if(++refresh_ticks >= (PIT_FRQ_HZ/REFRESH_FPS)) {
		refresh_ticks = 0;
		refreshScreen();
	}
#endif
}

void syscall_handler(struct intr_frame *iframe)
{
	long sys_num, arg_num, ret;
	int *pt;
	int i;

#define SYSCALL_ARG_MAX 4
	long arg[SYSCALL_ARG_MAX];

	pt = (int*) (((int)&iframe->ss) - 4);
	sys_num = *pt++;
	arg_num = syscall_tbl[sys_num][1];

	for(i = 0; i<arg_num; i++)
		arg[i] = *pt++;

	switch(arg_num)
	{
		case 0:
			ret = ((int(*)(void))syscall_tbl[sys_num][0])();
			break;
		case 1:
			ret = ((int(*)(int))syscall_tbl[sys_num][0])(arg[0]);
			break;
		case 2:
			ret = ((int(*)(int, int))syscall_tbl[sys_num][0])(arg[0], arg[1]);
			break;
		case 3:
			ret = ((int(*)(int, int, int))syscall_tbl[sys_num][0]) (arg[0], arg[1], arg[2]);
			break;
	}
	
	*(pt + 3) = ret;		// 리턴값 넣어주기.
}
