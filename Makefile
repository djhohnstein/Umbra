CC = gcc
EXTRA_CFLAGS=-I$(PWD)/include -I$(PWD)/src
obj-m += umbra.o
umbra-objs :=  main.o src/ftrace_manager.o src/creds_manager.o src/hookers.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean