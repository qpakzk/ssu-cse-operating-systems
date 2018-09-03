#ifndef __PAGING_H__
#define __PAGING_H_

#define CR0_FLAG_PG			0x80000000		// Paging Flag
#define CR0_FLAG_PE			0x00000001		// Protect Mode Flag

#define PAGE_FLAG_PRESENT	0x01			// Present
#define PAGE_FLAG_RW		0x02			// Read & Write
#define PAGE_FLAG_USER		0x04			// User level

#define PAGE_SIZE			4096			// 4kb Page
#define PAGE_TBL_SIZE		4096			// 4kb Page Table
#define PAGE_DIR_SIZE		4096			// 4kb Page Directory

#define PAGE_DIR_ADDR		0x00020000		// 0x20000 ~ 0x20FFF : # of Page Table = 1024
#define PAGE_TBL_ADDR		0x00021000		// 0x21000 ~ 

void Init_Paging();

#endif
