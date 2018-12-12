#include <filesys/inode.h>
#include <device/ata.h>
#include <device/block.h>
#include <bitmap.h>
#include <device/console.h>
#include <string.h>

//Block 12~15 valid inode number = 256
//inode size = 64byte, block size = 4096byte, 64 inode/block

char tmpblock[SSU_BLOCK_SIZE];
char tmpblock_indirect[SSU_BLOCK_SIZE]; //4096
extern struct process *cur_process;

void init_inode_table(struct ssu_fs *fs)
{
	struct bitmap *imap = fs->fs_inodemap;
	int i;

	for(i=0; i<NUM_INODEBLOCK; i++)
		fs_readblock(fs, SSU_INODE_BLOCK + i, ((char*)inode_tbl) + (i * SSU_BLOCK_SIZE));	

	if(!bitmap_test(imap, INO_ROOTDIR))
	{
		memset(inode_tbl, 0, sizeof(struct inode) * NUM_INODE);
		//Unvalid, Reserved
		bitmap_set(imap, 0, true);
		bitmap_set(imap, 1, true);

		inode_tbl[0].sn_ino = 0;
		inode_tbl[0].sn_size = 0;
		inode_tbl[0].sn_type = SSU_TYPE_DIR;
		inode_tbl[0].sn_nlink = 0;
		inode_tbl[0].sn_refcount = 0;
		inode_tbl[0].sn_fs = fs;

		//Root directory set

		make_dir(inode_tbl, ".");
		sync_inode(fs, inode_tbl + INO_ROOTDIR);
		sync_bitmapblock(fs);
	}

	cur_process->rootdir = inode_tbl + INO_ROOTDIR;
	cur_process->cwd = cur_process->rootdir;
}

struct inode* inode_create(struct ssu_fs *fs, uint16_t type)
{
	struct inode *new_inode;
	struct bitmap *imap = fs->fs_inodemap;
	int idx;

	for(idx = 0; idx <NUM_INODE; idx++)
		if(!bitmap_test(imap, idx))
			break;

	if(idx >= NUM_INODE)
		return NULL;

	new_inode = inode_tbl + idx;
	new_inode->sn_ino = idx;
	new_inode->sn_size = 0;
	new_inode->sn_type = type;
	new_inode->sn_nlink = 0;
	new_inode->sn_refcount = 0;
	new_inode->sn_fs = fs;
	new_inode->cnt_data_block = 0;

	bitmap_set(imap, idx, true);

	sync_inode(fs, new_inode);
	sync_bitmapblock(fs);

	return new_inode;
}

int inode_write(struct inode *in, uint32_t offset, char * buf, int len)
{
	int result=0;
	struct ssu_fs * fs = in->sn_fs;
	//시작 lbn
	uint32_t start_blkoff = offset / SSU_BLOCK_SIZE;
	//시작 datablock의 오프셋
	uint32_t start_off = offset % SSU_BLOCK_SIZE;
	//마지막 lbn
	uint32_t end_blkoff = (offset + len) / SSU_BLOCK_SIZE;
	//마지막 datablock의 오프셋
	uint32_t end_off = (offset + len) % SSU_BLOCK_SIZE;
	//오프셋이 파일 크기를 초과하면 안됨
	if(offset > in->sn_size)
		return -1;

	//data block 개수만큼 write
	for(int lbn = start_blkoff, i = 0; lbn <= end_blkoff; lbn++, i++) {
		int pbn = lbn_to_pbn(in, lbn);
		int off;
		//pbn에 해당하는 data block을 read
		memset(tmpblock, 0, SSU_BLOCK_SIZE);
		fs_readblock(fs, pbn, tmpblock);

		//한 블록인 경우
		if(lbn == start_blkoff && lbn == end_blkoff)
			memcpy(tmpblock + start_off, buf, len);
		//첫 블록인 경우
		else if(lbn == start_blkoff)
			memcpy(tmpblock + start_off, buf, SSU_BLOCK_SIZE - start_off);
		//마지막 블록인 경우
		else if(lbn == end_blkoff) {
			off = len - end_off;
			memcpy(tmpblock, buf + off, end_off);
		}
		//중간 블록인 경우
		else {
			off = SSU_BLOCK_SIZE - start_off;
			if(i > 1)
				off += (i - 1) * SSU_BLOCK_SIZE;
			memcpy(tmpblock, buf + off, SSU_BLOCK_SIZE);
		}

		//lbn에 해당하는 pbn에 write
		fs_writeblock(fs, pbn, tmpblock);
	}

	//파일 크기 갱신
	if(in->sn_size < offset+len)
		in->sn_size = offset+len;
	sync_inode(fs, in);

	//lbn과 pbn의 시작과 마지막 오프셋 출력
	printk("start_blkoff = %d\n", start_blkoff);
	printk("end_blkoff = %d\n", end_blkoff);
	printk("physical start_blkoff = %d\n", lbn_to_pbn(in, start_blkoff));
	printk("physical end_blkoff = %d\n", lbn_to_pbn(in, end_blkoff));

	return result;
}

