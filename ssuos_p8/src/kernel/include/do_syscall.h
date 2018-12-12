#ifndef	   __DO_SYSCALL_H__
#define	   __DO_SYSCALL_H__

pid_t do_fork(proc_func func, void* aux1);
void do_exit(int status);
pid_t do_wait(int *status);
int do_ssuread(int type, char* buf, int len);
void do_shutdown(void);
int do_open(const char *pathname, int flags);
int do_read(int fd, char *buf, int len);
int do_write(int fd, const char *buf, int len);
int do_lseek(int fd, int offset, int whence);

#endif
