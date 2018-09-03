;16비트 코드
[BITS 16]
;0x7c00: 메모리 상의 부트로더 위치 
;0x7c00 주소에서 코드 시작
org 0x7c00

START:
;0xb8000: 비디오 메모리 시작 주소
;비디오 메모리 주소 설정
mov ax, 0xb800
mov	es, ax
mov	ax, 0x00
mov	bx, 0
;비디오 메모리 크기
mov	cx, 80*25*2

;비디오 메모리를 0으로 초기화
CLS:
;es:bx = 0xb800:0x00 = 0xb8000
mov	[es:bx], ax
add	bx, 1
loop CLS

;메시지 인덱스로 활용
mov di, MESSAGE
mov si, 0

;메시지 출력
PRINT_MESSAGE:
;인덱스에 해당하는 메시지의 문자를 al에 저장
mov al, byte [di]

;al이 0이면 종료
cmp	al, 0
je END_MESSAGE

;비디오 메모리에 메시지 문자 저장
mov byte [es:si], al
;문자색은 흰색
mov byte [es:si+1], 0x07

;메시지의 다음 문자로 이동
inc di
;비디오 메모리의 다음 위치로 이동
;1바이트는 문자값, 1바이트는 속성값
add si, 2

jmp PRINT_MESSAGE
END_MESSAGE:

jmp $

;출력할 메시지
MESSAGE: db "Hello, Sangwon's World", 0

; 현재 위치에서 510번지까지 0x00으로 채우기
times 510 - ($ - $$) db 0x00

;부트 섹터로 표기하기 위해
;511번지에 0x55 저장
db 0x55
;512번지에 0xaa 저장
db 0xaa
