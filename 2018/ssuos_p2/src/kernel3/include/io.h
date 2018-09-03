#ifndef	   __IO_H__
#define	   __IO_H__

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
#endif
