all: bootloader disk.img

bootloader: dummy
	@echo 
	@echo =============== Start bootloader Build ===============
	@echo  
	make -C bootloader
	@echo	
	@echo =============== complete bootloader Build ===============
	@echo

dummy:

disk.img: dummy 
	@echo
	@echo =============== Start Disk Image Build ===============
	@echo
	cat bootprint.bin > disk.bin
	dd if=disk.bin of=disk.img bs=10240000 seek=0 count=1 conv=notrunc
	cp -f disk.img ../bochs
	@echo
	@echo =============== Complete Disk Image build ================
	@echo
	@echo
	@echo =============== Complete all Build ===============
	@echo

run:
	make -C ../bochs run

clean:
	make -C bootloader clean
#	make -C kernel clean
	rm -rf disk.bin
