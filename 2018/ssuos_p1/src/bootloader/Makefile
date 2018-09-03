all: bootprint.bin

bootprint.bin: bootprint.asm
	nasm -f bin bootprint.asm -o bootprint.bin
	cp bootprint.bin ../
clean:
	rm -rf bootprint.bin
