all:
	make -C ./bootloader
	make -C ./ucos2


clean:
	make clean -C ./bootloader
	make clean -C ./ucos2
















