[BITS 16]

START:   
mov		ax, 0xb800
mov		es, ax
mov		ax, 0x00
mov		bx, 0
mov		cx, 80*25*2

CLS:
mov		[es:bx], ax
add		bx, 1
loop 	CLS
