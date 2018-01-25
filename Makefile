obj-m += android_module.o

ARCH=arm
KERN_DIR=/home/nillyr/goldfish
CROSS_COMPILE=/home/nillyr/arm-eabi-4.6/bin/arm-eabi-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean
