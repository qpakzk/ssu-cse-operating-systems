#include <device/kbd.h>
#include <type.h>
#include <device/console.h>
#include <interrupt.h>
#include <device/io.h>
#include <ssulib.h>
#include <proc/proc.h>
#include <string.h>

static Key_Status KStat;

extern struct process *cur_foreground_process;
Kbd_buffer kbd_buffer[MAX_KBD_BUFFER];

static BYTE Kbd_Map[4][KBDMAPSIZE] = {
	{ /* default */
		0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0x00, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0x00, '\\', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* capslock */
		0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0x00, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0x00, '\\', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* Shift */
		0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 0x00,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x00, 0x00, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0x00, '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* Capslock & Shift  */
		0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 0x00,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 0x00, 0x00, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 0x00, '|', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', '<', '>', '?', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}
};

static bool kbd_remove_char();

void init_kbd(void)
{
	KStat.ShiftFlag = 0;
	KStat.CapslockFlag = 0;
	KStat.NumlockFLag = 0;
	KStat.ScrolllockFlag = 0;
	KStat.ExtentedFlag = 0;
	KStat.PauseFlag = 0;
	KStat.CtrlFlag = 0;

	for(int i = 0; i < MAX_KBD_BUFFER; i++)
	{
		memset(kbd_buffer[i].buf, 0x00, MAX_KBD_BUFFER);
		kbd_buffer[i].head = 0;
		kbd_buffer[i].tail = 0;
		kbd_buffer[i].used = false;
	}

	reg_handler(33, kbd_handler);
}

void UpdateKeyStat(BYTE Scancode)
{
	if(Scancode & 0x80) // release
	{
		if(Scancode == 0xB6 || Scancode == 0xAA)
		{
			KStat.ShiftFlag = FALSE;
		}

		if(Scancode == 0x9D)
		{
			KStat.CtrlFlag = FALSE;
		}
	}
	else // press
	{
		if(Scancode == 0x3A && KStat.CapslockFlag)
		{
			KStat.CapslockFlag = FALSE;
		}
		else if(Scancode == 0x3A)
			KStat.CapslockFlag = TRUE;
		else if(Scancode == 0x36 || Scancode == 0x2A)
		{
			KStat.ShiftFlag = TRUE;
		}

		if(Scancode == 0x1D)
		{
			KStat.CtrlFlag = TRUE;
		}

	}

	if(Scancode == 0xE0)
	{
		KStat.ExtentedFlag = TRUE;
	}
	else if(KStat.ExtentedFlag == TRUE && Scancode != 0xE0)
	{
		KStat.ExtentedFlag = FALSE;
	}
}

BOOL ConvertScancodeToASCII(BYTE Scancode, BYTE *Asciicode)
{
	if(KStat.PauseFlag > 0)
	{
		KStat.PauseFlag--;
		return FALSE;
	}

	if(KStat.ExtentedFlag == TRUE)
	{
		if(Scancode & 0x80)	
			return FALSE;
		*Asciicode = Scancode;
		return TRUE;
	}

	if(Scancode == 0xE1)
	{
		*Asciicode = 0x00;
		KStat.PauseFlag = 2;
		return FALSE;
	}
	else if(Scancode == 0xE0)
	{
		*Asciicode = 0x00;
		return FALSE;
	}

	if(!(Scancode & 0x80))
	{
		if(KStat.ShiftFlag & KStat.CapslockFlag)
		{
			*Asciicode = Kbd_Map[3][Scancode & 0x7F];
		}
		else if(KStat.ShiftFlag)
		{
			*Asciicode = Kbd_Map[2][Scancode & 0x7F];
		}
		else if(KStat.CapslockFlag)
		{
			*Asciicode = Kbd_Map[1][Scancode & 0x7F];
		}
		else
		{
			*Asciicode = Kbd_Map[0][Scancode];
		}

		return TRUE;
	}
	return FALSE;
}

bool isFull()
{
	int *head = &cur_foreground_process->kbd_buffer->head;
	int *tail = &cur_foreground_process->kbd_buffer->tail;

	return (*head-1) % BUFSIZ == *tail;
}

bool isEmpty()
{
	int head = cur_foreground_process->kbd_buffer->head;
	int tail = cur_foreground_process->kbd_buffer->tail;

	return head == tail;
}

void kbd_handler(struct intr_frame *iframe)
{
	BYTE asciicode;
	BYTE data = inb(0x60);

	if(ConvertScancodeToASCII(data, &asciicode))
	{

#ifdef SCREEN_SCROLL
		if( KStat.ExtentedFlag == TRUE)
		{
			switch(asciicode)
			{
				case 72://UP
					break;
				case 80 ://DOWN
					break;
				case 75 ://LEFT
					break;
				case 77 ://RIGHT
					break;
				case 73 :// PageUp
					scroll_screen(-1);
					break;
				case 81 :// PageDown
					scroll_screen(+1);
					break;
				case 79 ://End
					set_fallow();
					break;
				default:
					break;
			}
		}
		else if( !isFull() && asciicode != 0)
		{
			if(KStat.CtrlFlag == FALSE || data != 0x26 && data != 0x0F)
			{
				int *tail = &cur_foreground_process->kbd_buffer->tail;
				char *kbd_buf = cur_foreground_process->kbd_buffer->buf;

				kbd_buf[*tail] = asciicode;
				*tail = (*tail + 1) % BUFSIZ;
			}
		}

#endif

	}

	if(KStat.CtrlFlag == TRUE && data == 0x26)
	{
		clearScreen();
	}

	if(KStat.CtrlFlag == TRUE && data == 0x0F)
	{
		next_foreground_proc();
	}

	UpdateKeyStat(data);
}

char kbd_read_char()
{
	int *head = &cur_foreground_process->kbd_buffer->head;
	char *kbd_buf = cur_foreground_process->kbd_buffer->buf;

	if( isEmpty())
		return -1;

	char ret;
	ret = kbd_buf[*head];
	*head = (*head + 1)%BUFSIZ;

	return ret;
}

//입력 버퍼 할당
struct Kbd_buffer *get_kbd_buffer(){
	int i;

	for(i = 0; i < MAX_KBD_BUFFER; i++){
		if(kbd_buffer[i].used == false){
			kbd_buffer[i].used = true;
			return &kbd_buffer[i];
		}
	}

	return NULL;
}
