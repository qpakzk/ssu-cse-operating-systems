#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "type.h"

#pragma pack(push, 1)

typedef struct kChStruct
{
		unsigned char bCh;
		unsigned char bAtt;
} CHAR;

int Glob_x;
int Glob_y;

#pragma pack(pop)

#define SCREEN_SCROLL
#define SERIAL_STDOUT

#define HSCREEN 80
#define VSCREEN 25
#define SIZE_SCREEN HSCREEN * VSCREEN
#define NSCROLL 100
#define SIZE_SCROLL NSCROLL * HSCREEN
#define VIDIO_MEMORY 0xB8000

#define IO_BASE 0x3F8               /* I/O port base address for the first serial port. */
#define FIRST_SPORT (IO_BASE)
#define LINE_STATUS (IO_BASE + 5)   /* Line Status Register(read-only). */
#define THR_EMPTY 0x20              /* Transmitter Holding Reg Empty. */

#define buf_e (buf_w + SIZE_NSCROLL)
#define SCROLL_END buf_s + SIZE_SCROLL
// Foreground 프로세스마다 입출력 버퍼 할당 시 아래 메크로로 변경
// #define buf_e (cur_console->buf_w + SIZE_NSCROLL)
// #define SCROLL_END (cur_console->buf_s + SIZE_SCROLL)

#define MAX_CONSOLE_NUM 10

typedef struct Console{
	char buf_s[SIZE_SCROLL]; 
	int Glob_x;
	int Glob_y;
	char *buf_w;	
	char *buf_p;	
	int sum_y;
	bool a_s;
	bool used;
}Console;

void init_console(void);

void PrintCharToScreen(int x, int y, const char *pString);
void PrintChar(int x, int y, const char String);

void clrScreen(void);
void clearScreen(void);
void scroll(void);
int printk(const char *fmt, ...);

#ifdef SCREEN_SCROLL
void refreshScreen(void);
void scroll_screen(int offset);
void set_fallow(void);
void set_cursor(void);
#endif

#endif
