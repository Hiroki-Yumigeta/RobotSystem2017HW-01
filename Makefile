obj-m:=swled.o
swled.ko: swled.c
	make -C /usr/src/linux M=`pwd` V=1 modules
clean:
	make -C /usr/src/linux M=`pwd` V=1 clean

