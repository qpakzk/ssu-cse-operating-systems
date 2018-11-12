#ifndef __FS_H__
#define __FS_H__

#include <filesys/vnode.h>

int mount(char *volname, char *dirname);
struct vnode *do_mount(char *volname, char *dirname);
#endif