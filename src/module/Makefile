obj-m := rubi.o
#rubi-objs := rubi_tcp_hook.o rubi_udp_hook.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
indent:
	Lindent *.c *.h
