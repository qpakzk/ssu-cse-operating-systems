#include <kbd.h>
#include <type.h>
#include <console.h>

static Key_Status KStat = {0,};

static BYTE Kbd_Map[4][KBDMAPSIZE] = {
	{ /* 기본 값 */
		0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x00, '\t',
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0x00, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0x00, '\\', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* capslock 눌린 상태 */
		0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x00, '\t',
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0x00, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0x00, '\\', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* Shift 눌린 상태 */
		0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x00, 0x00,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x00, 0x00, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0x00, '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	{ /* Capslock & Shift 눌린 상태 */
		0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x00, 0x00,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 0x00, 0x00, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 0x00, '|', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', '<', '>', '?', 0x00, 0x00, 0x00, ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '-', 0x00, 0x00, 0x00, '+', 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}
};

// 넘버락 스크롤락 추가 필요
void UpdateKeyStat(BYTE Scancode)
{
	/* up key */
	if(Scancode & 0x80)
	{
		/* Shift 키 입력 확인 */
		if(Scancode == 0xB6 || Scancode == 0xAA)
		{
			KStat.ShiftFlag = FALSE;
		}
	}
	/* down key */
	else
	{
		/* Caps lock 키 입력 확인 LED도 끄고 켜야 함 */
		if(Scancode == 0x3A && KStat.CapslockFlag)
		{
			KStat.CapslockFlag = FALSE;
		}
		else if(Scancode == 0x3A)
			KStat.CapslockFlag = TRUE;
		/* Shift 키 입력 확인 */
		else if(Scancode == 0x36 || Scancode == 0x2A)
		{
			KStat.ShiftFlag = TRUE;
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
	/* Pause 키 값은 무시 */
	if(KStat.PauseFlag > 0)
	{
		KStat.PauseFlag--;
		return FALSE;
	}

	/* Pause 키는 다음으로 두번의 키 값이 들어옴 */
	if(Scancode == 0xE1)
	{
		*Asciicode = 0x00;
		KStat.PauseFlag = 2;
		return FALSE;
	}
	/* 확장형 키코드는 첫번째 키 값은 날리고
	   다음부터 오는 값을 사용 */
	else if(Scancode == 0xE0)
	{
		*Asciicode = 0x00;
		return FALSE;
	}

	/* Key down 일때만 Ascii 리턴 */
	/* 추후엔 키 업 했을때 플레그를 추가해서 확인 가능하게 수정해야함 */
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
