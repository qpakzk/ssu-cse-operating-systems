all: bootloader kernel disk.img

bootloader: dummy
	@echo 
	@echo =============== Start bootloader Build ===============
	@echo  
	make -C boot/i386
	@echo	
	@echo =============== complete bootloader Build ===============
	@echo

dummy:

kernel: dummy
	@echo 
	@echo =============== Start Kernel Build ===============
	@echo 
	make -C kernel
	make -C kernel2
	make -C kernel3
	@echo	
	@echo =============== Complete Kernel Build ===============
	@echo

disk.img: dummy 
	@echo
	@echo =============== Start Disk Image Build ===============
	@echo
	cat bootloader.bin boot1.bin Main.bin > disk.bin
	#dd if=disk.bin of=disk.img bs=10240000 seek=0 count=1 conv=notrunc
	#dd if=/dev/zero of=disk.img bs=10240000 seek=0 count=1
	dd if=disk.bin of=disk.img bs=10240000 seek=0 count=1 conv=notrunc
	dd if=Main2.bin of=disk.img bs=5120000 seek=1 count=1 conv=notrunc
	dd if=Main3.bin of=disk.img bs=7680000 seek=1 count=1 conv=notrunc
	cp -f disk.img ../bochs
	@echo
	@echo =============== Complete Disk Image build ================
	@echo
	@echo
	@echo =============== Complete all Build ===============
	@echo

run:
	make -C ../bochs run

disk:
	bximage

set:
	ctags -R
	touch tmp
	mkcscope.sh < tmp
	rm tmp

rr: all run 
	   
cr: clean rr

rd: all debug

cd: clean rd

debug:
	make -C ../bochs debug

clean:
	make -C boot/i386 clean
	make -C kernel clean
	make -C ../bochs clean
	rm -rf disk.bin
	rm -rf bootloader.bin
	rm -rf Main.bin
	rm -rf boot1.bin
	rm -rf tags ../tags
	rm -rf cscope.* ../cscope.*

