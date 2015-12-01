#Declaration for kernel directory
KDIR ?= ~/Kernels/linux-socfpga
#Declaration for C file cross compile
CC = ~/Kernels/gcc-linaro-arm-linux-gnueabihf-4.7/bin/arm-linux-gnueabihf-gcc

all:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean
	
help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help

The Kbuild file:
obj-m += platform_spi_device.o
obj-m += platform_spi.o
