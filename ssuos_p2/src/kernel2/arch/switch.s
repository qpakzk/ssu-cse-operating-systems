//#include "../include/switch.h"

.set SWITCH_CUR, 20
.set SWITCH_NEXT, 24

.global switch_process

.func switch_process
switch_process:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %ebp
	pushl %esi
	pushl %edi

.global process_stack_ofs
	mov process_stack_ofs, %edx

	movl 32(%esp), %eax
	movl %esp, (%eax, %edx, 1)

	movl 36(%esp), %eax
	movl (%eax, %edx, 1), %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax

	ret
