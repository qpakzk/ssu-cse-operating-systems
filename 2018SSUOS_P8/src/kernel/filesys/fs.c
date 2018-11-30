#include <filesys/inode.h>
#include <filesys/fs.h>
#include <bitmap.h>
#include <device/block.h>
#include <device/ata.h>
#include <device/console.h>
#include <string.h>


void view_bitmap(struct bitmap *b);

static unsigned char bitmapblock[SSU_BLOCK_SIZE];
extern struct blk_dev ata1_blk_dev;

void init_filesystem(void)
{
	fs_mount(&filesys, &ata1_blk_dev);
	printk("mount device : [%s], nblocks[%d]\n", filesys.fs_sb.sb_volname, filesys.fs_sb.sb_nblocks);

	init_inode_table(&filesys);
}

//For DEBUG
void view_bitmap(struct bitmap *b)
{
	int i,j;
	int cnt = 48;

	for(j=0; j<cnt; )
	{
		for(i=0; i<8 && j<cnt; i++,j++)
			printk("%d ", bitmap_test(b, j) ? 1 : 0);

		printk("    ");

		for(i=0; i<8 && j<cnt; i++,j++)
			printk("%d ", bitmap_test(b, j) ? 1 : 0);

		printk("\n");
	}
	
	printk("\n");
}

int
fs_mount(struct ssu_fs *fs, struct blk_dev *dev)
{
	int result = 0;
	int i;

	char superblock[SSU_BLOCK_SIZE];

	fs->fs_device = dev;


	result = fs_readblock(fs, SSU_SUPER_BLOCK, superblock);
	if(result)
	{
		printk("Read Superblock Error");
		return -1;
	}
	fs->fs_sb.sb_magic = *(uint32_t*)superblock;
	if(fs->fs_sb.sb_magic != SSU_SB_MAGIC)
	{
		fs->fs_sb.sb_magic = SSU_SB_MAGIC;

		if(dev->type == BLK_DEV_HDD)
			set_volname(&fs->fs_sb, "hdd0");

		fs->fs_sb.sb_nblocks = (fs->fs_device->blk_count / (SSU_BLOCK_SIZE / dev->blk_size));	
	}
	else
	{
		fs->fs_sb.sb_nblocks = *((uint32_t*)(superblock + sizeof(uint32_t)));
		set_volname(&fs->fs_sb, superblock + sizeof(uint32_t) * 2); 
	}


	//load bitmap block from HDD
	result = fs_readblock(fs, SSU_BITMAP_BLOCK, bitmapblock);

	//load block bitmap
	fs->fs_blkmap = (struct bitmap*) bitmapblock;
	if(fs->fs_blkmap->bit_cnt != fs->fs_sb.sb_nblocks)
	{
		fs->fs_blkmap = bitmap_create_in_buf(fs->fs_sb.sb_nblocks, bitmapblock, SSU_BLOCK_SIZE/2);
		for(i=0; i<SSU_DATA_BLOCK; i++)
		{
			bitmap_set(fs->fs_blkmap, i, true);
		}

		printk("initialize block bitmap : %d bit\n", fs->fs_blkmap->bit_cnt);
	}
	else
		printk("load block bitmap : %d bit\n", fs->fs_blkmap->bit_cnt);


	//load inode bitmap
	fs->fs_inodemap = (struct bitmap*)(bitmapblock + SSU_BLOCK_SIZE/2);
	if(fs->fs_inodemap->bit_cnt != NUM_INODE)
	{
		fs->fs_inodemap = bitmap_create_in_buf(NUM_INODE, bitmapblock + SSU_BLOCK_SIZE/2, SSU_BLOCK_SIZE/2);
		printk("initialize inode bitmap : %d bit\n", fs->fs_inodemap->bit_cnt);
	}
	else
		printk("load inode bitmap : %d bit\n", fs->fs_inodemap->bit_cnt);

		
	sync_ssufs(fs, 0, (char*)(&fs->fs_sb));
	return result;
}

int fs_readblock(struct ssu_fs *fs, uint32_t blknum, char *buf)
{ 
	int result = 0;
	int startsec = blknum * SECTORCOUNT(fs->fs_device);
	int i;
	for(i=0; i<SECTORCOUNT(fs->fs_device); i++)
		DEVOP_READ(fs->fs_device, startsec + i, buf + (i * fs->fs_device->blk_size));
	
	return result;
}

int fs_writeblock(struct ssu_fs *fs, uint32_t blknum, const char *buf)
{
	int result = 0;
	int startsec = blknum * SECTORCOUNT(fs->fs_device);
	int i;
	
	for(i=0; i<SECTORCOUNT(fs->fs_device); i++)
		DEVOP_WRITE(fs->fs_device, startsec + i, buf + (i * fs->fs_device->blk_size));
	
	return result;
}


int
balloc(struct bitmap *b, unsigned *index)
{
	int i;
	int end = bitmap_size(b);

	for(i = 0; i <end; i++)
	{
		if(!bitmap_test(b, i))
		{
			*index = i;
			bitmap_set(b, i, true);
			return i;
		}
	}

	return -1;
}

int 
sync_bitmapblock(struct ssu_fs *fs)
{
	int result = 0;

	result = fs_writeblock(fs, SSU_BITMAP_BLOCK, bitmapblock);

	return result;
}

static int 
sync_superblock(struct ssu_fs *fs, const char* superblock)
{
	int result = 0;

	result = fs_writeblock(fs, SSU_SUPER_BLOCK, superblock);

	return result;
}

static int 
sync_ssufs(struct ssu_fs *fs, const char* inodes, const char* superblock)
{
	int result = 0;

	result = sync_bitmapblock(fs);
	if(result)
	{
		printk("sync_bitmapblock() error\n");
		return -1;
	}

	result = sync_superblock(fs, superblock);
	if(result)
	{
		printk("sync_superblock() error\n");
		return -1;
	}
}

static void
set_volname(struct ssu_superblock *sb, const char *name)
{
	int i;
	for(i=0; name[i] != '\0' && i < VOLNAME_LEN; i++)
		sb->sb_volname[i] = name[i];
	
	sb->sb_volname[i] = '\0';
}
