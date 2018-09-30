	.file	"Main.c"
	.text
.Ltext0:
	.comm	IDT,2040,32
	.comm	IDTR,6,4
	.comm	Glob_x,4,4
	.comm	Glob_y,4,4
	.local	MEM_SIZE
	.comm	MEM_SIZE,4,4
	.section	.rodata
.LC0:
	.string	"SSUOS KERNEL 2 START!!!!\n"
	.text
	.globl	OslabMain
	.type	OslabMain, @function
OslabMain:
.LFB0:
	.file 1 "arch/Main.c"
	.loc 1 15 0
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$8, %esp
	.loc 1 16 0
	subl	$12, %esp
	pushl	$.LC0
	call	printk
	addl	$16, %esp
	.loc 1 17 0
	call	main_init
.L2:
	.loc 1 19 0 discriminator 1
	jmp	.L2
	.cfi_endproc
.LFE0:
	.size	OslabMain, .-OslabMain
	.section	.rodata
.LC1:
	.string	"C OslabMain start!!!!\n"
.LC2:
	.string	"%s"
.LC3:
	.string	"Memory Detecting\n"
.LC4:
	.string	"-Memory size = %u bytes\n"
.LC5:
	.string	"-Memory size = %u Kbytes\n"
.LC6:
	.string	"-Memory size = %u Mbytes\n"
.LC7:
	.string	"PIT Intialization\n"
.LC8:
	.string	"Interrupt Intialization\n"
.LC9:
	.string	"Process Initialization\n"
	.text
	.globl	main_init
	.type	main_init, @function
