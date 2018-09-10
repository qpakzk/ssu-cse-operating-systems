org	0x7c00   

[BITS 16]

SECTION .text

START:
	; 스택 설정
	mov	ax, 0x07c0
	mov	ss, ax
	mov	sp, 0x13FE
	mov	bp, 0x13FE

	jmp	BOOT1_LOAD ;BOOT1_LOAD로 점프

BOOT1_LOAD:
	; buffer address pointer [es:bx]
	mov     ax, 0x0900 
	mov     es, ax
	mov     bx, 0x0

	; INT 0x13, AH=0x02: Read Sectors From Drive
	mov     ah, 2
	; Sectors To Read Count
	mov     al, 0x4		
	mov     ch, 0		; cylinder
	mov     cl, 2		; sector
	mov     dh, 0		; head
	mov     dl, 0x80	; drive

	int     0x13	
	jc      BOOT1_LOAD

	; 비디오 메모리를 0으로 초기화하여 화면 지우기
	mov	ax, 0xb800
	mov	es, ax
	mov	bx, 0
	mov	cx, 80*25*2

CLS:
	mov	[es:bx], ax
	inc	bx
	loop 	CLS

	; 3개 커널 목록 출력 
	call 	PRINT_MENU

	; 1번 커널 선택
	mov	word [partition_num], 1
	push	word [partition_num]
	push	select
	call	PRINT
	add	sp, 4

GET_KEY:
	; 키보드 인터럽트
	mov	ah, 0x00
	int	0x16

	; 엔터 키
	cmp	ah, 0x1c
	; 커널로 이동
	je	KERNEL_LOAD

	; 위 화살표 키
	cmp	ah, 0x48
	je	UP	

	; 아래 화살표 키
	cmp	ah, 0x50
	je	DOWN

	jne	GET_KEY

UP:
	; 가리키는 커널 번호를 위 커널 번호로 수정
	mov	bx, word [partition_num]
	dec	bx	
	cmp	bx, 1
	jge	MOVE_SELECT

	jmp	GET_KEY
DOWN:
	; 가리키는 커널 번호를 아래 커널 번호로 수정
	mov	bx, word [partition_num]
	inc	bx
	cmp	bx, 3
	jle	MOVE_SELECT

	jmp	GET_KEY
MOVE_SELECT:
	; 변경된 위치 업데이트
	mov	word [partition_num], bx
	
	; 새로운 위치를 가리키도록 출력
	call	PRINT_MENU
	push	word [partition_num]
	push 	select
	call	PRINT
	add	sp, 4

	jmp	GET_KEY

KERNEL_LOAD:
	; buffer address pointer [es:bx]
	mov     ax, 0x1000	
    	mov     es, ax		
    	mov     bx, 0x0		

	; INT 0x13, AH=0x02: Read Sectors From Drive
    	mov     ah, 2
	; Sectors To Read Count		
    	mov     al, 0x3f

	; 파티션 번호에 따라 해당하는 커널 로딩 
	mov	si, word [partition_num]
	cmp	si, 1
	je	ONE
	
	cmp	si, 2
	je	TWO

	cmp	si, 3
	je	THREE

; 커널 1
ONE:
	; LBA=0x0006 변환
	mov     ch, 0		; cylinder	
	mov     cl, 0x6		; sector
	mov     dh, 0		; head
	jmp	INT
; 커널 2
TWO:
	; LBA=0x2710 변환
	mov	ch, 0x9		; cylinder
	mov	cl, 0x2f	; sector
	mov	dh, 0xe		; head
	jmp	INT
THREE:
	; LBA=0x3a98 변환
	mov     ch, 0xe		; cylinder
	mov     cl, 0x7		; sector
	mov     dh, 0xe		; head
	jmp	INT
INT:
	; drive
	mov     dl, 0x80  

	int     0x13
	jc      KERNEL_LOAD

	jmp	0x0900:0x0000

; 문자열 출력함수
PRINT:
	push 	bp
	mov	bp, sp
	pusha	

	; 비디오 메모리
	mov	ax, 0xb800
	mov	es, ax

	; 파티션 번호
	mov	ax, word [bp + 6]
	dec	ax

	; 파티션 번호에 맞는 라인으로 이동
	mov	si, 160
	mul	si
	mov	di, ax

	; 문자열 출력
	mov	si, word [bp + 4]
PRINT_LOOP:
	mov	al, byte [si]
	
	cmp	al, 0
	je	PRINT_END
	
	mov 	byte [es:di], al
	mov 	byte [es:di + 1], 0x07
	
	inc 	si
	add 	di, 2

	jmp 	PRINT_LOOP
PRINT_END:	
	
	popa
	mov 	sp, bp
	pop 	bp
	ret

; 선택되지 않은 커널 목록 출력
PRINT_MENU:
	push	bp
	mov	bp, sp
	pusha

	%assign	i 1

	; 3회 반복
	%rep 	3

	; 커널 번호와 문자열을 파라미터로 넘겨서 출력 수행
	push	i  
	%if 	i == 1
		push 	ssuos_1
	%elif 	i == 2
		push 	ssuos_2
	%elif 	i == 3
		push 	ssuos_3
	%endif
 
	call 	PRINT
	add 	sp, 4
	%assign	i i + 1
	%endrep
	
	popa
	mov 	sp, bp
	pop 	bp
	ret

select db "[O]",0
ssuos_1 db "[ ] SSUOS_1",0
ssuos_2 db "[ ] SSUOS_2",0
ssuos_3 db "[ ] SSUOS_3",0
ssuos_4 db "[ ] SSUOS_4",0
partition_num : resw 1
times   446-($-$$) db 0x00

PTE:
partition1 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition2 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition3 db 0x80, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x98, 0x3a, 0x00, 0x00, 0x3f, 0x0, 0x00, 0x00
partition4 db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
times 	510-($-$$) db 0x00
dw	0xaa55
