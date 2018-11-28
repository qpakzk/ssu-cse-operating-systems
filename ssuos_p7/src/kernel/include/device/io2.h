#ifndef	   __IO_H__
#define	   __IO_H__

#include <stddef.h>
#include <stdint.h>

extern void outb(unsigned short port, unsigned char data);
extern unsigned char inb(unsigned short port);
extern void outl(unsigned short port, unsigned int data);
extern unsigned int inl(unsigned short port);
extern void iowait();

extern int read_cr0(void);
extern int write_cr0(unsigned data);
extern int read_cr3(void);
extern int write_cr3(unsigned data);
extern int read_cr4(void);
extern int write_cr4(unsigned data);

extern void dev_shutdown(void);


static inline uint16_t
inw (uint16_t port)
{
	uint16_t data;
	asm volatile("inw	%w1, %w0" : "=a"(data) : "d"(port));
	return data;
}

static inline void
insw (uint16_t port, void *addr, size_t cnt)
{	
	int i;
	char* buf = addr;
	uint16_t data;
	for(i = 0; i < cnt; i++)
	{
		data = inw(port);
		*(buf+i*2+1) = (unsigned char) ((data)>>8);
		*(buf+i*2) = (unsigned char) (data);
	}
   // asm volatile ("rep insw" : "+D" (addr), "+c" (cnt) : "d" (port) : "memory");
}

static inline void
outsw (uint16_t port, const void *addr, size_t cnt)
{
	asm volatile ("rep outsw" : "+S" (addr), "+c" (cnt) : "d" (port));
}

#endif
