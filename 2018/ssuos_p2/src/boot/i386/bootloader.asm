org	0x7c00   

[BITS 16]

START:   
		jmp		BOOT1_LOAD ;BOOT1_LOAD로 점프

BOOT1_LOAD:
	mov     ax, 0x0900 
        mov     es, ax
        mov     bx, 0x0

        mov     ah, 2	
        mov     al, 0x4		
        mov     ch, 0	
        mov     cl, 2	
        mov     dh, 0		
        mov     dl, 0x80

        int     0x13	
        jc      BOOT1_LOAD

	call	CLEAR_SCREEN
	call	PRINT_MENU

	mov	bx, 0
	push	bx
	push	select
	call	PRINT_MSG
	add	sp, 4

GET_KEY:
	mov	ah, 0x00
	int	0x16

	cmp	ah, 0x1c
	je	KERNEL_PRINT

	cmp	ah, 0x48
	je	UP	

	cmp	ah, 0x50
	je	DOWN

	jne	GET_KEY

UP:
	call	PRINT_MENU
	sub	bx, 1

	cmp	bx, 0
	jge	MOVE_SELECT

	mov	bx, 0
	jmp	MOVE_SELECT

DOWN:
	call	PRINT_MENU
	add	bx, 1

	cmp	bx, 2
	jle	MOVE_SELECT

	mov	bx, 2
	jmp	MOVE_SELECT	

MOVE_SELECT:
	push	bx
	push 	select
	call	PRINT_MSG
	add	sp, 4

	jmp	GET_KEY

CLEAR_SCREEN:
	push 	bp
	mov 	bp, sp
	
	push 	ax
	push 	es
	push 	bx
	push 	cx

	mov	ax, 0xb800
	mov	es, ax
	mov	bx, 0
	mov	cx, 80*25*2

CLS:
	mov	[es:bx], ax
	add	bx, 1
loop 	CLS

	pop 	cx
	pop 	bx
	pop 	es
	pop 	ax

	mov 	sp, bp
	pop 	bp
	ret

PRINT_MSG:
	push 	bp
	mov	bp, sp
	
	push	ax
	push	es
	push	si
	push	di

	mov	ax, 0xb800
	mov	es, ax

	mov	ax, word [bp + 6]
	mov	si, 160
	mul	si
	mov	di, ax

	mov	si, word [bp + 4]
PRINT_LOOP:
	mov	al, byte [si]
	
	cmp	al, 0
	je	PRINT_END
	
	mov 	byte [es:di], al
	mov 	byte [es:di + 1], 0x07

	add 	di, 2
	add 	si, 1

	jmp 	PRINT_LOOP
PRINT_END:	
	
	pop 	di
	pop 	si
	pop 	es
	pop 	ax

	mov 	sp, bp
	pop 	bp
	ret

PRINT_MENU:
	push bp
	mov bp, sp

	%assign	i 0

	%rep 	3
	push	i  
	%if 	i == 0
		push 	ssuos_1
	%elif 	i == 1
		push 	ssuos_2
	%elif 	i == 2
		push 	ssuos_3
	%else
		jmp 	END
	%endif

	call 	PRINT_MSG
	add 	sp, 4
	%assign	i i + 1
	%endrep
END:	
	mov sp, bp
	pop bp
	ret

	
KERNEL_PRINT:
	cmp	bx, 0
	je	KERNEL1
	
	cmp	bx, 1
	je	KERNEL2	

	cmp	bx, 2
	je 	KERNEL3

KERNEL1:
	add	bx, 3
	push	bx
	push	kernel_1
	jmp	RESULT
KERNEL2:
	add	bx, 3
	push	bx
	push	kernel_2
	jmp	RESULT
KERNEL3:
	add	bx, 3
	push	bx
	push	kernel_3
	jmp	RESULT

RESULT:
	call	PRINT_MSG
	add	sp, 4

	jmp	$

KERNEL_LOAD:
	mov     ax, 0x1000	
        mov     es, ax		
        mov     bx, 0x0		

        mov     ah, 2		
        mov     al, 0x3f	
        mov     ch, 0		
        mov     cl, 0x6	
        mov     dh, 0     
        mov     dl, 0x80  

        int     0x13
        jc      KERNEL_LOAD

jmp		0x0900:0x0000

select db "[O]",0
ssuos_1 db "[ ] SSUOS_1",0
ssuos_2 db "[ ] SSUOS_2",0
ssuos_3 db "[ ] SSUOS_3",0
ssuos_4 db "[ ] SSUOS_4",0
partition_num : resw 1
kernel_1 db "kernel 1", 0
kernel_2 db "kernel 2", 0
kernel_3 db "kernel 3", 0
times   446-($-$$) db 0x00

PTE:
partition1 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition2 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition3 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x98, 0x3a, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition4 db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
times 	510-($-$$) db 0x00
dw	0xaa55
