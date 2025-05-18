KERNELDIR := /home/user/linux/imx6ull/driver/linux_kernel
CURRENT_PATH := $(shell pwd)
obj-m := character_device.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean