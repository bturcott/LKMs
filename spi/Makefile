#Declaration for kernel directory
KDIR ?= ~/Kernels/linux-socfpga
#Declaration for C file cross compile
CC = ~/Kernels/gcc-linaro-arm-linux-gnueabihf-4.7/bin/arm-linux-gnueabihf-gcc
#Source and executable are for companion C program if applicable
#SOURCE = custom_leds.c
#EXECUTABLE = lkm_test

default:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR)
	#$(CC) $(SOURCE) -o $(EXECUTABLE)
clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean
	
help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help

The Kbuild file:
obj-m := fpga_spi.o



