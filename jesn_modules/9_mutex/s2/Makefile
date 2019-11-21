obj-m += jesn_m9_2.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)



default :
	$(MAKE) -C $(KDIR) M=$(PWD) modules


clean :
	rm *.mod.*
	rm *.ko
	rm *.o
