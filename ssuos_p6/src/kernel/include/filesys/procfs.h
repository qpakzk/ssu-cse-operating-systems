#ifndef __PROCFS_H__
#define __PROCFS_H__

#include <filesys/vnode.h>
#include <type.h>

#define PROC_NAME_LEN			32
#define PROC_DATA_LEN			80

#define NUM_PROC_INODE		64

#define PROC_REG_TYPE			0
#define PROC_DIR_TYPE			1

#define PROC_ROOT				1

struct proc_inode{
	uint32_t p_no;
	uint32_t	pid;
	uint32_t p_type;
	uint32_t p_size;
	char p_name[PROC_NAME_LEN];
	char p_data[PROC_NAME_LEN];
};

static struct proc_inode proc_inode_table[NUM_PROC_INODE];

struct vnode *init_procfs(struct vnode *mnt_vnode);
int proc_process_ls();
int proc_process_cd(char *dirname);
int proc_process_info_ls();
int proc_process_info_cd(char *dirname);
int proc_process_info_cat(char *filename);
int proc_link_ls();

struct proc_inode *proc_inode_alloc();
struct proc_inode *procfs_mkdir(char *name, uint32_t parent, uint32_t pid);
void proc_inode_free(struct proc_inode *);
void proc_make_vnode_tree(struct proc_inode *root_proc_inode, struct vnode *mnt_vnode);
struct vnode *proc_make_process_to_file(struct vnode *parent_vnode, struct proc_inode *proc_inode);
struct vnode *proc_set_vnode(struct vnode *vnode, struct vnode *parent_vnode, struct proc_inode *proc_inode);
int proc_pid_ls();
// int proc_ls();
int proc_pid_cd(char *dirname);
#endif