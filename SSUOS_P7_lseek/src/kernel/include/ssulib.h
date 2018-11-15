#ifndef __SSULIB_H__
#define __SSULIB_H__

#include <type.h>
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)

#define BUFSIZ 128

void memcpy(void* from, void* to, uint32_t len);
int strncmp(char* b1, char* b2, int len);
bool getkbd(char *buf, int len);
int getToken(char* buf, char tok[][BUFSIZ], int max);
int generic_read(int fd, void *buf, size_t len);
int generic_write(int fd, void *buf, size_t len);
int generic_close(int fd);
#endif
