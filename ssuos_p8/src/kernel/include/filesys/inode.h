#ifndef	   __INODE_H__
#define	   __INODE_H__

#include <synch.h>
#include <filesys/fs.h>
#include <proc/proc.h>

#define		INO_ROOTDIR						2
#define		NUM_DIRECT						2
#define 		NUM_INDIRECT						8
#define		NUM_INODE						256
#define		FILENAME_LEN					60
#define		INODE_PER_BLOCK					64

#define		SSU_TYPE_INVALID				0
#define		SSU_TYPE_FILE					1
#define		SSU_TYPE_DIR					2


//64byte inode
struct inode
{
	uint32_t sn_ino;						// inode number. In SSU-FS, i-number is inode-table index
	uint32_t sn_size;						// file size(Byte)
	uint16_t sn_type;						// file type
	uint16_t sn_nlink;						// number of using direct block
	uint32_t sn_refcount; 					// open count
	uint32_t sn_directblock[NUM_DIRECT];	// index of data block
	uint32_t sn_indirectblock[NUM_INDIRECT]; //index of indirect data block
	struct ssu_fs * sn_fs;							// filesystem of inode
	uint32_t cnt_data_block;					// count of indirect  block
};


//디렉토리 파일에 쓰이는 내용을 스트럭쳐화 해놓은것
/*
 * On-disk directory entry
 */
struct direntry {
	uint32_t de_ino;			/* Inode number */
	char de_name[FILENAME_LEN];		/* Filename */
};

struct inode inode_tbl[NUM_INODE];

void init_inode_table(struct ssu_fs *fs);
struct inode* inode_create(struct ssu_fs *fs, uint16_t type);
int inode_write(struct inode * in, uint32_t offset, char * buf, int len);
int inode_read(struct inode * in, uint32_t offset, char * buf, int len);
static int sync_inode(struct ssu_fs *fs, struct inode* inode);
static void clear_inode(struct inode* in);
struct inode *inode_open(const char *pathname);

int make_dir(struct inode *cwd, char *name);
static int num_direntry(struct inode *in);

int lbn_to_pbn(struct inode * in, uint32_t lbn );


//flag, mode is not used.
int open_namei(const char *path, int flag, int mode, struct inode* ret);
int path_lookup(const char *path, struct inode* ret);

void list_segment(struct inode *cwd);
int change_dir(struct inode *cwd, char *path);

int get_curde(struct inode *cwd, struct direntry * de);
#endif
