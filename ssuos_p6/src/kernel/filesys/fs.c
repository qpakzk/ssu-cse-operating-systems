#include <filesys/fs.h>
#include <filesys/vnode.h>
#include <filesys/ssufs.h>
#include <proc/proc.h>

//devide type
#define NULL_DEVICE 			0
#define BLK_DEVICE 			1

#define VOLNAME_LEN		16

//filesystem
#define SSUFS 						1
#define PROCFS 					2

struct filesystem_info{
	char name[VOLNAME_LEN];
	uint16_t type;
	uint16_t  filesystem;
	uint32_t superblock_offset;
};

struct filesystem_info filesystem_list[] = {
	{"hdd1", BLK_DEVICE, SSUFS, 2},
	{"proc", NULL_DEVICE, PROCFS, NULL},
};

extern struct process *cur_process;

struct filesystem_info *get_filesystem_info(char *volname){
	int i;

	for(i = 0; i < sizeof(filesystem_list)/sizeof(struct filesystem_info); i++){
		if(strcmp(volname, filesystem_list[i].name) == 0){
			return &filesystem_list[i];
		}
	}

	return NULL;
}

void init_filesystem()
{
	struct vnode *root_vnode;

	root_vnode = do_mount("hdd1", "/");

	cur_process->rootdir = root_vnode;
	cur_process->cwd = root_vnode;
}

int mount(char *volname, char *dirname){
	struct vnode *mnt_vnode;

	mnt_vnode = do_mount(volname, dirname);
}

struct vnode *do_mount(char *volname, char *dirname){
	struct filesystem_info *fs_info;
	struct vnode *mnt_vnode;

	fs_info = get_filesystem_info(volname);

	if(fs_info == NULL){
		printk("no volname in filesystem list\r\n");
		return NULL;
	}

	//check dirname
	if(cur_process->cwd == NULL)
		mnt_vnode = vnode_alloc();
	else{
		mnt_vnode = find_vnode(dirname);

		if(mnt_vnode == NULL){
			printk("no directory in current directory\n");
			return NULL;
		}
	}

	switch(fs_info->filesystem){
		case SSUFS:
		mnt_vnode = init_ssufs(fs_info->name, fs_info->superblock_offset, mnt_vnode);
		break;
		case PROCFS:
		mnt_vnode = init_procfs(mnt_vnode);
		break;

	}

	return mnt_vnode;
}

