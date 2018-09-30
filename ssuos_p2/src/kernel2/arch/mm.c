#include <mm.h>

void detect_mem()
{
	memSize();
	return;
}

unsigned long nGetMemSize()
{
	unsigned long *pAddr = (unsigned long *)0x20000; 
	unsigned long tmp;
	while (1) {
		pAddr += (4*1024*1024);
		tmp = *pAddr;
		*pAddr = 0x12345678;
		if(*pAddr != 0x12345678) {
			pAddr -= (4*1024*1024);
			while (1) {
				pAddr += (4*1024);
				tmp = *pAddr;
				*pAddr = 0x87654321;
				if(*pAddr != 0x87654321)
					break;
				*pAddr = tmp;
			}
			break;
		}
		*pAddr = tmp;
	}         
                      
	return (unsigned long)pAddr;
}

unsigned long memSize()
{
	if( MEM_SIZE == 0 )
	{
		MEM_SIZE = nGetMemSize();
	}
	return MEM_SIZE;
}
