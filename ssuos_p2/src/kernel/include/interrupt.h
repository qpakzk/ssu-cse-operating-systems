#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#pragma pack(push, 1)

#define INTERRUPT_NUM   255
#define CODE_SEGMENT    0x08

#define cli()   __asm__ __volatile("cli")
#define sti()   __asm__ __volatile("sti")

#define iret()  __asm__ __volatile("leave\n\tiret")

#define reg_push() __asm__ __volatile(\
		"pushl %eax\n\tpushl %ebx\n"\
		"pushl %ecx\n\tpushl %edx\n"\
		"pushl %ebp\n\tpushl %esi\n"\
		"pushl %edi"\
		)

#define reg_pop() __asm__ __volatile(\
		"popl %edi\n\tpopl %esi\n" \
		"popl %ebp\n\tpopl %edx\n" \
		"popl %ecx\n\tpopl %ebx\n" \
		"popl %eax" \
		)     

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

_ID     IDT[INTERRUPT_NUM];
_IDTR   IDTR;

#pragma pack(pop)

int init_int(void);
void reg_handler(unsigned short idx, unsigned long handler);
unsigned long get_ticks(void);

#endif