int inode_read(struct inode * in, uint32_t offset, char * buf, int len)
{

	//modify the inode_write to activate indirect blocks.
	int result=0;
	struct ssu_fs * fs = in->sn_fs;
	uint32_t blkoff = offset / SSU_BLOCK_SIZE;
	uint32_t res_off = offset % SSU_BLOCK_SIZE;

	if(offset > in->sn_size)
		return -1;
	
	memset(tmpblock, 0, SSU_BLOCK_SIZE);
	fs_readblock(fs, in->sn_directblock[blkoff], tmpblock);

	memcpy(buf, tmpblock + res_off, len);
	return result;

	//
}

int lbn_to_pbn(struct inode * in, uint32_t lbn )
{
	struct ssu_fs * fs = in->sn_fs;
	int pbn=0;

	//lbn이 direct block일 경우
	if(lbn < NUM_DIRECT) {
		//direct block의 인덱스
		pbn = in->sn_directblock[lbn];

		//direct block이 할당되어 있지 않을 경우
		if(pbn == 0) {
			//data block 할당
			balloc(fs->fs_blkmap, &pbn);
			sync_bitmapblock(fs);
			in->sn_nlink++;

			in->sn_directblock[lbn] = pbn;
		}
	}
	//lbn이 indirect block일 경우
	else {
		//indirect block의 인덱스
		int blk_idx = (lbn - NUM_DIRECT) / 1024;
		//indirect block의 오프셋
		int blkoff = (lbn - NUM_DIRECT) % 1024;

		//in->cnt_data_block: indirect block으로 할당된 data block의 개수
		//indirect block으로 할당되어 있지 않은 data block을 찾는 경우
		if(in->cnt_data_block <= (lbn - NUM_DIRECT)) {
			//indirect block이 할당되어 있지 않은 경우
			if((in-> cnt_data_block == 0) ||
				//필요로 하는 indirect block이 할당되어 있지 않은 경우
				(((in->cnt_data_block - 1) / 1024) < blk_idx)) {
				//indirect block 할당
				//indirect block을 위한 block bitmap 설정
				balloc(fs->fs_blkmap, &(in->sn_indirectblock[blk_idx]));
				sync_bitmapblock(fs);
			}

			//indirect block을 read하여 tmpblock_indirect에 저장한다
			memset(tmpblock_indirect, 0, SSU_BLOCK_SIZE);
			fs_readblock(fs, in->sn_indirectblock[blk_idx], tmpblock_indirect);

			//indirect block으로 할당할 data block의 block bitmap 설정
			balloc(fs->fs_blkmap, &pbn);
			sync_bitmapblock(fs);
			in->cnt_data_block++;

			//indirect block으로 할당한 data block의 pbn을 indirect block에 write한다
			memcpy(tmpblock_indirect + blkoff * sizeof(int), &pbn, sizeof(int));
			fs_writeblock(fs, in->sn_indirectblock[blk_idx], tmpblock_indirect);
		}
		//indirect block을 통해 lbn에 매칭되는 data block 찾기
		else {
			memset(tmpblock_indirect, 0, SSU_BLOCK_SIZE);
			fs_readblock(fs, in->sn_indirectblock[blk_idx], tmpblock_indirect);
			memcpy(&pbn, tmpblock_indirect + blkoff * sizeof(int), sizeof(int));
		}
	}
	return pbn;
}

struct inode *inode_open(const char *pathname)
{
	struct inode *inode_cur;
	struct inode *new_in;
	struct direntry buf_dir;
	char fname_buf[FILENAME_LEN];
	int ndir;
	int i;

	inode_cur = cur_process->cwd;
	for(i=0; pathname[i] != '\0'; i++)
		fname_buf[i] = pathname[i];
	fname_buf[i] = '\0';

	ndir = num_direntry(inode_cur);
	for(i=0; i<ndir; i++)
	{   
		inode_read(inode_cur, i*sizeof(struct direntry), (char *)&buf_dir, sizeof(struct direntry));
		if(strncmp(buf_dir.de_name, fname_buf,FILENAME_LEN) == 0) return &(inode_tbl[buf_dir.de_ino]); // 중복 이름 존재, 생성 없이 리턴
	}

