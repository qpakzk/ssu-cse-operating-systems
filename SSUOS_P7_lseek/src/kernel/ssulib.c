#include <ssulib.h>
#include <device/console.h>
#include <device/io.h>
#include <syscall.h>
#include <filesys/file.h>
#include <string.h>

//option flag
#define NO_OPT 0x0
#define E_OPT  0x1
#define A_OPT  0x2
#define RE_OPT 0x4
#define C_OPT  0x8

// void memcpy(void* from, void* to, uint32_t len)
// {
// 	uint32_t *p1 = (uint32_t*)from;
// 	uint32_t *p2 = (uint32_t*)to;
// 	int i, e;

// 	e = len/sizeof(uint32_t);
// 	for(i = 0; i<e; i++)
// 		p2[i] = p1[i];

// 	e = len%sizeof(uint32_t);
// 	if( e != 0)
// 	{
// 		uint8_t *p3 = (uint8_t*)p1;
// 		uint8_t *p4 = (uint8_t*)p2;

// 		for(i = 0; i<e; i++)
// 			p4[i] = p3[i];
// 	}
// }

int strncmp(char* b1, char* b2, int len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		char c = b1[i] - b2[i];
		if(c)
			return c;
		if(b1[i] == 0)
			return 0;
	}
	return 0;
}

bool getkbd(char *buf, int len) 
{
	char ch;
	int offset = 0;

	len--;

	for(; offset < len && buf[offset] != 0; offset++)
		if(buf[offset] == '\n')
		{
			for(;offset>=0;offset--)
				buf[offset] = 0;
			offset++;
			break;
		}

	while ( (ch = ssuread()) >= 0)
	{
		if(ch == '\b' && offset == 0)
		{
			set_cursor();
			continue;
		}
		printk("%c",ch);
		set_cursor();
		if (ch == '\b')
		{
			buf[offset-1] = 0;
			offset--;
		}
		else if (ch == '\n')
		{
			buf[offset] = ch;
			return FALSE;
		}
		else
		{
			buf[offset] = ch;
			offset++;
		}
		if(offset == len) offset--;
	}
	
	/*
	{
		if (ch == '\b')
		{
			if(offset == 0)
			{
				set_cursor();
				continue;
			}
			buf[offset-1] = 0;
			offset -= 2;
			printk("%c",ch);
		}
		else if (ch == '\n')
		{
			buf[offset] = ch;
			printk("%c",ch);
			return FALSE;
		}
		else
		{
			buf[offset] = ch;
			printk("%c",ch);
		}

		if(offset < len) offset++;
	}*/

	return TRUE;
}


int getToken(char* buf, char token[][BUFSIZ], int max_tok)
{
	int num_tok = 0;
	int off_tok = 0;
	while(num_tok < max_tok && *buf != '\n')
	{
		if(*buf == ' ') 
		{
			token[num_tok][off_tok] = 0;
			while(*buf == ' ') buf++;
			off_tok = 0;
			num_tok++;
		}
		else
		{
			token[num_tok][off_tok] = *buf;
			off_tok++;
			buf++;
		}
	}
	token[num_tok][off_tok] = 0;
	num_tok++;


	return num_tok;
}

int generic_read(int fd, void *buf, size_t len)
{
	struct ssufile *cursor;
	uint16_t *pos =  &(cur_process->file[fd]->pos);

	if( (cursor = cur_process->file[fd]) == NULL)
		return -1;

	if (~cursor->flags & O_RDONLY)
		return -1;

	//오류 수정
	if (*pos + len > cursor->inode->sn_size)
		len = cursor->inode->sn_size - *pos;

	file_read(cur_process->file[fd]->inode,*pos,buf,len);
	*pos += len;
	//printk("in generic read : %d \n", *pos);
	return len;
}

int generic_write(int fd, void *buf, size_t len)
{
	struct ssufile *cursor;
	uint16_t *pos =  &(cur_process->file[fd]->pos);

	if( (cursor = cur_process->file[fd]) == NULL)
		return -1;

	if (~cursor->flags & O_WRONLY)
		return -1;

	//오류 수정
	file_write(cur_process->file[fd]->inode,*pos,buf,len);
	*pos += len;
	//printk("in generic write : %d \n", *pos);
	return len;
}

