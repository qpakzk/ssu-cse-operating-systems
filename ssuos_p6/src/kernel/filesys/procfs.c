#include <filesys/procfs.h>
#include <filesys/vnode.h>
#include <proc/proc.h>
#include <device/console.h>
#include <list.h>
#include <string.h>
#include <ssulib.h>

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

}

int proc_process_info_cat(char *filename)
{
	//cat time : process의 time_used 출력
	if(!strcmp(filename, "time")) {
		printk("time_used : %llu\n", cur_process->time_used);
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

}
