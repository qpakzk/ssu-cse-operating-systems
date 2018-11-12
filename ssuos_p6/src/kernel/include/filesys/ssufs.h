#ifndef __SSUFS_H__
#define __SSUFS_H__

#include <filesys/vnode.h>
#include <type.h>

#define 	VOLNAME_LEN					16
#define		SSU_SB_MAGIC					0x19950404
#define		SSU_BLOCK_SIZE					4096

#define		SECTORCOUNT(dev)				(SSU_BLOCK_SIZE / dev->blk_size)

#define 	NUM_INODE_BLOCK				4
#define		NUM_INODE						(NUM_INODE_BLOCK*SSU_BLOCK_SIZE/sizeof(struct ssufs_inode))
#define		NUM_DIRECT						10
#define 	INODE_PER_BLOCK				((int)(SSU_BLOCK_SIZE/sizeof(struct ssufs_inode)))
#define		INODE_ROOT						2

#define 	SSU_REG_TYPE						0
#define 	SSU_DIR_TYPE						1

#define DEVOP_READ(dev, sec, buf)	(dev->ops->read(dev->device_info, sec, buf))
#define DEVOP_WRITE(dev, sec, buf)	(dev->ops->write(dev->device_info, sec, buf))

#define		SSU_BITMAP_BLOCK(superblock)		(superblock+1)
#define		SSU_INODE_BLOCK(superblock)		(superblock+2)
#define		SSU_DATA_BLOCK(superblock)		(superblock+6)

#define		FILENAME_LEN					56

struct ssufs_superblock{
	uint32_t			sb_magic;
	uint32_t			sb_nblocks;
	char				sb_volname[VOLNAME_LEN];
	struct blk_dev *	blkdev;
	struct 	bitmap *	blkmap;
	struct 	bitmap *	inodemap;
	uint32_t			lba;
};

struct ssufs_inode{
	uint32_t		i_no;
	uint32_t		i_size;
	uint32_t		i_type;
	uint32_t		i_refcount;
	uint32_t 		i_nlink;
	struct 	ssufs_superblock*	ssufs_sb;
	uint32_t		i_direct[NUM_DIRECT];
};

struct dirent{
	uint32_t		d_ino;
	uint32_t		d_type;
	char			d_name[FILENAME_LEN];
};

static struct ssufs_inode ssufs_inode_table[NUM_INODE];

struct vnode *init_ssufs(char *volname, uint32_t lba, struct vnode *mnt_root);
int ssufs_readblock(struct ssufs_superblock *sb, uint32_t blknum, char *buf);
int ssufs_writeblock(struct ssufs_superblock *sb, uint32_t blknum, char *buf);
int ssufs_load_databitmapblock(struct ssufs_superblock *sb);
int ssufs_load_inodebitmapblock(struct ssufs_superblock *sb);
int ssufs_load_inodetable(struct ssufs_superblock *sb);

int ssufs_sync(struct ssufs_superblock *sb);
int ssufs_inode_write(struct ssufs_inode *inode, uint32_t offset, char *buf, uint32_t len);
int ssuf_inode_read(struct inode *inode, uint32_t offset, char *buf, uint32_t len);
struct ssufs_inode *inode_alloc(uint32_t type);

struct vnode *make_vnode_tree(struct ssufs_superblock *sb, struct vnode *mnt_root);
void set_vnode(struct vnode *vnode, struct vnode *parent_vnode, struct ssufs_inode *inode);
int get_curde(struct ssufs_inode *cwd, struct dirent * de);
int ssufs_mkdir(char *dirname);


#endif