#ifndef __VNODE_H__
#define __VNODE_H__

#include <type.h>
#include <list.h>

#define NUM_VNODE 		256
#define LEN_VNODE_NAME	64

#define REG_TYPE			0
#define DIR_TYPE			1

struct vnodeop{
	int (*mkdir)(char *);
	int (*ls)();
	int (*cd)(char *);
	int (* cat)(char *);
};

struct vnode {
	uint32_t v_no;
	uint32_t type;
	struct vnode *v_parent;
	char v_name[LEN_VNODE_NAME];
	struct vnodeop v_op;
	struct list childlist;
	struct list_elem elem;
	void *info;
};

static struct vnode vnode_table[NUM_VNODE];

struct vnode *vnode_alloc();
void vnode_free(struct vnode *vnode);
struct vnode *find_vnode(char *name);
struct vnode *copy_vnode(struct vnode *ori_vnode, struct vnode *des_vnode);
#endif