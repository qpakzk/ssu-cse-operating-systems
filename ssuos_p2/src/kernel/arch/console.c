#include <interrupt.h>
#include <console.h>
#include <type.h>
#include <kbd.h>
#include <io.h>
#include <pit.h>
#include <stdarg.h>

#define HSCREEN 80
#define VSCREEN 25
#define SIZE_SCREEN HSCREEN * VSCREEN
#define NSCROLL 30
#define SIZE_SCROLL NSCROLL * HSCREEN
#define VIDIO_MEMORY 0xB8000

#define IO_BASE 0x3F8				/* I/O port base address for the first serial port. */
#define FIRST_SPORT (IO_BASE)
#define LINE_STATUS (IO_BASE + 5)	/* Line Status Register(read-only). */
#define THR_EMPTY 0x20				/* Transmitter Holding Reg Empty. */

char next_line[2] = "\r\n";

char buf_s[SIZE_SCROLL];
char *buf_w = buf_s;
char *buf_p = buf_s;

bool a_s = TRUE;

#define buf_e (buf_w + SIZE_NSCROLL)
#define SCROLL_END buf_s + SIZE_SCROLL

char* scr_end = buf_s + SIZE_SCROLL;

void refreshScreen(void)
{
	CHAR *p_s= (CHAR *) VIDIO_MEMORY;
	int i;

	if(a_s)
		buf_p = buf_w;

	char* b = buf_p;

	for(i = 0; i < SIZE_SCREEN; i++, b++, p_s++)
	{
		if(b >= SCROLL_END)
			b -= SIZE_SCROLL;
		if(*b != NULL){
			p_s->bAtt = 0x07;
			p_s->bCh = *b;
		}
	}
}

void printCharToSerial(const char *pString)
{
	int i;
	cli();
	for(;*pString != NULL; pString++)
	{
		if(*pString != '\n'){
			while((inb(LINE_STATUS) & THR_EMPTY) == 0)
				continue;
			outb(FIRST_SPORT, *pString);
			
		}
		else{
			for(i=0; i<2; i++){
				while((inb(LINE_STATUS) & THR_EMPTY) == 0)
					continue;
				outb(FIRST_SPORT, next_line[i]);
			}
		}
	}
	sti();
}

void PrintCharToScreen(int x, int y, const char *pString) 
{
	Glob_x = x;
	Glob_y = y;
	int i = 0;
	while(pString[i] != 0)
	{
		PrintChar(Glob_x++, Glob_y, pString[i++]);
	}
}

void PrintChar(int x, int y, const char String) 
{
	int i = 0;
	if (String == '\n') {
		if((y+1) > VSCREEN) {
			scroll();
			y--;
		}
		Glob_x = 0;
		Glob_y = y+1;

		return;
	}
	else {
		if ((y >= VSCREEN) && (x >= 0)) {
			scroll();
			x = 0; y--;
		}      	              	

		char* b = &buf_w[y * HSCREEN + x];
		if(b >= SCROLL_END)
			b-= SIZE_SCROLL;
		*b = String;

		if(Glob_x >= HSCREEN)
		{
			Glob_x = 0;
			Glob_y++;
		}    
	}
}

void clrScreen(void) 
{
	CHAR *pScreen = (CHAR *) 0xb8000;
	int i;

	for (i = 0; i < 80*25; i++) {
		(*pScreen).bAtt = 0x07;
		(*pScreen++).bCh = ' ';
	}   
	Glob_x = 0;
	Glob_y = 0;
}

void scroll(void) 
{
	buf_w += HSCREEN;
	while(buf_w > SCROLL_END)
		buf_w -= SIZE_SCROLL;

	Glob_y--;
}

int printk(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int len;

	va_start(args, fmt);
	len = vsprintk(buf, fmt, args);
	va_end(args);
	
	printCharToSerial(buf);
	PrintCharToScreen(Glob_x, Glob_y, buf);

	return len;
}

void scroll_screen(int offset)
{
	if(a_s == TRUE && offset > 0 && buf_p == buf_w)
		return;

	a_s = FALSE;

	buf_p = (char*)((int)buf_p + (HSCREEN * offset));
	if(buf_p >= SCROLL_END)
		buf_p = (char*)((int)buf_p - SIZE_SCROLL);
	else if(buf_p < buf_s)
		buf_p = (char*)((int)buf_p + SIZE_SCROLL);
}

void set_fallow(void)
{
	a_s = TRUE;
}
