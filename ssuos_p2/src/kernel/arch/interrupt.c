#include <interrupt.h>
#include <console.h>
#include <type.h>
#include <pit.h>
#include <sched.h>
#include <io.h>
#include <kbd.h>
#include <proc.h>
#include <ssulib.h>
#include <switch.h>

static unsigned long ticks = 0;

#define TIMER_MAX 10
#define REFRESH_FPS 20

unsigned long get_ticks(void)
{
	return ticks;
}

void default_handler(int int_number) 
{
	cli();
	reg_push();

	PrintChar(Glob_x++, Glob_y, '@');
	outb(0x20, 0x20);
	reg_pop();
	sti();
	iret();
}

void timer_handler(int int_number)
{
	cli();
	reg_push();

	ticks++;

	cur_process->time_slice++;
	if(cur_process->time_slice >= TIMER_MAX)
	{
		schedule();
	}

	static unsigned long refresh_ticks = 0;
	if(++refresh_ticks >= (PIT_FRQ_HZ/REFRESH_FPS))
	{
	   refresh_ticks = 0;
	   refreshScreen();
	}

	outb(0x20, 0x20);
	reg_pop();
	sti();
	iret();
}

void kbd_handler(int int_number)
{
	cli();
	reg_push();
	BYTE asciicode;
	BYTE data = inb(0x60);

	UpdateKeyStat(data);
	if(ConvertScancodeToASCII(data, &asciicode))
	{
		switch(asciicode)
		{
			case 'u':
				scroll_screen(-1);
				break;
			case 'd' :
				scroll_screen(+1);
				break;
			case 'f':
				set_fallow();
				break;
			default:
				break;
		}
	}

	outb(0x20, 0x20);
	reg_pop();
	sti();
	iret();
}

int init_int(void)
{
	int i;

	cli();
	/* IDT 초기화 */
	for (i = 0 ; i <= 48 ; i++)
	{
		IDT[i].LowOffset = (unsigned short)((unsigned long)default_handler & 0xffff);
		IDT[i].CodeSelector = (unsigned short)CODE_SEGMENT;
		IDT[i].Type = (unsigned short)0x8e00;      // P=1, DPL=00, D=1
		IDT[i].HighOffset = (unsigned short)(((unsigned long)default_handler >> 16) & 0xffff);
	}

	/* IDTR 설정 */
	IDTR.Size = (unsigned short)((INTERRUPT_NUM * 8) - 1);
	IDTR.Address = (unsigned long)&IDT;

	reg_handler(32, (unsigned long)timer_handler);
	printk("%s", "Timer Handler Registration\n");

	reg_handler(33, (unsigned long)kbd_handler);
	printk("%s", "Keyboard Handler Registeration\n");

	/* IDTR 등록 */
	__asm__ __volatile("lidt %0":"=m" (IDTR));

	sti();
	return 1;
}

void reg_handler(unsigned short idx, unsigned long handler) 
{
	//cli();
	IDT[idx].LowOffset = (unsigned short)((unsigned long)handler & 0xffff);
	IDT[idx].CodeSelector = (unsigned short)CODE_SEGMENT;
	IDT[idx].Type = (unsigned short)0x8e00;      // P=1, DPL=00, D=1
	IDT[idx].HighOffset = (unsigned short)(((unsigned long)handler >> 16) & 0xffff);

	/* IDTR 설정 */
	IDTR.Size = (unsigned short)((INTERRUPT_NUM * 8) - 1); 
	IDTR.Address = (unsigned long)&IDT;

	return;
}