main_init:
.LFB1:
	.loc 1 25 0
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$8, %esp
	.loc 1 26 0
	movl	$0, Glob_x
	.loc 1 27 0
	movl	$2, Glob_y
	.loc 1 29 0
	subl	$8, %esp
	pushl	$.LC1
	pushl	$.LC2
	call	printk
	addl	$16, %esp
	.loc 1 31 0
	call	detect_mem
	.loc 1 32 0
	subl	$8, %esp
	pushl	$.LC3
	pushl	$.LC2
	call	printk
	addl	$16, %esp
	.loc 1 33 0
	call	memSize
	subl	$8, %esp
	pushl	%eax
	pushl	$.LC4
	call	printk
	addl	$16, %esp
	.loc 1 34 0
	call	memSize
	shrl	$10, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$.LC5
	call	printk
	addl	$16, %esp
	.loc 1 35 0
	call	memSize
	shrl	$20, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$.LC6
	call	printk
	addl	$16, %esp
	.loc 1 37 0
	call	init_pit
	.loc 1 38 0
	subl	$8, %esp
	pushl	$.LC7
	pushl	$.LC2
	call	printk
	addl	$16, %esp
	.loc 1 40 0
	call	init_int
	.loc 1 41 0
	subl	$8, %esp
	pushl	$.LC8
	pushl	$.LC2
	call	printk
	addl	$16, %esp
	.loc 1 43 0
	call	proc_init
	.loc 1 44 0
	subl	$8, %esp
	pushl	$.LC9
	pushl	$.LC2
	call	printk
	addl	$16, %esp
	.loc 1 46 0
	call	refreshScreen
	.loc 1 47 0
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	main_init, .-main_init
.Letext0:
	.file 2 "./include/interrupt.h"
	.file 3 "./include/console.h"
	.file 4 "./include/mm.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x16e
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF22
	.byte	0xc
	.long	.LASF23
	.long	.LASF24
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.string	"_ID"
	.byte	0x8
	.byte	0x2
	.byte	0x1c
	.long	0x62
	.uleb128 0x3
	.long	.LASF0
	.byte	0x2
	.byte	0x1e
	.long	0x62
	.byte	0
	.uleb128 0x3
	.long	.LASF1
	.byte	0x2
	.byte	0x1f
	.long	0x62
	.byte	0x2
	.uleb128 0x3
	.long	.LASF2
	.byte	0x2
	.byte	0x20
	.long	0x62
	.byte	0x4
	.uleb128 0x3
	.long	.LASF3
	.byte	0x2
	.byte	0x21
	.long	0x62
	.byte	0x6
	.byte	0
	.uleb128 0x4
	.byte	0x2
	.byte	0x7
	.long	.LASF7
	.uleb128 0x5
	.string	"_ID"
	.byte	0x2
	.byte	0x22
	.long	0x25
	.uleb128 0x6
	.long	.LASF4
	.byte	0x6
	.byte	0x2
	.byte	0x24
	.long	0x99
	.uleb128 0x3
	.long	.LASF5
	.byte	0x2
	.byte	0x26
	.long	0x62
	.byte	0
	.uleb128 0x3
	.long	.LASF6
	.byte	0x2
	.byte	0x27
	.long	0x99
	.byte	0x2
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.long	.LASF8
	.uleb128 0x7
	.long	.LASF4
	.byte	0x2
	.byte	0x28
	.long	0x74
	.uleb128 0x4
	.byte	0x1
	.byte	0x8
	.long	.LASF9
	.uleb128 0x4
	.byte	0x1
	.byte	0x6
	.long	.LASF10
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.long	.LASF11
	.uleb128 0x4
	.byte	0x8
	.byte	0x5
	.long	.LASF12
	.uleb128 0x4
	.byte	0x1
	.byte	0x6
	.long	.LASF13
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.long	.LASF14
	.uleb128 0x8
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x4
	.byte	0x8
	.byte	0x7
	.long	.LASF15
	.uleb128 0x9
	.long	.LASF16
	.byte	0x1
	.byte	0xe
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x9
	.long	.LASF17
	.byte	0x1
	.byte	0x18
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0xa
	.long	.LASF25
	.byte	0x4
	.byte	0x8
	.long	0x99
	.uleb128 0x5
	.byte	0x3
	.long	MEM_SIZE
	.uleb128 0xb
	.long	0x69
	.long	0x126
	.uleb128 0xc
	.long	0x126
	.byte	0xfe
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.long	.LASF18
	.uleb128 0xd
	.string	"IDT"
	.byte	0x2
	.byte	0x2a
	.long	0x116
	.uleb128 0x5
	.byte	0x3
	.long	IDT
	.uleb128 0xe
	.long	.LASF19
	.byte	0x2
	.byte	0x2b
	.long	0xa0
	.uleb128 0x5
	.byte	0x3
	.long	IDTR
	.uleb128 0xe
	.long	.LASF20
	.byte	0x3
	.byte	0xc
	.long	0xd5
	.uleb128 0x5
	.byte	0x3
	.long	Glob_x
	.uleb128 0xe
	.long	.LASF21
	.byte	0x3
	.byte	0xd
	.long	0xd5
	.uleb128 0x5
	.byte	0x3
	.long	Glob_y
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF16:
	.string	"OslabMain"
.LASF4:
	.string	"_IDTR"
.LASF24:
	.string	"/home/kym/os/ssuos_p2/ssuos_p2 (test)/src/kernel2"
.LASF2:
	.string	"Type"
.LASF21:
	.string	"Glob_y"
.LASF15:
	.string	"long long unsigned int"
.LASF9:
	.string	"unsigned char"
.LASF22:
	.string	"GNU C11 5.4.0 20160609 -m32 -mtune=generic -march=i686 -g -O0 -ffreestanding -fno-stack-protector"
.LASF0:
	.string	"LowOffset"
.LASF8:
	.string	"long unsigned int"
.LASF7:
	.string	"short unsigned int"
.LASF3:
	.string	"HighOffset"
.LASF20:
	.string	"Glob_x"
.LASF19:
	.string	"IDTR"
.LASF11:
	.string	"unsigned int"
.LASF17:
	.string	"main_init"
.LASF10:
	.string	"char"
.LASF25:
	.string	"MEM_SIZE"
.LASF5:
	.string	"Size"
.LASF1:
	.string	"CodeSelector"
.LASF18:
	.string	"sizetype"
.LASF12:
	.string	"long long int"
.LASF14:
	.string	"short int"
.LASF6:
	.string	"Address"
.LASF13:
	.string	"signed char"
.LASF23:
	.string	"arch/Main.c"
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
