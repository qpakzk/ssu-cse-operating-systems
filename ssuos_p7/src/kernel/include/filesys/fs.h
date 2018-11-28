#ifndef		__SSU_FS_H__
#define		__SSU_FS_H__

#include <type.h>

#define		SSU_SB_MAGIC					0x19930921
#define		SSU_BLOCK_SIZE					4096
#define		UNVALID_BLOCK					6
#define		SSU_BOOT_BLOCK					0
#define		SSU_RESERVED_BLOCK				1

#define		SSU_SUPER_BLOCK					2
#define		SSU_BITMAP_BLOCK				3
#define		SSU_INODE_BLOCK					4
#define		SSU_DATA_BLOCK					8

#define		VOLNAME_LEN						20
#define		NUM_INODEBLOCK					4

#define	SECTORCOUNT(dev)			(SSU_BLOCK_SIZE / dev->blk_size)

#define DEVOP_READ(dev, sec, buf)	(dev->ops->read(dev->device_info, sec, buf))
#define DEVOP_WRITE(dev, sec, buf)	(dev->ops->write(dev->device_info, sec, buf))


//Disk superblock, No implement
struct ssu_superblock{
	uint32_t sb_magic;					
	uint32_t sb_nblocks;
	char sb_volname[VOLNAME_LEN];
	uint32_t padding;
};


struct ssu_fs{
	struct ssu_superblock fs_sb;
	struct blk_dev *fs_device;
	struct bitmap *fs_blkmap;
	struct bitmap *fs_inodemap;
};

void init_filesystem(void);
static int fs_mount(struct ssu_fs *fs, struct blk_dev *dev);
static int sync_superblock(struct ssu_fs *fs, const char* superblock);
static int sync_ssufs(struct ssu_fs *fs, const char* inodes, const char* superblock);
int sync_bitmapblock(struct ssu_fs *fs);
void view_bitmap(struct bitmap *b);
static void set_volname(struct ssu_superblock *sb, const char *name);

int fs_readblock(struct ssu_fs *fs, uint32_t blknum, char *buf);
int fs_writeblock(struct ssu_fs *fs, uint32_t blknum, const char *buf);
int balloc(struct bitmap *b, unsigned *index);

static struct ssu_fs filesys;

#endif
