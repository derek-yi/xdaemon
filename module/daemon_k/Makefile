ifneq ($(KERNELRELEASE),)

obj-m:=daemon_k.o

else

ifeq ($(ARCH), arm)
  KERNELDIR = $(kernel_dir)
else
  KERNELDIR:=/lib/modules/$(shell uname -r)/build
endif
#KERNELDIR:=/lib/modules/$(shell uname -r)/build

PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.mod.c *.mod.o *.ko

endif
