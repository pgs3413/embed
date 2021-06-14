
ifeq ($(KERNELRELEASE), )
	KERNELDIR ?= /home/linux/workdir/kernel/linux-3.0.15

	PWD := $(shell pwd)

modules :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	
modules_install :
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean :
	rm -f *.o *.mod* *.order *.symvers

.PHONY : modules modules_install clean

else
	obj-m +=fs4412_key_drv.o

endif 
	
