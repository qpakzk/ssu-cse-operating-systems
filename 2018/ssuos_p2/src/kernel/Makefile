arch = ./arch
lib = ./lib

all: Main.bin
	
CFLAGS = -O0 -g -I./include -ffreestanding -m32 -nostdlib -nostdinc -static -fno-stack-protector
AFLAGS = -f elf32
ENTRY = OslabMain

dummy:

#Main
Main.o: Main.s 
	gcc -c $^ $(CFLAGS)
Main.s: $(arch)/Main.c
	gcc -c -S $^ $(CFLAGS)

#arch c files
interrupt.o:  $(arch)/interrupt.c
	gcc -c $(arch)/interrupt.c $(CFLAGS)
pit.o: $(arch)/pit.c
	gcc -c $^ $(CFLAGS) 
kbd.o: $(arch)/kbd.c
	gcc -c $^ $(CFLAGS) 
paging.o: $(arch)/paging.c
	gcc -c $^ $(CFLAGS) 
sched.o: $(arch)/sched.c   
	gcc -c $^ $(CFLAGS)
proc.o: $(arch)/proc.c  
	gcc -c $^ $(CFLAGS)
console.o: $(arch)/console.c  
	gcc -c $^ $(CFLAGS)
malloc.o: $(arch)/malloc.c  
	gcc -c $^ $(CFLAGS)
mm.o: $(arch)/mm.c  
	gcc -c $^ $(CFLAGS)

#arch assembly
io.o: $(arch)/io.asm
	nasm $(AFLAGS) $(arch)/io.asm -o io.o
crset.o: $(arch)/crset.asm
	nasm $(AFLAGS) $(arch)/crset.asm -o crset.o
mem.o: $(arch)/mem.asm
	nasm $(AFLAGS) $(arch)/mem.asm -o mem.o

switch.o: $(arch)/switch.s
	gcc -c $^ $(CFLAGS)

#lib
list.o: $(lib)/list.c
	gcc -o $@ -c $^ $(CFLAGS)
vsprintk.o: $(lib)/vsprintk.c
	gcc -o $@ -c $^ $(CFLAGS) 
ssulib.o: $(lib)/ssulib.c
	gcc -o $@ -c $^ $(CFLAGS) 


Main.elf: Main.o io.o pit.o interrupt.o kbd.o crset.o paging.o vsprintk.o list.o \
	        sched.o switch.o proc.o console.o ssulib.o malloc.o mem.o mm.o
	ld -Ttext 0x10000 $^ -o $@ -I./include --entry $(ENTRY) -m elf_i386

Main.bin: Main.elf 
	objcopy -j .text -j .data -j .rodata -j .bss -S -O binary Main.elf Main.bin
	cp Main.bin ../

clean:
	rm -rf *.o
	rm -rf *.s
	rm -rf Main.elf
	rm -rf Main.bin

reset: clean all
