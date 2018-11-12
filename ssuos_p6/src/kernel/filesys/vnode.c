#include <filesys/vnode.h>
#include <proc/proc.h>
#include <ssulib.h>
#include <string.h>
#include <list.h>

struct vnode *vnode_alloc(){
	int i;

	for(i=1; i < NUM_VNODE; i++){
		if(vnode_table[i].v_no == NULL){
			vnode_table[i].v_no = i;
			list_init(&vnode_table[i].childlist);
			return &vnode_table[i];
		}
	}

	return NULL;
}

void vnode_free(struct vnode *vnode){
	int vnode_num = vnode->v_no;

	memset(&vnode_table[vnode_num], 0x00, sizeof(struct vnode));
}

struct vnode *find_vnode(char *name){
	struct list_elem *e;
	struct vnode *cwd = cur_process->cwd;

	for(e = list_begin (&cwd->childlist); e != list_end (&cwd->childlist); e = list_next (e))
	{
		struct vnode* v = list_entry(e, struct vnode, elem);
		if(strcmp(v->v_name, name) == 0)
			return v;
	}

	return NULL;
}

struct vnode *copy_vnode(struct vnode *ori_vnode, struct vnode *des_vnode)
{
	uint32_t v_no = des_vnode->v_no;

	memcpy(des_vnode, ori_vnode, sizeof(struct vnode));

	des_vnode->v_no = v_no;
}