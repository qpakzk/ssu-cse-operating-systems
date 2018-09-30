#include <interrupt.h>
#include <console.h>
#include <type.h>
#include <kbd.h>
#include <io.h>
#include <pit.h>
#include <stdarg.h>
#include <ssulib.h>
#include <proc.h>
#include <mm.h>

void main_init(void);

void OslabMain(void)
{
	main_init();

	while(1);

	return;
}

void main_init(void)
{
	Glob_x = 0;
	Glob_y = 2;

	printk("%s", "C OslabMain start!!!!\n");

    detect_mem();
    printk("%s", "Memory Detecting\n");
    printk("-Memory size = %u bytes\n", memSize());
    printk("-Memory size = %u Kbytes\n", memSize()/1024);
    printk("-Memory size = %u Mbytes\n", memSize()/(1024*1024));

	init_pit();	
	printk("%s", "PIT Intialization\n");

	init_int();
	printk("%s", "Interrupt Intialization\n"); 

	proc_init();
	printk("%s", "Process Initialization\n");

	refreshScreen();
}
