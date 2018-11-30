#ifndef __SSU_FILE_H__
#define __SSU_FILE_H__

#include <filesys/inode.h>

#define NR_FILEDES 5

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END -1

struct ssufile {
	struct inode *inode;
	uint16_t pos;
	uint8_t flags;
	uint8_t unused;
};

int file_seek(uint32_t fd, uint16_t pos);
int file_open(struct inode *inode, int flags, int mode);
int file_close(uint32_t fd);
int file_read(struct inode *file, size_t offset, void *buf, size_t len);
int file_write(struct inode *file, size_t offset, void *buf, size_t len);
//int file_close(uint32_t fd );

#endif
