#include <filesys/ssufs.h>
#include <filesys/procfs.h>
#include <filesys/vnode.h>
#include <proc/proc.h>
#include <device/console.h>
#include <list.h>
#include <string.h>
#include <ssulib.h>
#include <ctype.h>

extern struct list p_list;
extern struct process *cur_process;

struct vnode *init_procfs(struct vnode *mnt_vnode)
{
	mnt_vnode->v_op.ls = proc_process_ls;
	mnt_vnode->v_op.mkdir = NULL;
	mnt_vnode->v_op.cd = proc_process_cd;

	return mnt_vnode;
}

int proc_process_ls()
{
	int result = 0;
	struct list_elem *e;

	printk(". .. ");
	for(e = list_begin (&p_list); e != list_end (&p_list); e = list_next (e))
	{
		struct process* p = list_entry(e, struct process, elem_all);

		printk("%d ", p->pid);
	}
	printk("\n");

	return result;
}

int proc_process_cd(char *dirname)
{
	struct vnode *vnode = vnode_alloc();
	vnode->v_parent = cur_process->cwd;
	memcpy(vnode->v_name, dirname, FILENAME_LEN);

	if(isdigit(dirname[0])) {
		cur_process->cwd = vnode;
		vnode->v_op.ls = proc_process_info_ls;
		vnode->v_op.cat = proc_process_info_cat;
		vnode->v_op.cd = proc_process_info_cd;
	}
}

int proc_process_info_ls()
{
	int result = 0;

	printk(". .. ");
	printk("cwd root time stack");

	printk("\n");

	return result;
}

int proc_process_info_cd(char *dirname)
{
	struct vnode *vnode = vnode_alloc();
	vnode->v_parent = cur_process->cwd;
	memcpy(vnode->v_name, dirname, FILENAME_LEN);

	if(!strcmp(dirname, "cwd") || !strcmp(dirname, "root")) {
		cur_process->cwd = vnode;
		vnode->info = (char *)dirname;
		vnode->v_op.ls = proc_link_ls;
	}
	else {
		return -1;
	}

	return 0;
}

int proc_process_info_cat(char *filename)
{
	//cat time : process의 time_used 출력
	if(!strcmp(filename, "time")) {
		printk("time_used : %lu\n", cur_process->time_used);
	}
	//cat stack : process의 stack 출력
	else if(!strcmp(filename, "stack")) {
		printk("stack : %x\n", cur_process->stack);
	}
	else {
		return -1;
	}

	return 0;
}

int proc_link_ls()
{
	char *dirname;
	struct vnode *vnode = cur_process->rootdir;
	struct vnode *child;
	struct list_elem *e;

	dirname = (char *)cur_process->cwd->info;

	if(!strcmp(dirname, "root")) {
		vnode = cur_process->rootdir;
	}
	else if(!strcmp(dirname, "cwd")) {
		vnode = cur_process->cwd;
	}
	else {
		return -1;
	}

	printk(". .. ");
	for(e = list_begin(&vnode->childlist); e != list_end(&vnode->childlist); e = list_next(e))
	{
		child = list_entry(e, struct vnode, elem);
		printk("%s ", child->v_name);
	}
	printk("\n");

	return 0;
}