	new_in = inode_create(cur_process->cwd->sn_fs,SSU_TYPE_FILE);
	buf_dir.de_ino = new_in->sn_ino;
	strlcpy(buf_dir.de_name, fname_buf, FILENAME_LEN);
	inode_write(inode_cur, num_direntry(inode_cur)*sizeof(buf_dir), (char *)&buf_dir, sizeof(buf_dir)); //상위 inode에 추가

	return new_in;
}


static int sync_inode(struct ssu_fs *fs, struct inode* inode)
{
	int result = 0;
	int offset = inode->sn_ino / INODE_PER_BLOCK; //offset to current inode index from SSU_INODE_BLOCK 

	result = fs_writeblock(fs, SSU_INODE_BLOCK + offset, (char*)(inode_tbl + offset*INODE_PER_BLOCK));
	return result;
}


int make_dir(struct inode *cwd, char *name)
{
	struct direntry newde, cde, pde, tmp;
	struct inode * newin;
	struct ssu_fs *fs = cwd->sn_fs;
	int ndir, i;
	int len = strnlen(name, FILENAME_LEN);

	if(len > FILENAME_LEN || len <= 0)
	{
		printk("Unvalid filename length.\n");
		return -1;	
	}

	if(cwd->sn_ino >= INO_ROOTDIR)
	{
		ndir = num_direntry(cwd);
		for(i=0; i<ndir; i++)
		{
			inode_read(cwd, i*sizeof(struct direntry), (char*)&tmp, sizeof(struct direntry));
			if( strncmp(name, tmp.de_name, len) == 0 && tmp.de_name[len] == 0)
			{
				printk("Already exist filename.\n");
				return -1;
			}
		}
	}

	newin = inode_create(fs, SSU_TYPE_DIR);

	newde.de_ino = newin->sn_ino;
	strlcpy(newde.de_name, name, len+1);


	if(cwd->sn_ino >= INO_ROOTDIR)
		inode_write(cwd, cwd->sn_size, (char*)&newde, sizeof(struct direntry));
	else
		cwd = newin;

	cde.de_ino = newde.de_ino;
	strlcpy(cde.de_name, ".", 2);

	pde.de_ino = cwd->sn_ino;
	strlcpy(pde.de_name, "..", 3);	

	inode_write(newin, newin->sn_size, (char*)&pde, sizeof(struct direntry));
	inode_write(newin, newin->sn_size, (char*)&cde, sizeof(struct direntry));

	return 0;
}

static int num_direntry(struct inode *in)
{
	if(in->sn_size % sizeof(struct direntry) != 0 || in->sn_type != SSU_TYPE_DIR)
		return -1;

	return in->sn_size / sizeof(struct direntry);
}


void list_segment(struct inode *cwd)
{
	int i;
	int ndir = num_direntry(cwd);
	struct inode *in;
	struct direntry de;

	printk("name | size | type | blocks | ino\n");
	for(i=0; i<ndir; i++)
	{
		inode_read(cwd, i*sizeof(struct direntry), (char*)&de, sizeof(struct direntry));
		in = &inode_tbl[de.de_ino];

		printk("%s | %d | %c | %d | %d\n", de.de_name, in->sn_size, 
				(in->sn_type == SSU_TYPE_DIR) ? 'd' : 'n', in->sn_nlink, in->sn_ino);
	}
}

int change_dir(struct inode *cwd, char *path)
{
	int i;
	int ndir = num_direntry(cwd);
	struct inode *in;
	struct direntry de;
	int len = strnlen(path, FILENAME_LEN);

	if(path == 0)
	{
		cur_process->cwd = cur_process->rootdir;
		return -1;
	}

	for(i=0; i<ndir; i++)
	{
		inode_read(cwd, i*sizeof(struct direntry), (char*)&de, sizeof(struct direntry));

		if( strncmp(path, de.de_name, len) == 0 && de.de_name[len] == 0)
		{
			in = &inode_tbl[de.de_ino];
			if(in->sn_type != SSU_TYPE_DIR)
			{
				printk("Not a Directory.\n");
				return -1;
			}
			cur_process->cwd = in;
			return 0;
		}
	}
	printk("Not found directory\n");
	return -1;
}

int get_curde(struct inode *cwd, struct direntry * de)
{
	struct inode *pwd;
	int i, ndir;

	//get parent dir
	inode_read(cwd, 0, (char*)de, sizeof(struct direntry));
	pwd = &inode_tbl[de->de_ino];
	ndir = num_direntry(pwd);
	for(i=2; i<ndir; i++)
	{
		inode_read(pwd, i*sizeof(struct direntry), (char*)de, sizeof(struct direntry));
		if(de->de_ino == cwd->sn_ino)
			return 0;
	}
	return -1;
}
