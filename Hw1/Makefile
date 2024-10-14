obj-m += led_driver.o
obj-m += seg_driver.o

all: RPI_build
clean: RPI_clean

RPI_build: 
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(PWD) M=$(shell pwd) modules
 
RPI_clean:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(PWD) M=$(shell pwd) clean
 
.PHONY: 
	all clean RPI_build RPI_clean