int generic_lseek(int fd, int offset, int whence, char *opt)
{
	struct ssufile *cursor;
	uint16_t *pos = &(cur_process->file[fd]->pos);
	int location;
	int file_size;
	int flag;
	int diff;
	int i;
	char buf[BUFSIZ];

	//cursor가 가리키고 있는 ssufile 구조체가 없을 경우
	if( (cursor = cur_process->file[fd]) == NULL)
		return -1;

	//option에 따라 플래그 설정
	if(opt == NULL)
		flag = NO_OPT;
	else if(!strcmp(opt, "-e"))
		flag = E_OPT;
	else if(!strcmp(opt, "-a"))
		flag = A_OPT;
	else if(!strcmp(opt, "-re"))
		flag = RE_OPT;
	else if(!strcmp(opt, "-c"))
		flag = C_OPT;

	//cursor가 가리키는 파일의 크기
	file_size = cursor->inode->sn_size;

	switch(whence) {
		case SEEK_END:
			//파일 크기 = 파일 끝 위치
			location = file_size;
			break;
		case SEEK_SET:
			//파일 처음 위치
			location = 0;
			break;
		case SEEK_CUR:
			//현재 위치
			location = *pos;
			break;
	}

	//a 옵션일 경우
	if(flag == A_OPT) {
		//offset의 음수 여부 판단 플래그
		bool is_negative = false;

		//offset이 음수일 경우
		if(offset < 0) {
			//플래그를 true로 전환
			is_negative = true;
			//절댓값으로 설정
			offset = -offset;
		}

		memset(buf, 0x00, sizeof(buf));
		//파일 포인터 처음 위치로 설정
		*pos = 0;
		//파일 전체 내용 read
		read(fd, buf, file_size);

		//read 시 파일 포인터가 변경되었으므로 처음으로 재설정
		*pos = 0;
		//마지막으로 설정된 파일 포인터 전까지의 파일 내용을 write
		for(i = 0; i < location; i++)
			write(fd, &buf[i], 1);

		//offset만큼 공간 추가
		for(i = 0; i < offset; i++)
			write(fd, "0", 1);

		//마지막으로 설정된 파일 포인터 이후 파일 내용 write
		for(i = location; i < file_size; i++)
			write(fd, &buf[i], 1);

		//offset이 음수인 경우 파일 포인터를 움직이지 않을 것이므로 offset을 0으로 초기화
		if(is_negative)
			offset = 0;
	}
	//whence에 해당하는 위치에 offset 적용
	location += offset;

	//시작 범위를 벗어날 경우
	if(location < 0) {
		//re 옵션일 경우
		if(flag == RE_OPT) {
			//파일 포인터 처음 위치로 설정
			*pos = 0;

			//파일 전체 내용 read
			memset(buf, 0x00, BUFSIZ);
			read(fd, buf, file_size);

			//read 시 파일 포인터가 변경되었으므로 처음으로 재설정
			*pos = 0;

			//offset 적용 후 파일 포인터의 음수 위치값의 절대값이 추가해야 할 공간이 됨
			diff = -location;

			//공간 추가
			for(i = 0; i < diff; i++)
				write(fd, "0", 1);
			//기존 파일 내용 write
			write(fd, buf, file_size);

			//마지막 파일 포인터 위치를 0으로 설정
			*pos = 0;
		}
		//c 옵션일 경우
		else if(flag == C_OPT) {
			//순환 구조이므로 파일 크기만큼 더해줌
			location += file_size;
			//파일 포인터 변경
			*pos = location;
		}
		//옵션이 없거나 e 옵션일 경우, e 옵션은 시작 지점을 넘어서는 경우에 대한 기능이 없음
		else if(flag == NO_OPT || flag == E_OPT)
			return -1;
	}
	//끝 범위를 벗어날 
	else if(location > file_size) {
		//e 옵션의 경우
		if(flag == E_OPT) {
			//파일 크기를 넘어선 만큼이 추가해야 할 공간
			diff = location - file_size;
			//파일 포인트를 끝으로 옮김
			*pos = file_size;
			//공간 추가
			for(i = 0; i < diff; i++)
				write(fd, "0", 1);
		}
		//c 옵션일 경우
		else if(flag == C_OPT) {
			//순환 구조이므로 modulo 연산 수행
			*pos = location % file_size;
		}
		//옵션이 없거나 re 옵션일 경우, re 옵션은 끝 지점을 넘어서는 경우에 대한 기능이 없음
		else if(flag == NO_OPT || flag == RE_OPT)
			return -1;
	}
	//범위 내의 경우
	else
		*pos = location;

	return *pos;
}
